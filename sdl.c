/*
 * Part of Astonia Client (c) Daniel Brockhaus. Please read license.txt.
 */

#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <png.h>
#include <zip.h>

#include "engine.h"

#include "main.h"
#include "sdl.h"
#include "sound.h"

#define max(a,b)    ((a)>(b)?(a):(b))
#define min(a,b)    ((a)<(b)?(a):(b))

#define IGET_A(c)       ((((uint32_t)(c))>>24)&0xFF)
#define IGET_R(c)       ((((uint32_t)(c))>>16)&0xFF)
#define IGET_G(c)       ((((uint32_t)(c))>>8)&0xFF)
#define IGET_B(c)       ((((uint32_t)(c))>>0)&0xFF)
#define IRGB(r,g,b)     (((r)<<0)|((g)<<8)|((b)<<16))
#define IRGBA(r,g,b,a)  (((a)<<24)|((r)<<16)|((g)<<8)|((b)<<0))

SDL_Window *sdlwnd;
SDL_Renderer *sdlren;

#if 1
#define MAX_TEXCACHE    4000
#define MAX_TEXHASH     2500
#else
#define MAX_TEXCACHE    15000
#define MAX_TEXHASH     10000
#endif

#define STX_NONE        (-1)

#define SF_USED         (1<<0)
#define SF_SPRITE       (1<<1)
#define SF_TEXT         (1<<2)
#define SF_DIDALLOC     (1<<3)
#define SF_DIDMAKE      (1<<4)
#define SF_DIDTEX       (1<<5)
#define SF_BUSY         (1<<6)

struct sdl_texture {
    SDL_Texture *tex;
    uint32_t *pixel;

    int prev,next;
    int hprev,hnext;

    uint16_t flags;

    // ---------- sprites ------------
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

    // ---------- text --------------
    uint16_t text_flags;
    uint32_t text_color;
    char *text;
    void *text_font;
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

SDL_Cursor *curs[20];

struct sdl_image *sdli=NULL;

long long mem_png=0,mem_tex=0;
long long texc_hit=0,texc_miss=0,texc_pre=0;

long long sdl_time_preload=0;
long long sdl_time_make=0;
long long sdl_time_make_main=0;
long long sdl_time_load=0;
long long sdl_time_alloc=0;
long long sdl_time_tex=0;
long long sdl_time_tex_main=0;
long long sdl_time_text=0;
long long sdl_time_blit=0;
long long sdl_time_pre2=0;
long long sdl_time_pre3=0;

int sdl_scale=1;
int sdl_frames=0;
int sdl_multi=4;

zip_t *sdl_zip1=NULL;
zip_t *sdl_zip2=NULL;

int sdl_pre_backgnd(void *ptr);
int sdl_create_cursors(void);

SDL_Texture *sdltgt;

SDL_sem *prework=NULL;
SDL_mutex *premutex=NULL;

int sdl_init(int width,int height,char *title) {
    extern float mouse_scale;
    int len,i;
    SDL_DisplayMode DM;

    if (SDL_Init(SDL_INIT_VIDEO|(enable_sound?SDL_INIT_AUDIO:0)) != 0){
        fail("SDL_Init Error: %s",SDL_GetError());
	    return 0;
    }

    SDL_GetCurrentDisplayMode(0, &DM);
    sdlwnd = SDL_CreateWindow(title, DM.w/2-width/2, DM.h/2-height/2, width, height, SDL_WINDOW_SHOWN);
    if (!sdlwnd) {
        fail("SDL_Init Error: %s",SDL_GetError());
        SDL_Quit();
	    return 0;
    }

    if (DM.w==width && DM.h==height) {
        //SDL_SetWindowFullscreen(sdlwnd,SDL_WINDOW_FULLSCREEN);  // true full screen
        SDL_SetWindowFullscreen(sdlwnd,SDL_WINDOW_FULLSCREEN_DESKTOP); // borderless windowed
    }

    sdlren=SDL_CreateRenderer(sdlwnd, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!sdlren){
        SDL_DestroyWindow(sdlwnd);
        fail("SDL_Init Error: %s",SDL_GetError());
        SDL_Quit();
        return 0;
    }

    len=sizeof(struct sdl_image)*MAXSPRITE;
    sdli=xcalloc(len*1,MEM_SDL_BASE);
    note("SDL Image cache needs %.2fM for image cache index",len/(1024.0*1024.0));
    if (!sdli) return fail("Out of memory in sdl_init");

    sdlt_cache=xcalloc(MAX_TEXHASH*sizeof(int),MEM_SDL_BASE);
    note("SDL texture cache needs %.2fM for cache index",MAX_TEXHASH*sizeof(int)/(1024.0*1024.0));
    if (!sdlt_cache) return fail("Out of memory in sdl_init");

    for (i=0; i<MAX_TEXHASH; i++)
        sdlt_cache[i]=STX_NONE;

    sdlt=xcalloc(MAX_TEXCACHE*sizeof(struct sdl_texture),MEM_SDL_BASE);
    note("SDL texture cache needs %.2fM for cache",MAX_TEXCACHE*sizeof(struct sdl_texture)/(1024.0*1024.0));
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

    SDL_RaiseWindow(sdlwnd);

    // We want SDL to translate scan codes to ASCII / Unicode
    // but we don't really want the SDL line editing stuff.
    // I hope just keeping it enabled all the time doesn't break
    // anything.
    SDL_StartTextInput();

    sdl_create_cursors();

    // decide on screen format
    if (width!=XRES) {
        extern int x_offset,y_offset;

        if (width/XRES>=4 && height/YRES>=4) sdl_scale=4;
        else if (width/XRES>=3 && height/YRES>=3) sdl_scale=3;
        else if (width/XRES>=2 && height/YRES>=2) sdl_scale=2;

        mouse_scale=sdl_scale;

        x_offset=(width/sdl_scale-XRES)/2;
        y_offset=(height/sdl_scale-YRES)/2;
    }

    sdl_zip1=zip_open("gx1.zip",ZIP_RDONLY,NULL);
    switch (sdl_scale) {
        case 2:     sdl_zip2=zip_open("gx2.zip",ZIP_RDONLY,NULL);
        case 3:     sdl_zip2=zip_open("gx3.zip",ZIP_RDONLY,NULL);
        case 4:     sdl_zip2=zip_open("gx4.zip",ZIP_RDONLY,NULL);
    }


    if (enable_sound && Mix_OpenAudio(44100,MIX_DEFAULT_FORMAT,2,2048)<0) {
        warn("initializing audio failed");
        enable_sound=0;
    }

    if (enable_sound) {
        int number_of_sound_channels = Mix_AllocateChannels(MAX_SOUND_CHANNELS);
        note("Allocated %d sound channels", number_of_sound_channels);
    }

    if (sdl_multi) {
        char buf[80];
        int n;

        prework=SDL_CreateSemaphore(0);
        premutex=SDL_CreateMutex();

        for (n=0; n<sdl_multi; n++) {
            sprintf(buf,"moac background worker %d",n);
            SDL_CreateThread(sdl_pre_backgnd,buf,(void *)(long long)n);
        }
    }

    return 1;
}

int maxpanic=0;

int sdl_clear(void) {
    SDL_SetRenderDrawColor(sdlren,0,0,0,255);
    SDL_RenderClear(sdlren);
    //note("mem: %.2fM PNG, %.2fM Tex, Hit: %ld, Miss: %ld, Max: %d\n",mem_png/(1024.0*1024.0),mem_tex/(1024.0*1024.0),texc_hit,texc_miss,maxpanic);
    maxpanic=0;
    return 1;
}

int sdl_render(void) {
    SDL_RenderPresent(sdlren);
    sdl_frames++;
    return 1;
}

uint32_t mix_argb(uint32_t c1,uint32_t c2,float w1,float w2) {
    int r1,r2,g1,g2,b1,b2,a1,a2;
    int r,g,b,a;

    a1=IGET_A(c1);
    a2=IGET_A(c2);
    if (!a1 && !a2) return 0; // save some work

    r1=IGET_R(c1);
    g1=IGET_G(c1);
    b1=IGET_B(c1);

    r2=IGET_R(c2);
    g2=IGET_G(c2);
    b2=IGET_B(c2);

    a=(a1*w1+a2*w2);
    r=(r1*w1+r2*w2);
    g=(g1*w1+g2*w2);
    b=(b1*w1+b2*w2);

    a=min(255,a);
    r=min(255,r);
    g=min(255,g);
    b=min(255,b);

    return IRGBA(r,g,b,a);
}

void sdl_smoothify(uint32_t *pixel,int xres,int yres,int scale) {
    int x,y;
    uint32_t c1,c2,c3,c4;

    switch (scale) {
        case 2:
            for (x=0; x<xres-2; x+=2) {
                for (y=0; y<yres-2; y+=2) {
                    c1=pixel[x+y*xres];             // top left
                    c2=pixel[x+y*xres+2];           // top right
                    c3=pixel[x+y*xres+xres*2];      // bottom left
                    c4=pixel[x+y*xres+2+xres*2];    // bottom right

                    pixel[x+y*xres+1]=mix_argb(c1,c2,0.5,0.5);
                    pixel[x+y*xres+xres]=mix_argb(c1,c3,0.5,0.5);
                    pixel[x+y*xres+1+xres]=mix_argb(mix_argb(c1,c2,0.5,0.5),mix_argb(c3,c4,0.5,0.5),0.5,0.5);
                }
            }
            break;
        case 3:
            for (x=0; x<xres-3; x+=3) {
                for (y=0; y<yres-3; y+=3) {
                    c1=pixel[x+y*xres];             // top left
                    c2=pixel[x+y*xres+3];           // top right
                    c3=pixel[x+y*xres+xres*3];      // bottom left
                    c4=pixel[x+y*xres+3+xres*3];    // bottom right

                    pixel[x+y*xres+1]=mix_argb(c1,c2,0.667,0.333);
                    pixel[x+y*xres+2]=mix_argb(c1,c2,0.333,0.667);

                    pixel[x+y*xres+xres*1]=mix_argb(c1,c3,0.667,0.333);
                    pixel[x+y*xres+xres*2]=mix_argb(c1,c3,0.333,0.667);

                    pixel[x+y*xres+1+xres*1]=mix_argb(mix_argb(c1,c2,0.5,0.5),mix_argb(c3,c4,0.5,0.5),0.5,0.5);
                    pixel[x+y*xres+2+xres*1]=mix_argb(mix_argb(c1,c2,0.333,0.667),mix_argb(c3,c4,0.333,0.667),0.667,0.333);
                    pixel[x+y*xres+1+xres*2]=mix_argb(mix_argb(c1,c2,0.667,0.333),mix_argb(c3,c4,0.667,0.333),0.333,0.667);
                    pixel[x+y*xres+2+xres*2]=mix_argb(mix_argb(c1,c2,0.333,0.667),mix_argb(c3,c4,0.333,0.667),0.333,0.667);
                }
            }
            break;

        case 4:
            for (x=0; x<xres-4; x+=4) {
                for (y=0; y<yres-4; y+=4) {
                    c1=pixel[x+y*xres];             // top left
                    c2=pixel[x+y*xres+4];           // top right
                    c3=pixel[x+y*xres+xres*4];      // bottom left
                    c4=pixel[x+y*xres+4+xres*4];    // bottom right

                    pixel[x+y*xres+1]=mix_argb(c1,c2,0.75,0.25);
                    pixel[x+y*xres+2]=mix_argb(c1,c2,0.50,0.50);
                    pixel[x+y*xres+3]=mix_argb(c1,c2,0.25,0.75);

                    pixel[x+y*xres+xres*1]=mix_argb(c1,c3,0.75,0.25);
                    pixel[x+y*xres+xres*2]=mix_argb(c1,c3,0.50,0.50);
                    pixel[x+y*xres+xres*3]=mix_argb(c1,c3,0.25,0.75);

                    pixel[x+y*xres+1+xres*1]=mix_argb(mix_argb(c1,c2,0.75,0.25),mix_argb(c3,c4,0.75,0.25),0.75,0.25);
                    pixel[x+y*xres+1+xres*2]=mix_argb(mix_argb(c1,c2,0.75,0.25),mix_argb(c3,c4,0.75,0.25),0.50,0.50);
                    pixel[x+y*xres+1+xres*3]=mix_argb(mix_argb(c1,c2,0.75,0.75),mix_argb(c3,c4,0.75,0.25),0.25,0.75);

                    pixel[x+y*xres+2+xres*1]=mix_argb(mix_argb(c1,c2,0.50,0.50),mix_argb(c3,c4,0.50,0.50),0.75,0.25);
                    pixel[x+y*xres+2+xres*2]=mix_argb(mix_argb(c1,c2,0.50,0.50),mix_argb(c3,c4,0.50,0.50),0.50,0.50);
                    pixel[x+y*xres+2+xres*3]=mix_argb(mix_argb(c1,c2,0.50,0.50),mix_argb(c3,c4,0.50,0.50),0.25,0.75);

                    pixel[x+y*xres+3+xres*1]=mix_argb(mix_argb(c1,c2,0.25,0.75),mix_argb(c3,c4,0.25,0.75),0.75,0.25);
                    pixel[x+y*xres+3+xres*2]=mix_argb(mix_argb(c1,c2,0.25,0.75),mix_argb(c3,c4,0.25,0.75),0.50,0.50);
                    pixel[x+y*xres+3+xres*3]=mix_argb(mix_argb(c1,c2,0.25,0.75),mix_argb(c3,c4,0.25,0.75),0.25,0.75);
                }
            }
            break;
        default:
            warn("Unsupported scale %d in sdl_load_image_png()",sdl_scale);
            break;
    }
}

void sdl_premulti(uint32_t *pixel,int xres,int yres,int scale) {
    int n,r,g,b,a;
    uint32_t c;

    for (n=0; n<xres*yres; n++) {
        c=pixel[n];

        a=IGET_A(c);
        if (!a) continue;

        r=IGET_R(c);
        g=IGET_G(c);
        b=IGET_B(c);

        r=min(255,r*255/a);
        g=min(255,g*255/a);
        b=min(255,b*255/a);

        c=IRGBA(r,g,b,a);
        pixel[n]=c;
    }
}

struct png_helper {
    char *filename;
    zip_t *zip;
    unsigned char **row;
    int xres;
    int yres;
    int bpp;

