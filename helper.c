/*
 * Part of Astonia Client (c) Daniel Brockhaus. Please read license.txt.
 */
#if 0
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define DD_OFFSET               0       // this has to be zero, so bzero on the structures default this
#define DD_CENTER               1       // also used in dd_drawtext
#define DD_NORMAL               2

#define DDFX_LEFTGRID           1       // NOGRID(?) has to be zero, so bzero on the structures default NOGRID(?)
#define DDFX_RIGHTGRID          2
//#define DLFX_AUTOGRID           3       // not really used by blitting functions, but by dl (you must not call copysprite with this value)

#define DDFX_NLIGHT             15
#define DDFX_BRIGHT             0

#define DDFX_MAX_FREEZE         8

struct ddfx {
    int sprite;             // sprite_fx:           primary sprite number - should be the first entry cause dl_qcmp sorts the by this

    signed char sink;
    unsigned char scale;        // scale in percent
    char cr,cg,cb;          // color balancing
    char clight,sat;        // lightness, saturation
    unsigned short c1,c2,c3,shine;  // color replacer

    char light;             // videocache_fx:       0=bright(DDFX_BRIGHT) 1=almost black; 15=normal (DDFX_NLIGHT)
    char freeze;            // videocache_fx:       0 to DDFX_MAX_FREEZE-1  !!! exclusive DDFX_MAX_FREEZE

    char grid;              // videocache_fx:       0, DDFX_LEFTGRID, DDFX_RIGHTGRID

    char ml,ll,rl,ul,dl;

    char align;             // blitpos_fx:          DDFX_NORMAL, DDFX_OFFSET, DDFX_CENTER
    short int clipsx,clipex; // blitpos_fx:          additional x - clipping around the offset
    short int clipsy,clipey; // blitpos_fx:          additional y - clipping around the offset
};

typedef struct ddfx DDFX;

#define DL_STEP 128

#define DLC_STRIKE      1
#define DLC_NUMBER	2
#define DLC_DUMMY	3       // used to get space in the list to reduce compares ;-)
#define DLC_PIXEL	4
#define DLC_BLESS	5
#define DLC_POTION	6
#define DLC_RAIN	7
#define DLC_PULSE	8
#define DLC_PULSEBACK	9

struct dl {
    int layer;
    int x,y,h;      // scrx=x scry=y-h sorted bye x,y ;) normally used for height, but also misused to place doors right
    // int movy;

    DDFX ddfx;

    // functions to call
    char call;
    int call_x1,call_y1,call_x2,call_y2,call_x3;
};

typedef struct dl DL;

#include "astonia.h"
#include "engine.h"

#define DSIZE   20
static DL *dls[DSIZE];
static int dl_size[DSIZE];
static int dl_tick[DSIZE];
static int dl_in=0,dl_out=0;

void helper_add_dl(int attick,DL **dl,int dlused) {
    int n;

    if (dls[dl_in]) free(dls[dl_in]);

    dls[dl_in]=malloc(sizeof(DL)*dlused);

    for (n=0; n<dlused; n++)
        memcpy(dls[dl_in]+n,dl[n],sizeof(DL));

    dl_size[dl_in]=dlused;
    dl_tick[dl_in]=attick;

    dl_in=(dl_in+1)%DSIZE;
}

int dl_cmp(const void *ca,const void *cb) {
    const DL*a=ca,*b=cb;
    int diff;

    if (a->call==DLC_DUMMY && b->call==DLC_DUMMY) return  0;
    if (a->call==DLC_DUMMY) return -1;
    if (b->call==DLC_DUMMY) return  1;

    diff=a->layer-b->layer;
    if (diff) return diff;

    diff=a->y-b->y;
    if (diff) return diff;

    diff=a->x-b->x;
    if (diff) return diff;

    return a->ddfx.sprite-b->ddfx.sprite;
}

static void msg_skip(DL *d,int i, char *text) {
    printf("  skipping L%d, P%d,%d, S%d (%s)\n",d[i].layer,d[i].x,d[i].y,d[i].ddfx.sprite,text);
}

