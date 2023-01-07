/*
 * Part of Astonia Client (c) Daniel Brockhaus. Please read license.txt.
 */

#include <windows.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <stdio.h>

#include "main.h"
#include "dd.h"
#include "client.h"
#include "sdl.h"

DDFONT *fonta_shaded=NULL;
DDFONT *fonta_framed=NULL;

DDFONT *fontb_shaded=NULL;
DDFONT *fontb_framed=NULL;

DDFONT *fontc_shaded=NULL;
DDFONT *fontc_framed=NULL;

float fontdim_a[256];
float fontdim_b[256];
float fontdim_c[256];
float *fontdim=fontdim_a;

void dd_create_font(void);
void dd_init_text(void);


// direct x basics //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// TODO: these should get used again for light calculations!
int dd_gamma=8;
int dd_lighteffect=16;
int newlight=0;

float mouse_scale=1.0f;      // mouse input needs to be scaled by this factor because the display window is stretched

int x_offset,y_offset;
int x_max,y_max;

int clipsx,clipsy,clipex,clipey;
int clipstore[32][4],clippos=0;

void dd_push_clip(void) {
    if (clippos>=32) return;

    clipstore[clippos][0]=clipsx;
    clipstore[clippos][1]=clipsy;
    clipstore[clippos][2]=clipex;
    clipstore[clippos][3]=clipey;
    clippos++;
}

void dd_pop_clip(void) {
    if (clippos==0) return;

    clippos--;
    clipsx=clipstore[clippos][0];
    clipsy=clipstore[clippos][1];
    clipex=clipstore[clippos][2];
    clipey=clipstore[clippos][3];
}

void dd_more_clip(int sx,int sy,int ex,int ey) {
    if (sx>clipsx) clipsx=sx;
    if (sy>clipsy) clipsy=sy;
    if (ex<clipex) clipex=ex;
    if (ey<clipey) clipey=ey;
}

void dd_set_clip(int sx,int sy,int ex,int ey) {
    clipsx=sx;
    clipsy=sy;
    clipex=ex;
    clipey=ey;
}

void dd_get_client_info(struct client_info *ci) {
    static MEMORYSTATUS memstat;

    bzero(&memstat,sizeof(memstat));
    memstat.dwLength=sizeof(memstat);
    GlobalMemoryStatus(&memstat);

    ci->vidmemtotal=0;
    ci->vidmemfree=0;

    ci->systemtotal=memstat.dwTotalPhys;
    ci->systemfree=memstat.dwAvailPhys;
}

int dd_init(int width,int height) {
    // set the clipping to the maximum possible
    clippos=0;
    clipsx=0;
    clipsy=0;
    clipex=XRES;
    clipey=YRES;

    // initialize the gfx loading stuff - TODO: call this in dd_init_cache();
    dd_create_font();
    dd_init_text();

    return 0;
}

int dd_exit(void) {
    return 0;
}

