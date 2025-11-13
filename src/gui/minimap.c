/*
 * Part of Astonia Client (c) Daniel Brockhaus. Please read license.txt.
 *
 * Minimap
 *
 *
 */

#include <stdint.h>
#include <SDL2/SDL.h>
#include <unistd.h>
#include <fcntl.h>

#include "../../src/astonia.h"
#include "../../src/gui.h"
#include "../../src/gui/_gui.h"
#include "../../src/client.h"
#include "../../src/game.h"
#include "../../src/sdl.h"

#define MINIMAP 40
#define MAXMAP  256
#define IRGBA(r,g,b,a)  (((a)<<24)|((r)<<16)|((g)<<8)|((b)<<0))

static int sx,sy,visible,mx,my,update1,update2,update3,orx,ory,rewrite_cnt;

static unsigned char _mmap[MAXMAP*MAXMAP];

static uint32_t mapix1[MAXMAP*MAXMAP];
static uint32_t mapix2[MINIMAP*MINIMAP*4];

#define MAXSAVEMAP  100
static int mapnr=-1;

SDL_Texture *maptex1=NULL,*maptex2=NULL;

void minimap_init(void) {

    if (game_options&GO_NOMAP) return;

    sx=dotx(DOT_MBR)-MAXMAP-6;
    sy=doty(DOT_MTL)+6;

    mx=dotx(DOT_MBR)-MINIMAP*2-6;
    my=doty(DOT_MTL)+6;

    memset(_mmap,0,sizeof(_mmap));
    visible=1;
    update1=update2=update3=1;

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
        update1=update2=update3=1;
    }
}

static void map_save(void);
static int map_load(void);

void minimap_update(void) {
    int x,y,xs,xe,ox,oy;

    if (game_options&GO_NOMAP) return;

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
    if (mapnr==-1 && update3) {
        update3=0;
        if (game_options&GO_MAPSAVE) mapnr=map_load();
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

    if (game_options&GO_NOMAP) return;

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
            for (iy=-MINIMAP; iy<MINIMAP; iy++) {
                for (ix=-MINIMAP; ix<MINIMAP; ix++) {

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
    if (game_options&GO_MAPSAVE) map_save();
    mapnr=-1;
    memset(_mmap,0,sizeof(_mmap));
    update1=update2=update3=1;
}

void minimap_toggle(void) {
    visible=(visible+1)%4;
}

void minimap_hide(void) {
    if (visible) visible=1;
}

static char *mapname(int i) {
    static char filename[MAX_PATH];

    if (game_options&GO_APPDATA) sprintf(filename,"%s\\Astonia\\map%03d.dat",localdata,i);
    else sprintf(filename,"bin/data/map%03d.dat",i);

    return filename;
}

static void map_save(void) {
    int i,cnt,handle;
    char *filename;

    for (i=cnt=0; i<MAXMAP*MAXMAP; i++) {
        if (_mmap[i]) cnt++;
    }
    if (cnt<250) return;

    // check if another client wrote the same map
    // in the meantime
    mapnr=map_load();

    // new map, find a save-slot
    if (mapnr==-1) {
        for (i=0; i<MAXSAVEMAP; i++) {
            filename=mapname(i);
            handle=open(filename,O_RDONLY);
            close(handle);
            if (handle==-1) break;
        }
        if (i==MAXSAVEMAP) {
            warn("Area map storage full! Please use /compactmap to merge duplicate maps.");
            return;
        }
        mapnr=i;
    }

    filename=mapname(mapnr);
    //note("saving area map to %s",filename);
    handle=open(filename,O_RDWR|O_BINARY|O_TRUNC|O_CREAT,0644);
    write(handle,_mmap,sizeof(_mmap));
    close(handle);
}

static int map_compare(char *tmap,char *xmap) {
    int i,hit,miss;

    for (i=hit=miss=0; i<MAXMAP*MAXMAP; i++) {
        // sightblock, fsprite or usable sightblock
        if (tmap[i]==1 || tmap[i]==2 || tmap[i]==5) {
            if (xmap[i]==1 || xmap[i]==2 || xmap[i]==5) hit++;
            else if (xmap[i]!=0) miss++;
        }
        // empty or csprite
        if (tmap[i]==3 || tmap[i]==4) {
            if (xmap[i]==3 || xmap[i]==4) hit++;
            else if (xmap[i]!=0) miss++;
        }
    }
    if (hit<200) return 0;
    if (miss>hit/100) return 0;

    return hit;
}

static void map_merge(char *xmap,char *tmap) {
    int i;

    // only overwrite empty parts of the map with loaded data.
    for (i=0; i<MAXMAP*MAXMAP; i++) {
        if (!xmap[i]) {
            if (tmap[i]==3) xmap[i]=4; // do not load csprites, they move too much
            else xmap[i]=tmap[i];
        }
    }
}

static int map_load(void) {
    int i,hit,handle,besti=-1,besthit=0;
    unsigned char tmap[MAXMAP*MAXMAP];
    char *filename;

    for (i=0; i<MAXSAVEMAP; i++) {
        filename=mapname(i);
        handle=open(filename,O_RDONLY|O_BINARY);
        if (handle==-1) continue;
        read(handle,tmap,sizeof(tmap));
        close(handle);

        if (!(hit=map_compare(tmap,_mmap))) continue;

        if (hit>besthit) {
            besti=i;
            besthit=hit;
        }
    }
    if (besti!=-1) {
        filename=mapname(besti);
        //note("loading area map from %s (%d hits)",filename,besthit);
        handle=open(filename,O_RDONLY);
        if (handle==-1) return -1;
        read(handle,tmap,sizeof(tmap));
        close(handle);

        map_merge(_mmap,tmap);

        return besti;
    }

    return -1;
}



void minimap_compact(void) {
    int i,j,handle;
    char *filename,tmap[MAXMAP*MAXMAP],xmap[MAXMAP*MAXMAP];

    if (game_options&GO_NOMAP) return;

    for (i=0; i<MAXSAVEMAP; i++) {
        filename=mapname(i);
        handle=open(filename,O_RDONLY|O_BINARY);
        if (handle==-1) continue;
        read(handle,tmap,sizeof(tmap));
        close(handle);

        for (j=i+1; j<MAXSAVEMAP; j++) {
            filename=mapname(j);
            handle=open(filename,O_RDONLY|O_BINARY);
            if (handle==-1) continue;
            read(handle,xmap,sizeof(xmap));
            close(handle);

            if (map_compare(tmap,xmap)) {
                map_merge(tmap,xmap);
                filename=mapname(i);
                handle=open(filename,O_RDONLY|O_BINARY);
                if (handle==-1) continue;
                write(handle,tmap,sizeof(tmap));
                close(handle);

                filename=mapname(j);
                unlink(filename);
                note("merged map %d into map %d",j,i);
            }
        }
    }
}

