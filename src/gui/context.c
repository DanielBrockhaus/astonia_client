/*
 * Part of Astonia Client (c) Daniel Brockhaus. Please read license.txt.
 *
 * Context Menu
 *
 * Displays the context menu on right click and handles mouse clicks
 * in the menu.
 *
 */

#include <windows.h>
#include <psapi.h>
#include <time.h>
#include <SDL2/SDL.h>

#include "../../src/astonia.h"
#include "../../src/gui.h"
#include "../../src/gui/_gui.h"
#include "../../src/client.h"
#include "../../src/game.h"
#include "../../src/sdl.h"
#include "../../src/modder.h"

int context_enabled=3;

static int c_on=0,c_x,c_y,d_y;
static int csel,isel,msel;

#define MAXLINE     20
#define MAXLEN      120
#define MENUWIDTH   100

struct menu {
    int linecnt;
    char line[MAXLINE][MAXLEN];
    int cmd[MAXLINE];
    int opt1[MAXLINE],opt2[MAXLINE];
};
struct menu menu;

static void makemenu(void) {
    int co;
    char *name="someone";

    if (csel!=-1) {
        co=map[csel].cn;
        if (co>=0 && co<MAXCHARS) {
            if (player[co].name[0]) name=player[co].name;
        }
    }

    menu.linecnt=0;

#if 0
    if (csel!=MAPDX*MAPDY/2) {
        sprintf(menu.line[menu.linecnt],"Walk");
        menu.cmd[menu.linecnt]=CMD_MAP_MOVE;
        menu.opt1[menu.linecnt]=originx-MAPDX/2+msel%MAPDX;
        menu.opt2[menu.linecnt]=originy-MAPDY/2+msel/MAPDX;
        menu.linecnt++;
    }
#endif

    if (isel!=-1) {
        if (map[isel].flags&CMF_TAKE) {
            sprintf(menu.line[menu.linecnt],"Take Item");
            menu.cmd[menu.linecnt]=CMD_ITM_TAKE;
            menu.opt1[menu.linecnt]=originx-MAPDX/2+isel%MAPDX;
            menu.opt2[menu.linecnt]=originy-MAPDY/2+isel/MAPDX;
            menu.linecnt++;
        } else if (map[isel].flags&CMF_USE) {
            if (csprite) sprintf(menu.line[menu.linecnt],"Use Item with");
            else sprintf(menu.line[menu.linecnt],"Use Item");
            menu.cmd[menu.linecnt]=CMD_ITM_USE;
            menu.opt1[menu.linecnt]=originx-MAPDX/2+isel%MAPDX;
            menu.opt2[menu.linecnt]=originy-MAPDY/2+isel/MAPDX;
            menu.linecnt++;
        }
    }
    if (csprite && !map[msel].isprite) {
        sprintf(menu.line[menu.linecnt],"Drop");
        menu.cmd[menu.linecnt]=CMD_MAP_DROP;
        menu.opt1[menu.linecnt]=originx-MAPDX/2+msel%MAPDX;
        menu.opt2[menu.linecnt]=originy-MAPDY/2+msel/MAPDX;
        menu.linecnt++;
    }

    if (csel!=-1) {
        if (csprite) {
            sprintf(menu.line[menu.linecnt],"Give to %s",name);
            menu.cmd[menu.linecnt]=CMD_CHR_GIVE;
            menu.opt1[menu.linecnt]=map[csel].cn;
            menu.opt2[menu.linecnt]=0;
            menu.linecnt++;
        }
        if (csel!=MAPDX*MAPDY/2) {
            sprintf(menu.line[menu.linecnt],"Attack %s",name);
            menu.cmd[menu.linecnt]=CMD_CHR_ATTACK;
            menu.opt1[menu.linecnt]=map[csel].cn;;
            menu.opt2[menu.linecnt]=0;
            menu.linecnt++;
        }

        if (csel==MAPDX*MAPDY/2) {
            if (value[0][V_FLASH]) {
                sprintf(menu.line[menu.linecnt],"Cast Flash");
                menu.cmd[menu.linecnt]=CMD_CHR_CAST_K;
                menu.opt1[menu.linecnt]=0;
                menu.opt2[menu.linecnt]=CL_FLASH;
                menu.linecnt++;
            }

            if (value[0][V_FREEZE]) {
                sprintf(menu.line[menu.linecnt],"Cast Freeze");
                menu.cmd[menu.linecnt]=CMD_CHR_CAST_K;
                menu.opt1[menu.linecnt]=0;
                menu.opt2[menu.linecnt]=CL_FREEZE;
                menu.linecnt++;
            }

            if (value[0][V_PULSE]) {
                sprintf(menu.line[menu.linecnt],"Cast Pulse");
                menu.cmd[menu.linecnt]=CMD_CHR_CAST_K;
                menu.opt1[menu.linecnt]=0;
                menu.opt2[menu.linecnt]=CL_PULSE;
                menu.linecnt++;
            }

            if (value[0][V_WARCRY]) {
                sprintf(menu.line[menu.linecnt],"Warcry");
                menu.cmd[menu.linecnt]=CMD_CHR_CAST_K;
                menu.opt1[menu.linecnt]=0;
                menu.opt2[menu.linecnt]=CL_WARCRY;
                menu.linecnt++;
            }

            if (value[0][V_MAGICSHIELD]) {
                sprintf(menu.line[menu.linecnt],"Cast Magic Shield");
                menu.cmd[menu.linecnt]=CMD_CHR_CAST_K;
                menu.opt1[menu.linecnt]=0;
                menu.opt2[menu.linecnt]=CL_MAGICSHIELD;
                menu.linecnt++;
            }

            if (value[0][V_HEAL]) {
                sprintf(menu.line[menu.linecnt],"Cast Heal");
                menu.cmd[menu.linecnt]=CMD_CHR_CAST_K;
                menu.opt1[menu.linecnt]=map[csel].cn;;
                menu.opt2[menu.linecnt]=CL_HEAL;
                menu.linecnt++;
            }

            if (value[0][V_BLESS]) {
                sprintf(menu.line[menu.linecnt],"Cast Bless");
                menu.cmd[menu.linecnt]=CMD_CHR_CAST_K;
                menu.opt1[menu.linecnt]=map[csel].cn;;
                menu.opt2[menu.linecnt]=CL_BLESS;
                menu.linecnt++;
            }
        } else {
            if (value[0][V_FIREBALL]) {
                sprintf(menu.line[menu.linecnt],"Fireball %s",name);
                menu.cmd[menu.linecnt]=CMD_CHR_CAST_K;
                menu.opt1[menu.linecnt]=map[csel].cn;
                menu.opt2[menu.linecnt]=CL_FIREBALL;
                menu.linecnt++;
            }

            if (value[0][V_FLASH]) {
                sprintf(menu.line[menu.linecnt],"L'ball %s",name);
                menu.cmd[menu.linecnt]=CMD_CHR_CAST_K;
                menu.opt1[menu.linecnt]=map[csel].cn;
                menu.opt2[menu.linecnt]=CL_BALL;
                menu.linecnt++;
            }

            if (value[0][V_HEAL]) {
                sprintf(menu.line[menu.linecnt],"Heal %s",name);
                menu.cmd[menu.linecnt]=CMD_CHR_CAST_K;
                menu.opt1[menu.linecnt]=map[csel].cn;;
                menu.opt2[menu.linecnt]=CL_HEAL;
                menu.linecnt++;
            }

            if (value[0][V_BLESS]) {
                sprintf(menu.line[menu.linecnt],"Bless %s",name);
                menu.cmd[menu.linecnt]=CMD_CHR_CAST_K;
                menu.opt1[menu.linecnt]=map[csel].cn;;
                menu.opt2[menu.linecnt]=CL_BLESS;
                menu.linecnt++;
            }
        }
    }

    sprintf(menu.line[menu.linecnt],"Examine ground");
    menu.cmd[menu.linecnt]=CMD_MAP_LOOK;
    menu.opt1[menu.linecnt]=originx-MAPDX/2+msel%MAPDX;
    menu.opt2[menu.linecnt]=originy-MAPDY/2+msel/MAPDX;
    menu.linecnt++;
    if (isel!=-1) {
        sprintf(menu.line[menu.linecnt],"Examine item");
        menu.cmd[menu.linecnt]=CMD_ITM_LOOK;
        menu.opt1[menu.linecnt]=originx-MAPDX/2+msel%MAPDX;
        menu.opt2[menu.linecnt]=originy-MAPDY/2+msel/MAPDX;
        menu.linecnt++;
    }
    if (csel!=-1) {
        sprintf(menu.line[menu.linecnt],"Inspect %s",name);
        menu.cmd[menu.linecnt]=CMD_CHR_LOOK;
        menu.opt1[menu.linecnt]=map[csel].cn;;
        menu.opt2[menu.linecnt]=0;
        menu.linecnt++;
    }
}

