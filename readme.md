
This file is part of Astonia Client (c) Daniel Brockhaus. Please read license.txt.

This is the new and (soon to be) shiny Astonia 3 client.

To compile it, install https://www.msys2.org/

Then run

```
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-SDL2 mingw-w64-x86_64-libpng mingw-w64-x86_64-SDL2_mixer mingw-w64-x86_64-libzip make
```

to install the 64 bit GCC compiler toolchain, a bunch of libraries and make.

Then clone or download the sources from Github to the current directory.

And run


```
make
```

to compile.


