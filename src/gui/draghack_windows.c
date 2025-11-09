#include <windows.h>
#include <SDL2/SDL.h>

#include "_gui.h"

void gui_sdl_draghack(void) {
    if (butsel!=-1 && (but[butsel].flags&BUTF_CAPTURE)) {
        SDL_Event eventSink;
        while (SDL_PollEvent(&eventSink)) { /*clear event queue*/ }

        // Windows code to fix dragging bug with window inactive
        INPUT input[2];
        ZeroMemory(input, sizeof(input));

        input[0].type=INPUT_MOUSE;
        input[0].mi.dwFlags=MOUSEEVENTF_LEFTUP;

        input[1].type=INPUT_MOUSE;
        input[1].mi.dwFlags=MOUSEEVENTF_LEFTDOWN;

        SendInput(2, input, sizeof(INPUT)); //note: SDL_PushEvent doesn't work
    }
}
