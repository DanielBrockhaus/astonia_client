/*
 * Part of Astonia Client (c) Daniel Brockhaus. Please read license.txt.
 *
 * convert.exe
 *
 * Will convert any seamless texture (of sufficient size, say 400x400 pixels) into a set of either wall
 * or floor tiles. Walls come in sets of four full height and four cut off. Sprite numbers need to be
 * added to the switch/case in convert_to_wall().
 *
 * Floors come in sets of nine (three by three) and are assumed to start on a multiple of ten (ie. 12000,
 * 12010, etc.). A different logic can be implemented in the switch/case in convert_to_floor()
 *
 * Usage: convert.exe <texture.png> <sprite nr> <w|f>
 *
 * The resulting tiles will be written to ../gfxp/x1..4/spritenr.png
 *
 * Missing directories will be created.
 *
 * Files will be overwritten without prompt!
 *
 * w|f means w for wall tiles or f for floor tiles.
 *
 */

#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <png.h>
#include <zip.h>

#ifdef _WIN32
#include <direct.h>
#define mkdir(pathname,mode) _mkdir(pathname)
#else
#include <sys/stat.h>
#endif

#ifndef STANDALONE
#include <SDL.h>
#include "../../src/sdl/_sdl.h"
#endif

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

void write_png_file(char *filename,uint32_t *pixel,int width,int height,int heightfill);

#ifdef STANDALONE

#define IGET_A(c)       ((((uint32_t)(c))>>24)&0xFF)
#define IGET_R(c)       ((((uint32_t)(c))>>16)&0xFF)
#define IGET_G(c)       ((((uint32_t)(c))>>8)&0xFF)
#define IGET_B(c)       ((((uint32_t)(c))>>0)&0xFF)
#define IRGB(r,g,b)     (((r)<<0)|((g)<<8)|((b)<<16))
#define IRGBA(r,g,b,a)  (((a)<<24)|((r)<<16)|((g)<<8)|((b)<<0))

struct sdl_image {
    uint32_t *pixel;

    uint16_t flags;
    int16_t xres,yres;
    int16_t xoff,yoff;
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
    if (!p->png_ptr) { fclose(fp); fprintf(stderr,"create read\n"); return -1; }

    p->info_ptr=png_create_info_struct(p->png_ptr);
    if (!p->info_ptr) { fclose(fp); png_destroy_read_struct(&p->png_ptr,(png_infopp)NULL,(png_infopp)NULL); fprintf(stderr,"create info1\n"); return -1; }

    if (p->zip) {
        png_set_read_fn(p->png_ptr,zp,png_helper_read);
    } else {
        png_init_io(p->png_ptr,fp);
    }
    png_set_strip_16(p->png_ptr);
    png_read_png(p->png_ptr,p->info_ptr,PNG_TRANSFORM_PACKING,NULL);

    p->row=png_get_rows(p->png_ptr,p->info_ptr);
    if (!p->row) { fclose(fp); png_destroy_read_struct(&p->png_ptr,&p->info_ptr,(png_infopp)NULL); fprintf(stderr,"read row\n"); return -1; }

    p->xres=png_get_image_width(p->png_ptr,p->info_ptr);
    p->yres=png_get_image_height(p->png_ptr,p->info_ptr);

    tmp=png_get_rowbytes(p->png_ptr,p->info_ptr);

    if (tmp==p->xres*3) p->bpp=24;
    else if (tmp==p->xres*4) p->bpp=32;
    else { fclose(fp); png_destroy_read_struct(&p->png_ptr,&p->info_ptr,(png_infopp)NULL); fprintf(stderr,"rowbytes!=xres*4 (%d, %d, %s)",tmp,p->xres,p->filename); return -1; }

    if (png_get_bit_depth(p->png_ptr,p->info_ptr)!=8) { fclose(fp); png_destroy_read_struct(&p->png_ptr,&p->info_ptr,(png_infopp)NULL); fprintf(stderr,"bit depth!=8\n"); return -1; }
    if (png_get_channels(p->png_ptr,p->info_ptr)!=p->bpp/8) { fclose(fp); png_destroy_read_struct(&p->png_ptr,&p->info_ptr,(png_infopp)NULL); fprintf(stderr,"channels!=format\n"); return -1; }

    if (p->zip) zip_fclose(zp);
    else fclose(fp);

    return 0;
}

void png_load_helper_exit(struct png_helper *p) {
    png_destroy_read_struct(&p->png_ptr,&p->info_ptr,(png_infopp)NULL);
}
#endif

