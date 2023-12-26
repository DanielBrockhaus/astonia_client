/*
 * Part of Astonia Client (c) Daniel Brockhaus. Please read license.txt.
 *
 * Graphical User Interface
 *
 * Processing user input and calling the other GUI functions mostly in display.c.
 * Also contains the main loop.
 *
 */

#include <time.h>
#include <SDL.h>

#include "../../src/astonia.h"
#include "../../src/gui.h"
#include "../../src/gui/_gui.h"
#include "../../src/client.h"
#include "../../src/game.h"
#include "../../src/sdl.h"
#include "../../src/modder.h"

uint64_t gui_time_misc=0;

DLL_EXPORT int game_slowdown=0;

#define MAXHELP		24
#define MAXQUEST2	10

void cmd_add_text(char *buf,int typ);

// globals

int skip=1,idle=0,tota=1,frames=0;

// globals display

int display_vc=0;
int display_help=0,display_quest=0;

int playersprite_override=0;
int nocut=0;

int update_skltab=0;
int show_look=0;

int gui_topoff;     // offset of the top bar *above* the top of the window (0 ... -38)

DLL_EXPORT unsigned short int healthcolor,manacolor,endurancecolor,shieldcolor;
DLL_EXPORT unsigned short int whitecolor,lightgraycolor,graycolor,darkgraycolor,blackcolor;
DLL_EXPORT unsigned short int lightredcolor,redcolor,darkredcolor;
DLL_EXPORT unsigned short int lightgreencolor,greencolor,darkgreencolor;
DLL_EXPORT unsigned short int lightbluecolor,bluecolor,darkbluecolor;
DLL_EXPORT unsigned short int textcolor;
DLL_EXPORT unsigned short int lightorangecolor,orangecolor,darkorangecolor;

unsigned int now;

int cur_cursor=0;
int mousex=300,mousey=300,vk_rbut,vk_lbut,shift_override=0,control_override=0;
DLL_EXPORT int vk_shift,vk_control,vk_alt;
int mousedx,mousedy;
int vk_item,vk_char,vk_spell;

int vk_special=0,vk_special_time=0;

// globals wea

DLL_EXPORT int weatab[12]={9,6,8,11,0,1,2,4,5,3,7,10};
char weaname[12][32]={"RING","HAND","HAND","RING","NECK","HEAD","BACK","BODY","BELT","ARMS","LEGS","FEET"};

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

struct special_tab special_tab[]={
    {"Walk",0,0,0,0,0},
    {"Use/Take",1,0,0,0,0},
    {"Attack/Give",0,1,0,0,0},
    {"Warcry",0,0,CL_WARCRY,TGT_SLF,V_WARCRY},
    {"Pulse",0,0,CL_PULSE,TGT_SLF,V_PULSE},
    {"Fireball-CHAR",0,1,CL_FIREBALL,TGT_CHR,V_FIREBALL},
    {"Fireball-MAP",0,0,CL_FIREBALL,TGT_MAP,V_FIREBALL},
    {"Firering",0,0,CL_FIREBALL,TGT_SLF,V_FIREBALL},
    {"LBall-CHAR",0,1,CL_BALL,TGT_CHR,V_FLASH},
    {"LBall-MAP",0,0,CL_BALL,TGT_MAP,V_FLASH},
    {"Flash",0,0,CL_FLASH,TGT_SLF,V_FLASH},
    {"Freeze",0,0,CL_FREEZE,TGT_SLF,V_FREEZE},
    {"Shield",0,0,CL_MAGICSHIELD,TGT_SLF,V_MAGICSHIELD},
    {"Bless-SELF",0,0,CL_BLESS,TGT_SLF,V_BLESS},
    {"Bless-CHAR",0,1,CL_BLESS,TGT_CHR,V_BLESS},
    {"Heal-SELF",0,0,CL_HEAL,TGT_SLF,V_HEAL},
    {"Heal-CHAR",0,1,CL_HEAL,TGT_CHR,V_HEAL}
};
int max_special=sizeof(special_tab)/sizeof(special_tab[0]);

int fkeyitem[4];

// globals cmd

int plrmn;                      // mn of player
int mapsel;                     // mn
int itmsel;                     // mn
int chrsel;                     // mn
int invsel;                     // index into item
int weasel;                     // index into weatab
int consel;                     // index into item
int sklsel;
int sklsel2;
int butsel;                     // is always set, if any of the others is set
int telsel;
int helpsel;
int questsel;
int colsel;
int actsel;
int skl_look_sel;
int last_right_click_invsel=-1;

int action_ovr=-1;

int capbut=-1;                  // the button capturing the mouse

int takegold;                   // the amout of gold to take

char hitsel[256];               // something in the text (dx_drawtext()) is selected
int hittype=0;

DLL_EXPORT SKLTAB *skltab=NULL;
int skltab_max=0;
DLL_EXPORT int skltab_cnt=0;

int invoff,max_invoff;
int conoff,max_conoff;
int skloff,max_skloff;
int __skldy;
int __invdy;

int lcmd;
int rcmd;

// transformation

int mapoffx,mapoffy;
int mapaddx,mapaddy;   // small offset to smoothen walking