int dd_copysprite_fx(DDFX *ddfx,int scrx,int scry) {
    int stx;

    PARANOIA(if (!ddfx) paranoia("dd_copysprite_fx: ddfx=NULL"); )
    PARANOIA(if (ddfx->light<0 || ddfx->light>16) paranoia("dd_copysprite_fx: ddfx->light=%d",ddfx->light); )
    PARANOIA(if (ddfx->freeze<0 || ddfx->freeze>=DDFX_MAX_FREEZE) paranoia("dd_copysprite_fx: ddfx->freeze=%d",ddfx->freeze); )

    stx=sdl_tx_load(ddfx->sprite,
                 ddfx->sink,
                 ddfx->freeze,
                 ddfx->grid,
                 ddfx->scale,
                 ddfx->cr,
                 ddfx->cg,
                 ddfx->cb,
                 ddfx->clight,
                 ddfx->sat,
                 ddfx->c1,
                 ddfx->c2,
                 ddfx->c3,
                 ddfx->shine,
                 ddfx->ml,
                 ddfx->ll,
                 ddfx->rl,
                 ddfx->ul,
                 ddfx->dl);

    if (stx==-1) return 0;

    // shift position according to align
    if (ddfx->align==DD_OFFSET) {
        scrx+=sdlt_xoff(stx);
        scry+=sdlt_yoff(stx);
    } else if (ddfx->align==DD_CENTER) {
        scrx-=sdlt_xres(stx)/2;
        scry-=sdlt_yres(stx)/2;
    }

    // add the additional cliprect
    if (ddfx->clipsx!=ddfx->clipex || ddfx->clipsy!=ddfx->clipey) {
        dd_push_clip();
        if (ddfx->clipsx!=ddfx->clipex) dd_more_clip(scrx-sdlt_xoff(stx)+ddfx->clipsx,clipsy,scrx-sdlt_xoff(stx)+ddfx->clipex,clipey);
        if (ddfx->clipsy!=ddfx->clipey) dd_more_clip(clipsx,scry-sdlt_yoff(stx)+ddfx->clipsy,clipex,scry-sdlt_yoff(stx)+ddfx->clipey);
    }

    // blit it
    sdl_blit(stx,scrx,scry,clipsx,clipsy,clipex,clipey,x_offset,y_offset);

    // remove additional cliprect
    if (ddfx->clipsx!=ddfx->clipex || ddfx->clipsy!=ddfx->clipey) dd_pop_clip();

    return 1;
}

void dd_copysprite_callfx(int sprite,int scrx,int scry,int light,int ml,int grid,int align) {
    DDFX ddfx;

    bzero(&ddfx,sizeof(DDFX));

    ddfx.sprite=sprite;
    if (light<1000) ddfx.light=DDFX_NLIGHT;
    ddfx.grid=grid;
    ddfx.align=align;

    ddfx.ml=ddfx.ll=ddfx.rl=ddfx.ul=ddfx.dl=ml;
    ddfx.sink=0;
    ddfx.scale=100;
    ddfx.cr=ddfx.cg=ddfx.cb=ddfx.clight=ddfx.sat=0;
    ddfx.c1=ddfx.c2=ddfx.c3=ddfx.shine=0;

    dd_copysprite_fx(&ddfx,scrx,scry);
}

void dd_copysprite(int sprite,int scrx,int scry,int light,int align) {
    DDFX ddfx;

    bzero(&ddfx,sizeof(DDFX));

    ddfx.sprite=sprite;
    ddfx.light=DDFX_NLIGHT;
    ddfx.align=align;

    ddfx.ml=ddfx.ll=ddfx.rl=ddfx.ul=ddfx.dl=light;
    ddfx.sink=0;
    ddfx.scale=100;
    ddfx.cr=ddfx.cg=ddfx.cb=ddfx.clight=ddfx.sat=0;
    ddfx.c1=ddfx.c2=ddfx.c3=ddfx.shine=0;

    dd_copysprite_fx(&ddfx,scrx,scry);
}

void dd_rect(int sx,int sy,int ex,int ey,unsigned short int color) {
    sdl_rect(sx,sy,ex,ey,color,clipsx,clipsy,clipex,clipey,x_offset,y_offset);
}

void dd_shaded_rect(int sx,int sy,int ex,int ey) {
    sdl_shaded_rect(sx,sy,ex,ey,0xffe0,clipsx,clipsy,clipex,clipey,x_offset,y_offset);
}

void dd_line(int fx,int fy,int tx,int ty,unsigned short col) {
    sdl_line(fx,fy,tx,ty,col,clipsx,clipsy,clipex,clipey,x_offset,y_offset);
}