int convert_to_wall(struct sdl_image *si,int sprite,int sdl_scale,struct png_helper *p) {
    int r,g,b,a,x,y,sx,sy;
    int xres,yres,tres,cres;
    int cutoff=0,scale=1,off=0;
    double sc;

    if (png_load_helper(p)) {
        fprintf(stderr,"Could not open texture for convert!\n");
        return 0;
    }

    switch (sprite%10) {
        case 0:
        case 1:
        case 2:
        case 3: scale=4; off=(sprite-(sprite/10)*10); break;

        case 4:
        case 5:
        case 6:
        case 7: cutoff=1; scale=4; off=(sprite-(sprite/10)*10-4); break;
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

    sc=2.0*p->xres/xres/scale;

    for (y=0; y<cres; y++) {
        for (x=0; x<xres; x++) {

            sx=x*sc+off*p->xres/scale;
            while (sx>=p->xres) sx-=p->xres;

            sy=y*sc;
            if (x<xres/2) sy-=(x/2+tres)*sc;
            else sy-=((xres-x)/2+tres)*sc;

            while (sy>=p->yres) sy-=p->yres;

            if (y<abs(x-xres/2)/2 || cres-y<abs(x-xres/2)/2) { // corners
                r=b=g=a=0;
            } else if (sy<0) {  // top
                sx=(x-xres/2+y*2)*sc/2+off*p->xres/scale;
                sy=(xres/2-x+y*2)*sc/2;
                if (sx>=0 && sy>=0 && sx<p->xres && sy<p->yres) {
                    if (p->bpp==32) {
                        r=p->row[(sy)][(sx)*4+0];
                        g=p->row[(sy)][(sx)*4+1];
                        b=p->row[(sy)][(sx)*4+2];
                        a=p->row[(sy)][(sx)*4+3];
                    } else {
                        r=p->row[(sy)][(sx)*3+0];
                        g=p->row[(sy)][(sx)*3+1];
                        b=p->row[(sy)][(sx)*3+2];
                        a=255;
                    }
                } else r=g=b=a=0;
            } else {    // sides
                if (p->bpp==32) {
                    r=p->row[(sy)][(sx)*4+0];
                    g=p->row[(sy)][(sx)*4+1];
                    b=p->row[(sy)][(sx)*4+2];
                    a=p->row[(sy)][(sx)*4+3];
                } else {
                    r=p->row[(sy)][(sx)*3+0];
                    g=p->row[(sy)][(sx)*3+1];
                    b=p->row[(sy)][(sx)*3+2];
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

    png_load_helper_exit(p);

#ifdef STANDALONE
    char fname[1024];
    sprintf(fname,"../gfxp/x%d/%08d/%08d.png",sdl_scale,(sprite/1000)*1000,sprite);
    write_png_file(fname,si->pixel,si->xres*sdl_scale,si->yres*sdl_scale,(-si->yoff*2-si->yres)*sdl_scale);
#endif

    return 0;
}

int convert_to_floor(struct sdl_image *si,int sprite,int sdl_scale,struct png_helper *p) {
    int r,g,b,a,x,y,sx,sy,l;
    int xres,yres;
    int scale=1,offx=0,offy=0;
    double scx,scy;

    if (png_load_helper(p)) {
        fprintf(stderr,"Could not open texture for convert!\n");
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

    scx=1.0*p->xres/xres/scale;
    scy=0.5*p->yres/yres/scale;

    for (y=0; y<yres; y++) {
        for (x=0; x<xres; x++) {

            if (x-xres/2>=0) l=(x-xres/2)/2;
            else l=(xres/2-x-1)/2;

            if (y<l || yres-y<=l) { // corners
                r=b=g=a=0;
            } else {
                sx=(x-xres/2+y*2)*scx+offx*(p->xres)/scale;
                sy=(xres/2-x+y*2)*scy+offy*(p->yres)/scale;

                if (sx>=0 && sy>=0 && sx<p->xres && sy<p->yres) {
                    if (p->bpp==32) {
                        r=p->row[(sy)][(sx)*4+0];
                        g=p->row[(sy)][(sx)*4+1];
                        b=p->row[(sy)][(sx)*4+2];
                        a=p->row[(sy)][(sx)*4+3];
                    } else {
                        r=p->row[(sy)][(sx)*3+0];
                        g=p->row[(sy)][(sx)*3+1];
                        b=p->row[(sy)][(sx)*3+2];
                        a=255;
                    }
                } else r=g=b=a=0;
            }
            si->pixel[x+y*xres]=IRGBA(r,g,b,a);
        }
    }

    png_load_helper_exit(p);

#ifdef STANDALONE
    char fname[1024];
    sprintf(fname,"../gfxp/x%d/%08d/%08d.png",sdl_scale,(sprite/1000)*1000,sprite);
    write_png_file(fname,si->pixel,si->xres*sdl_scale,si->yres*sdl_scale,0);
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


#ifdef STANDALONE
int main(int argc,char *args[]) {
    struct sdl_image si;
    struct png_helper p;
    int n,s,sprite;
    char buf[256];

    if (argc!=4 || (tolower(args[3][0])!='w' && tolower(args[3][0])!='f')) {
        printf("%s: <texture.png> <sprite nr> <w|f>\n",args[0]);
        return 1;
    }

    p.zip=NULL;
    p.filename=args[1];
    sprite=atoi(args[2]);

    mkdir("../gfxp",0777);
    for (s=1; s<5; s++) {
        sprintf(buf,"../gfxp/x%d",s);
        mkdir(buf,0777);
        sprintf(buf,"../gfxp/x%d/%08d",s,(sprite/1000)*1000);
        mkdir(buf,0777);
    }

    if (tolower(args[3][0])=='w') {
        for (s=1; s<5; s++) {
            for (n=0; n<8; n++) {
                convert_to_wall(&si,sprite+n,s,&p);
            }
        }
    } else if (tolower(args[3][0])=='f') {
        for (s=1; s<5; s++) {
            for (n=0; n<9; n++) {
                convert_to_floor(&si,sprite+n,s,&p);
            }
        }
    }
}
#endif

