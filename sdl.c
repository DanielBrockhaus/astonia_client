/*
 * Part of Astonia Client (c) Daniel Brockhaus. Please read license.txt.
 */

#include <stdint.h>
#include <windows.h>
#include <SDL2/SDL.h>
#include <png.h>

#include "main.h"
#include "sdl.h"

SDL_Window *sdlwnd;
SDL_Renderer *sdlren;

extern int gfx_force_png;

#define MAX_TEXCACHE    2500
#define MAX_TEXHASH     2500
#define STX_NONE        (-1)

#define SF_USED         (1<<0)

struct sdl_texture {
    SDL_Texture *tex;

    int prev,next;
    int hprev,hnext;

    uint16_t flags;

    // fx
    int32_t sprite;
    int8_t sink;
    uint8_t scale;
    int16_t cr,cg,cb,light,sat;
    uint16_t c1,c2,c3,shine;

    uint8_t freeze;
    uint8_t grid;

    // light
    int8_t ml,ll,rl,ul,dl;      // light in middle, left, right, up, down

    // primary
    uint16_t xres;              // x resolution in pixels
    uint16_t yres;              // y resolution in pixels
    int16_t xoff;               // offset to blit position
    int16_t yoff;               // offset to blit position
    uint16_t size;              // size in pixels (xres*yres) - TODO: needed?
};

struct sdl_texture *sdlt=NULL;
int sdlt_best,sdlt_last;
int *sdlt_cache;

struct sdl_image {
    uint32_t *pixel;

    uint16_t flags;
    int16_t xres,yres;
    int16_t xoff,yoff;
};

struct sdl_image *sdli=NULL;

long mem_png=0,mem_tex=0;
long texc_hit=0,texc_miss=0;

int sdl_init(int width,int height,char *title) {
    int len,i;

    if (SDL_Init(SDL_INIT_VIDEO) != 0){
        printf("SDL_Init Error: %s",SDL_GetError());
	    return 0;
    }

    sdlwnd = SDL_CreateWindow(title, 2560-width-100, 100, width, height, SDL_WINDOW_SHOWN);
    if (!sdlwnd) {
        printf("SDL_Init Error: %s",SDL_GetError());
        SDL_Quit();
	    return 0;
    }

    sdlren=SDL_CreateRenderer(sdlwnd, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!sdlren){
        SDL_DestroyWindow(sdlwnd);
        printf("SDL_Init Error: %s",SDL_GetError());
        SDL_Quit();
        return 0;
    }

    gfx_force_png=1;    // TODO: remove me, for now SDL only works with PNGs, not with PAKs

    len=sizeof(struct sdl_image)*MAXSPRITE;
    note("SDL Image cache needs %.2fM for image cache index",len/(1024.0*1024.0));
    sdli=calloc(len,1);
    if (!sdli) return fail("Out of memory in sdl_init");

    sdlt_cache=calloc(MAX_TEXHASH,sizeof(int));
    if (!sdlt_cache) return fail("Out of memory in sdl_init");

    for (i=0; i<MAX_TEXHASH; i++)
        sdlt_cache[i]=STX_NONE;

    sdlt=calloc(MAX_TEXCACHE,sizeof(struct sdl_texture));
    if (!sdlt) return fail("Out of memory in sdl_init");

    for (i=0; i<MAX_TEXCACHE; i++) {
        sdlt[i].flags=0;
        sdlt[i].prev=i-1;
        sdlt[i].next=i+1;
        sdlt[i].hnext=STX_NONE;
        sdlt[i].hprev=STX_NONE;
    }
    sdlt[0].prev=STX_NONE;
    sdlt[MAX_TEXCACHE-1].next=STX_NONE;
    sdlt_best=0;
    sdlt_last=MAX_TEXCACHE-1;



    return 1;
}

int maxpanic=0;
int sdl_clear(void) {
    SDL_SetRenderDrawColor(sdlren,31,63,127,255);
    SDL_RenderClear(sdlren);
    printf("mem: %.2fM PNG, %.2fM Tex, Hit: %ld, Miss: %ld, Max: %d\n",mem_png/(1024.0*1024.0),mem_tex/(1024.0*1024.0),texc_hit,texc_miss,maxpanic); fflush(stdout);
    maxpanic=0;
    return 1;
}

int sdl_render(void) {
    SDL_RenderPresent(sdlren);
    return 1;
}

