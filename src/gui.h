/*
 * Part of Astonia Client (c) Daniel Brockhaus. Please read license.txt.
 */

#include "dll.h"

#define MAXACTIONSLOT       14

struct quicks {
    int mn[9];      // 0 for invalid neighbours
    int qi[9];      // maxqick for invalid neighbours
    int mapx;
    int mapy;
    int cx;
    int cy;
};

typedef struct quicks QUICK;

extern int nocut;
extern unsigned int now;
extern int playersprite_override;
extern int mapaddx,mapaddy;
extern int mapoffx,mapoffy;
extern int mapaddx,mapaddy;   // small offset to smoothen walking
extern int plrmn;                      // mn of player
extern int itmsel;                     // mn
extern int chrsel;                     // mn
extern int mapsel;                     // mn

DLL_EXPORT extern unsigned short int healthcolor,manacolor,endurancecolor,shieldcolor;
DLL_EXPORT extern unsigned short int whitecolor,lightgraycolor,graycolor,darkgraycolor,blackcolor;
DLL_EXPORT extern unsigned short int lightredcolor,redcolor,darkredcolor;
DLL_EXPORT extern unsigned short int lightgreencolor,greencolor,darkgreencolor;
DLL_EXPORT extern unsigned short int lightbluecolor,bluecolor,darkbluecolor;
DLL_EXPORT extern unsigned short int lightorangecolor,orangecolor,darkorangecolor;
DLL_EXPORT extern unsigned short int textcolor;
extern int update_skltab;

extern int teleporter;
extern int show_tutor;
extern char tutor_text[1024];
extern int show_look;

void mtos(int mapx,int mapy,int *scrx,int *scry);
int stom(int scrx,int scry,int *mapx,int *mapy);
void set_mapoff(int cx,int cy,int mdx,int mdy);
void set_mapadd(int addx,int addy);
void update_user_keys(void);

int main_init(void);
int main_loop(void);
void main_exit(void);

extern int (*get_skltab_sep)(int i);
DLL_EXPORT int _get_skltab_sep(int i);
extern int (*get_skltab_index)(int n);
DLL_EXPORT int _get_skltab_index(int n);
extern int (*get_skltab_show)(int i);
DLL_EXPORT int _get_skltab_show(int i);

int context_getnm(void);
int context_action_enabled(void);
int hover_capture_text(char *line);
void hover_capture_tick(void);
void hover_invalidate_inv(int slot);
void hover_invalidate_inv_delayed(int slot);
void hover_invalidate_con(int slot);

extern int (*do_display_random)(void);
DLL_EXPORT int _do_display_random(void);

extern int (*do_display_help)(int);
DLL_EXPORT int _do_display_help(int nr);

extern char action_row[2][MAXACTIONSLOT];
extern int action_enabled;
void actions_loaded(void);

void minimap_clear(void);
void minimap_compact(void);

struct questlog{
    char *name;
    int minlevel,maxlevel;
    char *giver;
    char *area;
    int exp;
    unsigned int flags;
};
DLL_EXPORT extern struct questlog *game_questlog;
extern int questonscreen[10];
DLL_EXPORT extern int *game_questcount;
extern int gear_lock;
extern int last_right_click_invsel;

