/*
 * Part of a Mod (c) Modder
 *
 * See https://brockhaus.org/modding.html
 *
 * Use "make amod" to build.
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "../../src/amod/amod.h"

#if 0
#include "strings.c"
#include "ranks.c"
#include "skills.c"
#endif


__declspec(dllexport) void amod_gamestart(void) {
    note("Restart Client Demo v0.1 loaded.");
}

__declspec(dllexport) void amod_update_hover_texts(void) {
}

__declspec(dllexport) void amod_frame(void) {

}

__declspec(dllexport) void amod_areachange(void) {
}

__declspec(dllexport) int amod_keydown(int key) {
    if (key=='.') {
        char opt[20][100];
        sprintf(opt[0],"-u%s",username);
        sprintf(opt[1],"-p%s",password);
        sprintf(opt[2],"-d%s",server_url);
        sprintf(opt[3],"-w%d",want_width);
        sprintf(opt[4],"-h%d",want_height);
        sprintf(opt[5],"-o%llu",game_options);
        sprintf(opt[6],"-k%d",sdl_frames);
        sprintf(opt[7],"-c%d",sdl_cache_size);
        sprintf(opt[8],"-m%d",sdl_multi);

        printf("bin\\moac.exe ");
        for (int i=0; i<9; i++) {
            printf("%s ",opt[i]);
        }
        printf("\n");
        execl("bin\\moac.exe","bin\\moac.exe",opt[0],opt[1],opt[2],opt[3],opt[4],opt[5],opt[6],opt[7],opt[8],NULL);
    }
    return 0;
}


