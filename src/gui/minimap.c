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

    sx=dotx(DOT_MBR)-MAXMAP-6;
    sy=doty(DOT_MTL)+6;

    mx=dotx(DOT_MBR)-MINIMAP*2-6;
    my=doty(DOT_MTL)+6;

    memset(_mmap,0,sizeof(_mmap));
    visible=1;
    update1=update2=1;

    maptex1=sdl_create_texture(MAXMAP,MAXMAP);
    maptex2=sdl_create_texture(MINIMAP*2,MINIMAP*2);
    SDL_SetTextureBlendMode(maptex1,SDL_BLENDMODE_BLEND);
    SDL_SetTextureBlendMode(maptex2,SDL_BLENDMODE_BLEND);
}

static void set_pix(int x,int y,int val) {
    int val2;

    if ((val2=_mmap[x+y*MAXMAP])!=val) {

        // count how much of the map has changed permanently (not counting characters
        // and formerly unknown tiles or swapping between sightblocks and fsprites)
        if (val2!=0 && val2!=3 && val!=3 && !((val==1 && val2==2) || (val==2 && val2==1))) {
            //note("changed: %d to %d (%d,%d)",_mmap[x+y*MAXMAP],val,x,y);
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
            else if (map[x+y*MAPDX].csprite && x+y*MAPDX!=plrmn) set_pix(ox+x,oy+y,3);
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
    dd_pixel(x,y,IRGB(31,8,8));
    dd_pixel(x+1,y,IRGB(31,8,8));
    dd_pixel(x,y+1,IRGB(31,8,8));
    dd_pixel(x-1,y,IRGB(31,8,8));
    dd_pixel(x,y-1,IRGB(31,8,8));
}

static void draw_center2(int x,int y) {
    int i;

    dd_pixel(x,y,IRGB(31,8,8));

    for (i=0; i<3; i++) {
        dd_pixel(x+i,y,IRGB(31,8,8));
        dd_pixel(x,y+i,IRGB(31,8,8));
        dd_pixel(x-i,y,IRGB(31,8,8));
        dd_pixel(x,y-i,IRGB(31,8,8));
    }
}

void display_minimap(void) {
    int x,y,ix,iy,i;
    float dist;
    SDL_Rect dr,sr;

    if (visible&2) {  // display big map
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

        if (visible&1) {
            sr.x=0; sr.w=MAXMAP;
            sr.y=0; sr.h=MAXMAP;
            sdl_render_copy(maptex1,&sr,&dr);
            draw_center(sx+originx,sy+originy);
        } else {
            x=originx-MAXMAP/6;
            y=originy-MAXMAP/6;
            if (x<0) x=0;
            if (x>MAXMAP-MAXMAP/3) x=MAXMAP-MAXMAP/3;
            if (y<0) y=0;
            if (y>MAXMAP-MAXMAP/3) y=MAXMAP-MAXMAP/3;

            sr.x=x; sr.w=MAXMAP/3;
            sr.y=y; sr.h=MAXMAP/3;

            sdl_render_copy(maptex1,&sr,&dr);
            draw_center2(sx+(originx-x)*3+2,sy+(originy-y)*3+2);
        }

        dd_line(sx,sy,sx,sy+MAXMAP,0xffff);
        dd_line(sx,sy+MAXMAP,sx+MAXMAP,sy+MAXMAP,0xffff);
        dd_line(sx+MAXMAP,sy+MAXMAP,sx+MAXMAP,sy,0xffff);
        dd_line(sx+MAXMAP,sy,sx,sy,0xffff);

        dd_drawtext(sx+6,sy+6,0xffff,0,"N");
    }

    if (orx!=originx || ory!=originy) {
        update2=1;
        orx=originx;
        ory=originy;
    }

    if (visible==1) {
        if (update2) {
            bzero(mapix2,sizeof(mapix2));
            for (iy=-MINIMAP; iy<=MINIMAP; iy++) {
                for (ix=-MINIMAP; ix<=MINIMAP; ix++) {

                    dist=sqrtf(ix*ix+iy*iy);
                    if (dist>MINIMAP) continue;

                    x=originx+ix;
                    y=originy+iy;

                    if (x<0 || x>=MAXMAP || y<0 || y>=MAXMAP)
                        mapix2[MINIMAP+ix+iy*MINIMAP*2+MINIMAP*MINIMAP*2]=IRGBA(25,25,25,255);
                    else mapix2[MINIMAP+ix+iy*MINIMAP*2+MINIMAP*MINIMAP*2]=pix_col(x,y);
                }
            }
            SDL_UpdateTexture(maptex2,NULL,mapix2,MINIMAP*2*sizeof(uint32_t));
            update2=0;
        }

        dr.x=(mx+x_offset)*sdl_scale; dr.w=MINIMAP*2*sdl_scale;
        dr.y=(my+y_offset)*sdl_scale; dr.h=MINIMAP*2*sdl_scale;

        sr.x=0; sr.w=MINIMAP*2;
        sr.y=0; sr.h=MINIMAP*2;

        sdl_render_copy_ex(maptex2,&sr,&dr,45.0);
        draw_center(mx+MINIMAP,my+MINIMAP);

        for (i=0; i<sdl_scale; i++) {
            sdl_render_circle((mx+MINIMAP+x_offset)*sdl_scale,(my+MINIMAP+y_offset)*sdl_scale,(MINIMAP)*sdl_scale+i,0xffffffff);
        }
        dd_drawtext(mx+MINIMAP,my+4,0xffff,0,"N");
    }
}

void minimap_clear(void) {
    memset(_mmap,0,sizeof(_mmap));
    update1=update2=1;
}

void minimap_toggle(void) {
    visible=(visible+1)%4;
}

void minimap_hide(void) {
    visible=1;
}

