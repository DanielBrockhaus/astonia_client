/*
 * Part of Astonia Client (c) Daniel Brockhaus. Please read license.txt.
 */

void amod_init(void);
void amod_exit(void);
void amod_gamestart(void);
void amod_frame(void);
void amod_tick(void);

// Client exported functions
__declspec(dllimport) int note(const char *format,...) __attribute__((format(printf, 1, 2)));
__declspec(dllimport) int warn(const char *format,...) __attribute__((format(printf, 1, 2)));
__declspec(dllimport) int fail(const char *format,...) __attribute__((format(printf, 1, 2)));
__declspec(dllimport) void paranoia(const char *format,...) __attribute__((format(printf, 1, 2)));
__declspec(dllimport) void addline(const char *format,...) __attribute__((format(printf, 1, 2)));

// ignore
struct map;

// override-able functions, also exported from client
__declspec(dllimport) int _is_cut_sprite(int sprite);
__declspec(dllimport) int _is_mov_sprite(int sprite,int itemhint);
__declspec(dllimport) int _is_door_sprite(int sprite);
__declspec(dllimport) int _is_yadd_sprite(int sprite);
__declspec(dllimport) int _get_chr_height(int csprite);
__declspec(dllimport) int _trans_charno(int csprite,int *pscale,int *pcr,int *pcg,int *pcb,int *plight,int *psat,int *pc1,int *pc2,int *pc3,int *pshine,int attick);
__declspec(dllimport) int _get_player_sprite(int nr,int zdir,int action,int step,int duration,int attick);
__declspec(dllimport) void _trans_csprite(int mn,struct map *cmap,int attick);
__declspec(dllimport) int _get_lay_sprite(int sprite,int lay);
__declspec(dllimport) int _get_offset_sprite(int sprite,int *px,int *py);
__declspec(dllimport) int _additional_sprite(int sprite,int attick);
__declspec(dllimport) int _opt_sprite(int sprite);

// declarations for functions the mod might provide
int is_cut_sprite(int sprite);
int is_mov_sprite(int sprite,int itemhint);
int is_door_sprite(int sprite);
int is_yadd_sprite(int sprite);
int get_chr_height(int csprite);
int trans_charno(int csprite,int *pscale,int *pcr,int *pcg,int *pcb,int *plight,int *psat,int *pc1,int *pc2,int *pc3,int *pshine,int attick);
int get_player_sprite(int nr,int zdir,int action,int step,int duration,int attick);
void trans_csprite(int mn,struct map *cmap,int attick);
int get_lay_sprite(int sprite,int lay);
int get_offset_sprite(int sprite,int *px,int *py);
int additional_sprite(int sprite,int attick);
int opt_sprite(int sprite);

// ignore
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef abs
#define abs(a)	((a)<0 ? (-(a)) : (a))
#endif

#ifndef ARRAYSIZE
#define ARRAYSIZE(a) (sizeof(a)/sizeof((a)[0]))
#endif

struct complex_sprite {
    unsigned int sprite;
    unsigned short c1,c2,c3,shine;
    unsigned char cr,cg,cb;
    unsigned char light,sat;
    unsigned char scale;
};

struct map {
    // from map & item
    unsigned short int gsprite;     // background sprite
    unsigned short int gsprite2;    // background sprite
    unsigned short int fsprite;     // foreground sprite
    unsigned short int fsprite2;    // foreground sprite

    unsigned int isprite;       // item sprite
    unsigned short ic1,ic2,ic3;

    unsigned int flags;             // see CMF_

    // character
    unsigned int csprite;       // character base sprite
    unsigned int cn;        // character number (for commands)
    unsigned char cflags;       // character flags
    unsigned char action;       // character action, duration and step
    unsigned char duration;
    unsigned char step;
    unsigned char dir;      // direction the character is facing
    unsigned char health;       // character health (in percent)
    unsigned char mana;
    unsigned char shield;
    // 15 bytes

    // effects
    unsigned int ef[4];

    unsigned char sink;     // sink characters on this field
    int value;                      // testing purposes only
    int mmf;                        // more flags
    char rlight;                    // real client light - 0=invisible 1=dark, 14=normal (15=bright can't happen)
    struct complex_sprite rc;

    struct complex_sprite ri;

    struct complex_sprite rf;
    struct complex_sprite rf2;
    struct complex_sprite rg;
    struct complex_sprite rg2;

    char xadd;                      // add this to the x position of the field used for c sprite
    char yadd;                      // add this to the y position of the field used for c sprite
};

