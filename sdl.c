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
    SDL_SetRenderDrawColor(sdlren,0,255,0,255);
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

    //sc_make(&systemcache[sidx],&imagecache[iidx].image,sink,freeze,grid,scale,cr,cg,cb,light,sat,c1,c2,c3,shine,ml,ll,rl,ul,dl);

    SDL_Texture *texture = SDL_CreateTexture(sdlren, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, sdli[sprite].xres,sdli[sprite].yres);
    if (!texture) printf("SDL_texture Error: %s",SDL_GetError());
    SDL_UpdateTexture(texture,NULL,sdli[sprite].pixel,sdli[sprite].xres*sizeof(uint32_t));
    SDL_SetTextureBlendMode(texture,SDL_BLENDMODE_BLEND);

    mem_tex+=sdli[sprite].xres*sdli[sprite].yres*sizeof(uint32_t);
    sdlt[stx].tex=texture;
    sdlt[stx].xres=sdli[sprite].xres;
    sdlt[stx].yres=sdli[sprite].yres;

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


void sdl_blit(int stx,int scrx,int scry) {
    SDL_Rect r;

    r.x=scrx; r.w=sdlt[stx].xres;
    r.y=scry; r.h=sdlt[stx].yres;

    SDL_RenderCopy(sdlren,sdlt[stx].tex,NULL,&r);
}

