
#define GFXPATH "../gfx/"

#define MAXSPRITE 250000

int sdl_init(int width,int height,char *title);
int sdl_clear(void);
int sdl_render(void);

int sdl_ic_load(int sprite);
void sdl_blit(int sprite,int scrx,int scry);

