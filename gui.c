/*
 * Part of Astonia Client (c) Daniel Brockhaus. Please read license.txt.
 */

#include <windows.h>
#include <psapi.h>
#include <stdarg.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <time.h>
#include <SDL2/SDL.h>

#include "astonia.h"
#include "engine.h"

#define ISCLIENT
#define WANTMAPMN
#include "main.h"
#include "dd.h"
#include "client.h"
#include "skill.h"
#include "sprite.h"
#include "gui.h"
#include "sound.h"
#include "sdl.h"

// extern

extern int quit;
void set_map_values(struct map *cmap,int attick);
void init_game(int mcx,int mcy);
extern void exit_game(void);
void display_cmd();
void quest_select(int nr);
extern int gfx_force_png;
extern int gfx_force_dh;
extern int mirror,newmirror;

uint64_t gui_time_misc=0;

#define MAXHELP		24
#define MAXQUEST2	10

void cmd_add_text(char *buf);

// globals

int skip=1,idle=0,tota=1,frames=0;
int winxres,winyres;

// globals display

#ifdef DEVELOPER
int display_vc=1;
#else
int display_vc=0;
#endif
int display_help=0,display_quest=0;

int playersprite_override=0;
int nocut=0;

int update_skltab=0;
int show_look=0;

int gui_topoff;     // offset of the top bar *above* the top of the window (0 ... -38)

unsigned short int healthcolor,manacolor,endurancecolor,shieldcolor;
unsigned short int whitecolor,lightgraycolor,graycolor,darkgraycolor,blackcolor;
unsigned short int lightredcolor,redcolor,darkredcolor;
unsigned short int lightgreencolor,greencolor,darkgreencolor;
unsigned short int lightbluecolor,bluecolor,darkbluecolor;
unsigned short int textcolor;
unsigned short int lightorangecolor,orangecolor,darkorangecolor;

unsigned int now;

int cur_cursor=0;
int mousex=XRES/2,mousey=YRES/2,vk_shift,vk_control,vk_alt,vk_rbut,vk_lbut,shift_override=0,control_override=0;
int mousedx,mousedy;
int vk_item,vk_char,vk_spell;

DOT *dot=NULL;
BUT *but=NULL;

// globals wea

int weatab[12]={9,6,8,11,0,1,2,4,5,3,7,10};
char weaname[12][32]={"RING","HAND","HAND","RING","NECK","HEAD","BACK","BODY","BELT","ARMS","LEGS","FEET"};

// globals skltab

#define STV_EMPTYLINE           -1
#define STV_JUSTAVALUE          -2      // value is in curr

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

// globals spell

struct spell {
    int cl;                         // id of spell sent to server (0=look/spellmode change)
    char name[40];                  // name in text display
};

typedef struct spell SPELL;

// globals keytab

#define TGT_MAP 1
#define TGT_ITM 2
#define TGT_CHR 3
#define TGT_SLF 4

struct keytab {
    int keycode;
    int userdef;
    int vk_item,vk_char,vk_spell;
    char name[40];
    int tgt;
    int cl_spell;
    int skill;
    unsigned int usetime;
};

typedef struct keytab KEYTAB;

KEYTAB keytab[]={
    {'1',0,0,1,0,"FIREBALL",TGT_CHR,CL_FIREBALL,V_FIREBALL},
    {'2',0,0,1,0,"LIGHTNINGBALL",TGT_CHR,CL_BALL,V_FLASH},
    {'3',0,0,1,0,"FLASH",TGT_SLF,CL_FLASH,V_FLASH},
    {'4',0,0,1,0,"FREEZE",TGT_SLF,CL_FREEZE,V_FREEZE},
    {'5',0,0,1,0,"SHIELD",TGT_SLF,CL_MAGICSHIELD,V_MAGICSHIELD},
    {'6',0,0,1,0,"BLESS",TGT_CHR,CL_BLESS,V_BLESS},
    {'7',0,0,1,0,"HEAL",TGT_CHR,CL_HEAL,V_HEAL},
    {'8',0,0,1,0,"WARCRY",TGT_SLF,CL_WARCRY,V_WARCRY},
    {'9',0,0,1,0,"PULSE",TGT_SLF,CL_PULSE,V_PULSE},
    {'0',0,0,1,0,"FIRERING",TGT_SLF,CL_FIREBALL,V_FIREBALL},

    {'1',0,0,1,1,"FIREBALL",TGT_CHR,CL_FIREBALL,V_FIREBALL},
    {'2',0,0,1,1,"LIGHTNINGBALL",TGT_CHR,CL_BALL,V_FLASH},
    {'3',0,0,1,1,"FLASH",TGT_SLF,CL_FLASH,V_FLASH},
    {'4',0,0,1,1,"FREEZE",TGT_SLF,CL_FREEZE,V_FREEZE},
    {'5',0,0,1,1,"SHIELD",TGT_SLF,CL_MAGICSHIELD,V_MAGICSHIELD},
    {'6',0,0,1,1,"BLESS",TGT_CHR,CL_BLESS,V_BLESS},
    {'7',0,0,1,1,"HEAL",TGT_CHR,CL_HEAL,V_HEAL},
    {'8',0,0,1,1,"WARCRY",TGT_SLF,CL_WARCRY,V_WARCRY},
    {'9',0,0,1,1,"PULSE",TGT_SLF,CL_PULSE,V_PULSE},
    {'0',0,0,1,1,"FIRERING",TGT_SLF,CL_FIREBALL,V_FIREBALL},

    {'1',0,0,0,1,"FIREBALL",TGT_MAP,CL_FIREBALL,V_FIREBALL},
    {'2',0,0,0,1,"LIGHTNINGBALL",TGT_MAP,CL_BALL,V_FLASH},
    {'3',0,0,0,1,"FLASH",TGT_SLF,CL_FLASH,V_FLASH},
    {'4',0,0,0,1,"FREEZE",TGT_SLF,CL_FREEZE,V_FREEZE},
    {'5',0,0,0,1,"SHIELD",TGT_SLF,CL_MAGICSHIELD,V_MAGICSHIELD},
    {'6',0,0,0,1,"BLESS SELF",TGT_SLF,CL_BLESS,V_BLESS},
    {'7',0,0,0,1,"HEAL SELF",TGT_SLF,CL_HEAL,V_HEAL},
    {'8',0,0,0,1,"WARCRY",TGT_SLF,CL_WARCRY,V_WARCRY},
    {'9',0,0,0,1,"PULSE",TGT_SLF,CL_PULSE,V_PULSE},
    {'0',0,0,0,1,"FIRERING",TGT_SLF,CL_FIREBALL,V_FIREBALL},
};

int max_keytab=sizeof(keytab)/sizeof(KEYTAB);

struct special_tab {
    char *name;
    int shift_over;
    int control_over;
    int spell,target;
    int req;
};

int fkeyitem[4];

// globals cmd

int plrmn;                      // mn of player

int mapsel;                     // mn
int itmsel;                     // mn
int chrsel;                     // mn
int invsel;                     // index into item
int weasel;                     // index into weatab
int consel;                     // index into item
int splsel;
int sklsel;
int butsel;                     // is always set, if any of the others is set
int telsel;
int helpsel;
int questsel;
int colsel;
int skl_look_sel;

int capbut;                     // the button capturing the mouse

int takegold;                   // the amout of gold to take

char hitsel[256];               // something in the text (dx_drawtext()) is selected

SKLTAB *skltab=NULL;
int skltab_max=0;
int skltab_cnt=0;

int invoff,max_invoff;
int conoff,max_conoff;
int skloff,max_skloff;

int lcmd;
int rcmd;

int curspell_l=4;               // index into spelltab
int curspell_r=6;               // index into spelltab

#define CMD_NONE                0
#define CMD_MAP_MOVE            1
#define CMD_MAP_DROP            2

#define CMD_ITM_TAKE            3
#define CMD_ITM_USE             4
#define CMD_ITM_USE_WITH        5

#define CMD_CHR_ATTACK          6
#define CMD_CHR_GIVE            7

#define CMD_INV_USE             8
#define CMD_INV_USE_WITH        9
#define CMD_INV_TAKE            10
#define CMD_INV_SWAP            11
#define CMD_INV_DROP            12

#define CMD_WEA_USE             13
#define CMD_WEA_USE_WITH        14
#define CMD_WEA_TAKE            15
#define CMD_WEA_SWAP            16
#define CMD_WEA_DROP            17

#define CMD_CON_TAKE            18
#define CMD_CON_BUY             19
#define CMD_CON_SWAP            20
#define CMD_CON_DROP            21
#define CMD_CON_SELL            22

#define CMD_MAP_LOOK            23
#define CMD_ITM_LOOK            24
#define CMD_CHR_LOOK            25
#define CMD_INV_LOOK            26
#define CMD_WEA_LOOK            27
#define CMD_CON_LOOK            28

#define CMD_MAP_CAST_L          29
#define CMD_ITM_CAST_L          30
#define CMD_CHR_CAST_L          31
#define CMD_MAP_CAST_R          32
#define CMD_ITM_CAST_R          33
#define CMD_CHR_CAST_R          34
#define CMD_MAP_CAST_K        	35
#define CMD_CHR_CAST_K        	36
#define CMD_SLF_CAST_K        	37

#define CMD_SPL_SET_L           38
#define CMD_SPL_SET_R           39

#define CMD_SKL_RAISE           40

#define CMD_INV_OFF_UP          41
#define CMD_INV_OFF_DW          42
#define CMD_INV_OFF_TR          43

#define CMD_SKL_OFF_UP          44
#define CMD_SKL_OFF_DW          45
#define CMD_SKL_OFF_TR          46

#define CMD_CON_OFF_UP          47
#define CMD_CON_OFF_DW          48
#define CMD_CON_OFF_TR          49

#define CMD_USE_FKEYITEM        50

#define CMD_SAY_HITSEL          51

#define CMD_DROP_GOLD           52
#define CMD_TAKE_GOLD           53

#define CMD_JUNK_ITEM           54

#define CMD_SPEED0              55
#define CMD_SPEED1              56
#define CMD_SPEED2              57

#define CMD_CON_FASTTAKE	    61
#define CMD_CON_FASTBUY		    62
#define CMD_CON_FASTSELL	    63
#define CMD_TELEPORT		    64
#define CMD_CON_FASTDROP	    65

#define CMD_HELP_NEXT           66
#define CMD_HELP_PREV           67
#define CMD_HELP_MISC           68
#define CMD_HELP_CLOSE		    69
#define CMD_EXIT		        70
#define CMD_HELP		        71
#define CMD_NOLOOK		        72

#define CMD_COLOR		        73
#define CMD_SKL_LOOK		    74
#define CMD_QUEST		        75

// dot and but helpers

void set_dot(int didx,int x,int y,int flags) {
    PARANOIA(if (didx<0 || didx>=MAX_DOT) paranoia("set_dot: ill didx=%d",didx); )

    dot[didx].flags=flags;
    dot[didx].x=x;
    dot[didx].y=y;
}

int dotx(int didx) {
    return dot[didx].x;
}

int doty(int didx) {
    if (dot[didx].flags&DOTF_TOPOFF) return dot[didx].y+gui_topoff;
    return dot[didx].y;
}

void set_but(int bidx,int x,int y,int hitrad,int id,int val,int flags) {
    PARANOIA(if (bidx<0 || bidx>=MAX_BUT) paranoia("set_but: ill bidx=%d",bidx); )

    but[bidx].flags=flags;

    but[bidx].id=id;
    but[bidx].val=val;

    but[bidx].x=x;
    but[bidx].y=y;

    but[bidx].sqhitrad=hitrad*hitrad;
}

int butx(int bidx) {
    return but[bidx].x;
}

int buty(int bidx) {
    if (but[bidx].flags&BUTF_TOPOFF) return but[bidx].y+gui_topoff;
    return but[bidx].y;
}

// transformation

int mapoffx,mapoffy;
int mapaddx,mapaddy;   // small offset to smoothen walking

// set where to place the *center* of the map on the screen
void set_mapoff(int cx,int cy,int mdx,int mdy) {
    mapoffx=(cx-(mdx/2-mdy/2)*(FDX/2));
    mapoffy=(cy-(mdx/2+mdy/2)*(FDY/2));
}

// small offset to smoothen walking
// is set to the offset of the character in the center of the map (the player)
void set_mapadd(int addx,int addy) {
    mapaddx=addx;
    mapaddy=addy;
}

// map to screen
void mtos(int mapx,int mapy,int *scrx,int *scry) {
    *scrx=(mapoffx+mapaddx)+(mapx-mapy)*(FDX/2);
    *scry=(mapoffy+mapaddy)+(mapx+mapy)*(FDY/2);
}

// screen to map
void stom(int scrx,int scry,int *mapx,int *mapy) {
    extern int stom_off_x,stom_off_y;
    scrx-=stom_off_x;
    scry-=stom_off_y;
    scrx-=(mapoffx+mapaddx);
    scry-=(mapoffy+mapaddy)-10;
    *mapy=(40*scry-20*scrx-1)/(20*40);      // ??? -1 ???
    *mapx=(40*scry+20*scrx)/(20*40);
}

// dx

void dx_copysprite_emerald(int scrx,int scry,int emx,int emy) {
    DDFX ddfx;

    bzero(&ddfx,sizeof(ddfx));
    ddfx.sprite=37;
    ddfx.align=DD_OFFSET;
    ddfx.clipsx=emx*10;
    ddfx.clipsy=emy*10;
    ddfx.clipex=ddfx.clipsx+10;
    ddfx.clipey=ddfx.clipsy+10;
    ddfx.ml=ddfx.ll=ddfx.rl=ddfx.ul=ddfx.dl=DDFX_NLIGHT;
    ddfx.scale=100;
    dd_copysprite_fx(&ddfx,scrx-ddfx.clipsx-5,scry-ddfx.clipsy-5);
}

void dx_drawtext_gold(int x,int y,unsigned short int color,int amount) {
    if (amount>99) dd_drawtext_fmt(x,y,color,DD_CENTER|DD_FRAME|DD_SMALL,"%d.%02dG",amount/100,amount%100);
    else dd_drawtext_fmt(x,y,color,DD_CENTER|DD_FRAME|DD_SMALL,"%ds",amount);
}

// display

static void display_wear(void) {
    int b,i,x,y,yt;
    unsigned int sprite;
    unsigned short c1,c2,c3,shine;
    unsigned char scale,cr,cg,cb,light,sat;
    DDFX fx;

    for (b=BUT_WEA_BEG; b<=BUT_WEA_END; b++) {

        i=b-BUT_WEA_BEG;

        x=butx(b);
        y=buty(b);
        yt=y+23;

        dd_copysprite(SPR_ITPAD,x,y,DDFX_NLIGHT,DD_CENTER);
        if (i==weasel) dd_copysprite(SPR_ITSEL,x,y,DDFX_NLIGHT,DD_CENTER);
        if (item[weatab[i]]) {

            bzero(&fx,sizeof(fx));

            sprite=trans_asprite(0,item[weatab[i]],tick,&scale,&cr,&cg,&cb,&light,&sat,&c1,&c2,&c3,&shine);
            fx.sprite=sprite;
            fx.c1=c1;
            fx.c2=c2;
            fx.c3=c3;
            fx.cr=cr;
            fx.cg=cg;
            fx.cb=cb;
            fx.clight=light;
            fx.sat=sat;
            fx.shine=shine;
            fx.scale=scale;
            fx.sink=0;
            fx.align=DD_CENTER;
            fx.ml=fx.ll=fx.rl=fx.ul=fx.dl=i==weasel?FX_ITEMBRIGHT:FX_ITEMLIGHT;

            dd_copysprite_fx(&fx,x,y);
        }

        if (butsel>=BUT_WEA_BEG && butsel<=BUT_WEA_END && !vk_item && capbut==-1) dd_drawtext(x,yt,textcolor,DD_CENTER|DD_SMALL|DD_FRAME,weaname[i]);

        if ((cflags&IF_WNRRING) && i==0) dd_drawtext(x,yt,whitecolor,DD_CENTER|DD_SMALL|DD_FRAME,weaname[i]);
        if ((cflags&IF_WNRHAND) && i==1) dd_drawtext(x,yt,whitecolor,DD_CENTER|DD_SMALL|DD_FRAME,weaname[i]);
        if ((cflags&IF_WNLHAND) && i==2 && !(cflags&IF_WNTWOHANDED)) dd_drawtext(x,yt,whitecolor,DD_CENTER|DD_SMALL|DD_FRAME,weaname[i]);
        if ((cflags&IF_WNTWOHANDED) && i==2) dd_drawtext(x,yt,redcolor,DD_CENTER|DD_SMALL|DD_FRAME,weaname[i]);
        if ((cflags&IF_WNLRING) && i==3) dd_drawtext(x,yt,whitecolor,DD_CENTER|DD_SMALL|DD_FRAME,weaname[i]);
        if ((cflags&IF_WNNECK) && i==4) dd_drawtext(x,yt,whitecolor,DD_CENTER|DD_SMALL|DD_FRAME,weaname[i]);
        if ((cflags&IF_WNHEAD) && i==5) dd_drawtext(x,yt,whitecolor,DD_CENTER|DD_SMALL|DD_FRAME,weaname[i]);
        if ((cflags&IF_WNCLOAK) && i==6) dd_drawtext(x,yt,whitecolor,DD_CENTER|DD_SMALL|DD_FRAME,weaname[i]);
        if ((cflags&IF_WNBODY) && i==7) dd_drawtext(x,yt,whitecolor,DD_CENTER|DD_SMALL|DD_FRAME,weaname[i]);
        if ((cflags&IF_WNBELT) && i==8) dd_drawtext(x,yt,whitecolor,DD_CENTER|DD_SMALL|DD_FRAME,weaname[i]);
        if ((cflags&IF_WNARMS) && i==9) dd_drawtext(x,yt,whitecolor,DD_CENTER|DD_SMALL|DD_FRAME,weaname[i]);
        if ((cflags&IF_WNLEGS) && i==10) dd_drawtext(x,yt,whitecolor,DD_CENTER|DD_SMALL|DD_FRAME,weaname[i]);
        if ((cflags&IF_WNFEET) && i==11) dd_drawtext(x,yt,whitecolor,DD_CENTER|DD_SMALL|DD_FRAME,weaname[i]);

        if (i==2 && item[weatab[1]] && (item_flags[weatab[1]]&IF_WNTWOHANDED)) dd_copysprite(5,x,y,DDFX_NLIGHT,DD_CENTER);

        if (con_cnt && con_type==2 && itemprice[weatab[i]]) dx_drawtext_gold(x,y+12,textcolor,itemprice[weatab[i]]);
    }
}