void dd_display_strike(int fx,int fy,int tx,int ty) {
    int mx,my;
    int dx,dy,d,l;
    unsigned short col;

    dx=abs(tx-fx);
    dy=abs(ty-fy);

    mx=(fx+tx)/2+15-rrand(30);
    my=(fy+ty)/2+15-rrand(30);

    if (dx>=dy) {
        for (d=-4; d<5; d++) {
            l=(4-abs(d))*4;
            col=IRGB(l,l,31);
            dd_line(fx,fy,mx,my+d,col);
            dd_line(mx,my+d,tx,ty,col);
        }
    } else {
        for (d=-4; d<5; d++) {
            l=(4-abs(d))*4;
            col=IRGB(l,l,31);
            dd_line(fx,fy,mx+d,my,col);
            dd_line(mx+d,my,tx,ty,col);
        }
    }
}

void dd_draw_curve(int cx,int cy,int nr,int size,unsigned short col) {
    int n,x,y;

    for (n=nr*90; n<nr*90+90; n+=4) {
        x=sin(n/360.0*M_PI*2)*size+cx;
        y=cos(n/360.0*M_PI*2)*size*2/3+cy;

        if (x<clipsx) continue;
        if (y<clipsy) continue;
        if (x>=clipex) continue;
        if (y+10>=clipey) continue;

        sdl_pixel(x,y,col,x_offset,y_offset);
        sdl_pixel(x,y+5,col,x_offset,y_offset);
        sdl_pixel(x,y+10,col,x_offset,y_offset);
    }
}

void dd_display_pulseback(int fx,int fy,int tx,int ty) {
    int mx,my;
    int dx,dy,d,l;
    unsigned short col;

    dx=abs(tx-fx);
    dy=abs(ty-fy);

    mx=(fx+tx)/2+15-rrand(30);
    my=(fy+ty)/2+15-rrand(30);

    if (dx>=dy) {
        for (d=-4; d<5; d++) {
            l=(4-abs(d))*4;
            col=IRGB(l,31,l);
            dd_line(fx,fy,mx,my+d,col);
            dd_line(mx,my+d,tx,ty,col);
        }
    } else {
        for (d=-4; d<5; d++) {
            l=(4-abs(d))*4;
            col=IRGB(l,31,l);
            dd_line(fx,fy,mx+d,my,col);
            dd_line(mx+d,my,tx,ty,col);
        }
    }
}

// text

int dd_textlength(int flags,const char *text) {
    DDFONT *font;
    int x;
    const char *c;

    if (flags&DD_SMALL) { font=fontb; }
    else if (flags&DD_BIG) { font=fontc; }
    else { font=fonta; }

    for (x=0,c=text; *c && *c!=DDT; c++) x+=font[*c].dim;

    return (int)(x+0.5f);
}

int dd_textlen(int flags,const char *text,int n) {
    DDFONT *font;
    int x;
    const char *c;

    if (n<0) return dd_textlength(flags,text);

    if (flags&DD_SMALL) { font=fontb; }
    else if (flags&DD_BIG) { font=fontc; }
    else { font=fonta; }

    for (x=0,c=text; *c && *c!=DDT && n; c++,n--)  x+=font[*c].dim;

    return (int)(x+0.5f);
}

int dd_drawtext(int sx,int sy,unsigned short int color,int flags,const char *text) {
    DDFONT *font;

    if (flags&DD__SHADEFONT) {
        if (flags&DD_SMALL) font=fontb_shaded;
        else if (flags&DD_BIG) font=fontc_shaded;
        else font=fonta_shaded;
    } else if (flags&DD__FRAMEFONT) {
        if (flags&DD_SMALL) font=fontb_framed;
        else if (flags&DD_BIG) font=fontc_framed;
        else font=fonta_framed;
    } else {
        if (flags&DD_SMALL) font=fontb;
        else if (flags&DD_BIG) font=fontc;
        else font=fonta;
    }
    if (!font) return 42;

    if (flags&DD_SHADE) {
        dd_drawtext(sx-1,sy-1,IRGB(0,0,0),DT_LEFT|(flags&(DD_SMALL|DD_BIG|DD_CENTER|DD_RIGHT))|DD__SHADEFONT,text);
    } else if (flags&DD_FRAME) {
        dd_drawtext(sx-1,sy-1,IRGB(0,0,0),DT_LEFT|(flags&(DD_SMALL|DD_BIG|DD_CENTER|DD_RIGHT))|DD__FRAMEFONT,text);
    }

    sx=sdl_drawtext(sx,sy,color,flags,text,font,clipsx,clipsy,clipex,clipey,x_offset,y_offset);

    return sx;
}

