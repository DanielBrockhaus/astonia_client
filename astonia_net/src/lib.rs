use std::ffi::CStr;
use std::io;
use std::net::{IpAddr, SocketAddr, ToSocketAddrs};
use std::os::raw::{c_char, c_int};
use std::time::{Duration, Instant};

use mio::net::TcpStream as MioTcp;
use mio::{Events, Interest, Poll, Token};

#[cfg(unix)]
use std::os::unix::io::AsRawFd;

#[cfg(windows)]
use std::os::windows::io::AsRawSocket;

const POLL_READ: c_int = 1;
const POLL_WRITE: c_int = 2;
const TOKEN: Token = Token(0);

pub struct AstoniaSock {
    poll: Poll,
    events: Events,
    mio: MioTcp,
    connecting: bool,
}

#[inline]
fn resolve_one(host: &str, port: u16) -> Option<SocketAddr> {
    if let Ok(ip) = host.parse::<IpAddr>() {
        Some(SocketAddr::new(ip, port))
    } else {
        (host, port).to_socket_addrs().ok().and_then(|mut it| it.next())
    }
}

#[inline]
fn remaining_timeout(start: Instant, total: Option<Duration>) -> Option<Duration> {
    match total {
        None => None,
        Some(t) => {
            let elapsed = start.elapsed();
            if elapsed >= t {
                Some(Duration::from_millis(0))
            } else {
                Some(t - elapsed)
            }
        }
    }
}

// Drive a non-blocking connect until completion or timeout.
//
// Returns Ok(true)   -> now connected
//         Ok(false)  -> still pending (timeout or no writable yet)
//         Err(_)     -> connection error
#[inline(always)]
fn drive_connect(s: &mut AstoniaSock, total_to: Option<Duration>) -> io::Result<bool> {
    if !s.connecting {
        return Ok(true);
    }
    let start = Instant::now();

    loop {
        let to = remaining_timeout(start, total_to);
        if let Some(t) = total_to {
            if start.elapsed() >= t {
                return Ok(false);
            }
        }

        // Respect remaining timeout (or None = wait forever).
        s.poll.poll(&mut s.events, to)?;

        // Proceed only when we see a writable event for our token.
        let mut got_writable = false;
        for ev in s.events.iter() {
            if ev.token() == TOKEN && ev.is_writable() {
                got_writable = true;
                break;
            }
        }

        if !got_writable {
            // No writable yet; if no overall timeout was requested,
            // return pending so the caller/game loop can drive us.
            if total_to.is_none() {
                return Ok(false);
            }
            // Otherwise, continue looping within the (still active) timeout window.
            continue;
        }

        // Writable: check SO_ERROR, then peer_addr to confirm connection.
        match s.mio.take_error()? {
            Some(e) => return Err(e),
            None => match s.mio.peer_addr() {
                Ok(_) => {
                    s.connecting = false;
                    // Connected, allow poll to re-register for READABLE | WRITABLE
                    return Ok(true);
                }
                Err(ref e) if is_still_connecting(e) =>
                {
                    continue; // keep looping within timeout
                }
                Err(e) => return Err(e),
            },
        }
    }
}

#[inline(always)]
fn is_still_connecting(e: &io::Error) -> bool {
    use io::ErrorKind;
    if e.kind() == ErrorKind::WouldBlock || e.kind() == ErrorKind::NotConnected {
        return true;
    }
    match e.raw_os_error() {
        #[cfg(any(unix, target_os = "wasi"))]
        Some(code) if code == libc::EINPROGRESS => true,
        #[cfg(windows)]
        Some(10035) => true, // WSAEWOULDBLOCK
        _ => false,
    }
}

#[inline(always)]
fn try_recv_into(s: &MioTcp, buf: &mut [u8]) -> io::Result<usize> {
    s.try_io(|| {
        #[cfg(unix)]
        unsafe {
            let n = libc::recv(s.as_raw_fd(), buf.as_mut_ptr() as *mut _, buf.len(), 0);
            if n >= 0 { Ok(n as usize) } else { Err(io::Error::last_os_error()) }
        }

        #[cfg(windows)]
        unsafe {
            let n = libc::recvfrom(
                s.as_raw_socket() as usize,
                buf.as_mut_ptr() as *mut libc::c_char,
                buf.len() as i32,
                0,
                std::ptr::null_mut(),
                std::ptr::null_mut(),
            );
            if n >= 0 { Ok(n as usize) } else { Err(io::Error::last_os_error()) }
        }
    })
}

#[inline(always)]
fn try_send_from(s: &MioTcp, buf: &[u8]) -> io::Result<usize> {
    s.try_io(|| {
        #[cfg(unix)]
        unsafe {
            let n = libc::send(s.as_raw_fd(), buf.as_ptr() as *const _, buf.len(), 0);
            if n >= 0 { Ok(n as usize) } else { Err(io::Error::last_os_error()) }
        }

        #[cfg(windows)]
        unsafe {
            let n = libc::sendto(
                s.as_raw_socket() as usize,
                buf.as_ptr() as *const libc::c_char,
                buf.len() as i32,
                0,
                std::ptr::null(),
                0,
            );
            if n >= 0 { Ok(n as usize) } else { Err(io::Error::last_os_error()) }
        }
    })
}