static void display_look(void) {
    int b,i,x,y; //,yt;
    unsigned int sprite;
    unsigned short c1,c2,c3,shine;
    unsigned char scale,cr,cg,cb,light,sat;
    DDFX fx;
    extern char look_name[],look_desc[];
    extern int looksprite,lookc1,lookc2,lookc3;
    static int look_anim=4,look_step=0,look_dir=0;

    dd_copysprite(opt_sprite(994),151,50,DDFX_NLIGHT,DD_NORMAL);

    for (b=BUT_WEA_BEG; b<=BUT_WEA_END; b++) {
        i=b-BUT_WEA_BEG;

        // Intentionally using the array directly here
        // since we want the non-sliding Y
        x=but[b].x;
        y=but[b].y+50;

        dd_copysprite(SPR_ITPAD,x,y,DDFX_NLIGHT,DD_CENTER);
        if (lookinv[weatab[i]]) {

            bzero(&fx,sizeof(fx));

            sprite=trans_asprite(0,lookinv[weatab[i]],tick,&scale,&cr,&cg,&cb,&light,&sat,&c1,&c2,&c3,&shine);
            fx.sprite=sprite;
            fx.c1=c1;
            fx.c2=c2;
            fx.c3=c3;
            fx.shine=shine;
            fx.cr=cr;
            fx.cg=cg;
            fx.cb=cb;
            fx.clight=light;
            fx.sat=sat;
            fx.scale=scale;
            fx.sink=0;
            fx.align=DD_CENTER;
            fx.ml=fx.ll=fx.rl=fx.ul=fx.dl=FX_ITEMLIGHT;
            dd_copysprite_fx(&fx,x,y);
        }
    }
    dd_drawtext(220,100,0xffff,DD_LEFT,look_name);
    dd_drawtext_break(220,110,440,0xffff,DD_LEFT,look_desc);

    {
        int csprite,scale,cr,cg,cb,light,sat,c1,c2,c3,shine;

        bzero(&fx,sizeof(fx));

        csprite=trans_charno(looksprite,&scale,&cr,&cg,&cb,&light,&sat,&c1,&c2,&c3,&shine,tick);

        fx.sprite=get_player_sprite(csprite,look_dir,look_anim,look_step,16,tick);
        look_step++;
        if (look_step==16) {
            look_step=0;
            look_anim++;
            if (look_anim>6) {
                look_anim=4;
                look_dir+=2;
                if (look_dir>7) look_dir=0;
            }
        }
        fx.scale=scale;
        fx.shine=shine;
        fx.cr=cr;
        fx.cg=cg;
        fx.cb=cb;
        fx.clight=light;
        fx.sat=sat;

        if (looksprite<120) {
            fx.c1=lookc1;
            fx.c2=lookc2;
            fx.c3=lookc3;
        } else {
            fx.c1=c1;
            fx.c2=c2;
            fx.c3=c3;
        }
        fx.sink=0;
        fx.align=DD_OFFSET;
        fx.ml=fx.ll=fx.rl=fx.ul=fx.dl=FX_ITEMLIGHT;
        dd_copysprite_fx(&fx,190,160);
    }
}

static void display_inventory(void) {
    int b,i,x,y,yt;
    int c; // ,fkey[4];
    static char *fstr[4]={"F1","F2","F3","F4"};
    unsigned int sprite;
    unsigned short c1,c2,c3,shine;
    unsigned char scale,cr,cg,cb,light,sat;
    DDFX fx;

    // fkey[0]=fkey[1]=fkey[2]=fkey[3]=0;

    for (b=BUT_INV_BEG; b<=BUT_INV_END; b++) {

        i=30+invoff*INVDX+b-BUT_INV_BEG;
        c=(i-2)%4;

        x=butx(b);
        y=buty(b);
        yt=y+12;

        dd_copysprite(SPR_ITPAD,x,y,DDFX_NLIGHT,DD_CENTER);
        if (i==invsel) dd_copysprite(SPR_ITSEL,x,y,DDFX_NLIGHT,DD_CENTER);
        if (item[i]) {

            bzero(&fx,sizeof(fx));

            sprite=trans_asprite(0,item[i],tick,&scale,&cr,&cg,&cb,&light,&sat,&c1,&c2,&c3,&shine);
            fx.sprite=sprite;
            fx.shine=shine;
            fx.c1=c1;
            fx.c2=c2;
            fx.c3=c3;
            fx.cr=cr;
            fx.cg=cg;
            fx.cb=cb;
            fx.clight=light;
            fx.sat=sat;
            fx.scale=scale;
            fx.sink=0;
            fx.align=DD_CENTER;
            fx.ml=fx.ll=fx.rl=fx.ul=fx.dl=i==invsel?FX_ITEMBRIGHT:FX_ITEMLIGHT;
            dd_copysprite_fx(&fx,x,y);
            if ((sprite=additional_sprite(item[i],tick))!=0) {
                fx.sprite=sprite;
                dd_copysprite_fx(&fx,x,y);
            }

        }
        if (fkeyitem[c]==i) dd_drawtext(x,y-18,textcolor,DD_SMALL|DD_CENTER|DD_FRAME,fstr[c]);
        if (con_cnt && con_type==2 && itemprice[i]) dx_drawtext_gold(x,yt,textcolor,itemprice[i]);
    }
}

static void display_container(void) {
    int b,i,x,y,yt;
    unsigned short int color;
    unsigned int sprite;
    unsigned short c1,c2,c3,shine;
    unsigned char scale,cr,cg,cb,light,sat;
    DDFX fx;

    dd_copysprite(SPR_TEXTF,dot[DOT_CON].x-20,dot[DOT_CON].y-55,DDFX_NLIGHT,DD_NORMAL);
    if (con_type==1) dd_drawtext(dot[DOT_CON].x,dot[DOT_CON].y-50+2,textcolor,DD_LEFT|DD_LARGE,con_name);
    else dd_drawtext_fmt(dot[DOT_CON].x,dot[DOT_CON].y-50+2,textcolor,DD_LEFT|DD_LARGE,"%s's Shop",con_name);

    for (b=BUT_CON_BEG; b<=BUT_CON_END; b++) {

        i=conoff*CONDX+b-BUT_CON_BEG;

        x=butx(b);
        y=buty(b);
        yt=y+12;

        dd_copysprite(SPR_ITPAD,x,y,DDFX_NLIGHT,DD_CENTER);
        if (i==consel) dd_copysprite(SPR_ITSEL,x,y,DDFX_NLIGHT,DD_CENTER);
        if (i>=con_cnt) continue;
        if (container[i]) {
            bzero(&fx,sizeof(fx));

            sprite=trans_asprite(0,container[i],tick,&scale,&cr,&cg,&cb,&light,&sat,&c1,&c2,&c3,&shine);
            fx.sprite=sprite;
            fx.shine=shine;
            fx.c1=c1;
            fx.c2=c2;
            fx.c3=c3;
            fx.cr=cr;
            fx.cg=cg;
            fx.cb=cb;
            fx.clight=light;
            fx.sat=sat;
            fx.scale=scale;
            fx.sink=0;
            fx.align=DD_CENTER;
            fx.ml=fx.ll=fx.rl=fx.ul=fx.dl=i==consel?FX_ITEMBRIGHT:FX_ITEMLIGHT;
            dd_copysprite_fx(&fx,x,y);
        }

        if (con_type==2 && price[i]) {
            if (price[i]>gold && i!=consel) color=darkredcolor;
            else if (price[i]>gold && i==consel) color=redcolor;
            else if (i==consel) color=whitecolor;
            else color=textcolor;

            dx_drawtext_gold(x,yt,color,price[i]);
        }
    }
}

static void display_scrollbars() {
    // TODO: SPR_SCRBAR (image 26) is a single red pixel. This must be wrong?
    //dd_copysprite(SPR_SCRBAR,but[BUT_SCL_UP].x,but[BUT_SCL_UP].y+(but[BUT_SCL_DW].y-but[BUT_SCL_UP].y)/2,DDFX_NLIGHT,DD_OFFSET);
    dd_copysprite(SPR_SCRUP,but[BUT_SCL_UP].x,but[BUT_SCL_UP].y,butsel==BUT_SCL_UP?FX_ITEMBRIGHT:FX_ITEMLIGHT,DD_OFFSET);
    dd_copysprite(SPR_SCRLT,but[BUT_SCL_TR].x,but[BUT_SCL_TR].y,butsel==BUT_SCL_TR?FX_ITEMBRIGHT:FX_ITEMLIGHT,DD_OFFSET);
    dd_copysprite(SPR_SCRDW,but[BUT_SCL_DW].x,but[BUT_SCL_DW].y,butsel==BUT_SCL_DW?FX_ITEMBRIGHT:FX_ITEMLIGHT,DD_OFFSET);

    //dd_copysprite(SPR_SCRBAR,but[BUT_SCR_UP].x,but[BUT_SCR_UP].y+(but[BUT_SCR_DW].y-but[BUT_SCR_UP].y)/2,DDFX_NLIGHT,DD_OFFSET);
    dd_copysprite(SPR_SCRUP,but[BUT_SCR_UP].x,but[BUT_SCR_UP].y,butsel==BUT_SCR_UP?FX_ITEMBRIGHT:FX_ITEMLIGHT,DD_OFFSET);
    dd_copysprite(SPR_SCRRT,but[BUT_SCR_TR].x,but[BUT_SCR_TR].y,butsel==BUT_SCR_TR?FX_ITEMBRIGHT:FX_ITEMLIGHT,DD_OFFSET);
    dd_copysprite(SPR_SCRDW,but[BUT_SCR_DW].x,but[BUT_SCR_DW].y,butsel==BUT_SCR_DW?FX_ITEMBRIGHT:FX_ITEMLIGHT,DD_OFFSET);
}

static void display_citem(void) {
    int x,y;
    unsigned int sprite;
    unsigned short c1,c2,c3,shine;
    unsigned char scale,cr,cg,cb,light,sat;
    DDFX fx;

    // trashcan
    if (vk_item) {
        x=but[BUT_JNK].x;
        y=but[BUT_JNK].y;
        dd_copysprite(25,x,y,lcmd==CMD_JUNK_ITEM?DDFX_BRIGHT:DDFX_NLIGHT,DD_CENTER);
    }

    // citem
    if (!csprite) return;

    if (capbut==-1) {
        x=mousex;
        y=mousey;
    } else return;

    bzero(&fx,sizeof(fx));

    sprite=trans_asprite(0,csprite,tick,&scale,&cr,&cg,&cb,&light,&sat,&c1,&c2,&c3,&shine);
    fx.sprite=sprite;
    fx.shine=shine;
    fx.c1=c1;
    fx.c2=c2;
    fx.c3=c3;
    fx.cr=cr;
    fx.cg=cg;
    fx.cb=cb;
    fx.clight=light;
    fx.sat=sat;
    fx.scale=scale;
    fx.sink=0;
    fx.align=DD_CENTER;
    fx.ml=fx.ll=fx.rl=fx.ul=fx.dl=FX_ITEMLIGHT;
    dd_push_clip();
    dd_more_clip(0,0,800,600);
    dd_copysprite_fx(&fx,x,y);
    if ((sprite=additional_sprite(csprite,tick))!=0) {
        fx.sprite=sprite;
        dd_copysprite_fx(&fx,x,y);
    }

    if (cprice) dx_drawtext_gold(x,y+5+12,textcolor,cprice);
    dd_pop_clip();
}

static void display_skill(void) {
    int b,i,x,y,yt,bsx,bex,bsy,barsize;
    char buf[256];

    for (b=BUT_SKL_BEG; b<=BUT_SKL_END; b++) {

        i=skloff+b-BUT_SKL_BEG;

        x=butx(b);
        y=buty(b);
        yt=y-4;
        bsx=x+10;
        bex=x+SKLWIDTH;
        bsy=y+4;

        if (i>=skltab_cnt) continue;

        if (!(but[b].flags&BUTF_NOHIT)) {
            if (i==sklsel) dx_copysprite_emerald(x,y,4,2);
            else dx_copysprite_emerald(x,y,4,1);
        } else if (skltab[i].button) dx_copysprite_emerald(x,y,1,0);

        if (skltab[i].v==STV_EMPTYLINE) continue;

        if (skltab[i].v==STV_JUSTAVALUE) {
            dd_drawtext(bsx,yt,textcolor,DD_LARGE|DD_LEFT,skltab[i].name);
            dd_drawtext_fmt(bex,yt,textcolor,DD_LARGE|DD_RIGHT,"%d",skltab[i].curr);
            continue;
        }

        barsize=skltab[i].barsize;
        if (barsize>0) dd_rect(bsx,bsy,bsx+barsize,bsy+1,bluecolor);
        else if (barsize<0) dd_rect(bsx,bsy,bex+barsize,bsy+1,redcolor);

        switch (skltab[i].v) {
            case V_WEAPON:
            case V_SPEED:
            case V_LIGHT:
            case V_COLD:		sprintf(buf,"%d",skltab[i].curr);
                break;
            case V_ARMOR:		sprintf(buf,"%.2f",skltab[i].curr/20.0);
                break;
            case V_MANA:		sprintf(buf,"%d/%2d/%2d",mana,skltab[i].base,skltab[i].curr);
                break;
            case V_HP:
                if (lifeshield) sprintf(buf,"%d+%d/%2d/%2d",hp,lifeshield,skltab[i].base,skltab[i].curr);
                else sprintf(buf,"%d/%2d/%2d",hp,skltab[i].base,skltab[i].curr);
                break;
            case V_ENDURANCE:	sprintf(buf,"%d/%2d/%2d",endurance,skltab[i].base,skltab[i].curr);
                break;
            default:		if (skltab[i].v>=V_PROFBASE) sprintf(buf,"%d",skltab[i].base);
                else sprintf(buf,"%2d/%2d",skltab[i].base,skltab[i].curr);
                break;
        }

        dd_drawtext(bsx,yt,textcolor,DD_LARGE|DD_LEFT,skltab[i].name);
        dd_drawtext(bex,yt,textcolor,DD_LARGE|DD_RIGHT,buf);
    }
}

static void display_keys(void) {
    int i,x,u;
    char buf[256];
    unsigned short int col;

    for (u=i=0; i<max_keytab; i++) {
        if ((keytab[i].vk_item  && !vk_item) || (!keytab[i].vk_item  && vk_item)) continue;
        if ((keytab[i].vk_char  && !vk_char) || (!keytab[i].vk_char  && vk_char)) continue;
        if ((keytab[i].vk_spell && !vk_spell) || (!keytab[i].vk_spell && vk_spell)) continue;

        if (keytab[i].usetime>now-300) col=bluecolor;
        else col=textcolor;

        x=10+u++*((800-20)/10);

        if (keytab[i].skill==-1) continue;
        if (!value[0][keytab[i].skill]) continue;

        if (keytab[i].userdef) sprintf(buf,"%c/%c %s",keytab[i].keycode,keytab[i].userdef,keytab[i].name);
        else sprintf(buf,"%c %s",keytab[i].keycode,keytab[i].name);

        dd_drawtext(dotx(DOT_BOT)+x,doty(DOT_BOT)-20,col,DD_LEFT|DD_SMALL|DD_FRAME,buf);
    }
}

#define TELE_X	100
#define TELE_Y	40

int teleporter=0;
extern int may_teleport[64+32];

static int tele[64*2]={
    133,229,    //0	Cameron
    -1,-1,      //1
    143,206,    //2	Aston
    370,191,    //3	Tribe of the Isara
    370,179,    //4	Tribe of the Cerasa
    370,167,    //5	Cerasa Maze
    370,155,    //6	Cerasa Tunnels
    370,143,    //7	Zalina Entrance
    370,131,    //8	Tribe of the Zalina
    130,123,    //9	Teufelheim
    -1,-1,      //10
    -1,-1,      //11
    458,108,    //12	Ice 8
    458,96,     //13	Ice 7
    458,84,     //14	Ice 6
    458,72,     //15	Ice 5
    458,60,     //16	Ice 4
    225,123,    //17	Nomad Plains
    -1,-1,      //18
    -1,-1,      //19
    162,180,    // 20 forest
    164,167,    // 21 exkordon
    194,146,    // 22 brannington
    174,115,    // 23 grimroot
    139,149,    // 24 caligar
    205,132,    // 25 arkhata
    0,0,
};

static int mirror_pos[26*2]={
    346,210,
    346,222,
    346,234,
    346,246,
    346,258,
    346,270,
    346,282,
    346,294,

    384,210,
    384,222,
    384,234,
    384,246,
    384,258,
    384,270,
    384,282,
    384,294,

    429,210,
    429,222,
    429,234,
    429,246,
    429,258,
    429,270,
    429,282,
    429,294,

    469,210,
    469,222
};

static int clan_offset=0;

