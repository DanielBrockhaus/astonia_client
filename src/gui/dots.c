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

DOT *dot=NULL;
BUT *but=NULL;

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

void init_dots(void) {
    int i,x,y;

    // dots
    dot=xmalloc(MAX_DOT*sizeof(DOT),MEM_GUI);

    // top left, bottom right of screen
    set_dot(DOT_TL,0,0,0);
    set_dot(DOT_BR,XRES,YRES,0);

    // equipment, inventory, container. center of first displayed item.
    set_dot(DOT_WEA,180,20,YRES==YRES0 ? 0 : DOTF_TOPOFF);
    set_dot(DOT_INV,660,YRES-540+398,0);
    set_dot(DOT_CON,20,YRES-540+398,0);

    // inventory top left and bottom right
    set_dot(DOT_IN1,645,YRES-540+378,0);
    set_dot(DOT_IN2,795,YRES-540+538,0);

    // top and bottom window
    set_dot(DOT_TOP,0,  0,YRES==YRES0 ? 0 : DOTF_TOPOFF);
    set_dot(DOT_BOT,0,YRES-540+370,0);

    // scroll bars
    set_dot(DOT_SCL,160+5,0,0);
    set_dot(DOT_SCR,640-5,0,0);
    set_dot(DOT_SCU,0,YRES-540+385,0);
    set_dot(DOT_SCD,0,YRES-540+530,0);

    // chat text
    set_dot(DOT_TXT,230,YRES-540+378,0);
    set_dot(DOT_TX2,624,YRES-540+538,0);

    // skill list
    set_dot(DOT_SKL,8,YRES-540+384,0);
    set_dot(DOT_SK2,156,YRES-540+538,0);

    // gold
    set_dot(DOT_GLD,195,YRES-540+520,0);

    // trashcan
    set_dot(DOT_JNK,610,YRES-540+520,0);

    // speed options: stealth/normal/fast
    set_dot(DOT_MOD,181,YRES-540+393,0);

    // map top left, bottom right, center
    set_dot(DOT_MTL,  0, 40,YRES==YRES0 ? 0 : DOTF_TOPOFF);
    set_dot(DOT_MBR,800,YRES-540+376,0);
    set_dot(DOT_MCT,400,YRES/2-40-(YRES==YRES0 ? 40 : 0),0);

    // help and quest window
    set_dot(DOT_HLP,0,(YRES==YRES0 ? 40 : 0),0);
    set_dot(DOT_HL2,222,(YRES==YRES0 ? 40 : 0)+394,0);

    // teleporter window
    set_dot(DOT_TEL,100,40,0);

    // look at window
    set_dot(DOT_LOK,150,50,0);

    // color picker window
    set_dot(DOT_COL,340,210,0);

    // buts
    but=xmalloc(MAX_BUT*sizeof(BUT),MEM_GUI);

    set_but(BUT_MAP,XRES/2,YRES/2,0,BUTID_MAP,0,BUTF_NOHIT);

    // note to self: do not use dotx(),doty() here because the moving top bar logic is built into the
    // button flags as well
    for (i=0; i<12; i++) set_but(BUT_WEA_BEG+i,dot[DOT_WEA].x+i*FDX,dot[DOT_WEA].y+0,40,BUTID_WEA,0,YRES==YRES0 ? 0 : BUTF_TOPOFF);
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
    set_but(BUT_HELP_DRAG,(dotx(DOT_HLP)+dotx(DOT_HL2))/2,doty(DOT_HLP)+6,0,0,0,BUTF_CAPTURE|BUTF_MOVEEXEC);
}

