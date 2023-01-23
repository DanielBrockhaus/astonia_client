/*
 * Part of Astonia Client (c) Daniel Brockhaus. Please read license.txt.
 *
 * Display Color Picker Window
 *
 * Display the color picker window, and maps mouse clicks
 */

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "../astonia.h"
#include "../gui.h"
#include "../gui/_gui.h"
#include "../client.h"
#include "../game.h"

int show_color=0,show_cur=0;
int show_color_c[3]={1,1,1};
int show_cx=0;

void display_color(void) {
    int csprite,scale,cr,cg,cb,light,sat,c1,c2,c3,shine;
    static int col_anim=4,col_step=0,col_dir=0;
    DDFX fx;

    if (!show_color) return;

    dd_copysprite(51082,dotx(DOT_COL),doty(DOT_COL),14,0);

    if (show_cur==0) dx_copysprite_emerald(dotx(DOT_COL)-38,doty(DOT_COL)+40,2,2);
    else dx_copysprite_emerald(dotx(DOT_COL)-38,doty(DOT_COL)+40,2,1);
    if (show_cur==1) dx_copysprite_emerald(dotx(DOT_COL)-38+12,doty(DOT_COL)+40,2,2);
    else dx_copysprite_emerald(dotx(DOT_COL)-38+12,doty(DOT_COL)+40,2,1);
    if (show_cur==2) dx_copysprite_emerald(dotx(DOT_COL)-38+24,doty(DOT_COL)+40,2,2);
    else dx_copysprite_emerald(dotx(DOT_COL)-38+24,doty(DOT_COL)+40,2,1);

    dd_copysprite(51083,dotx(DOT_COL)-55,doty(DOT_COL)-50+64-IGET_R(show_color_c[show_cur])*2,14,0);
    dd_copysprite(51083,dotx(DOT_COL)-55+20,doty(DOT_COL)-50+64-IGET_G(show_color_c[show_cur])*2,14,0);
    dd_copysprite(51083,dotx(DOT_COL)-55+40,doty(DOT_COL)-50+64-IGET_B(show_color_c[show_cur])*2,14,0);

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
    dd_copysprite_fx(&fx,dotx(DOT_COL)+30,doty(DOT_COL));
}

int get_color(int x,int y) {
    if (!show_color) return -1;

    if (abs(x-dotx(DOT_COL)+38)<4 && abs(y-doty(DOT_COL)-39)<4) return 1;
    if (abs(x-dotx(DOT_COL)+26)<4 && abs(y-doty(DOT_COL)-39)<4) return 2;
    if (abs(x-dotx(DOT_COL)+14)<4 && abs(y-doty(DOT_COL)-39)<4) return 3;

    if (abs(x-dotx(DOT_COL)+50)<11 && abs(y-doty(DOT_COL)+18)<34) { show_cx=64-(y-doty(DOT_COL)+50); return 4; }
    if (abs(x-dotx(DOT_COL)+30)<11 && abs(y-doty(DOT_COL)+18)<34) { show_cx=64-(y-doty(DOT_COL)+50); return 5; }
    if (abs(x-dotx(DOT_COL)+10)<11 && abs(y-doty(DOT_COL)+18)<34) { show_cx=64-(y-doty(DOT_COL)+50); return 6; }

    if (abs(x-dotx(DOT_COL)-43)<10 && abs(y-doty(DOT_COL)-39)<5) return 7;
    if (abs(x-dotx(DOT_COL)-19)<10 && abs(y-doty(DOT_COL)-39)<5) return 8;

    if (x-dotx(DOT_COL)<-60 || x-dotx(DOT_COL)>60 || y-doty(DOT_COL)<-60 || y-doty(DOT_COL)>60) return -1;

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

