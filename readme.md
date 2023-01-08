
This file is part of Astonia Client (c) Daniel Brockhaus. Please read license.txt.

This is the new and (soon to be) shiny Astonia 3 client.

To compile it, install https://www.msys2.org/

Then run

pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-SDL2 mingw-w64-x86_64-libpng make

to install the 64 bit GCC compiler toolchain, the SDL2 library and the
PNG library. And make, of course.

Then download the old client source code archive from my website at:

https://brockhaus.org/astonia3.html

It contains next to the old source code a lot of graphics and other resources
you'll need. Unzip it to a folder of your liking. Then download these sources
from Github into the moac directory. This will overwrite several files.

Then run

make

to compile.


