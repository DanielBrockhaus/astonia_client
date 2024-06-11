/*
 * Part of Astonia Client (c) Daniel Brockhaus. Please read license.txt.
 */

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
__declspec(dllimport) int note(const char *format,...) __attribute__((format(printf, 1, 2)));
__declspec(dllimport) int warn(const char *format,...) __attribute__((format(printf, 1, 2)));
__declspec(dllimport) int fail(const char *format,...) __attribute__((format(printf, 1, 2)));
__declspec(dllimport) void paranoia(const char *format,...) __attribute__((format(printf, 1, 2)));
__declspec(dllimport) void addline(const char *format,...) __attribute__((format(printf, 1, 2)));
// game, drawing
__declspec(dllimport) void dd_push_clip(void);
__declspec(dllimport) void dd_pop_clip(void);
__declspec(dllimport) void dd_more_clip(int sx,int sy,int ex,int ey);
__declspec(dllimport) void dd_copysprite(int sprite,int scrx,int scry,int light,int align);
__declspec(dllimport) int dd_copysprite_fx(DDFX *ddfx,int scrx,int scry);
__declspec(dllimport) void dd_rect(int sx,int sy,int ex,int ey,unsigned short int color);
__declspec(dllimport) void dd_line(int fx,int fy,int tx,int ty,unsigned short col);
__declspec(dllimport) int dd_textlength(int flags,const char *text);
__declspec(dllimport) int dd_drawtext(int sx,int sy,unsigned short int color,int flags,const char *text);
__declspec(dllimport) int dd_drawtext_break(int x,int y,int breakx,unsigned short color,int flags,const char *ptr);
__declspec(dllimport) int dd_drawtext_break_length(int x,int y,int breakx,unsigned short color,int flags,const char *ptr);
__declspec(dllimport) void dd_pixel(int x,int y,unsigned short col);
__declspec(dllimport) int dd_drawtext_fmt(int sx,int sy,unsigned short int color,int flags,const char *format,...);
__declspec(dllimport) int dd_drawtext_break_fmt(int sx,int sy,int breakx,unsigned short int color,int flags,const char *format,...);
__declspec(dllimport) int dd_drawtext_nl(int x,int y,int unsigned short color,int flags,const char *ptr);
// gui, dots and buttons
__declspec(dllimport) int dotx(int didx);
__declspec(dllimport) int doty(int didx);
__declspec(dllimport) int butx(int bidx);
__declspec(dllimport) int buty(int bidx);
// gui helperls
__declspec(dllimport) int get_near_ground(int x,int y);
__declspec(dllimport) int get_near_item(int x,int y,int flag,int looksize);
__declspec(dllimport) int get_near_char(int x,int y,int looksize);
__declspec(dllimport) int mapmn(int x,int y);
// misc
__declspec(dllimport) void set_teleport(int idx,int x,int y);
__declspec(dllimport) int exp2level(int val);
__declspec(dllimport) int level2exp(int level);
__declspec(dllimport) int mil_rank(int exp);
// client / server communication
__declspec(dllimport) void client_send(void *buf,int len);


// ---------- Client exported data structures -------------
__declspec(dllimport) extern int skltab_cnt;
__declspec(dllimport) extern struct skltab *skltab;
__declspec(dllimport) extern int item_flags[];
__declspec(dllimport) extern int weatab[];
__declspec(dllimport) int act;
__declspec(dllimport) int actx;
__declspec(dllimport) int acty;
__declspec(dllimport) extern int __yres;

__declspec(dllimport) int vk_shift,vk_control,vk_alt;

__declspec(dllimport) unsigned int cflags;        // current item flags
__declspec(dllimport) unsigned int csprite;       // and sprite

__declspec(dllimport) int originx;
__declspec(dllimport) int originy;
__declspec(dllimport) struct map map[MAPDX*MAPDY];
__declspec(dllimport) struct map map2[MAPDX*MAPDY];