int sdl_load_image_png(struct sdl_image *si,char *filename) {
    int x,y,xres,yres,tmp,r,g,b,a,sx,sy,ex,ey;
    int format;
    unsigned char **row;
    FILE *fp;
    png_structp png_ptr;
    png_infop info_ptr;
    png_infop end_info;

    fp=fopen(filename,"rb");
    if (!fp) return -1;

    png_ptr=png_create_read_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
    if (!png_ptr) { fclose(fp); printf("create read\n"); return -1; }

    info_ptr=png_create_info_struct(png_ptr);
    if (!info_ptr) { fclose(fp); png_destroy_read_struct(&png_ptr,(png_infopp)NULL,(png_infopp)NULL); printf("create info1\n"); return -1; }

    end_info=png_create_info_struct(png_ptr);
    if (!end_info) { fclose(fp); png_destroy_read_struct(&png_ptr,&info_ptr,(png_infopp)NULL); printf("create info2\n"); return -1; }

    png_init_io(png_ptr,fp);
    png_set_strip_16(png_ptr);
    png_read_png(png_ptr,info_ptr,PNG_TRANSFORM_PACKING,NULL);

    row=png_get_rows(png_ptr,info_ptr);
    if (!row) { fclose(fp); png_destroy_read_struct(&png_ptr,&info_ptr,(png_infopp)NULL); printf("read row\n"); return -1; }

    xres=png_get_image_width(png_ptr,info_ptr);
    yres=png_get_image_height(png_ptr,info_ptr);

    tmp=png_get_rowbytes(png_ptr,info_ptr);

    if (tmp==xres*3) format=3;
    else if (tmp==xres*4) format=4;
    else { fclose(fp); png_destroy_read_struct(&png_ptr,&info_ptr,(png_infopp)NULL); printf("rowbytes!=xres*4 (%d)",tmp); return -1; }

    if (png_get_bit_depth(png_ptr,info_ptr)!=8) { fclose(fp); png_destroy_read_struct(&png_ptr,&info_ptr,(png_infopp)NULL); printf("bit depth!=8\n"); return -1; }
    if (png_get_channels(png_ptr,info_ptr)!=format) { fclose(fp); png_destroy_read_struct(&png_ptr,&info_ptr,(png_infopp)NULL); printf("channels!=format\n"); return -1; }

    // prescan
    sx=xres;
    sy=yres;
    ex=0;
    ey=0;

    for (y=0; y<yres; y++) {
        for (x=0; x<xres; x++) {
            if (format==4 && (row[y][x*4+3]==0 || (row[y][x*4+0]==255 && row[y][x*4+1]==0 && row[y][x*4+2]==255))) continue;
            if (format==3 && ((row[y][x*3+0]==255 && row[y][x*3+1]==0 && row[y][x*3+2]==255))) continue;
            if (x<sx) sx=x;
            if (x>ex) ex=x;
            if (y<sy) sy=y;
            if (y>ey) ey=y;
        }
    }

    if (ex<sx) ex=sx-1;
    if (ey<sy) ey=sy-1;

    // write
    si->flags=1;
    si->xres=ex-sx+1;
    si->yres=ey-sy+1;
    si->xoff=-(xres/2)+sx;
    si->yoff=-(yres/2)+sy;

    si->pixel=malloc(si->xres*si->yres*sizeof(uint32_t));
    mem_png+=si->xres*si->yres*sizeof(uint32_t);

    if (format==4) {
        for (y=0; y<si->yres; y++) {
            for (x=0; x<si->xres; x++) {

                r=row[(sy+y)][(sx+x)*4+0];
                g=row[(sy+y)][(sx+x)*4+1];
                b=row[(sy+y)][(sx+x)*4+2];
                a=row[(sy+y)][(sx+x)*4+3];

                if (r==255 && g==0 && b==255) a=0;

                if (a) {
                    r=min(255,r*255/a);
                    g=min(255,g*255/a);
                    b=min(255,b*255/a);
                } else r=g=b=0;

                si->pixel[x+y*si->xres]=(a<<24)|(b<<16)|(g<<8)|(r);
            }
        }
    } else {
        for (y=0; y<si->yres; y++) {
            for (x=0; x<si->xres; x++) {

                r=row[(sy+y)][(sx+x)*3+0];
                g=row[(sy+y)][(sx+x)*3+1];
                b=row[(sy+y)][(sx+x)*3+2];
                if (r==255 && g==0 && b==255) a=0;
                else a=255;

                si->pixel[x+y*si->xres]=(a<<24)|(b<<16)|(g<<8)|(r);
            }
        }
    }

    png_destroy_read_struct(&png_ptr,&info_ptr,(png_infopp)NULL);
    fclose(fp);

    return 0;
}


int sdl_load_image(struct sdl_image *si,int sprite) {
    char filename[1024];

    if (sprite>MAXSPRITE || sprite<0) {
        note("sdl_load_image: illegal sprite %d wanted",sprite);
        return -1;
    }

    sprintf(filename,"%s%08d/%08d.png",GFXPATH,(sprite/1000)*1000,sprite);
    if (sdl_load_image_png(si,filename)==0) return 0;
    paranoia("%s not found",filename);

    return -1;
}

int sdl_ic_load(int sprite) {

    if (sprite>=MAXSPRITE || sprite<0) {
        note("illegal sprite %d wanted in sdl_ic_load",sprite);
        return -1;
    }
    if (sdli[sprite].flags) return sprite;

    if (sdl_load_image(sdli+sprite,sprite)) return -1;

    return sprite;
}

