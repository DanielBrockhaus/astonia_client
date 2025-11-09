#pragma once
#include <stddef.h>
#include <stdint.h>

/* Opaque handle type; defined in Rust. */
struct astonia_sock;
typedef struct astonia_sock astonia_sock;

/* Connect non-blocking to host:port.
   If timeout_ms >= 0, waits up to timeout_ms for the socket to become writable (connected).
   Returns NULL on failure/timeout. */
astonia_sock* astonia_net_connect(const char* host, uint16_t port, int timeout_ms);

/* Poll readiness. mask: bit 1=READ, bit 2=WRITE.
   Returns bitmask (1/2/3), 0 on timeout, -1 on error. */
int astonia_net_poll(astonia_sock* s, int mask, int timeout_ms);

/* Receive up to cap bytes into dst.
   Returns >0 = bytes read, 0 = closed, -1 = would-block/error. */
ptrdiff_t astonia_net_recv(astonia_sock* s, void* dst, size_t cap);

/* Send up to len bytes from src.
   Returns >0 = bytes sent, 0 = treated as closed, -1 = would-block/error. */
ptrdiff_t astonia_net_send(astonia_sock* s, const void* src, size_t len);

/* If the peer is IPv4, write its address (network byte order) to *out_be.
   Returns 0 on success, -1 on error or if peer is IPv6. */
int astonia_net_peer_ipv4(astonia_sock* s, uint32_t* out_be);

/* Close and free the handle (safe to call with NULL). */
void astonia_net_close(astonia_sock* s);