int get_teleport(int x,int y) {
    int n;

    if (!teleporter) return -1;

    // map teleports
    for (n=0; n<64; n++) {
        if (!tele[n*2]) break;
        if (tele[n*2]==-1) continue;
        if (!may_teleport[n]) continue;

        if (abs(tele[n*2]+TELE_X-x)<8 && abs(tele[n*2+1]+TELE_Y-y)<8) return n;

    }

    // clan teleports
    for (n=0; n<8; n++) {
        if (!may_teleport[n+64+clan_offset]) continue;

        if (abs(TELE_X+337-x)<8 && abs(TELE_Y+24+n*12-y)<8) return n+64;
    }
    for (n=0; n<8; n++) {
        if (!may_teleport[n+64+8+clan_offset]) continue;

        if (abs(TELE_X+389-x)<8 && abs(TELE_Y+24+n*12-y)<8) return n+64+8;
    }

    // mirror selector
    for (n=0; n<26; n++) {
        if (abs(mirror_pos[n*2]+TELE_X-x)<8 && abs(mirror_pos[n*2+1]+TELE_Y-y)<8) return n+101;
    }

    if (abs(389+TELE_X-x)<8 && abs(24+8*12+TELE_Y-y)<8) return 1042;

    return -1;
}

void display_teleport(void) {
    int n;

    if (!teleporter) return;

    if (!clan_offset) dd_copysprite(53519,TELE_X+520/2,TELE_Y+320/2,14,0);
    else dd_copysprite(53520,TELE_X+520/2,TELE_Y+320/2,14,0);

    for (n=0; n<64; n++) {
        if (!tele[n*2]) break;
        if (tele[n*2]==-1) continue;

        if (!may_teleport[n]) dx_copysprite_emerald(tele[n*2]+TELE_X,tele[n*2+1]+TELE_Y,2,0);
        else if (telsel==n) dx_copysprite_emerald(tele[n*2]+TELE_X,tele[n*2+1]+TELE_Y,2,2);
        else dx_copysprite_emerald(tele[n*2]+TELE_X,tele[n*2+1]+TELE_Y,2,1);
    }

    for (n=0; n<8; n++) {
        if (!may_teleport[n+64+clan_offset]) dx_copysprite_emerald(337+TELE_X,24+n*12+TELE_Y,3,0);
        else if (telsel==n+64+clan_offset) dx_copysprite_emerald(337+TELE_X,24+n*12+TELE_Y,3,2);
        else dx_copysprite_emerald(337+TELE_X,24+n*12+TELE_Y,3,1);
    }
    for (n=0; n<8; n++) {
        if (8+clan_offset+n==31) continue;
        if (!may_teleport[n+64+8+clan_offset]) dx_copysprite_emerald(389+TELE_X,24+n*12+TELE_Y,3,0);
        else if (telsel==n+64+8+clan_offset) dx_copysprite_emerald(389+TELE_X,24+n*12+TELE_Y,3,2);
        else dx_copysprite_emerald(389+TELE_X,24+n*12+TELE_Y,3,1);
    }

    for (n=0; n<26; n++) {
        if (telsel==n+101) dx_copysprite_emerald(mirror_pos[n*2]+TELE_X,mirror_pos[n*2+1]+TELE_Y,1,2);
        else if (newmirror==n+1) dx_copysprite_emerald(mirror_pos[n*2]+TELE_X,mirror_pos[n*2+1]+TELE_Y,1,1);
        else dx_copysprite_emerald(mirror_pos[n*2]+TELE_X,mirror_pos[n*2+1]+TELE_Y,1,0);
    }

    if (telsel==1042) dx_copysprite_emerald(389+TELE_X,24+8*12+TELE_Y,2,2);
    else dx_copysprite_emerald(389+TELE_X,24+8*12+TELE_Y,2,1);

}

char tutor_text[1024]={""};
int show_tutor=0;

void display_tutor(void) {
    int x,y,n,mx=dotx(DOT_BOT)+626,my=doty(DOT_BOT)-370+416-80;
    char *ptr,buf[80];

    if (!show_tutor) return;

    // TODO: test me
    dd_rect(dotx(DOT_BOT)+220,doty(DOT_BOT)-370+350-100,dotx(DOT_BOT)+630,doty(DOT_BOT)-370+426-100,IRGB(24,22,16));
    dd_line(dotx(DOT_BOT)+220,doty(DOT_BOT)-370+350-100,dotx(DOT_BOT)+630,doty(DOT_BOT)-370+350-100,IRGB(12,10,4));
    dd_line(dotx(DOT_BOT)+630,doty(DOT_BOT)-370+350-100,dotx(DOT_BOT)+630,doty(DOT_BOT)-370+426-100,IRGB(12,10,4));
    dd_line(dotx(DOT_BOT)+220,doty(DOT_BOT)-370+426-100,dotx(DOT_BOT)+360,doty(DOT_BOT)-370+426-100,IRGB(12,10,4));
    dd_line(dotx(DOT_BOT)+220,doty(DOT_BOT)-370+350-100,dotx(DOT_BOT)+220,doty(DOT_BOT)-370+426-100,IRGB(12,10,4));

    x=dotx(DOT_BOT)+224; y=doty(DOT_BOT)-370+354-20; ptr=tutor_text;
    while (*ptr) {
        while (*ptr==' ') ptr++;
        while (*ptr=='$') {
            ptr++;
            x=dotx(DOT_BOT)+224;
            y+=10;
            if (y>=my) break;
        }
        while (*ptr==' ') ptr++;
        n=0;
        while (*ptr && *ptr!=' ' && *ptr!='$' && n<79) buf[n++]=*ptr++;
        buf[n]=0;
        if (x+dd_textlength(DD_LEFT|DD_LARGE,buf)>=mx) {
            x=dotx(DOT_BOT)+224;
            y+=10;
            if (y>=my) break;
        }
        x=dd_drawtext(x,y,IRGB(12,10,4),DD_LEFT|DD_LARGE,buf)+3;
    }
}

#define COLO_X	(400-120/2)
#define COLO_Y	(270-120/2)

int show_color=0,show_cur=0;
int show_color_c[3]={1,1,1};
int show_cx=0;

void display_color(void) {
    int csprite,scale,cr,cg,cb,light,sat,c1,c2,c3,shine;
    static int col_anim=4,col_step=0,col_dir=0;
    DDFX fx;

    if (!show_color) return;

    dd_copysprite(51082,COLO_X,COLO_Y,14,0);

    if (show_cur==0) dx_copysprite_emerald(COLO_X-38,COLO_Y+40,2,2);
    else dx_copysprite_emerald(COLO_X-38,COLO_Y+40,2,1);
    if (show_cur==1) dx_copysprite_emerald(COLO_X-38+12,COLO_Y+40,2,2);
    else dx_copysprite_emerald(COLO_X-38+12,COLO_Y+40,2,1);
    if (show_cur==2) dx_copysprite_emerald(COLO_X-38+24,COLO_Y+40,2,2);
    else dx_copysprite_emerald(COLO_X-38+24,COLO_Y+40,2,1);

    dd_copysprite(51083,COLO_X-55,COLO_Y-50+64-IGET_R(show_color_c[show_cur])*2,14,0);
    dd_copysprite(51083,COLO_X-55+20,COLO_Y-50+64-IGET_G(show_color_c[show_cur])*2,14,0);
    dd_copysprite(51083,COLO_X-55+40,COLO_Y-50+64-IGET_B(show_color_c[show_cur])*2,14,0);

    bzero(&fx,sizeof(fx));

    csprite=trans_charno(map[MAPDX*MAPDY/2].csprite,&scale,&cr,&cg,&cb,&light,&sat,&c1,&c2,&c3,&shine,tick);

    fx.sprite=get_player_sprite(csprite,col_dir,col_anim,col_step,16,tick);
    col_step++;
    if (col_step==16) {
        col_step=0;
        col_anim++;
        if (col_anim>6) {
            col_anim=4;
            col_dir+=2;
            if (col_dir>7) col_dir=0;
        }
    }
    fx.scale=scale;
    fx.shine=shine;
    fx.cr=cr;
    fx.cg=cg;
    fx.cb=cb;
    fx.clight=light;
    fx.sat=sat;

    fx.c1=show_color_c[0];
    fx.c2=show_color_c[1];
    fx.c3=show_color_c[2];

    fx.sink=0;
    fx.align=DD_OFFSET;
    fx.ml=fx.ll=fx.rl=fx.ul=fx.dl=FX_ITEMLIGHT;
    dd_copysprite_fx(&fx,COLO_X+30,COLO_Y);
}

int get_color(int x,int y) {
    if (!show_color) return -1;

    if (abs(x-COLO_X+38)<4 && abs(y-COLO_Y-39)<4) return 1;
    if (abs(x-COLO_X+26)<4 && abs(y-COLO_Y-39)<4) return 2;
    if (abs(x-COLO_X+14)<4 && abs(y-COLO_Y-39)<4) return 3;

    if (abs(x-COLO_X+50)<11 && abs(y-COLO_Y+18)<34) { show_cx=64-(y-COLO_Y+50); return 4; }
    if (abs(x-COLO_X+30)<11 && abs(y-COLO_Y+18)<34) { show_cx=64-(y-COLO_Y+50); return 5; }
    if (abs(x-COLO_X+10)<11 && abs(y-COLO_Y+18)<34) { show_cx=64-(y-COLO_Y+50); return 6; }

    if (abs(x-COLO_X-43)<10 && abs(y-COLO_Y-39)<5) return 7;
    if (abs(x-COLO_X-19)<10 && abs(y-COLO_Y-39)<5) return 8;

    //addline("x=%d, y=%d",x-COLO_X,y-COLO_Y);

    if (x-COLO_X<-60 || x-COLO_X>60 || y-COLO_Y<-60 || y-COLO_Y>60) return -1;

    return 0;
}

void cmd_color(int nr) {
    int val;
    char buf[80];

    switch (nr) {
        case 1:		show_cur=0; break;
        case 2:		show_cur=1; break;
        case 3:		show_cur=2; break;
        case 4:		val=max(min(31,show_cx/2),1);
                    show_color_c[show_cur]=IRGB(
                                   val,
                                   IGET_G(show_color_c[show_cur]),
                                   IGET_B(show_color_c[show_cur]));
                    break;
        case 5:		val=max(min(31,show_cx/2),1);
                    show_color_c[show_cur]=IRGB(
                                   IGET_R(show_color_c[show_cur]),
                                   val,
                                   IGET_B(show_color_c[show_cur]));
                    break;
        case 6:		val=max(min(31,show_cx/2),1);
                    show_color_c[show_cur]=IRGB(
                                   IGET_R(show_color_c[show_cur]),
                                   IGET_G(show_color_c[show_cur]),
                                   val);
                    break;
        case 7:		show_color=0; break;
        case 8:		sprintf(buf,"/col1 %d %d %d",IGET_R(show_color_c[0]),IGET_G(show_color_c[0]),IGET_B(show_color_c[0]));
                    cmd_text(buf);
                    sprintf(buf,"/col2 %d %d %d",IGET_R(show_color_c[1]),IGET_G(show_color_c[1]),IGET_B(show_color_c[1]));
                    cmd_text(buf);
                    sprintf(buf,"/col3 %d %d %d",IGET_R(show_color_c[2]),IGET_G(show_color_c[2]),IGET_B(show_color_c[2]));
                    cmd_text(buf);
                    break;
    }
}

// date stuff
#define DAYLEN		(60*60*2)
#define HOURLEN		(DAYLEN/24)
#define MINLEN		(HOURLEN/60)

void trans_date(int t,int *phour,int *pmin) {
    if (pmin) *pmin=(t/MINLEN)%60;
    if (phour) *phour=(t/HOURLEN)%24;
}

static char time_text[120];

static void display_screen(void) {
    int h,m;
    int h1,h2,m1,m2;
    static int rh1=0,rh2=0,rm1=0,rm2=0;
    extern int realtime;

    dd_copysprite(opt_sprite(999),dotx(DOT_TOP),doty(DOT_TOP),DDFX_NLIGHT,DD_NORMAL);

    trans_date(realtime,&h,&m);

    h1=h/10*3;
    h2=h%10*3;
    m1=m/10*3;
    m2=m%10*3;

    if (h1!=rh1) rh1++;
    if (rh1==30) rh1=0;

    if (h2!=rh2) rh2++;
    if (rh2==30) rh2=0;

    if (m1!=rm1) rm1++;
    if (rm1==18) rm1=0;

    if (m2!=rm2) rm2++;
    if (rm2==30) rm2=0;

    dd_copysprite(200+rh1,dotx(DOT_TOP)+730+0*10-2,doty(DOT_TOP)+5+3,DDFX_NLIGHT,DD_NORMAL);
    dd_copysprite(200+rh2,dotx(DOT_TOP)+730+1*10-2,doty(DOT_TOP)+5+3,DDFX_NLIGHT,DD_NORMAL);
    dd_copysprite(200+rm1,dotx(DOT_TOP)+734+2*10-2,doty(DOT_TOP)+5+3,DDFX_NLIGHT,DD_NORMAL);
    dd_copysprite(200+rm2,dotx(DOT_TOP)+734+3*10-2,doty(DOT_TOP)+5+3,DDFX_NLIGHT,DD_NORMAL);

    sprintf(time_text,"%02d:%02d Astonia Standard Time",h,m);

    dd_copysprite(opt_sprite(998),dotx(DOT_BOT),doty(DOT_BOT),DDFX_NLIGHT,DD_NORMAL);
}


static void display_text(void) {
    dd_display_text();

    if (dd_scantext(mousex,mousey,hitsel));
    else hitsel[0]=0;
    display_cmd();
}

static void display_gold(void) {
    int x,y;

    x=but[BUT_GLD].x;
    y=but[BUT_GLD].y;

    dd_copysprite(SPR_GOLD_BEG+7,x,y,lcmd==CMD_TAKE_GOLD || lcmd==CMD_DROP_GOLD?DDFX_BRIGHT:DDFX_NLIGHT,DD_CENTER);

    if (capbut==BUT_GLD) {
        dx_drawtext_gold(x,y,textcolor,takegold);
        dx_drawtext_gold(x,y+12,textcolor,gold-takegold);
    } else {
        dx_drawtext_gold(x,y+12,textcolor,gold);
    }
}

static void display_mode(void) {
    static char *speedtext[3]={"NORMAL","FAST","STEALTH"};
    int sel,seltxt,lg;
    unsigned short int col;

    // walk
    if (butsel>=BUT_MOD_WALK0 && butsel<=BUT_MOD_WALK2) { seltxt=butsel-BUT_MOD_WALK0; lg=2; col=seltxt==pspeed?lightbluecolor:bluecolor; } else { seltxt=pspeed; lg=1; col=lightbluecolor; }
    sel=pspeed;

    dx_copysprite_emerald(but[BUT_MOD_WALK0].x,but[BUT_MOD_WALK0].y,4,(sel==0?lg:0));
    dx_copysprite_emerald(but[BUT_MOD_WALK1].x,but[BUT_MOD_WALK1].y,4,(sel==1?lg:0));
    dx_copysprite_emerald(but[BUT_MOD_WALK2].x,but[BUT_MOD_WALK2].y,4,(sel==2?lg:0));

    dd_drawtext(but[BUT_MOD_WALK0].x,but[BUT_MOD_WALK0].y+7,bluecolor,DD_SMALL|DD_FRAME|DD_CENTER,"F6");
    dd_drawtext(but[BUT_MOD_WALK1].x,but[BUT_MOD_WALK1].y+7,bluecolor,DD_SMALL|DD_FRAME|DD_CENTER,"F5");
    dd_drawtext(but[BUT_MOD_WALK2].x,but[BUT_MOD_WALK2].y+7,bluecolor,DD_SMALL|DD_FRAME|DD_CENTER,"F7");

    if (*speedtext[sel]) dd_drawtext(but[BUT_MOD_WALK0].x,but[BUT_MOD_WALK0].y-13,col,DD_SMALL|DD_CENTER|DD_FRAME,speedtext[seltxt]);
}

static char bless_text[120];
static char freeze_text[120];
static char potion_text[120];
static char rage_text[120];
static char level_text[120];
static char rank_text[120];

static void display_mouseover(void) {
    if (mousey>=doty(DOT_BOT)-370+496-60 && mousey<=doty(DOT_BOT)-370+551-60) {
        if (mousex>=dotx(DOT_BOT)+207 && mousex<=dotx(DOT_BOT)+214) dd_drawtext(mousex,mousey-16,0xffff,DD_BIG|DD_FRAME|DD_CENTER,rage_text);
        if (mousex>=dotx(DOT_BOT)+187 && mousex<=dotx(DOT_BOT)+194) dd_drawtext(mousex,mousey-16,0xffff,DD_BIG|DD_FRAME|DD_CENTER,freeze_text);
        if (mousex>=dotx(DOT_BOT)+177 && mousex<=dotx(DOT_BOT)+184) dd_drawtext(mousex,mousey-16,0xffff,DD_BIG|DD_FRAME|DD_CENTER,potion_text);
    }

    if (mousex>=dotx(DOT_BOT)+25 && mousex<=dotx(DOT_BOT)+135) {
        if (mousey>=doty(DOT_TOP)+5 && mousey<=doty(DOT_TOP)+13) dd_drawtext(mousex+16,mousey-4,0xffff,DD_BIG|DD_FRAME,level_text);
        if (mousey>=doty(DOT_TOP)+22 && mousey<=doty(DOT_TOP)+30) dd_drawtext(mousex+16,mousey-4,0xffff,DD_BIG|DD_FRAME,rank_text);
    }

    if (mousex>=dotx(DOT_TOP)+728 && mousex<=dotx(DOT_TOP)+772 && mousey>=doty(DOT_TOP)+7 && mousey<=doty(DOT_TOP)+17)
        dd_drawtext(mousex-16,mousey-4,0xffff,DD_BIG|DD_FRAME|DD_RIGHT,time_text);
}

