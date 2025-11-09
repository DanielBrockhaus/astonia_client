/*
 * Part of Astonia Client (c) Daniel Brockhaus. Please read license.txt.
 */

#include "../dll.h"
#include "amod_structs.h"

void amod_init(void);
void amod_exit(void);
char *amod_version(void);
void amod_gamestart(void);
void amod_frame(void);
void amod_tick(void);
void amod_mouse_move(int x,int y);
void amod_update_hover_texts(void);

// the following functions should return 1 if they process the event and want the client
// and all later mods to ignore it.
// return -1 if you want the client to ignore it, but allow other mods to process it.
// return 0 otherwise
int amod_mouse_click(int x,int y,int what);
int amod_keydown(int key);  // if you catch keydown ...
int amod_keyup(int key);    // ... you must also catch keyup
int amod_client_cmd(char *buf);

// main mod only:
int amod_process(char *buf);    // return length of server command, 0 = unknown
int amod_prefetch(char *buf);   // return length of server command, 0 = unknown
int amod_display_skill_line(int v,int base,int curr,int cn,char *buf);
int amod_is_playersprite(int sprite);

// --------- Client exported functions -----------

// basics
DLL_IMPORT int note(const char *format,...) __attribute__((format(printf, 1, 2)));
DLL_IMPORT int warn(const char *format,...) __attribute__((format(printf, 1, 2)));
DLL_IMPORT int fail(const char *format,...) __attribute__((format(printf, 1, 2)));
DLL_IMPORT void paranoia(const char *format,...) __attribute__((format(printf, 1, 2)));
DLL_IMPORT void addline(const char *format,...) __attribute__((format(printf, 1, 2)));
// game, drawing
DLL_IMPORT void dd_push_clip(void);
DLL_IMPORT void dd_pop_clip(void);
DLL_IMPORT void dd_more_clip(int sx,int sy,int ex,int ey);
DLL_IMPORT void dd_copysprite(int sprite,int scrx,int scry,int light,int align);
DLL_IMPORT int dd_copysprite_fx(DDFX *ddfx,int scrx,int scry);
DLL_IMPORT void dd_rect(int sx,int sy,int ex,int ey,unsigned short int color);
DLL_IMPORT void dd_line(int fx,int fy,int tx,int ty,unsigned short col);
DLL_IMPORT int dd_textlength(int flags,const char *text);
DLL_IMPORT int dd_drawtext(int sx,int sy,unsigned short int color,int flags,const char *text);
DLL_IMPORT int dd_drawtext_break(int x,int y,int breakx,unsigned short color,int flags,const char *ptr);
DLL_IMPORT int dd_drawtext_break_length(int x,int y,int breakx,unsigned short color,int flags,const char *ptr);
DLL_IMPORT void dd_pixel(int x,int y,unsigned short col);
DLL_IMPORT int dd_drawtext_fmt(int sx,int sy,unsigned short int color,int flags,const char *format,...);
DLL_IMPORT int dd_drawtext_break_fmt(int sx,int sy,int breakx,unsigned short int color,int flags,const char *format,...);
DLL_IMPORT int dd_drawtext_nl(int x,int y,int unsigned short color,int flags,const char *ptr);
// gui, dots and buttons
DLL_IMPORT int dotx(int didx);
DLL_IMPORT int doty(int didx);
DLL_IMPORT int butx(int bidx);
DLL_IMPORT int buty(int bidx);
// gui helperls
DLL_IMPORT int get_near_ground(int x,int y);
DLL_IMPORT int get_near_item(int x,int y,int flag,int looksize);
DLL_IMPORT int get_near_char(int x,int y,int looksize);
DLL_IMPORT int mapmn(int x,int y);
// misc
DLL_IMPORT void set_teleport(int idx,int x,int y);
DLL_IMPORT int exp2level(int val);
DLL_IMPORT int level2exp(int level);
DLL_IMPORT int mil_rank(int exp);
// client / server communication
DLL_IMPORT void client_send(void *buf,int len);


// ---------- Client exported data structures -------------
DLL_IMPORT extern int skltab_cnt;
DLL_IMPORT extern struct skltab *skltab;
DLL_IMPORT extern int item_flags[];
DLL_IMPORT extern int weatab[];
DLL_IMPORT int act;
DLL_IMPORT int actx;
DLL_IMPORT int acty;
DLL_IMPORT extern int __yres;

DLL_IMPORT int vk_shift,vk_control,vk_alt;

DLL_IMPORT unsigned int cflags;        // current item flags
DLL_IMPORT unsigned int csprite;       // and sprite

DLL_IMPORT int originx;
DLL_IMPORT int originy;
DLL_IMPORT struct map map[MAPDX*MAPDY];
DLL_IMPORT struct map map2[MAPDX*MAPDY];

DLL_IMPORT int value[2][V_MAX];
DLL_IMPORT int item[INVENTORYSIZE];
DLL_IMPORT int item_flags[INVENTORYSIZE];
DLL_IMPORT int hp;
DLL_IMPORT int mana;
DLL_IMPORT int rage;
DLL_IMPORT int endurance;
DLL_IMPORT int lifeshield;
DLL_IMPORT int experience;
DLL_IMPORT int experience_used;
DLL_IMPORT int mil_exp;
DLL_IMPORT int gold;
DLL_IMPORT struct player player[MAXCHARS];
DLL_IMPORT union ceffect ceffect[MAXEF];
DLL_IMPORT unsigned char ueffect[MAXEF];
DLL_IMPORT int con_type;
DLL_IMPORT char con_name[80];
DLL_IMPORT int con_cnt;
DLL_IMPORT int container[CONTAINERSIZE];
DLL_IMPORT int price[CONTAINERSIZE];
DLL_IMPORT int itemprice[CONTAINERSIZE];
DLL_IMPORT int cprice;
DLL_IMPORT int lookinv[12];
DLL_IMPORT int looksprite,lookc1,lookc2,lookc3;
DLL_IMPORT char look_name[80];
DLL_IMPORT char look_desc[1024];
DLL_IMPORT char pent_str[7][80];
DLL_IMPORT int pspeed;
DLL_IMPORT int protocol_version;