#define DDFX_LEFTGRID           1       // NOGRID(?) has to be zero, so bzero on the structures default NOGRID(?)
#define DDFX_RIGHTGRID          2

#define DDFX_MAX_FREEZE         8

#define IGET_A(c)       ((((uint32_t)(c))>>24)&0xFF)
#define IGET_R(c)       ((((uint32_t)(c))>>0)&0xFF)
#define IGET_G(c)       ((((uint32_t)(c))>>8)&0xFF)
#define IGET_B(c)       ((((uint32_t)(c))>>16)&0xFF)
#define IRGB(r,g,b)     (((r)<<0)|((g)<<8)|((b)<<16))
#define IRGBA(r,g,b,a)  (((a)<<24)|((r)<<0)|((g)<<8)|((b)<<16))

static inline uint32_t sdl_light(int light,uint32_t irgb) {
    int r,g,b,a;

    r=IGET_R(irgb);
    g=IGET_G(irgb);
    b=IGET_B(irgb);
    a=IGET_A(irgb);

    if (light==0) {
        r=min(255,r*2+4);
        g=min(255,g*2+4);
        b=min(255,b*2+4);
    } else {
        r=r*light/15;
        g=g*light/15;
        b=b*light/15;
    }

    return IRGBA(r,g,b,a);
}

static inline uint32_t sdl_freeze(int freeze,uint32_t irgb) {
    int r,g,b,a;

    r=IGET_R(irgb);
    g=IGET_G(irgb);
    b=IGET_B(irgb);
    a=IGET_A(irgb);

    r=min(255,r+255*freeze/(3*DDFX_MAX_FREEZE-1));
    g=min(255,g+255*freeze/(3*DDFX_MAX_FREEZE-1));
    b=min(255,b+255*3*freeze/(3*DDFX_MAX_FREEZE-1));

    return IRGBA(r,g,b,a);
}


#define REDCOL		(0.40)
#define GREENCOL	(0.70)
#define BLUECOL		(0.70)

#define OGET_R(c) ((((unsigned short int)(c))>>10)&0x1F)
#define OGET_G(c) ((((unsigned short int)(c))>>5)&0x1F)
#define OGET_B(c) ((((unsigned short int)(c))>>0)&0x1F)


static uint32_t sdl_shine_pix(uint32_t irgb,unsigned short shine) {
    int a;
    double r,g,b;

    r=IGET_R(irgb)/127.5;
    g=IGET_G(irgb)/127.5;
    b=IGET_B(irgb)/127.5;
    a=IGET_A(irgb);

    printf("r=%.2f -> ",r);

    r=((r*r*r*r)*shine+r*(100.0-shine))/200.0;
    g=((g*g*g*g)*shine+g*(100.0-shine))/200.0;
    b=((b*b*b*b)*shine+b*(100.0-shine))/200.0;

    printf("%.2f\n",r); fflush(stdout);

    if (r>1.0) r=1.0;
    if (g>1.0) g=1.0;
    if (b>1.0) b=1.0;

    irgb=IRGBA((int)(r*255.0),(int)(g*255.0),(int)(b*255.0),a);

    return irgb;
}

static uint32_t sdl_colorize_pix(uint32_t irgb,unsigned short c1v,unsigned short c2v,unsigned short c3v) {
    double rf,gf,bf,m,rm,gm,bm;
    double c1=0,c2=0,c3=0;
    double shine=0;
    int r,g,b,a;

    rf=IGET_R(irgb)/255.0;
    gf=IGET_G(irgb)/255.0;
    bf=IGET_B(irgb)/255.0;

    m=max(max(rf,gf),bf)+0.000001;
    rm=rf/m; gm=gf/m; bm=bf/m;

    // channel 1: green max
    if (c1v && gm>0.99 && rm<GREENCOL && bm<GREENCOL) {
        c1=gf-max(bf,rf);
        if (c1v&0x8000) shine+=gm-max(rm,bm);

        gf-=c1;
    }

    m=max(max(rf,gf),bf)+0.000001;
    rm=rf/m; gm=gf/m; bm=bf/m;

    // channel 2: blue max
    if (c2v && bm>0.99 && rm<BLUECOL && gm<BLUECOL) {
        c2=bf-max(rf,gf);
        if (c2v&0x8000) shine+=bm-max(rm,gm);

        bf-=c2;
    }

    m=max(max(rf,gf),bf)+0.000001;
    rm=rf/m; gm=gf/m; bm=bf/m;

    // channel 3: red max
    if (c3v && rm>0.99 && gm<REDCOL && bm<REDCOL) {
        c3=rf-max(gf,bf);
        if (c3v&0x8000) shine+=rm-max(gm,bm);

        rf-=c3;
    }

    // sanity
    rf=max(0,rf);
    gf=max(0,gf);
    bf=max(0,bf);

    // collect
    r=min(255,
          8*2*c1*OGET_R(c1v)+
          8*2*c2*OGET_R(c2v)+
          8*2*c3*OGET_R(c3v)+
          8*rf*31);
    g=min(255,
          8*2*c1*OGET_G(c1v)+
          8*2*c2*OGET_G(c2v)+
          8*2*c3*OGET_G(c3v)+
          8*gf*31);
    b=min(255,
          8*2*c1*OGET_B(c1v)+
          8*2*c2*OGET_B(c2v)+
          8*2*c3*OGET_B(c3v)+
          8*bf*31);

    a=IGET_A(irgb);

    irgb=IRGBA(r,g,b,a);

    if (shine>0.1) irgb=sdl_shine_pix(irgb,(int)(shine*50));

    return irgb;
}