void gui_dump(FILE *fp) {
    fprintf(fp,"GUI datadump:\n");

    fprintf(fp,"skip: %d\n",skip);
    fprintf(fp,"idle: %d\n",idle);
    fprintf(fp,"tota: %d\n",tota);
    fprintf(fp,"frames: %d\n",frames);

    fprintf(fp,"\n");
}

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
int stom(int scrx,int scry,int *mapx,int *mapy) {

    if (scrx<dotx(DOT_MTL) || scrx>=dotx(DOT_MBR) || scry<doty(DOT_MTL) || scry>=dotx(DOT_MBR)) return 0;

    scrx-=stom_off_x;
    scry-=stom_off_y;
    scrx-=(mapoffx+mapaddx);
    scry-=(mapoffy+mapaddy)-10;
    *mapy=(40*scry-20*scrx-1)/(20*40);      // ??? -1 ???
    *mapx=(40*scry+20*scrx)/(20*40);

    return 1;
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


int (*do_display_help)(int)=_do_display_help;
DLL_EXPORT int _do_display_help(int nr) {
    int x=dotx(DOT_HLP)+10,y=doty(DOT_HLP)+8,oldy;

    switch (nr) {
        case 1:
            y=dd_drawtext_break(x,y,x+192,whitecolor,0,"Help Index"); y+=15;

            y=dd_drawtext_break(x,y,x+192,whitecolor,0,"Fast Help"); y+=5;
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"Walk: LEFT-CLICK");
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"Look on Ground:  RIGHT-CLICK");
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"Take/Drop/Use: SHIFT LEFT-CLICK");
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"Look at Item: SHIFT RIGHT-CLICK");
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"Attack/Give: CTRL LEFT-CLICK");
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"Look at Character: CTRL RIGHT-CLICK");
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"Use Item in Inventory: LEFT-CLICK or F1...F4");
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"Fast/Normal/Stealth: F5/F6/F7");
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"Scroll Chat Window: PAGE-UP/PAGE-DOWN");
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"Repeat last Tell: TAB");
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"Close Help: F11");
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"Show Walls: F8");
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"Quit Game: F12 - preferably on blue square");
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"Assign Wheel Button: Use Wheel"); y+=10;

            oldy=y;
            y=dd_drawtext_break(x,y,x+192,lightbluecolor,0,"* A");
            y=dd_drawtext_break(x,y,x+192,lightbluecolor,0,"* A");
            y=dd_drawtext_break(x,y,x+192,lightbluecolor,0,"* B");
            y=dd_drawtext_break(x,y,x+192,lightbluecolor,0,"* C");
            y=dd_drawtext_break(x,y,x+192,lightbluecolor,0,"* C");
            y=dd_drawtext_break(x,y,x+192,lightbluecolor,0,"* C");
            y=dd_drawtext_break(x,y,x+192,lightbluecolor,0,"* D");
            y=dd_drawtext_break(x,y,x+192,lightbluecolor,0,"* E");
            y=dd_drawtext_break(x,y,x+192,lightbluecolor,0,"* F");
            y=dd_drawtext_break(x,y,x+192,lightbluecolor,0,"* G");
            y=dd_drawtext_break(x,y,x+192,lightbluecolor,0,"* I");
            dd_drawtext_break(x,y,x+192,lightbluecolor,0,"* K");

            y=oldy;
            y=dd_drawtext_break(x+100,y,x+192,lightbluecolor,0,"* L");
            y=dd_drawtext_break(x+100,y,x+192,lightbluecolor,0,"* M");
            y=dd_drawtext_break(x+100,y,x+192,lightbluecolor,0,"* N");
            y=dd_drawtext_break(x+100,y,x+192,lightbluecolor,0,"* P");
            y=dd_drawtext_break(x+100,y,x+192,lightbluecolor,0,"* Q");
            y=dd_drawtext_break(x+100,y,x+192,lightbluecolor,0,"* R");
            y=dd_drawtext_break(x+100,y,x+192,lightbluecolor,0,"* S");
            y=dd_drawtext_break(x+100,y,x+192,lightbluecolor,0,"* S");
            y=dd_drawtext_break(x+100,y,x+192,lightbluecolor,0,"* S");
            y=dd_drawtext_break(x+100,y,x+192,lightbluecolor,0,"* T");
            y=dd_drawtext_break(x+100,y,x+192,lightbluecolor,0,"* W");
            break;

        case 2:

            y=dd_drawtext_break(x,y,x+192,whitecolor,0,"Accounts"); y+=5;
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"It is your responsibility to store your account in a safe place. If someone steals or messes with your account, you're still responsible. If you manage to lose your account, it is lost. If you lose your password, the only thing we can do is send it to your account's e-mail address. If that e-mail address turns out to be wrong or doesn't exist, there is nothing more we can do for you."); y+=10;

            y=dd_drawtext_break(x,y,x+192,whitecolor,0,"Account Payments"); y+=5;
            y=dd_drawtext_break_fmt(x,y,x+192,graycolor,0,"If you are having trouble with your account payments, or if you have questions concerning account payments, please write %s.",game_email_cash); y+=10;

            y=dd_drawtext_break(x,y,x+192,whitecolor,0,"Account Sharing"); y+=5;
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"When you share accounts with another player, you are risking the security of your game characters and their equipment.  In most cases of account sharing, characters are stripped of their equipment and their game gold by the one(s) that the account is being shared with.  Characters can end up locked or banned from the game, or with negative leveling experience.  Be wise, don't share!"); y+=10;
            break;
        case 3:

            y=dd_drawtext_break(x,y,x+192,whitecolor,0,"Alias commands"); y+=5;
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"Text phrases that you use repeatedly can be stored as Alias commands and retrieved with a few characters. You can store up to 32 alias commands. To store an alias command, you first have to pick a phrase to store, then give that phrase a name. The alias command for storing text is: /alias <alias phrase name> <phrase>"); y+=10;
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"For example, to get the phrase, \"Let's go penting today!\", whenever you type p1, you'd type: /alias p1 Let's go penting today!"); y+=10;
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"To delete an alias, first type: /alias to bring up your list of aliases.  Choose which alias you want to delete, then type:  /alias <name of alias>."); y+=10;
            break;

        case 4:
            y=dd_drawtext_break(x,y,x+192,whitecolor,0,"Banking"); y+=5;
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"Money and items can be stored in the Imperial Bank.  You have one account per character, and you can access your account at any bank.  SHIFT + LEFT CLICK on the cabinet in the bank to access your Depot (item storage locker).  Only gold coins can be deposited in your account or depot - silver coins cannot be deposited.  Talk to the banker to get more information about banking."); y+=10;

            y=dd_drawtext_break(x,y,x+192,whitecolor,0,"Base/Mod Values"); y+=5;
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"All your attributes and skills show two values:  the first one is the base value (it shows how much you have raised it); the second is the modified (mod) value.  It consists of the base value, plus bonuses from your base attributes and special items.  No skill or spell values can be raised through items by more than 50% of its base value."); y+=10;
            break;
        case 5:

            y=dd_drawtext_break(x,y,x+192,whitecolor,0,"Chat and Channels"); y+=5;
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"Everything you hear and say is displayed in the chat window at the bottom of your screen.  To talk to a character standing next to you, just type what you'd like to say and hit ENTER.  To say something in the general chat channel (the \"Gossip\" channel) which will be heard by all other players, type:  /c2 <your message> and hit ENTER.  Use the PAGE UP and PAGE DOWN keys on your keyboard to scroll the chat window up and down.  To see a list of channels in the game, type:  /channels.  To join a channel, type:  /join <channel number>.  To leave a channel, type:  /leave <channel number>.  Spamming, offensive language, and disruptive chatter is not allowed.  To send a message to a particular player, type:  /tell <player name> and then your message.  Nobody else will hear what you said."); y+=10;

            y=dd_drawtext_break(x,y,x+192,whitecolor,0,"Clans"); y+=5;
            y=dd_drawtext_break_fmt(x,y,x+192,graycolor,0,"There is detailed information about clans in the Game Manual at %s. To see a list of clans in the game, type: /clan.",game_url); y+=10;
            break;

        case 6:
            y=dd_drawtext_break(x,y,x+192,whitecolor,0,"Colors"); y+=5;
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"All characters enter the game with a set of default colors for their clothing and hair, but you can change the color of your character's shirt, pants/skirt, and hair/cap if you choose."); y+=10;
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"Matte Colors.  Use the Color Toolbox in the game to choose matte colors.  Type:  /col1  to bring up the Color Toolbox.  From left to right, the three circles at the bottom represent shirt color, pants/skirt color, and hair/cap color.  Click on a circle, then move the color bars to find a color that you like.  The model in the box displays your color choices.  Click the Exit button to exit the Color Toolbox."); y+=10;
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"Glossy Colors.  To make glossy colors, you use a command typed into the chat area instead of using the Color Toolbox.  Like mixing paints, the number values you choose (between 1-31) for the red (R), green (G), and blue (B) amounts determine how much of each is mixed in.  Adding an extra 31 to the red (R) value makes the color combination you have chosen a glossy color."); y+=10;
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"When typing the command, you first start by hitting the spacebar on your keyboard once, then  typing one of these commands:  "); y+=10;
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"/col1 <R><G><B> shirt color"); y+=5;
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"/col2 <R><G><B> pants/skirt/cape color"); y+=5;
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"/col3 <R><G><B> hair/cap color"); y+=10;
            break;
        case 7:
            y=dd_drawtext_break(x,y,x+192,whitecolor,0,"Commands"); y+=5;
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"Type: /help to see a list of all available commands.  You can type:  /status  to see a list of optional toggle commands that may aid your character's performance."); y+=10;

            y=dd_drawtext_break(x,y,x+192,whitecolor,0,"Complains/Harassment"); y+=5;
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"If another player harasses you,  type:  /complain <player><reason>  This command sends a screenshot of your chat window to Game Management.  Replace <player> with the name of the player bothering you.  The <reason> portion of the command is for you to enter your own comments regarding the situation. Please be aware that only the last 80 lines of text are sent and that each server-change (teleport) erases this buffer.  You can also type:  /ignore <name>  to ignore the things that player is saying to you."); y+=10;
            break;
        case 8:
            y=dd_drawtext_break(x,y,x+192,whitecolor,0,"Dying"); y+=5;
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"When you die, you will suddenly find yourself on a blue square - the last blue square that you stepped on.  You may see a message displayed on your screen telling you the name and level of the enemy that killed you.  If you were not saved, then your corpse will be at the place where you died and all of your items from your Equipment area and your Inventory will still be on your corpse. You have 30 minutes to make it back to your corpse to get these items. After 30 minutes, your corpse disappears, along with your items."); y+=10;
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"Allowing Access to Your Corpse:"); y+=5;
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"You can allow another player to retrieve your items from your corpse by typing: /allow <player name> Quest items and keys can only be retrieved from a corpse by the one who has died, even if you /allow someone to access to your corpse."); y+=10;
            break;

        case 9:
            y=dd_drawtext_break(x,y,x+192,whitecolor,0,"Enhancing Equipment"); y+=5;
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"Enhancing equipment is silver plating and/or gold plating a piece of equipment to make it stronger and more valuable.  To silver/gold plate an item, you will need silver/gold nuggets from the mines.  You must silver plate an item before you can gold plate it.  Silver adds +1 to all the stat(s) of an item;  for example, if you have a +2 parry sword, after silvering it you will have a +3 parry sword.  Gold plating then adds another +1 to all stat(s).  Once you have gold plated an item, you can only enhance it further by using orbs and/or welds.  To figure how much silver/gold you need for enhancing an item, the formula is:  (highest stat on item + 1) x 100 = amount of silver/gold needed. Silvering/guilding will add a level to weapons/armors: lvl 20 sword will become lvl 30 sword after silvering etc."); y+=10;
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"Silvering/guilding an item will also increase its level requirement.  For example, a level 20 sword will then become a level 30 sword."); y+=10;
            break;
        case 10:
            y=dd_drawtext_break(x,y,x+192,whitecolor,0,"Fighting"); y+=5;
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"To attack somebody, place your cursor over the character you'd like to attack, and then hit CTRL + LEFT CLICK."); y+=10;
            break;
        case 11:
            y=dd_drawtext_break(x,y,x+192,whitecolor,0,"Gold"); y+=5;
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"Gold and silver are the monetary units used in the game.  To give money to another person, place your cursor over your gold (bottom of your screen), LEFT CLICK, and slowly drag your mouse upwards until you have the amount on your cursor that you want.  Then, let your cursor rest on the person you wish to give the money to, and hit CTRL + LEFT CLICK."); y+=10;
            break;
        case 12:
            y=dd_drawtext_break(x,y,x+192,whitecolor,0,"Items"); y+=5;
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"To use or pick up items around you, put your cursor over the item and hit SHIFT + LEFT CLICK.  To use an item in your Inventory, LEFT CLICK on it.  To give an item to another character, take the item by using SHIFT + LEFT CLICK, then pull it over the other character and hit CTRL + LEFT CLICK.  To loot the corpses of slain enemies, place your cursor over the body and hit SHIFT + LEFT CLICK."); y+=10;
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"Some items have level restrictions and/or skill restrictions.  Some items can only be worn by mages, warriors, or seyans.  For example, a mage cannot use a sword and a warrior cannot use a staff.  If you cannot equip an item it may be because your class of character cannot wear that particular item, or because of level/skill restrictions on that item.  RIGHT click on the item to read more about it."); y+=10;
            break;
        case 13:
            y=dd_drawtext_break(x,y,x+192,whitecolor,0,"Karma"); y+=5;
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"Karma is a measurement of how well a player has obeyed game rules. When you receive a Punishment, you lose karma. All players enter the game with 0 karma. If you receive a Level 1 punishment, for example, your karma will drop to -1. Please review the Laws, Rules, and Regulations section in the Game Manual to familiarize yourself with the punishment system."); y+=10;
            break;
        case 14:
            y=dd_drawtext_break(x,y,x+192,whitecolor,0,"Leaving the game"); y+=5;
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"To leave the game, step on one of the blue tile rest areas and hit F12. You can also hit F12 when not on a blue tile, but your character will stay in that same spot for five minutes and risks being attacked."); y+=10;

            y=dd_drawtext_break(x,y,x+192,whitecolor,0,"Light"); y+=5;
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"Torches provide the main source of light in the game.  To use a torch, equip it, then LEFT CLICK on it to light it.  It is a good idea to carry extras with you at all times."); y+=10;

            y=dd_drawtext_break(x,y,x+192,whitecolor,0,"Looking at characters/items"); y+=5;
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"To look at a character, place your cursor over him and hit CTRL + RIGHT CLICK.  To look at an item around you, place your cursor over the item and hit  SHIFT + RIGHT CLICK.  To look at an item in your Equipment/Inventory areas or in a shop window, (place your cursor over the item and) RIGHT CLICK."); y+=10;
            break;
        case 15:
            y=dd_drawtext_break(x,y,x+192,whitecolor,0,"Mirrors"); y+=5;
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"Each area can have up to 26 mirrors (servers), to allow more players to be online at once.  Your mirror number determines which mirror you use.  You can see which mirror you are currently on by \"looking\" at yourself (place your cursor over yourself and hit CTRL + RIGHT CLICK).  If you would like to meet a player on a different mirror, go to a teleport station, click on the corresponding mirror number (M1 to M26) and teleport to the area that the other player is in.  You have to teleport, even if the other player is in the same area."); y+=10;
            break;
        case 16:
            y=dd_drawtext_break(x,y,x+192,whitecolor,0,"Navigational Directions"); y+=5;
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"Compass directions (North, East, South, West) are the same in the game as in real life.  North, for example, would be 'up' (the top of your screen).  East would be to the direct right of your screen."); y+=10;

            y=dd_drawtext_break(x,y,x+192,whitecolor,0,"Negative Experience"); y+=5;
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"When you die, you lose experience points. Too many deaths can result in Negative Experience. Once this Negative Experience is made up, then experience points obtained will once again count towards leveling."); y+=10;
            break;
        case 17:
            y=dd_drawtext_break(x,y,x+192,whitecolor,0,"Player Killing"); y+=5;
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"A PKer (Player Killer) is one that has chosen to kill other PKers - which also means that he can be killed by other PKers too.  If you are killed, the items in your Equipment area and your Inventory can be taken by the one who killed you.  You must be level 10 or higher and have a paid account to become a PKer.  To enable your PK status, type:  /playerkiller.  To attack someone who is a playerkiller and within your level range, type:  /hate <name> To disable your PK status, you must wait four (4) weeks since you last killed someone, then type:  /playerkiller"); y+=10;

            y=dd_drawtext_break(x,y,x+192,whitecolor,0,"The Pentagram Quest"); y+=5;
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"The Pentagram Quest is a game that all players (from about level 10 and up) can play together.  It's an ongoing game that takes place in \"the pents\" - a large, cavernous area partitioned off according to player levels.  The walls and floors of this area are covered with \"stars\" (pentagrams) and the object of the game is to touch as many pentagrams as possible as you fight off the evil demons that inhabit the area.  Once a randomly chosen number of pentagrams have been touched, the pents are \"solved\", and you receive experience points for the pentagrams you touched. The entrances to the Pentagram Quest  are southeast of blue squares in Aston - be sure to SHIFT + RIGHT CLICK on the doors to determine which level area is right for you!"); y+=10;
            break;
        case 18:
            y=dd_drawtext_break(x,y,x+192,whitecolor,0,"Quests"); y+=5;
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"Your first quest will be to find Lydia, the daughter of Gwendylon the mage.  She will ask you to find a potion which was stolen the night before.  Lydia lives in the grey building across from the fortress (the place where you first arrived in the game)."); y+=10;
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"NPCs (Non-Player Characters) give the quests in the game.  Even if you talked to an NPC before, talk to him again; he may tell you something new or give you another quest.  Say \"hi\" or <name> \"repeat\" to get an NPC to talk to you.   Be sure to step all the way into a room or area as you quest; monsters, chests, and doors may be hidden in the shadows.  Carry a torch to light your way, and always check the bodies of slain enemies (SHIFT + LEFT CLICK)."); y+=10;
            y=dd_drawtext_break(x,y,x+192,whitecolor,0,"Questions"); y+=5;
            y=dd_drawtext_break_fmt(x,y,x+192,graycolor,0,"If you have a question while in the game, you can always ask a Staffer. Staffers and other admin can be recognized by their name being in capital letters (i.e. \"COLOMAN\" is a member of Admin, \"Coloman\" is not).  For any other game related questions, please write to %s.",game_email_main); y+=10;
            break;
        case 19:
            y=dd_drawtext_break(x,y,x+192,whitecolor,0,"Reading books, signs, etc."); y+=5;
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"To read books, use SHIFT + LEFT CLICK.  To read signs, use SHIFT + RIGHT CLICK."); y+=10;
            break;
        case 20:
            y=dd_drawtext_break(x,y,x+192,whitecolor,0,"Saves"); y+=5;
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"With each new level you obtain as you play, you also receive a Save. A Save is a gift from Ishtar: if you die, your items stay with you instead of having them left on your corpse, and you will not get negative experience. The maximum number of Saves that a player can have at any time is 10."); y+=10;

            y=dd_drawtext_break(x,y,x+192,whitecolor,0,"Scamming"); y+=5;
            y=dd_drawtext_break_fmt(x,y,x+192,graycolor,0,"Most cases of scamming happen when players share passwords.  NEVER give your password to another player for any reason!  Make your passwords hard to guess by using a combination of numbers and letters.  Change your password often; go to %s, then click on Account Management to change your password.  Always use an NPC Trader when trading with another player.  The NPC Trader can be found in most towns in or near the banks - he is a non-playing character that will handle the trade for both parties.  If a player does not want to use an NPC Trader for trading with you, then do not trade with him - he could potentially steal your items.  Do not put your items on the ground when trading with another player or you risk losing them.  Be wary of loaning your equipment to others - unfortunately, many never see their items again.  Players are able to perform welding in the game, but welds are very valuable and should not be traded away too early.  Hold on to your welds until you learn more about the game!",game_url); y+=10;
            break;

        case 21:
            y=dd_drawtext_break(x,y,x+192,whitecolor,0,"Shops"); y+=5;
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"To open a shop window with a merchant, type:  trade <merchant name>  When trading with a merchant, the items for sale are shown at the bottom-left of your screen (the view of your skills/stats is temporarily replaced by the shop window).  To read about the items, RIGHT CLICK on them.  To buy something, LEFT CLICK on it.  To sell an item from your inventory, LEFT CLICK on it."); y+=10;

            y=dd_drawtext_break(x,y,x+192,whitecolor,0,"Skills/Stats"); y+=5;
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"In your stats/skills window (bottom-left of your screen) you see red/blue lines below your skills. Blue indicates that you have enough experience points to raise the skill; red indicates that you don't have enough experience points. To raise a skill, CLICK on the blue orb next to the skill."); y+=10;

            y=dd_drawtext_break(x,y,x+192,whitecolor,0,"Spells"); y+=5;
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"To use a spell, hit ALT and the corresponding number of the spell that appears on your screen.  Mages - to cast the Bless spell on another player, rest your cursor on him and hit CTRL 6.  The Bless spell raises base attributes (Wisdom, Intuition, Agility, Strength) by 1/4 modified bless value (rounded down), but by no more than 50%.  Warriors - hit ALT 8 to use Warcry."); y+=10;
            break;
        case 22:
            y=dd_drawtext_break(x,y,x+192,whitecolor,0,"Staffers"); y+=5;
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"Staffers are members of Game Management that help keep the in-game playing field running smoothly.  Staffers and other admin can be recognized by their name being in capital letters (i.e. \"COLOMAN\" is a member of Admin, \"Coloman\" is not).  Staffers help keep the peace, answer questions, and can give out karma (if needed) to unruly players."); y+=10;
            break;
        case 23:
            y=dd_drawtext_break(x,y,x+192,whitecolor,0,"Talking"); y+=5;
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"Everything you hear and say is displayed in the chat window at the bottom of your screen.  To talk to a character standing next to you, just type what you'd like to say and hit ENTER.  To say something in the general chat channel (the \"Gossip\" channel) which will be heard by all other players, type:  /c2 <your message> and hit ENTER.  Use the PAGE-UP and PAGE-DOWN keys on your keyboard to scroll the chat window up and down."); y+=10;

            y=dd_drawtext_break(x,y,x+192,whitecolor,0,"Transport System"); y+=5;
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"You will find Teleport Stations, relics of the ancient culture, all over the world.  SHIFT + LEFT CLICK on the Teleport Station to see a map of all Teleport Stations.  CLICK on the destination of your choice.  You will only be able to teleport to a destination that you have reached at least once before by foot.  Touch any new Teleport Station on your way so that you can teleport there in times to come."); y+=10;
            break;
        case 24:
            y=dd_drawtext_break(x,y,x+192,whitecolor,0,"Walking"); y+=5;
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"To walk, move your cursor to the place where you'd like to go, then LEFT CLICK on your destination."); y+=10;
            break;
    }
    return y;
}

