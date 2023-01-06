/*
 * Part of Astonia Client (c) Daniel Brockhaus. Please read license.txt.
 */

#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <stdlib.h>
#include <windows.h>
#include <windowsx.h>
#include <process.h>
#include <errno.h>
#include <math.h>
#include "main.h"
#include "dd.h"

/* NOTE: we could speed up sound a lot by caching and re-using buffers containing recently played sounds */

extern int enable_sound;



int init_sound(void) {

    if (!enable_sound) return -1;

    // TODO: re-add sound. maybe.

    return -1;
}

#if 0
static char *sfx_name[]={
    "..\\sfx\\null.wav",            //0
    "..\\sfx\\sdemonawaken.wav",        //1
    "..\\sfx\\door.wav",            //2
    "..\\sfx\\door2.wav",           //3
    "..\\sfx\\man_dead.wav",        //4
    "..\\sfx\\thunderrumble3.wav",      //5
    "..\\sfx\\explosion.wav",       //6
    "..\\sfx\\hit_body2.wav",       //7
    "..\\sfx\\miss.wav",            //8
    "..\\sfx\\man_hurt.wav",        //9
    "..\\sfx\\pigeon.wav",          //10
    "..\\sfx\\crow.wav",            //11
    "..\\sfx\\crow2.wav",           //12
    "..\\sfx\\laughingman6.wav",        //13
    "..\\sfx\\drip1.wav",           //14
    "..\\sfx\\drip2.wav",           //15
    "..\\sfx\\drip3.wav",           //16
    "..\\sfx\\howl1.wav",           //17
    "..\\sfx\\howl2.wav",           //18
    "..\\sfx\\bird1.wav",           //19
    "..\\sfx\\bird2.wav",           //20
    "..\\sfx\\bird3.wav",           //21
    "..\\sfx\\catmeow2.wav",        //22
    "..\\sfx\\cricket.wav",         //23
    "..\\sfx\\specht.wav",          //24
    "..\\sfx\\haeher.wav",          //25
    "..\\sfx\\owl1.wav",            //26
    "..\\sfx\\owl2.wav",            //27
    "..\\sfx\\owl3.wav",            //28
    "..\\sfx\\magic.wav",           //29
    "..\\sfx\\flash.wav",           //30	lightning strike
    "..\\sfx\\scarynote.wav",       //31	freeze
    "..\\sfx\\woman_hurt.wav",      //32
    "..\\sfx\\woman_dead.wav",      //33
    "..\\sfx\\parry1.wav",          //34
    "..\\sfx\\parry2.wav",          //35
    "..\\sfx\\dungeon_breath1.wav",     //36
    "..\\sfx\\dungeon_breath2.wav",     //37
    "..\\sfx\\pents_mood1.wav",     //38
    "..\\sfx\\pents_mood2.wav",     //39
    "..\\sfx\\pents_mood3.wav",     //40
    "..\\sfx\\ancient_activate.wav",    //41
    "..\\sfx\\pent_activate.wav",       //42
    "..\\sfx\\ancient_runout.wav",      //43

    "..\\sfx\\bubble1.wav",         //44
    "..\\sfx\\bubble2.wav",         //45
    "..\\sfx\\bubble3.wav",         //46
    "..\\sfx\\whale1.wav",          //47
    "..\\sfx\\whale2.wav",          //48
    "..\\sfx\\whale3.wav",          //49

    NULL
};
#endif

void play_pak_sound(int nr,int vol,int p) {
    if (!enable_sound) return;

    // force volume and pan to sane values
    if (vol>0) vol=0;
    if (vol<-9999) vol=-9999;

    if (p>9999) p=9999;
    if (p<-9999) p=-9999;

    // TODO: play the sound
}

