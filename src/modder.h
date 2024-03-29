/*
 * Part of Astonia Client (c) Daniel Brockhaus. Please read license.txt.
 */

#define MAXMOD  6

int amod_init(void);
void amod_exit(void);
char *amod_version(int idx);
void amod_gamestart(void);
void amod_frame(void);
void amod_tick(void);
int amod_display_skill_line(int v,int base,int curr,int cn,char *buf);
void amod_mouse_move(int x,int y);
int amod_mouse_click(int x,int y,int what);
void amod_mouse_capture(int onoff);
void amod_areachange(void);
int amod_keydown(int key);
int amod_keyup(int key);
int amod_textinput(int key);
void amod_update_hover_texts(void);
int amod_client_cmd(char *buf);
int amod_process(char *buf);
int amod_prefetch(char *buf);
int amod_is_playersprite(int sprite);

int sharedmem_init(void);
void sharedmem_update(void);
void sharedmem_exit(void);

extern char *game_email_main;
extern char *game_email_cash;
extern char *game_url;
extern char **game_rankname;
extern int *game_rankcount;

