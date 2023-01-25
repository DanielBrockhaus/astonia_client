/*
 * Part of Astonia Client (c) Daniel Brockhaus. Please read license.txt.
 *
 * Mod Loading
 *
 * Loads and initializes the amod.
 *
 */

#include <stdint.h>
#include <windows.h>

#include "../../src/astonia.h"
#include "../../src/modder.h"
#include "../../src/modder/_modder.h"
#include "../../src/game.h"
#include "../../src/game/_game.h"

void (*_amod_init)(void)=NULL;
void (*_amod_exit)(void)=NULL;
void (*_amod_gamestart)(void)=NULL;
void (*_amod_frame)(void)=NULL;
void (*_amod_tick)(void)=NULL;

char *game_email_main="<no one>";
char *game_email_cash="<no one>";
char *game_url="<nowhere>";

int amod_init(void) {
	HMODULE dll_instance=NULL;
	void *tmp;

	dll_instance=LoadLibrary("bin\\amod.dll");
    if (!dll_instance) return 0;

    if ((tmp=GetProcAddress(dll_instance,"amod_init"))) _amod_init=tmp;
    if ((tmp=GetProcAddress(dll_instance,"amod_exit"))) _amod_exit=tmp;
    if ((tmp=GetProcAddress(dll_instance,"amod_gamestart"))) _amod_gamestart=tmp;
    if ((tmp=GetProcAddress(dll_instance,"amod_frame"))) _amod_frame=tmp;
    if ((tmp=GetProcAddress(dll_instance,"amod_tick"))) _amod_tick=tmp;

    if ((tmp=GetProcAddress(dll_instance,"is_cut_sprite"))) is_cut_sprite=tmp;
    if ((tmp=GetProcAddress(dll_instance,"is_mov_sprite"))) is_mov_sprite=tmp;
    if ((tmp=GetProcAddress(dll_instance,"is_door_sprite"))) is_door_sprite=tmp;
    if ((tmp=GetProcAddress(dll_instance,"is_yadd_sprite"))) is_yadd_sprite=tmp;
    if ((tmp=GetProcAddress(dll_instance,"get_chr_height"))) get_chr_height=tmp;
    if ((tmp=GetProcAddress(dll_instance,"trans_charno"))) trans_charno=tmp;
    if ((tmp=GetProcAddress(dll_instance,"get_player_sprite"))) get_player_sprite=tmp;
    if ((tmp=GetProcAddress(dll_instance,"trans_csprite"))) trans_csprite=tmp;
    if ((tmp=GetProcAddress(dll_instance,"get_lay_sprite"))) get_lay_sprite=tmp;
    if ((tmp=GetProcAddress(dll_instance,"get_offset_sprite"))) get_offset_sprite=tmp;
    if ((tmp=GetProcAddress(dll_instance,"additional_sprite"))) additional_sprite=tmp;
    if ((tmp=GetProcAddress(dll_instance,"opt_sprite"))) opt_sprite=tmp;

    if ((tmp=GetProcAddress(dll_instance,"game_email_main"))) game_email_main=tmp;
    if ((tmp=GetProcAddress(dll_instance,"game_email_cash"))) game_email_cash=tmp;
    if ((tmp=GetProcAddress(dll_instance,"game_url"))) game_url=tmp;
    if ((tmp=GetProcAddress(dll_instance,"game_rankname"))) game_rankname=tmp;
    if ((tmp=GetProcAddress(dll_instance,"game_rankcount"))) game_rankcount=tmp;

    if (_amod_init) _amod_init();

    return 1;
}

void amod_exit(void) {
    if (_amod_exit) _amod_exit();
}

void amod_gamestart(void) {
    if (_amod_gamestart) _amod_gamestart();
}

void amod_frame(void) {
    if (_amod_frame) _amod_frame();
}

void amod_tick(void) {
    if (_amod_tick) _amod_tick();
}

