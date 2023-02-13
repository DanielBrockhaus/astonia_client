/*
 * Part of Astonia Client (c) Daniel Brockhaus. Please read license.txt.
 *
 * Context Menu
 *
 * Displays the context menu on right click and handles mouse clicks
 * in the menu.
 *
 */

#include <time.h>
#include <SDL2/SDL.h>

#include "../../src/astonia.h"
#include "../../src/gui.h"
#include "../../src/gui/_gui.h"
#include "../../src/client.h"
#include "../../src/game.h"
#include "../../src/sdl.h"
#include "../../src/modder.h"

static int c_on=0,c_x,c_y,d_y,csel,isel,msel,ori_x,ori_y;

#define MAXLINE     20
#define MAXLEN      120
#define MENUWIDTH   100
#define MENUHEIGHT  (8*10+8)

struct menu {
    int linecnt;
    char line[MAXLINE][MAXLEN];
    int cmd[MAXLINE];
    int opt1[MAXLINE],opt2[MAXLINE];
};
struct menu menu;

static void update_ori(void) {
    int x,y;

    if (msel!=-1) {
        x=(msel%MAPDX)+ori_x-originx;
        y=(msel/MAPDX)+ori_y-originy;
        if (x<0 || x>=MAPDX || y<0 || y>=MAPDX) msel=-1;
        else msel=x+y*MAPDX;
    }

    if (isel!=-1) {
        x=(isel%MAPDX)+ori_x-originx;
        y=(isel/MAPDX)+ori_y-originy;
        if (x<0 || x>=MAPDX || y<0 || y>=MAPDX) isel=-1;
        else isel=x+y*MAPDX;
    }

    if (csel!=-1) {
        x=(csel%MAPDX)+ori_x-originx;
        y=(csel/MAPDX)+ori_y-originy;
        if (x<0 || x>=MAPDX || y<0 || y>=MAPDX) csel=-1;
        else csel=x+y*MAPDX;
    }

    ori_x=originx;
    ori_y=originy;
}