static uint32_t sdl_colorbalance(uint32_t irgb,char cr,char cg,char cb,char light,char sat) {
    int r,g,b,a,grey;

    r=IGET_R(irgb);
    g=IGET_G(irgb);
    b=IGET_B(irgb);
    a=IGET_A(irgb);

    // lightness
    if (light) {
        r+=light; g+=light; b+=light;
    }

    // saturation
    if (sat) {
        grey=(r+g+b)/3;
        r=((r*(20-sat))+(grey*sat))/20;
        g=((g*(20-sat))+(grey*sat))/20;
        b=((b*(20-sat))+(grey*sat))/20;
    }

    // color balancing
    cr*=0.75; cg*=0.75; cg*=0.75;

    r+=cr; g-=cr/2; b-=cr/2;
    r-=cg/2; g+=cg; b-=cg/2;
    r-=cb/2; g-=cb/2; b+=cb;

    if (r<0) r=0;
    if (g<0) g=0;
    if (b<0) b=0;

    if (r>255) { g+=(r-255)/2; b+=(r-255)/2; r=255; }
    if (g>255) { r+=(g-255)/2; b+=(g-255)/2; g=255; }
    if (b>255) { r+=(b-255)/2; g+=(b-255)/2; b=255; }

    if (r>255) r=255;
    if (g>255) g=255;
    if (b>255) b=255;

    irgb=IRGBA(r,g,b,a);

    return irgb;
}