static void display_selfspells(void) {
    int n,nr,cn,step;

    cn=map[mapmn(MAPDX/2,MAPDY/2)].cn;
    if (!cn) return;

    sprintf(bless_text,"Bless: Not active");
    sprintf(freeze_text,"Freeze: Not active");
    sprintf(potion_text,"Potion: Not active");

    for (n=0; n<4; n++) {
        nr=find_cn_ceffect(cn,n);
        if (nr==-1) continue;

        switch (ceffect[nr].generic.type) {
            case 9:
                step=50-50*(ceffect[nr].bless.stop-tick)/(ceffect[nr].bless.stop-ceffect[nr].bless.start);
                dd_push_clip();
                dd_more_clip(0,0,800,doty(DOT_BOT)+119);
                if (ceffect[nr].bless.stop-tick<24*30 && (tick&4)) dd_copysprite(997,dotx(DOT_BOT)+179+2*10,doty(DOT_BOT)+68+step,DDFX_BRIGHT,DD_NORMAL);
                else dd_copysprite(997,dotx(DOT_BOT)+179+2*10,doty(DOT_BOT)+68+step,DDFX_NLIGHT,DD_NORMAL);
                dd_pop_clip();
                sprintf(bless_text,"Bless: %ds to go",(ceffect[nr].bless.stop-tick)/24);
                break;
            case 11:
                step=50-50*(ceffect[nr].freeze.stop-tick)/(ceffect[nr].freeze.stop-ceffect[nr].freeze.start);
                dd_push_clip();
                dd_more_clip(0,0,800,doty(DOT_BOT)+119);
                dd_copysprite(997,dotx(DOT_BOT)+179+1*10,doty(DOT_BOT)+68+step,DDFX_NLIGHT,DD_NORMAL);
                dd_pop_clip();
                sprintf(freeze_text,"Freeze: %ds to go",(ceffect[nr].freeze.stop-tick)/24);
                break;

            case 14:
                step=50-50*(ceffect[nr].potion.stop-tick)/(ceffect[nr].potion.stop-ceffect[nr].potion.start);
                dd_push_clip();
                dd_more_clip(0,0,800,doty(DOT_BOT)+119);
                if (step>=40 && (tick&4)) dd_copysprite(997,dotx(DOT_BOT)+179+0*10,doty(DOT_BOT)+68+step,DDFX_BRIGHT,DD_NORMAL);
                else dd_copysprite(997,dotx(DOT_BOT)+179+0*10,doty(DOT_BOT)+68+step,DDFX_NLIGHT,DD_NORMAL);
                dd_pop_clip();
                sprintf(potion_text,"Potion: %ds to go",(ceffect[nr].potion.stop-tick)/24);
                break;
        }
    }
}

static void display_exp(void) {
    int level,step,total,expe,cn,clevel,nlevel;
    static int last_exp=0,exp_ticker=0;

    sprintf(level_text,"Level: unknown");

    cn=map[MAPDX*MAPDY/2].cn;
    level=player[cn].level;

    expe=experience;
    clevel=exp2level(expe);
    nlevel=level+1;

    step=level2exp(nlevel)-expe;
    total=level2exp(nlevel)-level2exp(clevel);
    if (step>total) step=total; // ugh. fix for level 1 with 0 exp

    if (total) {
        if (last_exp!=expe) {
            exp_ticker=3;
            last_exp=expe;
        }

        dd_push_clip();
        dd_more_clip(0,0,dotx(DOT_TOP)+31+100-100*step/total,doty(DOT_TOP)+8+7);
        dd_copysprite(996,dotx(DOT_TOP)+31,doty(DOT_TOP)+7,exp_ticker?DDFX_BRIGHT:DDFX_NLIGHT,DD_NORMAL);
        dd_pop_clip();

        if (exp_ticker) exp_ticker--;

        sprintf(level_text,"Level: From %d to %d",clevel,nlevel);
    }
}

static char *rankname[]={
    "nobody",               //0
    "Private",              //1
    "Private First Class",  //2
    "Lance Corporal",       //3
    "Corporal",             //4
    "Sergeant",             //5     lvl 30
    "Staff Sergeant",       //6
    "Master Sergeant",      //7
    "First Sergeant",       //8     lvl 45
    "Sergeant Major",       //9
    "Second Lieutenant",    //10    lvl 55
    "First Lieutenant",     //11
    "Captain",              //12
    "Major",                //13
    "Lieutenant Colonel",   //14
    "Colonel",              //15
    "Brigadier General",    //16
    "Major General",        //17
    "Lieutenant General",   //18
    "General",              //19
    "Field Marshal",        //20    lvl 105
    "Knight of Astonia",    //21
    "Baron of Astonia",     //22
    "Earl of Astonia",      //23
    "Warlord of Astonia"    //24    lvl 125
};

static int mil_rank(int exp) {
    int n;

    for (n=1; n<50; n++) {
        if (exp<n*n*n) return n-1;
    }
    return 99;
}

static void display_military(void) {
    int step,total,rank,cost1,cost2;
    extern int mil_exp;

    sprintf(rank_text,"Rank: none or unknown");

    rank=mil_rank(mil_exp);
    cost1=rank*rank*rank;
    cost2=(rank+1)*(rank+1)*(rank+1);

    total=cost2-cost1;
    step=mil_exp-cost1;
    if (step>total) step=total;

    if (mil_exp && total) {
        if (rank<24) {
            dd_push_clip();
            dd_more_clip(0,0,dotx(DOT_TOP)+31+100*step/total,doty(DOT_TOP)+8+24);
            dd_copysprite(993,dotx(DOT_TOP)+31,doty(DOT_TOP)+24,DDFX_NLIGHT,DD_NORMAL);
            dd_pop_clip();

            sprintf(rank_text,"Rank: '%s' to '%s'",rankname[rank],rankname[rank+1]);
        } else sprintf(rank_text,"Rank: Warlord of Astonia");
    }
}

static void display_rage(void) {
    int step;

    sprintf(rage_text,"Rage: Not active");

    if (!value[0][V_RAGE] || !rage) return;

    step=50-50*rage/value[0][V_RAGE];
    dd_push_clip();
    dd_more_clip(0,0,800,doty(DOT_BOT)+119);
    dd_copysprite(997,dotx(DOT_BOT)+179+3*10,doty(DOT_BOT)+68+step,DDFX_NLIGHT,DD_NORMAL);
    dd_pop_clip();

    sprintf(rage_text,"Rage: %d%%",100*rage/value[0][V_RAGE]);
}

void display_game_special(void) {
    extern int display_gfx,display_time;
    int dx;

    if (!display_gfx) return;

    switch (display_gfx) {
        // TODO: these are the ugly tutorial arrows
        // since we want to re-write the input parts of the
        // GUI there's no point in updating them now
        // so: Make a new tutorial. Eventually.
        case 1:		dd_copysprite(50473,343,540,14,0); break;
        case 2:		dd_copysprite(50473,423,167,14,0); break;
        case 3:		dx=(tick-display_time)*450/120;
            if (dx<450) dd_copysprite(50475,175+dx,60,14,0);
            break;
        case 4:		dd_copysprite(50475,218,60,14,0); break;
        case 5:		dd_copysprite(50475,257,60,14,0); break;
        case 6:		dd_copysprite(50475,23,45,14,0); break;
        case 7:		dd_copysprite(50475,75,47,14,0); break;
        case 8:		dd_copysprite(50475,763,62,14,0); break;

        case 9:		dx=(tick-display_time)*150/120;
            if (dx<150) dd_copysprite(50474,188,447+dx,14,0);
            break;

        case 10:	dd_copysprite(50474,205,459,14,0); break;

        case 11:	dx=(tick-display_time)*150/120;
            if (dx<150) dd_copysprite(50476,200,440+dx,14,0);
            break;

        case 12:	dx=(tick-display_time)*150/120;
            if (dx<150) dd_copysprite(50476,618,445+dx,14,0);
            break;

        case 13:	dd_copysprite(50476,625,456,14,0); break;
        case 14:	dd_copysprite(50476,700,456,14,0); break;
        case 15:	dd_copysprite(50476,741,456,14,0); break;

        case 16:	dd_copysprite(50476,353,203,14,0); break;

        case 17:	dd_copysprite(50473,722,382,14,0); dd_copysprite(50475,257,60,14,0); break;

        // TODO: this is used to display the maps in earth underground
        // needs testing.
        default:	dd_copysprite(display_gfx,550,210,14,0); break;
    }
}

char perf_text[256];
static void set_cmd_states(void);

static void display(void) {
    extern int socktimeout,kicked_out;
    extern int memptrs[MAX_MEM];
    extern int memsize[MAX_MEM];
    extern int memused;
    extern int memptrused;
    extern long long sdl_time_make,sdl_time_tex,sdl_time_tex_main,sdl_time_text,sdl_time_blit;
    int t;
    long long start=SDL_GetTicks64();
    static int top_opening=0,top_closing=1,top_open=0;

#if 0
    // Performance for stuff happening during the actual tick only.
    // So zero them now after preload is done.
    sdl_time_make=0;
    sdl_time_tex=0;
    sdl_time_text=0;
    sdl_time_blit=0;
#endif

    if (mousey<23 && !top_opening && !top_open) { top_opening=1; top_closing=0; }
    if (mousey>60 && !top_closing && top_open) { top_closing=1; top_opening=0; }

    if (top_opening) {
        gui_topoff=-38+top_opening; top_opening+=6;
        if (top_opening>=38) { top_open=1; top_opening=0; }
    }

    if (top_open) gui_topoff=0;

    if (top_closing) {
        gui_topoff=-top_closing; top_closing+=6;
        if (top_closing>=38) { top_open=0; top_closing=0; }
    }


#if 0
    if (mousey<40) gui_topoff=0;
    else if (mousey<78) gui_topoff=-mousey+40;
    else gui_topoff=-38;
#endif

    set_cmd_states();

    if (sockstate<4 && ((t=time(NULL)-socktimeout)>10 || !originx)) {
        dd_rect(0,0,800,540,blackcolor);
        display_screen();
        display_text();
        if ((now/1000)&1) dd_drawtext(800/2,540/2-60,redcolor,DD_CENTER|DD_LARGE,"not connected");
        dd_copysprite(60,800/2,(540-240)/2,DDFX_NLIGHT,DD_CENTER);
        dd_copysprite(1,mousex,mousey+5,DDFX_BRIGHT,DD_CENTER);
        if (!kicked_out) {
            dd_drawtext_fmt(800/2,540/2-40,textcolor,DD_SMALL|DD_CENTER|DD_FRAME,"Trying to establish connection. %d seconds...",t);
            if (t>15) {
                dd_drawtext_fmt(800/2,540/2-20,textcolor,DD_LARGE|DD_CENTER|DD_FRAME,"If you have connection problems, please try a different connection in the lower left of the client startup screen.");
                dd_drawtext_fmt(800/2,540/2-0,textcolor,DD_LARGE|DD_CENTER|DD_FRAME,"Additional information can be found at www.astonia.com.");
            }
        }
        return;
    }

    dd_push_clip();
    dd_more_clip(dotx(DOT_MTL),doty(DOT_MTL),dotx(DOT_MBR),doty(DOT_MBR));
    display_game();
    dd_pop_clip();

    display_screen();

    display_keys();
    if (show_look) display_look();
    display_wear();
    display_inventory();
    if (con_cnt) display_container();
    else display_skill();
    display_scrollbars();
    display_text();
    display_gold();
    display_mode();
    display_selfspells();
    display_exp();
    display_military();
    display_teleport();
    display_color();
    display_rage();
    display_game_special();
    display_tutor();
    display_citem();

    int duration=SDL_GetTicks64()-start;

    if (display_vc) {
        extern long long texc_miss,texc_pre; //mem_tex,
        extern uint64_t sdl_backgnd_wait,sdl_backgnd_work,sdl_time_preload,sdl_time_load,gui_time_network;
        extern uint64_t sdl_time_pre1,sdl_time_pre2,sdl_time_pre3,sdl_time_mutex,sdl_time_alloc,sdl_time_make_main;
        extern int x_offset,y_offset; //pre_2,pre_in,pre_3;
        //static int dur=0,make=0,tex=0,text=0,blit=0,stay=0;
        static int size;
        static unsigned char dur_graph[100],size1_graph[100],size2_graph[100]; //,size3_graph[100]; //,size_graph[100];load_graph[100],
        static unsigned char pre1_graph[100]; //,pre2_graph[100]; //,pre3_graph[100];
        int px=800-110,py=45+gui_topoff-10;
        PROCESS_MEMORY_COUNTERS mi;

        GetProcessMemoryInfo(GetCurrentProcess(),&mi,sizeof(mi));

        //dd_drawtext_fmt(px,py+=10,0xffff,DD_SMALL|DD_LEFT|DD_FRAME|DD_NOCACHE,"skip %3.0f%%",100.0*skip/tota);
        //dd_drawtext_fmt(px,py+=10,0xffff,DD_SMALL|DD_LEFT|DD_FRAME|DD_NOCACHE,"idle %3.0f%%",100.0*idle/tota);
        //dd_drawtext_fmt(px,py+=10,IRGB(8,31,8),DD_LEFT|DD_FRAME|DD_NOCACHE,"Tex: %5.2f MB",mem_tex/(1024.0*1024.0));
        dd_drawtext_fmt(px,py+=10,IRGB(8,31,8),DD_LEFT|DD_FRAME|DD_NOCACHE,"Mem: %5.2f MB",mi.WorkingSetSize/(1024.0*1024.0));

#if 0
        if (pre_in>=pre_3) size=pre_in-pre_3;
        else size=16384+pre_in-pre_3;

        dd_drawtext_fmt(px,py+=10,0xffff,DD_SMALL|DD_LEFT|DD_FRAME|DD_NOCACHE,"PreC %d",size);
#endif
        //dd_drawtext_fmt(px,py+=10,0xffff,DD_SMALL|DD_LEFT|DD_FRAME|DD_NOCACHE,"Miss %lld",texc_miss);
        //dd_drawtext_fmt(px,py+=10,0xffff,DD_SMALL|DD_LEFT|DD_FRAME|DD_NOCACHE,"Prel %lld",texc_pre);

        py+=10;

        size=duration+gui_time_network;
        dd_drawtext(px,py+=10,IRGB(8,31,8),DD_LEFT|DD_FRAME,"Render");
        sdl_bargraph_add(sizeof(dur_graph),dur_graph,size<42?size:42);
        sdl_bargraph(px,py+=40,sizeof(dur_graph),dur_graph,x_offset,y_offset);
#if 0
        size=gui_time_network;
        dd_drawtext_fmt(px,py+=10,IRGB(8,31,8),DD_LEFT|DD_FRAME,"Network");
        sdl_bargraph_add(sizeof(pre2_graph),pre2_graph,size<42?size:42);
        sdl_bargraph(px,py+=40,sizeof(pre2_graph),pre2_graph,x_offset,y_offset);

        size=sdl_time_pre1;
        dd_drawtext(px,py+=10,IRGB(8,31,8),DD_LEFT|DD_FRAME,"Alloc");
        sdl_bargraph_add(sizeof(size1_graph),size3_graph,size<42?size:42);
        sdl_bargraph(px,py+=40,sizeof(size1_graph),size3_graph,x_offset,y_offset);
#endif

#if 0
        size=gui_time_misc;
        dd_drawtext_fmt(px,py+=10,IRGB(8,31,8),DD_SMALL|DD_LEFT|DD_FRAME,"misc");
        sdl_bargraph_add(sizeof(pre2_graph),pre2_graph,size<42?size:42);
        sdl_bargraph(px,py+=40,sizeof(pre2_graph),pre2_graph,x_offset,y_offset);

        size=sdl_time_alloc;
        dd_drawtext(px,py+=10,IRGB(8,31,8),DD_LEFT|DD_FRAME,"Alloc");
        sdl_bargraph_add(sizeof(size1_graph),load_graph,size<42?size:42);
        sdl_bargraph(px,py+=40,sizeof(size1_graph),load_graph,x_offset,y_offset);
#endif

        size=sdl_time_pre1+sdl_time_pre3;
        dd_drawtext(px,py+=10,IRGB(8,31,8),DD_LEFT|DD_FRAME,"Pre-Main");
        sdl_bargraph_add(sizeof(size1_graph),size2_graph,size<42?size:42);
        sdl_bargraph(px,py+=40,sizeof(size1_graph),size2_graph,x_offset,y_offset);
#if 0

#endif
        if (sdl_multi) {
            size=sdl_backgnd_work/sdl_multi;
            dd_drawtext_fmt(px,py+=10,IRGB(8,31,8),DD_LEFT|DD_FRAME,"Pre-Back (%d)",sdl_multi);
        } else {
            size=sdl_time_pre2;
            dd_drawtext_fmt(px,py+=10,IRGB(8,31,8),DD_LEFT|DD_FRAME,"Make",sdl_multi);
        }
        sdl_bargraph_add(sizeof(pre1_graph),pre1_graph,size<42?size:42);
        sdl_bargraph(px,py+=40,sizeof(pre1_graph),pre1_graph,x_offset,y_offset);
#if 0
            dd_drawtext_fmt(px,py+=10,IRGB(8,31,8),DD_SMALL|DD_LEFT|DD_FRAME,"Mutex");
            sdl_bargraph_add(sizeof(pre2_graph),pre2_graph,sdl_time_mutex/sdl_multi<42?sdl_time_mutex/sdl_multi:42);
            sdl_bargraph(px,py+=40,sizeof(pre2_graph),pre2_graph,x_offset,y_offset);
#endif
#if 0
        dd_drawtext(px,py+=10,IRGB(8,31,8),DD_SMALL|DD_LEFT|DD_FRAME,"Pre-Queue Tot");
        sdl_bargraph_add(sizeof(size_graph),size_graph,size/4<42?size/4:42);
        sdl_bargraph(px,py+=40,sizeof(size_graph),size_graph,x_offset,y_offset);

        dd_drawtext(px,py+=10,IRGB(8,31,8),DD_SMALL|DD_LEFT|DD_FRAME,"Pre2");
        sdl_bargraph_add(sizeof(pre2_graph),pre2_graph,sdl_time_pre2<42?sdl_time_pre2:42);
        sdl_bargraph(px,py+=40,sizeof(pre2_graph),pre2_graph,x_offset,y_offset);

        dd_drawtext(px,py+=10,IRGB(8,31,8),DD_SMALL|DD_LEFT|DD_FRAME,"Texture");
        sdl_bargraph_add(sizeof(pre3_graph),pre3_graph,sdl_time_pre3<42?sdl_time_pre3:42);
        sdl_bargraph(px,py+=40,sizeof(pre3_graph),pre3_graph,x_offset,y_offset);

#endif
#if 0
        if (pre_2>=pre_3) size=pre_2-pre_3;
        else size=16384+pre_2-pre_3;

        dd_drawtext(px,py+=10,IRGB(8,31,8),DD_SMALL|DD_LEFT|DD_FRAME,"Size Tex");
        sdl_bargraph_add(sizeof(size3_graph),size3_graph,size/4<42?size/4:42);
        sdl_bargraph(px,py+=40,sizeof(size3_graph),size3_graph,x_offset,y_offset);


        if (duration>10 && (!stay || duration>dur)) {
            dur=duration;
            make=sdl_time_make;
            tex=sdl_time_tex;
            text=sdl_time_text;
            blit=sdl_time_blit;
            stay=24*6;
        }

        if (stay>0) {
            stay--;
            dd_drawtext_fmt(px,py+=20,0xffff,DD_SMALL|DD_LEFT|DD_FRAME|DD_NOCACHE,"Dur %dms (%.0f%%)",dur,100.0*(make+tex+text+blit)/dur);
            dd_drawtext_fmt(px,py+=10,0xffff,DD_SMALL|DD_LEFT|DD_FRAME|DD_NOCACHE,"Make %dms (%.0f%%)",make,100.0*make/dur);
            dd_drawtext_fmt(px,py+=10,0xffff,DD_SMALL|DD_LEFT|DD_FRAME|DD_NOCACHE,"Tex %dms (%.0f%%)",tex,100.0*tex/dur);
            dd_drawtext_fmt(px,py+=10,0xffff,DD_SMALL|DD_LEFT|DD_FRAME|DD_NOCACHE,"Text %dms (%.0f%%)",text,100.0*text/dur);
            dd_drawtext_fmt(px,py+=10,0xffff,DD_SMALL|DD_LEFT|DD_FRAME|DD_NOCACHE,"Blit %dms (%.0f%%)",blit,100.0*blit/dur);
        }
#endif
        sdl_time_preload=0;
        sdl_time_make=0;
        sdl_time_tex=0;
        sdl_time_text=0;
        sdl_time_blit=0;
        sdl_backgnd_work=0;
        sdl_backgnd_wait=0;
        sdl_time_load=0;
        sdl_time_pre1=0;
        sdl_time_pre2=0;
        sdl_time_pre3=0;
        sdl_time_mutex=0;
        sdl_time_tex_main=0;
        gui_time_misc=0;
        sdl_time_alloc=0;
        texc_miss=0;
        texc_pre=0;
        sdl_time_make_main=0;
        gui_time_network=0;

    } //else dd_drawtext_fmt(650,15,0xffff,DD_SMALL|DD_FRAME,"Mirror %d",mirror);

    sprintf(perf_text,"mem usage=%.2f/%.2fMB, %.2f/%.2fKBlocks",
            memsize[0]/1024.0/1024.0,memused/1024.0/1024.0,
            memptrs[0]/1024.0,memptrused/1024.0);

    display_mouseover();
}

