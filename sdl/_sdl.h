/*
 * Part of Astonia Client (c) Daniel Brockhaus. Please read license.txt.
 */

#ifndef HAVE_DDFONT
#define HAVE_DDFONT
struct ddfont {
    int dim;
    unsigned char *raw;
};
#endif

#define DDT             '°' // draw text terminator - (zero stays one, too)

int sdl_ic_load(int sprite);


#define MAX_SOUND_CHANNELS   32
#define MAXSOUND            100

