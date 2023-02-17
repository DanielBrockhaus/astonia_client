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
#include "../../src/amod/amod.h"

#if 0
#include "strings.c"
#include "ranks.c"
#include "skills.c"
#endif


__declspec(dllexport) void amod_gamestart(void) {
    note("Mod v0.1 loaded.");
}

__declspec(dllexport) void amod_update_hover_texts(void) {
    sprintf(hover_bless_text,"HUHU!");
}

__declspec(dllexport) void amod_frame(void) {

}

__declspec(dllexport) void amod_areachange(void) {
}

__declspec(dllexport) int amod_keydown(int key) {
    return 0;
}