static void sdl_make(struct sdl_texture *st,struct sdl_image *si,
                     signed char sink,unsigned char freeze,unsigned char grid,
                     unsigned char scale,char cr,char cg,char cb,
                     char light,char sat,
                     unsigned short c1v,unsigned short c2v,unsigned short c3v,
                     unsigned short shine,
                     char ml,char ll,char rl,char ul,char dl) {
    int x,y;
    double ix,iy,low_x,low_y,high_x,high_y,dbr,dbg,dbb,dba;
    uint32_t irgb,*sptr;
    uint32_t *pixel;

    if (si->xres==0 || si->yres==0) scale=100;    // !!! needs better handling !!!

    if (scale!=100) {
        st->xres=ceil((double)(si->xres-1)*scale/100.0);
        st->yres=ceil((double)(si->yres-1)*scale/100.0);
        st->size=st->xres*st->yres;

        st->xoff=floor(si->xoff*scale/100.0+0.5);
        st->yoff=floor(si->yoff*scale/100.0+0.5);
    } else {
        st->xres=si->xres;
        st->yres=si->yres;
        st->size=st->xres*st->yres;
        st->xoff=si->xoff;
        st->yoff=si->yoff;
    }

    if (sink) sink=min(sink,max(0,st->yres-4));

    pixel=calloc(st->xres*st->yres,sizeof(uint32_t));

    for (y=0; y<st->yres; y++) {
        for (x=0; x<st->xres; x++) {

            if (scale!=100) {
                ix=x*100.0/scale;
                iy=y*100.0/scale;

                high_x=ix-floor(ix);
                high_y=iy-floor(iy);
                low_x=1-high_x;
                low_y=1-high_y;

                irgb=si->pixel[(int)(floor(ix)+floor(iy)*si->xres)];

                if (c1v || c2v || c3v) irgb=sdl_colorize_pix(irgb,c1v,c2v,c3v);
                dba=IGET_A(irgb)*low_x*low_y;
                dbr=IGET_R(irgb)*low_x*low_y;
                dbg=IGET_G(irgb)*low_x*low_y;
                dbb=IGET_B(irgb)*low_x*low_y;

                irgb=si->pixel[(int)(ceil(ix)+floor(iy)*si->xres)];

                if (c1v || c2v || c3v) irgb=sdl_colorize_pix(irgb,c1v,c2v,c3v);
                dba+=IGET_A(irgb)*high_x*low_y;
                dbr+=IGET_R(irgb)*high_x*low_y;
                dbg+=IGET_G(irgb)*high_x*low_y;
                dbb+=IGET_B(irgb)*high_x*low_y;

                irgb=si->pixel[(int)(floor(ix)+ceil(iy)*si->xres)];

                if (c1v || c2v || c3v) irgb=sdl_colorize_pix(irgb,c1v,c2v,c3v);
                dba+=IGET_A(irgb)*low_x*high_y;
                dbr+=IGET_R(irgb)*low_x*high_y;
                dbg+=IGET_G(irgb)*low_x*high_y;
                dbb+=IGET_B(irgb)*low_x*high_y;

                irgb=si->pixel[(int)(ceil(ix)+ceil(iy)*si->xres)];

                if (c1v || c2v || c3v) irgb=sdl_colorize_pix(irgb,c1v,c2v,c3v);
                dba+=IGET_A(irgb)*high_x*high_y;
                dbr+=IGET_R(irgb)*high_x*high_y;
                dbg+=IGET_G(irgb)*high_x*high_y;
                dbb+=IGET_B(irgb)*high_x*high_y;

                irgb=IRGBA(((int)dbr),((int)dbg),((int)dbb),((int)dba));

            } else {
                irgb=si->pixel[x+y*si->xres];
                if (c1v || c2v || c3v) irgb=sdl_colorize_pix(irgb,c1v,c2v,c3v);
            }

            if (cr || cg || cb || light || sat) irgb=sdl_colorbalance(irgb,cr,cg,cb,light,sat);
            if (shine) irgb=sdl_shine_pix(irgb,shine);

            sptr=&pixel[x+y*st->xres];

            if (ll!=ml || rl!=ml || ul!=ml || dl!=ml) {
                int r,g,b,a;
                int r1=0,r2=0,r3=0,r4=0,r5=0;
                int g1=0,g2=0,g3=0,g4=0,g5=0;
                int b1=0,b2=0,b3=0,b4=0,b5=0;
                int v1,v2,v3,v4,v5=0;
                int div;

                // TODO: Tilesizes are hardcoded, with 10px being 1/4th, 20px 1/2.
                if (y<10+(20-abs(20-x))/2) {
                    if (x/2<20-y) {
                        v2=-(x/2-(20-y))+1;
                        r2=IGET_R(sdl_light(ll,irgb));
                        g2=IGET_G(sdl_light(ll,irgb));
                        b2=IGET_B(sdl_light(ll,irgb));
                    } else v2=0;
                    if (x/2>20-y) {
                        v3=(x/2-(20-y))+1;
                        r3=IGET_R(sdl_light(rl,irgb));
                        g3=IGET_G(sdl_light(rl,irgb));
                        b3=IGET_B(sdl_light(rl,irgb));
                    } else v3=0;
                    if (x/2>y) {
                        v4=(x/2-y)+1;
                        r4=IGET_R(sdl_light(ul,irgb));
                        g4=IGET_G(sdl_light(ul,irgb));
                        b4=IGET_B(sdl_light(ul,irgb));
                    } else v4=0;
                    if (x/2<y) {
                        v5=-(x/2-y)+1;
                        r5=IGET_R(sdl_light(dl,irgb));
                        g5=IGET_G(sdl_light(dl,irgb));
                        b5=IGET_B(sdl_light(dl,irgb));
                    } else v5=0;
                } else {
                    if (x<10) {
                        v2=(10-x)*2-2;
                        r2=IGET_R(sdl_light(ll,irgb));
                        g2=IGET_G(sdl_light(ll,irgb));
                        b2=IGET_B(sdl_light(ll,irgb));
                    } else v2=0;
                    if (x>10 && x<20) {
                        v3=(x-10)*2-2;
                        r3=IGET_R(sdl_light(rl,irgb));
                        g3=IGET_G(sdl_light(rl,irgb));
                        b3=IGET_B(sdl_light(rl,irgb));
                    } else v3=0;
                    if (x>20 && x<30) {
                        v5=(10-(x-20))*2-2;
                        r5=IGET_R(sdl_light(dl,irgb));
                        g5=IGET_G(sdl_light(dl,irgb));
                        b5=IGET_B(sdl_light(dl,irgb));
                    } else v5=0;
                    if (x>30 && x<40) {
                        v4=(x-30)*2-2;
                        r4=IGET_R(sdl_light(ul,irgb));
                        g4=IGET_G(sdl_light(ul,irgb));
                        b4=IGET_B(sdl_light(ul,irgb));
                    } else v4=0;
                }

                v1=20-(v2+v3+v4+v5)/2;
                r1=IGET_R(sdl_light(ml,irgb));
                g1=IGET_G(sdl_light(ml,irgb));
                b1=IGET_B(sdl_light(ml,irgb));

                div=v1+v2+v3+v4+v5;

                a=IGET_A(irgb);
                r=(r1*v1+r2*v2+r3*v3+r4*v4+r5*v5)/div;
                g=(g1*v1+g2*v2+g3*v3+g4*v4+g5*v5)/div;
                b=(b1*v1+b2*v2+b3*v3+b4*v4+b5*v5)/div;

                irgb=IRGBA(r,g,b,a);

            } else irgb=sdl_light(ml,irgb);

            if (sink) {
                if (st->yres-sink<y) irgb&=0xffffff;    // zero alpha to make it transparent
            }

            if (freeze) irgb=sdl_freeze(freeze,irgb);

            if (grid==DDFX_LEFTGRID) { if ((st->xoff+x+st->yoff+y)&1) irgb&=0xffffff; }
            if (grid==DDFX_RIGHTGRID) {  if ((st->xoff+x+st->yoff+y+1)&1) irgb&=0xffffff; }

            *sptr=irgb;
        }
    }
    SDL_Texture *texture = SDL_CreateTexture(sdlren,SDL_PIXELFORMAT_RGBA32,SDL_TEXTUREACCESS_STATIC,st->xres,st->yres);
    if (!texture) printf("SDL_texture Error: %s",SDL_GetError());
    SDL_UpdateTexture(texture,NULL,pixel,st->xres*sizeof(uint32_t));
    SDL_SetTextureBlendMode(texture,SDL_BLENDMODE_BLEND);

    mem_tex+=st->xres*st->yres*sizeof(uint32_t);
    st->tex=texture;
}

