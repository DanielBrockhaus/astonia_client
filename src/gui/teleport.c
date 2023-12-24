/*
 * Part of Astonia Client (c) Daniel Brockhaus. Please read license.txt.
 *
 * Display Teleport Window and Helpers
 *
 * Display the teleport window, and maps mouse clicks
 */

#include <stdint.h>
#include <stdlib.h>

#include "../../src/astonia.h"
#include "../../src/gui.h"
#include "../../src/gui/_gui.h"
#include "../../src/client.h"
#include "../../src/game.h"

int teleporter=0;

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

int clan_offset=0;

DLL_EXPORT void set_teleport(int idx,int x,int y) {
    if (idx<0 || idx>=64) return;

    tele[idx*2]=x;
    tele[idx*2+1]=y;
}

int get_teleport(int x,int y) {
    int n;

    if (!teleporter) return -1;

    // map teleports
    for (n=0; n<64; n++) {
        if (!tele[n*2]) break;
        if (tele[n*2]==-1) continue;
        if (!may_teleport[n]) continue;

        if (abs(tele[n*2]+dotx(DOT_TEL)-x)<8 && abs(tele[n*2+1]+doty(DOT_TEL)-y)<8) return n;

    }

    // clan teleports
    for (n=0; n<8; n++) {
        if (!may_teleport[n+64+clan_offset]) continue;

        if (abs(dotx(DOT_TEL)+337-x)<8 && abs(doty(DOT_TEL)+24+n*12-y)<8) return n+64;
    }
    for (n=0; n<8; n++) {
        if (!may_teleport[n+64+8+clan_offset]) continue;

        if (abs(dotx(DOT_TEL)+389-x)<8 && abs(doty(DOT_TEL)+24+n*12-y)<8) return n+64+8;
    }

    // mirror selector
    for (n=0; n<26; n++) {
        if (abs(mirror_pos[n*2]+dotx(DOT_TEL)-x)<8 && abs(mirror_pos[n*2+1]+doty(DOT_TEL)-y)<8) return n+101;
    }

    if (abs(389+dotx(DOT_TEL)-x)<8 && abs(24+8*12+doty(DOT_TEL)-y)<8) return 1042;

    return -1;
}

void display_teleport(void) {
    int n;

    if (!teleporter) return;

    if (!clan_offset) dd_copysprite(53519,dotx(DOT_TEL)+520/2,doty(DOT_TEL)+320/2,14,0);
    else dd_copysprite(53520,dotx(DOT_TEL)+520/2,doty(DOT_TEL)+320/2,14,0);

    for (n=0; n<64; n++) {
        if (!tele[n*2]) break;
        if (tele[n*2]==-1) continue;

        if (!may_teleport[n]) dx_copysprite_emerald(tele[n*2]+dotx(DOT_TEL),tele[n*2+1]+doty(DOT_TEL),2,0);
        else if (telsel==n) dx_copysprite_emerald(tele[n*2]+dotx(DOT_TEL),tele[n*2+1]+doty(DOT_TEL),2,2);
        else dx_copysprite_emerald(tele[n*2]+dotx(DOT_TEL),tele[n*2+1]+doty(DOT_TEL),2,1);
    }

    for (n=0; n<8; n++) {
        if (!may_teleport[n+64+clan_offset]) dx_copysprite_emerald(337+dotx(DOT_TEL),24+n*12+doty(DOT_TEL),3,0);
        else if (telsel==n+64+clan_offset) dx_copysprite_emerald(337+dotx(DOT_TEL),24+n*12+doty(DOT_TEL),3,2);
        else dx_copysprite_emerald(337+dotx(DOT_TEL),24+n*12+doty(DOT_TEL),3,1);
    }
    for (n=0; n<8; n++) {
        if (8+clan_offset+n==31) continue;
        if (!may_teleport[n+64+8+clan_offset]) dx_copysprite_emerald(389+dotx(DOT_TEL),24+n*12+doty(DOT_TEL),3,0);
        else if (telsel==n+64+8+clan_offset) dx_copysprite_emerald(389+dotx(DOT_TEL),24+n*12+doty(DOT_TEL),3,2);
        else dx_copysprite_emerald(389+dotx(DOT_TEL),24+n*12+doty(DOT_TEL),3,1);
    }

    for (n=0; n<26; n++) {
        if (telsel==n+101) dx_copysprite_emerald(mirror_pos[n*2]+dotx(DOT_TEL),mirror_pos[n*2+1]+doty(DOT_TEL),1,2);
        else if (newmirror==n+1) dx_copysprite_emerald(mirror_pos[n*2]+dotx(DOT_TEL),mirror_pos[n*2+1]+doty(DOT_TEL),1,1);
        else dx_copysprite_emerald(mirror_pos[n*2]+dotx(DOT_TEL),mirror_pos[n*2+1]+doty(DOT_TEL),1,0);
    }

    if (telsel==1042) dx_copysprite_emerald(389+dotx(DOT_TEL),24+8*12+doty(DOT_TEL),2,2);
    else dx_copysprite_emerald(389+dotx(DOT_TEL),24+8*12+doty(DOT_TEL),2,1);
}

