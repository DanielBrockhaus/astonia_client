/*
 * Part of Astonia Client (c) Daniel Brockhaus. Please read license.txt.
 */

#define GFXPATH "../gfx/"

#define MAXSPRITE 250000

#define XRES    800
#define YRES    600

extern float mouse_scale;   // mouse input needs to be scaled by this factor because the display window is stretched

extern int dd_gamma;
extern int dd_lighteffect;
extern int largetext;

extern int xres;                // pitch of the back surfache (ddbs)

void dd_push_clip(void);
void dd_pop_clip(void);
void dd_more_clip(int sx,int sy,int ex,int ey);
void dd_set_clip(int sx,int sy,int ex,int ey);

int dd_init(int width,int height);
int dd_exit(void);

void dd_pixel(int x,int y,unsigned short col);
void dd_draw_bless(int x,int y,int ticker,int strength,int front);
void dd_draw_potion(int x,int y,int ticker,int strength,int front);
void dd_draw_rain(int x,int y,int ticker,int strength,int front);
void dd_draw_curve(int cx,int cy,int nr,int size,unsigned short col);
void dd_display_pulseback(int fx,int fy,int tx,int ty);
int dd_drawtext_break(int x,int y,int breakx,unsigned short color,int flags,const char *ptr);
void dd_line(int fx,int fy,int tx,int ty,unsigned short col);
void dd_display_text(void);
int dd_scantext(int x,int y,char *hit);
int dd_char_len(char c);
int dd_drawtext_char(int sx,int sy,int c,unsigned short int color);
void dd_shaded_rect(int sx,int sy,int ex,int ey);
void dd_text_pageup(void);
void dd_text_pagedown(void);
int dd_text_init_done(void);
void dd_add_text(char *ptr);
void dd_set_textfont(int nr);

#define DD_OFFSET               0       // this has to be zero, so bzero on the structures default this
#define DD_CENTER               1       // also used in dd_drawtext
#define DD_NORMAL               2

#define DDFX_LEFTGRID           1       // NOGRID(?) has to be zero, so bzero on the structures default NOGRID(?)
#define DDFX_RIGHTGRID          2
//#define DLFX_AUTOGRID           3       // not really used by blitting functions, but by dl (you must not call copysprite with this value)

#define DDFX_NLIGHT             15
#define DDFX_BRIGHT             0

#define DDFX_MAX_FREEZE         8

struct ddfx {
    int sprite;             // sprite_fx:           primary sprite number - should be the first entry cause dl_qcmp sorts the by this

    signed char sink;
    unsigned char scale;        // scale in percent
    char cr,cg,cb;          // color balancing
    char clight,sat;        // lightness, saturation
    unsigned short c1,c2,c3,shine;  // color replacer

    char light;             // videocache_fx:       0=bright(DDFX_BRIGHT) 1=almost black; 15=normal (DDFX_NLIGHT)
    char freeze;            // videocache_fx:       0 to DDFX_MAX_FREEZE-1  !!! exclusive DDFX_MAX_FREEZE

    char grid;              // videocache_fx:       0, DDFX_LEFTGRID, DDFX_RIGHTGRID

    char ml,ll,rl,ul,dl;

    char align;             // blitpos_fx:          DDFX_NORMAL, DDFX_OFFSET, DDFX_CENTER
    short int clipsx,clipex; // blitpos_fx:          additional x - clipping around the offset
    short int clipsy,clipey; // blitpos_fx:          additional y - clipping around the offset
};

typedef struct ddfx DDFX;

struct xxximage {
    unsigned short int xres;
    unsigned short int yres;
    short int xoff;
    short int yoff;

    unsigned short int *rgb;        // irgb format
    unsigned char *a;               // 5 bit alpha
};

typedef struct xxximage IMAGE;

extern unsigned short rgbcolorkey;
extern unsigned short rgbcolorkey2;
extern unsigned short scrcolorkey;
extern unsigned short *rgb2scr;
extern unsigned short *scr2rgb;
extern unsigned short **rgbfx_light;

#define IGET_R(c) ((((unsigned short int)(c))>>10)&0x1F)
#define IGET_G(c) ((((unsigned short int)(c))>>5)&0x1F)
#define IGET_B(c) ((((unsigned short int)(c))>>0)&0x1F)
#define IRGB(r,g,b) (((r)<<10)|((g)<<5)|((b)<<0))

int dd_copysprite_fx(DDFX *ddfx,int scrx,int scry);
void dd_copysprite(int sprite,int scrx,int scry,int light,int align);
void dd_copysprite_callfx(int sprite,int scrx,int scry,int light,int mli,int grid,int align);

void dd_darken_rect(int sx,int sy,int ex,int ey);
void dd_rect(int sx,int sy,int ex,int ey,unsigned short int color);

#define DD_LARGE_CHARSET "abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789,.;:+-*/~@#'\"?!&%()[]=<>|_$"
#define DD_SMALL_CHARSET "abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.+-"

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

#define DDT             '°' // draw text terminator - (zero stays one, too)

int dd_textlength(int flags,const char *text);
int dd_textlen(int flags,const char *text,int n);
int dd_drawtext(int sx,int sy,unsigned short int color,int flags,const char *text);
int dd_drawtext_fmt(int sx,int sy,unsigned short int color,int flags,const char *format,...);

void dd_display_strike(int fx,int fy,int tx,int ty);

#ifndef HAVE_DDFONT
#define HAVE_DDFONT
struct ddfont {
    int dim;
    unsigned char *raw;
};
#endif
struct ddfont; typedef struct ddfont DDFONT;

extern DDFONT fonta[];

extern DDFONT *fonta_shaded;
extern DDFONT *fonta_framed;

extern DDFONT fontb[];

extern DDFONT *fontb_shaded;
extern DDFONT *fontb_framed;

extern DDFONT fontc[];

extern DDFONT *fontc_shaded;
extern DDFONT *fontc_framed;