static void sdl_tx_best(int stx) {
    PARANOIA(if (stx==STX_NONE) paranoia("sdl_tx_best(): sidx=SIDX_NONE"); )
    PARANOIA(if (stx>=MAX_TEXCACHE) paranoia("sdl_tx_best(): sidx>max_systemcache (%d>=%d)",stx,MAX_TEXCACHE); )

    if (sdlt[stx].prev==STX_NONE) {

        PARANOIA(if (stx!=sdlt_best) paranoia("sdl_tx_best(): stx should be best"); )

        return;
    } else if (sdlt[stx].next==STX_NONE) {

        PARANOIA(if (stx!=sdlt_last) paranoia("sdl_tx_best(): sidx should be last"); )

        sdlt_last=sdlt[stx].prev;
        sdlt[sdlt_last].next=STX_NONE;
        sdlt[sdlt_best].prev=stx;
        sdlt[stx].prev=STX_NONE;
        sdlt[stx].next=sdlt_best;
        sdlt_best=stx;

        return;
    } else {
        sdlt[sdlt[stx].prev].next=sdlt[stx].next;
        sdlt[sdlt[stx].next].prev=sdlt[stx].prev;
        sdlt[stx].prev=STX_NONE;
        sdlt[stx].next=sdlt_best;
        sdlt[sdlt_best].prev=stx;
        sdlt_best=stx;
        return;
    }
}

static inline int hashfunc(int sprite,int ml,int ll,int rl,int ul,int dl) {
    int hash;

    hash=sprite^(ml<<2)^(ll<<4)^(ll<<6)^(rl<<8)^(ul<<10)^(dl<<12);

    return hash%MAX_TEXHASH;
}