int dd_drawtext_break(int x,int y,int breakx,unsigned short color,int flags,const char *ptr) {
    char buf[256];
    int xp,n;
    float size;

    xp=x;

    while (*ptr) {
        while (*ptr==' ') ptr++;

        for (n=0; n<256 && *ptr && *ptr!=' '; buf[n++]=*ptr++);
        buf[n]=0;

        size=dd_textlength(flags,buf);
        if (xp+size>breakx) {
            xp=x; y+=10;
        }
        dd_drawtext(xp,y,color,flags,buf);
        xp+=size+4;
    }
    return y+10;
}


void dd_pixel(int x,int y,unsigned short col) {
    sdl_pixel(x,y,col,x_offset,y_offset);
}

int dd_drawtext_fmt(int sx,int sy,unsigned short int color,int flags,const char *format,...) {
    char buf[1024];
    va_list va;

    va_start(va,format);
    vsprintf(buf,format,va);
    va_end(va);

    return dd_drawtext(sx,sy,color,flags,buf);
}


static int bless_init=0;
static int bless_sin[36];
static int bless_cos[36];
static int bless_hight[200];

void dd_draw_bless_pix(int x,int y,int nr,int color,int front) {
    int sy;

    sy=bless_sin[nr%36];
    if (front && sy<0) return;
    if (!front && sy>=0) return;

    x+=bless_cos[nr%36];
    y=y+sy+bless_hight[nr%200];

    if (x<clipsx || x>=clipex || y<clipsy || y>=clipey) return;

    sdl_pixel(x,y,color,x_offset,y_offset);
}

void dd_draw_rain_pix(int x,int y,int nr,int color,int front) {
    int sy;

    x+=((nr/30)%30)+15;
    sy=((nr/330)%20)+10;
    if (front && sy<0) return;
    if (!front && sy>=0) return;

    y=y+sy-((nr*2)%60)-60;

    if (x<clipsx || x>=clipex || y<clipsy || y>=clipey) return;

    sdl_pixel(x,y,color,x_offset,y_offset);
}

void dd_draw_bless(int x,int y,int ticker,int strength,int front) {
    int step,nr;
    double light;

    if (!bless_init) {
        for (nr=0; nr<36; nr++) {
            bless_sin[nr]=sin((nr%36)/36.0*M_PI*2)*8;
            bless_cos[nr]=cos((nr%36)/36.0*M_PI*2)*16;
        }
        for (nr=0; nr<200; nr++) {
            bless_hight[nr]=-20+sin((nr%200)/200.0*M_PI*2)*20;
        }
        bless_init=1;
    }

    if (ticker>62) light=1.0;
    else light=(ticker)/62.0;

    for (step=0; step<strength*10; step+=17) {
        dd_draw_bless_pix(x,y,ticker+step+0,IRGB(((int)(24*light)),((int)(24*light)),((int)(31*light))),front);
        dd_draw_bless_pix(x,y,ticker+step+1,IRGB(((int)(20*light)),((int)(20*light)),((int)(28*light))),front);
        dd_draw_bless_pix(x,y,ticker+step+2,IRGB(((int)(16*light)),((int)(16*light)),((int)(24*light))),front);
        dd_draw_bless_pix(x,y,ticker+step+3,IRGB(((int)(12*light)),((int)(12*light)),((int)(20*light))),front);
        dd_draw_bless_pix(x,y,ticker+step+4,IRGB(((int)(8*light)),((int)(8*light)),((int)(16*light))),front);
    }
}

