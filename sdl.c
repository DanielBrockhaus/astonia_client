#include <SDL2/SDL.h>

SDL_Window *sdlwnd;

extern int gfx_force_png;

int sdl_init(int width,int height,char *title) {

    if (SDL_Init(SDL_INIT_VIDEO) != 0){
        printf("SDL_Init Error: %s",SDL_GetError());
	    return 0;
    }

    sdlwnd = SDL_CreateWindow(title, 2560-700, 100, 640, 480, SDL_WINDOW_SHOWN);
    if (!sdlwnd) {
        printf("SDL_Init Error: %s",SDL_GetError());
        SDL_Quit();
	    return 0;
    }

    gfx_force_png=1;    // TODO: remove me, for now SDL only works with PNGs, not with PAKs

    return 1;
}

