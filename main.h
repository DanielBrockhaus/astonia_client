/*
 * Part of Astonia Client (c) Daniel Brockhaus. Please read license.txt.
 */


#define DEVELOPER               // this one will compile the developer version - comment me out for the final release

#define PARANOIA(a) a
#define DOSOUND
//#define RELAY

extern int panic_reached;
extern int quit;

// helper

#define sign_p(v)       ((v)>=0?1:-1)
#define sign_n(v)       ((v)>0?1:(v)<0?-1:0)
#ifndef ARRAYSIZE
#define ARRAYSIZE(a) (sizeof(a)/sizeof((a)[0]))
#endif

// io
int rread(int fd,void *ptr,int size);
char* load_ascii_file(char *filename,int ID);

// memory

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
#define MEM_TEMP4       18
#define MEM_TEMP5       19
#define MEM_TEMP6       20
#define MEM_TEMP7       21
#define MEM_TEMP8       22
#define MEM_TEMP9       23
#define MEM_TEMP10      24
#define MAX_MEM         25

int xmemcheck(void *ptr);
void* xmalloc(int size,int ID);
void* xcalloc(int size,int ID);
void* xrealloc(void *ptr,int size,int ID);
void* xrecalloc(void *ptr,int size,int ID);
void xfree(void *ptr);
char* xstrdup(const char *src,int ID);
void list_mem(void);

void addptr(void ***list,int *count,void *ptr,int ID);
void delptr(void ***list,int *count,void *ptr,int ID);

// random

void rrandomize(void);
void rseed(int seed);
int rrand(int range);

// messages

int  note(const char *format,...) __attribute__((format(printf, 1, 2)));
int  warn(const char *format,...) __attribute__((format(printf, 1, 2)));
int  fail(const char *format,...) __attribute__((format(printf, 1, 2)));
void paranoia(const char *format,...) __attribute__((format(printf, 1, 2)));

void addline(const char *format,...) __attribute__((format(printf, 1, 2)));

// misc
void save_options(void);

