/*
 * Part of Astonia Client (c) Daniel Brockhaus. Please read license.txt.
 *
 * Dots and Buttons
 *
 * Dots are used to position GUI elements. Positioning can be changed by
 * changing init_dots() or individual elements of the dots and/or button
 * array.
 */

#include <stdint.h>

#include "../../src/astonia.h"
#include "../../src/gui.h"
#include "../../src/gui/_gui.h"

extern int __textdisplay_sy;

DOT *dot=NULL;
BUT *but=NULL;

// dot and but helpers

void set_dot(int didx,int x,int y,int flags) {
    PARANOIA(if (didx<0 || didx>=MAX_DOT) paranoia("set_dot: ill didx=%d",didx); )

    dot[didx].flags=flags;
    dot[didx].x=x;
    dot[didx].y=y;
}

DLL_EXPORT int dotx(int didx) {
    return dot[didx].x;
}

DLL_EXPORT int doty(int didx) {
    if (dot[didx].flags&DOTF_TOPOFF) return dot[didx].y+gui_topoff;
    return dot[didx].y;
}

void set_but(int bidx,int x,int y,int hitrad,int flags) {
    PARANOIA(if (bidx<0 || bidx>=MAX_BUT) paranoia("set_but: ill bidx=%d",bidx); )

    but[bidx].flags=flags;

    but[bidx].x=x;
    but[bidx].y=y;

    but[bidx].sqhitrad=hitrad*hitrad;
}

DLL_EXPORT int butx(int bidx) {
    return but[bidx].x;
}

DLL_EXPORT int buty(int bidx) {
    if (but[bidx].flags&BUTF_TOPOFF) return but[bidx].y+gui_topoff;
    return but[bidx].y;
}

#define stop        (game_options&GO_SMALLTOP)
#define sbot        (game_options&GO_SMALLBOT)

void dots_update(void) {

    set_dot(DOT_TUT,(XRES-410)/2,doty(DOT_MBR)-100-(context_action_enabled()?30:0),0);
}

