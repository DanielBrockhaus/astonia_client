/*
 * Part of Astonia Client (c) Daniel Brockhaus. Please read license.txt.
 *
 * Display Game Map
 *
 * Displays the actual game map by putting everything that is happening on-screen
 * into an array, then sorting that array by layer and depth and displaying it
 * via the stuff in dd.c.
 */

#include <stdint.h>
#include <stddef.h>
#include <SDL2/SDL.h>

#include "../../src/astonia.h"
#include "../../src/game.h"
#include "../../src/game/_game.h"
#include "../../src/gui.h"
#include "../../src/client.h"

static int fsprite_cnt=0,f2sprite_cnt=0,gsprite_cnt=0,g2sprite_cnt=0,isprite_cnt=0,csprite_cnt=0;
static int qs_time=0,dg_time=0,ds_time=0;
int stom_off_x=0,stom_off_y=0;

static DL *dllist=NULL;
static DL **dlsort=NULL;
static int dlused=0,dlmax=0;
static int stat_dlsortcalls,stat_dlused;
int namesize=DD_SMALL;

DL* dl_next(void) {
    int d;
    ptrdiff_t diff;
    DL *rem;

    if (dlused==dlmax) {
        rem=dllist;
        dllist=xrealloc(dllist,(dlmax+DL_STEP)*sizeof(DL),MEM_DL);
        dlsort=xrealloc(dlsort,(dlmax+DL_STEP)*sizeof(DL *),MEM_DL);
        diff=(unsigned char *)dllist-(unsigned char *)rem;
        for (d=0; d<dlmax; d++) dlsort[d]=(DL *)(((unsigned char *)(dlsort[d]))+diff);
        for (d=dlmax; d<dlmax+DL_STEP; d++) dlsort[d]=&dllist[d];
        dlmax+=DL_STEP;
    } else if (dlused>dlmax) {
        fail("dlused normally shouldn't exceed dlmax - the error is somewhere else ;-)");
        return dlsort[dlused-1];
    }

    dlused++;
    bzero(dlsort[dlused-1],sizeof(DL));

    if (dlused%16==0) {
        dlsort[dlused-1]->call=DLC_DUMMY;
        return dl_next();
    }

    dlsort[dlused-1]->ddfx.sink=0;
    dlsort[dlused-1]->ddfx.scale=100;
    dlsort[dlused-1]->ddfx.cr=dlsort[dlused-1]->ddfx.cg=dlsort[dlused-1]->ddfx.cb=dlsort[dlused-1]->ddfx.clight=dlsort[dlused-1]->ddfx.sat=0;
    dlsort[dlused-1]->ddfx.c1=0;
    dlsort[dlused-1]->ddfx.c2=0;
    dlsort[dlused-1]->ddfx.c3=0;
    dlsort[dlused-1]->ddfx.shine=0;
    return dlsort[dlused-1];
}

DL* dl_next_set(int layer,int sprite,int scrx,int scry,int light) {
    DL *dl;
    DDFX *ddfx;

    if (sprite>MAXSPRITE || sprite<0) {
        note("trying to add illegal sprite %d in dl_next_set",sprite);
        return NULL;
    }

    ddfx=&(dl=dl_next())->ddfx;

    dl->x=scrx;
    dl->y=scry;
    dl->layer=layer;

    ddfx->sprite=sprite;
    ddfx->ml=ddfx->ll=ddfx->rl=ddfx->ul=ddfx->dl=light;
    ddfx->sink=0;
    ddfx->scale=100;
    ddfx->cr=ddfx->cg=ddfx->cb=ddfx->clight=ddfx->sat=0;
    ddfx->c1=0;
    ddfx->c2=0;
    ddfx->c3=0;
    ddfx->shine=0;

    return dl;
}

DL* dl_call_strike(int layer,int x1,int y1,int h1,int x2,int y2,int h2) {
    DL *dl;

    dl=dl_next();

    dl->call=DLC_STRIKE;
    dl->layer=layer;
    dl->call_x1=x1;
    dl->call_y1=y1-h1;
    dl->call_x2=x2;
    dl->call_y2=y2-h2;

    if (y1>y2) {
        dl->x=x1;
        dl->y=y1;
    } else {
        dl->x=x2;
        dl->y=y2;
    }

    return dl;
}

DL* dl_call_pulseback(int layer,int x1,int y1,int h1,int x2,int y2,int h2) {
    DL *dl;

    dl=dl_next();

    dl->call=DLC_PULSEBACK;
    dl->layer=layer;
    dl->call_x1=x1;
    dl->call_y1=y1-h1;
    dl->call_x2=x2;
    dl->call_y2=y2-h2;

    if (y1>y2) {
        dl->x=x1;
        dl->y=y1;
    } else {
        dl->x=x2;
        dl->y=y2;
    }

    return dl;
}

DL* dl_call_bless(int layer,int x,int y,int ticker,int strength,int front) {
    DL *dl;

    dl=dl_next();

    dl->call=DLC_BLESS;
    dl->layer=layer;

    dl->call_x1=x;
    dl->call_y1=y;
    dl->call_x2=ticker;
    dl->call_y2=strength;
    dl->call_x3=front;

    dl->x=x;
    if (front) dl->y=y+8;
    else dl->y=y-8;

    return dl;
}

DL* dl_call_pulse(int layer,int x,int y,int nr,int size,int color) {
    DL *dl;

    dl=dl_next();

    dl->call=DLC_PULSE;
    dl->layer=layer;

    dl->call_x1=x;
    dl->call_y1=y-20;
    dl->call_x2=nr;
    dl->call_y2=size;
    dl->call_x3=color;

    dl->x=x;
    switch (nr) {
        case 0:		dl->x=x+20; dl->y=y+10; break;
        case 1:		dl->x=x+20; dl->y=y-10; break;
        case 2:		dl->x=x-20; dl->y=y-10; break;
        case 3:		dl->x=x-20; dl->y=y+10; break;


    }

    return dl;
}

DL* dl_call_potion(int layer,int x,int y,int ticker,int strength,int front) {
    DL *dl;

    dl=dl_next();

    dl->call=DLC_POTION;
    dl->layer=layer;

    dl->call_x1=x;
    dl->call_y1=y;
    dl->call_x2=ticker;
    dl->call_y2=strength;
    dl->call_x3=front;

    dl->x=x;
    if (front) dl->y=y+8;
    else dl->y=y-8;

    return dl;
}

DL* dl_call_rain(int layer,int x,int y,int nr,int color) {
    DL *dl;
    int sy;

    x+=((nr/30)%30)+15;
    sy=y+((nr/330)%20)+10;
    y=sy-((nr*2)%60)-60;

    dl=dl_next();

    dl->call=DLC_PIXEL;
    dl->layer=layer;

    dl->call_x1=x;
    dl->call_y1=y;
    dl->call_x2=color;

    dl->x=x;
    dl->y=sy;

    return dl;
}

DL* dl_call_rain2(int layer,int x,int y,int ticker,int strength,int front) {
    DL *dl;

    dl=dl_next();

    dl->call=DLC_RAIN;
    dl->layer=layer;

    dl->call_x1=x;
    dl->call_y1=y;
    dl->call_x2=ticker;
    dl->call_y2=strength;
    dl->call_x3=front;

    dl->x=x;
    if (front) dl->y=y+10;
    else dl->y=y-10;

    return dl;
}

DL* dl_call_number(int layer,int x,int y,int nr) {
    DL *dl;

    dl=dl_next();

    dl->call=DLC_NUMBER;
    dl->layer=layer;
    dl->call_x1=x;
    dl->call_y1=y;
    dl->call_x2=nr;

    return dl;
}

