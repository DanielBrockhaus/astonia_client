/*
 * Part of Astonia Client (c) Daniel Brockhaus. Please read license.txt.
 */

#include "dll.h"

#define DD_OFFSET        0       // this has to be zero, so bzero on the structures default this
#define DD_CENTER        1       // also used in dd_drawtext
#define DD_NORMAL        2

#define DD_LEFT         0
#define DD_CENTER       1
#define DD_RIGHT        2
#define DD_SHADE        4
#define DD_LARGE        0
#define DD_SMALL        8
#define DD_FRAME        16
#define DD_BIG        	32
#define DD_NOCACHE      64
#define DD_WFRAME       (DD_FRAME|512)

#define SPR_WALK        11

#define SPR_FFIELD      10
#define SPR_FIELD       12

#define SPR_ITPAD       13
#define SPR_ITSEL       14

#define SPR_SCRUP       27 // 20 // 17
#define SPR_SCRLT       28 // 21 // 19
#define SPR_SCRDW       29 // 23 // 18
#define SPR_SCRRT       30 // 22 // 19
#define SPR_SCRBAR      26

#define SPR_RAISE       19
#define SPR_CLOSE       36
#define SPR_TEXTF       35

#define SPR_GOLD_BEG    100
#define SPR_GOLD_END    109

#define DDFX_NLIGHT             15
#define DDFX_BRIGHT             0

#define MMF_SIGHTBLOCK  (1<<1)  // indicates sight block (set_map_lights)
#define MMF_DOOR        (1<<2)  // a door - helpful when cutting sprites - (set_map_sprites)
#define MMF_CUT         (1<<3)  // indicates cut (set_map_cut)

#define MMF_STRAIGHT_T  (1<<5)  // (set_map_straight)
#define MMF_STRAIGHT_B  (1<<6)  // (set_map_straight)
#define MMF_STRAIGHT_L  (1<<7)  // (set_map_straight)
#define MMF_STRAIGHT_R  (1<<8)  // (set_map_straight)


#define IGET_R(c) ((((unsigned short int)(c))>>10)&0x1F)
#define IGET_G(c) ((((unsigned short int)(c))>>5)&0x1F)
#define IGET_B(c) ((((unsigned short int)(c))>>0)&0x1F)
#define IRGB(r,g,b) (((r)<<10)|((g)<<5)|((b)<<0))

struct ddfx {
    int sprite;             // sprite_fx:           primary sprite number - should be the first entry cause dl_qcmp sorts the by this

    signed char sink;
    unsigned char scale;        // scale in percent
    char cr,cg,cb;          // color balancing
    char clight,sat;        // lightness, saturation
    unsigned short c1,c2,c3,shine;  // color replacer

    char light;             // videocache_fx:       0=bright(DDFX_BRIGHT) 1=almost black; 15=normal (DDFX_NLIGHT)
    char freeze;            // videocache_fx:       0 to DDFX_MAX_FREEZE-1  !!! exclusive DDFX_MAX_FREEZE

    char ml,ll,rl,ul,dl;

    char align;             // blitpos_fx:          DDFX_NORMAL, DDFX_OFFSET, DDFX_CENTER
    short int clipsx,clipex; // blitpos_fx:          additional x - clipping around the offset
    short int clipsy,clipey; // blitpos_fx:          additional y - clipping around the offset

    unsigned char alpha;
};

typedef struct ddfx DDFX;

extern float mouse_scale;   // mouse input needs to be scaled by this factor because the display window is stretched
extern char user_keys[10];
extern int namesize;
extern int stom_off_x,stom_off_y;
extern int __textdisplay_sy;
extern int x_offset,y_offset;

