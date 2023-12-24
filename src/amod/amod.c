/*
 * Part of a Mod (c) Modder
 *
 * See https://brockhaus.org/modding.html
 *
 * Use "make amod" to build.
 *
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "../../src/amod/amod.h"

#if 0
#include "strings.c"
#include "ranks.c"
#include "skills.c"
#include "quests.c"
#endif

DLL_EXPORT char *amod_version(void) {
    return "Restart Demo 0.4";
}

DLL_EXPORT void amod_gamestart(void) {
    note("Restart Client Demo v0.4 loaded.");
}

DLL_EXPORT int amod_client_cmd(char *buf) {
    static unsigned long long option_ovr=0;

    if (!strncmp(buf, "#slow",5)) {
        game_slowdown=1-game_slowdown;
        return 1;
    }

    if (!strncmp(buf, "#option ", 8)) {
    	option_ovr=strtoull(&buf[8],NULL,10);
        addline("Old options=%llu, new options=%llu",game_options,option_ovr);
    	return 1;
    }

    if (!strncmp(buf,"#reset",6)) {
        char opt[20][100];
        sprintf(opt[0],"-u%s",username);
        sprintf(opt[1],"-p%s",password);
        sprintf(opt[2],"-d%s",server_url);
        sprintf(opt[3],"-w%d",want_width);
        sprintf(opt[4],"-h%d",want_height);
        sprintf(opt[5],"-o%llu",option_ovr?option_ovr:game_options);
        sprintf(opt[6],"-k%d",frames_per_second);
        sprintf(opt[7],"-c%d",sdl_cache_size);
        sprintf(opt[8],"-m%d",sdl_multi);
        sprintf(opt[9],"-t%d",server_port);

        printf("bin\\moac.exe ");
        for (int i=0; i<10; i++) {
            printf("%s ",opt[i]);
        }
        printf("\n");
        execl("bin\\moac.exe","bin\\moac.exe",opt[0],opt[1],opt[2],opt[3],opt[4],opt[5],opt[6],opt[7],opt[8],opt[9],NULL);
    }

    if (!strncmp(buf,"#echo",5)) {
        addline("Echo from mod!");
    	return -1;
    }
    return 0;
}

DLL_EXPORT int amod_keydown(int key) {
    return 0;
}
DLL_EXPORT int amod_keyup(int key) {
    return 0;
}

DLL_EXPORT int amod_is_playersprite(int sprite) {
    return (sprite==800 || sprite==801);
}


DLL_EXPORT int amod_process(char *buf) {
    switch (buf[0]) {
        case SV_MOD1:
            addline("process got sv_mod1");
            return 5;
    }
    return 0;
}
DLL_EXPORT int amod_prefetch(char *buf) {
    switch (buf[0]) {
        case SV_MOD1:   return 5;
    }
    return 0;
}

DLL_EXPORT void amod_frame(void) {

}

DLL_EXPORT int do_display_help(int nr) {
    int x=dotx(DOT_HLP)+10,y=doty(DOT_HLP)+8;

    switch (nr) {
        case 19:
            y=dd_drawtext_break(x,y,x+192,whitecolor,0,"Reading books, signs, etc."); y+=5;
            y=dd_drawtext_break(x,y,x+192,graycolor,0,"To read a book, turn off your computer and take the book. Open it. Choose a page. Read. To read signs, use SHIFT + RIGHT CLICK."); y+=10;
            break;

        default:
            return _do_display_help(nr);
    }

    return y;
}