void dd_draw_potion(int x,int y,int ticker,int strength,int front) {
    int step,nr;
    double light;

    if (!bless_init) {
        for (nr=0; nr<36; nr++) {
            bless_sin[nr]=sin((nr%36)/36.0*M_PI*2)*8;
            bless_cos[nr]=cos((nr%36)/36.0*M_PI*2)*16;
        }
        for (nr=0; nr<200; nr++) {
            bless_hight[nr]=-20+sin((nr%200)/200.0*M_PI*2)*20;
        }
        bless_init=1;
    }


    if (ticker>62) light=1.0;
    else light=(ticker)/62.0;

    for (step=0; step<strength*10; step+=17) {
        dd_draw_bless_pix(x,y,ticker+step+0,IRGB(((int)(31*light)),((int)(24*light)),((int)(24*light))),front);
        dd_draw_bless_pix(x,y,ticker+step+1,IRGB(((int)(28*light)),((int)(20*light)),((int)(20*light))),front);
        dd_draw_bless_pix(x,y,ticker+step+2,IRGB(((int)(24*light)),((int)(16*light)),((int)(16*light))),front);
        dd_draw_bless_pix(x,y,ticker+step+3,IRGB(((int)(20*light)),((int)(12*light)),((int)(12*light))),front);
        dd_draw_bless_pix(x,y,ticker+step+4,IRGB(((int)(16*light)),((int)(8*light)),((int)(8*light))),front);
    }
}

void dd_draw_rain(int x,int y,int ticker,int strength,int front) {
    int step;

    for (step=-(strength*100); step<0; step+=237) {
        dd_draw_rain_pix(x,y,-ticker+step+0,IRGB(31,24,16),front);
        dd_draw_rain_pix(x,y,-ticker+step+1,IRGB(24,16,8),front);
        dd_draw_rain_pix(x,y,-ticker+step+2,IRGB(16,8,0),front);
    }
}

void dd_create_letter(unsigned char *rawrun,int sx,int sy,int val,char letter[16][16]) {
    int x=sx,y=sy;

    while (*rawrun!=255) {
        if (*rawrun==254) {
            y++;
            x=sx;
            rawrun++;
            continue;
        }

        x+=*rawrun++;

        letter[y][x]=val;
    }
}

char* dd_create_rawrun(char letter[16][16]) {
    char *ptr,*fon,*last;
    int x,y,step;

    last=fon=ptr=xmalloc(256,MEM_TEMP);

    for (y=3; y<16; y++) {
        step=0;
        for (x=3; x<16; x++) {
            if (letter[y][x]==2) {
                *ptr++=step; last=ptr;
                step=1;
            } else step++;
        }
        *ptr++=254;
    }
    ptr=last;
    *ptr++=255;

    fon=xrealloc(fon,ptr-fon,MEM_GLOB);
    return fon;
}

void create_shade_font(DDFONT *src,DDFONT *dst) {
    char letter[16][16];
    int c;

    for (c=0; c<128; c++) {
        bzero(letter,sizeof(letter));
        dd_create_letter(src[c].raw,4,5,2,letter);
        dd_create_letter(src[c].raw,5,4,2,letter);
        dd_create_letter(src[c].raw,4,4,1,letter);
        dst[c].raw=dd_create_rawrun(letter);
        dst[c].dim=src[c].dim;
    }
}

void create_frame_font(DDFONT *src,DDFONT *dst) {
    char letter[16][16];
    int c;

    for (c=0; c<128; c++) {
        bzero(letter,sizeof(letter));
        dd_create_letter(src[c].raw,5,4,2,letter);
        dd_create_letter(src[c].raw,3,4,2,letter);
        dd_create_letter(src[c].raw,4,5,2,letter);
        dd_create_letter(src[c].raw,4,3,2,letter);
        dd_create_letter(src[c].raw,5,5,2,letter);
        dd_create_letter(src[c].raw,5,3,2,letter);
        dd_create_letter(src[c].raw,3,5,2,letter);
        dd_create_letter(src[c].raw,3,3,2,letter);
        dd_create_letter(src[c].raw,4,4,1,letter);
        dst[c].raw=dd_create_rawrun(letter);
        dst[c].dim=src[c].dim;
    }
}