int sdl_tx_load(int sprite,int sink,int freeze,int grid,int scale,int cr,int cg,int cb,int light,int sat,int c1,int c2,int c3,int shine,int ml,int ll,int rl,int ul,int dl) {
    int stx,ptx,ntx,panic=0;
    int hash=hashfunc(sprite,ml,ll,rl,ul,dl);

    if (sprite>=MAXSPRITE || sprite<0) {
        note("illegal sprite %d wanted in sdl_tx_load",sprite);
        return -1;
    }

    for (stx=sdlt_cache[hash]; stx!=STX_NONE; stx=sdlt[stx].hnext,panic++) {

        if (panic>999) {
            printf("%04d: stx=%d, hprev=%d, hnext=%d sprite=%d (%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d), PANIC\n",panic,stx,sdlt[stx].hprev,sdlt[stx].hnext,sprite,
                   sdlt[stx].sink,sdlt[stx].freeze,sdlt[stx].grid,sdlt[stx].scale,
                   sdlt[stx].cr,sdlt[stx].cg,sdlt[stx].cb,sdlt[stx].light,
                   sdlt[stx].sat,sdlt[stx].c1,sdlt[stx].c2,sdlt[stx].c3,
                   sdlt[stx].shine,sdlt[stx].ml,sdlt[stx].ll,sdlt[stx].rl,
                   sdlt[stx].ul,sdlt[stx].dl);
            fflush(stdout);
            if (panic>1099) exit(42);
        }
        if (sdlt[stx].sprite!=sprite) continue;
        if (sdlt[stx].sink!=sink) continue;
        if (sdlt[stx].freeze!=freeze) continue;
        if (sdlt[stx].grid!=grid) continue;
        if (sdlt[stx].scale!=scale) continue;
        if (sdlt[stx].cr!=cr) continue;
        if (sdlt[stx].cg!=cg) continue;
        if (sdlt[stx].cb!=cb) continue;
        if (sdlt[stx].light!=light) continue;
        if (sdlt[stx].sat!=sat) continue;
        if (sdlt[stx].c1!=c1) continue;
        if (sdlt[stx].c2!=c2) continue;
        if (sdlt[stx].c3!=c3) continue;
        if (sdlt[stx].shine!=shine) continue;
        if (sdlt[stx].ml!=ml) continue;
        if (sdlt[stx].ll!=ll) continue;
        if (sdlt[stx].rl!=rl) continue;
        if (sdlt[stx].ul!=ul) continue;
        if (sdlt[stx].dl!=dl) continue;

        if (panic>maxpanic) maxpanic=panic;

        sdl_tx_best(stx);

        // remove from old pos
        ntx=sdlt[stx].hnext;
        ptx=sdlt[stx].hprev;

        if (ptx==STX_NONE) sdlt_cache[hash]=ntx;
        else sdlt[ptx].hnext=sdlt[stx].hnext;

        if (ntx!=STX_NONE) sdlt[ntx].hprev=sdlt[stx].hprev;

        // add to top pos
        ntx=sdlt_cache[hash];

        if (ntx!=STX_NONE) sdlt[ntx].hprev=stx;

        sdlt[stx].hprev=STX_NONE;
        sdlt[stx].hnext=ntx;

        sdlt_cache[hash]=stx;

        texc_hit++;

        return stx;
    }

    stx=sdlt_last;

    // delete
    if (sdlt[stx].flags) {
        int hash2=hashfunc(sdlt[stx].sprite,sdlt[stx].ml,sdlt[stx].ll,sdlt[stx].rl,sdlt[stx].ul,sdlt[stx].dl);

        ntx=sdlt[stx].hnext;
        ptx=sdlt[stx].hprev;

        if (ptx==STX_NONE) {
            if (sdlt_cache[hash2]!=stx) {
                printf("sdli[sprite].stx!=stx\n");
                exit(42);
            }
            sdlt_cache[hash2]=ntx;
        } else {
            sdlt[ptx].hnext=sdlt[stx].hnext;
        }

        if (ntx!=STX_NONE) {
            sdlt[ntx].hprev=sdlt[stx].hprev;
        }

        mem_tex-=sdlt[stx].xres*sdlt[stx].yres*sizeof(uint32_t);
        SDL_DestroyTexture(sdlt[stx].tex);
        sdlt[stx].flags=0;
    }

    // build
    sdl_ic_load(sprite);

    sdl_make(sdlt+stx,sdli+sprite,sink,freeze,grid,scale,cr,cg,cb,light,sat,c1,c2,c3,shine,ml,ll,rl,ul,dl);

    // init
    sdlt[stx].flags=SF_USED;
    sdlt[stx].sprite=sprite;
    sdlt[stx].sink=sink;
    sdlt[stx].freeze=freeze;
    sdlt[stx].grid=grid;
    sdlt[stx].scale=scale;
    sdlt[stx].cr=cr;
    sdlt[stx].cg=cg;
    sdlt[stx].cb=cb;
    sdlt[stx].light=light;
    sdlt[stx].sat=sat;
    sdlt[stx].c1=c1;
    sdlt[stx].c2=c2;
    sdlt[stx].c3=c3;
    sdlt[stx].shine=shine;
    sdlt[stx].ml=ml;
    sdlt[stx].ll=ll;
    sdlt[stx].rl=rl;
    sdlt[stx].ul=ul;
    sdlt[stx].dl=dl;

    ntx=sdlt_cache[hash];

    if (ntx!=STX_NONE) sdlt[ntx].hprev=stx;

    sdlt[stx].hprev=STX_NONE;
    sdlt[stx].hnext=ntx;

    sdlt_cache[hash]=stx;

    sdl_tx_best(stx);

    texc_miss++;

    return stx;
}

static void sdl_blit_tex(SDL_Texture *tex,int sx,int sy,int clipsx,int clipsy,int clipex,int clipey,int x_offset,int y_offset) {
    int addx=0,addy=0,dx,dy;
    SDL_Rect dr,sr;

    SDL_QueryTexture(tex, NULL, NULL, &dx, &dy);

    if (sx<clipsx) { addx=clipsx-sx; dx-=addx; sx=clipsx; }
    if (sy<clipsy) { addy=clipsy-sy; dy-=addy; sy=clipsy; }
    if (sx+dx>=clipex) dx=clipex-sx;
    if (sy+dy>=clipey) dy=clipey-sy;

    dr.x=sx+x_offset; dr.w=dx;
    dr.y=sy+y_offset; dr.h=dy;

    sr.x=addx; sr.w=dx;
    sr.y=addy; sr.h=dy;

    SDL_RenderCopy(sdlren,tex,&sr,&dr);
}


void sdl_blit(int stx,int sx,int sy,int clipsx,int clipsy,int clipex,int clipey,int x_offset,int y_offset) {
    sdl_blit_tex(sdlt[stx].tex,sx,sy,clipsx,clipsy,clipex,clipey,x_offset,y_offset);
}

#define MAXFONTHEIGHT   36

