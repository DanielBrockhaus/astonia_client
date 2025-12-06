# We Have Moved!
Any possible future development will happen at [AstoniaCommunity](https://github.com/AstoniaCommunity/astonia_community_client).



# Astonia Client
This file is part of Astonia Client (c) Daniel Brockhaus. Please read license.txt.

## Windows Executable
You can find a ready-to-run client that works with a vanilla server under
releases: https://github.com/DanielBrockhaus/astonia_client/releases

## Build from Source
If you wish to build your own, install https://www.msys2.org/

Make sure that "c:\msys64\mingw64\bin" and "C:\msys64\usr\bin" (if you installed
into the default directory) end up in your Windows PATH.

Open a shell (CMD.EXE or Powershell) and run

```
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-SDL2 mingw-w64-x86_64-libpng
pacman -S mingw-w64-x86_64-SDL2_mixer mingw-w64-x86_64-libzip make zip
pacman -S mingw64/mingw-w64-x86_64-dwarfstack
```

to install the 64 bit GCC compiler toolchain, a bunch of libraries, make and
zip.

Also install git lfs (large file system) as described here: https://docs.github.com/en/repositories/working-with-files/managing-large-files/installing-git-large-file-storage

Clone or download the sources from Github to the current directory.

And run


```
make
```

to compile.

### Errors
If pacman, make or gcc are not found something with your MSYS2 installation
went wrong. The same goes for missing headers during compile or missing
libraries during linking. And even missing DLLs when running the client point
to an incomplete MSYS2 installation.

### Make Distribution
```
make distrib
```
Will copy all neccessary DLLs to the build directory and create a zip archive
with the executables and graphics.

### Make A Mod
```
make amod
```
To compile your mod (located in src/amod/). More information about modding the
client is on my website: https://brockhaus.org/modding.html

