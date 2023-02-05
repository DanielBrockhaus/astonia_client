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
#include "../../src/client.h"
#include "../../src/gui.h"

void (*_amod_init)(void)=NULL;
void (*_amod_exit)(void)=NULL;
void (*_amod_gamestart)(void)=NULL;
void (*_amod_frame)(void)=NULL;
void (*_amod_tick)(void)=NULL;
int (*_amod_display_skill_line)(int v,int base,int curr,int cn,char *buf)=NULL;
void (*_amod_mouse_move)(int x,int y)=NULL;
int (*_amod_mouse_click)(int x,int y,int what)=NULL;

char *game_email_main="<no one>";
char *game_email_cash="<no one>";
char *game_url="<nowhere>";

int amod_init(void) {
	HMODULE dll_instance=NULL;
	void *tmp;

	dll_instance=LoadLibrary("bin\\amod.dll");
    if (!dll_instance) return 0;

    // amod
    if ((tmp=GetProcAddress(dll_instance,"amod_init"))) _amod_init=tmp;
    if ((tmp=GetProcAddress(dll_instance,"amod_exit"))) _amod_exit=tmp;
    if ((tmp=GetProcAddress(dll_instance,"amod_gamestart"))) _amod_gamestart=tmp;
    if ((tmp=GetProcAddress(dll_instance,"amod_frame"))) _amod_frame=tmp;
    if ((tmp=GetProcAddress(dll_instance,"amod_tick"))) _amod_tick=tmp;
    if ((tmp=GetProcAddress(dll_instance,"amod_display_skill_line"))) _amod_display_skill_line=tmp;
    if ((tmp=GetProcAddress(dll_instance,"amod_mouse_move"))) _amod_mouse_move=tmp;
    if ((tmp=GetProcAddress(dll_instance,"amod_mouse_click"))) _amod_mouse_click=tmp;

    // client functions
    if ((tmp=GetProcAddress(dll_instance,"is_cut_sprite"))) is_cut_sprite=tmp;
    if ((tmp=GetProcAddress(dll_instance,"is_mov_sprite"))) is_mov_sprite=tmp;
    if ((tmp=GetProcAddress(dll_instance,"is_door_sprite"))) is_door_sprite=tmp;
    if ((tmp=GetProcAddress(dll_instance,"is_yadd_sprite"))) is_yadd_sprite=tmp;
    if ((tmp=GetProcAddress(dll_instance,"get_chr_height"))) get_chr_height=tmp;
    if ((tmp=GetProcAddress(dll_instance,"trans_asprite"))) trans_asprite=tmp;
    if ((tmp=GetProcAddress(dll_instance,"trans_charno"))) trans_charno=tmp;
    if ((tmp=GetProcAddress(dll_instance,"get_player_sprite"))) get_player_sprite=tmp;
    if ((tmp=GetProcAddress(dll_instance,"trans_csprite"))) trans_csprite=tmp;
    if ((tmp=GetProcAddress(dll_instance,"get_lay_sprite"))) get_lay_sprite=tmp;
    if ((tmp=GetProcAddress(dll_instance,"get_offset_sprite"))) get_offset_sprite=tmp;
    if ((tmp=GetProcAddress(dll_instance,"additional_sprite"))) additional_sprite=tmp;
    if ((tmp=GetProcAddress(dll_instance,"opt_sprite"))) opt_sprite=tmp;
    if ((tmp=GetProcAddress(dll_instance,"get_skltab_index"))) get_skltab_index=tmp;
    if ((tmp=GetProcAddress(dll_instance,"get_skltab_sep"))) get_skltab_sep=tmp;
    if ((tmp=GetProcAddress(dll_instance,"get_skltab_show"))) get_skltab_show=tmp;
    if ((tmp=GetProcAddress(dll_instance,"do_display_random"))) do_display_random=tmp;

    // client variables
    if ((tmp=GetProcAddress(dll_instance,"game_email_main"))) game_email_main=tmp;
    if ((tmp=GetProcAddress(dll_instance,"game_email_cash"))) game_email_cash=tmp;
    if ((tmp=GetProcAddress(dll_instance,"game_url"))) game_url=tmp;
    if ((tmp=GetProcAddress(dll_instance,"game_rankname"))) game_rankname=tmp;
    if ((tmp=GetProcAddress(dll_instance,"game_rankcount"))) game_rankcount=tmp;
    if ((tmp=GetProcAddress(dll_instance,"game_v_max"))) game_v_max=tmp;
    if ((tmp=GetProcAddress(dll_instance,"game_skill"))) game_skill=tmp;
    if ((tmp=GetProcAddress(dll_instance,"game_skilldesc"))) game_skilldesc=tmp;
    if ((tmp=GetProcAddress(dll_instance,"game_v_profbase"))) game_v_profbase=tmp;

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

int amod_display_skill_line(int v,int base,int curr,int cn,char *buf) {
    if (_amod_display_skill_line) return _amod_display_skill_line(v,base,curr,cn,buf);
    return 0;
}

void amod_mouse_move(int x,int y) {
    if (_amod_mouse_move) _amod_mouse_move(x,y);
}

int amod_mouse_click(int x,int y,int what) {
    if (_amod_mouse_click) return _amod_mouse_click(x,y,what);
    return 0;
}
