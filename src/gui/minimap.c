/*
 * Part of Astonia Client (c) Daniel Brockhaus. Please read license.txt.
 *
 * Minimap
 *
 *
 */

#include <stdint.h>
#include <SDL2/SDL.h>

#include "../../src/astonia.h"
#include "../../src/gui.h"
#include "../../src/gui/_gui.h"
#include "../../src/client.h"
#include "../../src/game.h"
#include "../../src/sdl.h"

#define MINIMAP 40
#define MAXMAP  256
#define IRGBA(r,g,b,a)  (((a)<<24)|((r)<<16)|((g)<<8)|((b)<<0))

static int sx,sy,visible,mx,my,update1,update2,orx,ory,rewrite_cnt;

static unsigned char _mmap[MAXMAP*MAXMAP];

static uint32_t mapix1[MAXMAP*MAXMAP];
static uint32_t mapix2[MINIMAP*MINIMAP*4];

SDL_Texture *maptex1=NULL,*maptex2=NULL;

void minimap_init(void) {

    sx=(dotx(DOT_MTL)+dotx(DOT_MBR))/2-MAXMAP/2;
    sy=(doty(DOT_MTL)+doty(DOT_MBR))/2-MAXMAP/2;

    mx=dotx(DOT_MBR)-MINIMAP*2-10;
    my=doty(DOT_MTL)+10;

    memset(_mmap,0,sizeof(_mmap));
    visible=0;
    update1=update2=1;

    maptex1=sdl_create_texture(MAXMAP,MAXMAP);
    maptex2=sdl_create_texture(MINIMAP*2,MINIMAP*2);
    SDL_SetTextureBlendMode(maptex1,SDL_BLENDMODE_BLEND);
    SDL_SetTextureBlendMode(maptex2,SDL_BLENDMODE_BLEND);
}

static void set_pix(int x,int y,int val) {
    if (_mmap[x+y*MAXMAP]!=val) {

        // count how much of the map has changed permanently (not counting characters
        // and formerly unknown tiles)
        if (_mmap[x+y*MAXMAP]!=0 && _mmap[x+y*MAXMAP]!=3 && val!=3) {
            rewrite_cnt++;
        }

        _mmap[x+y*MAXMAP]=val;
        update1=update2=1;
    }
}

void minimap_update(void) {
    int x,y,xs,xe,ox,oy;

    ox=originx-DIST;
    oy=originy-DIST;

    rewrite_cnt=0;
    for (y=1; y<DIST*2; y++) {
        if (y+oy<0) continue;
        if (y+oy>=MAXMAP) continue;

		if (y<DIST) { xs=DIST-y; xe=DIST+y; }
		else { xs=y-DIST; xe=DIST*3-y; }

		for (x=xs+1; x<xe; x++) {
            if (x+ox<0) continue;
            if (x+ox>=MAXMAP) continue;
            if (!(map[x+y*MAPDX].flags&CMF_VISIBLE)) continue;


            if (map[x+y*MAPDX].mmf&MMF_SIGHTBLOCK) {
                if (map[x+y*MAPDX].flags&CMF_USE) set_pix(ox+x,oy+y,5);
                else set_pix(ox+x,oy+y,1);
            } else if (map[x+y*MAPDX].fsprite) set_pix(ox+x,oy+y,2);
            else if (map[x+y*MAPDX].csprite) set_pix(ox+x,oy+y,3);
            else set_pix(ox+x,oy+y,4);
        }
    }
    if (rewrite_cnt>4) {
        memset(_mmap,0,sizeof(_mmap));
        update1=update2=1;
        note("MAP CHANGED: %d",rewrite_cnt);
    }
}

static uint32_t pix_col(int x,int y) {
    switch (_mmap[x+y*MAXMAP]) {

        case 1: return IRGBA(180,180,180,255);
        case 2: return IRGBA(140,140,220,255);
        case 3: return IRGBA(60,220,60,255);
        case 4: return IRGBA(60,60,60,255);
        case 5: return IRGBA(120,80,80,255);
        case 0:
        default: return IRGBA(25,25,25,255);
    }
}

static void draw_center(int x,int y) {
    dd_pixel(x,y,IRGB(15,8,8));
    dd_pixel(x+1,y,IRGB(15,8,8));
    dd_pixel(x,y+1,IRGB(15,8,8));
    dd_pixel(x-1,y,IRGB(15,8,8));
    dd_pixel(x,y-1,IRGB(15,8,8));
}

void display_minimap(void) {
    int x,y,ix,iy;
    float dist;
    SDL_Rect dr,sr;

    if (visible) {  // display big map
        if (update1) {
            for (y=0; y<MAXMAP; y++) {
                for (x=0; x<MAXMAP; x++) {
                    mapix1[x+y*MAXMAP]=pix_col(x,y);
                }
            }
            SDL_UpdateTexture(maptex1,NULL,mapix1,MAXMAP*sizeof(uint32_t));
            update1=0;
        }

        dr.x=(sx+x_offset)*sdl_scale; dr.w=MAXMAP*sdl_scale;
        dr.y=(sy+y_offset)*sdl_scale; dr.h=MAXMAP*sdl_scale;

        sr.x=0; sr.w=MAXMAP;
        sr.y=0; sr.h=MAXMAP;

        sdl_render_copy(maptex1,&sr,&dr);
        draw_center(sx+originx,sy+originy);
    }

    if (orx!=originx || ory!=originy) {
        update2=1;
        orx=originx;
        ory=originy;
    }

    if (update2) {
        bzero(mapix2,sizeof(mapix2));
        for (iy=-MINIMAP; iy<=MINIMAP; iy++) {
            for (ix=-MINIMAP; ix<=MINIMAP; ix++) {

                dist=sqrtf(ix*ix+iy*iy);
                if (dist>=MINIMAP) continue;

                x=originx+ix;
                y=originy+iy;

                if (x<0 || x>=MAXMAP || y<0 || y>=MAXMAP) {
                    continue;
                }

                if (_mmap[x+y*MAXMAP]) mapix2[MINIMAP+ix+iy*MINIMAP*2+MINIMAP*MINIMAP*2]=pix_col(x,y);
            }
        }
        SDL_UpdateTexture(maptex2,NULL,mapix2,MINIMAP*2*sizeof(uint32_t));
        update2=0;
    }

    dr.x=(mx+x_offset)*sdl_scale; dr.w=MINIMAP*2*sdl_scale;
    dr.y=(my+y_offset)*sdl_scale; dr.h=MINIMAP*2*sdl_scale;

    sr.x=0; sr.w=MINIMAP*2;
    sr.y=0; sr.h=MINIMAP*2;

    sdl_render_copy(maptex2,&sr,&dr);
    draw_center(mx+MINIMAP,my+MINIMAP);
}

void minimap_areachange(void) {
    memset(_mmap,0,sizeof(_mmap));
    update1=update2=1;
}

void minimap_toggle(void) {
    visible^=1;
}

void minimap_hide(void) {
    visible=0;
}

