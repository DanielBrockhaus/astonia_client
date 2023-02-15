/*
 * Part of Astonia Map Mod (c) Daniel Brockhaus
 *
 * See https://brockhaus.org/modding.html
 *
 * Use "make amod" to build.
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../../src/amod/amod.h"

#if 0
#include "strings.c"
#include "ranks.c"
#include "skills.c"
#endif

static int sx,sy,visible;
#define MAXMAP  256
static unsigned char _mmap[MAXMAP*MAXMAP];

__declspec(dllexport) void amod_gamestart(void) {
    note("Map Mod v0.1 loaded.");

    sx=dotx(DOT_MTL)+50;
    sy=doty(DOT_MTL)+10;

    memset(_mmap,0,sizeof(_mmap));
    visible=1;
}

__declspec(dllexport) void amod_frame(void) {
    int x,y,xs,xe,ox,oy;

    ox=originx-DIST;
    oy=originy-DIST;

    for (y=0; y<=DIST*2; y++) {
        if (y+oy<0) continue;
        if (y+oy>=MAXMAP) continue;

		if (y<DIST) { xs=DIST-y; xe=DIST+y; }
		else { xs=y-DIST; xe=DIST*3-y; }

		for (x=xs; x<=xe; x++) {
            if (x+ox<0) continue;
            if (x+ox>=MAXMAP) continue;
            if (!(map[x+y*MAPDX].flags&CMF_VISIBLE)) continue;

            if (map[x+y*MAPDX].mmf&MMF_SIGHTBLOCK) {
                _mmap[ox+x+(oy+y)*MAXMAP]=1;
            } else _mmap[ox+x+(oy+y)*MAXMAP]=2;
        }
    }

    if (visible) {
        for (y=0; y<MAXMAP; y++) {
            for (x=0; x<MAXMAP; x++) {
                if (x==originx && y==originy) dd_pixel(sx+x,sy+y,IRGB(15,5,5));
                else if (x==originx+1 && y==originy) dd_pixel(sx+x,sy+y,IRGB(15,5,5));
                else if (x==originx && y==originy+1) dd_pixel(sx+x,sy+y,IRGB(15,5,5));
                else if (x==originx-1 && y==originy) dd_pixel(sx+x,sy+y,IRGB(15,5,5));
                else if (x==originx && y==originy-1) dd_pixel(sx+x,sy+y,IRGB(15,5,5));
                else if (_mmap[x+y*MAXMAP]==1) dd_pixel(sx+x,sy+y,IRGB(15,15,15));
                else if (_mmap[x+y*MAXMAP]==2) dd_pixel(sx+x,sy+y,IRGB(7,7,7));
                else dd_pixel(sx+x,sy+y,IRGB(3,3,3));
            }
        }
    }
}

__declspec(dllexport) void amod_areachange(void) {
    memset(_mmap,0,sizeof(_mmap));
}

__declspec(dllexport) int amod_keydown(int key) {
    if (key==9) {
        visible^=1;
        return 1;
    }
    return 0;
}

