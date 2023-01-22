/*
 * Part of Astonia Client (c) Daniel Brockhaus. Please read license.txt.
 */

extern int enable_sound;
extern int sound_volume;

#define MAX_SOUND_CHANNELS   32
#define MAXSOUND            100

int init_sound(void);
void sound_exit();
void play_sound(int nr,int vol,int p);