void display_helpandquest(void) {
    if (display_help || display_quest)
        dd_copysprite(opt_sprite(995),dotx(DOT_HLP),doty(DOT_HLP),DDFX_NLIGHT,DD_NORMAL);

    if (display_help) do_display_help(display_help);
    if (display_quest) do_display_questlog(display_quest);
}

char perf_text[256];
static void set_cmd_states(void);

static void display_toplogic(void) {
    static int top_opening=0,top_closing=1,top_open=0;
    static int topframes=0;

    if (mousey<10) topframes++;
    else topframes=0;

    if (topframes>frames_per_second/2 && !top_opening && !top_open) { top_opening=1; top_closing=0; }
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
}

static int vk_special_dec(void) {
    int n,panic=99;

    for (n=(vk_special+max_special-1)%max_special; panic--; n=(n+max_special-1)%max_special) {
        if (!special_tab[n].req || value[0][special_tab[n].req]) {
            vk_special=n;
            return 1;
        }
    }
    return 0;
}

static int vk_special_inc(void) {
    int n,panic=99;

    for (n=(vk_special+1)%max_special; panic--; n=(n+1)%max_special) {
        if (!special_tab[n].req || value[0][special_tab[n].req]) {
            vk_special=n;
            return 1;
        }
    }
    return 0;
}

