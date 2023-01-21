
//#define TICKPRINT
#define SDL_FAST_MALLOC     // will use the C library instead of the error-checking client version

#define SDL_MOUM_NONE       0
#define SDL_MOUM_LUP        1
#define SDL_MOUM_LDOWN      2
#define SDL_MOUM_RUP        3
#define SDL_MOUM_RDOWN      4
#define SDL_MOUM_MUP        5
#define SDL_MOUM_MDOWN      6
#define SDL_MOUM_WHEEL      7

void dd_text_lineup(void);
void dd_text_linedown(void);

extern int sdl_cache_size;
extern int sdl_scale;
extern int sdl_frames;
extern int sdl_multi;
extern int sdl_fullscreen;
extern int frames_per_second;


