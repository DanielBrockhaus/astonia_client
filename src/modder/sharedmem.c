/*
 * Part of Astonia Client (c) Daniel Brockhaus. Please read license.txt.
 *
 * Shared Memory
 *
 * Shared some data with (non-DLL) mods.
 */

#include <stdint.h>
#include <stdio.h>
#include <windows.h>

#include "../../src/astonia.h"
#include "../../src/modder.h"
#include "../../src/modder/_modder.h"
#include "../../src/client.h"

struct sharedmem {
    int pid;
    char hp,shield,end,mana;

    char *base;
    int key, isprite, offX, offY;
    int flags, fsprite;
    char swapped;
} __attribute__((packed));

static struct sharedmem *sm;
static HANDLE hMapFile;

static void random_dungeon_tracker(void) {

    sm->base = (char*)GetModuleHandle(NULL);

    if ((char*)&originx<(char*)&originy) {
        sm->key=(char*)&originx-sm->base;
        sm->swapped=0;
    } else {
        sm->key=(char*)&originy-sm->base;
        sm->swapped=1;
    }

    sm->isprite = (char*)&map[MAXMN/2]-sm->base+(char*)&map->isprite-(char*)&map;
    sm->flags = (char*)&map->flags - (char*)&map->isprite;
    sm->fsprite = (char*)&map->fsprite - (char*)&map->isprite;

    sm->offX = sizeof(map[0]);
    sm->offY = MAPDY;
}

int sharedmem_init(void) {
    int pid;
    char fname[80];

    pid=GetProcessId(GetCurrentProcess());
    sprintf(fname,"MOAC%d",pid);

    hMapFile = CreateFileMapping(
                 INVALID_HANDLE_VALUE,    // use paging file
                 NULL,                    // default security
                 PAGE_READWRITE,          // read/write access
                 0,                       // maximum object size (high-order DWORD)
                 sizeof(struct sharedmem),// maximum object size (low-order DWORD)
                 fname);                  // name of mapping object

    if (hMapFile == NULL) {
      fail("Could not create file mapping object (%lu).\n",GetLastError());
      return 0;
    }
    sm = (void*) MapViewOfFile(hMapFile,   // handle to map object
                        FILE_MAP_ALL_ACCESS, // read/write permission
                        0,
                        0,
                        sizeof(struct sharedmem));

    if (sm==NULL) {
      fail("Could not map view of file (%lu).\n",GetLastError());
      CloseHandle(hMapFile);
      return 1;
    }

    note("shared memory initialized as: %s",fname);

    sm->pid=pid;
    random_dungeon_tracker();
#if 1
    printf("base: %p\n",sm->base);
    printf("key: %d\n",sm->key);
    printf("isprite: %d\n",sm->isprite);
    printf("offX: %d\n",sm->offX);
    printf("offY: %d\n",sm->offY);
    printf("flags: %d\n",sm->flags);
    printf("fsprite: %d\n",sm->fsprite);
    printf("swapped: %d\n",sm->swapped);
#endif
    return 0;
}

void sharedmem_exit(void) {
    UnmapViewOfFile(sm);
    CloseHandle(hMapFile);
}

static int get_lifeshield_max(void) {
    if (value[0][V_MAGICSHIELD]) return value[0][V_MAGICSHIELD];
    return value[0][V_WARCRY];
}

void sharedmem_update(void) {
    int lifep,shieldp,endup,manap;

    if (value[0][V_HP]) lifep=100*hp/value[0][V_HP]; else lifep=100;
    if (get_lifeshield_max()) shieldp=100*lifeshield/get_lifeshield_max(); else shieldp=100;
    if (value[0][V_MANA]) manap=100*mana/value[0][V_MANA]; else manap=100;
    if (value[0][V_ENDURANCE]) endup=100*endurance/value[0][V_ENDURANCE]; else endup=100;

    sm->hp=lifep;
    sm->shield=shieldp;
    sm->mana=manap;
    sm->end=endup;
}