void display_wheel(void) {
    int i;

    dd_push_clip();
    dd_more_clip(0,0,800,600);

    if (now-vk_special_time<2000) {
        int n,panic=99;

        dd_shaded_rect(mousex+5,mousey-7-20,mousex+71,mousey+31,0x0000,95);

        for (n=(vk_special+1)%max_special,i=-1; panic-- && i>-3; n=(n+1)%max_special) {
            if (!special_tab[n].req || value[0][special_tab[n].req]) {
                dd_drawtext(mousex+9,mousey-3+i*10,graycolor,DD_LEFT,special_tab[n].name);
                i--;
            }
        }
        dd_drawtext(mousex+9,mousey-3,whitecolor,DD_LEFT,special_tab[vk_special].name);

        for (n=(vk_special+max_special-1)%max_special,i=1; panic-- && i<3; n=(n+max_special-1)%max_special) {
            if (!special_tab[n].req || value[0][special_tab[n].req]) {
                dd_drawtext(mousex+9,mousey-3+i*10,graycolor,DD_LEFT,special_tab[n].name);
                i++;
            }
        }
    }
    dd_pop_clip();
}

size_t get_memory_usage(void);

static void display(void) {
    extern int memptrs[MAX_MEM];
    extern int memsize[MAX_MEM];
    extern int memused;
    extern int memptrused;
    extern long long sdl_time_make,sdl_time_tex,sdl_time_tex_main,sdl_time_text,sdl_time_blit;
    int t,tmp;
    long long start=SDL_GetTicks64();

#if 0
    // Performance for stuff happening during the actual tick only.
    // So zero them now after preload is done.
    sdl_time_make=0;
    sdl_time_tex=0;
    sdl_time_text=0;
    sdl_time_blit=0;
#endif

    if ((tmp=sdl_check_mouse())) {
        mousex=-1;
        if (tmp==-1) mousey=0;
        else mousey=YRES/2;
    }

    display_toplogic();
    if (game_slowdown) {
        display_toplogic();
        display_toplogic();
        display_toplogic();
    }
    set_cmd_states();

    if (sockstate<4 && ((t=time(NULL)-socktimeout)>10 || !originx)) {
        dd_rect(0,0,800,540,blackcolor);
        display_screen();
        display_text();
        if ((now/1000)&1) dd_drawtext(800/2,540/2-60,redcolor,DD_CENTER|DD_LARGE,"not connected");
        dd_copysprite(60,800/2,(540-240)/2,DDFX_NLIGHT,DD_CENTER);
        if (!kicked_out) {
            dd_drawtext_fmt(800/2,540/2-40,textcolor,DD_SMALL|DD_CENTER|DD_FRAME,"Trying to establish connection. %d seconds...",t);
            if (t>15) {
                dd_drawtext_fmt(800/2,540/2-0,textcolor,DD_LARGE|DD_CENTER|DD_FRAME,"Please check %s for troubleshooting advice.",game_url);
            }
        }
        goto display_graphs;    // I know, I know. goto considered harmful and all that.
        return;
    }

    dd_push_clip();
    dd_more_clip(dotx(DOT_MTL),doty(DOT_MTL),dotx(DOT_MBR),doty(DOT_MBR));
    display_game();
    dd_pop_clip();

    display_screen();

    display_keys();
    if (game_options&GO_WHEEL) display_wheel();
    if (show_look) display_look();
    display_wear();
    display_inventory();
    display_action();
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
    display_selfbars();
    display_minimap();
    display_citem();
    context_display(mousex,mousey);
    display_helpandquest(); // display last because it is on top

display_graphs:

    int duration=SDL_GetTicks64()-start;

    if (display_vc) {
        extern long long texc_miss,texc_pre; //mem_tex,
        extern uint64_t sdl_backgnd_wait,sdl_backgnd_work,sdl_time_preload,sdl_time_load,gui_time_network;
        extern uint64_t gui_frametime,gui_ticktime;
        extern uint64_t sdl_time_pre1,sdl_time_pre2,sdl_time_pre3,sdl_time_mutex,sdl_time_alloc,sdl_time_make_main;
        extern int x_offset,y_offset; //pre_2,pre_in,pre_3;
        //static int dur=0,make=0,tex=0,text=0,blit=0,stay=0;
        static int size;
        static unsigned char dur_graph[100],size1_graph[100],size2_graph[100],size3_graph[100]; //,size_graph[100];load_graph[100],
        static unsigned char pre1_graph[100],pre2_graph[100],pre3_graph[100];
        //static int frame_min=99,frame_max=0,frame_step=0;
        //static int tick_min=99,tick_max=0,tick_step=0;
        int px=800-110,py=35+(!(game_options&GO_SMALLTOP) ? 0 : gui_topoff);

        //dd_drawtext_fmt(px,py+=10,0xffff,DD_SMALL|DD_LEFT|DD_FRAME|DD_NOCACHE,"skip %3.0f%%",100.0*skip/tota);
        //dd_drawtext_fmt(px,py+=10,0xffff,DD_SMALL|DD_LEFT|DD_FRAME|DD_NOCACHE,"idle %3.0f%%",100.0*idle/tota);
        //dd_drawtext_fmt(px,py+=10,IRGB(8,31,8),DD_LEFT|DD_FRAME|DD_NOCACHE,"Tex: %5.2f MB",mem_tex/(1024.0*1024.0));
        dd_drawtext_fmt(px,py+=10,IRGB(8,31,8),DD_LEFT|DD_FRAME|DD_NOCACHE,"Mem: %5.2f MB",get_memory_usage()/(1024.0*1024.0));

#if 0
        if (pre_in>=pre_3) size=pre_in-pre_3;
        else size=16384+pre_in-pre_3;

        dd_drawtext_fmt(px,py+=10,0xffff,DD_SMALL|DD_LEFT|DD_FRAME|DD_NOCACHE,"PreC %d",size);
#endif
#if 0
        extern int pre_in,pre_1,pre_2,pre_3;
        extern int texc_used;
        py+=10;
        dd_drawtext_fmt(px,py+=10,IRGB(8,31,8),DD_LEFT|DD_FRAME|DD_NOCACHE,"PreI %d",pre_in);
        dd_drawtext_fmt(px,py+=10,IRGB(8,31,8),DD_LEFT|DD_FRAME|DD_NOCACHE,"Pre1 %d",pre_1);
        dd_drawtext_fmt(px,py+=10,IRGB(8,31,8),DD_LEFT|DD_FRAME|DD_NOCACHE,"Pre2 %d",pre_2);
        dd_drawtext_fmt(px,py+=10,IRGB(8,31,8),DD_LEFT|DD_FRAME|DD_NOCACHE,"Pre3 %d",pre_3);
        dd_drawtext_fmt(px,py+=10,IRGB(8,31,8),DD_LEFT|DD_FRAME|DD_NOCACHE,"Used %d",texc_used);
        dd_drawtext_fmt(px,py+=10,IRGB(8,31,8),DD_LEFT|DD_FRAME|DD_NOCACHE,"Size %d",sdl_cache_size);
#endif
        //dd_drawtext_fmt(px,py+=10,0xffff,DD_SMALL|DD_LEFT|DD_FRAME|DD_NOCACHE,"Miss %lld",texc_miss);
        //dd_drawtext_fmt(px,py+=10,0xffff,DD_SMALL|DD_LEFT|DD_FRAME|DD_NOCACHE,"Prel %lld",texc_pre);

        py+=10;

        size=duration+gui_time_network;
        dd_drawtext(px,py+=10,IRGB(8,31,8),DD_LEFT|DD_FRAME,"Render");
        sdl_bargraph_add(sizeof(dur_graph),dur_graph,size<42?size:42);
        sdl_bargraph(px,py+=40,sizeof(dur_graph),dur_graph,x_offset,y_offset);

#if 0
        if (gui_frametime<frame_min) frame_min=gui_frametime;
        if (gui_frametime>frame_max) frame_max=gui_frametime;
        dd_drawtext_fmt(px,py+=10,IRGB(8,31,8),DD_NOCACHE|DD_LEFT|DD_FRAME,"FT %d %d",frame_min,frame_max);

        if (gui_ticktime<tick_min) tick_min=gui_ticktime;
        if (gui_ticktime>tick_max) tick_max=gui_ticktime;
        dd_drawtext_fmt(px,py+=10,IRGB(8,31,8),DD_NOCACHE|DD_LEFT|DD_FRAME,"TT %d %d",tick_min,tick_max);
#endif
        size=gui_frametime/2;
        dd_drawtext_fmt(px,py+=10,IRGB(8,31,8),DD_NOCACHE|DD_LEFT|DD_FRAME,"Frametime %lld",gui_frametime);
        sdl_bargraph_add(sizeof(pre2_graph),pre2_graph,size<42?size:42);
        sdl_bargraph(px,py+=40,sizeof(pre2_graph),pre2_graph,x_offset,y_offset);

        size=gui_ticktime/2;
        dd_drawtext_fmt(px,py+=10,IRGB(8,31,8),DD_NOCACHE|DD_LEFT|DD_FRAME,"Ticktime %lld",gui_ticktime);
        sdl_bargraph_add(sizeof(pre3_graph),pre3_graph,size<42?size:42);
        sdl_bargraph(px,py+=40,sizeof(pre3_graph),pre3_graph,x_offset,y_offset);
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


        size=(lasttick+q_size)*2;
        dd_drawtext_fmt(px,py+=10,IRGB(8,31,8),DD_FRAME|DD_LEFT,"Queue %d",size/2);
        sdl_bargraph_add(sizeof(pre2_graph),size3_graph,size<42?size:42);
        sdl_bargraph(px,py+=40,sizeof(pre2_graph),size3_graph,x_offset,y_offset);
#if 0
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
            dd_drawtext_fmt(px,py+=10,IRGB(8,31,8),DD_LEFT|DD_FRAME,"Make");
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
#if 0
        if (SDL_GetTicks()-frame_step>1000) {
            frame_step=SDL_GetTicks();
            frame_min=99;
            frame_max=0;
        }
        if (SDL_GetTicks()-tick_step>1000) {
            tick_step=SDL_GetTicks();
            tick_min=99;
            tick_max=0;
        }
#endif
    } //else dd_drawtext_fmt(650,15,0xffff,DD_SMALL|DD_FRAME,"Mirror %d",mirror);

    sprintf(perf_text,"mem usage=%.2f/%.2fMB, %.2f/%.2fKBlocks",
            memsize[0]/1024.0/1024.0,memused/1024.0/1024.0,
            memptrs[0]/1024.0,memptrused/1024.0);

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

        case CMD_ACTION:        cursor=SDL_CUR_c_use; break;

        default:                cursor=SDL_CUR_c_only; break;
    }

    if (cur_cursor!=cursor) {
        sdl_set_cursor(cursor);
        cur_cursor=cursor;
    }
}

