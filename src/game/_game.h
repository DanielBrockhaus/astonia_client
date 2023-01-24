/*
 * Part of Astonia Client (c) Daniel Brockhaus. Please read license.txt.
 */

#define GND_LAY         100
#define GND2_LAY        101
#define GNDSHD_LAY      102
#define GNDSTR_LAY      103
#define GNDTOP_LAY      104
#define GNDSEL_LAY      105
#define GME_LAY         110
#define GME_LAY2        111
#define GMEGRD_LAYADD   500
#define TOP_LAY         1000

#define SO_DARK     (1<<0)  // Dark GUI by Tegra

#define DDFX_MAX_FREEZE         8

#define DD_LARGE_CHARSET "abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789,.;:+-*/~@#'\"?!&%()[]=<>|_$"
#define DD_SMALL_CHARSET "abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.+-"

#define DD__SHADEFONT	128
#define DD__FRAMEFONT	256

#define DDT             '°' // draw text terminator - (zero stays one, too)

#define DL_STEP 128

#define DLC_STRIKE      1
#define DLC_NUMBER	2
#define DLC_DUMMY	3       // used to get space in the list to reduce compares ;-)
#define DLC_PIXEL	4
#define DLC_BLESS	5
#define DLC_POTION	6
#define DLC_RAIN	7
#define DLC_PULSE	8
#define DLC_PULSEBACK	9

struct xxximage {
    unsigned short int xres;
    unsigned short int yres;
    short int xoff;
    short int yoff;

    unsigned short int *rgb;        // irgb format
    unsigned char *a;               // 5 bit alpha
};
typedef struct xxximage IMAGE;

struct dl {
    int layer;
    int x,y,h;      // scrx=x scry=y-h sorted bye x,y ;) normally used for height, but also misused to place doors right
    // int movy;

    DDFX ddfx;

    // functions to call
    char call;
    int call_x1,call_y1,call_x2,call_y2,call_x3;
};
typedef struct dl DL;

#ifndef HAVE_DDFONT
#define HAVE_DDFONT
struct ddfont {
    int dim;
    unsigned char *raw;
};
#endif
struct ddfont; typedef struct ddfont DDFONT;

extern unsigned short rgbcolorkey;
extern unsigned short rgbcolorkey2;
extern unsigned short scrcolorkey;
extern unsigned short *rgb2scr;
extern unsigned short *scr2rgb;
extern unsigned short **rgbfx_light;
extern DDFONT fonta[];
extern DDFONT *fonta_shaded;
extern DDFONT *fonta_framed;
extern DDFONT fontb[];
extern DDFONT *fontb_shaded;
extern DDFONT *fontb_framed;
extern DDFONT fontc[];
extern DDFONT *fontc_shaded;
extern DDFONT *fontc_framed;

int is_top_sprite(int sprite,int itemhint);
int is_cut_sprite(int sprite);
int is_mov_sprite(int sprite,int itemhint);
int is_door_sprite(int sprite);
int is_yadd_sprite(int sprite);
int get_chr_height(int csprite);
int get_lay_sprite(int sprite,int lay);
int get_offset_sprite(int sprite,int *px,int *py);
int rread(int fd,void *ptr,int size);
char* load_ascii_file(char *filename,int ID);
int xmemcheck(void *ptr);
void rrandomize(void);
void rseed(int seed);
int rrand(int range);

int dd_init(void);
int dd_exit(void);
void dd_pixel(int x,int y,unsigned short col);
void dd_draw_bless(int x,int y,int ticker,int strength,int front);
void dd_draw_potion(int x,int y,int ticker,int strength,int front);
void dd_draw_rain(int x,int y,int ticker,int strength,int front);
void dd_draw_curve(int cx,int cy,int nr,int size,unsigned short col);
void dd_display_pulseback(int fx,int fy,int tx,int ty);
int dd_text_init_done(void);
void dd_add_text(char *ptr);
void dd_set_textfont(int nr);
void dd_create_font(void);
void dd_init_text(void);
void dd_display_strike(int fx,int fy,int tx,int ty);