#[unsafe(no_mangle)]
pub extern "C" fn astonia_net_connect(
    host: *const c_char,
    port: u16,
    timeout_ms: c_int,
) -> *mut AstoniaSock {
    if host.is_null() {
        return std::ptr::null_mut();
    }
    let host_str = match (unsafe { CStr::from_ptr(host) }).to_str() {
        Ok(s) if !s.is_empty() => s,
        _ => return std::ptr::null_mut(),
    };
    let addr = match resolve_one(host_str, port) {
        Some(a) => a,
        None => return std::ptr::null_mut(),
    };

    // Non-blocking connect with mio.
    let mut mio = match MioTcp::connect(addr) {
        Ok(m) => m,
        Err(_) => return std::ptr::null_mut(),
    };

    let poll = match Poll::new() {
        Ok(p) => p,
        Err(_) => return std::ptr::null_mut(),
    };
    let token = Token(0);
    if poll
        .registry()
        .register(&mut mio, token, Interest::WRITABLE)
        .is_err()
    {
        return std::ptr::null_mut();
    }

    let mut s = Box::new(AstoniaSock {
        mio,
        poll,
        events: Events::with_capacity(8),
        connecting: true,
    });

    // If caller wants immediate return, leave connection pending.
    if timeout_ms == 0 {
        return Box::into_raw(s);
    }

    // Else, try to complete within timeout.
    let total = if timeout_ms < 0 {
        None
    } else {
        Some(Duration::from_millis(timeout_ms as u64))
    };
    match drive_connect(&mut s, total) {
        Ok(true) | Ok(false) => Box::into_raw(s), // connected or still pending, let game drive it
        Err(_) => std::ptr::null_mut(),
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn astonia_net_poll(
    sock: *mut AstoniaSock,
    mask: c_int,
    timeout_ms: c_int,
) -> c_int {
    let Some(s) = (unsafe { sock.as_mut() }) else { return -1; };

    // nothing requested – avoid work
    if mask == 0 { return 0; }

    // If still connecting, drive connect using the provided timeout.
    if s.connecting {
        let total = if timeout_ms < 0 {
            None
        } else {
            Some(Duration::from_millis(timeout_ms as u64))
        };
        match drive_connect(s, total) {
            Ok(true) => {
                // Connected now; continue to normal poll below.
            }
            Ok(false) => {
                // Still pending; if the caller only asked writable, reflect that we’re not writable yet.
                // Return 0 (no events) rather than block here—game loop keeps calling us.
                return 0;
            }
            Err(_) => return -1,
        }
    }

    // Always re-register in poll no matter what.
    if s.poll.registry().reregister(&mut s.mio, TOKEN, Interest::READABLE | Interest::WRITABLE).is_err() {
        let _ = s.poll.registry().register(&mut s.mio, TOKEN, Interest::READABLE | Interest::WRITABLE);
    }

    // Normal event poll.
    let to = if timeout_ms < 0 {
        None
    } else {
        Some(Duration::from_millis(timeout_ms as u64))
    };
    if s.poll.poll(&mut s.events, to).is_err() {
        return -1;
    }

    let mut res = 0;
    for e in s.events.iter() {
        if e.token() == TOKEN {
            if (mask & POLL_READ) != 0 && e.is_readable() {
                res |= POLL_READ;
            }
            if (mask & POLL_WRITE) != 0 && e.is_writable() {
                res |= POLL_WRITE;
            }
        }
    }
    res
}

#[unsafe(no_mangle)]
pub extern "C" fn astonia_net_recv(sock: *mut AstoniaSock, dst: *mut u8, cap: usize) -> isize {
    let Some(s) = (unsafe { sock.as_mut() }) else { return -1; };
    if dst.is_null() || cap == 0 { return 0; }
    let buf = unsafe { std::slice::from_raw_parts_mut(dst, cap) };

    match try_recv_into(&s.mio, buf) {
        Ok(n) => n as isize,
        Err(e) if e.kind() == io::ErrorKind::WouldBlock => -1,
        Err(_) => -1,
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn astonia_net_send(sock: *mut AstoniaSock, src: *const u8, len: usize) -> isize {
    let Some(s) = (unsafe { sock.as_mut() }) else { return -1; };
    if src.is_null() || len == 0 { return 0; }
    let buf = unsafe { std::slice::from_raw_parts(src, len) };

    match try_send_from(&s.mio, buf) {
        Ok(n) => n as isize,
        Err(e) if e.kind() == io::ErrorKind::WouldBlock => -1,
        Err(_) => -1,
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn astonia_net_peer_ipv4(sock: *mut AstoniaSock, out_be: *mut u32) -> c_int {
    use std::net::SocketAddr;
    let Some(s) = (unsafe { sock.as_mut() }) else { return -1; };
    let peer = match s.mio.peer_addr() {
        Ok(a) => a,
        Err(_) => return -1,
    };
    let v4 = match peer {
        SocketAddr::V4(v4) => v4,
        SocketAddr::V6(_) => return -1,
    };
    if out_be.is_null() { return -1; }
    let be = u32::from_be_bytes(v4.ip().octets());
    (unsafe { *out_be = be; });
    0
}

#[unsafe(no_mangle)]
pub extern "C" fn astonia_net_close(sock: *mut AstoniaSock) {
    if !sock.is_null() {
        drop(unsafe { Box::from_raw(sock) });
    }
}