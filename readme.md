
This file is part of Astonia Client (c) Daniel Brockhaus. Please read license.txt.

This is the new and (soon to be) shiny Astonia 3 client.

To compile it, install https://www.msys2.org/

Make sure that "C:\msys64\usr\bin" and "C:\msys64\usr\bin" (if you installed
into the default directory) end up in your Windows PATH.

Open a shell (CMD.EXE or Powershell) and run

```
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-SDL2 mingw-w64-x86_64-libpng
pacman -S mingw-w64-x86_64-SDL2_mixer mingw-w64-x86_64-libzip make
```

to install the 64 bit GCC compiler toolchain, a bunch of libraries and make.

Clone or download the sources from Github to the current directory.

And run


```
make
```

to compile.

If pacman, make or gcc are not found something with your MSYS2 installation
went wrong. The same goes for missing headers during compile or missing
libraries during linking. And even missing DLLs when running the client point
to an incomplete MSYS2 installation.