int dl_qcmp(const void *ca,const void *cb) {
    DL*a,*b;
    int diff;

    stat_dlsortcalls++;

    a=*(DL **)ca;
    b=*(DL **)cb;

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

void draw_pixel(int x,int y,int color) {
    dd_pixel(x,y,color);
}

void dl_play(void) {
    int d,start;
    void helper_cmp_dl(int attick,DL **dl,int dlused);

    //helper_cmp_dl(tick,dlsort,dlused);

    start=SDL_GetTicks();
    stat_dlsortcalls=0;
    stat_dlused=dlused;
    qsort(dlsort,dlused,sizeof(DL *),dl_qcmp);
    qs_time+=SDL_GetTicks()-start;

    for (d=0; d<dlused && !quit; d++) {
        if (dlsort[d]->call==0) {
            dd_copysprite_fx(&dlsort[d]->ddfx,dlsort[d]->x,dlsort[d]->y-dlsort[d]->h);
        } else {
            switch (dlsort[d]->call) {
                case DLC_STRIKE:
                    dd_display_strike(dlsort[d]->call_x1,dlsort[d]->call_y1,dlsort[d]->call_x2,dlsort[d]->call_y2);
                    break;
                case DLC_NUMBER:
                    dd_drawtext_fmt(dlsort[d]->call_x1,dlsort[d]->call_y1,0xffff,DD_CENTER|DD_SMALL|DD_FRAME,"%d",dlsort[d]->call_x2);
                    break;
                case DLC_DUMMY:
                    break;
                case DLC_PIXEL:
                    draw_pixel(dlsort[d]->call_x1,dlsort[d]->call_y1,dlsort[d]->call_x2);
                    break;
                case DLC_BLESS:
                    dd_draw_bless(dlsort[d]->call_x1,dlsort[d]->call_y1,dlsort[d]->call_x2,dlsort[d]->call_y2,dlsort[d]->call_x3);
                    break;
                case DLC_POTION:
                    dd_draw_potion(dlsort[d]->call_x1,dlsort[d]->call_y1,dlsort[d]->call_x2,dlsort[d]->call_y2,dlsort[d]->call_x3);
                    break;
                case DLC_RAIN:
                    dd_draw_rain(dlsort[d]->call_x1,dlsort[d]->call_y1,dlsort[d]->call_x2,dlsort[d]->call_y2,dlsort[d]->call_x3);
                    break;
                case DLC_PULSE:
                    dd_draw_curve(dlsort[d]->call_x1,dlsort[d]->call_y1,dlsort[d]->call_x2,dlsort[d]->call_y2,dlsort[d]->call_x3);
                    break;
                case DLC_PULSEBACK:
                    dd_display_pulseback(dlsort[d]->call_x1,dlsort[d]->call_y1,dlsort[d]->call_x2,dlsort[d]->call_y2);
                    break;
            }
        }
    }

    dlused=0;
}

void sdl_pre_add(int attick,int sprite,signed char sink,unsigned char freeze,unsigned char scale,char cr,char cg,char cb,char light,char sat,int c1,int c2,int c3,int shine,char ml,char ll,char rl,char ul,char dl);

void dl_prefetch(int attick) {
    void helper_add_dl(int attick,DL **dl,int dlused);
    int d;

    //helper_add_dl(attick,dlsort,dlused);

    for (d=0; d<dlused && !quit; d++) {
        if (dlsort[d]->call==0) {
            sdl_pre_add(attick,
                    dlsort[d]->ddfx.sprite,
                    dlsort[d]->ddfx.sink,
                    dlsort[d]->ddfx.freeze,
                    dlsort[d]->ddfx.scale,
                    dlsort[d]->ddfx.cr,
                    dlsort[d]->ddfx.cg,
                    dlsort[d]->ddfx.cb,
                    dlsort[d]->ddfx.clight,
                    dlsort[d]->ddfx.sat,
                    dlsort[d]->ddfx.c1,
                    dlsort[d]->ddfx.c2,
                    dlsort[d]->ddfx.c3,
                    dlsort[d]->ddfx.shine,
                    dlsort[d]->ddfx.ml,
                    dlsort[d]->ddfx.ll,
                    dlsort[d]->ddfx.rl,
                    dlsort[d]->ddfx.ul,
                    dlsort[d]->ddfx.dl);
        }
    }

    dlused=0;
}

// analyse
QUICK *quick;
int maxquick;

void set_map_lights(struct map *cmap) {
    int i,mn;

    for (i=0; i<maxquick; i++) {

        mn=quick[i].mn[4];

        if (!(cmap[mn].flags&CMF_VISIBLE)) {
            cmap[mn].rlight=0;
            continue;
        }

        cmap[mn].value=0;
        cmap[mn].rlight=(cmap[mn].flags&CMF_LIGHT);

        if (cmap[mn].rlight!=15) {
            cmap[mn].rlight=max(0,cmap[mn].rlight);
            cmap[mn].rlight=min(14,cmap[mn].rlight);
        }
        cmap[mn].mmf=0;

        if (cmap[mn].rlight==15) {
            if (cmap[quick[i].mn[1]].flags&CMF_VISIBLE) cmap[mn].rlight=min((unsigned)cmap[mn].rlight,cmap[quick[i].mn[1]].flags&CMF_LIGHT);
            if (cmap[quick[i].mn[3]].flags&CMF_VISIBLE) cmap[mn].rlight=min((unsigned)cmap[mn].rlight,cmap[quick[i].mn[3]].flags&CMF_LIGHT);
            if (cmap[quick[i].mn[5]].flags&CMF_VISIBLE) cmap[mn].rlight=min((unsigned)cmap[mn].rlight,cmap[quick[i].mn[5]].flags&CMF_LIGHT);
            if (cmap[quick[i].mn[7]].flags&CMF_VISIBLE) cmap[mn].rlight=min((unsigned)cmap[mn].rlight,cmap[quick[i].mn[7]].flags&CMF_LIGHT);

            if (cmap[mn].rlight==15) {
                if (cmap[quick[i].mn[0]].flags&CMF_VISIBLE) cmap[mn].rlight=min((unsigned)cmap[mn].rlight,cmap[quick[i].mn[0]].flags&CMF_LIGHT);
                if (cmap[quick[i].mn[2]].flags&CMF_VISIBLE) cmap[mn].rlight=min((unsigned)cmap[mn].rlight,cmap[quick[i].mn[2]].flags&CMF_LIGHT);
                if (cmap[quick[i].mn[6]].flags&CMF_VISIBLE) cmap[mn].rlight=min((unsigned)cmap[mn].rlight,cmap[quick[i].mn[6]].flags&CMF_LIGHT);
                if (cmap[quick[i].mn[8]].flags&CMF_VISIBLE) cmap[mn].rlight=min((unsigned)cmap[mn].rlight,cmap[quick[i].mn[8]].flags&CMF_LIGHT);

                if (cmap[mn].rlight==15) {
                    cmap[mn].rlight=0;
                    continue;
                }
            }

            cmap[mn].mmf|=MMF_SIGHTBLOCK;
        }

        cmap[mn].rlight=15-cmap[mn].rlight;

        if (game_options&GO_LOWLIGHT) {
            switch (cmap[mn].rlight) {
                case 2: cmap[mn].rlight=1; break;
                case 3:
                case 4: cmap[mn].rlight=5; break;
                case 6:
                case 7:
                case 8: cmap[mn].rlight=9; break;
                case 10:
                case 11: cmap[mn].rlight=12; break;
                case 13: cmap[mn].rlight=14; break;
            }
        }
    }
}

void sprites_colorbalance(struct map *cmap,int mn,int r,int g,int b) {
    cmap[mn].rf.cr=min(120,cmap[mn].rf.cr+r);
    cmap[mn].rf.cg=min(120,cmap[mn].rf.cg+g);
    cmap[mn].rf.cb=min(120,cmap[mn].rf.cb+b);

    cmap[mn].rf2.cr=min(120,cmap[mn].rf2.cr+r);
    cmap[mn].rf2.cg=min(120,cmap[mn].rf2.cg+g);
    cmap[mn].rf2.cb=min(120,cmap[mn].rf2.cb+b);

    cmap[mn].rg.cr=min(120,cmap[mn].rg.cr+r);
    cmap[mn].rg.cg=min(120,cmap[mn].rg.cg+g);
    cmap[mn].rg.cb=min(120,cmap[mn].rg.cb+b);

    cmap[mn].rg2.cr=min(120,cmap[mn].rg2.cr+r);
    cmap[mn].rg2.cg=min(120,cmap[mn].rg2.cg+g);
    cmap[mn].rg2.cb=min(120,cmap[mn].rg2.cb+b);

    cmap[mn].ri.cr=min(120,cmap[mn].ri.cr+r);
    cmap[mn].ri.cg=min(120,cmap[mn].ri.cg+g);
    cmap[mn].ri.cb=min(120,cmap[mn].ri.cb+b);

    cmap[mn].rc.cr=min(120,cmap[mn].rc.cr+r);
    cmap[mn].rc.cg=min(120,cmap[mn].rc.cg+g);
    cmap[mn].rc.cb=min(120,cmap[mn].rc.cb+b);
}

#define RANDOM(a)	(rand()%(a))
#define MAXBUB		100
struct bubble {
    int type;
    int origx,origy;
    int cx,cy;
    int state;
};

struct bubble bubble[MAXBUB];


void add_bubble(int x,int y,int h) {
    int n;
    int offx,offy;

    mtos(originx,originy,&offx,&offy);
    offx-=mapaddx*2; offy-=mapaddx*2;

    for (n=0; n<MAXBUB; n++) {
        if (!bubble[n].state) {
            bubble[n].state=1;
            bubble[n].origx=x+offx;
            bubble[n].origy=y+offy;
            bubble[n].cx=x+offx;
            bubble[n].cy=y-h+offy;
            bubble[n].type=RANDOM(3);
            //addline("added bubble at %d,%d",offx,offy);
            return;
        }
    }
}

void show_bubbles(void) {
    int n,spr,offx,offy;
    DL *dl;
    //static int oo=0;

    mtos(originx,originy,&offx,&offy);
    offx-=mapaddx*2; offy-=mapaddy*2;
    //if (oo!=mapaddx) addline("shown bubble at %d,%d %d,%d",offx,offy,oo=mapaddx,mapaddy);

    for (n=0; n<MAXBUB; n++) {
        if (!bubble[n].state) continue;

        spr=(bubble[n].state-1)%6;
        if (spr>3) spr=3-(spr-3);
        spr+=bubble[n].type*3;

        dl=dl_next_set(GME_LAY,1140+spr,bubble[n].cx-offx,bubble[n].origy-offy,DDFX_NLIGHT);
        dl->h=bubble[n].origy-bubble[n].cy;
        bubble[n].state++;
        bubble[n].cx+=2-RANDOM(5);
        bubble[n].cy-=1+RANDOM(3);
        if (bubble[n].cy<1) bubble[n].state=0;
        if (bubble[n].state>50) bubble[n].state=0;
    }

}

void set_map_sprites(struct map *cmap,int attick) {
    int i,mn;

    for (i=0; i<maxquick; i++) {

        mn=quick[i].mn[4];

        if (!cmap[mn].rlight) continue;

        if (cmap[mn].gsprite) cmap[mn].rg.sprite=trans_asprite(mn,cmap[mn].gsprite,attick,&cmap[mn].rg.scale,&cmap[mn].rg.cr,&cmap[mn].rg.cg,&cmap[mn].rg.cb,&cmap[mn].rg.light,&cmap[mn].rg.sat,&cmap[mn].rg.c1,&cmap[mn].rg.c2,&cmap[mn].rg.c3,&cmap[mn].rg.shine);
        else cmap[mn].rg.sprite=0;
        if (cmap[mn].fsprite) cmap[mn].rf.sprite=trans_asprite(mn,cmap[mn].fsprite,attick,&cmap[mn].rf.scale,&cmap[mn].rf.cr,&cmap[mn].rf.cg,&cmap[mn].rf.cb,&cmap[mn].rf.light,&cmap[mn].rf.sat,&cmap[mn].rf.c1,&cmap[mn].rf.c2,&cmap[mn].rf.c3,&cmap[mn].rf.shine);
        else cmap[mn].rf.sprite=0;
        if (cmap[mn].gsprite2) cmap[mn].rg2.sprite=trans_asprite(mn,cmap[mn].gsprite2,attick,&cmap[mn].rg2.scale,&cmap[mn].rg2.cr,&cmap[mn].rg2.cg,&cmap[mn].rg2.cb,&cmap[mn].rg2.light,&cmap[mn].rg2.sat,&cmap[mn].rg2.c1,&cmap[mn].rg2.c2,&cmap[mn].rg2.c3,&cmap[mn].rg2.shine);
        else cmap[mn].rg2.sprite=0;
        if (cmap[mn].fsprite2) cmap[mn].rf2.sprite=trans_asprite(mn,cmap[mn].fsprite2,attick,&cmap[mn].rf2.scale,&cmap[mn].rf2.cr,&cmap[mn].rf2.cg,&cmap[mn].rf2.cb,&cmap[mn].rf2.light,&cmap[mn].rf2.sat,&cmap[mn].rf2.c1,&cmap[mn].rf2.c2,&cmap[mn].rf2.c3,&cmap[mn].rf2.shine);
        else cmap[mn].rf2.sprite=0;

        if (cmap[mn].isprite) {
            cmap[mn].ri.sprite=trans_asprite(mn,cmap[mn].isprite,attick,&cmap[mn].ri.scale,&cmap[mn].ri.cr,&cmap[mn].ri.cg,&cmap[mn].ri.cb,&cmap[mn].ri.light,&cmap[mn].ri.sat,&cmap[mn].ri.c1,&cmap[mn].ri.c2,&cmap[mn].ri.c3,&cmap[mn].ri.shine);
            if (cmap[mn].ic1 || cmap[mn].ic2 || cmap[mn].ic3) {
                cmap[mn].ri.c1=cmap[mn].ic1;
                cmap[mn].ri.c2=cmap[mn].ic2;
                cmap[mn].ri.c3=cmap[mn].ic3;
            }

            if (is_door_sprite(cmap[mn].ri.sprite)) cmap[mn].mmf|=MMF_DOOR;
        } else cmap[mn].ri.sprite=0;
        if (cmap[mn].csprite) trans_csprite(mn,cmap,attick);
    }
}

static void set_map_cut(struct map *cmap) {
    int i,mn,mn2,i2;
    unsigned int tmp;

    if (nocut) return;

    // change sprites
    for (i=0; i<maxquick; i++) {
        mn=quick[i].mn[0];
        i2=quick[i].qi[0];
        if (mn) mn2=quick[i2].mn[0];
        else mn2=0;

        if ((!mn || !cmap[mn].rlight ||
             ((unsigned)abs(is_cut_sprite(cmap[mn].rf.sprite))!=cmap[mn].rf.sprite && is_cut_sprite(cmap[mn].rf.sprite)>0) ||
             ((unsigned)abs(is_cut_sprite(cmap[mn].rf2.sprite))!=cmap[mn].rf2.sprite && is_cut_sprite(cmap[mn].rf2.sprite)>0) ||
             ((unsigned)abs(is_cut_sprite(cmap[mn].ri.sprite))!=cmap[mn].ri.sprite  && is_cut_sprite(cmap[mn].ri.sprite)>0)) &&
            (!mn2 || !cmap[mn2].rlight ||
             ((unsigned)abs(is_cut_sprite(cmap[mn2].rf.sprite))!=cmap[mn2].rf.sprite && is_cut_sprite(cmap[mn2].rf.sprite)>0) ||
             ((unsigned)abs(is_cut_sprite(cmap[mn2].rf2.sprite))!=cmap[mn2].rf2.sprite && is_cut_sprite(cmap[mn2].rf2.sprite)>0) ||
             ((unsigned)abs(is_cut_sprite(cmap[mn2].ri.sprite))!=cmap[mn2].ri.sprite && is_cut_sprite(cmap[mn2].ri.sprite)>0) )) continue;


        cmap[quick[i].mn[4]].mmf|=MMF_CUT;
    }
    for (i=0; i<maxquick; i++) {
        if (!(cmap[quick[i].mn[4]].mmf&MMF_CUT)) continue;

        if (is_cut_sprite(cmap[quick[i].mn[4]].rf.sprite)<0 &&
            ((!(cmap[quick[i].mn[1]].mmf&MMF_CUT) && is_cut_sprite(cmap[quick[i].mn[1]].rf.sprite)) ||
             (!(cmap[quick[i].mn[3]].mmf&MMF_CUT) && is_cut_sprite(cmap[quick[i].mn[3]].rf.sprite)))) continue;

        tmp=abs(is_cut_sprite(cmap[quick[i].mn[4]].rf.sprite));
        if (tmp!=cmap[quick[i].mn[4]].rf.sprite) cmap[quick[i].mn[4]].rf.sprite=tmp;

        tmp=abs(is_cut_sprite(cmap[quick[i].mn[4]].rf2.sprite));
        if (tmp!=cmap[quick[i].mn[4]].rf2.sprite) cmap[quick[i].mn[4]].rf2.sprite=tmp;

        tmp=abs(is_cut_sprite(cmap[quick[i].mn[4]].ri.sprite));
        if (tmp!=cmap[quick[i].mn[4]].ri.sprite) cmap[quick[i].mn[4]].ri.sprite=tmp;
    }
}

void set_map_straight(struct map *cmap) {
    int i,mn,mna,vl,vr,vt,vb,wl,wr,wt,wb;

    for (i=0; i<maxquick; i++) {

        mn=quick[i].mn[4];

        if (!cmap[mn].rlight) continue;

        if ((mna=quick[i].mn[3])!=0) { vl=cmap[mna].rlight; wl=cmap[mna].mmf&MMF_SIGHTBLOCK; } else vl=wl=0;
        if ((mna=quick[i].mn[5])!=0) { vr=cmap[mna].rlight; wr=cmap[mna].mmf&MMF_SIGHTBLOCK; } else vr=wr=0;
        if ((mna=quick[i].mn[1])!=0) { vt=cmap[mna].rlight; wt=cmap[mna].mmf&MMF_SIGHTBLOCK; } else vt=wt=0;
        if ((mna=quick[i].mn[7])!=0) { vb=cmap[mna].rlight; wb=cmap[mna].mmf&MMF_SIGHTBLOCK; } else vb=wb=0;

        if (!(cmap[mn].mmf&MMF_SIGHTBLOCK)) {
            if ((!vl || wl) && (!vb || wb) &&   vt        &&   vr        && (!wl || !wb)) cmap[mn].mmf|=MMF_STRAIGHT_L;
            if (vl        &&   vb        && (!vt || wt) && (!vr || wr) && (!wt || !wr)) cmap[mn].mmf|=MMF_STRAIGHT_R;
            if ((!vl || wl) &&   vb        && (!vt || wt) &&   vr        && (!wl || !wt)) cmap[mn].mmf|=MMF_STRAIGHT_T;
            if (vl        && (!vb || wb) &&   vt        && (!vr || wr) && (!wb || !wr)) cmap[mn].mmf|=MMF_STRAIGHT_B;
        } else {
            if (!vt && !vr && !(wl && wb)) cmap[mn].mmf|=MMF_STRAIGHT_R;
            if (!vb && !vl && !(wr && wt)) cmap[mn].mmf|=MMF_STRAIGHT_L;
        }

    }

}

void set_map_values(struct map *cmap,int attick) {
    set_map_lights(cmap);
    set_map_sprites(cmap,attick);
    set_map_cut(cmap);
    set_map_straight(cmap);
}

static int trans_x(int frx,int fry,int tox,int toy,int step,int start) {
    int x,y,dx,dy;

    dx=(tox-frx);
    dy=(toy-fry);

    if (abs(dx)>abs(dy)) { dy=dy*step/abs(dx); dx=dx*step/abs(dx); } else { dx=dx*step/abs(dy); dy=dy*step/abs(dy); }

    x=frx*1024+512;
    y=fry*1024+512;

    x+=dx*(tick-start);
    y+=dy*(tick-start);

    x-=(originx-DIST)*1024;
    y-=(originy-DIST)*1024;

    return (x-y)*20/1024+mapoffx+mapaddx;
}

static int trans_y(int frx,int fry,int tox,int toy,int step,int start) {
    int x,y,dx,dy;

    dx=(tox-frx);
    dy=(toy-fry);

    if (abs(dx)>abs(dy)) { dy=dy*step/abs(dx); dx=dx*step/abs(dx); } else { dx=dx*step/abs(dy); dy=dy*step/abs(dy); }

    x=frx*1024+512;
    y=fry*1024+512;

    x+=dx*(tick-start);
    y+=dy*(tick-start);

    x-=(originx-DIST)*1024;
    y-=(originy-DIST)*1024;

    return (x+y)*10/1024+mapoffy+mapaddy /*MR*/-FDY/2;
}

static void display_game_spells(void) {
    int i,mn,scrx,scry,x,y,dx,sprite,start;
    int nr,fn,e;
    int mapx,mapy,mna,x1,y1,x2,y2,h1,h2,size,n;
    DL *dl;
    int light;
    float alpha;

    start=SDL_GetTicks();

    for (i=0; i<maxquick; i++) {

        mn=quick[i].mn[4];
        scrx=mapaddx+quick[i].cx;
        scry=mapaddy+quick[i].cy;
        light=map[mn].rlight;

        if (!light) continue;

        map[mn].sink=0;

        if (map[mn].gsprite>=59405 && map[mn].gsprite<=59413) map[mn].sink=8;
        if (map[mn].gsprite>=59414 && map[mn].gsprite<=59422) map[mn].sink=16;
        if (map[mn].gsprite>=59423 && map[mn].gsprite<=59431) map[mn].sink=24;
        if (map[mn].gsprite>=20815 && map[mn].gsprite<=20823) map[mn].sink=36;

        for (e=0; e<68; e++) {

            if (e<4) {
                if ((fn=map[mn].ef[e])!=0) nr=find_ceffect(fn);
                else continue;
            } else if (map[mn].cn) {
                for (nr=e-4; nr<MAXEF; nr++) {
                    if (ueffect[nr] && is_char_ceffect(ceffect[nr].generic.type) && (unsigned)ceffect[nr].flash.cn==map[mn].cn) break;
                }
                if (nr==MAXEF) break;
                else e=nr+4;
            } else break;;

            if (nr!=-1) {
                //addline("%d %d %d %d %d",fn,e,nr,ceffect[nr].generic.type,map[mn].cn);
                //if (e>3) addline("%d: effect %d at %d",tick,ceffect[nr].generic.type,nr);
                switch (ceffect[nr].generic.type) {

                    case 1: // shield
                        if (tick-ceffect[nr].shield.start<3) {
                            dl=dl_next_set(GME_LAY,1002+tick-ceffect[nr].shield.start,scrx+map[mn].xadd,scry+map[mn].yadd+1,DDFX_NLIGHT);
                            if (!dl) { note("error in shield #1"); break; }
                        }
                        break;

                    case 5: // flash
                        x=scrx+map[mn].xadd+cos(2*M_PI*(now%1000)/1000.0)*16;
                        y=scry+map[mn].yadd+sin(2*M_PI*(now%1000)/1000.0)*8;
                        dl=dl_next_set(GME_LAY,1006,x,y,DDFX_NLIGHT); // shade
                        if (!dl) { note("error in flash #1"); break; }
                        dl=dl_next_set(GME_LAY,1005,x,y,DDFX_NLIGHT); // small lightningball
                        if (!dl) { note("error in flash #2"); break; }
                        dl->h=50;
                        break;

                    case 3: // strike
                            // set source coords - mna is source
                        mapx=ceffect[nr].strike.x-originx+DIST;
                        mapy=ceffect[nr].strike.y-originy+DIST;
                        mna=mapmn(mapx,mapy);
                        mtos(mapx,mapy,&x1,&y1);

                        if (map[mna].cn==0) { // no char, so source should be a lightning ball
                            h1=20;
                        } else {  // so i guess we spell from a char (use the flying ball as source)
                            x1=x1+map[mna].xadd+cos(2*M_PI*(now%1000)/1000.0)*16;
                            y1=y1+map[mna].yadd+sin(2*M_PI*(now%1000)/1000.0)*8;
                            h1=50;
                        }

                        // set target coords - mn is target
                        x2=scrx+map[mn].xadd;
                        y2=scry+map[mn].yadd;
                        h2=25;

                        // sanity check
                        if (abs(x1-x2)+abs(y1-y2)>200) break;

                        // mn is target
                        dl_call_strike(GME_LAY,x1,y1,h1,x2,y2,h2);
                        //addline("strike %d,%d to %d,%d",x1,y1,x2,y2);
                        break;

                    case 7: // explosion
                        if (tick-ceffect[nr].explode.start<8) {
                            x=scrx;
                            y=scry;

                            if (ceffect[nr].explode.base>=50450 && ceffect[nr].explode.base<=50454) {
                                dx=15;
                                sprite=50450;
                            } else {
                                dx=15;
                                sprite=ceffect[nr].explode.base;
                            }

                            dl=dl_next_set(GME_LAY2,min(sprite+tick-ceffect[nr].explode.start,sprite+7),x,y-dx,DDFX_NLIGHT);

                            if (!dl) { note("error in explosion #1"); break; }
                            dl->h=dx;
                            if (ceffect[nr].explode.base<50450 || ceffect[nr].explode.base>50454) {
                                if (map[mn].flags&CMF_UNDERWATER) { dl->ddfx.cb=min(120,dl->ddfx.cb+80); dl->ddfx.sat=min(20,dl->ddfx.sat+10); }
                                break;
                            }
                            if (ceffect[nr].explode.base==50451) dl->ddfx.c1=IRGB(16,12,0);

                            dl=dl_next_set(GME_LAY2,min(sprite+8+tick-ceffect[nr].explode.start,sprite+15),x,y+dx,DDFX_NLIGHT);
                            if (!dl) { note("error in explosion #2"); break; }
                            dl->h=dx;
                            if (ceffect[nr].explode.base==50451) dl->ddfx.c1=IRGB(16,12,0);
                        }

                        break;

                    case 8: // warcry
                        alpha=-2*M_PI*(now%1000)/1000.0;

                        for (x1=0; x1<4; x1++) {
                            x=scrx+map[mn].xadd+cos(alpha+x1*M_PI/2)*15;
                            y=scry+map[mn].yadd+sin(alpha+x1*M_PI/2)*15/2;
                            dl=dl_next_set(GME_LAY,1020+(tick/4+x1)%4,x,y,DDFX_NLIGHT);
                            if (!dl) { note("error in warcry #1"); break; }
                            dl->h=40;
                        }


                        break;
                    case 9: // bless
                        dl_call_bless(GME_LAY,scrx+map[mn].xadd,scry+map[mn].yadd,ceffect[nr].bless.stop-tick,ceffect[nr].bless.strength,1);
                        dl_call_bless(GME_LAY,scrx+map[mn].xadd,scry+map[mn].yadd,ceffect[nr].bless.stop-tick,ceffect[nr].bless.strength,0);
                        break;

                    case 10: // heal
                        dl=dl_next_set(GME_LAY,50114,scrx+map[mn].xadd,scry+map[mn].yadd+1,DDFX_NLIGHT);
                        if (!dl) { note("error in heal #1"); break; }
                        break;

                    case 12: // burn //
                        x=scrx+map[mn].xadd;
                        y=scry+map[mn].yadd-3;
                        dl=dl_next_set(GME_LAY,1024+((tick)%10),x,y,DDFX_NLIGHT); // burn behind
                        if (!dl) { note("error in bun #1"); break; }
                        if (map[mn].flags&CMF_UNDERWATER) { dl->ddfx.cb=min(120,dl->ddfx.cb+80); dl->ddfx.sat=min(20,dl->ddfx.sat+10); }

                        x=scrx+map[mn].xadd;
                        y=scry+map[mn].yadd+3;
                        dl=dl_next_set(GME_LAY,1024+((5+tick)%10),x,y,DDFX_NLIGHT); // small lightningball
                        if (!dl) { note("error in burn #2"); break; }
                        if (map[mn].flags&CMF_UNDERWATER) { dl->ddfx.cb=min(120,dl->ddfx.cb+80); dl->ddfx.sat=min(20,dl->ddfx.sat+10); }

                        break;
                    case 13: // mist
                        if (tick-ceffect[nr].mist.start<24) {
                            x=scrx;
                            y=scry;
                            dl=dl_next_set(GME_LAY+1,1034+(tick-ceffect[nr].mist.start),x,y,DDFX_NLIGHT);
                            if (!dl) { note("error in mist #1"); break; }
                        }
                        break;

                    case 14:    // potion
                        dl_call_potion(GME_LAY,scrx+map[mn].xadd,scry+map[mn].yadd,ceffect[nr].potion.stop-tick,ceffect[nr].potion.strength,1);
                        dl_call_potion(GME_LAY,scrx+map[mn].xadd,scry+map[mn].yadd,ceffect[nr].potion.stop-tick,ceffect[nr].potion.strength,0);
                        break;

                    case 15:    // earth-rain
                        dl_call_rain2(GME_LAY,scrx,scry,tick,ceffect[nr].earthrain.strength,1);
                        dl_call_rain2(GME_LAY,scrx,scry,tick,ceffect[nr].earthrain.strength,0);
                        break;
                    case 16:    // earth-mud
                        mapx=mn%MAPDX+originx-MAPDX/2;
                        mapy=mn/MAPDX+originy-MAPDY/2;
                        dl=dl_next_set(GME_LAY-1,50254+(mapx%3)+((mapy/3)%3),scrx,scry,light);
                        if (!dl) { note("error in mud #1"); break; }
                        map[mn].sink=12;
                        break;
                    case 21:    // pulse
                        size=((tick-ceffect[nr].pulse.start)%6)*4+10;
                        for (n=0; n<4; n++) {
                            dl_call_pulse(GME_LAY,scrx,scry-3,n,size+1,IRGB(0,12,0));
                            dl_call_pulse(GME_LAY,scrx,scry-2,n,size-2,IRGB(0,16,0));
                            dl_call_pulse(GME_LAY,scrx,scry-1,n,size-1,IRGB(0,20,0));
                            dl_call_pulse(GME_LAY,scrx,scry,n,size,IRGB(16,31,16));
                        }
                        break;
                    case 22:    // pulseback
                                // set source coords - mna is source
                        mapx=ceffect[nr].pulseback.x-originx+DIST;
                        mapy=ceffect[nr].pulseback.y-originy+DIST;
                        mna=mapmn(mapx,mapy);
                        mtos(mapx,mapy,&x1,&y1);

                        if (map[mna].cn==0) { // no char, so source should be a lightning ball
                            h1=20;
                        } else {  // so i guess we spell from a char (use the flying ball as source)
                            h1=50;
                        }

                        // set target coords - mn is target
                        x2=scrx+map[mn].xadd;
                        y2=scry+map[mn].yadd;
                        h2=25;

                        // sanity check
                        if (abs(x1-x2)+abs(y1-y2)>200) break;

                        // mn is target
                        dl_call_pulseback(GME_LAY,x1,y1,h1,x2,y2,h2);
                        //addline("strike %d,%d to %d,%d",x1,y1,x2,y2);
                        break;
                    case 23:    // fire ringlet
                        if (tick-ceffect[nr].firering.start<7) {
                            dl=dl_next_set(GME_LAY,51601+(tick-ceffect[nr].firering.start)*2,scrx,scry+20,DDFX_NLIGHT);
                            dl->h=40;
                            if (map[mn].flags&CMF_UNDERWATER) { dl->ddfx.cb=min(120,dl->ddfx.cb+80); dl->ddfx.sat=min(20,dl->ddfx.sat+10); }
                            dl=dl_next_set(GME_LAY,51600+(tick-ceffect[nr].firering.start)*2,scrx,scry,DDFX_NLIGHT);
                            dl->h=20;
                            if (map[mn].flags&CMF_UNDERWATER) { dl->ddfx.cb=min(120,dl->ddfx.cb+80); dl->ddfx.sat=min(20,dl->ddfx.sat+10); }
                        }
                        break;
                    case 24:    // forever blowing bubbles...
                        if (ceffect[nr].bubble.yoff) add_bubble(scrx+map[mn].xadd,scry+map[mn].yadd,ceffect[nr].bubble.yoff);
                        else add_bubble(scrx,scry,ceffect[nr].bubble.yoff);
                        break;
                }
            }
        }
    }

    ds_time=SDL_GetTicks()-start;
}

static void display_game_spells2(void) {
    int x,y,nr,mapx,mapy,mn;
    DL *dl;

    for (nr=0; nr<MAXEF; nr++) {
        if (!ueffect[nr]) continue;

        switch (ceffect[nr].generic.type) {
            case 2: // ball
                x=trans_x(ceffect[nr].ball.frx,ceffect[nr].ball.fry,ceffect[nr].ball.tox,ceffect[nr].ball.toy,128,ceffect[nr].ball.start);
                y=trans_y(ceffect[nr].ball.frx,ceffect[nr].ball.fry,ceffect[nr].ball.tox,ceffect[nr].ball.toy,128,ceffect[nr].ball.start);

                stom(x,y,&mapx,&mapy);
                mn=mapmn(mapx,mapy);
                if (!map[mn].rlight) break;

                dl=dl_next_set(GME_LAY,1008,x,y,DDFX_NLIGHT);      // shade
                if (!dl) { note("error in ball #1"); break; }
                dl=dl_next_set(GME_LAY,1000,x,y,DDFX_NLIGHT);   // lightningball
                if (!dl) { note("error in ball #2"); break; }
                dl->h=20;
                break;
            case 4: // fireball
                x=trans_x(ceffect[nr].fireball.frx,ceffect[nr].fireball.fry,ceffect[nr].fireball.tox,ceffect[nr].fireball.toy,1024,ceffect[nr].fireball.start);
                y=trans_y(ceffect[nr].fireball.frx,ceffect[nr].fireball.fry,ceffect[nr].fireball.tox,ceffect[nr].fireball.toy,1024,ceffect[nr].fireball.start);

                stom(x,y,&mapx,&mapy);
                mn=mapmn(mapx,mapy);
                if (!map[mn].rlight) break;

                dl=dl_next_set(GME_LAY,1007,x,y,DDFX_NLIGHT);      // shade
                if (!dl) { note("error in fireball #1"); break; }
                if (map[mn].flags&CMF_UNDERWATER) { dl->ddfx.cb=min(120,dl->ddfx.cb+80); dl->ddfx.sat=min(20,dl->ddfx.sat+10); }
                dl=dl_next_set(GME_LAY,1001,x,y,DDFX_NLIGHT);   // fireball
                if (!dl) { note("error in fireball #2"); break; }
                if (map[mn].flags&CMF_UNDERWATER) { dl->ddfx.cb=min(120,dl->ddfx.cb+80); dl->ddfx.sat=min(20,dl->ddfx.sat+10); }
                dl->h=20;
                break;
            case 17:    // edemonball
                x=trans_x(ceffect[nr].edemonball.frx,ceffect[nr].edemonball.fry,ceffect[nr].edemonball.tox,ceffect[nr].edemonball.toy,256,ceffect[nr].edemonball.start);
                y=trans_y(ceffect[nr].edemonball.frx,ceffect[nr].edemonball.fry,ceffect[nr].edemonball.tox,ceffect[nr].edemonball.toy,256,ceffect[nr].edemonball.start);

                stom(x,y,&mapx,&mapy);
                mn=mapmn(mapx,mapy);
                if (!map[mn].rlight) break;

                dl=dl_next_set(GME_LAY,50281,x,y,DDFX_NLIGHT);      // shade
                if (!dl) { note("error in edemonball #1"); break; }
                dl=dl_next_set(GME_LAY,50264,x,y,DDFX_NLIGHT);   // edemonball
                if (!dl) { note("error in edemonball #2"); break; }
                dl->h=10;

                if (ceffect[nr].edemonball.base==1) dl->ddfx.c1=IRGB(16,12,0);
                //else if (ceffect[nr].edemonball.base==2) dl->ddfx.tint=EDEMONBALL_TINT3;
                //else if (ceffect[nr].edemonball.base==3) dl->ddfx.tint=EDEMONBALL_TINT4;

                break;
        }
    }
}

static char* roman(int nr) {
    int h,t,o;
    static char buf[80];
    char *ptr=buf;

    if (nr>399) return "???";

    h=nr/100;
    nr-=h*100;
    t=nr/10;
    nr-=t*10;
    o=nr;

    while (h) {*ptr++='C'; h--; }

    if (t==9) { *ptr++='X'; *ptr++='C'; t=0; }
    if (t>4) { *ptr++='L'; t-=5; }
    if (t==4) { *ptr++='X'; *ptr++='L'; t=0; }
    while (t) {*ptr++='X'; t--; }

    if (o==9) { *ptr++='I'; *ptr++='X'; o=0; }
    if (o>4) { *ptr++='V'; o-=5; }
    if (o==4) { *ptr++='I'; *ptr++='V'; o=0; }
    while (o) {*ptr++='I'; o--; }

    *ptr=0;

    return buf;
}

static void display_game_names(void) {
    int i,mn,scrx,scry,x,y,col,frame;
    char *sign;
    unsigned short clancolor[33];

    clancolor[1]=IRGB(31,0,0);
    clancolor[2]=IRGB(0,31,0);
    clancolor[3]=IRGB(0,0,31);
    clancolor[4]=IRGB(31,31,0);
    clancolor[5]=IRGB(31,0,31);
    clancolor[6]=IRGB(0,31,31);
    clancolor[7]=IRGB(31,16,16);
    clancolor[8]=IRGB(16,16,31);

    clancolor[9]=IRGB(24,8,8);
    clancolor[10]=IRGB(8,24,8);
    clancolor[11]=IRGB(8,8,24);
    clancolor[12]=IRGB(24,24,8);
    clancolor[13]=IRGB(24,8,24);
    clancolor[14]=IRGB(8,24,24);
    clancolor[15]=IRGB(24,24,24);
    clancolor[16]=IRGB(16,16,16);

    clancolor[17]=IRGB(31,24,24);
    clancolor[18]=IRGB(24,31,24);
    clancolor[19]=IRGB(24,24,31);
    clancolor[20]=IRGB(31,31,24);
    clancolor[21]=IRGB(31,24,31);
    clancolor[22]=IRGB(24,31,31);
    clancolor[23]=IRGB(31,8,8);
    clancolor[24]=IRGB(8,8,31);

    clancolor[25]=IRGB(16,8,8);
    clancolor[26]=IRGB(8,16,8);
    clancolor[27]=IRGB(8,8,16);
    clancolor[28]=IRGB(16,16,8);
    clancolor[29]=IRGB(16,8,16);
    clancolor[30]=IRGB(8,16,16);
    clancolor[31]=IRGB(8,31,8);
    clancolor[32]=IRGB(31,8,31);

    for (i=0; i<maxquick; i++) {

        mn=quick[i].mn[4];
        scrx=mapaddx+quick[i].cx;
        scry=mapaddy+quick[i].cy;

        if (!map[mn].rlight) continue;
        if (!map[mn].csprite) continue;
        if (map[mn].gsprite==51066) continue;
        if (map[mn].gsprite==51067) continue;

        x=scrx+map[mn].xadd;
        y=scry+4+map[mn].yadd+get_chr_height(map[mn].csprite)-25+get_sink(mn,map);

        col=whitecolor;
        frame=DD_FRAME;

        if (player[map[mn].cn].clan) {
            col=clancolor[player[map[mn].cn].clan];
            if (player[map[mn].cn].clan==3) frame=DD_WFRAME;
        }

        sign="";
        if (player[map[mn].cn].pk_status==5) sign=" **";
        else if (player[map[mn].cn].pk_status==4) sign=" *";
        else if (player[map[mn].cn].pk_status==3) sign=" ++";
        else if (player[map[mn].cn].pk_status==2) sign=" +";
        else if (player[map[mn].cn].pk_status==1) sign=" -";

        if (namesize!=DD_SMALL) y-=3;
        dd_drawtext_fmt(x,y,col,DD_CENTER|namesize|frame,"%s%s",player[map[mn].cn].name,sign);


        if (namesize!=DD_SMALL) y+=3;
        y+=12;
        dd_drawtext(x,y,whitecolor,DD_CENTER|DD_SMALL|DD_FRAME,roman(player[map[mn].cn].level));


        x-=12;
        y-=6;
        if (map[mn].health>1) {
            dd_rect(x,y,x+25,y+1,blackcolor);
            dd_rect(x,y,x+map[mn].health/4,y+1,healthcolor);
            y++;
        }
        if (map[mn].shield>1) {
            dd_rect(x,y,x+25,y+1,blackcolor);
            dd_rect(x,y,x+map[mn].shield/4,y+1,shieldcolor);
            y++;
        }
        if (map[mn].mana>1) {
            dd_rect(x,y,x+25,y+1,blackcolor);
            dd_rect(x,y,x+map[mn].mana/4,y+1,manacolor);
        }
    }
}

static void display_game_act(void) {
    int mn,scrx,scry,mapx,mapy;
    char *actstr;
    int acttyp;

    // act
    actstr=NULL;

    switch (act) {
        case PAC_MOVE:          acttyp=0; actstr=""; break;

        case PAC_FIREBALL:      acttyp=1; actstr="fireball"; break;
        case PAC_BALL:          acttyp=1; actstr="ball"; break;
        case PAC_LOOK_MAP:      acttyp=1; actstr="look"; break;
        case PAC_DROP:          acttyp=1; actstr="drop"; break;

        case PAC_TAKE:          acttyp=2; actstr="take"; break;
        case PAC_USE:           acttyp=2; actstr="use"; break;

        case PAC_KILL:          acttyp=3; actstr="attack"; break;
        case PAC_HEAL:          acttyp=3; actstr="heal"; break;
        case PAC_BLESS:         acttyp=3; actstr="bless"; break;
        case PAC_FREEZE:        acttyp=3; actstr="freeze"; break;
        case PAC_GIVE:          acttyp=3; actstr="give"; break;

        case PAC_IDLE:          acttyp=-1; break;
        case PAC_MAGICSHIELD:   acttyp=-1; break;
        case PAC_FLASH:         acttyp=-1; break;
        case PAC_WARCRY:        acttyp=-1; break;
        case PAC_BERSERK:       acttyp=-1; break;
        default:                acttyp=-1; break;
    }

    if (acttyp!=-1 && actstr) {
        mn=mapmn(actx-originx+MAPDX/2,acty-originy+MAPDY/2);
        mapx=mn%MAPDX;
        mapy=mn/MAPDX;
        mtos(mapx,mapy,&scrx,&scry);
        if (acttyp==0) dl_next_set(GNDSEL_LAY,5,scrx,scry,DDFX_NLIGHT);
        else dd_drawtext(scrx,scry,textcolor,DD_CENTER|DD_SMALL|DD_FRAME,actstr);
    }
}

int get_sink(int mn,struct map *cmap) {
    int x,y,mn2=-1,xp,yp,tot;

    x=cmap[mn].xadd;
    y=cmap[mn].yadd;

    xp=mn%MAPDX;
    yp=mn/MAPDX;

    if (x==0 && y==0) return cmap[mn].sink;

    if (x>0 && y==0 && xp<MAPDX-1) { tot=40;  mn2=mn-MAPDX+1; }
    if (x<0 && y==0 && xp>0) { tot=40; mn2=mn+MAPDX-1; }
    if (x==0 && y>0 && yp<MAPDY-1) { tot=20; mn2=mn+MAPDX+1; }
    if (x==0 && y<0 && yp>0) { tot=20; mn2=mn-MAPDX-1; }

    if (x>0 && y>0 && xp<MAPDX-1 && yp<MAPDY-1) { tot=30; mn2=mn+1; }
    if (x>0 && y<0 && xp<MAPDY-1 && yp>0) { tot=30; mn2=mn-MAPDX; }
    if (x<0 && y>0 && xp>0 && yp<MAPDY-1) { tot=30; mn2=mn+MAPDX; }
    if (x<0 && y<0 && xp>0 && yp>0) { tot=30;  mn2=mn-1; }

    if (mn2==-1) return cmap[mn].sink;

    x=abs(x);
    y=abs(y);

    return (cmap[mn].sink*(tot-x-y)+cmap[mn2].sink*(x+y))/tot;
}

void display_game_map(struct map *cmap) {
    int i,nr,mapx,mapy,mn,scrx,scry,light,mna,sprite,sink,xoff,yoff,start;
    DL *dl;
    int heightadd;

    start=SDL_GetTicks();

    for (i=0; i<maxquick; i++) {

        mn=quick[i].mn[4];
        scrx=mapaddx+quick[i].cx;
        scry=mapaddy+quick[i].cy;
        light=cmap[mn].rlight;

        // field is invisible - draw a black square and ignore everything else
        if (!light) { dl_next_set(GNDSTR_LAY,0,scrx,scry,DDFX_NLIGHT); continue; }

        // blit the grounds and straighten it, if neccassary ...
        if (cmap[mn].rg.sprite) {
            dl=dl_next_set(get_lay_sprite(cmap[mn].gsprite,GND_LAY),cmap[mn].rg.sprite,scrx,scry-10,light);
            if (!dl) { note("error in game #1"); continue; }

            if ((mna=quick[i].mn[3])!=0 && (cmap[mna].rlight)) dl->ddfx.ll=cmap[mna].rlight;
            else dl->ddfx.ll=light;
            if ((mna=quick[i].mn[5])!=0 && (cmap[mna].rlight)) dl->ddfx.rl=cmap[mna].rlight;
            else dl->ddfx.rl=light;
            if ((mna=quick[i].mn[1])!=0 && (cmap[mna].rlight)) dl->ddfx.ul=cmap[mna].rlight;
            else dl->ddfx.ul=light;
            if ((mna=quick[i].mn[7])!=0 && (cmap[mna].rlight)) dl->ddfx.dl=cmap[mna].rlight;
            else dl->ddfx.dl=light;

            dl->ddfx.scale=cmap[mn].rg.scale;
            dl->ddfx.cr=cmap[mn].rg.cr;
            dl->ddfx.cg=cmap[mn].rg.cg;
            dl->ddfx.cb=cmap[mn].rg.cb;
            dl->ddfx.clight=cmap[mn].rg.light;
            dl->ddfx.sat=cmap[mn].rg.sat;
            dl->ddfx.c1=cmap[mn].rg.c1;
            dl->ddfx.c2=cmap[mn].rg.c2;
            dl->ddfx.c3=cmap[mn].rg.c3;
            dl->ddfx.shine=cmap[mn].rg.shine;
            dl->h=-10;

            if (cmap[mn].flags&CMF_INFRA) { dl->ddfx.cr=min(120,dl->ddfx.cr+80); dl->ddfx.sat=min(20,dl->ddfx.sat+15); }
            if (cmap[mn].flags&CMF_UNDERWATER) { dl->ddfx.cb=min(120,dl->ddfx.cb+80); dl->ddfx.sat=min(20,dl->ddfx.sat+10); }

            gsprite_cnt++;
        }

        // ... 2nd (gsprite2)
        if (cmap[mn].rg2.sprite) {
            dl=dl_next_set(get_lay_sprite(cmap[mn].gsprite2,GND2_LAY),cmap[mn].rg2.sprite,scrx,scry,light);
            if (!dl) { note("error in game #2"); continue; }

            if ((mna=quick[i].mn[3])!=0 && (cmap[mna].rlight)) dl->ddfx.ll=cmap[mna].rlight;
            else dl->ddfx.ll=light;
            if ((mna=quick[i].mn[5])!=0 && (cmap[mna].rlight)) dl->ddfx.rl=cmap[mna].rlight;
            else dl->ddfx.rl=light;
            if ((mna=quick[i].mn[1])!=0 && (cmap[mna].rlight)) dl->ddfx.ul=cmap[mna].rlight;
            else dl->ddfx.ul=light;
            if ((mna=quick[i].mn[7])!=0 && (cmap[mna].rlight)) dl->ddfx.dl=cmap[mna].rlight;
            else dl->ddfx.dl=light;

            dl->ddfx.scale=cmap[mn].rg2.scale;
            dl->ddfx.cr=cmap[mn].rg2.cr;
            dl->ddfx.cg=cmap[mn].rg2.cg;
            dl->ddfx.cb=cmap[mn].rg2.cb;
            dl->ddfx.clight=cmap[mn].rg2.light;
            dl->ddfx.sat=cmap[mn].rg2.sat;
            dl->ddfx.c1=cmap[mn].rg2.c1;
            dl->ddfx.c2=cmap[mn].rg2.c2;
            dl->ddfx.c3=cmap[mn].rg2.c3;
            dl->ddfx.shine=cmap[mn].rg2.shine;

            if (cmap[mn].flags&CMF_INFRA) { dl->ddfx.cr=min(120,dl->ddfx.cr+80); dl->ddfx.sat=min(20,dl->ddfx.sat+15); }
            if (cmap[mn].flags&CMF_UNDERWATER) { dl->ddfx.cb=min(120,dl->ddfx.cb+80); dl->ddfx.sat=min(20,dl->ddfx.sat+10); }

            g2sprite_cnt++;
        }

        if (cmap[mn].mmf&MMF_STRAIGHT_T) dl_next_set(GNDSTR_LAY,50,scrx,scry,DDFX_NLIGHT);
        if (cmap[mn].mmf&MMF_STRAIGHT_B) dl_next_set(GNDSTR_LAY,51,scrx,scry,DDFX_NLIGHT);
        if (cmap[mn].mmf&MMF_STRAIGHT_L) dl_next_set(GNDSTR_LAY,52,scrx,scry,DDFX_NLIGHT);
        if (cmap[mn].mmf&MMF_STRAIGHT_R) dl_next_set(GNDSTR_LAY,53,scrx,scry,DDFX_NLIGHT);

        // blit fsprites
        if (cmap[mn].rf.sprite) {

            dl=dl_next_set(get_lay_sprite(cmap[mn].fsprite,GME_LAY),cmap[mn].rf.sprite,scrx,scry-9,light);
            if (!dl) { note("error in game #3"); continue; }
            dl->h=-9;
            if ((mna=quick[i].mn[3])!=0 && (cmap[mna].rlight)) dl->ddfx.ll=cmap[mna].rlight;
            else dl->ddfx.ll=light;
            if ((mna=quick[i].mn[5])!=0 && (cmap[mna].rlight)) dl->ddfx.rl=cmap[mna].rlight;
            else dl->ddfx.rl=light;
            if ((mna=quick[i].mn[1])!=0 && (cmap[mna].rlight)) dl->ddfx.ul=cmap[mna].rlight;
            else dl->ddfx.ul=light;
            if ((mna=quick[i].mn[7])!=0 && (cmap[mna].rlight)) dl->ddfx.dl=cmap[mna].rlight;
            else dl->ddfx.dl=light;

            if (no_lighting_sprite(cmap[mn].fsprite)) dl->ddfx.ll=dl->ddfx.rl=dl->ddfx.ul=dl->ddfx.dl=dl->ddfx.ml;

            // fsprite can increase the height of items and fsprite2
            heightadd=is_yadd_sprite(cmap[mn].rf.sprite);

            dl->ddfx.scale=cmap[mn].rf.scale;
            dl->ddfx.cr=cmap[mn].rf.cr;
            dl->ddfx.cg=cmap[mn].rf.cg;
            dl->ddfx.cb=cmap[mn].rf.cb;
            dl->ddfx.clight=cmap[mn].rf.light;
            dl->ddfx.sat=cmap[mn].rf.sat;
            dl->ddfx.c1=cmap[mn].rf.c1;
            dl->ddfx.c2=cmap[mn].rf.c2;
            dl->ddfx.c3=cmap[mn].rf.c3;
            dl->ddfx.shine=cmap[mn].rf.shine;

            if (cmap[mn].flags&CMF_INFRA) { dl->ddfx.cr=min(120,dl->ddfx.cr+80); dl->ddfx.sat=min(20,dl->ddfx.sat+15); }
            if (cmap[mn].flags&CMF_UNDERWATER) { dl->ddfx.cb=min(120,dl->ddfx.cb+80); dl->ddfx.sat=min(20,dl->ddfx.sat+10); }

            if (get_offset_sprite(cmap[mn].fsprite,&xoff,&yoff)) {
                dl->x+=xoff;
                dl->y+=yoff;
            }

            fsprite_cnt++;
        } else heightadd=0;

        // ... 2nd (fsprite2)
        if (cmap[mn].rf2.sprite) {

            dl=dl_next_set(get_lay_sprite(cmap[mn].fsprite2,GME_LAY),cmap[mn].rf2.sprite,scrx,scry+1,light);
            if (!dl) { note("error in game #5"); continue; }
            dl->h=1;
            if ((mna=quick[i].mn[3])!=0 && (cmap[mna].rlight)) dl->ddfx.ll=cmap[mna].rlight;
            else dl->ddfx.ll=light;
            if ((mna=quick[i].mn[5])!=0 && (cmap[mna].rlight)) dl->ddfx.rl=cmap[mna].rlight;
            else dl->ddfx.rl=light;
            if ((mna=quick[i].mn[1])!=0 && (cmap[mna].rlight)) dl->ddfx.ul=cmap[mna].rlight;
            else dl->ddfx.ul=light;
            if ((mna=quick[i].mn[7])!=0 && (cmap[mna].rlight)) dl->ddfx.dl=cmap[mna].rlight;
            else dl->ddfx.dl=light;

            if (no_lighting_sprite(cmap[mn].fsprite2)) dl->ddfx.ll=dl->ddfx.rl=dl->ddfx.ul=dl->ddfx.dl=dl->ddfx.ml;

            dl->y+=1;
            dl->h+=1;
            dl->h+=heightadd;
            dl->ddfx.scale=cmap[mn].rf2.scale;
            dl->ddfx.cr=cmap[mn].rf2.cr;
            dl->ddfx.cg=cmap[mn].rf2.cg;
            dl->ddfx.cb=cmap[mn].rf2.cb;
            dl->ddfx.clight=cmap[mn].rf2.light;
            dl->ddfx.sat=cmap[mn].rf2.sat;
            dl->ddfx.c1=cmap[mn].rf2.c1;
            dl->ddfx.c2=cmap[mn].rf2.c2;
            dl->ddfx.c3=cmap[mn].rf2.c3;
            dl->ddfx.shine=cmap[mn].rf2.shine;

            if (cmap[mn].flags&CMF_INFRA) { dl->ddfx.cr=min(120,dl->ddfx.cr+80); dl->ddfx.sat=min(20,dl->ddfx.sat+15); }
            if (cmap[mn].flags&CMF_UNDERWATER) { dl->ddfx.cb=min(120,dl->ddfx.cb+80); dl->ddfx.sat=min(20,dl->ddfx.sat+10); }

            if (get_offset_sprite(cmap[mn].fsprite2,&xoff,&yoff)) {
                dl->x+=xoff;
                dl->y+=yoff;
            }

            f2sprite_cnt++;
        }

        // blit items
        if (cmap[mn].isprite) {
            dl=dl_next_set(get_lay_sprite(cmap[mn].isprite,GME_LAY),cmap[mn].ri.sprite,scrx,scry-8,itmsel==mn?DDFX_BRIGHT:light);
            if (!dl) { note("error in game #8 (%d,%d)",cmap[mn].ri.sprite,cmap[mn].isprite); continue; }


#if 0
            // Disabled shaded lighting for items. It is often wrong and needs re-doing
            if ((mna=quick[i].mn[3])!=0 && (cmap[mna].rlight)) dl->ddfx.ll=cmap[mna].rlight;
            else dl->ddfx.ll=light;
            if ((mna=quick[i].mn[5])!=0 && (cmap[mna].rlight)) dl->ddfx.rl=cmap[mna].rlight;
            else dl->ddfx.rl=light;
            if ((mna=quick[i].mn[1])!=0 && (cmap[mna].rlight)) dl->ddfx.ul=cmap[mna].rlight;
            else dl->ddfx.ul=light;
            if ((mna=quick[i].mn[7])!=0 && (cmap[mna].rlight)) dl->ddfx.dl=cmap[mna].rlight;
            else dl->ddfx.dl=light;
#else
            dl->ddfx.ll=dl->ddfx.rl=dl->ddfx.ul=dl->ddfx.dl=dl->ddfx.ml;
#endif

            dl->h+=heightadd-8;
            dl->ddfx.scale=cmap[mn].ri.scale;
            dl->ddfx.cr=cmap[mn].ri.cr;
            dl->ddfx.cg=cmap[mn].ri.cg;
            dl->ddfx.cb=cmap[mn].ri.cb;
            dl->ddfx.clight=cmap[mn].ri.light;
            dl->ddfx.sat=cmap[mn].ri.sat;
            dl->ddfx.c1=cmap[mn].ri.c1;
            dl->ddfx.c2=cmap[mn].ri.c2;
            dl->ddfx.c3=cmap[mn].ri.c3;
            dl->ddfx.shine=cmap[mn].ri.shine;

            if (cmap[mn].flags&CMF_INFRA) { dl->ddfx.cr=min(120,dl->ddfx.cr+80); dl->ddfx.sat=min(20,dl->ddfx.sat+15); }
            if (cmap[mn].flags&CMF_UNDERWATER) { dl->ddfx.cb=min(120,dl->ddfx.cb+80); dl->ddfx.sat=min(20,dl->ddfx.sat+10); }

            if (cmap[mn].flags&CMF_TAKE) {
                dl->ddfx.sink=min(12,cmap[mn].sink);
                dl->y+=min(6,cmap[mn].sink/2);
                dl->h+=-min(6,cmap[mn].sink/2);
            } else if (cmap[mn].flags&CMF_USE) {
                dl->ddfx.sink=min(20,cmap[mn].sink);
                dl->y+=min(10,cmap[mn].sink/2);
                dl->h+=-min(10,cmap[mn].sink/2);
            }

            if (get_offset_sprite(cmap[mn].isprite,&xoff,&yoff)) {
                dl->x+=xoff;
                dl->y+=yoff;
            }

            isprite_cnt++;
        }

        // blit chars
        if (cmap[mn].csprite) {
            dl=dl_next_set(GME_LAY,cmap[mn].rc.sprite,scrx+cmap[mn].xadd,scry+cmap[mn].yadd,chrsel==mn?DDFX_BRIGHT:light);
            if (!dl) { note("error in game #9"); continue; }
            sink=get_sink(mn,cmap);
            dl->ddfx.sink=sink;
            dl->y+=sink/2;
            dl->h=-sink/2;
            dl->ddfx.scale=cmap[mn].rc.scale;
            //addline("sprite=%d, scale=%d",cmap[mn].rc.sprite,cmap[mn].rc.scale);
            dl->ddfx.cr=cmap[mn].rc.cr;
            dl->ddfx.cg=cmap[mn].rc.cg;
            dl->ddfx.cb=cmap[mn].rc.cb;
            dl->ddfx.clight=cmap[mn].rc.light;
            dl->ddfx.sat=cmap[mn].rc.sat;
            dl->ddfx.c1=cmap[mn].rc.c1;
            dl->ddfx.c2=cmap[mn].rc.c2;
            dl->ddfx.c3=cmap[mn].rc.c3;
            dl->ddfx.shine=cmap[mn].rc.shine;

            // check for spells on char
            for (nr=0; nr<MAXEF; nr++) {
                if (!ueffect[nr]) continue;
                if ((unsigned int)ceffect[nr].freeze.cn==map[mn].cn && ceffect[nr].generic.type==11) { // freeze
                    int diff;

                    if ((diff=tick-ceffect[nr].freeze.start)<DDFX_MAX_FREEZE*4) {   // starting
                        dl->ddfx.freeze=diff/4;
                    } else if (ceffect[nr].freeze.stop<tick) {          // already finished
                        continue;
                    } else if ((diff=ceffect[nr].freeze.stop-tick)<DDFX_MAX_FREEZE*4) { // ending
                        dl->ddfx.freeze=diff/4;
                    } else dl->ddfx.freeze=DDFX_MAX_FREEZE-1;       // running
                }
                if ((unsigned int)ceffect[nr].curse.cn==map[mn].cn && ceffect[nr].generic.type==18) { // curse

                    dl->ddfx.sat=min(20,dl->ddfx.sat+(ceffect[nr].curse.strength/4)+5);
                    dl->ddfx.clight=min(120,dl->ddfx.clight+ceffect[nr].curse.strength*2+40);
                    dl->ddfx.cb=min(80,dl->ddfx.cb+ceffect[nr].curse.strength/2+10);
                }
                if ((unsigned int)ceffect[nr].cap.cn==map[mn].cn && ceffect[nr].generic.type==19) { // palace cap

                    dl->ddfx.sat=min(20,dl->ddfx.sat+20);
                    dl->ddfx.clight=min(120,dl->ddfx.clight+80);
                    dl->ddfx.cb=min(80,dl->ddfx.cb+80);
                }
                if ((unsigned int)ceffect[nr].lag.cn==map[mn].cn && ceffect[nr].generic.type==20) { // lag

                    dl->ddfx.sat=min(20,dl->ddfx.sat+20);
                    dl->ddfx.clight=max(-120,dl->ddfx.clight-80);
                }
            }
            if (cmap[mn].gsprite==51066) {
                dl->ddfx.sat=20;
                dl->ddfx.cr=80;
                dl->ddfx.clight=-80;
                dl->ddfx.shine=50;
                dl->ddfx.ml=dl->ddfx.ll=dl->ddfx.rl=dl->ddfx.ul=dl->ddfx.dl=chrsel==mn?DDFX_BRIGHT:DDFX_NLIGHT;
            } else if (cmap[mn].gsprite==51067) {
                dl->ddfx.sat=20;
                dl->ddfx.cb=80;
                dl->ddfx.clight=-80;
                dl->ddfx.shine=50;
                dl->ddfx.ml=dl->ddfx.ll=dl->ddfx.rl=dl->ddfx.ul=dl->ddfx.dl=chrsel==mn?DDFX_BRIGHT:DDFX_NLIGHT;
            } else {
                if (cmap[mn].flags&CMF_INFRA) { dl->ddfx.cr=min(120,dl->ddfx.cr+80); dl->ddfx.sat=min(20,dl->ddfx.sat+15); }
                if (cmap[mn].flags&CMF_UNDERWATER) { dl->ddfx.cb=min(120,dl->ddfx.cb+80); dl->ddfx.sat=min(20,dl->ddfx.sat+10); }
            }

            csprite_cnt++;
        }
    }
    show_bubbles();
    dg_time+=SDL_GetTicks()-start;

    if (cmap==map) {            // avoid acting on prefetch
        // selection on ground
        if (mapsel!=-1 || context_getnm()!=-1) {
            if (context_getnm()!=-1) mn=context_getnm();
            else mn=mapsel;
            mapx=mn%MAPDX;
            mapy=mn/MAPDX;
            mtos(mapx,mapy,&scrx,&scry);
            if (cmap[mn].rlight==0 || (cmap[mn].mmf&MMF_SIGHTBLOCK)) sprite=SPR_FFIELD;
            else sprite=SPR_FIELD;
            dl=dl_next_set(GNDSEL_LAY,sprite,scrx,scry,DDFX_NLIGHT);
            if (!dl) note("error in game #10");
        }
        // act (field) quick and dirty
        if (act==PAC_MOVE) display_game_act();

        dl_play();

        // act (text)  quick and dirty
        if (act!=PAC_MOVE) display_game_act();
    }
}

void display_pents(void) {
    int n,col,yoff;

    for (n=0; n<7; n++) {
        switch (pent_str[n][0]) {
            case '0':	col=graycolor; break;
            case '1':	col=redcolor; break;
            case '2':	col=greencolor; break;
            case '3':	col=bluecolor; break;

            default:	continue;
        }
        if (context_action_enabled()) yoff=30;
        else yoff=0;
        dd_drawtext(dotx(DOT_BOT)+550,doty(DOT_BOT)-80+n*10-yoff,col,DD_SMALL|DD_FRAME,pent_str[n]+1);
    }
}

void display_game(void) {
    display_game_spells();
    display_game_spells2();
    display_game_map(map);
    display_game_names();
    display_pents();
}

// make quick
int quick_qcmp(const void *va,const void *vb) {
    const QUICK *a;
    const QUICK *b;

    a=(QUICK *)va;
    b=(QUICK *)vb;

    if (a->mapx+a->mapy<b->mapx+b->mapy) return -1;
    else if (a->mapx+a->mapy>b->mapx+b->mapy) return 1;

    return a->mapx-b->mapx;
}

void make_quick(int game,int mcx,int mcy) {
    int cnt;
    int x,y,xs,xe,i,ii;
    int dist=DIST;

    if (game) {
        set_mapoff(mcx,mcy,MAPDX,MAPDY);
        set_mapadd(0,0);
    }

    // calc maxquick
    for (i=y=0; y<=dist*2; y++) {
        if (y<dist) { xs=dist-y; xe=dist+y; } else { xs=y-dist; xe=dist*3-y; }
        for (x=xs; x<=xe; x++) {
            i++;
        }
    }
    maxquick=i;

    // set quick (and mn[4]) in server order
    quick=xrealloc(quick,(maxquick+1)*sizeof(QUICK),MEM_GAME);
    for (i=y=0; y<=dist*2; y++) {
        if (y<dist) { xs=dist-y; xe=dist+y; } else { xs=y-dist; xe=dist*3-y; }
        for (x=xs; x<=xe; x++) {
            quick[i].mn[4]=x+y*(dist*2+1);

            quick[i].mapx=x;
            quick[i].mapy=y;
            mtos(x,y,&quick[i].cx,&quick[i].cy);
            i++;
        }
    }

    // sort quick in client order
    qsort(quick,maxquick,sizeof(QUICK),quick_qcmp);

    // set quick neighbours
    cnt=0;
    for (i=0; i<maxquick; i++) {
        for (y=-1; y<=1; y++) {
            for (x=-1; x<=1; x++) {

                if (x==1 || (x==0 && y==1)) {
                    for (ii=i+1; ii<maxquick; ii++) if (quick[i].mapx+x==quick[ii].mapx && quick[i].mapy+y==quick[ii].mapy) break;
                            else cnt++;
                } else if (x==-1 || (x==0 && y==-1)) {
                    for (ii=i-1; ii>=0; ii--) if (quick[i].mapx+x==quick[ii].mapx && quick[i].mapy+y==quick[ii].mapy) break;
                            else cnt++;
                    if (ii==-1) ii=maxquick;
                } else {
                    ii=i;
                }

                if (ii==maxquick) {
                    quick[i].mn[(x+1)+(y+1)*3]=0;
                    quick[i].qi[(x+1)+(y+1)*3]=maxquick;
                } else {
                    quick[i].mn[(x+1)+(y+1)*3]=quick[ii].mn[4];
                    quick[i].qi[(x+1)+(y+1)*3]=ii;
                }
            }

        }
    }

    // set values for quick[maxquick]
    for (y=-1; y<=1; y++) {
        for (x=-1; x<=1; x++) {
            quick[maxquick].mn[(x+1)+(y+1)*3]=0;
            quick[maxquick].qi[(x+1)+(y+1)*3]=maxquick;
        }
    }
}

// init, exit

void init_game(int mcx,int mcy) {
    make_quick(1,mcx,mcy);
}

void exit_game(void) {
    xfree(quick);
    quick=NULL;
    maxquick=0;
    xfree(dllist);
    dllist=NULL;
    xfree(dlsort);
    dlsort=NULL;
    dlused=0;
    dlmax=0;


}

void prefetch_game(int attick) {

    set_map_values(map2,attick);
    set_mapadd(-map2[mapmn(MAPDX/2,MAPDY/2)].xadd,-map2[mapmn(MAPDX/2,MAPDY/2)].yadd);
    display_game_map(map2);
    dl_prefetch(attick);

#ifdef TICKPRINT
    printf("Prefetch %d\n",attick);
#endif
}

