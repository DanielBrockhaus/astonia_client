
This file is part of Astonia Client (c) Daniel Brockhaus. Please read license.txt.

This is the new and (soon to be) shiny Astonia 3 client.

To compile it, install https://www.msys2.org/

Then run

```
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-SDL2 mingw-w64-x86_64-libpng mingw-w64-x86_64-SDL2_mixer make
```

to install the 64 bit GCC compiler toolchain, the SDL2 graphics and audio library and the
PNG library. And make, of course.

Create a directory structure like this:

```
mkdir astonia3
cd astonia3
mkdir moac
mkdir gfx
cd moac
```

Then clone or download the sources from Github to the current directory
("astonia3/moac").

Then download https://brockhaus.org/astonia_client_pngs.zip and extract it in
the main folder ("astonia3"). This should put the "x1","x2", etc. folders into
"astonia3/gfx" resulting in, e.g. "astonia3/gfx/x1".

Go back into the moac folder and run

```
make
```

to compile.