DLL_IMPORT unsigned short int healthcolor,manacolor,endurancecolor,shieldcolor;
DLL_IMPORT unsigned short int whitecolor,lightgraycolor,graycolor,darkgraycolor,blackcolor;
DLL_IMPORT unsigned short int lightredcolor,redcolor,darkredcolor;
DLL_IMPORT unsigned short int lightgreencolor,greencolor,darkgreencolor;
DLL_IMPORT unsigned short int lightbluecolor,bluecolor,darkbluecolor;
DLL_IMPORT unsigned short int lightorangecolor,orangecolor,darkorangecolor;
DLL_IMPORT unsigned short int textcolor;

DLL_IMPORT struct quest quest[MAXQUEST];
DLL_IMPORT struct shrine_ppd shrine;

DLL_IMPORT char hover_bless_text[120];
DLL_IMPORT char hover_freeze_text[120];
DLL_IMPORT char hover_potion_text[120];
DLL_IMPORT char hover_rage_text[120];
DLL_IMPORT char hover_level_text[120];
DLL_IMPORT char hover_rank_text[120];
DLL_IMPORT char hover_time_text[120];

DLL_IMPORT char *target_server;

DLL_IMPORT char password[16];
DLL_IMPORT char username[40];
DLL_IMPORT int tick;
DLL_IMPORT int mirror;
DLL_IMPORT int realtime;
DLL_IMPORT char server_url[256];
DLL_IMPORT int server_port;
DLL_IMPORT int want_width;
DLL_IMPORT int want_height;
DLL_IMPORT int sdl_scale;
DLL_IMPORT int sdl_frames;
DLL_IMPORT int sdl_multi;
DLL_IMPORT int sdl_cache_size;
DLL_IMPORT int frames_per_second;
DLL_IMPORT uint64_t game_options;
DLL_IMPORT int game_slowdown;


// ---------------- override-able functions, also exported from client ----------------
DLL_IMPORT int _is_cut_sprite(int sprite);
DLL_IMPORT int _is_mov_sprite(int sprite,int itemhint);
DLL_IMPORT int _is_door_sprite(int sprite);
DLL_IMPORT int _is_yadd_sprite(int sprite);
DLL_IMPORT int _get_chr_height(int csprite);
DLL_IMPORT int _trans_asprite(int mn,int sprite,int attick,unsigned char *pscale,unsigned char *pcr,unsigned char *pcg,unsigned char *pcb,unsigned char *plight,unsigned char *psat,unsigned short *pc1,unsigned short *pc2,unsigned short *pc3,unsigned short *pshine);
DLL_IMPORT int _trans_charno(int csprite,int *pscale,int *pcr,int *pcg,int *pcb,int *plight,int *psat,int *pc1,int *pc2,int *pc3,int *pshine,int attick);
DLL_IMPORT int _get_player_sprite(int nr,int zdir,int action,int step,int duration,int attick);
DLL_IMPORT void _trans_csprite(int mn,struct map *cmap,int attick);
DLL_IMPORT int _get_lay_sprite(int sprite,int lay);
DLL_IMPORT int _get_offset_sprite(int sprite,int *px,int *py);
DLL_IMPORT int _additional_sprite(int sprite,int attick);
DLL_IMPORT int _opt_sprite(int sprite);
DLL_IMPORT int _no_lighting_sprite(int sprite);
DLL_IMPORT int _get_skltab_sep(int i);
DLL_IMPORT int _get_skltab_index(int n);
DLL_IMPORT int _get_skltab_show(int i);
DLL_IMPORT int _do_display_random(void);
DLL_IMPORT int _do_display_help(int nr);

// ------------ declarations for functions the mod might provide -------------------
int is_cut_sprite(int sprite);
int is_mov_sprite(int sprite,int itemhint);
int is_door_sprite(int sprite);
int is_yadd_sprite(int sprite);
int get_chr_height(int csprite);
int trans_asprite(int mn,int sprite,int attick,unsigned char *pscale,unsigned char *pcr,unsigned char *pcg,unsigned char *pcb,unsigned char *plight,unsigned char *psat,unsigned short *pc1,unsigned short *pc2,unsigned short *pc3,unsigned short *pshine);
int trans_charno(int csprite,int *pscale,int *pcr,int *pcg,int *pcb,int *plight,int *psat,int *pc1,int *pc2,int *pc3,int *pshine,int attick);
int get_player_sprite(int nr,int zdir,int action,int step,int duration,int attick);
void trans_csprite(int mn,struct map *cmap,int attick);
int get_lay_sprite(int sprite,int lay);
int get_offset_sprite(int sprite,int *px,int *py);
int additional_sprite(int sprite,int attick);
int opt_sprite(int sprite);
int no_lighting_sprite(int sprite);
int get_skltab_sep(int i);
int get_skltab_index(int n);
int get_skltab_show(int i);
int do_display_random(void);
int do_display_help(int nr);


