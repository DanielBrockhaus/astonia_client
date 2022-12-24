#include <stdint.h>
#include <windows.h>
#include <SDL2/SDL.h>
#include <png.h>

#include "main.h"
#include "sdl.h"

SDL_Window *sdlwnd;
SDL_Renderer *sdlren;

extern int gfx_force_png;

struct sdl_image {
    int flags;
    int xres,yres;
    int xoff,yoff;
    uint32_t *pixel;
};

struct sdl_image *sdli=NULL;


int sdl_init(int width,int height,char *title) {
    int len;

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
    sdli=malloc(len);
    if (!sdli) return fail("Out of memory in sdl_init");
    bzero(sdli,len);

    return 1;
}

int sdl_clear(void) {
    SDL_SetRenderDrawColor(sdlren,0,0,0,255);
    SDL_RenderClear(sdlren);
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

void sdl_blit(int sprite,int scrx,int scry) {
    struct sdl_image *si;
    SDL_Rect r;

    si=sdli+sprite;

    SDL_Texture *texture = SDL_CreateTexture(sdlren, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, si->xres,si->yres);
    if (!texture) printf("SDL_texture Error: %s",SDL_GetError());
    SDL_UpdateTexture(texture,NULL,si->pixel,si->xres*sizeof(uint32_t));

    r.x=scrx; r.w=si->xres;
    r.y=scry; r.h=si->yres;

    SDL_SetTextureBlendMode(texture,SDL_BLENDMODE_BLEND);
    SDL_RenderCopy(sdlren, texture, NULL, &r);

    SDL_DestroyTexture(texture);
}

