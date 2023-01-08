/*
 * Part of Astonia Client (c) Daniel Brockhaus. Please read license.txt.
 */

#define GFXPATH "../gfx/"

#define MAXSPRITE 250000

#define XRES    800
#define YRES    600

#ifndef HAVE_DDFONT
#define HAVE_DDFONT
struct ddfont {
    int dim;
    unsigned char *raw;
};
#endif

#define DDT             '°' // draw text terminator - (zero stays one, too)

int sdl_init(int width,int height,char *title);
void sdl_exit(void);
void sdl_loop(void);

int sdl_clear(void);
int sdl_render(void);

int sdl_ic_load(int sprite);
void sdl_blit(int stx,int sx,int sy,int clipsx,int clipsy,int clipex,int clipey,int x_offset,int y_offset);
int sdl_tx_load(int sprite,int sink,int freeze,int grid,int scale,int cr,int cg,int cb,int light,int sat,int c1,int c2,int c3,int shine,int ml,int ll,int rl,int ul,int dl,
                const char *text,int text_color,int text_flags,void *text_font);
int sdl_drawtext(int sx,int sy,unsigned short int color,int flags,const char *text,struct ddfont *font,int clipsx,int clipsy,int clipex,int clipey,int x_offset,int y_offset);
void sdl_rect(int sx,int sy,int ex,int ey,unsigned short int color,int clipsx,int clipsy,int clipex,int clipey,int x_offset,int y_offset);
void sdl_shaded_rect(int sx,int sy,int ex,int ey,unsigned short int color,int clipsx,int clipsy,int clipex,int clipey,int x_offset,int y_offset);
void sdl_pixel(int x,int y,unsigned short color,int x_offset,int y_offset);
void sdl_line(int fx,int fy,int tx,int ty,unsigned short color,int clipsx,int clipsy,int clipex,int clipey,int x_offset,int y_offset);

#define SDL_KEYM_SHIFT  1
#define SDL_KEYM_CTRL   2
#define SDL_KEYM_ALT    4

int sdl_keymode(void);
void sdl_set_cursor_pos(int x,int y);
void sdl_show_cursor(int flag);
void sdl_capture_mouse(int flag);

#define SDL_CUR_c_only      1
#define SDL_CUR_c_take      2
#define SDL_CUR_c_drop      3
#define SDL_CUR_c_attack    4
#define SDL_CUR_c_raise     5
#define SDL_CUR_c_give      6
#define SDL_CUR_c_use       7
#define SDL_CUR_c_usewith   8
#define SDL_CUR_c_swap      9
#define SDL_CUR_c_sell      10
#define SDL_CUR_c_buy       11
#define SDL_CUR_c_look      12
#define SDL_CUR_c_set       13
#define SDL_CUR_c_spell     14
#define SDL_CUR_c_pix       15
#define SDL_CUR_c_say       16
#define SDL_CUR_c_junk      17
#define SDL_CUR_c_get       18

void sdl_set_cursor(int cursor);

int sdlt_xoff(int stx);
int sdlt_yoff(int stx);
int sdlt_xres(int stx);
int sdlt_yres(int stx);

uint32_t *sdl_load_png(char *filename,int *dx,int *dy);
