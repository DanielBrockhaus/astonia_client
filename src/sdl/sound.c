/*
 * Part of Astonia Client (c) Daniel Brockhaus. Please read license.txt.
 *
 * Sound
 *
 * Loads and plays sounds via SDL2 library.
 */

#include <stdio.h>
#include <zip.h>
#include <SDL.h>
#include <SDL_mixer.h>

#include "../../src/astonia.h"
#include "../../src/sdl.h"
#include "../../src/sdl/_sdl.h"

int sound_volume=128;
static uint64_t time_play_sound=0;

static char *sfx_name[];
static int sfx_name_cnt;
static Mix_Chunk *sound_effect[MAXSOUND];

int init_sound(void) {
    int i,len,err;
    zip_t *sz;
    zip_file_t *zp;
    zip_stat_t stat;
    SDL_RWops *rw;
    char *buf;

    if (!(game_options&GO_SOUND)) return -1;

    sz=zip_open("res/sx.zip",ZIP_RDONLY,&err);
    if (!sz) {
        warn("Opening sx.zip failed with error code %d.",err);
        game_options&=~GO_SOUND;
        return -1;
    }

    for (i=1; i<sfx_name_cnt; i++) {
        if (zip_stat(sz,sfx_name[i],0,&stat) && !(stat.valid&ZIP_STAT_SIZE)) {
            warn("Could not stat sound file %s in archive.",sfx_name[i]);
            sound_effect[i]=NULL;
            continue;
        }
        len=stat.size;

        zp=zip_fopen(sz,sfx_name[i],0);
        if (!zp) {
            warn("Could not open sound file %s in archive.",sfx_name[i]);
            sound_effect[i]=NULL;
            continue;
        }

        buf=xmalloc(len,MEM_TEMP6);
        if (!buf) {
            warn("Could alloc memory for sound file %s.",sfx_name[i]);
            sound_effect[i]=NULL; zip_fclose(zp);
            continue;
        }
        if (zip_fread(zp,buf,len)!=len) {
            warn("Could alloc read sound file %s from archive.",sfx_name[i]);
            sound_effect[i]=NULL; zip_fclose(zp); xfree(buf);
            continue;
        }
        zip_fclose(zp);

        rw=SDL_RWFromConstMem(buf,len);

		sound_effect[i]=Mix_LoadWAV_RW(rw,1);
		if (!sound_effect[i]) {
			warn("Could alloc memory for sound file %s.",sfx_name[i]);
            sound_effect[i]=NULL; xfree(buf);
            continue;
		}
        xfree(buf);
	}
    zip_close(sz);

    return 0;
}

void sound_exit() {
    int i;

	// Free all sound effects
	// Starting at 1 since 0 is null
	for (i=1; i<sfx_name_cnt; i++) {
		Mix_FreeChunk(sound_effect[i]);
		sound_effect[i]=NULL;
	}

	return;
}

void play_sdl_sound(int nr,int distance,int angle)
{
	static int sound_channel = 0;
	uint64_t time_start;

	// Check if sound is enabled
	if (!(game_options&GO_SOUND)) return;

    if (nr<1 || nr>=sfx_name_cnt) return;

	// For debugging/optimization
	time_start = SDL_GetTicks64();

#if 0
	note("nr = %d: %s, distance = %d, angle = %d", nr, sfx_name[nr], distance, angle);
#endif

	// Set position of sound relative to where you are
	Mix_SetPosition(sound_channel, angle, distance);

	// Ensure volume is set for channel - Should probably be put elsewhere
	Mix_Volume(sound_channel, sound_volume);

	// Play sound
	Mix_PlayChannel(sound_channel, sound_effect[nr], 0);

	// Increment sound channel so the next sound played is on it's own layer and doesn't cancel this one
	sound_channel++;
	if (sound_channel >= MAX_SOUND_CHANNELS) sound_channel = 0;

	// For debug/optimization
	time_play_sound += SDL_GetTicks64() - time_start;

	return;
}

void play_sound(int nr,int vol,int p) {
    int dist,angle;
    if (!(game_options&GO_SOUND)) return;

    // force volume and pan to sane values
    if (vol>0) vol=0;
    if (vol<-9999) vol=-9999;

    if (p>9999) p=9999;
    if (p<-9999) p=-9999;

    // translate parameters to SDL
    // TODO: change client server protocol to provide angle instead of position
    dist=-(int)(vol)*255.0/10000.0;
    angle=(int)p/10000.0*180.0;

#if 0
	note("nr = %d: %s, distance = %d, angle = %d (vol=%d, p=%d)", nr, sfx_name[nr], dist, angle, vol, p);
#endif

    play_sdl_sound(nr,dist,angle);
}


static char *sfx_name[]={
    "sfx/null.wav",            //0
    "sfx/sdemonawaken.wav",        //1
    "sfx/door.wav",            //2
    "sfx/door2.wav",           //3
    "sfx/man_dead.wav",        //4
    "sfx/thunderrumble3.wav",      //5
    "sfx/explosion.wav",       //6
    "sfx/hit_body2.wav",       //7
    "sfx/miss.wav",            //8
    "sfx/man_hurt.wav",        //9
    "sfx/pigeon.wav",          //10
    "sfx/crow.wav",            //11
    "sfx/crow2.wav",           //12
    "sfx/laughingman6.wav",        //13
    "sfx/drip1.wav",           //14
    "sfx/drip2.wav",           //15
    "sfx/drip3.wav",           //16
    "sfx/howl1.wav",           //17
    "sfx/howl2.wav",           //18
    "sfx/bird1.wav",           //19
    "sfx/bird2.wav",           //20
    "sfx/bird3.wav",           //21
    "sfx/catmeow2.wav",        //22
    "sfx/cricket.wav",         //23
    "sfx/specht.wav",          //24
    "sfx/haeher.wav",          //25
    "sfx/owl1.wav",            //26
    "sfx/owl2.wav",            //27
    "sfx/owl3.wav",            //28
    "sfx/magic.wav",           //29
    "sfx/flash.wav",           //30	lightning strike
    "sfx/scarynote.wav",       //31	freeze
    "sfx/woman_hurt.wav",      //32
    "sfx/woman_dead.wav",      //33
    "sfx/parry1.wav",          //34
    "sfx/parry2.wav",          //35
    "sfx/dungeon_breath1.wav",     //36
    "sfx/dungeon_breath2.wav",     //37
    "sfx/pents_mood1.wav",     //38
    "sfx/pents_mood2.wav",     //39
    "sfx/pents_mood3.wav",     //40
    "sfx/ancient_activate.wav",    //41
    "sfx/pent_activate.wav",       //42
    "sfx/ancient_runout.wav",      //43

    "sfx/bubble1.wav",         //44
    "sfx/bubble2.wav",         //45
    "sfx/bubble3.wav",         //46
    "sfx/whale1.wav",          //47
    "sfx/whale2.wav",          //48
    "sfx/whale3.wav"           //49
};

static int sfx_name_cnt=ARRAYSIZE(sfx_name);