SDL_Texture *sdl_maketext(const char *text,struct ddfont *font,uint32_t color,int flags) {
    uint32_t *pixel,*dst;
    unsigned char *rawrun;
    int x,y=0,sizex,sizey=0,sx=0;
    const char *c;

    for (sizex=0,c=text; *c; c++) sizex+=font[*c].dim;

    pixel=calloc(sizex*MAXFONTHEIGHT,sizeof(uint32_t));
    if (pixel==NULL) return NULL;

    while (*text && *text!=DDT) {

        if (*text<0) { note("PANIC: char over limit"); text++; continue; }

        rawrun=font[*text].raw;

        x=sx;
        y=0;

        dst=pixel+x+y*sizex;

        while (*rawrun!=255) {

            if (*rawrun==254) {
                y++;
                x=sx;
                rawrun++;
                dst=pixel+x+y*sizex;
                if (y>sizey) sizey=y;
                continue;
            }

            dst+=*rawrun;
            x+=*rawrun;

            rawrun++;
            *dst=color;
        }
        sx+=font[*text++].dim;
    }

    if (sizex<1 || sizey<1) return NULL;

    sizey++;
    SDL_Texture *texture = SDL_CreateTexture(sdlren,SDL_PIXELFORMAT_RGBA32,SDL_TEXTUREACCESS_STATIC,sizex,sizey);
    if (!texture) printf("SDL_texture Error: %s",SDL_GetError());
    SDL_UpdateTexture(texture,NULL,pixel,sizex*sizeof(uint32_t));
    SDL_SetTextureBlendMode(texture,SDL_BLENDMODE_BLEND);

    return texture;
}

#define DD_LEFT         0
#define DD_CENTER       1
#define DD_RIGHT        2
#define DD_SHADE        4
#define DD_LARGE        0
#define DD_SMALL        8
#define DD_FRAME        16
#define DD_BIG        	32

#define DD__SHADEFONT	128
#define DD__FRAMEFONT	256

int sdl_drawtext(int sx,int sy,unsigned short int color,int flags,const char *text,struct ddfont *font,int clipsx,int clipsy,int clipex,int clipey,int x_offset,int y_offset) {
    int dx,dy;
    SDL_Texture *tex;
    int r,g,b,a;

    r=(int)((((color>>11)&31)/31.0f)*255.0f);
    g=(int)((((color>>5) &63)/63.0f)*255.0f);
    b=(int)((((color)    &31)/31.0f)*255.0f);
    a=255;

    tex=sdl_maketext(text,font,IRGBA(r,g,b,a),flags);

    SDL_QueryTexture(tex, NULL, NULL, &dx, &dy);

    if (flags&DD_CENTER) sx-=dx/2;
    else if (flags&DD_RIGHT) sx-=dx;

    sdl_blit_tex(tex,sx,sy,clipsx,clipsy,clipex,clipey,x_offset,y_offset);

    SDL_DestroyTexture(tex);

    return 1;
}

void sdl_rect(int sx,int sy,int ex,int ey,unsigned short int color,int clipsx,int clipsy,int clipex,int clipey,int x_offset,int y_offset) {
    int r,g,b,a;
    SDL_Rect rc;

    r=(int)((((color>>11)&31)/31.0f)*255.0f);
    g=(int)((((color>>5) &63)/63.0f)*255.0f);
    b=(int)((((color)    &31)/31.0f)*255.0f);
    a=255;

    if (sx<clipsx) sx=clipsx;
    if (sy<clipsy) sy=clipsy;
    if (ex>clipex) ex=clipex;
    if (ey>clipey) ey=clipey;

    if (sx>ex || sy>ey) return;

    rc.x=sx+x_offset; rc.w=ex-sx;
    rc.y=sy+y_offset; rc.h=ey-sy;

    SDL_SetRenderDrawColor(sdlren,r,g,b,a);
    SDL_RenderFillRect(sdlren,&rc);
}


void sdl_pixel(int x,int y,unsigned short color,int x_offset,int y_offset) {
    int r,g,b,a;

    r=(int)((((color>>11)&31)/31.0f)*255.0f);
    g=(int)((((color>>5) &63)/63.0f)*255.0f);
    b=(int)((((color)    &31)/31.0f)*255.0f);
    a=255;

    SDL_SetRenderDrawColor(sdlren,r,g,b,a);
    SDL_RenderDrawPoint(sdlren,x+x_offset,y+y_offset);
}


void sdl_line(int fx,int fy,int tx,int ty,unsigned short color,int clipsx,int clipsy,int clipex,int clipey,int x_offset,int y_offset) {
    int r,g,b,a;

    r=(int)((((color>>11)&31)/31.0f)*255.0f);
    g=(int)((((color>>5) &63)/63.0f)*255.0f);
    b=(int)((((color)    &31)/31.0f)*255.0f);
    a=255;

    if (fx<clipsx) fx=clipsx;
    if (fy<clipsy) fy=clipsy;
    if (fx>=clipex) fx=clipex-1;
    if (fy>=clipey) fy=clipey-1;

    if (tx<clipsx) tx=clipsx;
    if (ty<clipsy) ty=clipsy;
    if (tx>=clipex) tx=clipex-1;
    if (ty>=clipey) ty=clipey-1;

    fx+=x_offset; tx+=x_offset;
    fy+=y_offset; ty+=y_offset;

    SDL_SetRenderDrawColor(sdlren,r,g,b,a);
    SDL_RenderDrawLine(sdlren,fx,fy,tx,ty);
}


