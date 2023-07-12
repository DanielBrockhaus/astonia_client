/*
 * Part of Astonia Client (c) Daniel Brockhaus. Please read license.txt.
 */

#define DEVELOPER               // this one will compile the developer version - comment me out for the final release

//#define TICKPRINT

#define SDL_FAST_MALLOC     // will use the C library instead of the error-checking client version

#define MAXSPRITE 250000

#define TICKS           24                      // ticks (game data updates) per second
#define FRAMES          (frames_per_second)     // frames (display updates) per second
#define MPT             (1000/TICKS)            // milliseconds per tick
#define MPF             (1000/FRAMES)           // milliseconds per frame

#define DIST		    25
#define FDX             40      // width of a map tile
#define FDY             20      // height of a map tile

#define XRES    800
#define YRES    (__yres)
#define YRES0   600
#define YRES1   650
#define YRES2   500

#define PARANOIA(a) a

#define bzero(ptr,size) memset(ptr,0,size)

#define MEM_NONE        0
#define MEM_GLOB        1
#define MEM_TEMP        2
#define MEM_ELSE        3
#define MEM_DL          4
#define MEM_IC          5
#define MEM_SC          6
#define MEM_VC          7
#define MEM_PC          8
#define MEM_GUI         9
#define MEM_GAME        10
#define MEM_TEMP11      11
#define MEM_VPC         12
#define MEM_VSC         13
#define MEM_VLC         14
#define MEM_SDL_BASE    15
#define MEM_SDL_PIXEL   16
#define MEM_SDL_PNG     17
#define MEM_SDL_PIXEL2  18
#define MEM_TEMP5       19
#define MEM_TEMP6       20
#define MEM_TEMP7       21
#define MEM_TEMP8       22
#define MEM_TEMP9       23
#define MEM_TEMP10      24
#define MAX_MEM         25

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef abs
#define abs(a)	((a)<0 ? (-(a)) : (a))
#endif

#ifndef ARRAYSIZE
#define ARRAYSIZE(a) (sizeof(a)/sizeof((a)[0]))
#endif

#define DOT_TL          0       // top left?
#define DOT_BR          1       // bottom right?
#define DOT_WEA         2       // worn equipment
#define DOT_INV         3       // inventory
#define DOT_CON         4       // container
#define DOT_SCL         5       // scroll bar left, uses only X
#define DOT_SCR         6       // scroll bar right, uses only X
#define DOT_SCU         7       // scroll bars up arrows at this Y
#define DOT_SCD         8       // scroll bars down arrors at thy Y
#define DOT_TXT         9       // chat window
#define DOT_MTL         10      // map top left
#define DOT_MBR         11      // map bottom right
#define DOT_SKL         12      // skill list
#define DOT_GLD         13      // gold
#define DOT_JNK         14      // trashcan
#define DOT_MOD         15      // speed mode
#define DOT_MCT         16      // map center
#define DOT_TOP         17      // top left corner of equipment bar
#define DOT_BOT         18      // top left corner of bottom window holding skills, chat, etc.
#define DOT_TX2         19      // chat window bottom right
#define DOT_SK2         20      // skill list window bottom right
#define DOT_IN1         21      // inventory top left
#define DOT_IN2         22      // inventory bottom right
#define DOT_HLP         23      // help top left
#define DOT_HL2         24      // help bottom right
#define DOT_TEL         25      // teleporter top left
#define DOT_COL         26      // color picker top left
#define DOT_LOK         27      // look at character window (show_look), top left
#define DOT_BO2         28      // bottom right of bottom window
#define DOT_ACT         29      // action bar top left
#define DOT_SSP         30      // self-spell-bars top left
#define DOT_TUT         31      // tutor window top left
#define MAX_DOT         32

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

extern int __yres;
extern int quit;
extern int frames_per_second;
extern char localdata[MAX_PATH];

#define GO_DARK     (1ull<<0)  // Dark GUI by Tegra
#define GO_CONTEXT  (1ull<<1)  // Right-Click Context Menu
#define GO_ACTION   (1ull<<2)  // Action Bar and Key Bindings
#define GO_SMALLBOT (1ull<<3)  // Smaller Bottom Window
#define GO_SMALLTOP (1ull<<4)  // Smaller Top Window
#define GO_BIGBAR   (1ull<<5)  // Show big health bar etc.
#define GO_SOUND    (1ull<<6)  // Enable sound
#define GO_LARGE    (1ull<<7)  // Use large font
#define GO_FULL     (1ull<<8)  // Use true full screen mode
#define GO_WHEEL    (1ull<<9)  // Use old mouse wheel logic
#define GO_PREDICT  (1ull<<10) // Process some commands early for faster responses (prefetch() instead of process())
#define GO_SHORT    (1ull<<11) // Less command delay, more stutter in animations
#define GO_APPDATA  (1ull<<12) // Use Windows %appdata% to store configuration and logs
#define GO_MAPSAVE  (1ull<<13) // Load/Save minimap data
#define GO_LIGHTER  (1ull<<14) // Gamma increase, sort of
#define GO_LIGHTER2 (1ull<<15) // More gamma increase
#define GO_TINYTOP  (1ull<<16) // Slide out top only when mouse cursor is over window border

#define GO_NOTSET   (1ull<<63) // No -o given on command line

extern uint64_t game_options;

void addline(const char *format,...) __attribute__((format(printf, 1, 2)));
int  note(const char *format,...) __attribute__((format(printf, 1, 2)));
int  warn(const char *format,...) __attribute__((format(printf, 1, 2)));
int  fail(const char *format,...) __attribute__((format(printf, 1, 2)));
void paranoia(const char *format,...) __attribute__((format(printf, 1, 2)));
void display_messagebox(char *title,char *text);

void* xmalloc(int size,int ID);
void* xcalloc(int size,int ID);
void* xrealloc(void *ptr,int size,int ID);
void* xrecalloc(void *ptr,int size,int ID);
void xfree(void *ptr);
char* xstrdup(const char *src,int ID);

int rrand(int range);

void init_dots(void);
int dotx(int didx);
int doty(int didx);
int butx(int bidx);
int buty(int bidx);

void dd_set_offset(int x,int y);


