/*
 * Part of Astonia Client (c) Daniel Brockhaus. Please read license.txt.
 *
 * Display Windows
 *
 * Equipment, inventory, text, ... windows.
 */

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "../../src/astonia.h"
#include "../../src/gui.h"
#include "../../src/gui/_gui.h"
#include "../../src/game.h"
#include "../../src/client.h"
#include "../../src/modder.h"

char tutor_text[1024]={""};
int show_tutor=0;

static void dx_drawtext_gold(int x,int y,unsigned short int color,int amount) {
    if (amount>99) dd_drawtext_fmt(x,y,color,DD_CENTER|DD_FRAME|DD_SMALL,"%d.%02dG",amount/100,amount%100);
    else dd_drawtext_fmt(x,y,color,DD_CENTER|DD_FRAME|DD_SMALL,"%ds",amount);
}

void display_wear(void) {
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

void display_look(void) {
    int b,i,x,y; //,yt;
    unsigned int sprite;
    unsigned short c1,c2,c3,shine;
    unsigned char scale,cr,cg,cb,light,sat;
    DDFX fx;
    static int look_anim=4,look_step=0,look_dir=0;

    dd_copysprite(opt_sprite(994),dotx(DOT_LOK),doty(DOT_LOK),DDFX_NLIGHT,DD_NORMAL);

    for (b=BUT_WEA_BEG; b<=BUT_WEA_END; b++) {
        i=b-BUT_WEA_BEG;

        x=dotx(DOT_LOK)+but[b].x-dotx(DOT_WEA)+30;
        y=doty(DOT_LOK)+20;

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
    dd_drawtext(dotx(DOT_LOK)+70,doty(DOT_LOK)+50,0xffff,DD_LEFT,look_name);
    dd_drawtext_break(dotx(DOT_LOK)+70,doty(DOT_LOK)+60,dotx(DOT_LOK)+290,0xffff,DD_LEFT,look_desc);

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
        dd_copysprite_fx(&fx,dotx(DOT_LOK)+40,doty(DOT_LOK)+110);
    }
}

void display_inventory(void) {
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
            fx.ml=fx.ll=fx.rl=fx.ul=fx.dl=(i==invsel)?FX_ITEMBRIGHT:FX_ITEMLIGHT;
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

void display_container(void) {
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

void display_gold(void) {
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

void display_citem(void) {
    int x,y;
    unsigned int sprite;
    unsigned short c1,c2,c3,shine;
    unsigned char scale,cr,cg,cb,light,sat;
    DDFX fx;

    // trashcan
    if (vk_item || csprite) {
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

void display_scrollbars(void) {
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

void display_skill(void) {
    int b,i,x,y,yt,bsx,bex,bsy,barsize,cn;
    char buf[256];

    cn=map[MAPDX*MAPDY/2].cn;

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
            case V_COLD:
                sprintf(buf,"%d",skltab[i].curr);
                break;
            case V_ARMOR:
                sprintf(buf,"%.2f",skltab[i].curr/20.0);
                break;
            case V_MANA:
                sprintf(buf,"%d/%2d/%2d",mana,skltab[i].base,skltab[i].curr);
                break;
            case V_HP:
                if (lifeshield) sprintf(buf,"%d+%d/%2d/%2d",hp,lifeshield,skltab[i].base,skltab[i].curr);
                else sprintf(buf,"%d/%2d/%2d",hp,skltab[i].base,skltab[i].curr);
                break;
            case V_ENDURANCE:
                sprintf(buf,"%d/%2d/%2d",endurance,skltab[i].base,skltab[i].curr);
                break;
            default:
                if (!amod_display_skill_line(skltab[i].v,skltab[i].base,skltab[i].curr,cn,buf)) {
                    if (skltab[i].v>=V_PROFBASE) sprintf(buf,"%d",skltab[i].base);
                    else sprintf(buf,"%2d/%2d",skltab[i].base,skltab[i].curr);
                }
                break;
        }

        dd_drawtext(bsx,yt,textcolor,DD_LARGE|DD_LEFT,skltab[i].name);
        dd_drawtext(bex,yt,textcolor,DD_LARGE|DD_RIGHT,buf);
    }
}

void display_keys(void) {
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

        dd_drawtext(dotx(DOT_BOT)+x,doty(DOT_BOT)-6,col,DD_LEFT|DD_SMALL|DD_FRAME,buf);
    }
}

void display_tutor(void) {
    int x,y,n,mx=dotx(DOT_BOT)+626,my=doty(DOT_BOT)-370+416-80;
    char *ptr,buf[80];

    if (!show_tutor) return;

    dd_rect(dotx(DOT_BOT)+220,doty(DOT_BOT)-140,dotx(DOT_BOT)+630,doty(DOT_BOT)-50,IRGB(24,22,16));
    dd_line(dotx(DOT_BOT)+220,doty(DOT_BOT)-140,dotx(DOT_BOT)+630,doty(DOT_BOT)-140,IRGB(12,10,4));
    dd_line(dotx(DOT_BOT)+630,doty(DOT_BOT)-140,dotx(DOT_BOT)+630,doty(DOT_BOT)-50,IRGB(12,10,4));
    dd_line(dotx(DOT_BOT)+220,doty(DOT_BOT)-50,dotx(DOT_BOT)+360,doty(DOT_BOT)-50,IRGB(12,10,4));
    dd_line(dotx(DOT_BOT)+220,doty(DOT_BOT)-140,dotx(DOT_BOT)+220,doty(DOT_BOT)-50,IRGB(12,10,4));

    x=dotx(DOT_BOT)+224; y=doty(DOT_BOT)-136; ptr=tutor_text;
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

// date stuff
#define DAYLEN		(60*60*2)
#define HOURLEN		(DAYLEN/24)
#define MINLEN		(HOURLEN/60)

static void trans_date(int t,int *phour,int *pmin) {
    if (pmin) *pmin=(t/MINLEN)%60;
    if (phour) *phour=(t/HOURLEN)%24;
}

void display_screen(void) {
    int h,m;
    int h1,h2,m1,m2;
    static int rh1=0,rh2=0,rm1=0,rm2=0;

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

    sprintf(hover_time_text,"%02d:%02d Astonia Standard Time",h,m);

    dd_copysprite(opt_sprite(998),dotx(DOT_BOT),doty(DOT_BOT),DDFX_NLIGHT,DD_NORMAL);
}


void display_text(void) {

    dd_display_text();

    if (dd_scantext(mousex,mousey,hitsel));
    else hitsel[0]=0;

    display_cmd();
}

void display_mode(void) {
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

void display_selfspells(void) {
    int n,nr,cn,step;

    cn=map[mapmn(MAPDX/2,MAPDY/2)].cn;
    if (!cn) return;

    sprintf(hover_bless_text,"Bless: Not active");
    sprintf(hover_freeze_text,"Freeze: Not active");
    sprintf(hover_potion_text,"Potion: Not active");

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
                sprintf(hover_bless_text,"Bless: %ds to go",(ceffect[nr].bless.stop-tick)/24);
                break;
            case 11:
                step=50-50*(ceffect[nr].freeze.stop-tick)/(ceffect[nr].freeze.stop-ceffect[nr].freeze.start);
                dd_push_clip();
                dd_more_clip(0,0,800,doty(DOT_BOT)+119);
                dd_copysprite(997,dotx(DOT_BOT)+179+1*10,doty(DOT_BOT)+68+step,DDFX_NLIGHT,DD_NORMAL);
                dd_pop_clip();
                sprintf(hover_freeze_text,"Freeze: %ds to go",(ceffect[nr].freeze.stop-tick)/24);
                break;

            case 14:
                step=50-50*(ceffect[nr].potion.stop-tick)/(ceffect[nr].potion.stop-ceffect[nr].potion.start);
                dd_push_clip();
                dd_more_clip(0,0,800,doty(DOT_BOT)+119);
                if (step>=40 && (tick&4)) dd_copysprite(997,dotx(DOT_BOT)+179+0*10,doty(DOT_BOT)+68+step,DDFX_BRIGHT,DD_NORMAL);
                else dd_copysprite(997,dotx(DOT_BOT)+179+0*10,doty(DOT_BOT)+68+step,DDFX_NLIGHT,DD_NORMAL);
                dd_pop_clip();
                sprintf(hover_potion_text,"Potion: %ds to go",(ceffect[nr].potion.stop-tick)/24);
                break;
        }
    }
}

void display_exp(void) {
    int level,step,total,expe,cn,clevel,nlevel;
    static int last_exp=0,exp_ticker=0;

    sprintf(hover_level_text,"Level: unknown");

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
        dd_more_clip(0,0,dotx(DOT_TOP)+31+100-(int)(100ll*step/total),doty(DOT_TOP)+8+7);
        dd_copysprite(996,dotx(DOT_TOP)+31,doty(DOT_TOP)+7,exp_ticker?DDFX_BRIGHT:DDFX_NLIGHT,DD_NORMAL);
        dd_pop_clip();

        if (exp_ticker) exp_ticker--;

        sprintf(hover_level_text,"Level: From %d to %d",clevel,nlevel);
    }
}

char *_game_rankname[]={
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
char **game_rankname=_game_rankname;

int _game_rankcount=ARRAYSIZE(_game_rankname);
int *game_rankcount=&_game_rankcount;

static int mil_rank(int exp) {
    int n;

    for (n=1; n<50; n++) {
        if (exp<n*n*n) return n-1;
    }
    return 99;
}

void display_military(void) {
    int step,total,rank,cost1,cost2;

    sprintf(hover_rank_text,"Rank: none or unknown");

    rank=mil_rank(mil_exp);
    cost1=rank*rank*rank;
    cost2=(rank+1)*(rank+1)*(rank+1);

    total=cost2-cost1;
    step=mil_exp-cost1;
    if (step>total) step=total;

    if (mil_exp && total) {
        if (rank<*game_rankcount) {
            dd_push_clip();
            dd_more_clip(0,0,dotx(DOT_TOP)+31+100*step/total,doty(DOT_TOP)+8+24);
            dd_copysprite(993,dotx(DOT_TOP)+31,doty(DOT_TOP)+24,DDFX_NLIGHT,DD_NORMAL);
            dd_pop_clip();

            sprintf(hover_rank_text,"Rank: '%s' to '%s'",game_rankname[rank],game_rankname[rank+1]);
        } else sprintf(hover_rank_text,game_rankname[*game_rankcount-1]);
    }
}

void display_rage(void) {
    int step;

    sprintf(hover_rage_text,"Rage: Not active");

    if (!value[0][V_RAGE] || !rage) return;

    step=50-50*rage/value[0][V_RAGE];
    dd_push_clip();
    dd_more_clip(0,0,800,doty(DOT_BOT)+119);
    dd_copysprite(997,dotx(DOT_BOT)+179+3*10,doty(DOT_BOT)+68+step,DDFX_NLIGHT,DD_NORMAL);
    dd_pop_clip();

    sprintf(hover_rage_text,"Rage: %d%%",100*rage/value[0][V_RAGE]);
}

void display_game_special(void) {
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

static char action_row[2][12]={
    "asd   fg   h",
    " qwertzuiop "
};
static char *action_text[12]={
    "Attack",
    "Fireball",
    "Lightning Ball",
    "Flash",
    "Freeze",
    "Magic Shield",
    "Bless",
    "Heal",
    "Warcry",
    "Pulse",
    "Firering",
    "Take/Use"
};

static int action_skill[12]={
    V_PERCEPT,
    V_FIREBALL,
    V_FLASH,
    V_FLASH,
    V_FREEZE,
    V_MAGICSHIELD,
    V_BLESS,
    V_HEAL,
    V_WARCRY,
    V_PULSE,
    V_FIREBALL,
    V_PERCEPT
};

int has_action_skill(int i) {
    return value[0][action_skill[i]];
}

void display_action(void) {
    int i;
    char buf[4];

    if (!context_action_enabled()) return;

    for (i=0; i<12; i++) {
        if (!has_action_skill(i)) continue;
        dd_copysprite(800+i,butx(BUT_ACT_BEG+i),buty(BUT_ACT_BEG+i),(i==actsel || i==action_ovr)?DDFX_BRIGHT:DDFX_NLIGHT,0);
        if (i==actsel) {
            dd_drawtext(butx(BUT_ACT_BEG+i),buty(BUT_ACT_BEG+i)-30,IRGB(31,31,31),DD_FRAME|DD_CENTER,action_text[i]);
        }
        if (action_row[0][i]>' ') {
            buf[0]=action_row[0][i]; buf[1]=0;
            dd_drawtext(butx(BUT_ACT_BEG+i)-10,buty(BUT_ACT_BEG+i)-14,IRGB(31,31,31),DD_FRAME|DD_CENTER,buf);
        }
        if (action_row[1][i]>' ') {
            buf[0]=action_row[1][i]; buf[1]=0;
            dd_drawtext(butx(BUT_ACT_BEG+i)+10,buty(BUT_ACT_BEG+i)-14,IRGB(31,31,31),DD_FRAME|DD_CENTER,buf);
        }
    }
}

