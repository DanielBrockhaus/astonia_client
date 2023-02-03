/*
 * Part of Astonia Mod (c) ModderMcModFace.
 *
 * See https://brockhaus.org/modding.html
 *
 * Use "make amod" to build.
 *
 */

#if 0
#include "bmod.c"
#else

#include <stdint.h>
#include <stdio.h>
#include "../../src/amod/amod.h"

#include "strings.c"
#include "ranks.c"
#include "skills.c"

static int x,y;

__declspec(dllexport) void amod_gamestart(void) {
    note("A Mod by ModderMcModFace loaded.");
    x=(dotx(DOT_MBR)+dotx(DOT_MTL))/2;
    y=(doty(DOT_MBR)+doty(DOT_MTL))/2;
}

__declspec(dllexport) void amod_frame(void) {
    static int dx=1,dy=1,step=0;
    x+=dx;
    if (x>dotx(DOT_MBR)-2) dx=-1;
    if (x<dotx(DOT_MTL)+2) dx=1;
    y+=dy;
    if (y>doty(DOT_MBR)-2) dy=-1;
    if (y<doty(DOT_MTL)+2) dy=1;
    //dd_pixel(x,y,IRGB(31,15,7));
    dd_copysprite(1024+step,x,y,15,DD_CENTER);
    step++; if (step>9) step=0;
    dd_drawtext_fmt(10,100,IRGB(31,31,31),DD_SHADE|DD_NOCACHE,"x=%d, y=%d",x,y);
}

#endif