void set_cmd_key_states(void) {
    SDL_Keymod km;

    km=SDL_GetModState();

    vk_shift=(km&SDL_KEYM_SHIFT) || shift_override;
    vk_control=(km&SDL_KEYM_CTRL) || control_override;
    vk_alt=(km&SDL_KEYM_ALT)!=0;

    vk_char=vk_control;
    vk_item=vk_shift;
    vk_spell=vk_alt;
}

DLL_EXPORT int get_near_ground(int x,int y) {
    int mapx,mapy;

    if (!stom(x,y,&mapx,&mapy)) return -1;

    if (mapx<0 || mapy<0 || mapx>=MAPDX || mapy>=MAPDY) return -1;

    return mapmn(mapx,mapy);
}

DLL_EXPORT int get_near_item(int x,int y,int flag,int looksize) {
    int mapx,mapy,sx,sy,ex,ey,mn,scrx,scry,nearest=-1;
    double dist,nearestdist=100000000;

    if (!stom(mousex,mousey,&mapx,&mapy)) return -1;

    sx=max(0,mapx-looksize);
    sy=max(0,mapy-looksize);;
    ex=min(MAPDX-1,mapx+looksize);
    ey=min(MAPDY-1,mapy+looksize);

    for (mapy=sy; mapy<=ey; mapy++) {
        for (mapx=sx; mapx<=ex; mapx++) {

            mn=mapmn(mapx,mapy);

            if (!(map[mn].rlight)) continue;
            if (!(map[mn].flags&flag)) continue;
            if (!(map[mn].isprite)) continue;

            mtos(mapx,mapy,&scrx,&scry);

            dist=(x-scrx)*(x-scrx)+(y-scry)*(y-scry);

            if (dist<nearestdist) {
                nearestdist=dist;
                nearest=mn;
            }
        }
    }

    return nearest;
}

