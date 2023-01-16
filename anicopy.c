/*
 * Part of Astonia Client (c) Daniel Brockhaus. Please read license.txt.
 */

/*

This is a little helper that will rename files from my DAZ3D naming convention
to what the Astonia client uses. This is probably only helpful to me, but adding
it is easier...
*/

#include <stdlib.h>
#include <stdio.h>


/* DAZ3D Animation Track:

TODO: move all 8X animation to the front of the track
so we can stop rendering at an earlier frame for the
extra directions

00      Standing Still (0)      8X
01-08   Walking (8)             8X
09      (reference frame)
10-17   Use / Give (72)         4X
18      (reference frame)
19-26   Take / Drop (104)       4X
27      (reference frame)
28-35   Attack 1 (136)          4X
36      (reference frame)
37-44   Attack 2 (168)          4X
45      (reference frame)
46-53   Attack 3 (200)          4X
54      (reference frame)
55-62   Directed Magic 1 (232)  8X
63      (reference frame)
64-71   Freeze / Warcry (296)   4X
72      (reference frame)
73-80   Die (328)               4X
81      (reference frame)
82-89   Idle (800)              8X
90      (reference frame)


a00    45°
b00     0°
c00   -45°
d00   -90°
e00  -135°
f00   180°
g00   135°
h00    90°

*/


int main(int argc,char *args[]) {
    int n,v,i;

    if (argc!=2) {
        printf("Usage: %s <sprite number>\n",args[0]);
        return 1;
    }

    v=atoi(args[1]);
    if (v<100 || v>250) {
        printf("<sprite number> needs to be between 100 and 250.\n");
        return 1;
    }

    v*=1000;

    // 4X directions first
    for (n=0; n<4; n++) {
        printf("move /y %c00.png %08d.png\n",'a'+n*2,v+n*2); // standing still 8X
        for (i=0; i<8; i++) {   // walking 8X
            printf("move /y %c%02d.png %08d.png\n",'a'+n*2,i+  1,v+n*16+i+  8);
        }
        printf("del %c09.png\n",'a'+n*2);
        for (i=0; i<8; i++) {   // use 4X
            printf("move /y %c%02d.png %08d.png\n",'a'+n*2,i+ 10,v+n*8+i+ 72);
        }
        printf("del %c18.png\n",'a'+n*2);
        for (i=0; i<8; i++) {   // take 4X
            printf("move /y %c%02d.png %08d.png\n",'a'+n*2,i+ 19,v+n*8+i+104);
        }
        printf("del %c27.png\n",'a'+n*2);
        for (i=0; i<8; i++) {   // attack 1 4X
            printf("copy /y %c%02d.png %08d.png\n",'a'+n*2,i+ 28,v+n*8+i+200);
            printf("copy /y %c%02d.png %08d.png\n",'a'+n*2,i+ 28,v+n*8+i+168);
            printf("move /y %c%02d.png %08d.png\n",'a'+n*2,i+ 28,v+n*8+i+136);
        }
        printf("del %c36.png\n",'a'+n*2);
    }

    // 8X directions next
    for (n=0; n<4; n++) {
        printf("move /y %c00.png %08d.png\n",'a'+n*2+1,v+n*2+1); // standing still 8X
        for (i=0; i<8; i++) {   // walking 8X
            printf("move /y %c%02d.png %08d.png\n",'a'+n*2+1,i+  1,v+n*16+8+i+  8);
        }
        printf("del %c09.png\n",'a'+n*2+1);
    }

    return 0;
}

/*

anicopy 220 >../gfx/x4/00220000/_x.bat

*/