static void makemenu(void) {
    int co;
    char *name="someone";

    update_ori();

    if (csel!=-1) {
        co=map[csel].cn;
        if (co>0 && co<MAXCHARS) {
            if (player[co].name[0]) name=player[co].name;
        } else csel=-1;
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
        menu.opt1[menu.linecnt]=originx-MAPDX/2+isel%MAPDX;
        menu.opt2[menu.linecnt]=originy-MAPDY/2+isel/MAPDX;
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

    if (!(game_options&GO_CONTEXT)) return 0;

    csel=get_near_char(mx,my,3);
    isel=get_near_item(mx,my,CMF_USE|CMF_TAKE,3);
    msel=get_near_ground(mx,my);
    ori_x=originx;
    ori_y=originy;

    makemenu();
    if (menu.linecnt==1) {
        cmd_look_map(ori_x-MAPDX/2+msel%MAPDX,ori_y-MAPDY/2+msel/MAPDX);
        return 1;
    }

    c_on=1;
    c_x=mx;
    c_y=my-10;
    d_y=8;

    if (c_x<dotx(DOT_MTL)+10) c_x=dotx(DOT_MTL)+10;
    if (c_x>dotx(DOT_MBR)-MENUWIDTH-10) c_x=dotx(DOT_MBR)-MENUWIDTH-10;
    if (c_y<doty(DOT_MTL)+10) c_y=doty(DOT_MTL)+10;
    if (c_y>doty(DOT_MBR)-MENUHEIGHT-10) c_y=doty(DOT_MBR)-MENUHEIGHT-10;

    return 1;
}

int context_getnm(void) {
    if (!(game_options&GO_CONTEXT)) return -1;
    update_ori();

    if (c_on) return msel;
    else return -1;
}

void context_stop(void) {
    c_on=0;
}
void context_display(int mx,int my) {
    int x,y,n;

    if ((game_options&GO_CONTEXT) && c_on) {
        makemenu();

        d_y=menu.linecnt*10+8;

        dd_shaded_rect(c_x,c_y,c_x+MENUWIDTH,c_y+d_y,0x0000,95);
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

    if ((game_options&GO_CONTEXT) && c_on) {
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

    if (!(game_options&GO_ACTION)) return 0;

    if (key=='#' || key==CMD_UP || key==CMD_DOWN || key==9 || key=='/') {
        keymode=1;
    } else if (key==CMD_RETURN) {
        if (keymode==1) {
            keymode=0;
            return 0;
        }
        keymode=1;
    }
    if (keymode) return 0;

    return 1;
}

static int lcmd_override=CMD_NONE;

void context_key_reset(void) {
    lcmd_override=CMD_NONE;
}

void context_keydown(int key) {

    if (!(game_options&GO_ACTION)) return;
    if (keymode) return;

    // ignore key-down while over action bar
    if (actsel!=-1) return;

    switch (action_key2slot(key)) {
        case 0:     lcmd_override=CMD_CHR_ATTACK; break;
        case 1:     lcmd_override=CMD_CHR_CAST_L; break;
        case 2:     lcmd_override=CMD_CHR_CAST_R; break;
        case 6:
        case 7:     lcmd_override=CMD_CHR_CAST_K; break;
        case 11:    lcmd_override=CMD_ITM_USE; break;

        case 101:   lcmd_override=CMD_MAP_CAST_L; break;
        case 102:   lcmd_override=CMD_MAP_CAST_R; break;
        case 103:
        case 104:
        case 105:
        case 106:
        case 107:
        case 108:
        case 109:
        case 110:   lcmd_override=CMD_SLF_CAST_K; break;
    }
}

int context_key_set_cmd(void) {
    if (!(game_options&GO_ACTION)) return 0;
    if (keymode) return 0;
    if (lcmd_override==CMD_NONE) return 0;

    switch (lcmd_override) {
        case CMD_CHR_ATTACK:
        case CMD_CHR_CAST_L:
        case CMD_CHR_CAST_R:
        case CMD_CHR_CAST_K:    chrsel=get_near_char(mousex,mousey,3); break;
        case CMD_MAP_CAST_L:
        case CMD_MAP_CAST_R:    mapsel=get_near_ground(mousex,mousey); break;

        case CMD_ITM_USE:
        case CMD_ITM_USE_WITH:
        case CMD_ITM_TAKE:
        case CMD_CHR_GIVE:
        case CMD_MAP_DROP:
            chrsel=get_near_char(mousex,mousey,3);
            itmsel=get_near_item(mousex,mousey,CMF_TAKE|CMF_USE,csprite?0:3);
            mapsel=get_near_ground(mousex,mousey);
            if (csprite) {
                if (chrsel!=-1) {
                    itmsel=-1;
                    lcmd_override=CMD_CHR_GIVE;
                } else if (itmsel!=-1) {
                    if (map[itmsel].flags&CMF_USE) lcmd_override=CMD_ITM_USE_WITH;
                    else itmsel=-1;
                } else if (mapsel!=-1) lcmd_override=CMD_MAP_DROP;
            } else {
                if (itmsel!=-1) {
                    if (map[itmsel].flags&CMF_TAKE) lcmd_override=CMD_ITM_TAKE;
                    else if (map[itmsel].flags&CMF_USE) lcmd_override=CMD_ITM_USE;
                    else itmsel=-1;
                }
                chrsel=-1;
            }
            break;
    }
    lcmd=lcmd_override;

    return 1;
}

void context_keyup(int key) {
    int csel,isel,msel;

    lcmd_override=CMD_NONE;

    if (!(game_options&GO_ACTION)) return;
    if (keymode) return;

    if (actsel!=-1) {
        action_set_key(actsel,key);
        return;
    }

    if (mousex>=dotx(DOT_MTL) && mousey>=doty(DOT_MTL) && mousex<dotx(DOT_MBR) && mousey<doty(DOT_MBR)) {
        csel=get_near_char(mousex,mousey,3);
        isel=get_near_item(mousex,mousey,CMF_USE|CMF_TAKE,csprite?0:3);
        msel=get_near_ground(mousex,mousey);
    } else csel=isel=msel=-1;

    switch (action_key2slot(key)) {
        case 0:     if (csel!=-1) cmd_kill(map[csel].cn); break;
        case 1:     if (csel!=-1) cmd_some_spell(CL_FIREBALL,0,0,map[csel].cn); break;
        case 2:     if (csel!=-1) cmd_some_spell(CL_BALL,0,0,map[csel].cn); break;
        case 6:     if (csel!=-1) cmd_some_spell(CL_BLESS,0,0,map[csel].cn); break;
        case 7:     if (csel!=-1) cmd_some_spell(CL_HEAL,0,0,map[csel].cn); break;
        case 11:
            if (csprite) {
                if (csel!=-1) cmd_give(map[csel].cn);
                else if (isel!=-1 && (map[isel].flags&CMF_USE)) cmd_use(originx-MAPDX/2+isel%MAPDX,originy-MAPDY/2+isel/MAPDX);
                else if (msel!=-1) cmd_drop(originx-MAPDX/2+msel%MAPDX,originy-MAPDY/2+msel/MAPDX);
            } else if (isel!=-1) {
                if (map[isel].flags&CMF_TAKE) cmd_take(originx-MAPDX/2+isel%MAPDX,originy-MAPDY/2+isel/MAPDX);
                else if (map[isel].flags&CMF_USE) cmd_use(originx-MAPDX/2+isel%MAPDX,originy-MAPDY/2+isel/MAPDX);
            }
            break;

        case 101:   if (msel!=-1) cmd_some_spell(CL_FIREBALL,originx-MAPDX/2+msel%MAPDX,originy-MAPDY/2+msel/MAPDX,0); break;
        case 102:   if (msel!=-1) cmd_some_spell(CL_BALL,originx-MAPDX/2+msel%MAPDX,originy-MAPDY/2+msel/MAPDX,0); break;
        case 103:   cmd_some_spell(CL_FLASH,0,0,map[plrmn].cn); break;
        case 104:   cmd_some_spell(CL_FREEZE,0,0,map[plrmn].cn); break;
        case 105:   cmd_some_spell(CL_MAGICSHIELD,0,0,map[plrmn].cn); break;
        case 106:   cmd_some_spell(CL_BLESS,0,0,map[plrmn].cn); break;
        case 107:   cmd_some_spell(CL_HEAL,0,0,map[plrmn].cn); break;
        case 108:   cmd_some_spell(CL_WARCRY,0,0,map[plrmn].cn); break;
        case 109:   cmd_some_spell(CL_PULSE,0,0,map[plrmn].cn); break;
        case 110:   cmd_some_spell(CL_FIREBALL,0,0,map[plrmn].cn); break;
    }
}

int context_key_set(int onoff) {
    int old;
    if (!(game_options&GO_ACTION)) return 1;
    old=keymode;
    keymode=onoff;
    return old;
}

int context_key_isset(void) {
    if (!(game_options&GO_ACTION)) return 1;
    return keymode;
}

int context_key_enabled(void) {
    return(game_options&GO_ACTION);
}

void context_action_enable(int onoff) {
    action_enabled=onoff;
    save_options();
}
int context_action_enabled(void) {
    return(game_options&GO_ACTION) && action_enabled;
}
