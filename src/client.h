/*
 * Part of Astonia Client (c) Daniel Brockhaus. Please read license.txt.
 */

#define MAXCHARS	2048

#define MAXQUEST	100

#define MAXEF		64

#define CMF_LIGHT	(1+2+4+8)
#define CMF_VISIBLE	16
#define CMF_TAKE	32
#define CMF_USE		64
#define CMF_INFRA	128
#define CMF_UNDERWATER	256

#define VERSION         0x030100

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

#define V_PROFBASE	    (*game_v_profbase)  // 43
#define P_MAX		    20
#define V_MAX	        200

#define CL_NOP			1
#define CL_MOVE			2
#define CL_SWAP			3
#define CL_TAKE			4
#define CL_DROP			5
#define CL_KILL			6
#define CL_CONTAINER		7
#define CL_TEXT			8
#define CL_USE			9
#define CL_BLESS		10
#define CL_FIREBALL		11
#define CL_HEAL			12
#define CL_MAGICSHIELD		13
#define CL_FREEZE		14
#define CL_RAISE		15
#define CL_USE_INV		16
#define CL_FLASH		17
#define CL_BALL			18
#define CL_WARCRY		19
#define CL_LOOK_CONTAINER	20
#define CL_LOOK_MAP		21
#define CL_LOOK_INV		22
#define CL_LOOK_CHAR		23
#define CL_LOOK_ITEM		24
#define CL_GIVE			25
#define CL_SPEED		26
#define CL_STOP			27
#define CL_TAKE_GOLD		28
#define CL_DROP_GOLD		29
#define CL_JUNK_ITEM		30

#define CL_TICKER		33
#define CL_CONTAINER_FAST	34
#define CL_FASTSELL		35
#define CL_LOG			36
#define CL_TELEPORT		37
#define CL_PULSE		38
#define CL_PING			39
#define CL_GETQUESTLOG		40
#define CL_REOPENQUEST		41

#define PAC_IDLE	0
#define PAC_MOVE	1
#define PAC_TAKE	2
#define PAC_DROP	3
#define PAC_KILL	4
#define PAC_USE		5
#define PAC_BLESS	6
#define PAC_HEAL	7
#define PAC_FREEZE	8
#define PAC_FIREBALL	9
#define PAC_BALL	10
#define PAC_MAGICSHIELD	11
#define PAC_FLASH	12
#define PAC_WARCRY	13
#define PAC_LOOK_MAP	14
#define PAC_GIVE	15
#define PAC_BERSERK	16

#define INVENTORYSIZE	110
#define CONTAINERSIZE	(INVENTORYSIZE)

#define IF_USE		(1<<4)
#define IF_WNHEAD       (1<<5)	// can be worn on head
#define IF_WNNECK       (1<<6)	// etc...
#define IF_WNBODY       (1<<7)
#define IF_WNARMS       (1<<8)
#define IF_WNBELT       (1<<9)
#define IF_WNLEGS       (1<<10)
#define IF_WNFEET       (1<<11)
#define IF_WNLHAND      (1<<12)
#define IF_WNRHAND      (1<<13)
#define IF_WNCLOAK      (1<<14)
#define IF_WNLRING      (1<<15)
#define IF_WNRRING      (1<<16)
#define IF_WNTWOHANDED	(1<<17)	// two-handed weapon, fills both WNLHAND & WNRHAND

#define MAPDX			(DIST*2+1)
#define MAPDY			(DIST*2+1)
#define MAXMN           (MAPDX*MAPDY)

#define QF_OPEN		1
#define QF_DONE		2

#define QLF_REPEATABLE	(1u<<0)
#define QLF_XREPEAT	(1u<<1)

#define MAXSHRINE	256

struct quest {
    unsigned char done:6;
    unsigned char flags:2;
};

struct shrine_ppd {
    unsigned int used[MAXSHRINE/32];
    unsigned char continuity;
};

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

#define CL_MAX_SURFACE	32

struct client_surface {
    unsigned int xres:14;
    unsigned int yres:14;
    unsigned int type:4;
};

struct client_info {
    unsigned int skip;
    unsigned int idle;
    unsigned int vidmemtotal;
    unsigned int vidmemfree;
    unsigned int systemtotal;
    unsigned int systemfree;
    struct client_surface surface[CL_MAX_SURFACE];
};

extern struct map map[MAPDX*MAPDY];
extern struct map map2[MAPDX*MAPDY];

extern int value[2][V_MAX];
extern int *game_v_max;
extern int *game_v_profbase;
extern int item[INVENTORYSIZE];
extern int item_flags[INVENTORYSIZE];
extern int hp;
extern int mana;
extern int rage;
extern int endurance;
extern int lifeshield;
extern int experience;
extern int experience_used;
extern int gold;
extern int tick;
extern int lasttick;                    // ticks in inbuf
extern int q_size;

extern unsigned int cflags;     // current item (item under mouse cursor) flags
extern unsigned int csprite;    // and sprite

extern int con_cnt;
extern int con_type;
extern char con_name[80];
extern int container[CONTAINERSIZE];
extern int price[CONTAINERSIZE];
extern int itemprice[CONTAINERSIZE];
extern int cprice;
extern int protocol_version;

extern int lookinv[12];
extern int looklevel;
extern int mirror,newmirror;
extern int may_teleport[64+32];
extern int pspeed;   // 0=ill 1=stealth 2=normal 3=fast

extern char username[40];
extern char password[16];

extern union ceffect ceffect[MAXEF];
extern unsigned char ueffect[MAXEF];
extern struct player player[MAXCHARS];

extern int originx;
extern int originy;

extern int sockstate;
extern int socktimeout;
extern char *target_server;
extern int target_port;
extern int kicked_out;

extern char **game_skilldesc;
extern struct skill *game_skill;

extern int act;
extern int actx;
extern int acty;

extern int lookinv[12];
extern int looksprite,lookc1,lookc2,lookc3;
extern char look_name[80];
extern char look_desc[1024];

extern int realtime;
extern int mil_exp;
extern int display_gfx;
extern int display_time;

extern char pent_str[7][80];

extern struct quest quest[];
extern struct shrine_ppd shrine;

void cmd_text(char *text);
int mapmn(int x,int y);
int find_cn_ceffect(int cn,int skip);
int find_ceffect(int fn);
int level2exp(int level);
int exp2level(int val);
int raise_cost(int v,int n);
void cmd_move(int x,int y);
void cmd_swap(int with);
void cmd_use_inv(int with);
void cmd_take(int x,int y);
void cmd_look_map(int x,int y);
void cmd_look_item(int x,int y);
void cmd_look_inv(int pos);
void cmd_look_char(int cn);
void cmd_use(int x,int y);
void cmd_drop(int x,int y);
void cmd_speed(int mode);
void cmd_log(char *text);
void cmd_stop(void);
void cmd_kill(int cn);
void cmd_give(int cn);
void cmd_some_spell(int spell,int x,int y,int chr);
void cmd_raise(int vn);
void cmd_con(int pos);
void cmd_look_con(int pos);
void cmd_drop_gold(void);
void cmd_take_gold(int amount);
void cmd_junk_item(void);
void cmd_getquestlog(void);
void cmd_reopen_quest(int nr);
void cmd_fastsell(int with);
void cmd_con_fast(int pos);
void cmd_teleport(int nr);

int poll_network(void);
int next_tick(void);
int do_tick(void);
void cl_client_info(struct client_info *ci);
void cl_ticker(void);
int close_client(void);
int is_char_ceffect(int type);