DLL_EXPORT int dd_textlength(int flags,const char *text);
int dd_textlen(int flags,const char *text,int n);
DLL_EXPORT int dd_drawtext(int sx,int sy,unsigned short int color,int flags,const char *text);
DLL_EXPORT int dd_drawtext_fmt(int sx,int sy,unsigned short int color,int flags,const char *format,...) __attribute__((format(printf, 5, 6)));
DLL_EXPORT int dd_drawtext_break_fmt(int sx,int sy,int breakx,unsigned short int color,int flags,const char *format,...)  __attribute__((format(printf, 6, 7)));
DLL_EXPORT int dd_drawtext_nl(int x,int y,int unsigned short color,int flags,const char *ptr);
DLL_EXPORT int dd_copysprite_fx(DDFX *ddfx,int scrx,int scry);
DLL_EXPORT void dd_copysprite(int sprite,int scrx,int scry,int light,int align);
void dd_copysprite_callfx(int sprite,int scrx,int scry,int light,int mli,int align);
DLL_EXPORT int dd_drawtext_break(int x,int y,int breakx,unsigned short color,int flags,const char *ptr);
DLL_EXPORT int dd_drawtext_break_length(int x,int y,int breakx,unsigned short color,int flags,const char *ptr);
DLL_EXPORT void dd_rect(int sx,int sy,int ex,int ey,unsigned short int color);
DLL_EXPORT void dd_push_clip(void);
DLL_EXPORT void dd_pop_clip(void);
DLL_EXPORT void dd_more_clip(int sx,int sy,int ex,int ey);
void dd_set_clip(int sx,int sy,int ex,int ey);
void dd_text_pageup(void);
void dd_text_pagedown(void);
DLL_EXPORT void dd_line(int fx,int fy,int tx,int ty,unsigned short col);
void dd_display_text(void);
int dd_scantext(int x,int y,char *hit);
int dd_char_len(char c);
int dd_drawtext_char(int sx,int sy,int c,unsigned short int color);
void dd_shaded_rect(int sx,int sy,int ex,int ey,unsigned short color,unsigned short alpha);
void dd_text_lineup(void);
void dd_text_linedown(void);
int dd_offset_x(void);
int dd_offset_y(void);
void dd_list_text(void);
DLL_EXPORT void dd_pixel(int x,int y,unsigned short col);
extern int (*trans_asprite)(int mn,int sprite,int attick,unsigned char *pscale,unsigned char *pcr,unsigned char *pcg,unsigned char *pcb,unsigned char *plight,unsigned char *psat,unsigned short *pc1,unsigned short *pc2,unsigned short *pc3,unsigned short *pshine);
DLL_EXPORT int _trans_asprite(int mn,int sprite,int attick,unsigned char *pscale,unsigned char *pcr,unsigned char *pcg,unsigned char *pcb,unsigned char *plight,unsigned char *psat,unsigned short *pc1,unsigned short *pc2,unsigned short *pc3,unsigned short *pshine);
extern int (*trans_charno)(int csprite,int *pscale,int *pcr,int *pcg,int *pcb,int *plight,int *psat,int *pc1,int *pc2,int *pc3,int *pshine,int attick);
DLL_EXPORT int _trans_charno(int csprite,int *pscale,int *pcr,int *pcg,int *pcb,int *plight,int *psat,int *pc1,int *pc2,int *pc3,int *pshine,int attick);
extern int (*additional_sprite)(int sprite,int attick);
DLL_EXPORT int _additional_sprite(int sprite,int attick);
extern int (*get_player_sprite)(int nr,int zdir,int action,int step,int duration,int attick);
DLL_EXPORT int _get_player_sprite(int nr,int zdir,int action,int step,int duration,int attick);
void save_options(void);
extern int (*opt_sprite)(int sprite);
DLL_EXPORT int _opt_sprite(int sprite);
extern int (*no_lighting_sprite)(int sprite);
DLL_EXPORT int _no_lighting_sprite(int sprite);

struct map;
int get_sink(int mn,struct map *cmap);

void list_mem(void);

void display_game(void);

void set_map_values(struct map *cmap,int attick);
void quest_select(int nr);
void init_game(int mcx,int mcy);
void exit_game(void);