int context_open(int mx,int my) {

    if (!(context_enabled&1)) return 0;

    csel=get_near_char(mx,my,3);
    isel=get_near_item(mx,my,CMF_USE|CMF_TAKE,3);
    msel=get_near_ground(mx,my);
    note("csel=%d, isel=%d, msel=%d",csel,isel,msel);

    c_on=1;
    c_x=mx;
    c_y=my-10;
    d_y=8;

    if (c_x<dotx(DOT_MTL)+10) c_x=dotx(DOT_MTL)+10;
    if (c_x>dotx(DOT_MBR)-MENUWIDTH-10) c_x=dotx(DOT_MBR)-MENUWIDTH-10;
    if (c_y<doty(DOT_MTL)+10) c_y=doty(DOT_MTL)+10;
    if (c_y>doty(DOT_MBR)-MENUWIDTH-10) c_y=doty(DOT_MBR)-MENUWIDTH-10;

    return 1;
}

int context_getnm(void) {
    if (!(context_enabled&1)) return -1;

    if (c_on) return msel;
    else return -1;
}

void context_stop(void) {
    c_on=0;
}
void context_display(int mx,int my) {
    int x,y,n;

    if ((context_enabled&1) && c_on) {
        makemenu();

        d_y=menu.linecnt*10+8;

        dd_shaded_rect(c_x,c_y,c_x+MENUWIDTH,c_y+d_y,0x0000);
        x=c_x+4;
        y=c_y+4;

        for (n=0; n<menu.linecnt; n++) {
            if (mousex>c_x && mousex<c_x+MENUWIDTH && mousey>=c_y+n*10+4 && mousey<c_y+n*10+14) dd_drawtext(x,y,whitecolor,DD_LEFT,menu.line[n]);
            else dd_drawtext(x,y,graycolor,DD_LEFT,menu.line[n]);
            y+=10;
        }
    }
}