void dd_create_font(void) {
    if (fonta_shaded) return;

    fonta_shaded=xmalloc(sizeof(DDFONT)*128,MEM_GLOB); create_shade_font(fonta,fonta_shaded);
    fontb_shaded=xmalloc(sizeof(DDFONT)*128,MEM_GLOB); create_shade_font(fontb,fontb_shaded);
    fontc_shaded=xmalloc(sizeof(DDFONT)*128,MEM_GLOB); create_shade_font(fontc,fontc_shaded);

    fonta_framed=xmalloc(sizeof(DDFONT)*128,MEM_GLOB); create_frame_font(fonta,fonta_framed);
    fontb_framed=xmalloc(sizeof(DDFONT)*128,MEM_GLOB); create_frame_font(fontb,fontb_framed);
    fontc_framed=xmalloc(sizeof(DDFONT)*128,MEM_GLOB); create_frame_font(fontc,fontc_framed);

}

DDFONT *textfont=fonta;
int textdisplay_dy=10;

int dd_drawtext_char(int sx,int sy,int c,unsigned short int color) {

    if (c>127 || c<0) return 0;

    return sdl_drawtext(sx,sy,color,0,(char*)&c,textfont,clipsx,clipsy,clipex,clipey,x_offset,y_offset)-sx;
}

int dd_text_len(const char *text) {
    int x;
    const char *c;

    for (x=0,c=text; *c; c++) x+=textfont[*c].dim;

    return (int)(x+0.5f);
}

int dd_char_len(char c) {
    return textfont[c].dim;
}



// ---------------------> Chat Window <-----------------------------
#define MAXTEXTLINES		256
#define MAXTEXTLETTERS		256

#define TEXTDISPLAY_X		230
#define TEXTDISPLAY_Y		438

#define TEXTDISPLAY_DY		(textdisplay_dy)

#define TEXTDISPLAY_SX		396
#define TEXTDISPLAY_SY		150

#define TEXTDISPLAYLINES	(TEXTDISPLAY_SY/TEXTDISPLAY_DY)

int textnextline=0,textdisplayline=0;

struct letter {
    char c;
    unsigned char color;
    unsigned char link;
};

struct letter *text=NULL;

unsigned short palette[256];

void dd_init_text(void) {
    text=xcalloc(sizeof(struct letter)*MAXTEXTLINES*MAXTEXTLETTERS,MEM_GLOB);
    palette[0]=IRGB(31,31,31);    // normal white text (talk, game messages)
    palette[1]=IRGB(16,16,16);    // dark gray text (now entering ...)
    palette[2]=IRGB(16,31,16);    // light green (normal chat)
    palette[3]=IRGB(31,16,16);    // light red (announce)
    palette[4]=IRGB(16,16,31);    // light blue (text links)
    palette[5]=IRGB(24,24,31);    // orange (item desc headings)
    palette[6]=IRGB(31,31,16);    // yellow (tells)
    palette[7]=IRGB(16,24,31);    // violet (staff chat)
    palette[8]=IRGB(24,24,31);    // light violet (god chat)

    palette[9]=IRGB(24,24,16);    // chat - auction
    palette[10]=IRGB(24,16,24);    // chat - grats
    palette[11]=IRGB(16,24,24);    // chat	- mirror
    palette[12]=IRGB(31,24,16);    // chat - info
    palette[13]=IRGB(31,16,24);    // chat - area
    palette[14]=IRGB(16,31,24);    // chat - v2, games
    palette[15]=IRGB(24,31,16);    // chat - public clan
    palette[16]=IRGB(24,16,31);    // chat	- internal clan

    palette[17]=IRGB(31,31,31);    // fake white text (hidden links)
}