DLL_EXPORT int get_near_char(int x,int y,int looksize) {
    int mapx,mapy,sx,sy,ex,ey,mn,scrx,scry,nearest=-1;
    double dist,nearestdist=100000000;

    if (!stom(mousex,mousey,&mapx,&mapy)) return -1;

    mn=mapmn(mapx,mapy);
    if (mn==MAPDX*MAPDY/2) return mn;   // return player character if clicked directly

    sx=max(0,mapx-looksize);
    sy=max(0,mapy-looksize);;
    ex=min(MAPDX-1,mapx+looksize);
    ey=min(MAPDY-1,mapy+looksize);

    for (mapy=sy; mapy<=ey; mapy++) {
        for (mapx=sx; mapx<=ex; mapx++) {

            mn=mapmn(mapx,mapy);

            if (context_key_enabled() && mn==MAPDX*MAPDY/2) continue; // ignore player character if NOT clicked directly

            if (!(map[mn].rlight)) continue;
            if (!(map[mn].csprite)) continue;

            mtos(mapx,mapy,&scrx,&scry);

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
        conoff+=mousedy/LINEHEIGHT;
        mousedy=mousedy%LINEHEIGHT;
    } else conoff=ny;

    if (conoff<0) conoff=0;
    if (conoff>max_conoff) conoff=max_conoff;

    if (con_cnt) but[BUT_SCL_TR].y=but[BUT_SCL_UP].y+10+(but[BUT_SCL_DW].y-but[BUT_SCL_UP].y-20)*conoff/max(1,max_conoff);
}

int (*get_skltab_index)(int n)=_get_skltab_index;
DLL_EXPORT int _get_skltab_index(int n) {
    static int itab[V_MAX+1]={
        -1,
        0,1,2,                          // powers
        3,4,5,6,                        // bases
        7,8,9,10,38,41,                 // armor etc
        12,13,14,15,16,40,              // fight skills
        17,18,19,20,21,22,23,24,        // 2ndary fight skills
        28,29,30,31,32,33,34,11,39,     // spells
        25,26,27,35,36,37,              // misc skills
        42,                             // profession
        43,44,45,46,47,48,49,50,51,52,  // professions 1-10
        53,54,55,56,57,58,59,60,61,62,  // professions 11-20
        -2                              // end marker
    };

    return itab[n];
}

int (*get_skltab_sep)(int i)=_get_skltab_sep;
DLL_EXPORT int _get_skltab_sep(int i) {
    return (i==0 || i==3 || i==7 || i==12 || i==17 || i==25 || i==28 || i==42 || i==43);
}

int (*get_skltab_show)(int i)=_get_skltab_show;
DLL_EXPORT int _get_skltab_show(int i) {
    return (i==V_WEAPON || i==V_ARMOR || i==V_SPEED || i==V_LIGHT);
}

static void set_skltab(void) {
    int i,use,flag,n;
    int experience_left,raisecost;


    experience_left=experience-experience_used;

    for (flag=use=0,n=0; n<=V_MAX; n++) {

        i=get_skltab_index(n);
        if (i==-2) break;

        if (flag && get_skltab_sep(i)) {

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

        } else if (value[0][i] || value[1][i] || get_skltab_show(i)) {

            if (use==skltab_max) skltab=xrealloc(skltab,(skltab_max+=8)*sizeof(SKLTAB),MEM_GUI);

            if (value[1][i] && i!=V_DEMON && i!=V_COLD && i<V_PROFBASE) skltab[use].button=1;
            else skltab[use].button=0;

            skltab[use].v=i;

            strcpy(skltab[use].name,game_skill[i].name);
            skltab[use].base=value[1][i];
            skltab[use].curr=value[0][i];
            skltab[use].raisecost=raisecost=raise_cost(i,value[1][i]);

            if (experience_left>=0) {
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
        for (b=BUT_CON_BEG; b<=BUT_CON_END; b++) but[b].flags|=BUTF_NOHIT;
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
    if (nr>=0 && nr<=(*game_v_max)) {
        addline("%s: %s",game_skill[nr].name,game_skilldesc[nr]);
    } else addline("Unknown.");
}

// con_type: 1=grave or depot, 2=merchant
static void set_cmd_invsel(void) {
    if (context_key_enabled() && con_type==2 && con_cnt && csprite && invsel!=-1) {
        if (item[invsel]) lcmd=CMD_INV_SWAP;
        else lcmd=CMD_INV_DROP;
    } else if (context_key_enabled() && !con_cnt) {
        if (invsel==-1) return;
        if (item[invsel]) {
            if (csprite) lcmd=CMD_INV_SWAP;
            else lcmd=CMD_INV_TAKE;
        } else {
            if (csprite) lcmd=CMD_INV_DROP;
            else lcmd=CMD_INV_TAKE; // show anyway for people who want to click faster than the server responds
        }
    } else {
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
    }
}

void set_cmd_weasel(void) {
    if (context_key_enabled()) {
        if (weasel==-1) return;
        if (item[weatab[weasel]]) {
            if (csprite) lcmd=CMD_WEA_SWAP;
            else lcmd=CMD_WEA_TAKE;
        } else {
            if (csprite) lcmd=CMD_WEA_DROP;
            else lcmd=CMD_WEA_TAKE; // show anyway for people who want to click faster than the server responds
        }
    } else {
        if (weasel!=-1 && !vk_item && !vk_char && !csprite &&  item[weatab[weasel]]) lcmd=CMD_WEA_USE;
        if (weasel!=-1 && !vk_item && !vk_char && !csprite && !item[weatab[weasel]]) lcmd=CMD_WEA_USE;      // fake
        if (weasel!=-1 && !vk_item && !vk_char &&  csprite &&  item[weatab[weasel]]) lcmd=CMD_WEA_USE_WITH;
        if (weasel!=-1 && !vk_item && !vk_char &&  csprite && !item[weatab[weasel]]) lcmd=CMD_WEA_USE_WITH; // fake
        if (weasel!=-1 &&  vk_item && !vk_char && !csprite &&  item[weatab[weasel]]) lcmd=CMD_WEA_TAKE;
        if (weasel!=-1 &&  vk_item && !vk_char && !csprite && !item[weatab[weasel]]) lcmd=CMD_WEA_TAKE; // fake - slot is empty so i can't take
        if (weasel!=-1 &&  vk_item && !vk_char &&  csprite &&  item[weatab[weasel]]) lcmd=CMD_WEA_SWAP;
        if (weasel!=-1 &&  vk_item && !vk_char &&  csprite && !item[weatab[weasel]]) lcmd=CMD_WEA_DROP;
    }
}

void set_cmd_consel(void) {
    if (context_key_enabled()) {
        if (consel==-1 || !con_cnt) return;
        if (con_type==1) { // grave
            if (!csprite) lcmd=CMD_CON_FASTTAKE;
            else {
                if (container[consel]) lcmd=CMD_CON_SWAP;
                else lcmd=CMD_CON_DROP;
            }
        } else { // shop
            if (!csprite) lcmd=CMD_CON_FASTBUY;
            else lcmd=CMD_CON_SELL;
        }
    } else {
        if (consel!=-1 &&  vk_item && !vk_char && !csprite && con_type==1 && con_cnt &&  container[consel]) lcmd=CMD_CON_TAKE;
        if (consel!=-1 &&  vk_item && !vk_char && !csprite && con_type==1 && con_cnt && !container[consel]) lcmd=CMD_CON_TAKE;  // fake - slot is empty so i can't take (buy is also not possible)

        if (consel!=-1 && !vk_item && !vk_char && !csprite && con_type==1 && con_cnt &&  container[consel]) lcmd=CMD_CON_FASTTAKE;
        if (consel!=-1 && !vk_item && !vk_char && !csprite && con_type==1 && con_cnt && !container[consel]) lcmd=CMD_CON_FASTTAKE;  // fake

        if (consel!=-1 &&  vk_item && !vk_char && !csprite && con_type==2 && con_cnt) lcmd=CMD_CON_BUY;
        if (consel!=-1 && !vk_item && !vk_char && !csprite && con_type==2 && con_cnt) lcmd=CMD_CON_FASTBUY;

        if (consel!=-1 &&  vk_item && !vk_char &&  csprite && con_type==1 && con_cnt &&  container[consel]) lcmd=CMD_CON_SWAP;
        if (consel!=-1 &&  vk_item && !vk_char &&  csprite && con_type==1 && con_cnt && !container[consel]) lcmd=CMD_CON_DROP;
        if (consel!=-1 &&  vk_item && !vk_char &&  csprite && con_type==2 && con_cnt) lcmd=CMD_CON_SELL;
    }
}

static void set_cmd_states(void) {
    int i,c,x,y;
    static int oldconcnt=0; // ;-)
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
        strcpy(title,buf);
        sdl_set_title(title);
    }

    // update fkeyitem
    fkeyitem[0]=fkeyitem[1]=fkeyitem[2]=fkeyitem[3]=0;
    for (i=30; i<INVENTORYSIZE; i++) {
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
    butsel=mapsel=itmsel=chrsel=invsel=weasel=consel=sklsel=sklsel2=telsel=helpsel=colsel=skl_look_sel=questsel=actsel=-1;

    if ((display_help || display_quest) && mousex>=dotx(DOT_HLP) && mousex<=dotx(DOT_HL2)-40 && mousey>=doty(DOT_HLP) && mousey<=doty(DOT_HLP)+12)
        butsel=BUT_HELP_DRAG;

    if ((display_help || display_quest) && butsel==-1) {
        if (mousex>=dotx(DOT_HLP) && mousex<=dotx(DOT_HL2) && mousey>=doty(DOT_HLP) && mousey<=doty(DOT_HL2)) {
            butsel=BUT_HELP_MISC;

            if (display_help==1 && mousex>=dotx(DOT_HLP)+7 && mousex<=dotx(DOT_HLP)+136 && mousey>=198+doty(DOT_HLP) && mousey<=198+doty(DOT_HLP)+12*10) {
                helpsel=(mousey-(198+doty(DOT_HLP)))/10+2;
                if (mousex>dotx(DOT_HLP)+110) helpsel+=12;

                if (helpsel<2 || helpsel>MAXHELP) helpsel=-1;
            }

            if (display_quest && mousex>=dotx(DOT_HLP)+165 && mousex<=dotx(DOT_HLP)+199) {
                int tmp,y;

                tmp=(mousey-(doty(DOT_HLP)+16))/40;
                y=tmp*40+doty(DOT_HLP)+16;
                if (tmp>=0 && tmp<=8 && mousey>=y && mousey<=y+10) {
                    int qos=questonscreen[tmp];
                    if ((qos!=-1) && (game_questlog[qos].flags&QLF_REPEATABLE) && (quest[qos].flags&QF_DONE) && quest[qos].done<10) questsel=tmp;
                }
            }
        }
        if (mousex>=dotx(DOT_HLP)+177 && mousex<=dotx(DOT_HLP)+196 && mousey>=doty(DOT_HL2)-20 && mousey<=doty(DOT_HL2)-10) {
            butsel=BUT_HELP_PREV;
        }
        if (mousex>=dotx(DOT_HLP)+200 && mousex<=dotx(DOT_HLP)+219 && mousey>=doty(DOT_HL2)-20 && mousey<=doty(DOT_HL2)-10) {
            butsel=BUT_HELP_NEXT;
        }
        if (mousex>=dotx(DOT_HLP)+211 && mousex<=dotx(DOT_HLP)+224 && mousey>=doty(DOT_HLP)+2 && mousey<=doty(DOT_HLP)+12) {
            butsel=BUT_HELP_CLOSE;
        }
    }

    if (mousex>=dotx(DOT_TOP)+704 && mousex<=dotx(DOT_TOP)+739 && mousey>=doty(DOT_TOP)+22 && mousey<=doty(DOT_TOP)+30) butsel=BUT_HELP;
    if (mousex>=dotx(DOT_TOP)+741 && mousex<=dotx(DOT_TOP)+775 && mousey>=doty(DOT_TOP)+22 && mousey<=doty(DOT_TOP)+30) butsel=BUT_QUEST;
    if (mousex>=dotx(DOT_TOP)+704 && mousex<=dotx(DOT_TOP)+723 && mousey>=doty(DOT_TOP)+7 && mousey<=doty(DOT_TOP)+18) butsel=BUT_EXIT;

    // hit teleport?
    telsel=get_teleport(mousex,mousey);
    if (telsel!=-1) butsel=BUT_TEL;

    colsel=get_color(mousex,mousey);
    if (colsel!=-1) butsel=BUT_COLOR;

    if (teleporter && butsel==-1) {
        if (mousex>=dotx(DOT_TEL) && mousex<=dotx(DOT_TEL)+520 && mousey>=doty(DOT_TEL) && mousey<=doty(DOT_TEL)+320) {
            butsel=BUT_TEL_MISC;
        }
    }

    if (show_look && mousex>=dotx(DOT_LOK)+493 && mousex<=dotx(DOT_LOK)+500 && mousey>=doty(DOT_LOK)+3 && mousey<=doty(DOT_LOK)+10) butsel=BUT_NOLOOK;

    if (butsel==-1 && context_key_enabled()) {
        butsel=get_near_button(mousex,mousey);
        if (context_action_enabled()) {
            if (butsel>=BUT_ACT_BEG && butsel<=BUT_ACT_END && has_action_skill(butsel-BUT_ACT_BEG)) actsel=butsel-BUT_ACT_BEG;
            if (butsel==BUT_ACT_LCK || butsel==BUT_ACT_OPN) ;
            else butsel=-1;
        } else {
            if (butsel==BUT_ACT_OPN && mousey>buty(BUT_ACT_OPN)) ;
            else butsel=-1;
        }
    }

    // hit map
    if (!hitsel[0] && butsel==-1 && mousex>=dotx(DOT_MTL) && mousey>=doty(DOT_MTL) && doty(DOT_MBR) && mousey<doty(DOT_MBR)) {
        if (action_ovr==13) {
            itmsel=get_near_item(mousex,mousey,CMF_USE|CMF_TAKE,3);
            if (itmsel==-1) chrsel=get_near_char(mousex,mousey,3);
            if (itmsel==-1 && chrsel==-1) mapsel=get_near_ground(mousex,mousey);
        } else {
            if (vk_char || (action_ovr!=-1 && (action_ovr!=11 || csprite) && action_ovr!=2)) chrsel=get_near_char(mousex,mousey,vk_char?MAPDX:3);
            if (chrsel==-1 && (vk_item || action_ovr==11)) itmsel=get_near_item(mousex,mousey,CMF_USE|CMF_TAKE,csprite?0:MAPDX);
            if (chrsel==-1 && itmsel==-1 && !vk_char && (!vk_item || csprite)) mapsel=get_near_ground(mousex,mousey);

            if (mapsel!=-1 || itmsel!=-1 || chrsel!=-1)  butsel=BUT_MAP;
        }
    }

    // skill text lines for hover text
    if (!hitsel[0] && butsel==-1 && !con_cnt) {
        for (i=0; i<=BUT_SKL_END-BUT_SKL_BEG; i++) {
            x=butx(i+BUT_SKL_BEG);
            y=buty(i+BUT_SKL_BEG);
            if (mousex>x+10 && mousex<x+SKLWIDTH && mousey>y-5 && mousey<y+5) {
                sklsel2=i;
                break;
            }
        }
    }

    // buttons
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

    if (context_key_set_cmd()) ;
    else if (action_ovr!=-1) {
        if (action_ovr==0 && chrsel!=-1) lcmd=CMD_CHR_ATTACK;
        else if (action_ovr==1 && chrsel!=-1) lcmd=CMD_CHR_CAST_L;
        else if (action_ovr==2) lcmd=CMD_MAP_CAST_R;
        else if (action_ovr==11) {
            if (itmsel!=-1) {
                if (map[itmsel].flags&CMF_TAKE) {   // take needs to come first as dropped items can be usable
                    lcmd=CMD_ITM_TAKE;
                } else if (map[itmsel].flags&CMF_USE) {
                    if (csprite) lcmd=CMD_ITM_USE_WITH;
                    else lcmd=CMD_ITM_USE;
                }
            } else if (chrsel!=-1 && csprite) lcmd=CMD_CHR_GIVE;
            else if (mapsel!=-1 && csprite) lcmd=CMD_MAP_DROP;
        } else if (action_ovr==13) {
            if (itmsel!=-1) lcmd=CMD_ITM_LOOK;
            else if (chrsel!=-1) lcmd=CMD_CHR_LOOK;
            else if (mapsel!=-1) lcmd=CMD_MAP_LOOK;
        }
    } else {
        if (mapsel!=-1 && !vk_item && !vk_char) lcmd=CMD_MAP_MOVE;
        if (mapsel!=-1 &&  vk_item && !vk_char && csprite) lcmd=CMD_MAP_DROP;

        if (itmsel!=-1 &&  vk_item && !vk_char && !csprite && map[itmsel].flags&CMF_USE) lcmd=CMD_ITM_USE;
        if (itmsel!=-1 &&  vk_item && !vk_char && !csprite && map[itmsel].flags&CMF_TAKE) lcmd=CMD_ITM_TAKE;
        if (itmsel!=-1 &&  vk_item && !vk_char &&  csprite && map[itmsel].flags&CMF_USE) lcmd=CMD_ITM_USE_WITH;

        if (chrsel!=-1 && !vk_item &&  vk_char && !csprite) lcmd=CMD_CHR_ATTACK;
        if (chrsel!=-1 && !vk_item &&  vk_char &&  csprite) lcmd=CMD_CHR_GIVE;
    }

    set_cmd_invsel();
    set_cmd_weasel();
    set_cmd_consel();

    if (telsel!=-1) lcmd=CMD_TELEPORT;
    if (colsel!=-1) lcmd=CMD_COLOR;
    if (actsel!=-1) lcmd=CMD_ACTION;

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
        if ((vk_item || csprite) && butsel==BUT_JNK) lcmd=CMD_JUNK_ITEM;

        if (butsel>=BUT_MOD_WALK0 && butsel<=BUT_MOD_WALK2) lcmd=CMD_SPEED0+butsel-BUT_MOD_WALK0;

        if (butsel==BUT_HELP_MISC) lcmd=CMD_HELP_MISC;
        if (butsel==BUT_HELP_PREV) lcmd=CMD_HELP_PREV;
        if (butsel==BUT_HELP_NEXT) lcmd=CMD_HELP_NEXT;
        if (butsel==BUT_HELP_CLOSE) lcmd=CMD_HELP_CLOSE;
        if (butsel==BUT_HELP_DRAG) lcmd=CMD_HELP_DRAG;
        if (butsel==BUT_EXIT) lcmd=CMD_EXIT;
        if (butsel==BUT_HELP) lcmd=CMD_HELP;
        if (butsel==BUT_QUEST) lcmd=CMD_QUEST;
        if (butsel==BUT_NOLOOK) lcmd=CMD_NOLOOK;

        if (butsel==BUT_ACT_LCK) lcmd=CMD_ACTION_LOCK;
        if (butsel==BUT_ACT_OPN) lcmd=CMD_ACTION_OPEN;
        if (butsel==BUT_WEA_LCK) lcmd=CMD_WEAR_LOCK;
    }

    // set rcmd
    rcmd=CMD_NONE;
    if (action_ovr==-1) {
        skl_look_sel=get_skl_look(mousex,mousey);
        if (con_cnt==0 && skl_look_sel!=-1) rcmd=CMD_SKL_LOOK;
        else if (!vk_spell) {
            if (mapsel!=-1) rcmd=CMD_MAP_LOOK;
            if (itmsel!=-1) rcmd=CMD_ITM_LOOK;
            if (chrsel!=-1) rcmd=CMD_CHR_LOOK;
            if (context_key_enabled()) {
                if (invsel!=-1) rcmd=CMD_INV_USE;
                if (weasel!=-1) rcmd=CMD_WEA_USE;
            } else {
                if (invsel!=-1) rcmd=CMD_INV_LOOK;
                if (weasel!=-1) rcmd=CMD_WEA_LOOK;
                if (consel!=-1) rcmd=CMD_CON_LOOK;
            }
        } else {
            if (mapsel!=-1) rcmd=CMD_MAP_CAST_R;
            if (itmsel!=-1) rcmd=CMD_ITM_CAST_R;
            if (chrsel!=-1) rcmd=CMD_CHR_CAST_R;
        }
    } else rcmd=CMD_ACTION_CANCEL;

    if (gear_lock) { // gear lock resets cmds to none if on
        // no fast-equip from inventory
        if (invsel!=-1 && lcmd==CMD_INV_USE && !(item_flags[invsel]&IF_USE)) lcmd=CMD_NONE;
        if (invsel!=-1 && rcmd==CMD_INV_USE && !(item_flags[invsel]&IF_USE)) rcmd=CMD_NONE;

        // no fast-unequip from equipment
        if (weasel!=-1 && lcmd==CMD_WEA_USE && !(item_flags[weatab[weasel]]&IF_USE)) lcmd=CMD_NONE;
        if (weasel!=-1 && rcmd==CMD_WEA_USE && !(item_flags[weatab[weasel]]&IF_USE)) rcmd=CMD_NONE;

        // no take/swap/drop from equipment unless it is the left-hand-slot (for torches)
        if (weasel!=2 && (lcmd==CMD_WEA_TAKE || lcmd==CMD_WEA_DROP || lcmd==CMD_WEA_SWAP)) lcmd=CMD_NONE;
        if (weasel!=2 && (rcmd==CMD_WEA_TAKE || rcmd==CMD_WEA_DROP || rcmd==CMD_WEA_SWAP)) rcmd=CMD_NONE;
    }

    // set cursor
    if (vk_rbut) set_cmd_cursor(rcmd);
    else set_cmd_cursor(lcmd);
}

void help_drag(void) {
    int x,y;

    x=dot[DOT_HLP].x+mousedx;
    y=dot[DOT_HLP].y+mousedy;

    if (x<dotx(DOT_TL)) mousedx+=dotx(DOT_TL)-x;
    if (y<doty(DOT_TL)) mousedy+=doty(DOT_TL)-y;

    if (x>dotx(DOT_BR)+dotx(DOT_HLP)-dotx(DOT_HL2)) mousedx+=dotx(DOT_BR)+dotx(DOT_HLP)-dotx(DOT_HL2)-x;
    if (y>doty(DOT_BR)-20) mousedy+=doty(DOT_BR)-20-y;

    dot[DOT_HLP].x+=mousedx;
    dot[DOT_HLP].y+=mousedy;
    dot[DOT_HL2].x+=mousedx;
    dot[DOT_HL2].y+=mousedy;
    but[BUT_HELP_DRAG].x+=mousedx;
    but[BUT_HELP_DRAG].y+=mousedy;

    mousedx=mousedy=0;
}

static void cmd_action(void) {

    // nag the player to click the lock again
    if (!act_lck) addline("Please disable key-binding mode (the padlock to the left)!");

    switch (actsel) {
        case 0:
        case 1:
        case 2:
        case 11:
        case 13:    action_ovr=actsel; break;

        case 3:     cmd_some_spell(CL_FLASH,0,0,map[plrmn].cn); break;
        case 4:     cmd_some_spell(CL_FREEZE,0,0,map[plrmn].cn); break;
        case 5:     cmd_some_spell(CL_MAGICSHIELD,0,0,map[plrmn].cn); break;
        case 6:     cmd_some_spell(CL_BLESS,0,0,map[plrmn].cn); break;
        case 7:     cmd_some_spell(CL_HEAL,0,0,map[plrmn].cn); break;
        case 8:     cmd_some_spell(CL_WARCRY,0,0,map[plrmn].cn); break;
        case 9:     cmd_some_spell(CL_PULSE,0,0,map[plrmn].cn); break;
        case 10:    cmd_some_spell(CL_FIREBALL,0,0,map[plrmn].cn); break;
        case 12:    minimap_toggle(); break;
    }
}

static void exec_cmd(int cmd,int a) {
    action_ovr=-1;
    context_key_reset();

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
        case CMD_INV_LOOK:      cmd_look_inv(invsel); last_right_click_invsel=invsel; return;
        case CMD_WEA_LOOK:      cmd_look_inv(weatab[weasel]); last_right_click_invsel=weatab[weasel]; return;
        case CMD_CON_LOOK:      cmd_look_con(consel); last_right_click_invsel=INVENTORYSIZE+consel; return;

        case CMD_MAP_CAST_L:    cmd_some_spell(CL_FIREBALL,originx-MAPDX/2+mapsel%MAPDX,originy-MAPDY/2+mapsel/MAPDX,0); break;
        case CMD_ITM_CAST_L:    cmd_some_spell(CL_FIREBALL,originx-MAPDX/2+itmsel%MAPDX,originy-MAPDY/2+itmsel/MAPDX,0); break;
        case CMD_CHR_CAST_L:    cmd_some_spell(CL_FIREBALL,0,0,map[chrsel].cn); break;
        case CMD_MAP_CAST_R:    cmd_some_spell(CL_BALL,originx-MAPDX/2+mapsel%MAPDX,originy-MAPDY/2+mapsel/MAPDX,0); break;
        case CMD_ITM_CAST_R:    cmd_some_spell(CL_BALL,originx-MAPDX/2+itmsel%MAPDX,originy-MAPDY/2+itmsel/MAPDX,0); break;
        case CMD_CHR_CAST_R:    cmd_some_spell(CL_BALL,0,0,map[chrsel].cn); break;

        case CMD_SLF_CAST_K:	cmd_some_spell(a,0,0,map[plrmn].cn); break;
        case CMD_MAP_CAST_K:    cmd_some_spell(a,originx-MAPDX/2+mapsel%MAPDX,originy-MAPDY/2+mapsel/MAPDX,0); break;
        case CMD_CHR_CAST_K:    cmd_some_spell(a,0,0,map[chrsel].cn); break;

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

        case CMD_SAY_HITSEL:    cmd_add_text(hitsel,hittype); break;

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
        case CMD_HELP_DRAG:     help_drag(); return;
        case CMD_HELP:		if (display_help) display_help=0;
            else { display_help=1; display_quest=0; }
            return;
        case CMD_QUEST:		if (display_quest) display_quest=0;
            else { display_quest=1; display_help=0; }
            return;

        case CMD_EXIT:		quit=1; return;
        case CMD_NOLOOK:	show_look=0; return;

        case CMD_ACTION:    cmd_action(); return;
        case CMD_ACTION_CANCEL: return; // action gets cancelled on top
        case CMD_ACTION_LOCK:   display_action_lock(); return;
        case CMD_ACTION_OPEN:   display_action_open(); return;
        case CMD_WEAR_LOCK:     display_wear_lock(); return;
    }
    return;
}

void gui_insert(void) {
    char *text;

    text=SDL_GetClipboardText();

    cmd_add_text(text,0);

    SDL_free(text);
}

void gui_sdl_keyproc(int wparam) {
    int i;

    if (wparam!=SDLK_ESCAPE && wparam!=SDLK_F12 && amod_keydown(wparam)) return;

    switch (wparam) {

        case SDLK_ESCAPE:       cmd_stop(); context_stop(); show_look=0; display_gfx=0; teleporter=0; show_tutor=0; display_help=0;
                                display_quest=0; show_color=0; context_key_reset(); action_ovr=-1; minimap_hide();
                                if (context_key_enabled()) cmd_reset();
                                context_key_set(0);
                                return;
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

        case SDLK_F10:		    display_vc^=1; list_mem(); dd_list_text(); return;

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
        case SDLK_TAB:          cmd_proc(9); return;

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

        case 'm':               if (vk_shift && vk_control && !context_key_enabled()) minimap_toggle();
                                else goto spellbindkey;
                                return;

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
            if (!vk_item && !vk_char && !vk_spell) {
                context_keydown(wparam);
                return;
            }

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

        case '+':
        case '=':
            if (!context_key_isset()) context_action_enable(1);
            break;
        case '-':
            if (!context_key_isset()) context_action_enable(0);
            break;

        //case '<':               dd_sceweup(); break;

        case SDLK_INSERT:
            if (vk_shift && !vk_control && !vk_alt) gui_insert();
            break;
    }
}

void gui_sdl_mouseproc(int x,int y,int what,int clicks) {
    int delta,tmp;
    static int mdown=0;

    switch (what) {
        case SDL_MOUM_NONE:
            mousex=x;
            mousey=y;

            if (capbut!=-1) {
                if (mousex!=XRES/2 || mousey!=YRES/2) {
                    mousedx+=(mousex-(XRES/2))/sdl_scale;
                    mousedy+=(mousey-(YRES/2))/sdl_scale;
                    sdl_set_cursor_pos(XRES/2,YRES/2);
                }
            }

            mousex/=sdl_scale;
            mousey/=sdl_scale;
            mousex-=dd_offset_x();
            mousey-=dd_offset_y();

            if (butsel!=-1 && vk_lbut && (but[butsel].flags&BUTF_MOVEEXEC)) exec_cmd(lcmd,0);

            amod_mouse_move(mousex,mousey);
            break;

        case SDL_MOUM_LDOWN:
            vk_lbut=1;

            if (amod_mouse_click(mousex,mousey,what)) break;

            if (butsel!=-1 && capbut==-1 && (but[butsel].flags&BUTF_CAPTURE)) {
                amod_mouse_capture(1);
                sdl_show_cursor(0);
                sdl_capture_mouse(1);
                mousedx=0;
                mousedy=0;
                sdl_set_cursor_pos(XRES/2,YRES/2);
                capbut=butsel;
            }
            break;


        case SDL_MOUM_MUP:
            shift_override=0;
            control_override=0;
            mdown=0;
            if ((game_options&GO_WHEEL) &&special_tab[vk_special].spell) {
                if (special_tab[vk_special].target==TGT_MAP) exec_cmd(CMD_MAP_CAST_K,special_tab[vk_special].spell);
                else if (special_tab[vk_special].target==TGT_CHR) exec_cmd(CMD_CHR_CAST_K,special_tab[vk_special].spell);
                else if (special_tab[vk_special].target==TGT_SLF) exec_cmd(CMD_SLF_CAST_K,special_tab[vk_special].spell);
                break;
            }
            // fall through intended
        case SDL_MOUM_LUP:
            vk_lbut=0;

            if (amod_mouse_click(mousex,mousey,what)) break;
            if (context_click(mousex,mousey)) break;

            if (capbut!=-1) {
                sdl_set_cursor_pos((but[capbut].x+dd_offset_x())*sdl_scale,(but[capbut].y+dd_offset_y())*sdl_scale);
                sdl_capture_mouse(0);
                sdl_show_cursor(1);
                amod_mouse_capture(0);
                if (!(but[capbut].flags&BUTF_MOVEEXEC)) exec_cmd(lcmd,0);
                capbut=-1;
            } else {
                if ((tmp=context_key_click())!=CMD_NONE) exec_cmd(tmp,0);
                else exec_cmd(lcmd,0);
            }
            break;

        case SDL_MOUM_RDOWN:
            vk_rbut=1;
            if (amod_mouse_click(mousex,mousey,what)) break;
            context_stop();
            break;

        case SDL_MOUM_RUP:
            vk_rbut=0;
            if (amod_mouse_click(mousex,mousey,what)) break;
            if (rcmd==CMD_MAP_LOOK && context_open(mousex,mousey)) break;
            context_stop();
            exec_cmd(rcmd,0);
            break;

        case SDL_MOUM_WHEEL:
            delta=y;

            if (amod_mouse_click(0,delta,what)) break;

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

            if (game_options&GO_WHEEL) {
                while (delta>0) { vk_special_inc(); delta--; }
                while (delta<0) { vk_special_dec(); delta++; }
                vk_special_time=now;

                if (mdown) {
                    shift_override=special_tab[vk_special].shift_over;
                    control_override=special_tab[vk_special].control_over;
                }
            }
            break;

        case SDL_MOUM_MDOWN:
            if (game_options&GO_WHEEL) {
                shift_override=special_tab[vk_special].shift_over;
                control_override=special_tab[vk_special].control_over;
            } else shift_override=1;
            mdown=1;
            break;
    }
}

void init_colors(void) {
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
}

int main_init(void) {

    init_colors();
    init_dots();

    set_invoff(0,0);
    set_skloff(0,0);
    set_conoff(0,0);

    init_game(dotx(DOT_MCT),doty(DOT_MCT));

    minimap_init();

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
        if (!sdl_is_shown() || !sdl_pre_do(tick)) SDL_Delay(1);
        tnow=SDL_GetTicks();
    } while (t>tnow);

    if (sdl_is_shown())
        sdl_render();
}

int nextframe,nexttick;
uint64_t gui_time_network=0;
uint64_t gui_frametime=0;
uint64_t gui_ticktime=0;

int calc_tick_delay_short(int size) {
    int tmp;
    switch (size) {
        case 0:     tmp=MPT*2.00; break;
        case 1:     tmp=MPT*1.25; break;
        case 2:     tmp=MPT*1.10; break;
        case 3:     tmp=MPT; break; // optimal
        case 4:     tmp=MPT-1; break;
        case 5:     tmp=MPT-1; break;
        case 6:     tmp=MPT*0.90; break;
        case 7:     tmp=MPT*0.75; break;
        case 8:     tmp=MPT*0.60; break;
        case 9:     tmp=MPT*0.50; break;
        default:    tmp=MPT*0.25; break;
    }
    return tmp;
}

int calc_tick_delay_normal(int size) {
    int tmp;
    switch (size) {
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
    return tmp;
}

int main_loop(void) {
    void prefetch_game(int attick);
    int tmp,timediff,ltick=0,attick;
    long long start;
    int do_one_tick=1;
    uint64_t gui_last_frame=0,gui_last_tick=0;

    amod_gamestart();

    nexttick=SDL_GetTicks()+MPT;
    nextframe=SDL_GetTicks()+MPF;

    while (!quit) {
        now=SDL_GetTicks();

        start=SDL_GetTicks64();
        poll_network();

        // synchronise frames and ticks if at the same speed
        if (sockstate==4 && MPF==MPT) nextframe=nexttick;

        // check if we can go on
        if (sockstate>2) {

            // decode as many ticks as we can
            // and add their contents to the prefetch queue
            while ((attick=next_tick()))
                if (!(attick&3) || !game_slowdown) prefetch_game(attick);

            // get one tick to display?
            timediff=nexttick-SDL_GetTicks();
            if (timediff<0 || nexttick<=nextframe) {  // do ticks when they are due, or before the corresponding frame is shown
                do_one_tick=1;
                gui_ticktime=SDL_GetTicks64()-gui_last_tick;
                gui_last_tick=SDL_GetTicks64();
                do_tick();
                ltick++;

                if (sockstate==4 && ltick%TICKS==0) {
                    cl_ticker();
                }
                amod_tick();
                #ifdef ENABLE_SHAREDMEM
                sharedmem_update();
                #endif
            }
        }

        if (sockstate==4) timediff=nextframe-SDL_GetTicks();
        else timediff=1;
        gui_time_network+=SDL_GetTicks64()-start;

        if (timediff>-MPF/2) {
#ifdef TICKPRINT
            printf("Display tick %d\n",tick);
#endif
            gui_frametime=SDL_GetTicks64()-gui_last_frame;
            gui_last_frame=SDL_GetTicks64();

            if (sdl_is_shown() && (!(tick&3) || !game_slowdown || sockstate!=4)) {
                sdl_clear();
                display();
                amod_frame();
                display_mouseover();
                minimap_update();
            }

            timediff=nextframe-SDL_GetTicks();
            if (timediff>0) idle+=timediff;
            else skip-=timediff;

            frames++;

            flip_at(nextframe);
        } else {
#ifdef TICKPRINT
            printf("Skip tick %d\n",tick);
#endif
            skip-=timediff;

            sdl_loop();
        }

        if (do_one_tick) {
            if (game_options&GO_SHORT) tmp=calc_tick_delay_short(lasttick+q_size);
            else tmp=calc_tick_delay_normal(lasttick+q_size);
            nexttick+=tmp;
            tota+=tmp;
            if (tick%24==0) { tota/=2; skip/=2; idle/=2; frames/=2; }

            do_one_tick=0;
        }

        nextframe+=MPF;

        // try to sync frame to tick?
        if (abs(nexttick-nextframe)<MPF/2) {
            nextframe=nexttick;
        }
    }

    close_client();

    return 0;
}