void init_dots(void) {
    int i,x,y,xc,yc;

    // dots
    dot=xmalloc(MAX_DOT*sizeof(DOT),MEM_GUI);

    // top left, bottom right of screen
    set_dot(DOT_TL,0,0,0);
    set_dot(DOT_BR,XRES,YRES,0);

    // top and bottom window
    set_dot(DOT_TOP,0,  0,!stop ? 0 : DOTF_TOPOFF);
    if (!sbot) set_dot(DOT_BOT,0,YRES-170,0);
    else set_dot(DOT_BOT,0,YRES-130,0);
    set_dot(DOT_BO2,XRES,YRES,0);

    // equipment, inventory, container. center of first displayed item.
    set_dot(DOT_WEA,180,20,!stop ? 0 : DOTF_TOPOFF);
    set_dot(DOT_INV,660,doty(DOT_BOT)+27,0);
    set_dot(DOT_CON,20,doty(DOT_BOT)+27,0);

    // inventory top left and bottom right
    set_dot(DOT_IN1,645,doty(DOT_BOT)+2,0);
    set_dot(DOT_IN2,795,doty(DOT_BO2)-2,0);
    if (!sbot) __invdy=4;
    else __invdy=3;

    // scroll bars
    set_dot(DOT_SCL,160+5,0,0);
    set_dot(DOT_SCR,640-5,0,0);
    set_dot(DOT_SCU,0,doty(DOT_BOT)+15,0);
    if (!sbot) set_dot(DOT_SCD,0,doty(DOT_BOT)+160,0);
    else set_dot(DOT_SCD,0,doty(DOT_BOT)+120,0);

    // self spell bars (bless, potion, rage, ...)
    if (!sbot) set_dot(DOT_SSP,dotx(DOT_BOT)+179,doty(DOT_BOT)+68,0);
    else set_dot(DOT_SSP,dotx(DOT_BOT)+179,doty(DOT_BOT)+52,0);

    // chat text
    set_dot(DOT_TXT,230,doty(DOT_BOT)+8,0);
    if (!sbot) {
        set_dot(DOT_TX2,624,doty(DOT_BOT)+158,0);
       __textdisplay_sy=150;
    } else {
        set_dot(DOT_TX2,624,doty(DOT_BOT)+118,0);
       __textdisplay_sy=110;
    }

    // skill list
    set_dot(DOT_SKL,8,doty(DOT_BOT)+12,0);
    set_dot(DOT_SK2,156,doty(DOT_BO2)-2,0);
    if (!sbot) __skldy=16;
    else __skldy=12;

    // gold
    set_dot(DOT_GLD,195,doty(DOT_BO2)-22,0);

    // trashcan
    set_dot(DOT_JNK,610,doty(DOT_BO2)-22,0);

    // speed options: stealth/normal/fast
    set_dot(DOT_MOD,181,doty(DOT_BOT)+24,0);

    // map top left, bottom right, center
    set_dot(DOT_MTL,  0, 40,!stop ? 0 : DOTF_TOPOFF);
    set_dot(DOT_MBR,800,min(doty(DOT_MTL)+450-(!stop?0:40),doty(DOT_BOT)+4),0);
    x=dotx(DOT_MBR)-dotx(DOT_MTL);
    y=doty(DOT_MBR)-doty(DOT_MTL)+(!stop?0:40);
    xc=x/2;
    if (y<430) yc=y/2+20;
    else if (y<450) yc=y/2+20-y+430;
    else yc=y/2;
    set_dot(DOT_MCT,dotx(DOT_MTL)+xc,doty(DOT_MTL)-(!stop?0:40)+yc,0);
    //note("map: %dx%d, center: %d,%d, origin: %d,%d, (%d,%d)",x,y,dotx(DOT_MCT),doty(DOT_MCT),dotx(DOT_MTL),doty(DOT_MTL),dotx(DOT_MBR),doty(DOT_MBR));

    // help and quest window
    set_dot(DOT_HLP,0,(!stop ? 40 : 0),0);
    set_dot(DOT_HL2,222,(!stop ? 40 : 0)+394,0);

    // teleporter window
    set_dot(DOT_TEL,(XRES-520)/2,(doty(DOT_MBR)-doty(DOT_MTL)-320-(!stop?0:40))/2+doty(DOT_MTL),0);

    // look at window
    set_dot(DOT_LOK,150,50,0);

    // color picker window
    set_dot(DOT_COL,340,210,0);

    // action bar
    set_dot(DOT_ACT,XRES-MAXACTIONSLOT*40-(XRES-MAXACTIONSLOT*40)/2,doty(DOT_BOT)-12,0);

    // tutor window
    dots_update();

    // buts
    but=xmalloc(MAX_BUT*sizeof(BUT),MEM_GUI);

    set_but(BUT_MAP,XRES/2,YRES/2,0,BUTF_NOHIT);

    // note to self: do not use dotx(),doty() here because the moving top bar logic is built into the
    // button flags as well
    for (i=0; i<12; i++) set_but(BUT_WEA_BEG+i,dot[DOT_WEA].x+i*FDX,dot[DOT_WEA].y+0,40,!stop ? 0 : BUTF_TOPOFF);
    for (x=0; x<4; x++) for (y=0; y<4; y++) set_but(BUT_INV_BEG+x+y*4,dot[DOT_INV].x+x*FDX,dot[DOT_INV].y+y*FDX,40,0);
    for (x=0; x<4; x++) for (y=0; y<4; y++) set_but(BUT_CON_BEG+x+y*4,dot[DOT_CON].x+x*FDX,dot[DOT_CON].y+y*FDX,40,0);
    for (i=0; i<16; i++) set_but(BUT_SKL_BEG+i,dot[DOT_SKL].x,dot[DOT_SKL].y+i*LINEHEIGHT,10,0);
    for (i=0; i<MAXACTIONSLOT; i++) set_but(BUT_ACT_BEG+i,dot[DOT_ACT].x+i*40,dot[DOT_ACT].y,18,0);

    set_but(BUT_WEA_LCK,dot[DOT_WEA].x+12*FDX-12,dot[DOT_WEA].y+4,18,!stop ? 0 : BUTF_TOPOFF);
    set_but(BUT_ACT_LCK,dot[DOT_ACT].x-40,dot[DOT_ACT].y,18,0);
    set_but(BUT_ACT_OPN,dot[DOT_ACT].x+MAXACTIONSLOT*40,dot[DOT_ACT].y,18,0);

    set_but(BUT_SCL_UP,dot[DOT_SCL].x+0,dot[DOT_SCU].y+0,30,0);
    set_but(BUT_SCL_TR,dot[DOT_SCL].x+0,dot[DOT_SCU].y+10,40,BUTF_CAPTURE|BUTF_MOVEEXEC);
    set_but(BUT_SCL_DW,dot[DOT_SCL].x+0,dot[DOT_SCD].y+0,30,0);

    set_but(BUT_SCR_UP,dot[DOT_SCR].x+0,dot[DOT_SCU].y+0,30,0);
    set_but(BUT_SCR_TR,dot[DOT_SCR].x+0,dot[DOT_SCU].y+10,40,BUTF_CAPTURE|BUTF_MOVEEXEC);
    set_but(BUT_SCR_DW,dot[DOT_SCR].x+0,dot[DOT_SCD].y+0,30,0);

    if (!stop) set_but(BUT_GLD,dot[DOT_GLD].x+0,dot[DOT_GLD].y+10,30,BUTF_CAPTURE);
    else set_but(BUT_GLD,dot[DOT_GLD].x+0,dot[DOT_GLD].y+10,15,BUTF_CAPTURE);

    set_but(BUT_JNK,dot[DOT_JNK].x+0,dot[DOT_JNK].y+0,30,0);

    set_but(BUT_MOD_WALK0,dot[DOT_MOD].x+1*14,dot[DOT_MOD].y+0*30,30,0);
    set_but(BUT_MOD_WALK1,dot[DOT_MOD].x+0*14,dot[DOT_MOD].y+0*30,30,0);
    set_but(BUT_MOD_WALK2,dot[DOT_MOD].x+2*14,dot[DOT_MOD].y+0*30,30,0);
    set_but(BUT_HELP_DRAG,(dotx(DOT_HLP)+dotx(DOT_HL2))/2,doty(DOT_HLP)+6,0,BUTF_CAPTURE|BUTF_MOVEEXEC);
}