void dd_set_textfont(int nr) {

    switch (nr) {
        case 0:	textfont=fonta; textdisplay_dy=10; fontdim=fontdim_a; break;
        case 1:	textfont=fontc; textdisplay_dy=12; fontdim=fontdim_c; break;
    }
    bzero(text,MAXTEXTLINES*MAXTEXTLETTERS*sizeof(struct letter));
    textnextline=textdisplayline=0;
}

void dd_display_text(void) {
    int n,m,rn,x,y,pos;
    char buf[256],*bp;
    unsigned short lastcolor=-1;

    for (n=textdisplayline,y=TEXTDISPLAY_Y; y<=TEXTDISPLAY_Y+TEXTDISPLAY_SY-TEXTDISPLAY_DY; n++,y+=TEXTDISPLAY_DY) {
        rn=n%MAXTEXTLINES;

        x=TEXTDISPLAY_X;
        pos=rn*MAXTEXTLETTERS;

        bp=buf;
        for (m=0; m<MAXTEXTLETTERS; m++,pos++) {
            if (text[pos].c==0) break;

            if (text[pos].c>0 && text[pos].c<32) continue;

            if (bp==buf) {
                lastcolor=text[pos].color;
            }
            if (lastcolor!=text[pos].color) {
                *bp=0;
                if (largetext) x=dd_drawtext(x,y,palette[lastcolor],DD_BIG,buf);
                else x=dd_drawtext(x,y,palette[lastcolor],0,buf);
                bp=buf; lastcolor=text[pos].color;
            }
            *bp++=text[pos].c;
        }
        if (bp!=buf) {
            *bp=0;
            if (largetext) dd_drawtext(x,y,palette[lastcolor],DD_BIG,buf);
            else dd_drawtext(x,y,palette[lastcolor],0,buf);
        }
    }
}

void dd_add_text(char *ptr) {
    int n,m,pos,color=0,link=0;
    int x=0,tmp;
    char buf[256];

    pos=textnextline*MAXTEXTLETTERS;
    bzero(text+pos,sizeof(struct letter)*MAXTEXTLETTERS);

    while (*ptr) {


        while (*ptr==' ') ptr++;
        while (*ptr=='°') {
            ptr++;
            switch (*ptr) {
                case 'c':	tmp=atoi(ptr+1);
                    if (tmp==18) link=0;
                    else if (tmp!=17) { color=tmp; link=0; }
                    if (tmp==4 || tmp==17) link=1;
                    ptr++;
                    while (isdigit(*ptr)) ptr++;
                    break;
                default:	ptr++; break;
            }
        }
        while (*ptr==' ') ptr++;

        n=0;
        while (*ptr && *ptr!=' ' && *ptr!='°' && n<49) buf[n++]=*ptr++;
        buf[n]=0;

        if (x+(tmp=dd_text_len(buf))>=TEXTDISPLAY_SX) {
            if (textdisplayline==(textnextline+(MAXTEXTLINES-TEXTDISPLAYLINES))%MAXTEXTLINES) textdisplayline=(textdisplayline+1)%MAXTEXTLINES;
            textnextline=(textnextline+1)%MAXTEXTLINES;
            pos=textnextline*MAXTEXTLETTERS;
            bzero(text+pos,sizeof(struct letter)*MAXTEXTLETTERS);
            x=tmp;

            for (m=0; m<2; m++) {
                text[pos].c=32;
                x+=textfont[32].dim;
                text[pos].color=color;
                text[pos].link=link;
                pos++;
            }

        } else x+=tmp;

        for (m=0; m<n; m++,pos++) {
            text[pos].c=buf[m];
            text[pos].color=color;
            text[pos].link=link;
        }
        text[pos].c=32;
        x+=textfont[32].dim;
        text[pos].color=color;
        text[pos].link=link;

        pos++;
    }
    pos=(pos+MAXTEXTLETTERS*MAXTEXTLINES-1)%(MAXTEXTLETTERS*MAXTEXTLINES);
    if (text[pos].c==32) {
        text[pos].c=0;
        text[pos].color=0;
        text[pos].link=0;
    }

    if (textdisplayline==(textnextline+(MAXTEXTLINES-TEXTDISPLAYLINES))%MAXTEXTLINES) textdisplayline=(textdisplayline+1)%MAXTEXTLINES;

    textnextline=(textnextline+1)%MAXTEXTLINES;
    if (textnextline==textdisplayline) textdisplayline=(textdisplayline+1)%MAXTEXTLINES;
    //textdisplayline=(textnextline+(MAXTEXTLINES-TEXTDISPLAYLINES))%MAXTEXTLINES;
}

