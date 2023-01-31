/*
 * Part of Astonia Client (c) Daniel Brockhaus. Please read license.txt.
 */

#include <stdint.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <png.h>
#include <zip.h>

#include "../../src/astonia.h"
#include "../../src/sdl.h"
#include "../../src/sdl/_sdl.h"

void write_png_file(char *filename,uint32_t *pixel,int width,int height,int heightfill);

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

int convert_to_wall(struct sdl_image *si,int sprite) {
    struct png_helper p;
    int r,g,b,a,x,y,sx,sy;
    int xres,yres,tres,cres;
    int cutoff=0,scale=1,off=0;
    double sc;

    p.zip=NULL;
    p.filename="C:/Users/User/Downloads/texture5.png";
    if (png_load_helper(&p)) {
        fail("Could not open texture for convert!");
        return 0;
    }

    switch (sprite) {
        case 14030:
        case 14031:
        case 14032:
        case 14033: scale=4; off=(sprite-14030); break;

        case 14034:
        case 14035:
        case 14036:
        case 14037: cutoff=1; scale=4; off=(sprite-14034); break;
    }

    si->flags=1;
    si->xoff=-20;
    si->yoff=-72;
    si->xres=40;
    si->yres=82;

    xres=si->xres*sdl_scale;
    yres=si->yres*sdl_scale;
    tres=xres/4;

    if (cutoff) {
        cres=30*sdl_scale;
        si->yres=cres/sdl_scale;
        si->yoff=-20;
        si->pixel=malloc(xres*cres*sizeof(uint32_t));
    } else {
        cres=yres;
        si->pixel=malloc(xres*yres*sizeof(uint32_t));
    }

    sc=2.0*p.xres/xres/scale;

    for (y=0; y<cres; y++) {
        for (x=0; x<xres; x++) {

            sx=x*sc+off*p.xres/scale;
            while (sx>=p.xres) sx-=p.xres;

            sy=y*sc;
            if (x<xres/2) sy-=(x/2+tres)*sc;
            else sy-=((xres-x)/2+tres)*sc;

            while (sy>=p.yres) sy-=p.yres;

            if (y<abs(x-xres/2)/2 || cres-y<abs(x-xres/2)/2) { // corners
                r=b=g=a=0;
            } else if (sy<0) {
                sx=(x-xres/2+y*2)*sc/2+off*p.xres/scale;
                sy=(xres/2-x+y*2)*sc/2;
                if (sx>=0 && sy>=0 && sx<p.xres && sy<p.yres) {
                    if (p.bpp==32) {
                        r=p.row[(sy)][(sx)*4+0];
                        g=p.row[(sy)][(sx)*4+1];
                        b=p.row[(sy)][(sx)*4+2];
                        a=p.row[(sy)][(sx)*4+3];
                    } else {
                        r=p.row[(sy)][(sx)*3+0];
                        g=p.row[(sy)][(sx)*3+1];
                        b=p.row[(sy)][(sx)*3+2];
                        a=255;
                    }
                } else r=g=b=a=0;
            } else {
                if (p.bpp==32) {
                    r=p.row[(sy)][(sx)*4+0];
                    g=p.row[(sy)][(sx)*4+1];
                    b=p.row[(sy)][(sx)*4+2];
                    a=p.row[(sy)][(sx)*4+3];
                } else {
                    r=p.row[(sy)][(sx)*3+0];
                    g=p.row[(sy)][(sx)*3+1];
                    b=p.row[(sy)][(sx)*3+2];
                    a=255;
                }
                if (x>=xres/2) {
                    r*=0.6;
                    g*=0.6;
                    b*=0.6;
                } else {
                    r*=0.8;
                    g*=0.8;
                    b*=0.8;
                }
            }

            si->pixel[x+y*xres]=IRGBA(r,g,b,a);
        }
    }

    png_load_helper_exit(&p);

#if 0
    char fname[1024];
    sprintf(fname,"../x%d_%08d_%08d.png",sdl_scale,(sprite/1000)*1000,sprite);
    write_png_file(fname,si->pixel,si->xres*sdl_scale,si->yres*sdl_scale,(-si->yoff*2-si->yres)*sdl_scale);
#endif

    return 0;
}

