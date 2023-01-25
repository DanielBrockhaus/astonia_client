/*
 * Part of Astonia Mod (c) ModderMcModFace.
 *
 * Says Hello
 *
 */

#include <stdint.h>
#include <stdio.h>

#include "../../src/amod/amod.h"
#if 0
#include "bmod.c"
#else
#include "strings.c"
#include "ranks.c"
#include "skills.c"

__declspec(dllexport) void amod_gamestart(void) {
    note("A Mod by ModderMcModFace loaded.");
}

__declspec(dllexport) int trans_charno(int csprite,int *pscale,int *pcr,int *pcg,int *pcb,int *plight,int *psat,int *pc1,int *pc2,int *pc3,int *pshine,int attick)
{
    int scale=100;

    switch (csprite) {
        case 120:   csprite=8; scale=67; break; // tiny skelly
        default:    return _trans_charno(csprite,pscale,pcr,pcg,pcb,plight,psat,pc1,pc2,pc3,pshine,attick);
    }

    if (pscale) *pscale=scale;

    return csprite;
}
#endif
