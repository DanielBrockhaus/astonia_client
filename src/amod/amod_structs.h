/*
 * Part of Astonia Client (c) Daniel Brockhaus. Please read license.txt.
 */

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

#define bzero(ptr,size) memset(ptr,0,size)

#define V_MAX	        200
#define DIST		    25
#define MAPDX			(DIST*2+1)
#define MAPDY			(DIST*2+1)
#define MAXMN           (MAPDX*MAPDY)
#define INVENTORYSIZE	110
#define CONTAINERSIZE	(INVENTORYSIZE)
#define MAXCHARS	    2048
#define MAXEF		    64

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

#define IGET_R(c) ((((unsigned short int)(c))>>10)&0x1F)
#define IGET_G(c) ((((unsigned short int)(c))>>5)&0x1F)
#define IGET_B(c) ((((unsigned short int)(c))>>0)&0x1F)
#define IRGB(r,g,b) (((r)<<10)|((g)<<5)|((b)<<0))

#define DOT_TL          0       // top left?
#define DOT_BR          1       // bottom right?
#define DOT_WEA         2       // worn equipment
#define DOT_INV         3       // inventory
#define DOT_CON         4       // container
#define DOT_SCL         5       // scroll bar left, uses only X
#define DOT_SCR         6       // scroll bar right, uses only X
#define DOT_SCU         7       // scroll bars up arrows at this Y
#define DOT_SCD         8       // scroll bars down arrors at thy Y
#define DOT_TXT         9       // chat window
#define DOT_MTL         10      // map top left
#define DOT_MBR         11      // map bottom right
#define DOT_SKL         12      // skill list
#define DOT_GLD         13      // gold
#define DOT_JNK         14      // trashcan
#define DOT_MOD         15      // speed mode
#define DOT_MCT         16      // map center
#define DOT_TOP         17      // top left corner of equipment bar
#define DOT_BOT         18      // top left corner of bottom window holding skills, chat, etc.
#define DOT_TX2         19      // chat window bottom right
#define DOT_SK2         20      // skill list window bottom right
#define DOT_IN1         21      // inventory top left
#define DOT_IN2         22      // inventory bottom right
#define DOT_HLP         23      // help top left
#define DOT_HL2         24      // help bottom right
#define DOT_TEL         25      // teleporter top left
#define DOT_COL         26      // color picker top left
#define DOT_LOK         27      // look at character window (show_look), top left
#define DOT_BO2         28      // bottom right of bottom window
#define DOT_ACT         29      // action bar top left
#define DOT_SSP         30      // self-spell-bars top left
#define DOT_TUT         31      // tutor window top left
#define MAX_DOT         32

#define V_HP		0
#define V_ENDURANCE	1
#define V_MANA		2

#define V_WIS         	3
#undef  V_INT           // everyone likes windoof
#define V_INT          	4
#define V_AGI         	5
#define V_STR       	6

#define V_ARMOR		7
#define V_WEAPON	8
#define V_LIGHT		9
#define V_SPEED		10

#define V_PULSE		11
#define V_DAGGER       	12
#define V_HAND         	13
#define V_STAFF        	14
#define V_SWORD        	15
#define V_TWOHAND      	16

#define V_ARMORSKILL   	17
#define V_ATTACK       	18
#define V_PARRY	       	19
#define V_WARCRY       	20
#define V_TACTICS      	21
#define V_SURROUND     	22
#define V_BODYCONTROL	23
#define V_SPEEDSKILL	24

#define V_BARTER       	25
#define V_PERCEPT      	26
#define V_STEALTH      	27

#define V_BLESS		28
#define V_HEAL		29
#define V_FREEZE	30
#define V_MAGICSHIELD	31
#define V_FLASH		32

#define V_FIREBALL	33
//#define V_BALL		34

#define V_REGENERATE	35
#define V_MEDITATE	36
#define V_IMMUNITY	37

#define V_DEMON		38
#define V_DURATION  39
#define V_RAGE		40
#define V_COLD		41
#define V_PROFESSION	42

#define SDL_MOUM_LUP        1
#define SDL_MOUM_LDOWN      2
#define SDL_MOUM_RUP        3
#define SDL_MOUM_RDOWN      4
#define SDL_MOUM_MUP        5
#define SDL_MOUM_MDOWN      6
#define SDL_MOUM_WHEEL      7

#define MAXSHRINE	256
#define MAXQUEST	100

#define CMF_LIGHT	(1+2+4+8)
#define CMF_VISIBLE	16
#define CMF_TAKE	32
#define CMF_USE		64
#define CMF_INFRA	128
#define CMF_UNDERWATER	256