static void msg_mis(DL *d,int i, DL *e,int j,char *text) {
    printf("  mismatch S: %d vs %d L: %d,%d,%d,%d,%d vs %d,%d,%d,%d,%d (%s) (%d,%d)\n",
           d[i].ddfx.sprite,e[j].ddfx.sprite,
           d[i].ddfx.ml,d[i].ddfx.ll,d[i].ddfx.rl,d[i].ddfx.ul,d[i].ddfx.dl,
           e[j].ddfx.ml,e[j].ddfx.ll,e[j].ddfx.rl,e[j].ddfx.ul,e[j].ddfx.dl,
           text,
           d[i].x,d[i].y);
}

void helper_dl_compare(DL *d1,DL *d2,int d1size,int d2size) {
    int p1,p2,match=0;

    p1=p2=0;
    while (p1<d1size && p2<d2size) {
        if (d1[p1].call!=0) { p1++; continue; }
        if (d2[p2].call!=0) { p2++; continue; }

        if (d1[p1].layer>d2[p2].layer) { msg_skip(d2,p2,"2L"); p2++; continue; }
        if (d1[p1].layer<d2[p2].layer) { msg_skip(d1,p1,"1L"); p1++; continue; }

        if (d1[p1].y>d2[p2].y) { msg_skip(d2,p2,"2Y"); p2++; continue; }
        if (d1[p1].y<d2[p2].y) { msg_skip(d1,p1,"1Y"); p1++; continue; }

        if (d1[p1].x>d2[p2].x) { msg_skip(d2,p2,"2X"); p2++; continue; }
        if (d1[p1].x<d2[p2].x) { msg_skip(d1,p1,"1X"); p1++; continue; }
#if 0
        if (d1[p1].ddfx.sprite>d2[p2].ddfx.sprite) { msg_skip(d2,p2,"2S"); p2++; continue; }
        if (d1[p1].ddfx.sprite<d2[p2].ddfx.sprite) { msg_skip(d1,p1,"1S"); p1++; continue; }
#else
        if (d1[p1].ddfx.sprite!=d2[p2].ddfx.sprite) msg_mis(d1,p1,d2,p2,"Sprite");
        else
#endif
        if (d1[p1].ddfx.ml!=d2[p2].ddfx.ml) msg_mis(d1,p1,d2,p2,"ML");
        else if (d1[p1].ddfx.ll!=d2[p2].ddfx.ll) msg_mis(d1,p1,d2,p2,"LL");
        else if (d1[p1].ddfx.rl!=d2[p2].ddfx.rl) msg_mis(d1,p1,d2,p2,"RL");
        else if (d1[p1].ddfx.ul!=d2[p2].ddfx.ul) msg_mis(d1,p1,d2,p2,"UL");
        else if (d1[p1].ddfx.dl!=d2[p2].ddfx.dl) msg_mis(d1,p1,d2,p2,"DL");

        //printf("Layer: %d %d\n",d1[p1].layer,d2[p2].layer);
        match++;
        p1++;
        p2++;
    }
    //printf("%d matches\n",match);
}

void helper_cmp_dl(int attick,DL **dl,int dlused) {
    int n;
    DL *dltmp;

    while (42) {
        if (dl_in==dl_out) {
            printf("EMPTY ON COMPARE\n"); fflush(stdout);
            return;
        }
        if (dl_tick[dl_out]>=attick && dl_tick[dl_out]<attick+20) break;
        printf("junking tick %d\n",dl_tick[dl_out]);
        dl_out=(dl_out+1)%DSIZE;
    }

    dltmp=malloc(sizeof(DL)*dlused);

    for (n=0; n<dlused; n++)
        memcpy(dltmp+n,dl[n],sizeof(DL));

    qsort(dltmp,dlused,sizeof(DL),dl_cmp);
    qsort(dls[dl_out],dl_size[dl_out],sizeof(DL),dl_cmp);

    printf("Compare tick %d:\n",attick);
    helper_dl_compare(dltmp,dls[dl_out],dlused,dl_size[dl_out]);

    free(dltmp);

    dl_out=(dl_out+1)%DSIZE;
}
#endif