int convert_to_floor(struct sdl_image *si,int sprite) {
    struct png_helper p;
    int r,g,b,a,x,y,sx,sy,l;
    int xres,yres;
    int scale=1,offx=0,offy=0;
    double scx,scy;

    p.zip=NULL;
    if (sprite>=12000 && sprite<=12008) p.filename="C:/Users/User/Downloads/texture8.png";
    else p.filename="C:/Users/User/Downloads/texture7.png";
    if (png_load_helper(&p)) {
        fail("Could not open texture for convert!");
        return 0;
    }

    switch (sprite%10) {
        case 0: scale=3; offx=0; offy=0; break;
        case 1: scale=3; offx=1; offy=0; break;
        case 2: scale=3; offx=2; offy=0; break;
        case 3: scale=3; offx=0; offy=1; break;
        case 4: scale=3; offx=1; offy=1; break;
        case 5: scale=3; offx=2; offy=1; break;
        case 6: scale=3; offx=0; offy=2; break;
        case 7: scale=3; offx=1; offy=2; break;
        case 8: scale=3; offx=2; offy=2; break;
    }

    si->flags=1;
    si->xoff=-20;
    si->yoff=-10;
    si->xres=40;
    si->yres=20;

    xres=si->xres*sdl_scale;
    yres=si->yres*sdl_scale;

    si->pixel=malloc(xres*yres*sizeof(uint32_t));

    scx=1.00*p.xres/(xres+2)/scale;
    scy=0.49*p.yres/(yres)/scale;

    for (y=0; y<yres; y++) {
        for (x=0; x<xres; x++) {

            if (x-xres/2>=0) l=(x-xres/2)/2;
            else l=(xres/2-x-1)/2;

            if (y<l || yres-y<=l) { // corners
                r=b=g=a=0;
            } else {
                sx=(x-xres/2+y*2)*scx+offx*p.xres/scale+3;
                sy=(xres/2-x+y*2)*scy+offy*p.yres/scale+1;
#if 0
                if (offx==2 && offy==2) {
                    printf("sx=%d,sy=%d (%d,%d) of (%d,%d) [%d,%d]\n",sx,sy,x,y,xres,yres,p.xres,p.yres);
                    fflush(stdout);
                }
#endif
                if (sx<0) {
                    printf("sx=%d,sy=%d (%d,%d) of (%d,%d) [%d,%d]\n",sx,sy,x,y,xres,yres,p.xres,p.yres);
                    fflush(stdout);
                    sx=0;
                }
                if (sy<0) {
                    printf("sx=%d,sy=%d (%d,%d) of (%d,%d) [%d,%d]\n",sx,sy,x,y,xres,yres,p.xres,p.yres);
                    fflush(stdout);
                    sy=0;
                }
                if (sx>=p.xres) {
                    printf("sx=%d,sy=%d (%d,%d) of (%d,%d) [%d,%d]\n",sx,sy,x,y,xres,yres,p.xres,p.yres);
                    fflush(stdout);
                    sx=p.xres-1;
                }
                if (sy>=p.yres) {
                    printf("sx=%d,sy=%d (%d,%d) of (%d,%d) [%d,%d]\n",sx,sy,x,y,xres,yres,p.xres,p.yres);
                    fflush(stdout);
                    sy=p.yres-1;
                }

                if (sx>=0 && sy>=0 && sx<p.xres && sy<p.yres) {
                    if (p.bpp==32) {
                        r=p.row[(sy)][(sx)*4+0];
                        g=p.row[(sy)][(sx)*4+1];
                        b=p.row[(sy)][(sx)*4+2];
                        a=p.row[(sy)][(sx)*4+3];
                    } else {
                        r=p.row[(sy)][(sx)*3+0];
                        g=p.row[(sy)][(sx)*3+1];
                        b=p.row[(sy)][(sx)*3+2];
                        a=255;
                    }
                } else r=g=b=a=255;
            }
            si->pixel[x+y*xres]=IRGBA(r,g,b,a);
        }
    }

    png_load_helper_exit(&p);

#if 0
    char fname[1024];
    sprintf(fname,"../x%d_%08d_%08d.png",sdl_scale,(sprite/1000)*1000,sprite);
    write_png_file(fname,si->pixel,si->xres*sdl_scale,si->yres*sdl_scale);
#endif

    return 0;
}

void write_png_file(char *filename,uint32_t *pixel,int width,int height,int heightfill) {
  int x,y;
  unsigned char *line;

  line=malloc(width*sizeof(uint32_t));

  FILE *fp = fopen(filename, "wb");
  if(!fp) abort();

  png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png) abort();

  png_infop info = png_create_info_struct(png);
  if (!info) abort();

  if (setjmp(png_jmpbuf(png))) abort();

  png_init_io(png, fp);

  // Output is 8bit depth, RGBA format.
  png_set_IHDR(
    png,
    info,
    width, height+heightfill,
    8,
    PNG_COLOR_TYPE_RGB_ALPHA,
    PNG_INTERLACE_NONE,
    PNG_COMPRESSION_TYPE_DEFAULT,
    PNG_FILTER_TYPE_DEFAULT
  );
  png_write_info(png, info);

  // To remove the alpha channel for PNG_COLOR_TYPE_RGB format,
  // Use png_set_filler().
  //png_set_filler(png, 0, PNG_FILLER_AFTER);

  for (y=0; y<height; y++) {
      for (x=0; x<width; x++) {
          line[x*4+0]=IGET_R(pixel[x+y*width]);
          line[x*4+1]=IGET_G(pixel[x+y*width]);
          line[x*4+2]=IGET_B(pixel[x+y*width]);
          line[x*4+3]=IGET_A(pixel[x+y*width]);
      }
      png_write_row(png,line);
  }
  for (y=0; y<heightfill; y++) {
      for (x=0; x<width; x++) {
          line[x*4+0]=0;
          line[x*4+1]=0;
          line[x*4+2]=0;
          line[x*4+3]=0;
      }
      png_write_row(png,line);
  }

  png_write_end(png, NULL);

  fclose(fp);

  png_destroy_write_struct(&png, &info);

  free(line);
}