#define MMF_SIGHTBLOCK  (1<<1)  // indicates sight block (set_map_lights)
#define MMF_DOOR        (1<<2)  // a door - helpful when cutting sprites - (set_map_sprites)
#define MMF_CUT         (1<<3)  // indicates cut (set_map_cut)

#define SV_MOD1         58
#define SV_MOD2         59
#define SV_MOD3         60
#define SV_MOD4         61
#define SV_MOD5         62

#define DDFX_NLIGHT             15
#define DDFX_BRIGHT             0

#define XRES    800
#define YRES    (__yres)

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

struct skill {
    char name[80];
    int base1,base2,base3;
    int cost;       // 0=not raisable, 1=skill, 2=attribute, 3=power
    int start;      // start value, pts up to this value are free
};

struct skltab {
    int v;                          // negative v-values indicate a special display (empty lines, negative exp, etc...)
    int button;         // show button
    char name[80];
    int base;
    int curr;
    int raisecost;
    int barsize;                    // positive is blue, negative is red
};

typedef struct skltab SKLTAB;

struct player {
    char name[80];
    int csprite;
    short level;
    unsigned short c1,c2,c3;
    unsigned char clan;
    unsigned char pk_status;
};

struct cef_generic {
    int nr;
    int type;
};

struct cef_shield {
    int nr;
    int type;
    int cn;
    int start;
};

struct cef_strike {
    int nr;
    int type;
    int cn;
    int x,y;    // target
};

struct cef_ball {
    int nr;
    int type;
    int start;
    int frx,fry;    // high precision coords
    int tox,toy;    // high precision coords
};

struct cef_fireball {
    int nr;
    int type;
    int start;
    int frx,fry;    // high precision coords
    int tox,toy;    // high precision coords
};

struct cef_edemonball {
    int nr;
    int type;
    int start;
    int base;
    int frx,fry;    // high precision coords
    int tox,toy;    // high precision coords
};

struct cef_flash {
    int nr;
    int type;
    int cn;
};

struct cef_explode {
    int nr;
    int type;
    int start;
    int base;
};

struct cef_warcry {
    int nr;
    int type;
    int cn;
    int stop;
};

struct cef_bless {
    int nr;
    int type;
    int cn;
    int start;
    int stop;
    int strength;
};

struct cef_heal {
    int nr;
    int type;
    int cn;
    int start;
};

struct cef_freeze {
    int nr;
    int type;
    int cn;
    int start;
    int stop;
};

struct cef_burn {
    int nr;
    int type;
    int cn;
    int stop;
};

struct cef_mist {
    int nr;
    int type;
    int start;
};

struct cef_pulse {
    int nr;
    int type;
    int start;
};

struct cef_pulseback {
    int nr;
    int type;
    int cn;
    int x,y;
};

struct cef_potion {
    int nr;
    int type;
    int cn;
    int start;
    int stop;
    int strength;
};

struct cef_earthrain {
    int nr;
    int type;
    int strength;
};

struct cef_earthmud {
    int nr;
    int type;
};

struct cef_curse {
    int nr;
    int type;
    int cn;
    int start;
    int stop;
    int strength;
};

struct cef_cap {
    int nr;
    int type;
    int cn;
};

struct cef_lag {
    int nr;
    int type;
    int cn;
};

struct cef_firering {
    int nr;
    int type;
    int cn;
    int start;
};

struct cef_bubble {
    int nr;
    int type;
    int yoff;
};

union ceffect {
    struct cef_generic generic;
    struct cef_shield shield;
    struct cef_strike strike;
    struct cef_ball ball;
    struct cef_fireball fireball;
    struct cef_flash flash;
    struct cef_explode explode;
    struct cef_warcry warcry;
    struct cef_bless bless;
    struct cef_heal heal;
    struct cef_freeze freeze;
    struct cef_burn burn;
    struct cef_mist mist;
    struct cef_potion potion;
    struct cef_earthrain earthrain;
    struct cef_earthmud earthmud;
    struct cef_edemonball edemonball;
    struct cef_curse curse;
    struct cef_cap cap;
    struct cef_lag lag;
    struct cef_pulse pulse;
    struct cef_pulseback pulseback;
    struct cef_firering firering;
    struct cef_bubble bubble;
};

#define QLF_REPEATABLE	(1u<<0)
#define QLF_XREPEAT	(1u<<1)

struct questlog {
    char *name;
    int minlevel,maxlevel;
    char *giver;
    char *area;
    int exp;
    unsigned int flags;
};

struct quest {
    unsigned char done:6;
    unsigned char flags:2;
};

struct shrine_ppd {
    unsigned int used[MAXSHRINE/32];
    unsigned char continuity;
};