// cmd

static void set_cmd_cursor(int cmd) {
    int cursor;

    // cursor
    switch (cmd) {
        case CMD_MAP_MOVE:      cursor=SDL_CUR_c_only; break;
        case CMD_MAP_DROP:      cursor=SDL_CUR_c_drop; break;

        case CMD_ITM_TAKE:      cursor=SDL_CUR_c_take; break;
        case CMD_ITM_USE:       cursor=SDL_CUR_c_use; break;
        case CMD_ITM_USE_WITH:  cursor=SDL_CUR_c_usewith; break;

        case CMD_CHR_ATTACK:    cursor=SDL_CUR_c_attack; break;
        case CMD_CHR_GIVE:      cursor=SDL_CUR_c_give; break;

        case CMD_INV_USE:       cursor=SDL_CUR_c_use; break;
        case CMD_INV_USE_WITH:  cursor=SDL_CUR_c_usewith; break;
        case CMD_INV_TAKE:      cursor=SDL_CUR_c_take; break;
        case CMD_INV_SWAP:      cursor=SDL_CUR_c_swap; break;
        case CMD_INV_DROP:      cursor=SDL_CUR_c_drop; break;

        case CMD_WEA_USE:       cursor=SDL_CUR_c_use; break;
        case CMD_WEA_USE_WITH:  cursor=SDL_CUR_c_usewith; break;
        case CMD_WEA_TAKE:      cursor=SDL_CUR_c_take; break;
        case CMD_WEA_SWAP:      cursor=SDL_CUR_c_swap; break;
        case CMD_WEA_DROP:      cursor=SDL_CUR_c_drop; break;

        case CMD_CON_TAKE:      cursor=SDL_CUR_c_take; break;
        case CMD_CON_FASTTAKE:	cursor=SDL_CUR_c_take; break;   // needs different cursor!!!

        case CMD_CON_BUY:       cursor=SDL_CUR_c_buy; break;
        case CMD_CON_FASTBUY:   cursor=SDL_CUR_c_buy; break;    // needs different cursor!!!

        case CMD_CON_SWAP:      cursor=SDL_CUR_c_swap; break;
        case CMD_CON_DROP:      cursor=SDL_CUR_c_drop; break;
        case CMD_CON_SELL:      cursor=SDL_CUR_c_sell; break;
        case CMD_CON_FASTSELL:  cursor=SDL_CUR_c_sell; break;   // needs different cursor!!!
        case CMD_CON_FASTDROP:  cursor=SDL_CUR_c_drop; break;   // needs different cursor!!!

        case CMD_MAP_LOOK:      cursor=SDL_CUR_c_look; break;
        case CMD_ITM_LOOK:      cursor=SDL_CUR_c_look; break;
        case CMD_CHR_LOOK:      cursor=SDL_CUR_c_look; break;
        case CMD_INV_LOOK:      cursor=SDL_CUR_c_look; break;
        case CMD_WEA_LOOK:      cursor=SDL_CUR_c_look; break;
        case CMD_CON_LOOK:      cursor=SDL_CUR_c_look; break;

        case CMD_MAP_CAST_L:    cursor=SDL_CUR_c_spell; break;
        case CMD_ITM_CAST_L:    cursor=SDL_CUR_c_spell; break;
        case CMD_CHR_CAST_L:    cursor=SDL_CUR_c_spell; break;
        case CMD_MAP_CAST_R:    cursor=SDL_CUR_c_spell; break;
        case CMD_ITM_CAST_R:    cursor=SDL_CUR_c_spell; break;
        case CMD_CHR_CAST_R:    cursor=SDL_CUR_c_spell; break;
        case CMD_SPL_SET_L:     cursor=SDL_CUR_c_set; break;
        case CMD_SPL_SET_R:     cursor=SDL_CUR_c_set; break;

        case CMD_SKL_RAISE:     cursor=SDL_CUR_c_raise; break;

        case CMD_SAY_HITSEL:    cursor=SDL_CUR_c_say; break;

        case CMD_DROP_GOLD:     cursor=SDL_CUR_c_drop; break;
        case CMD_TAKE_GOLD:     cursor=SDL_CUR_c_take; break;

        case CMD_JUNK_ITEM:     cursor=SDL_CUR_c_junk; break;


        case CMD_SPEED0:
        case CMD_SPEED1:
        case CMD_SPEED2:        cursor=SDL_CUR_c_set; break;

        case CMD_TELEPORT:	    cursor=SDL_CUR_c_take; break;

        case CMD_HELP_NEXT:
        case CMD_HELP_PREV:
        case CMD_HELP_CLOSE:	cursor=SDL_CUR_c_use; break;

        case CMD_HELP_MISC:	    if (helpsel!=-1) cursor=SDL_CUR_c_use;
                                else if (questsel!=-1) cursor=SDL_CUR_c_use;
                                else cursor=SDL_CUR_c_only;
                                break;

        case CMD_HELP:		    cursor=SDL_CUR_c_use; break;
        case CMD_QUEST:		    cursor=SDL_CUR_c_use; break;
        case CMD_EXIT:		    cursor=SDL_CUR_c_use; break;
        case CMD_NOLOOK:	    cursor=SDL_CUR_c_use; break;

        case CMD_COLOR:		    cursor=SDL_CUR_c_use; break;

        default:                cursor=SDL_CUR_c_only; break;
    }

    if (cur_cursor!=cursor) {
        sdl_set_cursor(cursor);
        cur_cursor=cursor;
    }
}

void set_cmd_key_states(void) {
    int km;

    km=sdl_keymode();

    vk_shift=(km&SDL_KEYM_SHIFT) || shift_override;
    vk_control=(km&SDL_KEYM_CTRL) || control_override;
    vk_alt=(km&SDL_KEYM_ALT)!=0;

    vk_char=vk_control;
    vk_item=vk_shift;
    vk_spell=vk_alt;
}

static int get_near_ground(int x,int y) {
    int mapx,mapy;
    extern int display_help,display_quest;

    if (display_help || display_quest) stom(x-110,y,&mapx,&mapy);
    else stom(x,y,&mapx,&mapy);

    if (mapx<0 || mapy<0 || mapx>=MAPDX || mapy>=MAPDY) return -1;

    return mapmn(mapx,mapy);
}

static int get_near_item(int x,int y,int flag,int small) {
    int mapx,mapy,sx,sy,ex,ey,mn,scrx,scry,nearest=-1,look;
    double dist,nearestdist=100000000;
    extern int display_help,display_quest;

    if (display_help || display_quest) stom(mousex-110,mousey,&mapx,&mapy);
    else stom(mousex,mousey,&mapx,&mapy);

    if (small) look=0;
    else look=MAPDX;

    sx=max(0,mapx-look);
    sy=max(0,mapy-look);;
    ex=min(MAPDX-1,mapx+look);
    ey=min(MAPDY-1,mapy+look);

    for (mapy=sy; mapy<=ey; mapy++) {
        for (mapx=sx; mapx<=ex; mapx++) {

            mn=mapmn(mapx,mapy);

            if (!(map[mn].rlight)) continue;
            if (!(map[mn].flags&flag)) continue;
            if (!(map[mn].isprite)) continue;

            mtos(mapx,mapy,&scrx,&scry);
            if (display_help || display_quest) scrx+=110;

            dist=(x-scrx)*(x-scrx)+(y-scry)*(y-scry);

            if (dist<nearestdist) {
                nearestdist=dist;
                nearest=mn;
            }
        }
    }

    return nearest;
}

static int get_near_char(int x,int y) {
    int mapx,mapy,sx,sy,ex,ey,mn,scrx,scry,nearest=-1,look;
    double dist,nearestdist=100000000;
    extern int display_help,display_quest;

    if (display_help || display_quest) stom(mousex-110,mousey,&mapx,&mapy);
    else stom(mousex,mousey,&mapx,&mapy);

    look=MAPDX;

    sx=max(0,mapx-look);
    sy=max(0,mapy-look);;
    ex=min(MAPDX-1,mapx+look);
    ey=min(MAPDY-1,mapy+look);

    for (mapy=sy; mapy<=ey; mapy++) {
        for (mapx=sx; mapx<=ex; mapx++) {

            mn=mapmn(mapx,mapy);

            if (!(map[mn].rlight)) continue;
            if (!(map[mn].csprite)) continue;

            mtos(mapx,mapy,&scrx,&scry);
            if (display_help || display_quest) scrx+=110;

            dist=(x-scrx)*(x-scrx)+(y-scry)*(y-scry);

            if (dist<nearestdist) {
                nearestdist=dist;
                nearest=mn;
            }
        }
    }

    return nearest;
}

static int get_near_button(int x,int y) {
    int b;
    int n=-1,ndist=1000000,dist;

    if (x<0 || y<0 || x>=XRES || y>=YRES) return -1;

    for (b=0; b<MAX_BUT; b++) {

        if (but[b].flags&BUTF_NOHIT) continue;

        dist=(butx(b)-x)*(butx(b)-x)+(buty(b)-y)*(buty(b)-y);
        if (dist>but[b].sqhitrad) continue;

        if (dist>ndist) continue;

        ndist=dist;
        n=b;
    }

    return n;
}

static void set_invoff(int bymouse,int ny) {
    if (bymouse) {
        invoff+=mousedy/LINEHEIGHT;
        mousedy=mousedy%LINEHEIGHT;
    } else invoff=ny;

    if (invoff<0) invoff=0;
    if (invoff>max_invoff) invoff=max_invoff;

    but[BUT_SCR_TR].y=but[BUT_SCR_UP].y+10+(but[BUT_SCR_DW].y-but[BUT_SCR_UP].y-20)*invoff/max(1,max_invoff);
}

static void set_skloff(int bymouse,int ny) {
    if (bymouse) {
        skloff+=mousedy/LINEHEIGHT;
        mousedy=mousedy%LINEHEIGHT;
    } else skloff=ny;

    if (skloff<0) skloff=0;
    if (skloff>max_skloff) skloff=max_skloff;

    if (!con_cnt) but[BUT_SCL_TR].y=but[BUT_SCL_UP].y+10+(but[BUT_SCL_DW].y-but[BUT_SCL_UP].y-20)*skloff/max(1,max_skloff);
}

static void set_conoff(int bymouse,int ny) {
    if (bymouse) {
        // conoff=(ny-(but[BUT_SCL_UP].y+10))*max_conoff/(but[BUT_SCL_DW].y-but[BUT_SCL_UP].y-20); // DIVISION BY ZERO can't happen
        conoff+=mousedy/LINEHEIGHT;
        mousedy=mousedy%LINEHEIGHT;
    } else conoff=ny;

    if (conoff<0) conoff=0;
    if (conoff>max_conoff) conoff=max_conoff;

    if (con_cnt) but[BUT_SCL_TR].y=but[BUT_SCL_UP].y+10+(but[BUT_SCL_DW].y-but[BUT_SCL_UP].y-20)*conoff/max(1,max_conoff);
}

static void set_skltab(void) {
    int i,use,flag,n;
    int experience_left,raisecost;
    static int itab[V_MAX+1]={
        -1,
        0,1,2,              // powers
        3,4,5,6,            // bases
        7,8,9,10,38,41,         // armor etc
        12,13,14,15,16,40,      // fight skills
        17,18,19,20,21,22,23,24,    // 2ndary fight skills
        28,29,30,31,32,33,34,11,39, // spells
        25,26,27,35,36,37,      // misc skills
        42,
        43,44,45,46,47,48,49,50,51,52,
        53,54,55,56,57,58,59,60,61,62
    };

    experience_left=experience-experience_used;

    //for (flag=use=0,i=-1; i<V_MAX; i++) {
    for (flag=use=0,n=0; n<=V_MAX; n++) {

        i=itab[n];

        if (flag && (i==0 || i==3 || i==7 || i==12 || i==17 || i==25 || i==28 || i==42 || i==43)) {

            if (use==skltab_max) skltab=xrealloc(skltab,(skltab_max+=8)*sizeof(SKLTAB),MEM_GUI);

            bzero(&skltab[use],sizeof(SKLTAB));
            skltab[use].v=STV_EMPTYLINE;

            use++;
            flag=0;
        }

        if (i==-1) {
            // negative exp

            if (experience_left>=0) continue;

            if (use==skltab_max) skltab=xrealloc(skltab,(skltab_max+=8)*sizeof(SKLTAB),MEM_GUI);

            strcpy(skltab[use].name,"Negative experience");
            skltab[use].v=STV_JUSTAVALUE;
            skltab[use].curr=(int)(-1000.0*experience_left/max(1,experience_used));
            skltab[use].button=0;

            use++;
            flag=1;

        } else if (value[0][i] || value[1][i] || i==V_WEAPON || i==V_ARMOR || i==V_SPEED || i==V_LIGHT) {

            if (use==skltab_max) skltab=xrealloc(skltab,(skltab_max+=8)*sizeof(SKLTAB),MEM_GUI);

            if (value[1][i] && i!=V_DEMON && i!=V_COLD && i<V_PROFBASE) skltab[use].button=1;
            else skltab[use].button=0;

            skltab[use].v=i;

            strcpy(skltab[use].name,skill[i].name);
            skltab[use].base=value[1][i];
            skltab[use].curr=value[0][i];
            skltab[use].raisecost=raisecost=raise_cost(i,value[1][i]);

            if (i==V_WEAPON || i==V_ARMOR || i==V_SPEED || i==V_LIGHT || i==V_DEMON || i==V_COLD || i>=V_PROFBASE) {
                skltab[use].barsize=0;
            } else if (experience_left>=0) {
                if (raisecost>0 && experience_left>=raisecost) skltab[use].barsize=max(1,raisecost*(SKLWIDTH-10)/experience_left);
                else if (experience_left>=0 && raisecost>0) skltab[use].barsize=-experience_left*(SKLWIDTH-10)/raisecost;
                else skltab[use].barsize=0;
            } else skltab[use].barsize=0;

            use++;
            flag=1;
        }
    }

    skltab_cnt=use;
    max_skloff=max(0,skltab_cnt-SKLDY);

    set_skloff(0,skloff);
}