    png_structp png_ptr;
    png_infop info_ptr;
};

void png_helper_read(png_struct *ps,unsigned char *buf,long long unsigned len) {
    zip_fread(png_get_io_ptr(ps),buf,len);
}

int png_load_helper(struct png_helper *p) {
    FILE *fp=NULL;
    zip_file_t *zp=NULL;
    int tmp;

    if (p->zip) {
        zp=zip_fopen(p->zip,p->filename,0);
        if (!zp) return -1;
    } else {
        fp=fopen(p->filename,"rb");
        if (!fp) return -1;
    }

    p->png_ptr=png_create_read_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
    if (!p->png_ptr) { fclose(fp); warn("create read\n"); return -1; }

    p->info_ptr=png_create_info_struct(p->png_ptr);
    if (!p->info_ptr) { fclose(fp); png_destroy_read_struct(&p->png_ptr,(png_infopp)NULL,(png_infopp)NULL); warn("create info1\n"); return -1; }

    if (p->zip) {
        png_set_read_fn(p->png_ptr,zp,png_helper_read);
    } else {
        png_init_io(p->png_ptr,fp);
    }
    png_set_strip_16(p->png_ptr);
    png_read_png(p->png_ptr,p->info_ptr,PNG_TRANSFORM_PACKING,NULL);

    p->row=png_get_rows(p->png_ptr,p->info_ptr);
    if (!p->row) { fclose(fp); png_destroy_read_struct(&p->png_ptr,&p->info_ptr,(png_infopp)NULL); warn("read row\n"); return -1; }

    p->xres=png_get_image_width(p->png_ptr,p->info_ptr);
    p->yres=png_get_image_height(p->png_ptr,p->info_ptr);

    tmp=png_get_rowbytes(p->png_ptr,p->info_ptr);

    if (tmp==p->xres*3) p->bpp=24;
    else if (tmp==p->xres*4) p->bpp=32;
    else { fclose(fp); png_destroy_read_struct(&p->png_ptr,&p->info_ptr,(png_infopp)NULL); warn("rowbytes!=xres*4 (%d, %d, %s)",tmp,p->xres,p->filename); return -1; }

    if (png_get_bit_depth(p->png_ptr,p->info_ptr)!=8) { fclose(fp); png_destroy_read_struct(&p->png_ptr,&p->info_ptr,(png_infopp)NULL); warn("bit depth!=8\n"); return -1; }
    if (png_get_channels(p->png_ptr,p->info_ptr)!=p->bpp/8) { fclose(fp); png_destroy_read_struct(&p->png_ptr,&p->info_ptr,(png_infopp)NULL); warn("channels!=format\n"); return -1; }

    if (p->zip) zip_fclose(zp);
    else fclose(fp);

    return 0;
}

void png_load_helper_exit(struct png_helper *p) {
    png_destroy_read_struct(&p->png_ptr,&p->info_ptr,(png_infopp)NULL);
}

// Load high res PNG
int sdl_load_image_png_(struct sdl_image *si,char *filename,zip_t *zip) {
    int x,y,r,g,b,a,sx,sy,ex,ey;
    uint32_t c;
    struct png_helper p;

    p.zip=zip;
    p.filename=filename;
    if (png_load_helper(&p)) return -1;

    // prescan
    sx=p.xres;
    sy=p.yres;
    ex=0;
    ey=0;

    for (y=0; y<p.yres; y++) {
        for (x=0; x<p.xres; x++) {
            if (p.bpp==32 && (p.row[y][x*4+3]==0 || (p.row[y][x*4+0]==255 && p.row[y][x*4+1]==0 && p.row[y][x*4+2]==255))) continue;
            if (p.bpp==24 && ((p.row[y][x*3+0]==255 && p.row[y][x*3+1]==0 && p.row[y][x*3+2]==255))) continue;
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
    si->xres=((ex-sx+sdl_scale-1)/sdl_scale)*sdl_scale;
    si->yres=((ey-sy+sdl_scale-1)/sdl_scale)*sdl_scale;;
    si->xoff=-(p.xres/2)+sx;
    si->yoff=-(p.yres/2)+sy;

    si->pixel=xmalloc(si->xres*si->yres*sizeof(uint32_t),MEM_SDL_PNG);
    mem_png+=si->xres*si->yres*sizeof(uint32_t);

    for (y=0; y<si->yres; y++) {
        for (x=0; x<si->xres; x++) {

            if (p.bpp==32) {
                r=p.row[(sy+y)][(sx+x)*4+0];
                g=p.row[(sy+y)][(sx+x)*4+1];
                b=p.row[(sy+y)][(sx+x)*4+2];
                a=p.row[(sy+y)][(sx+x)*4+3];
            } else {
                r=p.row[(sy+y)][(sx+x)*3+0];
                g=p.row[(sy+y)][(sx+x)*3+1];
                b=p.row[(sy+y)][(sx+x)*3+2];
                if (r==255 && g==0 && b==255) a=0;
                else a=255;
            }

            if (r==255 && g==0 && b==255) a=0;

            if (a) {    // pre-multiply rgb channel by alpha
                r=min(255,r*255/a);
                g=min(255,g*255/a);
                b=min(255,b*255/a);
            } else r=g=b=0;

            c=IRGBA(r,g,b,a);

            si->pixel[x+y*si->xres]=c;
        }
    }

    png_load_helper_exit(&p);

    si->xres/=sdl_scale;
    si->yres/=sdl_scale;
    si->xoff/=sdl_scale;
    si->yoff/=sdl_scale;

    return 0;
}

// Load and up-scale low res PNG
// TODO: add support for using a 2X image as a base for 4X
// and possibly the other way around too
int sdl_load_image_png(struct sdl_image *si,char *filename,zip_t *zip,int smoothify) {
    int x,y,r,g,b,a,sx,sy,ex,ey;
    uint32_t c;
    struct png_helper p;

    p.zip=zip;
    p.filename=filename;
    if (png_load_helper(&p)) return -1;

    // prescan
    sx=p.xres;
    sy=p.yres;
    ex=0;
    ey=0;

    for (y=0; y<p.yres; y++) {
        for (x=0; x<p.xres; x++) {
            if (p.bpp==32 && (p.row[y][x*4+3]==0 || (p.row[y][x*4+0]==255 && p.row[y][x*4+1]==0 && p.row[y][x*4+2]==255))) continue;
            if (p.bpp==24 && ((p.row[y][x*3+0]==255 && p.row[y][x*3+1]==0 && p.row[y][x*3+2]==255))) continue;
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
    si->xoff=-(p.xres/2)+sx;
    si->yoff=-(p.yres/2)+sy;

    si->pixel=xmalloc(si->xres*si->yres*sizeof(uint32_t)*sdl_scale*sdl_scale,MEM_SDL_PNG);
    mem_png+=si->xres*si->yres*sizeof(uint32_t);

    for (y=0; y<si->yres; y++) {
        for (x=0; x<si->xres; x++) {

            if (p.bpp==32) {
                r=p.row[(sy+y)][(sx+x)*4+0];
                g=p.row[(sy+y)][(sx+x)*4+1];
                b=p.row[(sy+y)][(sx+x)*4+2];
                a=p.row[(sy+y)][(sx+x)*4+3];
            } else {
                r=p.row[(sy+y)][(sx+x)*3+0];
                g=p.row[(sy+y)][(sx+x)*3+1];
                b=p.row[(sy+y)][(sx+x)*3+2];
                if (r==255 && g==0 && b==255) a=0;
                else a=255;
            }

            if (r==255 && g==0 && b==255) a=0;

            if (!a) // don't pre-multiply rgb channel by alpha because that needs to happen after scaling
                r=g=b=0;

            c=IRGBA(r,g,b,a);

            switch (sdl_scale) {
                case 1:
                    si->pixel[x+y*si->xres]=c;
                    break;
                case 2:
                    si->pixel[x*2+y*si->xres*4]=c;
                    si->pixel[x*2+y*si->xres*4+1]=c;
                    si->pixel[x*2+y*si->xres*4+si->xres*2]=c;
                    si->pixel[x*2+y*si->xres*4+1+si->xres*2]=c;
                    break;
                case 3:
                    si->pixel[x*3+y*si->xres*9+0]=c;
                    si->pixel[x*3+y*si->xres*9+0+si->xres*3]=c;
                    si->pixel[x*3+y*si->xres*9+0+si->xres*6]=c;

                    si->pixel[x*3+y*si->xres*9+1]=c;
                    si->pixel[x*3+y*si->xres*9+1+si->xres*3]=c;
                    si->pixel[x*3+y*si->xres*9+1+si->xres*6]=c;

                    si->pixel[x*3+y*si->xres*9+2]=c;
                    si->pixel[x*3+y*si->xres*9+2+si->xres*3]=c;
                    si->pixel[x*3+y*si->xres*9+2+si->xres*6]=c;
                    break;
                case 4:
                    si->pixel[x*4+y*si->xres*16+0]=c;
                    si->pixel[x*4+y*si->xres*16+0+si->xres*4]=c;
                    si->pixel[x*4+y*si->xres*16+0+si->xres*8]=c;
                    si->pixel[x*4+y*si->xres*16+0+si->xres*12]=c;

                    si->pixel[x*4+y*si->xres*16+1]=c;
                    si->pixel[x*4+y*si->xres*16+1+si->xres*4]=c;
                    si->pixel[x*4+y*si->xres*16+1+si->xres*8]=c;
                    si->pixel[x*4+y*si->xres*16+1+si->xres*12]=c;

                    si->pixel[x*4+y*si->xres*16+2]=c;
                    si->pixel[x*4+y*si->xres*16+2+si->xres*4]=c;
                    si->pixel[x*4+y*si->xres*16+2+si->xres*8]=c;
                    si->pixel[x*4+y*si->xres*16+2+si->xres*12]=c;

                    si->pixel[x*4+y*si->xres*16+3]=c;
                    si->pixel[x*4+y*si->xres*16+3+si->xres*4]=c;
                    si->pixel[x*4+y*si->xres*16+3+si->xres*8]=c;
                    si->pixel[x*4+y*si->xres*16+3+si->xres*12]=c;
                    break;
                default:
                    warn("Unsupported scale %d in sdl_load_image_png()",sdl_scale);
                    break;
            }
        }
    }

    if (sdl_scale>1 && smoothify) {
        sdl_smoothify(si->pixel,si->xres*sdl_scale,si->yres*sdl_scale,sdl_scale);
        sdl_premulti(si->pixel,si->xres*sdl_scale,si->yres*sdl_scale,sdl_scale);
    } else sdl_premulti(si->pixel,si->xres*sdl_scale,si->yres*sdl_scale,sdl_scale);

    png_load_helper_exit(&p);

    return 0;
}


int do_smoothify(int sprite) {

    // TODO: add more to this list
    if (sprite<=1000) return 1; // GUI
    if (sprite>=100000) return 1;   // all character sprites

    return 0;
}

int sdl_load_image(struct sdl_image *si,int sprite) {
    char filename[1024];

    if (sprite>MAXSPRITE || sprite<0) {
        note("sdl_load_image: illegal sprite %d wanted",sprite);
        return -1;
    }

    if (sdl_zip2) {
        sprintf(filename,"%08d.png",sprite);
        if (sdl_load_image_png(si,filename,sdl_zip2,do_smoothify(sprite))==0) return 0;
    }

#if 0
    if (sdl_scale>1) {
        sprintf(filename,"../gfx/x%d/%08d/%08d.png",sdl_scale,(sprite/1000)*1000,sprite);
        if (sdl_load_image_png_(si,filename,NULL)==0) return 0;
    }
#endif

    if (sdl_zip1) {
        sprintf(filename,"%08d.png",sprite);
        if (sdl_load_image_png(si,filename,sdl_zip1,do_smoothify(sprite))==0) return 0;
    }

#if 0
    sprintf(filename,"../gfx/x1/%08d/%08d.png",(sprite/1000)*1000,sprite);
    if (sdl_load_image_png(si,filename,NULL,do_smoothify(sprite))==0) return 0;
#endif
    fail("%s not found",filename);
    exit(42);
}

int sdl_ic_load(int sprite) {
    uint64_t start;

    start=SDL_GetTicks64();

    if (sprite>=MAXSPRITE || sprite<0) {
        note("illegal sprite %d wanted in sdl_ic_load",sprite);
        return -1;
    }
    if (sdli[sprite].flags) return sprite;

    if (sdl_load_image(sdli+sprite,sprite)) return -1;

    sdl_time_load+=SDL_GetTicks64()-start;

    return sprite;
}

#define DDFX_LEFTGRID           1       // NOGRID(?) has to be zero, so bzero on the structures default NOGRID(?)
#define DDFX_RIGHTGRID          2

#define DDFX_MAX_FREEZE         8

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

    r=((r*r*r*r)*shine+r*(100.0-shine))/200.0;
    g=((g*g*g*g)*shine+g*(100.0-shine))/200.0;
    b=((b*b*b*b)*shine+b*(100.0-shine))/200.0;

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

// TODO: add other sprites to this list
// TODO: move to sprite.c
int is_non_wall(int sprite) {

    switch (sprite) {
        case 14177:     return 1;   // earth underground door
        default:        return 0;
    }
}

static void sdl_make(struct sdl_texture *st,struct sdl_image *si,int preload) {
    int x,y,scale;
    double ix,iy,low_x,low_y,high_x,high_y,dbr,dbg,dbb,dba;
    uint32_t irgb;
    long long start;

    if (si->xres==0 || si->yres==0) scale=100;    // !!! needs better handling !!!
    else scale=st->scale;

    if (scale!=100) {
        st->xres=ceil((double)(si->xres-1)*scale/100.0);
        st->yres=ceil((double)(si->yres-1)*scale/100.0);

        st->xoff=floor(si->xoff*scale/100.0+0.5);
        st->yoff=floor(si->yoff*scale/100.0+0.5);
    } else {
        st->xres=si->xres;
        st->yres=si->yres;
        st->xoff=si->xoff;
        st->yoff=si->yoff;
    }

    if (st->sink) st->sink=min(st->sink,max(0,st->yres-4));

    if (!preload || preload==1) {
        if (st->flags&SF_DIDALLOC) {
            fail("double alloc for sprite %d (%s)",st->sprite,st->text);
            note("... sprite=%d (%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)",st->sprite,st->sink,st->freeze,st->grid,st->scale,st->cr,st->cg,st->cb,st->light,st->sat,st->c1,st->c2,st->c3,st->shine,st->ml,st->ll,st->rl,st->ul,st->dl);
            return;
        }

        st->pixel=xmalloc(st->xres*st->yres*sizeof(uint32_t)*sdl_scale*sdl_scale,MEM_SDL_PIXEL);
        st->flags|=SF_DIDALLOC;
    }


    if (!preload || preload==2) {
        if (!(st->flags&SF_DIDALLOC)) {
            fail("cannot make without alloc for sprite %d (%p)",st->sprite,st);
            note("... sprite=%d (%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)",st->sprite,st->sink,st->freeze,st->grid,st->scale,st->cr,st->cg,st->cb,st->light,st->sat,st->c1,st->c2,st->c3,st->shine,st->ml,st->ll,st->rl,st->ul,st->dl);
            return;
        }
        if (st->flags&SF_DIDMAKE) {
            fail("double make for sprite %d (%d)",st->sprite,preload);
            note("... sprite=%d (%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)",st->sprite,st->sink,st->freeze,st->grid,st->scale,st->cr,st->cg,st->cb,st->light,st->sat,st->c1,st->c2,st->c3,st->shine,st->ml,st->ll,st->rl,st->ul,st->dl);
            return;
        }

        start=SDL_GetTicks64();

        for (y=0; y<st->yres*sdl_scale; y++) {
            for (x=0; x<st->xres*sdl_scale; x++) {

                if (scale!=100) {
                    ix=x*100.0/scale;
                    iy=y*100.0/scale;

                    high_x=ix-floor(ix);
                    high_y=iy-floor(iy);
                    low_x=1-high_x;
                    low_y=1-high_y;

                    irgb=si->pixel[(int)(floor(ix)+floor(iy)*si->xres*sdl_scale)];

                    if (st->c1 || st->c2 || st->c3) irgb=sdl_colorize_pix(irgb,st->c1,st->c2,st->c3);
                    dba=IGET_A(irgb)*low_x*low_y;
                    dbr=IGET_R(irgb)*low_x*low_y;
                    dbg=IGET_G(irgb)*low_x*low_y;
                    dbb=IGET_B(irgb)*low_x*low_y;

                    irgb=si->pixel[(int)(ceil(ix)+floor(iy)*si->xres*sdl_scale)];

                    if (st->c1 || st->c2 || st->c3) irgb=sdl_colorize_pix(irgb,st->c1,st->c2,st->c3);
                    dba+=IGET_A(irgb)*high_x*low_y;
                    dbr+=IGET_R(irgb)*high_x*low_y;
                    dbg+=IGET_G(irgb)*high_x*low_y;
                    dbb+=IGET_B(irgb)*high_x*low_y;

                    irgb=si->pixel[(int)(floor(ix)+ceil(iy)*si->xres*sdl_scale)];

                    if (st->c1 || st->c2 || st->c3) irgb=sdl_colorize_pix(irgb,st->c1,st->c2,st->c3);
                    dba+=IGET_A(irgb)*low_x*high_y;
                    dbr+=IGET_R(irgb)*low_x*high_y;
                    dbg+=IGET_G(irgb)*low_x*high_y;
                    dbb+=IGET_B(irgb)*low_x*high_y;

                    irgb=si->pixel[(int)(ceil(ix)+ceil(iy)*si->xres*sdl_scale)];

                    if (st->c1 || st->c2 || st->c3) irgb=sdl_colorize_pix(irgb,st->c1,st->c2,st->c3);
                    dba+=IGET_A(irgb)*high_x*high_y;
                    dbr+=IGET_R(irgb)*high_x*high_y;
                    dbg+=IGET_G(irgb)*high_x*high_y;
                    dbb+=IGET_B(irgb)*high_x*high_y;

                    irgb=IRGBA(((int)dbr),((int)dbg),((int)dbb),((int)dba));

                } else {
                    irgb=si->pixel[x+y*si->xres*sdl_scale];
                    if (st->c1 || st->c2 || st->c3) irgb=sdl_colorize_pix(irgb,st->c1,st->c2,st->c3);
                }

                if (st->cr || st->cg || st->cb || st->light || st->sat) irgb=sdl_colorbalance(irgb,st->cr,st->cg,st->cb,st->light,st->sat);
                if (st->shine) irgb=sdl_shine_pix(irgb,st->shine);

                if (st->ll!=st->ml || st->rl!=st->ml || st->ul!=st->ml || st->dl!=st->ml) {
                    int r,g,b,a;
                    int r1=0,r2=0,r3=0,r4=0,r5=0;
                    int g1=0,g2=0,g3=0,g4=0,g5=0;
                    int b1=0,b2=0,b3=0,b4=0,b5=0;
                    int v1,v2,v3,v4,v5=0;
                    int div;

                    // TODO: This is actually just one direction a non-wall might be facing
                    // and needs to account for the other one as well.
                    if (is_non_wall(st->sprite)) {
                        if (x<10*sdl_scale) {
                            v2=(10*sdl_scale-x)*2-2;
                            r2=IGET_R(sdl_light(st->ll,irgb));
                            g2=IGET_G(sdl_light(st->ll,irgb));
                            b2=IGET_B(sdl_light(st->ll,irgb));
                        } else v2=0;
                        if (x>10*sdl_scale && x<20*sdl_scale) {
                            v3=(x-10*sdl_scale)*2-2;
                            r3=IGET_R(sdl_light(st->ml,irgb));
                            g3=IGET_G(sdl_light(st->ml,irgb));
                            b3=IGET_B(sdl_light(st->ml,irgb));
                        } else v3=0;
                        if (x>20*sdl_scale && x<30*sdl_scale) {
                            v5=(10*sdl_scale-(x-20*sdl_scale))*2-2;
                            r5=IGET_R(sdl_light(st->ml,irgb));
                            g5=IGET_G(sdl_light(st->ml,irgb));
                            b5=IGET_B(sdl_light(st->ml,irgb));
                        } else v5=0;
                        if (x>30*sdl_scale && x<40*sdl_scale) {
                            v4=(x-30*sdl_scale)*2-2;
                            r4=IGET_R(sdl_light(st->rl,irgb));
                            g4=IGET_G(sdl_light(st->rl,irgb));
                            b4=IGET_B(sdl_light(st->rl,irgb));
                        } else v4=0;
                    } else {
                        if (y<10*sdl_scale+(20*sdl_scale-abs(20*sdl_scale-x))/2) {

                            // This part calculates a floor tile, or the top of a wall tile
                            if (x/2<20*sdl_scale-y) {
                                v2=-(x/2-(20*sdl_scale-y))+1;
                                r2=IGET_R(sdl_light(st->ll,irgb));
                                g2=IGET_G(sdl_light(st->ll,irgb));
                                b2=IGET_B(sdl_light(st->ll,irgb));
                            } else v2=0;
                            if (x/2>20*sdl_scale-y) {
                                v3=(x/2-(20*sdl_scale-y))+1;
                                r3=IGET_R(sdl_light(st->rl,irgb));
                                g3=IGET_G(sdl_light(st->rl,irgb));
                                b3=IGET_B(sdl_light(st->rl,irgb));
                            } else v3=0;
                            if (x/2>y) {
                                v4=(x/2-y)+1;
                                r4=IGET_R(sdl_light(st->ul,irgb));
                                g4=IGET_G(sdl_light(st->ul,irgb));
                                b4=IGET_B(sdl_light(st->ul,irgb));
                            } else v4=0;
                            if (x/2<y) {
                                v5=-(x/2-y)+1;
                                r5=IGET_R(sdl_light(st->dl,irgb));
                                g5=IGET_G(sdl_light(st->dl,irgb));
                                b5=IGET_B(sdl_light(st->dl,irgb));
                            } else v5=0;
                        } else {

                            // This is for the lower part (left side and front as seen on the screen)
                            if (x<10*sdl_scale) {
                                v2=(10*sdl_scale-x)*2-2;
                                r2=IGET_R(sdl_light(st->ll,irgb));
                                g2=IGET_G(sdl_light(st->ll,irgb));
                                b2=IGET_B(sdl_light(st->ll,irgb));
                            } else v2=0;
                            if (x>10*sdl_scale && x<20*sdl_scale) {
                                v3=(x-10*sdl_scale)*2-2;
                                r3=IGET_R(sdl_light(st->rl,irgb));
                                g3=IGET_G(sdl_light(st->rl,irgb));
                                b3=IGET_B(sdl_light(st->rl,irgb));
                            } else v3=0;
                            if (x>20*sdl_scale && x<30*sdl_scale) {
                                v5=(10*sdl_scale-(x-20*sdl_scale))*2-2;
                                r5=IGET_R(sdl_light(st->dl,irgb));
                                g5=IGET_G(sdl_light(st->dl,irgb));
                                b5=IGET_B(sdl_light(st->dl,irgb));
                            } else v5=0;
                            if (x>30*sdl_scale && x<40*sdl_scale) {
                                v4=(x-30*sdl_scale)*2-2;
                                r4=IGET_R(sdl_light(st->ul,irgb));
                                g4=IGET_G(sdl_light(st->ul,irgb));
                                b4=IGET_B(sdl_light(st->ul,irgb));
                            } else v4=0;
                        }
                    }

                    v1=20*sdl_scale-(v2+v3+v4+v5)/2;
                    r1=IGET_R(sdl_light(st->ml,irgb));
                    g1=IGET_G(sdl_light(st->ml,irgb));
                    b1=IGET_B(sdl_light(st->ml,irgb));

                    div=v1+v2+v3+v4+v5;

                    a=IGET_A(irgb);
                    r=(r1*v1+r2*v2+r3*v3+r4*v4+r5*v5)/div;
                    g=(g1*v1+g2*v2+g3*v3+g4*v4+g5*v5)/div;
                    b=(b1*v1+b2*v2+b3*v3+b4*v4+b5*v5)/div;

                    irgb=IRGBA(r,g,b,a);

                } else irgb=sdl_light(st->ml,irgb);

                if (st->sink) {
                    if (st->yres*sdl_scale-st->sink<y) irgb&=0xffffff;    // zero alpha to make it transparent
                }

                if (st->freeze) irgb=sdl_freeze(st->freeze,irgb);

                if (st->grid==DDFX_LEFTGRID) { if ((st->xoff+x+st->yoff+y)&1) irgb&=0xffffff; }
                if (st->grid==DDFX_RIGHTGRID) {  if ((st->xoff+x+st->yoff+y+1)&1) irgb&=0xffffff; }

                st->pixel[x+y*st->xres*sdl_scale]=irgb;
            }
        }
        st->flags|=SF_DIDMAKE;

        if (preload) sdl_time_preload+=SDL_GetTicks64()-start;
        else sdl_time_make+=SDL_GetTicks64()-start;
    }

    if (!preload || preload==3) {
        if (!(st->flags&SF_DIDMAKE)) {
            fail("cannot texture without make for sprite %d (%d)",st->sprite,preload);
            //note("... sprite=%d (%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)",st->sprite,st->sink,st->freeze,st->grid,st->scale,st->cr,st->cg,st->cb,st->light,st->sat,st->c1,st->c2,st->c3,st->shine,st->ml,st->ll,st->rl,st->ul,st->dl);
            return;
        }
        if (st->flags&SF_DIDTEX) {
            fail("double texture for sprite %d (%d)",st->sprite,preload);
            //note("... sprite=%d (%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)",st->sprite,st->sink,st->freeze,st->grid,st->scale,st->cr,st->cg,st->cb,st->light,st->sat,st->c1,st->c2,st->c3,st->shine,st->ml,st->ll,st->rl,st->ul,st->dl);
            return;
        }

        start=SDL_GetTicks64();

        SDL_Texture *texture = SDL_CreateTexture(sdlren,SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STATIC,st->xres*sdl_scale,st->yres*sdl_scale);
        if (!texture) warn("SDL_texture Error: %s in sprite %d (%s, %d,%d) preload=%d",SDL_GetError(),st->sprite,st->text,st->xres,st->yres,preload);
        SDL_UpdateTexture(texture,NULL,st->pixel,st->xres*sizeof(uint32_t)*sdl_scale);
        SDL_SetTextureBlendMode(texture,SDL_BLENDMODE_BLEND);
        xfree(st->pixel);
        st->pixel=NULL;
        st->tex=texture;

        st->flags|=SF_DIDTEX;

        sdl_time_tex+=SDL_GetTicks64()-start;
    }
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

static inline unsigned int hashfunc(int sprite,int ml,int ll,int rl,int ul,int dl) {
    unsigned int hash;

    hash=sprite^(ml<<2)^(ll<<4)^(rl<<6)^(ul<<8)^(dl<<10);

    return hash%MAX_TEXHASH;
}

static inline unsigned int hashfunc_text(const char *text,int color,int flags) {
    unsigned int hash,t0,t1,t2,t3;

    t0=text[0];
    if (text[0]) {
        t1=text[1];
        if (text[1]) {
            t2=text[2];
            if (text[2]) {
                t3=text[3];
            } else t3=0;
        } else t2=t3=0;
    } else t1=t2=t3=0;

    hash=(t0<<0)^(t1<<3)^(t2<<6)^(t3<<9)^(color<<0)^(flags<<5);

    return hash%MAX_TEXHASH;
}

SDL_Texture *sdl_maketext(const char *text,struct ddfont *font,uint32_t color,int flags);

int sdl_tx_load(int sprite,int sink,int freeze,int grid,int scale,int cr,int cg,int cb,int light,int sat,int c1,int c2,int c3,int shine,int ml,int ll,int rl,int ul,int dl,
                const char *text,int text_color,int text_flags,void *text_font,int checkonly,int preload) {
    int stx,ptx,ntx,panic=0;
    int hash;

    if (!text) hash=hashfunc(sprite,ml,ll,rl,ul,dl);
    else hash=hashfunc_text(text,text_color,text_flags);

    if (sprite>=MAXSPRITE || sprite<0) {
        note("illegal sprite %d wanted in sdl_tx_load",sprite);
        return STX_NONE;
    }

    for (stx=sdlt_cache[hash]; stx!=STX_NONE; stx=sdlt[stx].hnext,panic++) {

        if (panic>999) {
            warn("%04d: stx=%d, hprev=%d, hnext=%d sprite=%d (%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d), PANIC\n",panic,stx,sdlt[stx].hprev,sdlt[stx].hnext,sprite,
                   sdlt[stx].sink,sdlt[stx].freeze,sdlt[stx].grid,sdlt[stx].scale,
                   sdlt[stx].cr,sdlt[stx].cg,sdlt[stx].cb,sdlt[stx].light,
                   sdlt[stx].sat,sdlt[stx].c1,sdlt[stx].c2,sdlt[stx].c3,
                   sdlt[stx].shine,sdlt[stx].ml,sdlt[stx].ll,sdlt[stx].rl,
                   sdlt[stx].ul,sdlt[stx].dl);
            if (panic>1099) exit(42);
        }
        if (text) {
            if (!(sdlt[stx].flags&SF_TEXT)) continue;
            if (!sdlt[stx].text || strcmp(sdlt[stx].text,text)) continue;
            if (sdlt[stx].text_flags!=text_flags) continue;
            if (sdlt[stx].text_color!=text_color) continue;
            if (sdlt[stx].text_font!=text_font) continue;
        } else {
            if (!(sdlt[stx].flags&SF_SPRITE)) continue;
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
        }

        if (checkonly) return 1;
        if (preload==1) return -1;

        if (panic>maxpanic) maxpanic=panic;

        if (!preload && (sdlt[stx].flags&SF_SPRITE)) {

            // wait for the background preloader if we have multiple threads
            while (sdl_multi) {
                if ((sdlt[stx].flags&SF_DIDMAKE)) break;

                //warn("waiting for graphics...");
                SDL_Delay(1);
            }

            // if we are single threaded do it yourself
            if (!sdl_multi && !(sdlt[stx].flags&SF_DIDMAKE)) {
                long long start=SDL_GetTicks64();
                sdl_make(sdlt+stx,sdli+sprite,2);
                sdl_time_make_main+=SDL_GetTicks64()-start;
            }

            // make texture now if preload didn't finish it
            if (!(sdlt[stx].flags&SF_DIDTEX)) {
                long long start=SDL_GetTicks64();
                sdl_make(sdlt+stx,sdli+sprite,3);
                sdl_time_tex_main+=SDL_GetTicks64()-start;
            }
        }

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
#if 0
        if (!preload && sdl_frames>10 && sprite) {    // wait for things to stabilize before reporting misses
            printf("hit  sprite=%5d (%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d) M\n",
                   sprite,
                   sdlt[stx].sink,sdlt[stx].freeze,sdlt[stx].grid,sdlt[stx].scale,
                   sdlt[stx].cr,sdlt[stx].cg,sdlt[stx].cb,sdlt[stx].light,
                   sdlt[stx].sat,sdlt[stx].c1,sdlt[stx].c2,sdlt[stx].c3,
                   sdlt[stx].shine,sdlt[stx].ml,sdlt[stx].ll,sdlt[stx].rl,
                   sdlt[stx].ul,sdlt[stx].dl);
            fflush(stdout);
        }
#endif
        if (!preload) texc_hit++;

        return stx;
    }
    if (checkonly) return 0;

    stx=sdlt_last;

    // delete
    if (sdlt[stx].flags) {
        int hash2;

        if (sdlt[stx].flags&SF_SPRITE) {
            if (sdl_multi) {
                while (sdlt[stx].flags&(SF_BUSY)) {
                    note("Delete: Busy");
                    SDL_Delay(1);
                }
                while (!(sdlt[stx].flags&(SF_DIDMAKE))) {
                    note("Delete: Busy2");
                    SDL_Delay(1);
                }

            }
            if (!(sdlt[stx].flags&(SF_DIDALLOC)))
                warn("Delete: noalloc");
            if (!(sdlt[stx].flags&(SF_DIDMAKE)))
                warn("Delete: nomake");
            if (!(sdlt[stx].flags&(SF_DIDTEX)))
                warn("Delete: notex");
        }

        if (sdlt[stx].flags&SF_SPRITE) hash2=hashfunc(sdlt[stx].sprite,sdlt[stx].ml,sdlt[stx].ll,sdlt[stx].rl,sdlt[stx].ul,sdlt[stx].dl);
        else if (sdlt[stx].flags&SF_TEXT) hash2=hashfunc_text(sdlt[stx].text,sdlt[stx].text_color,sdlt[stx].text_flags);
        else { hash2=0; warn("weird entry in texture cache!"); }

        ntx=sdlt[stx].hnext;
        ptx=sdlt[stx].hprev;

        if (ptx==STX_NONE) {
            if (sdlt_cache[hash2]!=stx) {
                fail("sdli[sprite].stx!=stx\n");
                exit(42);
            }
            sdlt_cache[hash2]=ntx;
        } else {
            sdlt[ptx].hnext=sdlt[stx].hnext;
        }

        if (ntx!=STX_NONE) {
            sdlt[ntx].hprev=sdlt[stx].hprev;
        }


        if (sdlt[stx].flags&SF_DIDMAKE) {
            mem_tex-=sdlt[stx].xres*sdlt[stx].yres*sizeof(uint32_t);
            SDL_DestroyTexture(sdlt[stx].tex);
        } else {
            warn("Delete unfinished texture?? stx=%d, sprite=%d",stx,sdlt[stx].sprite);
        }

        if (sdlt[stx].flags&SF_TEXT) xfree(sdlt[stx].text);

        sdlt[stx].flags=0;
    }

    // build
    if (text) {
        int w,h;
        sdlt[stx].tex=sdl_maketext(text,(struct ddfont *)text_font,text_color,text_flags);
        sdlt[stx].flags=SF_USED|SF_TEXT|SF_DIDALLOC|SF_DIDMAKE|SF_DIDTEX;
        sdlt[stx].text_color=text_color;
        sdlt[stx].text_flags=text_flags;
        sdlt[stx].text_font=text_font;
        sdlt[stx].text=xstrdup(text,MEM_TEMP7);
        if (sdlt[stx].tex) {
            SDL_QueryTexture(sdlt[stx].tex,NULL,NULL,&w,&h);
            sdlt[stx].xres=w;
            sdlt[stx].yres=h;
        } else sdlt[stx].xres=sdlt[stx].yres=0;
    } else {

        sdl_ic_load(sprite);

        // init
        sdlt[stx].flags=SF_USED|SF_SPRITE;
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

        sdl_make(sdlt+stx,sdli+sprite,preload);
    }

    mem_tex+=sdlt[stx].xres*sdlt[stx].yres*sizeof(uint32_t);

    ntx=sdlt_cache[hash];

    if (ntx!=STX_NONE) sdlt[ntx].hprev=stx;

    sdlt[stx].hprev=STX_NONE;
    sdlt[stx].hnext=ntx;

    sdlt_cache[hash]=stx;

    sdl_tx_best(stx);

    if (preload) texc_pre++;
    else if (sprite) {  // Do not count missed text sprites. Those are expected.
        texc_miss++;
#if 0
        if (sdl_frames>10) {    // wait for things to stabilize before reporting misses
            printf("miss sprite=%5d (%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d) M\n",
                   sprite,
                   sdlt[stx].sink,sdlt[stx].freeze,sdlt[stx].grid,sdlt[stx].scale,
                   sdlt[stx].cr,sdlt[stx].cg,sdlt[stx].cb,sdlt[stx].light,
                   sdlt[stx].sat,sdlt[stx].c1,sdlt[stx].c2,sdlt[stx].c3,
                   sdlt[stx].shine,sdlt[stx].ml,sdlt[stx].ll,sdlt[stx].rl,
                   sdlt[stx].ul,sdlt[stx].dl);
            fflush(stdout);
        }
#endif
    }

    return stx;
}

static void sdl_blit_tex(SDL_Texture *tex,int sx,int sy,int clipsx,int clipsy,int clipex,int clipey,int x_offset,int y_offset) {
    int addx=0,addy=0,dx,dy;
    SDL_Rect dr,sr;
    long long start=SDL_GetTicks64();

    SDL_QueryTexture(tex, NULL, NULL, &dx, &dy);

    dx/=sdl_scale; dy/=sdl_scale;
    if (sx<clipsx) { addx=clipsx-sx; dx-=addx; sx=clipsx; }
    if (sy<clipsy) { addy=clipsy-sy; dy-=addy; sy=clipsy; }
    if (sx+dx>=clipex) dx=clipex-sx;
    if (sy+dy>=clipey) dy=clipey-sy;
    dx*=sdl_scale; dy*=sdl_scale;

    dr.x=(sx+x_offset)*sdl_scale; dr.w=dx;
    dr.y=(sy+y_offset)*sdl_scale; dr.h=dy;

    sr.x=addx*sdl_scale; sr.w=dx;
    sr.y=addy*sdl_scale; sr.h=dy;

    SDL_RenderCopy(sdlren,tex,&sr,&dr);

    sdl_time_blit+=SDL_GetTicks64()-start;
}

void sdl_blit(int stx,int sx,int sy,int clipsx,int clipsy,int clipex,int clipey,int x_offset,int y_offset) {
    sdl_blit_tex(sdlt[stx].tex,sx,sy,clipsx,clipsy,clipex,clipey,x_offset,y_offset);
}

#define DD_LEFT         0
#define DD_CENTER       1
#define DD_RIGHT        2
#define DD_SHADE        4
#define DD_LARGE        0
#define DD_SMALL        8
#define DD_FRAME        16
#define DD_BIG        	32
#define DD_NOCACHE      64

#define DD__SHADEFONT	128
#define DD__FRAMEFONT	256

#define R16TO32(color)  (int)((((color>>10)&31)/31.0f)*255.0f)
#define G16TO32(color)  (int)((((color>>5) &31)/31.0f)*255.0f)
#define B16TO32(color)  (int)((((color)    &31)/31.0f)*255.0f)

#define MAXFONTHEIGHT   64

SDL_Texture *sdl_maketext(const char *text,struct ddfont *font,uint32_t color,int flags) {
    uint32_t *pixel,*dst;
    unsigned char *rawrun;
    int x,y=0,sizex,sizey=0,sx=0;
    const char *c;
    long long start=SDL_GetTicks64();

    for (sizex=0,c=text; *c; c++) sizex+=font[*c].dim*sdl_scale;

    if (flags&(DD__FRAMEFONT|DD__SHADEFONT)) sizex+=sdl_scale*2;

    pixel=xcalloc(sizex*MAXFONTHEIGHT*sizeof(uint32_t),MEM_SDL_PIXEL2);
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
        sx+=font[*text++].dim*sdl_scale;
    }

    if (sizex<1 || sizey<1) {
        xfree(pixel);
        return NULL;
    }
    sizey++;
    sdl_time_text+=SDL_GetTicks64()-start;

    start=SDL_GetTicks64();
    SDL_Texture *texture = SDL_CreateTexture(sdlren,SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STATIC,sizex,sizey);
    if (!texture) warn("SDL_texture Error: %s",SDL_GetError());
    SDL_UpdateTexture(texture,NULL,pixel,sizex*sizeof(uint32_t));
    SDL_SetTextureBlendMode(texture,SDL_BLENDMODE_BLEND);
    xfree(pixel);
    sdl_time_tex+=SDL_GetTicks64()-start;

    return texture;
}

int *dumpidx;

int dump_cmp(const void *ca,const void *cb) {
    int a,b;

    a=*(int*)ca;
    b=*(int*)cb;

    if (!sdlt[a].flags) return 1;
    if (!sdlt[b].flags) return -1;

    if (sdlt[a].flags&SF_TEXT) return 1;
    if (sdlt[b].flags&SF_TEXT) return -1;

    return sdlt[a].sprite-sdlt[b].sprite;
}

#ifdef DEVELOPER
void sdl_dump_spritechache(void) {
    int i,n,cnt=0,uni=0,text=0;
    long long size=0;
    FILE *fp;

    dumpidx=xmalloc(sizeof(int)*MAX_TEXCACHE,MEM_TEMP);
    for (i=0; i<MAX_TEXCACHE; i++) dumpidx[i]=i;

    qsort(dumpidx,MAX_TEXCACHE,sizeof(int),dump_cmp);

    fp=fopen("sdlt.txt","w");

    for (i=0; i<MAX_TEXCACHE; i++) {

        n=dumpidx[i];
        if (!sdlt[n].flags) break;

        if (sdlt[n].flags&SF_TEXT) text++;
        else {
            if (i==0) uni++;
            else if (sdlt[dumpidx[i]].sprite!=sdlt[dumpidx[i-1]].sprite) uni++;
            cnt++;
        }

        if (sdlt[n].flags&SF_SPRITE)
            fprintf(fp,"Sprite: %6d, Lights: %2d,%2d,%2d,%2d,%2d, Light: %3d, Colors: %3d,%3d,%3d, Colors: %4X,%4X,%4X, Sink: %2d, Freeze: %2d, Grid: %2d, Scale: %3d, Sat: %3d, Shine: %3d, %dx%d\n",
                   sdlt[n].sprite,
                   sdlt[n].ml,
                   sdlt[n].ll,
                   sdlt[n].rl,
                   sdlt[n].ul,
                   sdlt[n].dl,
                   sdlt[n].light,
                   sdlt[n].cr,
                   sdlt[n].cg,
                   sdlt[n].cb,
                   sdlt[n].c1,
                   sdlt[n].c2,
                   sdlt[n].c3,
                   sdlt[n].sink,
                   sdlt[n].freeze,
                   sdlt[n].grid,
                   sdlt[n].scale,
                   sdlt[n].sat,
                   sdlt[n].shine,
                   sdlt[n].xres,
                   sdlt[n].yres);
        if (sdlt[n].flags&SF_TEXT)
            fprintf(fp,"Color: %08X, Flags: %04X, Font: %p, Text: %s (%dx%d)\n",
                    sdlt[n].text_color,
                    sdlt[n].text_flags,
                    sdlt[n].text_font,
                    sdlt[n].text,
                    sdlt[n].xres,
                    sdlt[n].yres);

        size+=sdlt[n].xres*sdlt[n].yres*sizeof(uint32_t);

    }
    fprintf(fp,"\n%d unique sprites, %d sprites + %d texts of %d used. %.2fM texture memory.\n",uni,cnt,text,MAX_TEXCACHE,size/(1024.0*1024.0));
    fclose(fp);
    xfree(dumpidx);

}
#endif

void sdl_exit(void) {

    if (sdl_zip1) zip_close(sdl_zip1);
    if (sdl_zip2) zip_close(sdl_zip2);

    if (enable_sound) Mix_Quit();
#ifdef DEVELOPER
    sdl_dump_spritechache();
#endif
}

int sdl_drawtext(int sx,int sy,unsigned short int color,int flags,const char *text,struct ddfont *font,int clipsx,int clipsy,int clipex,int clipey,int x_offset,int y_offset) {
    int dx,stx;
    SDL_Texture *tex;
    int r,g,b,a;
    const char *c;

    r=R16TO32(color);
    g=G16TO32(color);
    b=B16TO32(color);
    a=255;

    if (flags&DD_NOCACHE) {
        tex=sdl_maketext(text,font,IRGBA(r,g,b,a),flags);
    } else {
        stx=sdl_tx_load(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,text,IRGBA(r,g,b,a),flags,font,0,0);
        tex=sdlt[stx].tex;
    }

    for (dx=0,c=text; *c; c++) dx+=font[*c].dim;

    if (tex) {
        if (flags&DD_CENTER) sx-=dx/2;
        else if (flags&DD_RIGHT) sx-=dx;

        sdl_blit_tex(tex,sx,sy,clipsx,clipsy,clipex,clipey,x_offset,y_offset);

        if (flags&DD_NOCACHE) SDL_DestroyTexture(tex);
    }

    return sx+dx;
}

void sdl_rect(int sx,int sy,int ex,int ey,unsigned short int color,int clipsx,int clipsy,int clipex,int clipey,int x_offset,int y_offset) {
    int r,g,b,a;
    SDL_Rect rc;

    r=R16TO32(color);
    g=G16TO32(color);
    b=B16TO32(color);
    a=255;

    if (sx<clipsx) sx=clipsx;
    if (sy<clipsy) sy=clipsy;
    if (ex>clipex) ex=clipex;
    if (ey>clipey) ey=clipey;

    if (sx>ex || sy>ey) return;

    rc.x=(sx+x_offset)*sdl_scale; rc.w=(ex-sx)*sdl_scale;
    rc.y=(sy+y_offset)*sdl_scale; rc.h=(ey-sy)*sdl_scale;

    SDL_SetRenderDrawColor(sdlren,r,g,b,a);
    SDL_RenderFillRect(sdlren,&rc);
}

void sdl_shaded_rect(int sx,int sy,int ex,int ey,unsigned short int color,int clipsx,int clipsy,int clipex,int clipey,int x_offset,int y_offset) {
    int r,g,b,a;
    SDL_Rect rc;

    r=R16TO32(color);
    g=G16TO32(color);
    b=B16TO32(color);
    a=95;

    if (sx<clipsx) sx=clipsx;
    if (sy<clipsy) sy=clipsy;
    if (ex>clipex) ex=clipex;
    if (ey>clipey) ey=clipey;

    if (sx>ex || sy>ey) return;

    rc.x=(sx+x_offset)*sdl_scale; rc.w=(ex-sx)*sdl_scale;
    rc.y=(sy+y_offset)*sdl_scale; rc.h=(ey-sy)*sdl_scale;

    SDL_SetRenderDrawColor(sdlren,r,g,b,a);
    SDL_SetRenderDrawBlendMode(sdlren,SDL_BLENDMODE_BLEND);
    SDL_RenderFillRect(sdlren,&rc);
}


void sdl_pixel(int x,int y,unsigned short color,int x_offset,int y_offset) {
    int r,g,b,a,i;
    SDL_Point pt[16];

    r=R16TO32(color);
    g=G16TO32(color);
    b=B16TO32(color);
    a=255;

    SDL_SetRenderDrawColor(sdlren,r,g,b,a);
    switch (sdl_scale) {
        case 1:     SDL_RenderDrawPoint(sdlren,x+x_offset,y+y_offset); return;
        case 2:     pt[0].x=(x+x_offset)*sdl_scale;
                    pt[0].y=(y+y_offset)*sdl_scale;
                    pt[1].x=pt[0].x+1;
                    pt[1].y=pt[0].y;
                    pt[2].x=pt[0].x;
                    pt[2].y=pt[0].y+1;
                    pt[3].x=pt[0].x+1;
                    pt[3].y=pt[0].y+1;
                    i=4;
                    break;
        case 3:     pt[0].x=(x+x_offset)*sdl_scale;
                    pt[0].y=(y+y_offset)*sdl_scale;
                    pt[1].x=pt[0].x+1;
                    pt[1].y=pt[0].y;
                    pt[2].x=pt[0].x;
                    pt[2].y=pt[0].y+1;
                    pt[3].x=pt[0].x+1;
                    pt[3].y=pt[0].y+1;
                    pt[4].x=pt[0].x+2;
                    pt[4].y=pt[0].y;
                    pt[5].x=pt[0].x;
                    pt[5].y=pt[0].y+2;
                    pt[6].x=pt[0].x+2;
                    pt[6].y=pt[0].y+2;
                    pt[7].x=pt[0].x+2;
                    pt[7].y=pt[0].y+1;
                    pt[8].x=pt[0].x+1;
                    pt[8].y=pt[0].y+2;
                    i=9;
                    break;
        case 4:     pt[0].x=(x+x_offset)*sdl_scale;
                    pt[0].y=(y+y_offset)*sdl_scale;
                    pt[1].x=pt[0].x+1;
                    pt[1].y=pt[0].y;
                    pt[2].x=pt[0].x;
                    pt[2].y=pt[0].y+1;
                    pt[3].x=pt[0].x+1;
                    pt[3].y=pt[0].y+1;
                    pt[4].x=pt[0].x+2;
                    pt[4].y=pt[0].y;
                    pt[5].x=pt[0].x;
                    pt[5].y=pt[0].y+2;
                    pt[6].x=pt[0].x+2;
                    pt[6].y=pt[0].y+2;
                    pt[7].x=pt[0].x+2;
                    pt[7].y=pt[0].y+1;
                    pt[8].x=pt[0].x+1;
                    pt[8].y=pt[0].y+2;
                    pt[9].x=pt[0].x+3;
                    pt[9].y=pt[0].y;
                    pt[10].x=pt[0].x+3;
                    pt[10].y=pt[0].y+1;
                    pt[11].x=pt[0].x+3;
                    pt[11].y=pt[0].y+2;
                    pt[12].x=pt[0].x+3;
                    pt[12].y=pt[0].y+3;
                    pt[13].x=pt[0].x;
                    pt[13].y=pt[0].y+3;
                    pt[14].x=pt[0].x+1;
                    pt[14].y=pt[0].y+3;
                    pt[15].x=pt[0].x+2;
                    pt[15].y=pt[0].y+3;
                    i=16;
                    break;
        default:    warn("unsupported scale %d in sdl_pixel()",sdl_scale); return;
    }
    SDL_RenderDrawPoints(sdlren,pt,i);
}

void sdl_line(int fx,int fy,int tx,int ty,unsigned short color,int clipsx,int clipsy,int clipex,int clipey,int x_offset,int y_offset) {
    int r,g,b,a;

    r=R16TO32(color);
    g=G16TO32(color);
    b=B16TO32(color);
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
    // TODO: This is a thinner line when scaled up. It looks surprisingly good. Maybe keep it this way?
    SDL_RenderDrawLine(sdlren,fx*sdl_scale,fy*sdl_scale,tx*sdl_scale,ty*sdl_scale);
}

void gui_sdl_keyproc(int wparam);
void gui_sdl_mouseproc(int x,int y,int but);
void cmd_proc(int key);

#define SDL_MOUM_NONE       0
#define SDL_MOUM_LUP        1
#define SDL_MOUM_LDOWN      2
#define SDL_MOUM_RUP        3
#define SDL_MOUM_RDOWN      4
#define SDL_MOUM_MUP        5
#define SDL_MOUM_MDOWN      6

void sdl_loop(void) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch(event.type) {
            case SDL_QUIT:
                quit=1;
                break;
            case SDL_KEYDOWN:
                gui_sdl_keyproc(event.key.keysym.sym);
                break;
            case SDL_TEXTINPUT:
                cmd_proc(event.text.text[0]);
                break;
            case SDL_MOUSEMOTION:
                gui_sdl_mouseproc(event.motion.x,event.motion.y,SDL_MOUM_NONE);
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button==SDL_BUTTON_LEFT) gui_sdl_mouseproc(event.motion.x,event.motion.y,SDL_MOUM_LDOWN);
                if (event.button.button==SDL_BUTTON_MIDDLE) gui_sdl_mouseproc(event.motion.x,event.motion.y,SDL_MOUM_MDOWN);
                if (event.button.button==SDL_BUTTON_RIGHT) gui_sdl_mouseproc(event.motion.x,event.motion.y,SDL_MOUM_RDOWN);
                break;
            case SDL_MOUSEBUTTONUP:
                if (event.button.button==SDL_BUTTON_LEFT) gui_sdl_mouseproc(event.motion.x,event.motion.y,SDL_MOUM_LUP);
                if (event.button.button==SDL_BUTTON_MIDDLE) gui_sdl_mouseproc(event.motion.x,event.motion.y,SDL_MOUM_MUP);
                if (event.button.button==SDL_BUTTON_RIGHT) gui_sdl_mouseproc(event.motion.x,event.motion.y,SDL_MOUM_RUP);
                break;

        }
    }
}


int sdl_keymode(void) {
    SDL_Keymod km;
    int ret=0;

    km=SDL_GetModState();

    if (km&KMOD_ALT) ret|=SDL_KEYM_ALT;
    if (km&KMOD_CTRL) ret|=SDL_KEYM_CTRL;
    if (km&KMOD_SHIFT) ret|=SDL_KEYM_SHIFT;

    return ret;
}

void sdl_set_cursor_pos(int x,int y) {
    SDL_WarpMouseInWindow(sdlwnd,x,y);
}

void sdl_show_cursor(int flag) {
    SDL_ShowCursor(flag ? SDL_ENABLE : SDL_DISABLE);
}

void sdl_capture_mouse(int flag) {
    SDL_CaptureMouse(flag);
}

int sdlt_xoff(int stx) {
    return sdlt[stx].xoff;
}
int sdlt_yoff(int stx) {
    return sdlt[stx].yoff;
}
int sdlt_xres(int stx) {
    return sdlt[stx].xres;
}
int sdlt_yres(int stx) {
    return sdlt[stx].yres;
}

uint32_t *sdl_load_png(char *filename,int *dx,int *dy) {
    int x,y,xres,yres,tmp,r,g,b,a;
    int format;
    unsigned char **row;
    uint32_t *pixel;
    FILE *fp;
    png_structp png_ptr;
    png_infop info_ptr;
    png_infop end_info;

    fp=fopen(filename,"rb");
    if (!fp) return NULL;

    png_ptr=png_create_read_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
    if (!png_ptr) { fclose(fp); warn("create read\n"); return NULL; }

    info_ptr=png_create_info_struct(png_ptr);
    if (!info_ptr) { fclose(fp); png_destroy_read_struct(&png_ptr,(png_infopp)NULL,(png_infopp)NULL); warn("create info1\n"); return NULL; }

    end_info=png_create_info_struct(png_ptr);
    if (!end_info) { fclose(fp); png_destroy_read_struct(&png_ptr,&info_ptr,(png_infopp)NULL); warn("create info2\n"); return NULL; }

    png_init_io(png_ptr,fp);
    png_set_strip_16(png_ptr);
    png_read_png(png_ptr,info_ptr,PNG_TRANSFORM_PACKING,NULL);

    row=png_get_rows(png_ptr,info_ptr);
    if (!row) { fclose(fp); png_destroy_read_struct(&png_ptr,&info_ptr,(png_infopp)NULL); warn("read row\n"); return NULL; }

    xres=png_get_image_width(png_ptr,info_ptr);
    yres=png_get_image_height(png_ptr,info_ptr);

    tmp=png_get_rowbytes(png_ptr,info_ptr);

    if (tmp==xres*3) format=3;
    else if (tmp==xres*4) format=4;
    else { fclose(fp); png_destroy_read_struct(&png_ptr,&info_ptr,(png_infopp)NULL); warn("rowbytes!=xres*4 (%d)",tmp); return NULL; }

    if (png_get_bit_depth(png_ptr,info_ptr)!=8) { fclose(fp); png_destroy_read_struct(&png_ptr,&info_ptr,(png_infopp)NULL); warn("bit depth!=8\n"); return NULL; }
    if (png_get_channels(png_ptr,info_ptr)!=format) { fclose(fp); png_destroy_read_struct(&png_ptr,&info_ptr,(png_infopp)NULL); warn("channels!=format\n"); return NULL; }

    // prescan
    if (dx) *dx=xres;
    if (dy) *dy=yres;

    pixel=xmalloc(xres*yres*sizeof(uint32_t),MEM_TEMP8);

    if (format==4) {
        for (y=0; y<yres; y++) {
            for (x=0; x<xres; x++) {

                r=row[y][x*4+0];
                g=row[y][x*4+1];
                b=row[y][x*4+2];
                a=row[y][x*4+3];

                if (a) {
                    r=min(255,r*255/a);
                    g=min(255,g*255/a);
                    b=min(255,b*255/a);
                } else r=g=b=0;

                pixel[x+y*xres]=IRGBA(r,g,b,a);
            }
        }
    } else {
        for (y=0; y<yres; y++) {
            for (x=0; x<xres; x++) {

                r=row[y][x*3+0];
                g=row[y][x*3+1];
                b=row[y][x*3+2];
                a=255;

                pixel[x+y*xres]=IRGBA(r,g,b,a);
            }
        }
    }

    png_destroy_read_struct(&png_ptr,&info_ptr,(png_infopp)NULL);
    fclose(fp);

    return pixel;
}

/* This function is a hack. It can only load one specific type of
   Windows cursor file: 32x32 pixels with 1 bit depth. */

SDL_Cursor *sdl_create_cursor(char *filename) {
    int handle;
    unsigned char mask[128],data[128],buf[326];

    handle=open(filename,O_RDONLY|O_BINARY);
    if (handle==-1) {
        warn("SDL Error: Could not open cursor file %s.\n",filename);
        return NULL;
    }

    if (read(handle,buf,326)!=326) {
        warn("SDL Error: Read cursor file failed.\n");
        return NULL;
    }
    close(handle);

    for (int i=0; i<32; i++) {
        for (int j=0; j<4; j++) {
            data[i*4+j]=(~buf[322-i*4+j])&(~buf[194-i*4+j]);
            mask[i*4+j]=buf[194-i*4+j];
        }
    }
    return SDL_CreateCursor(data,mask,32,32,6,6);
}

int sdl_create_cursors(void) {
    curs[SDL_CUR_c_only]=sdl_create_cursor("cursor/c_only.cur");
    curs[SDL_CUR_c_take]=sdl_create_cursor("cursor/c_take.cur");
    curs[SDL_CUR_c_drop]=sdl_create_cursor("cursor/c_drop.cur");
    curs[SDL_CUR_c_attack]=sdl_create_cursor("cursor/c_atta.cur");
    curs[SDL_CUR_c_raise]=sdl_create_cursor("cursor/c_rais.cur");
    curs[SDL_CUR_c_give]=sdl_create_cursor("cursor/c_give.cur");
    curs[SDL_CUR_c_use]=sdl_create_cursor("cursor/c_use.cur");
    curs[SDL_CUR_c_usewith]=sdl_create_cursor("cursor/c_usew.cur");
    curs[SDL_CUR_c_swap]=sdl_create_cursor("cursor/c_swap.cur");
    curs[SDL_CUR_c_sell]=sdl_create_cursor("cursor/c_sell.cur");
    curs[SDL_CUR_c_buy]=sdl_create_cursor("cursor/c_buy.cur");
    curs[SDL_CUR_c_look]=sdl_create_cursor("cursor/c_look.cur");
    curs[SDL_CUR_c_set]=sdl_create_cursor("cursor/c_set.cur");
    curs[SDL_CUR_c_spell]=sdl_create_cursor("cursor/c_spell.cur");
    curs[SDL_CUR_c_pix]=sdl_create_cursor("cursor/c_pix.cur");
    curs[SDL_CUR_c_say]=sdl_create_cursor("cursor/c_say.cur");
    curs[SDL_CUR_c_junk]=sdl_create_cursor("cursor/c_junk.cur");
    curs[SDL_CUR_c_get]=sdl_create_cursor("cursor/c_get.cur");

    return 1;
}

void sdl_set_cursor(int cursor) {
    if (cursor<SDL_CUR_c_only || cursor>SDL_CUR_c_get) return;
    SDL_SetCursor(curs[cursor]);
}


struct prefetch {
    int attick;

    int stx;

    int32_t sprite;
    int8_t sink;
    uint8_t scale;
    int16_t cr,cg,cb,light,sat;
    uint16_t c1,c2,c3,shine;

    uint8_t freeze;
    uint8_t grid;

    int8_t ml,ll,rl,ul,dl;
};

#define MAXPRE (16384)
static struct prefetch pre[MAXPRE];
int pre_in=0,pre_2=0,pre_3=0;

void sdl_pre_add(int attick,int sprite,signed char sink,unsigned char freeze,unsigned char grid,unsigned char scale,char cr,char cg,char cb,char light,char sat,int c1,int c2,int c3,int shine,char ml,char ll,char rl,char ul,char dl) {
    int n;
    long long start;

    if ((pre_in+1)%MAXPRE==pre_3) return; // buffer is full

    if (sprite>MAXSPRITE || sprite<0) {
        note("illegal sprite %d wanted in pre_add",sprite);
        return;
    }

    // Find in texture cache
    // Will allocate a new entry if not found, or return -1 if already in cache
    start=SDL_GetTicks64();
    n=sdl_tx_load(sprite,sink,freeze,grid,scale,cr,cg,cb,light,sat,c1,c2,c3,shine,ml,ll,rl,ul,dl,NULL,0,0,NULL,0,1);
    sdl_time_alloc+=SDL_GetTicks64()-start;
    if (n==-1) return;

    pre[pre_in].stx=n;
    pre[pre_in].attick=attick;
    pre[pre_in].sprite=sprite;
    pre[pre_in].sink=sink;
    pre[pre_in].freeze=freeze;
    pre[pre_in].grid=grid;
    pre[pre_in].scale=scale;
    pre[pre_in].cr=cr;
    pre[pre_in].cg=cg;
    pre[pre_in].cb=cb;
    pre[pre_in].light=light;
    pre[pre_in].sat=sat;
    pre[pre_in].c1=c1;
    pre[pre_in].c2=c2;
    pre[pre_in].c3=c3;
    pre[pre_in].shine=shine;
    pre[pre_in].ml=ml;
    pre[pre_in].ll=ll;
    pre[pre_in].rl=rl;
    pre[pre_in].dl=dl;
    pre[pre_in].ul=ul;

#if 0
    if (1) {
        printf("add  sprite=%5d (%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d) A\n",
                   sprite,
                   pre[pre_in].sink,pre[pre_in].freeze,pre[pre_in].grid,pre[pre_in].scale,
                   pre[pre_in].cr,pre[pre_in].cg,pre[pre_in].cb,pre[pre_in].light,
                   pre[pre_in].sat,pre[pre_in].c1,pre[pre_in].c2,pre[pre_in].c3,
                   pre[pre_in].shine,pre[pre_in].ml,pre[pre_in].ll,pre[pre_in].rl,
                   pre[pre_in].ul,pre[pre_in].dl);
    }
#endif
    pre_in=(pre_in+1)%MAXPRE;

    if (sdl_multi) SDL_SemPost(prework);
}

long long sdl_time_mutex=0;
void sdl_lock(void *a){
    long long start=SDL_GetTicks64();
    SDL_LockMutex(a);
    sdl_time_mutex+=SDL_GetTicks64()-start;
}

#define SDL_LockMutex(a)  sdl_lock(a)

int sdl_pre_2(void) {
    int i,work=0;

    if (pre_in==pre_2) return 0;  // prefetch buffer is empty

    for (i=pre_2; i!=pre_in; i=(i+1)%MAXPRE) {

        if (sdl_multi) SDL_LockMutex(premutex);

        if (pre[i].stx!=STX_NONE && !(sdlt[pre[i].stx].flags&(SF_DIDMAKE|SF_BUSY))) {

            sdlt[pre[i].stx].flags|=SF_BUSY;
            if (sdl_multi) SDL_UnlockMutex(premutex);

            sdl_make(sdlt+pre[i].stx,sdli+pre[i].sprite,2);

            if (sdl_multi) SDL_LockMutex(premutex);
            sdlt[pre[i].stx].flags&=~SF_BUSY;
            sdlt[pre[i].stx].flags|=SF_DIDMAKE;
            if (sdl_multi) SDL_UnlockMutex(premutex);
            work=1;
            break;

        } else {
            if (sdl_multi) SDL_UnlockMutex(premutex);
        }
    }

    if (sdl_multi) SDL_LockMutex(premutex);
    while (pre[pre_2].stx!=STX_NONE && (sdlt[pre[pre_2].stx].flags&SF_DIDMAKE) && pre_in!=pre_2) {
        work=1;
        pre_2=(pre_2+1)%MAXPRE;
    }
    if (sdl_multi) SDL_UnlockMutex(premutex);

    return work;
}

int sdl_pre_3(void) {
    int i;

    if (pre_2==pre_3) return 0;  // prefetch buffer is empty

    i=pre_3;
    if (pre[i].stx!=STX_NONE && !(sdlt[pre[i].stx].flags&SF_DIDTEX))
        sdl_make(sdlt+pre[i].stx,sdli+pre[i].sprite,3);

    pre_3=(pre_3+1)%MAXPRE;

    return 1;

}

int sdl_pre_do(int curtick) {
    long long start;

    start=SDL_GetTicks64();
    while (!sdl_multi && sdl_pre_2()) ;
    sdl_time_pre2+=SDL_GetTicks64()-start;

    start=SDL_GetTicks64();
    sdl_pre_3();
    sdl_time_pre3+=SDL_GetTicks64()-start;

    if (pre_2>=pre_3) return pre_2-pre_3;
    else return MAXPRE+pre_2-pre_3;
}

uint64_t sdl_backgnd_wait=0,sdl_backgnd_work=0;

int sdl_pre_backgnd(void *ptr) {
    uint64_t start;

    while (!quit) {
        start=SDL_GetTicks64();
        SDL_SemWait(prework);
        sdl_backgnd_wait+=SDL_GetTicks64()-start;

        start=SDL_GetTicks64();
        sdl_pre_2();
        sdl_backgnd_work+=SDL_GetTicks64()-start;
    }

    return 0;
}

void sdl_bargraph_add(int dx,unsigned char *data,int val) {
    memmove(data+1,data,dx-1);
    data[0]=val;
}

void sdl_bargraph(int sx,int sy,int dx,unsigned char *data,int x_offset,int y_offset) {
    int n;

    for (n=0; n<dx; n++) {
        if (data[n]>40) SDL_SetRenderDrawColor(sdlren,255,80,80,127);
        else SDL_SetRenderDrawColor(sdlren,80,255,80,127);

        SDL_RenderDrawLine(sdlren,
                           (sx+n+x_offset)*sdl_scale,(sy+y_offset)*sdl_scale,
                           (sx+n+x_offset)*sdl_scale,(sy-data[n]+y_offset)*sdl_scale);
    }
}


/*

for /r "." %a in (0*) do magick mogrify -resize 200% png32:"%~a"

-transparent rgb(255,0,255)
- specify output format (32 bits RGBA)


zip -0 gx1.zip -r -j -q x1
zip -0 gx2.zip -r -j -q x2
zip -0 gx3.zip -r -j -q x3
zip -0 gx4.zip -r -j -q x4

*/