int dd_text_init_done(void) {
    return text!=NULL;
}

int dd_scantext(int x,int y,char *hit) {
    int n,m,pos,panic=0,tmp=0;
    int dx;

    if (x<TEXTDISPLAY_X || y<TEXTDISPLAY_Y) return 0;
    if (x>TEXTDISPLAY_X+TEXTDISPLAY_SX) return 0;
    if (y>TEXTDISPLAY_Y+TEXTDISPLAY_SY) return 0;

    n=(y-TEXTDISPLAY_Y)/TEXTDISPLAY_DY;
    n=(n+textdisplayline)%MAXTEXTLINES;

    for (pos=n*MAXTEXTLETTERS,dx=m=0; m<MAXTEXTLETTERS && text[pos].c; m++,pos++) {
        if (text[pos].c>0 && text[pos].c<32) { dx=((int)text[pos].c)*12+TEXTDISPLAY_X; continue; }

        dx+=textfont[text[pos].c].dim;

        if (dx+TEXTDISPLAY_X>x) {
            if (text[pos].link) {   // link palette color
                while ((text[pos].link || text[pos].c==0) && panic++<5000) {
                    pos--;
                    if (pos<0) pos=MAXTEXTLETTERS*MAXTEXTLINES-1;
                }

                pos++;
                if (pos==MAXTEXTLETTERS*MAXTEXTLINES) pos=0;
                while ((text[pos].link || text[pos].c==0) && panic++<5000 && tmp<80) {
                    if (tmp>0 && text[pos].c==' ' && hit[tmp-1]==' ');
                    else if (text[pos].c) hit[tmp++]=text[pos].c;
                    pos++;
                }
                if (tmp>0 && hit[tmp-1]==' ') hit[tmp-1]=0;
                else hit[tmp]=0;
                return 1;
            }
            return 0;
        }
    }
    return 0;
}

void dd_text_lineup(void) {
    int tmp;

    tmp=(textdisplayline+MAXTEXTLINES-1)%MAXTEXTLINES;
    if (tmp!=textnextline) textdisplayline=tmp;
}

void dd_text_pageup(void) {
    int n,tmp;

    for (n=0; n<TEXTDISPLAYLINES; n++) {
        tmp=(textdisplayline+MAXTEXTLINES-1)%MAXTEXTLINES;
        if (tmp==textnextline) break;
        textdisplayline=tmp;
    }
}

void dd_text_linedown(void) {
    int tmp;

    tmp=(textdisplayline+1)%MAXTEXTLINES;
    if (tmp!=(textnextline+MAXTEXTLINES-TEXTDISPLAYLINES+1)%MAXTEXTLINES) textdisplayline=tmp;
}

void dd_text_pagedown(void) {
    int n,tmp;

    for (n=0; n<TEXTDISPLAYLINES; n++) {
        tmp=(textdisplayline+1)%MAXTEXTLINES;
        if (tmp==(textnextline+MAXTEXTLINES-TEXTDISPLAYLINES+1)%MAXTEXTLINES) break;
        textdisplayline=tmp;
    }
}