static void set_button_flags(void) {
    int b,i;

    if (con_cnt) {
        for (b=BUT_CON_BEG; b<=BUT_CON_END; b++) but[b].flags&=~BUTF_NOHIT;
        for (b=BUT_SKL_BEG; b<=BUT_SKL_END; b++) but[b].flags|=BUTF_NOHIT;
    } else {
        for (b=BUT_SKL_BEG; b<=BUT_SKL_END; b++) {
            i=skloff+b-BUT_SKL_BEG;
            if (i>=skltab_cnt || !skltab[i].button || skltab[i].barsize<=0) but[b].flags|=BUTF_NOHIT;
            else but[b].flags&=~BUTF_NOHIT;
        }
    }

}

static int is_fkey_use_item(int i) {
    switch (item[i]) {
        case 10290:
        case 10294:
        case 10298:
        case 10302:
        case 10000:
        case 50204:
        case 50205:
        case 50206:
        case 50207:
        case 50208:
        case 50209:
        case 50211:
        case 50212:	return 0;
        default:	return item_flags[i]&IF_USE;
    }
}

static int get_skl_look(int x,int y) {
    int b,i;
    for (b=BUT_SKL_BEG; b<=BUT_SKL_END; b++) {
        i=skloff+b-BUT_SKL_BEG;
        if (i>=skltab_cnt) continue;
        if (x>but[b].x-5 && x<but[b].x+70 && y>but[b].y-5 && y<but[b].y+5) return skltab[i].v;
    }
    return -1;
}

static void cmd_look_skill(int nr) {
    if (nr>=0 && nr<=V_MAX) {
        addline("%s: %s",skill[nr].name,skilldesc[nr]);
    } else addline("Unknown.");
}

static void set_cmd_states(void) {
    int i,c;
    static int oldconcnt=0; // ;-)
    extern int display_help,display_quest;
    static char title[256];
    char buf[256];

    set_cmd_key_states();
    set_map_values(map,tick);
    set_mapadd(-map[mapmn(MAPDX/2,MAPDY/2)].xadd,-map[mapmn(MAPDX/2,MAPDY/2)].yadd);

    // update
    if (update_skltab) { set_skltab(); update_skltab=0; }
    if (oldconcnt!=con_cnt) {
        conoff=0;
        max_conoff=(con_cnt/CONDX)-CONDY;
        oldconcnt=con_cnt;
        set_conoff(0,conoff);
        set_skloff(0,skloff);
    }
    max_invoff=((INVENTORYSIZE-30)/INVDX)-INVDY;
    set_button_flags();

    plrmn=mapmn(MAPDX/2,MAPDY/2);

    sprintf(buf,"%s - Astonia 3 v%d.%d.%d - (%u.%u.%u.%u:%u)",
            (map[plrmn].cn && player[map[plrmn].cn].name[0])?player[map[plrmn].cn].name:"Someone",
            (VERSION>>16)&255,(VERSION>>8)&255,(VERSION)&255,
            (target_server>>24)&255,
            (target_server>>16)&255,
            (target_server>>8)&255,
            (target_server>>0)&255,
            target_port);
    if (strcmp(title,buf)) {
        extern SDL_Window *sdlwnd;
        strcpy(title,buf);
        SDL_SetWindowTitle(sdlwnd,title);
    }

    // update fkeyitem
    fkeyitem[0]=fkeyitem[1]=fkeyitem[2]=fkeyitem[3]=0;
    for (i=30; i<30+INVENTORYSIZE; i++) {
        c=(i-2)%4;
        if (fkeyitem[c]==0 && (is_fkey_use_item(i))) fkeyitem[c]=i;
    }

    // a button captured - we leave all as is was (i know it's hard to update before, but i have to for the scrollbars)
    if (capbut!=-1) {
        // some very simple stuff is right here
        if (capbut==BUT_GLD) {
            takegold+=(mousedy/2)*(mousedy/2)*(mousedy<=0?1:-1);

            if (takegold<0) takegold=0;
            if (takegold>gold) takegold=gold;

            mousedy=0;
        }
        return;
    }

    // reset
    butsel=mapsel=itmsel=chrsel=invsel=weasel=consel=splsel=sklsel=telsel=helpsel=colsel=skl_look_sel=questsel=-1;

    // hit teleport?
    telsel=get_teleport(mousex,mousey);
    if (telsel!=-1) butsel=BUT_TEL;

    colsel=get_color(mousex,mousey);
    if (colsel!=-1) butsel=BUT_COLOR;

    if ((display_help || display_quest) && butsel==-1) {
        if (mousex>=0 && mousex<=222 && mousey>=doty(DOT_TOP)+40 && mousey<=374+2) {
            butsel=BUT_HELP_MISC;

            if (display_help==1 && mousex>=7 && mousex<=136 && mousey>=234-40 && mousey<=234-40+MAXHELP*10) { // 312
                helpsel=(mousey-234+40)/10+2;
                if (mousex>110) helpsel+=12;

                if (helpsel<2 || helpsel>MAXHELP) helpsel=-1;
            }

            if (display_quest && mousex>=165 && mousex<=199) {
                int tmp,y;

                tmp=(mousey-55)/40;
                y=tmp*40+55;
                if (tmp>=0 && tmp<=8 && mousey>=y && mousey<=y+10) {
                    questsel=tmp;
                }
            }
        }
        if (mousex>=177 && mousex<=196 && mousey>=378-20 && mousey<=385-20) {
            butsel=BUT_HELP_PREV;
        }
        if (mousex>=200 && mousex<=219 && mousey>=378-20 && mousey<=385-20) {
            butsel=BUT_HELP_NEXT;
        }
        if (mousex>=211 && mousex<=218+6 && mousey>=3+40 && mousey<=10+40+12) {
            butsel=BUT_HELP_CLOSE;
        }
    }

    if (mousex>=704 && mousex<=739 && mousey>=22 && mousey<=30) butsel=BUT_HELP;
    if (mousex>=741 && mousex<=775 && mousey>=22 && mousey<=30) butsel=BUT_QUEST;
    if (mousex>=704 && mousex<=723 && mousey>=7 && mousey<=18) butsel=BUT_EXIT;
    if (mousex>=643 && mousex<=650 && mousey>=53 && mousey<=60) butsel=BUT_NOLOOK;

    // hit map
    if (!hitsel[0] && butsel==-1 && mousex>=dotx(DOT_MTL) && mousey>=doty(DOT_MTL) && doty(DOT_MBR) && mousey<doty(DOT_MBR)) {
        if (vk_char) chrsel=get_near_char(mousex,mousey);
        if (chrsel==-1 && vk_item) itmsel=get_near_item(mousex,mousey,CMF_USE|CMF_TAKE,csprite);
        if (chrsel==-1 && itmsel==-1 && !vk_char && (!vk_item || csprite)) mapsel=get_near_ground(mousex,mousey);

        if (mapsel!=-1 || itmsel!=-1 || chrsel!=-1)  butsel=BUT_MAP;
    }

    if (!hitsel[0] && butsel==-1) {
        butsel=get_near_button(mousex,mousey);

        // translate button
        if (butsel>=BUT_INV_BEG && butsel<=BUT_INV_END) invsel=30+invoff*INVDX+butsel-BUT_INV_BEG;
        else if (butsel>=BUT_WEA_BEG && butsel<=BUT_WEA_END) weasel=butsel-BUT_WEA_BEG;
        else if (butsel>=BUT_CON_BEG && butsel<=BUT_CON_END) consel=conoff*CONDX+butsel-BUT_CON_BEG;
        else if (butsel>=BUT_SKL_BEG && butsel<=BUT_SKL_END) sklsel=skloff+butsel-BUT_SKL_BEG;
    }

    // set lcmd
    lcmd=CMD_NONE;

    if (mapsel!=-1 && !vk_item && !vk_char) lcmd=CMD_MAP_MOVE;
    if (mapsel!=-1 &&  vk_item && !vk_char && csprite) lcmd=CMD_MAP_DROP;

    if (itmsel!=-1 &&  vk_item && !vk_char && !csprite && map[itmsel].flags&CMF_USE) lcmd=CMD_ITM_USE;
    if (itmsel!=-1 &&  vk_item && !vk_char && !csprite && map[itmsel].flags&CMF_TAKE) lcmd=CMD_ITM_TAKE;
    if (itmsel!=-1 &&  vk_item && !vk_char &&  csprite && map[itmsel].flags&CMF_USE) lcmd=CMD_ITM_USE_WITH;

    if (chrsel!=-1 && !vk_item &&  vk_char && !csprite) lcmd=CMD_CHR_ATTACK;
    if (chrsel!=-1 && !vk_item &&  vk_char &&  csprite) lcmd=CMD_CHR_GIVE;

    if (invsel!=-1 && !vk_item && !vk_char && !csprite &&  item[invsel] && (!con_type || !con_cnt)) lcmd=CMD_INV_USE;
    if (invsel!=-1 && !vk_item && !vk_char && !csprite && !item[invsel] && (!con_type || !con_cnt)) lcmd=CMD_INV_USE;       // fake
    if (invsel!=-1 && !vk_item && !vk_char &&  csprite &&  item[invsel] && (!con_type || !con_cnt)) lcmd=CMD_INV_USE_WITH;

    if (invsel!=-1 && !vk_item && !vk_char && !csprite &&  item[invsel] &&  con_type==2 &&  con_cnt) lcmd=CMD_CON_FASTSELL;
    if (invsel!=-1 && !vk_item && !vk_char && !csprite && !item[invsel] &&  con_type==2 &&  con_cnt) lcmd=CMD_CON_FASTSELL; // fake
    if (invsel!=-1 && !vk_item && !vk_char &&  csprite &&  item[invsel] &&  con_type==2 &&  con_cnt) lcmd=CMD_CON_FASTSELL;

    if (invsel!=-1 && !vk_item && !vk_char && !csprite &&  item[invsel] &&  con_type==1 &&  con_cnt) lcmd=CMD_CON_FASTDROP;
    if (invsel!=-1 && !vk_item && !vk_char && !csprite && !item[invsel] &&  con_type==1 &&  con_cnt) lcmd=CMD_CON_FASTDROP; // fake
    if (invsel!=-1 && !vk_item && !vk_char &&  csprite &&  item[invsel] &&  con_type==1 &&  con_cnt) lcmd=CMD_CON_FASTDROP;

    if (invsel!=-1 && !vk_item && !vk_char &&  csprite && !item[invsel]) lcmd=CMD_INV_USE_WITH; // fake
    if (invsel!=-1 &&  vk_item && !vk_char && !csprite &&  item[invsel]) lcmd=CMD_INV_TAKE;
    if (invsel!=-1 &&  vk_item && !vk_char && !csprite && !item[invsel]) lcmd=CMD_INV_TAKE;  // fake - slot is empty so i can't take
    if (invsel!=-1 &&  vk_item && !vk_char &&  csprite &&  item[invsel]) lcmd=CMD_INV_SWAP;
    if (invsel!=-1 &&  vk_item && !vk_char &&  csprite && !item[invsel]) lcmd=CMD_INV_DROP;

    if (weasel!=-1 && !vk_item && !vk_char && !csprite &&  item[weatab[weasel]]) lcmd=CMD_WEA_USE;
    if (weasel!=-1 && !vk_item && !vk_char && !csprite && !item[weatab[weasel]]) lcmd=CMD_WEA_USE;      // fake
    if (weasel!=-1 && !vk_item && !vk_char &&  csprite &&  item[weatab[weasel]]) lcmd=CMD_WEA_USE_WITH;
    if (weasel!=-1 && !vk_item && !vk_char &&  csprite && !item[weatab[weasel]]) lcmd=CMD_WEA_USE_WITH; // fake
    if (weasel!=-1 &&  vk_item && !vk_char && !csprite &&  item[weatab[weasel]]) lcmd=CMD_WEA_TAKE;
    if (weasel!=-1 &&  vk_item && !vk_char && !csprite && !item[weatab[weasel]]) lcmd=CMD_WEA_TAKE; // fake - slot is empty so i can't take
    if (weasel!=-1 &&  vk_item && !vk_char &&  csprite &&  item[weatab[weasel]]) lcmd=CMD_WEA_SWAP;
    if (weasel!=-1 &&  vk_item && !vk_char &&  csprite && !item[weatab[weasel]]) lcmd=CMD_WEA_DROP;

    if (consel!=-1 &&  vk_item && !vk_char && !csprite && con_type==1 && con_cnt &&  container[consel]) lcmd=CMD_CON_TAKE;
    if (consel!=-1 &&  vk_item && !vk_char && !csprite && con_type==1 && con_cnt && !container[consel]) lcmd=CMD_CON_TAKE;  // fake - slot is empty so i can't take (buy is also not possible)

    if (consel!=-1 && !vk_item && !vk_char && !csprite && con_type==1 && con_cnt &&  container[consel]) lcmd=CMD_CON_FASTTAKE;
    if (consel!=-1 && !vk_item && !vk_char && !csprite && con_type==1 && con_cnt && !container[consel]) lcmd=CMD_CON_FASTTAKE;  // fake

    if (consel!=-1 &&  vk_item && !vk_char && !csprite && con_type==2 && con_cnt) lcmd=CMD_CON_BUY;
    if (consel!=-1 && !vk_item && !vk_char && !csprite && con_type==2 && con_cnt) lcmd=CMD_CON_FASTBUY;

    if (consel!=-1 &&  vk_item && !vk_char &&  csprite && con_type==1 && con_cnt &&  container[consel]) lcmd=CMD_CON_SWAP;
    if (consel!=-1 &&  vk_item && !vk_char &&  csprite && con_type==1 && con_cnt && !container[consel]) lcmd=CMD_CON_DROP;
    if (consel!=-1 &&  vk_item && !vk_char &&  csprite && con_type==2 && con_cnt) lcmd=CMD_CON_SELL;

    if (splsel!=-1 && !vk_item && !vk_char) lcmd=CMD_SPL_SET_L;

    if (telsel!=-1) lcmd=CMD_TELEPORT;
    if (colsel!=-1) lcmd=CMD_COLOR;


    if (lcmd==CMD_NONE) {
        if (butsel==BUT_SCR_UP) lcmd=CMD_INV_OFF_UP;
        if (butsel==BUT_SCR_DW) lcmd=CMD_INV_OFF_DW;
        if (butsel==BUT_SCR_TR && !vk_lbut) lcmd=CMD_INV_OFF_TR;

        if (butsel==BUT_SCL_UP && !con_cnt) lcmd=CMD_SKL_OFF_UP;
        if (butsel==BUT_SCL_DW && !con_cnt) lcmd=CMD_SKL_OFF_DW;
        if (butsel==BUT_SCL_TR && !con_cnt && !vk_lbut) lcmd=CMD_SKL_OFF_TR;

        if (butsel==BUT_SCL_UP &&  con_cnt) lcmd=CMD_CON_OFF_UP;
        if (butsel==BUT_SCL_DW &&  con_cnt) lcmd=CMD_CON_OFF_DW;
        if (butsel==BUT_SCL_TR &&  con_cnt && !vk_lbut) lcmd=CMD_CON_OFF_TR;

        if (sklsel!=-1) lcmd=CMD_SKL_RAISE;

        if (hitsel[0]) lcmd=CMD_SAY_HITSEL;

        if (vk_item && butsel==BUT_GLD && csprite>=SPR_GOLD_BEG && csprite<=SPR_GOLD_END) lcmd=CMD_DROP_GOLD;
        if (!vk_item && butsel==BUT_GLD && csprite>=SPR_GOLD_BEG && csprite<=SPR_GOLD_END) { takegold=cprice; lcmd=CMD_TAKE_GOLD; }
        if (!vk_item && butsel==BUT_GLD && !csprite) { takegold=0; lcmd=CMD_TAKE_GOLD; }
        if (vk_item && butsel==BUT_JNK) lcmd=CMD_JUNK_ITEM;

        if (butsel>=BUT_MOD_WALK0 && butsel<=BUT_MOD_WALK2) lcmd=CMD_SPEED0+butsel-BUT_MOD_WALK0;

        if (butsel==BUT_HELP_MISC) lcmd=CMD_HELP_MISC;
        if (butsel==BUT_HELP_PREV) lcmd=CMD_HELP_PREV;
        if (butsel==BUT_HELP_NEXT) lcmd=CMD_HELP_NEXT;
        if (butsel==BUT_HELP_CLOSE) lcmd=CMD_HELP_CLOSE;
        if (butsel==BUT_EXIT) lcmd=CMD_EXIT;
        if (butsel==BUT_HELP) lcmd=CMD_HELP;
        if (butsel==BUT_QUEST) lcmd=CMD_QUEST;
        if (butsel==BUT_NOLOOK) lcmd=CMD_NOLOOK;
    }


    // set rcmd
    rcmd=CMD_NONE;

    skl_look_sel=get_skl_look(mousex,mousey);
    if (con_cnt==0 && skl_look_sel!=-1) rcmd=CMD_SKL_LOOK;
    else if (!vk_spell) {
        if (mapsel!=-1) rcmd=CMD_MAP_LOOK;
        if (itmsel!=-1) rcmd=CMD_ITM_LOOK;
        if (chrsel!=-1) rcmd=CMD_CHR_LOOK;
        if (invsel!=-1) rcmd=CMD_INV_LOOK;
        if (weasel!=-1) rcmd=CMD_WEA_LOOK;
        if (consel!=-1) rcmd=CMD_CON_LOOK;
        if (splsel!=-1) rcmd=CMD_SPL_SET_R;
    } else {
        if (mapsel!=-1) rcmd=CMD_MAP_CAST_R;
        if (itmsel!=-1) rcmd=CMD_ITM_CAST_R;
        if (chrsel!=-1) rcmd=CMD_CHR_CAST_R;
        if (splsel!=-1) rcmd=CMD_SPL_SET_R;
    }

    // set cursor

    if (vk_rbut) set_cmd_cursor(rcmd);
    else set_cmd_cursor(lcmd);
}

