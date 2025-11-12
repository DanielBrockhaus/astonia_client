/*
 * Part of Astonia Client (c) Daniel Brockhaus. Please read license.txt.
 */

#define MAX_TEXCACHE    (sdl_cache_size)
#define MAX_TEXHASH     (sdl_cache_size)    // Note: MAX_TEXCACHE and MAX_TEXHASH do not have to be the same value. It just turned out to work well if they are.

#define STX_NONE        (-1)

#define IGET_A(c)       ((((uint32_t)(c))>>24)&0xFF)
#define IGET_R(c)       ((((uint32_t)(c))>>16)&0xFF)
#define IGET_G(c)       ((((uint32_t)(c))>>8)&0xFF)
#define IGET_B(c)       ((((uint32_t)(c))>>0)&0xFF)
#define IRGB(r,g,b)     (((r)<<0)|((g)<<8)|((b)<<16))
#define IRGBA(r,g,b,a)  (((a)<<24)|((r)<<16)|((g)<<8)|((b)<<0))

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

    int fortick;        // pre-cached for tick X

    // ---------- sprites ------------
    // fx
    int32_t sprite;
    int8_t sink;
    uint8_t scale;
    int16_t cr,cg,cb,light,sat;
    uint16_t c1,c2,c3,shine;

    uint8_t freeze;

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

struct sdl_image {
    uint32_t *pixel;

    uint16_t flags;
    int16_t xres,yres;
    int16_t xoff,yoff;
};

#ifndef HAVE_DDFONT
#define HAVE_DDFONT
struct ddfont {
    int dim;
    unsigned char *raw;
};
#endif

#define DDT             '\xB0' // draw text terminator - (zero stays one, too)

int sdl_ic_load(int sprite);
int sdl_pre_backgnd(void *ptr);
int sdl_create_cursors(void);

#define MAX_SOUND_CHANNELS   32
#define MAXSOUND            100

struct png_helper;
int png_load_helper(struct png_helper *p);
void png_load_helper_exit(struct png_helper *p);