int context_click(int mx,int my) {
    int n;

    if ((context_enabled&1) && c_on) {
        c_on=0;

        if (mx>c_x && mx<c_x+MENUWIDTH && my>=c_y && my<c_y+menu.linecnt*10+8) {
            n=(my-c_y-4)/10;
            if (n<0) n=0;
            if (n>=menu.linecnt) n=menu.linecnt-1;
            switch (menu.cmd[n]) {
                case CMD_MAP_MOVE:      cmd_move(menu.opt1[n],menu.opt2[n]); break;
                case CMD_MAP_DROP:      cmd_drop(menu.opt1[n],menu.opt2[n]); break;
                case CMD_MAP_LOOK:      cmd_look_map(menu.opt1[n],menu.opt2[n]); break;
                case CMD_ITM_TAKE:      cmd_take(menu.opt1[n],menu.opt2[n]); break;
                case CMD_ITM_USE:       cmd_use(menu.opt1[n],menu.opt2[n]); break;
                case CMD_ITM_LOOK:      cmd_look_item(menu.opt1[n],menu.opt2[n]); break;
                case CMD_CHR_ATTACK:    cmd_kill(menu.opt1[n]); break;
                case CMD_CHR_GIVE:      cmd_give(menu.opt1[n]); break;
                case CMD_CHR_LOOK:      cmd_look_char(menu.opt1[n]); break;
                case CMD_CHR_CAST_K:    cmd_some_spell(menu.opt2[n],0,0,menu.opt1[n]); break;
            }
            return 1;
        }
    }
    return 0;
}

static int keymode=0;
int context_key(int key) {
    int isel,csel;

    if (!(context_enabled&2)) return 0;

    if (key==CMD_RETURN) {
        if (keymode==1) {
            keymode=0;
            return 0;
        }
        keymode=1;
    }
    if (keymode) return 0;

    csel=get_near_char(mousex,mousey,3);
    isel=get_near_item(mousex,mousey,CMF_USE|CMF_TAKE,3);

    switch (key) {
        case 'a':   cmd_some_spell(CL_FLASH,0,0,0); break;
        case 's':   cmd_some_spell(CL_MAGICSHIELD,0,0,0); break;
        case 'd':   if (isel!=-1) cmd_use(originx-MAPDX/2+isel%MAPDX,originy-MAPDY/2+isel/MAPDX);
                    break;
        case 'y':   if (csel!=-1) cmd_kill(map[csel].cn);
                    break;


    }
    return 1;
}

int context_key_set(int onoff) {
    int old;
    if (!(context_enabled&2)) return 1;
    old=keymode;
    keymode=onoff;
    return old;
}

int context_key_isset(void) {
    if (!(context_enabled&2)) return 1;
    return keymode;
}

int context_key_enabled(void) {
    return(context_enabled&2);
}