static void exec_cmd(int cmd,int a) {
    extern int display_help,display_quest;

    switch (cmd) {
        case CMD_NONE:          return;

        case CMD_MAP_MOVE:      cmd_move(originx-MAPDX/2+mapsel%MAPDX,originy-MAPDY/2+mapsel/MAPDX); return;
        case CMD_MAP_DROP:      cmd_drop(originx-MAPDX/2+mapsel%MAPDX,originy-MAPDY/2+mapsel/MAPDX); return;

        case CMD_ITM_TAKE:      cmd_take(originx-MAPDX/2+itmsel%MAPDX,originy-MAPDY/2+itmsel/MAPDX); return;
        case CMD_ITM_USE:       cmd_use(originx-MAPDX/2+itmsel%MAPDX,originy-MAPDY/2+itmsel/MAPDX); return;
        case CMD_ITM_USE_WITH:  cmd_use(originx-MAPDX/2+itmsel%MAPDX,originy-MAPDY/2+itmsel/MAPDX); return;

        case CMD_CHR_ATTACK:    cmd_kill(map[chrsel].cn); return;
        case CMD_CHR_GIVE:      cmd_give(map[chrsel].cn); return;

        case CMD_INV_USE:       cmd_use_inv(invsel); return;
        case CMD_INV_USE_WITH:  cmd_use_inv(invsel); return;
        case CMD_INV_TAKE:      cmd_swap(invsel); return;
        case CMD_INV_SWAP:      cmd_swap(invsel); return;
        case CMD_INV_DROP:      cmd_swap(invsel); return;

        case CMD_CON_FASTDROP:  cmd_fastsell(invsel); return;
        case CMD_CON_FASTSELL:  cmd_fastsell(invsel); return;

        case CMD_WEA_USE:       cmd_use_inv(weatab[weasel]); return;
        case CMD_WEA_USE_WITH:  cmd_use_inv(weatab[weasel]); return;
        case CMD_WEA_TAKE:      cmd_swap(weatab[weasel]); return;
        case CMD_WEA_SWAP:      cmd_swap(weatab[weasel]); return;
        case CMD_WEA_DROP:      cmd_swap(weatab[weasel]); return;

        case CMD_CON_TAKE:      //return;
        case CMD_CON_BUY:       //return;
        case CMD_CON_SWAP:      //return;
        case CMD_CON_DROP:      //return;
        case CMD_CON_SELL:      cmd_con(consel); return;
        case CMD_CON_FASTTAKE:
        case CMD_CON_FASTBUY:	cmd_con_fast(consel); return;

        case CMD_MAP_LOOK:      cmd_look_map(originx-MAPDX/2+mapsel%MAPDX,originy-MAPDY/2+mapsel/MAPDX); return;
        case CMD_ITM_LOOK:      cmd_look_item(originx-MAPDX/2+itmsel%MAPDX,originy-MAPDY/2+itmsel/MAPDX); return;
        case CMD_CHR_LOOK:      cmd_look_char(map[chrsel].cn); return;
        case CMD_INV_LOOK:      cmd_look_inv(invsel); return;
        case CMD_WEA_LOOK:      cmd_look_inv(weatab[weasel]); return;
        case CMD_CON_LOOK:      cmd_look_con(consel); return;

        case CMD_MAP_CAST_L:    cmd_some_spell(/*spelltab[curspell_l].cl*/CL_FIREBALL,originx-MAPDX/2+mapsel%MAPDX,originy-MAPDY/2+mapsel/MAPDX,0); break;
        case CMD_ITM_CAST_L:    cmd_some_spell(/*spelltab[curspell_l].cl*/CL_FIREBALL,originx-MAPDX/2+itmsel%MAPDX,originy-MAPDY/2+itmsel/MAPDX,0); break;
        case CMD_CHR_CAST_L:    cmd_some_spell(/*spelltab[curspell_l].cl*/CL_FIREBALL,0,0,map[chrsel].cn); break;
        case CMD_MAP_CAST_R:    cmd_some_spell(/*spelltab[curspell_r].cl*/CL_BALL,originx-MAPDX/2+mapsel%MAPDX,originy-MAPDY/2+mapsel/MAPDX,0); break;
        case CMD_ITM_CAST_R:    cmd_some_spell(/*spelltab[curspell_r].cl*/CL_BALL,originx-MAPDX/2+itmsel%MAPDX,originy-MAPDY/2+itmsel/MAPDX,0); break;
        case CMD_CHR_CAST_R:    cmd_some_spell(/*spelltab[curspell_r].cl*/CL_BALL,0,0,map[chrsel].cn); break;

        case CMD_SLF_CAST_K:	cmd_some_spell(a,0,0,map[plrmn].cn); break;
        case CMD_MAP_CAST_K:    cmd_some_spell(a,originx-MAPDX/2+mapsel%MAPDX,originy-MAPDY/2+mapsel/MAPDX,0); break;
        case CMD_CHR_CAST_K:    cmd_some_spell(a,0,0,map[chrsel].cn); break;

        case CMD_SPL_SET_L:     curspell_l=splsel; break;
        case CMD_SPL_SET_R:     curspell_r=splsel; break;

        case CMD_SKL_RAISE:     cmd_raise(skltab[sklsel].v); break;

        case CMD_INV_OFF_UP:    set_invoff(0,invoff-1); break;
        case CMD_INV_OFF_DW:    set_invoff(0,invoff+1); break;
        case CMD_INV_OFF_TR:    set_invoff(1,0/*mousey*/); break;

        case CMD_SKL_OFF_UP:    set_skloff(0,skloff-1); break;
        case CMD_SKL_OFF_DW:    set_skloff(0,skloff+1); break;
        case CMD_SKL_OFF_TR:    set_skloff(1,0/*mousey*/); break;

        case CMD_CON_OFF_UP:    set_conoff(0,conoff-1); break;
        case CMD_CON_OFF_DW:    set_conoff(0,conoff+1); break;
        case CMD_CON_OFF_TR:    set_conoff(1,0/*mousey*/); break;

        case CMD_SAY_HITSEL:    cmd_add_text(hitsel); break;

        case CMD_USE_FKEYITEM:	cmd_use_inv(fkeyitem[a]); return;

        case CMD_DROP_GOLD:     cmd_drop_gold(); return;
        case CMD_TAKE_GOLD:     cmd_take_gold(takegold); return;

        case CMD_JUNK_ITEM:     cmd_junk_item(); return;

        case CMD_SPEED0:        if (pspeed!=0) cmd_speed(0); return;
        case CMD_SPEED1:        if (pspeed!=1) cmd_speed(1); return;
        case CMD_SPEED2:        if (pspeed!=2) cmd_speed(2); return;

        case CMD_TELEPORT:	if (telsel==1042) clan_offset=16-clan_offset;
            else {
                if (telsel>=64 && telsel<=100) cmd_teleport(telsel+clan_offset);
                else cmd_teleport(telsel);
            }
            return;
        case CMD_COLOR:		cmd_color(colsel); return;
        case CMD_SKL_LOOK:	cmd_look_skill(skl_look_sel); return;

        case CMD_HELP_NEXT:	if (display_help) { display_help++; if (display_help>MAXHELP) display_help=1; }
            if (display_quest) { display_quest++; if (display_quest>MAXQUEST2) display_quest=1; }
            return;
        case CMD_HELP_PREV:	if (display_help) { display_help--; if (display_help<1) display_help=MAXHELP; }
            if (display_quest) { display_quest--; if (display_quest<1) display_quest=MAXQUEST2; }
            return;
        case CMD_HELP_CLOSE:	display_help=0; display_quest=0; return;
        case CMD_HELP_MISC:	if (helpsel>0 && helpsel<=MAXHELP && display_help) display_help=helpsel;
            if (questsel!=-1) quest_select(questsel);
            return;
        case CMD_HELP:		if (display_help) display_help=0;
            else { display_help=1; display_quest=0; }
            return;
        case CMD_QUEST:		if (display_quest) display_quest=0;
            else { display_quest=1; display_help=0; }
            return;

        case CMD_EXIT:		quit=1; return;
        case CMD_NOLOOK:	show_look=0; return;

    }
    return;
}

#define GEN_SET_GAMMA           2 // a
#define GEN_SET_LIGHTEFFECT	5

int exec_gen(int gen,int a,char *c) {
    switch (gen) {
        case GEN_SET_GAMMA:
            if (a<1) return -1;
            if (a>31) return -1;
            dd_gamma=a;
            return dd_gamma;
        case GEN_SET_LIGHTEFFECT:
            if (a<1) return -1;
            if (a>31) return -1;
            dd_lighteffect=a;
            return dd_lighteffect;
    }
    return 0;
}

#define MAXCMDLINE	199
#define MAXHIST		20
static char cmdline[MAXCMDLINE+1]={""};
static char *history[MAXHIST];
static int cmdcursor=0,cmddisplay=0,histpos=-1;
extern char user_keys[];

void update_user_keys(void) {
    int n;

    for (n=0; n<10; n++) {
        keytab[n].userdef=user_keys[n];
        keytab[n+10].userdef=user_keys[n];
        keytab[n+20].userdef=user_keys[n];
    }
}

char* strcasestr(const char *haystack,const char *needle) {
    const char *ptr;

    for (ptr=needle; *haystack; haystack++) {
        if (toupper(*ptr)==toupper(*haystack)) {
            ptr++;
            if (!*ptr) return (char *)(haystack+(needle-ptr+1));
        } else ptr=needle;
    }
    return NULL;
}

int client_cmd(char *buf) {

    if (!strncmp(buf,"#ps ",3)) {
        playersprite_override=atoi(&buf[3]);
        return 1;
    }

    if (!strncmp(buf,"#gamma ",7)) {
        exec_gen(GEN_SET_GAMMA,atoi(&buf[7]),NULL);
        addline("using gamma %d",dd_gamma);
        return 1;
    }
    if (!strncmp(buf,"#light ",7)) {
        exec_gen(GEN_SET_LIGHTEFFECT,atoi(&buf[7]),NULL);
        addline("using light %d",dd_lighteffect);
        return 1;
    }
    if (!strncmp(buf,"#col1",5) || !strncmp(buf,"#col2",5) || !strncmp(buf,"#col3",5) ||
        !strncmp(buf,"/col1",5) || !strncmp(buf,"/col2",5) || !strncmp(buf,"/col3",5)) {
        show_color=1;
        show_cur=0;
        show_color_c[0]=map[MAPDX*MAPDY/2].rc.c1;
        show_color_c[1]=map[MAPDX*MAPDY/2].rc.c2;
        show_color_c[2]=map[MAPDX*MAPDY/2].rc.c3;
        return 1;
    }
    if (!strncmp(buf, "#sound ", 7)) {
    	play_sound(atoi(&buf[7]),0,0);
    	return 1;
    }
    if (!strncmp(buf, "#volume ", 8)) {
    	int new_sound_volume = atoi(&buf[8]);
    	if (new_sound_volume < 0) new_sound_volume = 0;
    	if (new_sound_volume >= 128) new_sound_volume = 128;
    	sound_volume = new_sound_volume;
    	addline("Volume is now at %d", sound_volume);
    	return 1;
    }
    if (!strncmp(buf, "#option ", 8)) {
    	int opt = atoi(&buf[8]);
        sprite_options^=opt;
        addline("Sprite_option is now  %08llX",sprite_options);
    	return 1;
    }
    if (!strncmp(buf,"#set ",5) || !strncmp(buf,"/set ",5)) {
        int what,key;
        char *ptr;

        ptr=buf+5;

        while (isspace(*ptr)) ptr++;
        what=atoi(ptr);
        if (what==0) what=9;
        else what--;

        while (isdigit(*ptr)) ptr++;
        while (isspace(*ptr)) ptr++;
        key=toupper(*ptr);

        if (what<0 || what>9) {
            addline("Spell is out of bounds (must be between 0 and 9)");
            return 1;
        }
        if (key<'A' || key>'Z') {
            addline("Key is out of bounds (must be between A and Z)");
            return 1;
        }
        user_keys[what]=key;
        update_user_keys();
        save_options();

        addline("Set key %c for spell %d.",key,what==9?0:what+1);

        return 1;
    }
    if (strcasestr(buf,password)) {
        addline("°c3Sorry, but you are not allowed to say your password. No matter what you're promised, do not give your password to anyone! The only things which happened to players who did are: Loss of all items, lots of negative experience, bad karma and locked characters. If you really, really think you have to tell your password to someone, then I'm sure you'll find a way around this block.");
        return 1;
    }

    return 0;
}

char rem_buf[10][256]={""};
int rem_in=0,rem_out=0;
void cmd_remember(char *ptr) {
    char *start=ptr,*dst;
    char tmp[256];

    if (*ptr!='#' && *ptr!='/') return;
    ptr++;
    if (*ptr!='t' && *ptr!='T') return;
    ptr++;
    if (*ptr==' ') goto do_remember;
    if (*ptr!='e' && *ptr!='E') return;
    ptr++;
    if (*ptr==' ') goto do_remember;
    if (*ptr!='l' && *ptr!='L') return;
    ptr++;
    if (*ptr==' ') goto do_remember;
    if (*ptr!='l' && *ptr!='L') return;
    ptr++;
    if (*ptr==' ') goto do_remember;
    return;

do_remember:
    while (isspace(*ptr)) ptr++;
    while (*ptr && !isspace(*ptr)) ptr++;

    for (dst=tmp; start<=ptr; *dst++=*start++);
    *dst=0;

    if (strcmp(tmp,rem_buf[rem_in])) {
        rem_in=(rem_in+1)%10;
        strcpy(rem_buf[rem_in],tmp);
    }
    rem_out=rem_in;
}

void cmd_fetch(char *ptr) {
    if (rem_out!=(rem_in+1)%10) strcpy(ptr,rem_buf[rem_out]);
    rem_out=(rem_out+9)%10;
}

void cmd_proc(int key) {
    switch (key) {
        case CMD_BACK:	if (cmdcursor<1) break;
            memmove(cmdline+cmdcursor-1,cmdline+cmdcursor,MAXCMDLINE-cmdcursor);
            cmdline[MAXCMDLINE-1]=0;
            cmdcursor--;
            break;
        case CMD_DELETE:
            memmove(cmdline+cmdcursor,cmdline+cmdcursor+1,MAXCMDLINE-cmdcursor-1);
            cmdline[MAXCMDLINE-1]=0;
            break;

        case CMD_LEFT:	if (cmdcursor>0) cmdcursor--;
            break;

        case CMD_RIGHT:	if (cmdcursor<MAXCMDLINE-1) {
                if (cmdline[cmdcursor]==0) cmdline[cmdcursor]=' ';
                cmdcursor++;
            }
            break;

        case CMD_HOME:	cmdcursor=0;
            break;

        case CMD_END:	for (cmdcursor=MAXCMDLINE-2; cmdcursor>=0; cmdcursor--) if (cmdline[cmdcursor]) break; cmdcursor++;
            break;

        case CMD_UP:	if (histpos<MAXHIST-1 && history[histpos+1]) histpos++;
            else break;
            bzero(cmdline,sizeof(cmdline));
            strcpy(cmdline,history[histpos]);
            for (cmdcursor=MAXCMDLINE-2; cmdcursor>=0; cmdcursor--) if (cmdline[cmdcursor]) break;
            cmdcursor++;
            break;

        case CMD_DOWN:	if (histpos>0) histpos--;
            else { bzero(cmdline,sizeof(cmdline)); cmdcursor=0; histpos=-1; break; }
            bzero(cmdline,sizeof(cmdline));
            strcpy(cmdline,history[histpos]);
            for (cmdcursor=MAXCMDLINE-2; cmdcursor>=0; cmdcursor--) if (cmdline[cmdcursor]) break;
            cmdcursor++;
            break;

        case CMD_RETURN:    // TODO: why is there a 13 here?
        case 13:	if (!client_cmd(cmdline) && cmdline[0]) cmd_text(cmdline);
            cmd_remember(cmdline);
            if (history[MAXHIST-1]) xfree(history[MAXHIST-1]);
            memmove(history+1,history,sizeof(history)-sizeof(history[0]));
            history[0]=xstrdup(cmdline,MEM_TEMP);
            cmdcursor=cmddisplay=0; histpos=-1;
            bzero(cmdline,sizeof(cmdline));
            break;

        case 9:		bzero(cmdline,sizeof(cmdline));
            cmd_fetch(cmdline);
            for (cmdcursor=MAXCMDLINE-2; cmdcursor>=0; cmdcursor--) if (cmdline[cmdcursor]) break;
            cmdcursor++;
            break;

        default:	if (key<32 || key>127) { /* addline("%d",key); */ break; }
            if (cmdcursor<MAXCMDLINE-1) {
                memmove(cmdline+cmdcursor+1,cmdline+cmdcursor,MAXCMDLINE-cmdcursor-1);
                cmdline[cmdcursor++]=key;
            }
            break;
    }
}

void cmd_add_text(char *buf) {
    while (*buf) cmd_proc(*buf++);
}

void display_cmd(int px,int py) {
    int n,x,tmp;

    if (cmdcursor<cmddisplay) cmddisplay=0;

    for (x=0,n=cmdcursor; n>cmddisplay; n--) {
        x+=dd_char_len(cmdline[n]);
        if (x>625-230-4) {
            cmddisplay=n;
            break;
        }
    }

    for (x=0,n=cmddisplay; n<MAXCMDLINE; n++) {
        tmp=dd_drawtext_char(dotx(DOT_TXT)+x,doty(DOT_TXT)+149,cmdline[n],IRGB(31,31,31));
        if (n==cmdcursor) {
            if (cmdline[n]) dd_shaded_rect(dotx(DOT_TXT)+x-1,doty(DOT_TXT)+149,dotx(DOT_TXT)+x+tmp+1,doty(DOT_TXT)+149+9);
            else dd_shaded_rect(dotx(DOT_TXT)+x,doty(DOT_TXT)+149,dotx(DOT_TXT)+x+4,doty(DOT_TXT)+149+9);
        }
        x+=tmp;
        if (x>dotx(DOT_TXT)+625-230) break;
    }
}