__declspec(dllimport) int value[2][V_MAX];
__declspec(dllimport) int item[INVENTORYSIZE];
__declspec(dllimport) int item_flags[INVENTORYSIZE];
__declspec(dllimport) int hp;
__declspec(dllimport) int mana;
__declspec(dllimport) int rage;
__declspec(dllimport) int endurance;
__declspec(dllimport) int lifeshield;
__declspec(dllimport) int experience;
__declspec(dllimport) int experience_used;
__declspec(dllimport) int mil_exp;
__declspec(dllimport) int gold;
__declspec(dllimport) struct player player[MAXCHARS];
__declspec(dllimport) union ceffect ceffect[MAXEF];
__declspec(dllimport) unsigned char ueffect[MAXEF];
__declspec(dllimport) int con_type;
__declspec(dllimport) char con_name[80];
__declspec(dllimport) int con_cnt;
__declspec(dllimport) int container[CONTAINERSIZE];
__declspec(dllimport) int price[CONTAINERSIZE];
__declspec(dllimport) int itemprice[CONTAINERSIZE];
__declspec(dllimport) int cprice;
__declspec(dllimport) int lookinv[12];
__declspec(dllimport) int looksprite,lookc1,lookc2,lookc3;
__declspec(dllimport) char look_name[80];
__declspec(dllimport) char look_desc[1024];
__declspec(dllimport) char pent_str[7][80];
__declspec(dllimport) int pspeed;
__declspec(dllimport) int protocol_version;

__declspec(dllimport) unsigned short int healthcolor,manacolor,endurancecolor,shieldcolor;
__declspec(dllimport) unsigned short int whitecolor,lightgraycolor,graycolor,darkgraycolor,blackcolor;
__declspec(dllimport) unsigned short int lightredcolor,redcolor,darkredcolor;
__declspec(dllimport) unsigned short int lightgreencolor,greencolor,darkgreencolor;
__declspec(dllimport) unsigned short int lightbluecolor,bluecolor,darkbluecolor;
__declspec(dllimport) unsigned short int lightorangecolor,orangecolor,darkorangecolor;
__declspec(dllimport) unsigned short int textcolor;

__declspec(dllimport) struct quest quest[MAXQUEST];
__declspec(dllimport) struct shrine_ppd shrine;

__declspec(dllimport) char hover_bless_text[120];
__declspec(dllimport) char hover_freeze_text[120];
__declspec(dllimport) char hover_potion_text[120];
__declspec(dllimport) char hover_rage_text[120];
__declspec(dllimport) char hover_level_text[120];
__declspec(dllimport) char hover_rank_text[120];
__declspec(dllimport) char hover_time_text[120];

__declspec(dllimport) int target_server;

__declspec(dllimport) char password[16];
__declspec(dllimport) char username[40];
__declspec(dllimport) int tick;
__declspec(dllimport) int mirror;
__declspec(dllimport) int realtime;
__declspec(dllimport) char server_url[256];
__declspec(dllimport) int server_port;
__declspec(dllimport) int want_width;
__declspec(dllimport) int want_height;
__declspec(dllimport) int sdl_scale;
__declspec(dllimport) int sdl_frames;
__declspec(dllimport) int sdl_multi;
__declspec(dllimport) int sdl_cache_size;
__declspec(dllimport) int frames_per_second;
__declspec(dllimport) uint64_t game_options;
__declspec(dllimport) int game_slowdown;


// ---------------- override-able functions, also exported from client ----------------
__declspec(dllimport) int _is_cut_sprite(int sprite);
__declspec(dllimport) int _is_mov_sprite(int sprite,int itemhint);
__declspec(dllimport) int _is_door_sprite(int sprite);
__declspec(dllimport) int _is_yadd_sprite(int sprite);
__declspec(dllimport) int _get_chr_height(int csprite);
__declspec(dllimport) int _trans_asprite(int mn,int sprite,int attick,unsigned char *pscale,unsigned char *pcr,unsigned char *pcg,unsigned char *pcb,unsigned char *plight,unsigned char *psat,unsigned short *pc1,unsigned short *pc2,unsigned short *pc3,unsigned short *pshine);
__declspec(dllimport) int _trans_charno(int csprite,int *pscale,int *pcr,int *pcg,int *pcb,int *plight,int *psat,int *pc1,int *pc2,int *pc3,int *pshine,int attick);
__declspec(dllimport) int _get_player_sprite(int nr,int zdir,int action,int step,int duration,int attick);
__declspec(dllimport) void _trans_csprite(int mn,struct map *cmap,int attick);
__declspec(dllimport) int _get_lay_sprite(int sprite,int lay);
__declspec(dllimport) int _get_offset_sprite(int sprite,int *px,int *py);
__declspec(dllimport) int _additional_sprite(int sprite,int attick);
__declspec(dllimport) int _opt_sprite(int sprite);
__declspec(dllimport) int _no_lighting_sprite(int sprite);
__declspec(dllimport) int _get_skltab_sep(int i);
__declspec(dllimport) int _get_skltab_index(int n);
__declspec(dllimport) int _get_skltab_show(int i);
__declspec(dllimport) int _do_display_random(void);
__declspec(dllimport) int _do_display_help(int nr);

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


