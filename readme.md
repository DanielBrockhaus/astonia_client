# Astonia Client
This file is part of Astonia Client (c) Daniel Brockhaus. Please read license.txt.

This is the new and shiny Astonia 3 client.

## Windows Executable
You can find a ready-to-run client that works with a vanilla server under
releases: https://github.com/DanielBrockhaus/astonia_client/releases

## Build from Source

### Windows Build
If you wish to build your own, install https://www.msys2.org/

Make sure that "c:\msys64\clang64\bin" and "C:\msys64\usr\bin" (if you installed
into the default directory) end up in your Windows PATH.

Open a shell (CMD.EXE or Powershell) and run

```
pacman -S mingw-w64-clang-x86_64-clang mingw-w64-clang-x86_64-SDL2 mingw-w64-clang-x86_64-libpng
pacman -S mingw-w64-clang-x86_64-SDL2_mixer mingw-w64-clang-x86_64-libzip make zip
pacman -S mingw-w64-clang-x86_64-dwarfstack mingw-w64-clang-x86_64-zig
```

to install the 64 bit Clang compiler toolchain, libraries, make, zip, and Zig.

#### Install Rust
In the MSYS2 shell, install Rust using:
```
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y --default-toolchain stable --target x86_64-pc-windows-gnu
```

Also install git lfs (large file system) as described here: https://docs.github.com/en/repositories/working-with-files/managing-large-files/installing-git-large-file-storage

Clone or download the sources from Github to the current directory.

#### Build with Zig (Recommended)
```
zig build --release=fast
```

#### Build with Make (Traditional)
```
make
```

### Errors
If pacman, make, clang, or zig are not found something with your MSYS2 installation
went wrong. The same goes for missing headers during compile or missing
libraries during linking. And even missing DLLs when running the client point
to an incomplete MSYS2 installation.

### Make Distribution
```
make distrib
```
Will copy all neccessary DLLs to the build directory and create a zip archive
with the executables and graphics.

### Linux Build
For Linux, you can use the provided Dockerfile to build in a containerized environment:
```
docker build -f Dockerfile.linux -t astonia-linux-build .
docker run --rm -i -e HOST_UID=$(id -u) -e HOST_GID=$(id -g) -v "$PWD:/app" -w /app astonia-linux-build
```

The built binaries will be in `zig-out/bin/`.

### Make A Mod
```
make amod
```
To compile your mod (located in src/amod/). More information about modding the
client is on my website: https://brockhaus.org/modding.html