void gui_sdl_keyproc(int wparam) {
    int i;
    extern int display_gfx;
    extern int display_help,display_quest;

    switch (wparam) {

        case SDLK_ESCAPE:       cmd_stop(); show_look=0; display_gfx=0; teleporter=0; show_tutor=0; display_help=0; display_quest=0; show_color=0; return;
        case SDLK_F1:           if (fkeyitem[0]) exec_cmd(CMD_USE_FKEYITEM,0); return;
        case SDLK_F2:           if (fkeyitem[1]) exec_cmd(CMD_USE_FKEYITEM,1); return;
        case SDLK_F3:           if (fkeyitem[2]) exec_cmd(CMD_USE_FKEYITEM,2); return;
        case SDLK_F4:           if (fkeyitem[3]) exec_cmd(CMD_USE_FKEYITEM,3); return;

        case SDLK_F5:		    cmd_speed(1); return;
        case SDLK_F6:           cmd_speed(0); return;
        case SDLK_F7:           cmd_speed(2); return;

        case SDLK_F8:		    nocut^=1; return;

        case SDLK_F9:		    if (display_quest) display_quest=0;
                                else { display_help=0; display_quest=1; }
                                return;

        case SDLK_F10:		    display_vc^=1; list_mem(); return;

        case SDLK_F11:          if (display_help) display_help=0;
                                else { display_quest=0; display_help=1; }
                                return;
        case SDLK_F12:          quit=1; return;

        case SDLK_RETURN:
        case SDLK_KP_ENTER:     cmd_proc(CMD_RETURN); return;
        case SDLK_DELETE:       cmd_proc(CMD_DELETE); return;
        case SDLK_BACKSPACE:    cmd_proc(CMD_BACK); return;
        case SDLK_LEFT:         cmd_proc(CMD_LEFT); return;
        case SDLK_RIGHT:        cmd_proc(CMD_RIGHT); return;
        case SDLK_HOME:         cmd_proc(CMD_HOME); return;
        case SDLK_END:          cmd_proc(CMD_END); return;
        case SDLK_UP:           cmd_proc(CMD_UP); return;
        case SDLK_DOWN:         cmd_proc(CMD_DOWN); return;

        case SDLK_KP_0:         wparam='0'; goto spellbindkey;
        case SDLK_KP_1:         wparam='1'; goto spellbindkey;
        case SDLK_KP_2:         wparam='2'; goto spellbindkey;
        case SDLK_KP_3:         wparam='3'; goto spellbindkey;
        case SDLK_KP_4:         wparam='4'; goto spellbindkey;
        case SDLK_KP_5:         wparam='5'; goto spellbindkey;
        case SDLK_KP_6:         wparam='6'; goto spellbindkey;
        case SDLK_KP_7:         wparam='7'; goto spellbindkey;
        case SDLK_KP_8:         wparam='8'; goto spellbindkey;
        case SDLK_KP_9:         wparam='9'; goto spellbindkey;

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
        case 'g':
        case 'h':
        case 'i':
        case 'j':
        case 'k':
        case 'l':
        case 'm':
        case 'n':
        case 'o':
        case 'p':
        case 'q':
        case 'r':
        case 's':
        case 't':
        case 'u':
        case 'v':
        case 'w':
        case 'x':
        case 'y':
        case 'z':
            spellbindkey:
            if (!vk_item && !vk_char && !vk_spell) return;

            wparam=toupper(wparam);

            for (i=0; i<max_keytab; i++) {

                if (keytab[i].keycode!=wparam && keytab[i].userdef!=wparam) continue;

                if ((keytab[i].vk_item  && !vk_item) || (!keytab[i].vk_item  && vk_item)) continue;
                if ((keytab[i].vk_char  && !vk_char) || (!keytab[i].vk_char  && vk_char)) continue;
                if ((keytab[i].vk_spell && !vk_spell) || (!keytab[i].vk_spell && vk_spell)) continue;

                if (keytab[i].cl_spell) {
                    if (keytab[i].tgt==TGT_MAP) exec_cmd(CMD_MAP_CAST_K,keytab[i].cl_spell);
                    else if (keytab[i].tgt==TGT_CHR) exec_cmd(CMD_CHR_CAST_K,keytab[i].cl_spell);
                    else if (keytab[i].tgt==TGT_SLF) exec_cmd(CMD_SLF_CAST_K,keytab[i].cl_spell);
                    else return;     // hu ?
                    keytab[i].usetime=now;
                    return;
                }
                return;
            }
            return;

        case SDLK_PAGEUP:       dd_text_pageup(); break;
        case SDLK_PAGEDOWN:     dd_text_pagedown(); break;
    }
}

void gui_sdl_mouseproc(int x,int y,int what) {
    extern int x_offset,y_offset;
    int delta;

    switch (what) {
        case SDL_MOUM_NONE:
            mousex=x;
            mousey=y;

            if (capbut!=-1) {
                if (mousex!=XRES/2 || mousey!=YRES/2) {
                    mousedx+=mousex-(XRES/2);
                    mousedy+=mousey-(YRES/2);
                    sdl_set_cursor_pos(XRES/2,YRES/2);
                }
            }

            mousex/=mouse_scale;
            mousey/=mouse_scale;
            mousex-=x_offset;
            mousey-=y_offset;

            if (butsel!=-1 && vk_lbut && (but[butsel].flags&BUTF_MOVEEXEC)) exec_cmd(lcmd,0);
            break;

        case SDL_MOUM_LDOWN:
            vk_lbut=1;

            if (butsel!=-1 && capbut==-1 && (but[butsel].flags&BUTF_CAPTURE)) {
                sdl_show_cursor(0);
                sdl_capture_mouse(1);
                mousedx=0;
                mousedy=0;
                sdl_set_cursor_pos(XRES/2,YRES/2);
                capbut=butsel;
            }
            break;


        case SDL_MOUM_LUP:
            vk_lbut=0;
            if (capbut!=-1) {
                sdl_set_cursor_pos(but[capbut].x*mouse_scale+x_offset,but[capbut].y*mouse_scale+y_offset);
                sdl_capture_mouse(0);
                sdl_show_cursor(1);
                if (!(but[capbut].flags&BUTF_MOVEEXEC)) exec_cmd(lcmd,0);
                capbut=-1;
            } else exec_cmd(lcmd,0);
            break;

        case SDL_MOUM_RDOWN:
            vk_rbut=1;
            break;

        case SDL_MOUM_RUP:
            vk_rbut=0;
            exec_cmd(rcmd,0);
            break;

        case SDL_MOUM_WHEEL:
            delta=y;

            if (mousex>=dotx(DOT_SKL) && mousex<dotx(DOT_SK2) && mousey>=doty(DOT_SKL) && mousey<doty(DOT_SK2)) {	// skill / depot / merchant
				while (delta>0) { if (!con_cnt) set_skloff(0,skloff-1); else set_conoff(0,conoff-1); delta--; }
				while (delta<0) { if (!con_cnt) set_skloff(0,skloff+1); else set_conoff(0,conoff+1); delta++; }
				break;
			}

			if (mousex>=dotx(DOT_TXT) && mousex<dotx(DOT_TX2) && mousey>=doty(DOT_TXT) && mousey<doty(DOT_TX2)) {	// chat
				while (delta>0) { dd_text_lineup(); dd_text_lineup(); dd_text_lineup(); delta--; }
				while (delta<0) { dd_text_linedown(); dd_text_linedown(); dd_text_linedown(); delta++; }
				break;
			}

			if (mousex>=dotx(DOT_IN1) && mousex<dotx(DOT_IN2) && mousey>=doty(DOT_IN1) && mousey<doty(DOT_IN2)) {	// inventory
				while (delta>0) { set_invoff(0,invoff-1); delta--; }
				while (delta<0) { set_invoff(0,invoff+1); delta++; }
				break;
			}
            break;
    }
}

int main_init(void) {
    int i,x,y;

    whitecolor=IRGB(31,31,31);
    lightgraycolor=IRGB(28,28,28);
    graycolor=IRGB(22,22,22);
    darkgraycolor=IRGB(15,15,15);
    blackcolor=IRGB(0,0,0);

    lightredcolor=IRGB(31,0,0);
    redcolor=IRGB(22,0,0);
    darkredcolor=IRGB(15,0,0);

    lightgreencolor=IRGB(0,31,0);
    greencolor=IRGB(0,22,0);
    darkgreencolor=IRGB(0,15,0);

    lightbluecolor=IRGB(5,15,31);
    bluecolor=IRGB(3,10,22);
    darkbluecolor=IRGB(1,5,15);

    lightorangecolor=IRGB(31,20,16);
    orangecolor=IRGB(31,16,8);
    darkorangecolor=IRGB(15,8,4);

    textcolor=IRGB(27,22,22);

    healthcolor=lightredcolor;
    manacolor=lightbluecolor;
    endurancecolor=IRGB(31,31,5);
    shieldcolor=IRGB(31,15,5);

    // dots
    dot=xmalloc(MAX_DOT*sizeof(DOT),MEM_GUI);

    // top left, bottom right of screen
    set_dot(DOT_TL,0,0,0);
    set_dot(DOT_BR,800,600,0);

    // equipment, inventory, container. center of first displayed item.
    set_dot(DOT_WEA,180,20,DOTF_TOPOFF);
    set_dot(DOT_INV,660,398,0);
    set_dot(DOT_CON,20,398,0);

    // inventory top left and bottom right
    set_dot(DOT_IN1,645,378,0);
    set_dot(DOT_IN2,795,538,0);

    // top and bottom window
    set_dot(DOT_TOP,0,  0,DOTF_TOPOFF);
    set_dot(DOT_BOT,0,370,0);

    // scroll bars
    set_dot(DOT_SCL,160+5,0,0);
    set_dot(DOT_SCR,640-5,0,0);
    set_dot(DOT_SCU,0,385,0);
    set_dot(DOT_SCD,0,530,0);

    // chat text
    set_dot(DOT_TXT,230,378,0);
    set_dot(DOT_TX2,624,538,0);

    // skill list
    set_dot(DOT_SKL,8,384,0);
    set_dot(DOT_SK2,156,538,0);

    // gold
    set_dot(DOT_GLD,195,520,0);

    // trashcan
    set_dot(DOT_JNK,610,520,0);

    // speed options: stealth/normal/fast
    set_dot(DOT_MOD,181,393,0);

    // map top left, bottom right, center
    set_dot(DOT_MTL,  0, 40,DOTF_TOPOFF);
    set_dot(DOT_MBR,800,376,0);
    set_dot(DOT_MCT,400,230,0);

    // buts
    but=xmalloc(MAX_BUT*sizeof(BUT),MEM_GUI);

    set_but(BUT_MAP,800/2,270,0,BUTID_MAP,0,BUTF_NOHIT);

    // note to self: do not use dotx(),doty() here because the moving top bar logic is built into the
    // button flags as well
    for (i=0; i<12; i++) set_but(BUT_WEA_BEG+i,dot[DOT_WEA].x+i*FDX,dot[DOT_WEA].y+0,40,BUTID_WEA,0,BUTF_TOPOFF);
    for (x=0; x<4; x++) for (y=0; y<4; y++) set_but(BUT_INV_BEG+x+y*4,dot[DOT_INV].x+x*FDX,dot[DOT_INV].y+y*FDX,40,BUTID_INV,0,0);
    for (x=0; x<4; x++) for (y=0; y<4; y++) set_but(BUT_CON_BEG+x+y*4,dot[DOT_CON].x+x*FDX,dot[DOT_CON].y+y*FDX,40,BUTID_CON,0,0);
    for (i=0; i<16; i++) set_but(BUT_SKL_BEG+i,dot[DOT_SKL].x,dot[DOT_SKL].y+i*LINEHEIGHT,40,BUTID_SKL,0,0);

    set_but(BUT_SCL_UP,dot[DOT_SCL].x+0,dot[DOT_SCU].y+0,30,BUTID_SCL,0,0);
    set_but(BUT_SCL_TR,dot[DOT_SCL].x+0,dot[DOT_SCU].y+10,40,BUTID_SCL,0,BUTF_CAPTURE|BUTF_MOVEEXEC);
    set_but(BUT_SCL_DW,dot[DOT_SCL].x+0,dot[DOT_SCD].y+0,30,BUTID_SCL,0,0);

    set_but(BUT_SCR_UP,dot[DOT_SCR].x+0,dot[DOT_SCU].y+0,30,BUTID_SCR,0,0);
    set_but(BUT_SCR_TR,dot[DOT_SCR].x+0,dot[DOT_SCU].y+10,40,BUTID_SCR,0,BUTF_CAPTURE|BUTF_MOVEEXEC);
    set_but(BUT_SCR_DW,dot[DOT_SCR].x+0,dot[DOT_SCD].y+0,30,BUTID_SCR,0,0);

    set_but(BUT_GLD,dot[DOT_GLD].x+0,dot[DOT_GLD].y+0,30,BUTID_GLD,0,BUTF_CAPTURE);

    set_but(BUT_JNK,dot[DOT_JNK].x+0,dot[DOT_JNK].y+0,30,BUTID_JNK,0,0);

    set_but(BUT_MOD_WALK0,dot[DOT_MOD].x+1*14,dot[DOT_MOD].y+0*30,30,BUTID_MOD,0,0);
    set_but(BUT_MOD_WALK1,dot[DOT_MOD].x+0*14,dot[DOT_MOD].y+0*30,30,BUTID_MOD,0,0);
    set_but(BUT_MOD_WALK2,dot[DOT_MOD].x+2*14,dot[DOT_MOD].y+0*30,30,BUTID_MOD,0,0);

    // other
    set_invoff(0,0);
    set_skloff(0,0);
    set_conoff(0,0);
    capbut=-1;

    // more inits
    init_game(dotx(DOT_MCT),doty(DOT_MCT));

    return 0;
}

void main_exit(void) {
    xfree(dot);
    dot=NULL;
    xfree(but);
    but=NULL;
    xfree(skltab);
    skltab=NULL;
    skltab_max=0;
    skltab_cnt=0;

    exit_game();
}

void flip_at(unsigned int t) {
    unsigned int tnow;
    int sdl_pre_do(int curtick);

    do {
        sdl_loop();
        if (!sdl_pre_do(tick)) Sleep(1);
        tnow=GetTickCount();
        /*if (GetActiveWindow()!=mainwnd) { // TODO: re-active this once we have the SDL window as only window?
            Sleep(100);
        } else Sleep(1); */
    } while (t>tnow);

    sdl_render();
}

int nextframe,nexttick;
uint64_t gui_time_network=0;

int main_loop(void) {
    void prefetch_game(int attick);
    int tmp,timediff,ltick=0,attick;
    extern int q_size;
    long long start;
    int do_one_tick=1;

    nexttick=GetTickCount()+MPT;
    nextframe=GetTickCount()+MPF;

    while (!quit) {

        now=GetTickCount();

        start=SDL_GetTicks64();
        poll_network();

        // check if we can go on
        if (sockstate>2) {

            // decode as many ticks as we can
            // and add their contents to the prefetch queue
            while ((attick=next_tick()))
                prefetch_game(attick);

            // get one tick to display?
            timediff=nexttick-GetTickCount();
            if (timediff<MPT/4) {  // do ticks slightly early
                do_one_tick=1;
                do_tick();
                ltick++;

                if (ltick==TICKS*10) {
                    void dd_get_client_info(struct client_info *ci);
                    struct client_info ci;

                    dd_get_client_info(&ci);
                    ci.idle=100*idle/tota;
                    ci.skip=100*skip/tota;
                    cl_client_info(&ci);
                }

                if (sockstate==4 && ltick%TICKS==0) {
                    cl_ticker();
                }
            }
        }

        if (sockstate==4) timediff=nextframe-GetTickCount();
        else timediff=1;
        gui_time_network+=SDL_GetTicks64()-start;

        if (timediff>-MPF/2) {
#ifdef TICKPRINT
            printf("Display tick %d, Frame %d\n",tick,frame);
#endif
            sdl_clear();
            display();

            timediff=nextframe-GetTickCount();
            if (timediff>0) idle+=timediff;
            else skip-=timediff;

            frames++;

            flip_at(nextframe);
        } else {
#ifdef TICKPRINT
            printf("Skip tick %d, Frame %d\n",tick,frame);
#endif
            skip-=timediff;

            sdl_loop();
        }

        if (do_one_tick) {
            switch (lasttick+q_size) {
                case 0:     tmp=MPT*2.00; break;
                case 1:     tmp=MPT*1.50; break;
                case 2:     tmp=MPT*1.40; break;
                case 3:     tmp=MPT*1.25; break;
                case 4:     tmp=MPT*1.10; break;
                case 5:     tmp=MPT+1; break;
                case 6:     tmp=MPT; break; // optimal
                case 7:     tmp=MPT-1; break;
                case 8:     tmp=MPT-1; break;
                case 9:     tmp=MPT*0.90; break;
                case 10:    tmp=MPT*0.75; break;
                case 11:    tmp=MPT*0.60; break;
                case 12:    tmp=MPT*0.50; break;
                default:    tmp=MPT*0.25; break;
            }
            nexttick+=tmp;
            tota+=tmp;
            if (tick%24==0) { tota/=2; skip/=2; idle/=2; frames/=2; }

            do_one_tick=0;
        }

        nextframe+=MPF;
    }

    close_client();

    return 0;
}

