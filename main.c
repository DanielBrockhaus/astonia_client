/*
 * Part of Astonia Client (c) Daniel Brockhaus. Please read license.txt.
 */

#include <windows.h>
#include <commctrl.h>
#include <stdarg.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <time.h>
#include <dsound.h>

#define ISCLIENT
#include "main.h"
#include "client.h"
#include "dd.h"
#include "resource.h"
#include "sound.h"
#include "gui.h"
#include "sdl.h"

// extern

extern long main_wnd_proc(HWND wnd,UINT msg,WPARAM wparam,LPARAM lparam) __attribute__((stdcall));
extern int main_init(void);
extern void main_exit(void);

int main_loop(void);

// globs

HICON mainicon;

int quit=0;
int quickstart=0;
int panic_reached=0;
int xmemcheck_failed=0;
HINSTANCE instance=NULL;
int opt_res=800;
int largetext=0;
int vendor=1;
extern int newlight;
char user_keys[10]={'Q','W','E','A','S','D','Z','X','C','V'};

char memcheck_failed_str[]={"TODO: memcheck failed"};  // TODO
char panic_reached_str[]={"TODO: panic failure"}; // TODO

int MAXLINESHOW=15;

FILE *errorfp;

// note, warn, fail, paranoia, addline

int note(const char *format,...) {
    va_list va;
    char buf[1024];


    va_start(va,format);
    vsprintf(buf,format,va);
    va_end(va);

    printf("NOTE: %s\n",buf);
#ifdef DEVELOPER
    addline("NOTE: %s\n",buf);
#endif

    return 0;
}

int warn(const char *format,...) {
    va_list va;
    char buf[1024];


    va_start(va,format);
    vsprintf(buf,format,va);
    va_end(va);

    printf("WARN: %s\n",buf);
    addline("WARN: %s\n",buf);

    return 0;
}

int fail(const char *format,...) {
    va_list va;
    char buf[1024];


    va_start(va,format);
    vsprintf(buf,format,va);
    va_end(va);

    fprintf(errorfp,"FAIL: %s\n",buf);
    addline("FAIL: %s\n",buf);

    return -1;
}

void paranoia(const char *format,...) {
    va_list va;

    fprintf(errorfp,"PARANOIA EXIT in ");

    va_start(va,format);
    vfprintf(errorfp,format,va);
    va_end(va);

    fprintf(errorfp,"\n");
    fflush(errorfp);

    exit(-1);
}

static int _addlinesep=0;

void addlinesep(void) {
    _addlinesep=1;
}

void addline(const char *format,...) {
    va_list va;
    char buf[1024];

    if (_addlinesep) {
        _addlinesep=0;
        addline("-------------");
    }

    va_start(va,format);
    vsnprintf(buf,sizeof(buf)-1,format,va); buf[sizeof(buf)-1]=0;
    va_end(va);

    if (dd_text_init_done()) dd_add_text(buf);
}

// io

int rread(int fd,void *ptr,int size) {
    int n;

    while (size>0) {
        n=read(fd,ptr,size);
        if (n<0) return -1;
        if (n==0) return 1;
        size-=n;
        ptr=((unsigned char *)(ptr))+n;
    }
    return 0;
}

char* load_ascii_file(char *filename,int ID) {
    int fd,size;
    char *ptr;

    if ((fd=open(filename,O_RDONLY|O_BINARY))==-1) return NULL;
    if ((size=lseek(fd,0,SEEK_END))==-1) { close(fd); return NULL; }
    if (lseek(fd,0,SEEK_SET)==-1) { close(fd); return NULL; }
    ptr=xmalloc(size+1,ID);
    if (rread(fd,ptr,size)) { xfree(ptr); close(fd); return NULL; }
    ptr[size]=0;
    close(fd);

    return ptr;
}

// memory

//#define malloc_proc(size) GlobalAlloc(GPTR,size)
//#define realloc_proc(ptr,size) GlobalReAlloc(ptr,size,GMEM_MOVEABLE)
//#define free_proc(ptr) GlobalFree(ptr)

HANDLE myheap;
#define malloc_proc(size) myheapalloc(size)
#define realloc_proc(ptr,size) myheaprealloc(ptr,size)
#define free_proc(ptr) myheapfree(ptr)

int memused=0;
int memptrused=0;

int maxmemsize=0;
int maxmemptrs=0;
int memptrs[MAX_MEM];
int memsize[MAX_MEM];

struct memhead {
    int size;
    int ID;
};

static char *memname[MAX_MEM]={
    "MEM_TOTA", //0
    "MEM_GLOB",
    "MEM_TEMP",
    "MEM_ELSE",
    "MEM_DL",
    "MEM_IC",   //5
    "MEM_SC",
    "MEM_VC",
    "MEM_PC",
    "MEM_GUI",
    "MEM_GAME", //10
    "MEM_TEMP11",
    "MEM_VPC",
    "MEM_VSC",
    "MEM_VLC",
    "MEM_SDL_BASE",
    "MEM_SDL_PIXEL",
    "MEM_SDL_PNG",
    "MEM_TEMP4",
    "MEM_TEMP5",
    "MEM_TEMP6",
    "MEM_TEMP7",
    "MEM_TEMP8",
    "MEM_TEMP9",
    "MEM_TEMP10"
};

void* myheapalloc(int size) {
    void *ptr;

    memptrused++;
    ptr=HeapAlloc(myheap,HEAP_ZERO_MEMORY,size);
    memused+=HeapSize(myheap,0,ptr);

    return ptr;
}

void* myheaprealloc(void *ptr,int size) {
    memused-=HeapSize(myheap,0,ptr);
    ptr=HeapReAlloc(myheap,HEAP_ZERO_MEMORY,ptr,size);
    memused+=HeapSize(myheap,0,ptr);

    return ptr;
}

void myheapfree(void *ptr) {
    memptrused--;
    memused-=HeapSize(myheap,0,ptr);
    HeapFree(myheap,0,ptr);
}

void list_mem(void) {
    int i,flag=0;
    MEMORYSTATUS ms;

    note("--mem----------------------");
    for (i=1; i<MAX_MEM; i++) {
        if (memsize[i] || memptrs[i])  {
            flag=1;
            note("%s %.2fMB in %d ptrs",memname[i],memsize[i]/(1024.0*1024.0),memptrs[i]);
        }
    }
    if (flag) note("%s %.2fMB in %d ptrs",memname[0],memsize[0]/(1024.0*1024.0),memptrs[0]);
    note("%s %.2fMB in %d ptrs","MEM_MAX",maxmemsize/(1024.0*1024.0),maxmemptrs);
    note("---------------------------");

    bzero(&ms,sizeof(ms));
    ms.dwLength=sizeof(ms);
    GlobalMemoryStatus(&ms);
    note("availphys=%.2fM",ms.dwAvailPhys/1024.0/1024.0);
    note("UsedMem=%.2fM / %d",memused/1024.0/1024.0,memptrused);

    note("validate says: %d",HeapValidate(myheap,0,NULL));

}

static int memcheckset=0;
static unsigned char memcheck[256];

int xmemcheck(void *ptr) {
    struct memhead *mem;
    unsigned char *head,*tail,*rptr;

    if (!ptr) return 0;

    mem=(struct memhead *)(((unsigned char *)(ptr))-8-sizeof(memcheck));

    // ID check
    if (mem->ID>=MAX_MEM) { note("xmemcheck: ill mem id (%d)",mem->ID); xmemcheck_failed=1; return -1; }

    // border check
    head=((unsigned char *)(mem))+8;
    rptr=((unsigned char *)(mem))+8+sizeof(memcheck);
    tail=((unsigned char *)(mem))+8+sizeof(memcheck)+mem->size;

    if (memcmp(head,memcheck,sizeof(memcheck))) { fail("xmemcheck: ill head in %s (ptr=%08X)",memname[mem->ID],rptr); xmemcheck_failed=1; return -1; }
    if (memcmp(tail,memcheck,sizeof(memcheck))) { fail("xmemcheck: ill tail in %s (ptr=%08X)",memname[mem->ID],rptr); xmemcheck_failed=1; return -1; }

    return 0;
}

void* xmalloc(int size,int ID) {
    struct memhead *mem;
    unsigned char *head,*tail,*rptr;

    if (!memcheckset) {
        note("initialized memcheck");
        for (memcheckset=0; memcheckset<sizeof(memcheck); memcheckset++) memcheck[memcheckset]=rrand(256);
        sprintf(memcheck,"!MEMCKECK MIGHT FAIL!");
        myheap=HeapCreate(0,0,0);
    }

    if (!size) return NULL;

    mem=malloc_proc(8+sizeof(memcheck)+size+sizeof(memcheck));
    if (!mem) { fail("OUT OF MEMORY !!!"); return NULL; }

    if (ID>=MAX_MEM) { fail("xmalloc: ill mem id"); return NULL; }

    mem->ID=ID;
    mem->size=size;
    memsize[mem->ID]+=mem->size;
    memptrs[mem->ID]+=1;
    memsize[0]+=mem->size;
    memptrs[0]+=1;

    if (memsize[0]>maxmemsize) maxmemsize=memsize[0];
    if (memptrs[0]>maxmemptrs) maxmemptrs=memptrs[0];

    head=((unsigned char *)(mem))+8;
    rptr=((unsigned char *)(mem))+8+sizeof(memcheck);
    tail=((unsigned char *)(mem))+8+sizeof(memcheck)+mem->size;

    // set memcheck
    memcpy(head,memcheck,sizeof(memcheck));
    memcpy(tail,memcheck,sizeof(memcheck));

    xmemcheck(rptr);

    return rptr;
}

void* xcalloc(int size,int ID) {
    void *ptr;

    ptr=xmalloc(size,ID);
    if (ptr) bzero(ptr,size);
    return ptr;
}

char* xstrdup(const char *src,int ID) {
    int size;
    char *dst;

    size=strlen(src)+1;

    dst=xmalloc(size,ID);
    if (!dst) return NULL;

    memcpy(dst,src,size);

    return dst;
}


void xfree(void *ptr) {
    struct memhead *mem;

    if (!ptr) return;
    if (xmemcheck(ptr)) return;

    // get mem
    mem=(struct memhead *)(((unsigned char *)(ptr))-8-sizeof(memcheck));

    // free
    memsize[mem->ID]-=mem->size;
    memptrs[mem->ID]-=1;
    memsize[0]-=mem->size;
    memptrs[0]-=1;

    free_proc(mem);
}

void xinfo(void *ptr) {
    struct memhead *mem;

    if (!ptr) { printf("NULL"); return; }
    if (xmemcheck(ptr)) { printf("ILL"); return; }

    // get mem
    mem=(struct memhead *)(((unsigned char *)(ptr))-8-sizeof(memcheck));

    printf("%d bytes",mem->size);
}

void* xrealloc(void *ptr,int size,int ID) {
    struct memhead *mem;
    unsigned char *head,*tail,*rptr;

    if (!ptr) return xmalloc(size,ID);
    if (!size) { xfree(ptr); return NULL; }
    if (xmemcheck(ptr)) return NULL;

    mem=(struct memhead *)(((unsigned char *)(ptr))-8-sizeof(memcheck));

    // realloc
    memsize[mem->ID]-=mem->size;
    memptrs[mem->ID]-=1;
    memsize[0]-=mem->size;
    memptrs[0]-=1;

    mem=realloc_proc(mem,8+sizeof(memcheck)+size+sizeof(memcheck));
    if (!mem) { fail("xrealloc: OUT OF MEMORY !!!"); return NULL; }

    mem->ID=ID;
    mem->size=size;
    memsize[mem->ID]+=mem->size;
    memptrs[mem->ID]+=1;
    memsize[0]+=mem->size;
    memptrs[0]+=1;

    if (memsize[0]>maxmemsize) maxmemsize=memsize[0];
    if (memptrs[0]>maxmemptrs) maxmemptrs=memptrs[0];

    head=((unsigned char *)(mem))+8;
    rptr=((unsigned char *)(mem))+8+sizeof(memcheck);
    tail=((unsigned char *)(mem))+8+sizeof(memcheck)+mem->size;

    // set memcheck
    memcpy(head,memcheck,sizeof(memcheck));
    memcpy(tail,memcheck,sizeof(memcheck));

    return rptr;
}

void* xrecalloc(void *ptr,int size,int ID) {
    struct memhead *mem;
    unsigned char *head,*tail,*rptr;

    if (!ptr) return xcalloc(size,ID);
    if (!size) { xfree(ptr); return NULL; }
    if (xmemcheck(ptr)) return NULL;

    mem=(struct memhead *)(((unsigned char *)(ptr))-8-sizeof(memcheck));

    // realloc
    memsize[mem->ID]-=mem->size;
    memptrs[mem->ID]-=1;
    memsize[0]-=mem->size;
    memptrs[0]-=1;

    mem=realloc_proc(mem,8+sizeof(memcheck)+size+sizeof(memcheck));
    if (!mem) { fail("xrecalloc: OUT OF MEMORY !!!"); return NULL; }

    if (size-mem->size>0) {
        bzero(((unsigned char *)(mem))+8+sizeof(memcheck)+mem->size,size-mem->size);
    }

    mem->ID=ID;
    mem->size=size;
    memsize[mem->ID]+=mem->size;
    memptrs[mem->ID]+=1;
    memsize[0]+=mem->size;
    memptrs[0]+=1;

    if (memsize[0]>maxmemsize) maxmemsize=memsize[0];
    if (memptrs[0]>maxmemptrs) maxmemptrs=memptrs[0];

    head=((unsigned char *)(mem))+8;
    rptr=((unsigned char *)(mem))+8+sizeof(memcheck);
    tail=((unsigned char *)(mem))+8+sizeof(memcheck)+mem->size;

    // set memcheck
    memcpy(head,memcheck,sizeof(memcheck));
    memcpy(tail,memcheck,sizeof(memcheck));

    return rptr;
}

void addptr(void ***list,int *count,void *ptr,int ID) {
    (*count)++;
    (*list)=(void **)xrealloc((*list),(*count)*sizeof(void *),ID);
    (*list)[*count-1]=ptr;
}

void delptr(void ***list,int *count,void *ptr,int ID) {
    int i;

    for (i=0; i<(*count); i++) if ((*list)[i]==ptr) break;
    if (i==(*count)) return;
    memmove(&(*list)[i],&(*list)[i+1],((*count)-i-1)*sizeof(void *));
    (*count)--;
    (*list)=(void **)xrealloc((*list),(*count)*sizeof(void *),ID);
}

// rrandom

void rrandomize(void) {
    srand(time(NULL));
}

void rseed(int seed) {
    srand(seed);
}

int rrand(int range) {
    int r;

    r=rand();
    return (range*r/(RAND_MAX+1));
}

// wsa network

int net_init(void) {
    WSADATA wsadata;

    if (WSAStartup(0x0002,&wsadata)) return -1;
    return 0;
}

int net_exit(void) {
    WSACleanup();
    return 0;
}

// parsing command line

char with_cmd;
int with_nr;

char filmfilename[1024];

unsigned int validate_intra(unsigned int ip) {
    if ((ip&0xff000000)==0x7f000000) return ip;
    if ((ip&0xff000000)==0x0a000000) return ip;
    if ((ip&0xffff0000)==0xac100000) return ip;
    if ((ip&0xffff0000)==0xc0a80000) return ip;

    return 0;
}

int parse_cmd(char *s) {
    int n;
    char *end;

    while (isspace(*s)) s++;
    //while (*s && *s!='-') s++;

    while (*s) {
        if (*s=='-') {
            s++;
            if (tolower(*s)=='u') {         // -u <username>
                s++;
                while (isspace(*s)) s++;
                n=0; while (n<40 && *s && !isspace(*s))	username[n++]=*s++;
                username[n]=0;
                quickstart=1;
            } else if (tolower(*s)=='p') {  // -p <password>
                s++;
                while (isspace(*s)) s++;
                n=0; while (n<16 && *s && !isspace(*s)) password[n++]=*s++;
                password[n]=0;
            } else if (tolower(*s)=='d') {
                s++;
                developer_server=strtol(s+1,&end,10);
                s=end;;
                if (developer_server!=554433) developer_server=0;
            } else if (tolower(*s)=='w') {    // -w vertical_resolution, currently supporting 600, 900, 1200 and 1800.
                    int tmp;
                    s++;
                    tmp=strtol(s+1,&end,10);
                    s=end;;

                    if (tmp==900) opt_res=IDC_RES1200;
                    else if (tmp==1200) opt_res=IDC_RES1600;
                    else if (tmp==1800) opt_res=IDC_RES2400;
                    else opt_res=IDC_RES800;
            } else if (tolower(*s)=='l') { //Large Text
                    s++;
                    largetext=1;
            } else if (tolower(*s)=='y') {
                extern unsigned int sock_server;
                s++;
                while (isspace(*s)) s++;
                sock_server=validate_intra(ntohl(inet_addr(s)));
                while (*s && *s!=' ') s++;
            } else if (tolower(*s)=='z') {
                extern unsigned short sock_port;
                s++;
                while (isspace(*s)) s++;
                sock_port=atoi(s);
                while (*s && *s!=' ') s++;
            } else if (tolower(*s)=='h') {
                extern char sock_user[80];
                int n=0;
                s++;
                while (isspace(*s)) s++;
                while (*s && !isspace(*s) && n<79) sock_user[n++]=*s++;
                sock_user[n]=0;
                while (*s && *s!=' ') s++;
            } else { printf("haha\n"); return -1; }
        } else { printf("huha\n"); return -2; }
        while (isspace(*s)) s++;
    }
    return 0;
}

// windows

int main_wnd_proc_safe(HWND wnd,UINT msg,WPARAM wparam,LPARAM lparam) {
    switch (msg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            quit=1;
            return 0;
        case WM_KEYDOWN:
            DestroyWindow(wnd);
            return 0;

    }
    return DefWindowProc(wnd,msg,wparam,lparam);
}

// startup

void update_res(HWND hwnd) {
    if (opt_res==IDC_RES800) CheckRadioButton(hwnd,IDC_RES800,IDC_RES2400,IDC_RES800);
    else if (opt_res==IDC_RES1200) CheckRadioButton(hwnd,IDC_RES800,IDC_RES2400,IDC_RES1200);
    else if (opt_res==IDC_RES1600) CheckRadioButton(hwnd,IDC_RES800,IDC_RES2400,IDC_RES1600);
    else if (opt_res==IDC_RES2400) CheckRadioButton(hwnd,IDC_RES800,IDC_RES2400,IDC_RES2400);
    else { CheckRadioButton(hwnd,IDC_RES800,IDC_RES2400,IDC_RES800); opt_res=IDC_RES800; }

}

int save_pwd=0;
void update_savepwd(HWND hwnd) {
    CheckDlgButton(hwnd,IDC_SAVEPWD,save_pwd);
}

int enable_sound=0;
void update_sound(HWND hwnd) {
    CheckDlgButton(hwnd,IDC_SOUND,enable_sound);
}

void update_newlight(HWND hwnd) {
    CheckDlgButton(hwnd,IDC_NEWLIGHT,newlight);
}


void update_large(HWND hwnd) {
    CheckDlgButton(hwnd,IDC_LARGETEXT,largetext);
}

void save_options(void) {
    int handle;
    int dummy;
    char nullbuff[sizeof(password)];

    bzero(nullbuff,sizeof(nullbuff));

    handle=open("conflict.dat",O_WRONLY|O_CREAT|O_TRUNC|O_BINARY,0666);
    if (handle==-1) return;

    write(handle,&dummy,sizeof(dummy));
    write(handle,&dummy,sizeof(dummy));
    write(handle,&dummy,sizeof(dummy));
    write(handle,username,sizeof(username));
    if (save_pwd) write(handle,password,sizeof(password));
    else write(handle,nullbuff,sizeof(password));
    write(handle,&save_pwd,sizeof(save_pwd));
    write(handle,&opt_res,sizeof(opt_res));
    write(handle,&dummy,sizeof(dummy));
    write(handle,&dummy,sizeof(dummy));
    write(handle,&dummy,sizeof(dummy));
    write(handle,&dummy,sizeof(dummy));
    write(handle,&largetext,sizeof(largetext));
    write(handle,&enable_sound,sizeof(enable_sound));
    write(handle,&user_keys,sizeof(user_keys));
    write(handle,&dummy,sizeof(dummy));
    write(handle,&newlight,sizeof(newlight));
    close(handle);
}

void load_options(void) {
    int handle,len,dummy;
    char buf[80];

    handle=open("vendor.dat",O_RDONLY|O_BINARY);
    if (handle!=-1) {
        len=read(handle,buf,sizeof(buf)-1);
        buf[len]=0;
        vendor=atoi(buf);
        close(handle);
    }

    handle=open("conflict.dat",O_RDONLY|O_BINARY);
    if (handle==-1) return;

    newlight=1;

    read(handle,&dummy,sizeof(dummy));
    read(handle,&dummy,sizeof(dummy));
    read(handle,&dummy,sizeof(dummy));
    read(handle,username,sizeof(username));
    read(handle,password,sizeof(password));
    read(handle,&save_pwd,sizeof(save_pwd));
    read(handle,&opt_res,sizeof(opt_res));
    read(handle,&dummy,sizeof(dummy));
    read(handle,&dummy,sizeof(dummy));
    read(handle,&dummy,sizeof(dummy));
    read(handle,&dummy,sizeof(dummy));
    read(handle,&largetext,sizeof(largetext));
    read(handle,&enable_sound,sizeof(enable_sound));
    read(handle,&user_keys,sizeof(user_keys));
    read(handle,&dummy,sizeof(dummy));
    read(handle,&newlight,sizeof(newlight));
    close(handle);
}

int check_username(char *ptr) {
    while (*ptr) if (isdigit(*ptr)) return 0;
        else ptr++;

    return 1;
}


BOOL WINAPI start_dlg_proc(HWND wnd,UINT msg,WPARAM wparam,LPARAM lparam) {
    char buf[80];

    switch (msg) {
        case WM_INITDIALOG:
            SetClassLong(wnd,GCL_HICON,(long)LoadIcon(instance,MAKEINTRESOURCE(1)));
            sprintf(buf,"Astonia 3 v%d.%02d.%02d",(VERSION>>16)&255,(VERSION>>8)&255,(VERSION)&255);
            SetWindowText(wnd,buf);
            SetDlgItemText(wnd,IDC_USERNAME,username);
            SetDlgItemText(wnd,IDC_PASSWORD,password);
            CreateWindowEx(WS_EX_STATICEDGE,"STATIC",NULL,SS_BITMAP|WS_VISIBLE|WS_CHILD,0,0,0,0,wnd,(HMENU)1999,instance,NULL);
            SendDlgItemMessage(wnd,1999,STM_SETIMAGE,IMAGE_BITMAP,(LPARAM)LoadBitmap(instance,MAKEINTRESOURCE(BITMAP_STARTUP)));
            //SetWindowPos(GetDlgItem(wnd,1999),0,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
            update_savepwd(wnd);
            update_sound(wnd);
            update_newlight(wnd);
            update_large(wnd);
            update_res(wnd);
            return TRUE;
        case WM_COMMAND:
            switch (wparam) {
                case IDCANCEL:
                    EndDialog(wnd,-1);
                    return TRUE;
                case IDOK:
                    GetDlgItemText(wnd,IDC_USERNAME,username,sizeof(username));
                    if (!check_username(username)) {
                        MessageBox(wnd,"The character name you entered does not seem to be correct. Note that this is not your account number but the name of the character you wish to play.","Error",MB_APPLMODAL|MB_OK|MB_ICONSTOP);
                        return TRUE;
                    }
                    GetDlgItemText(wnd,IDC_PASSWORD,password,sizeof(password));
                    save_options();
                    EndDialog(wnd,0);
                    return TRUE;

                case IDC_RES800:
                case IDC_RES1200:
                case IDC_RES1600:
                case IDC_RES2400:
                    opt_res=wparam;
                    update_res(wnd);
                    return 1;

                case IDC_SAVEPWD:
                    if (save_pwd) save_pwd=0;
                    else save_pwd=1;
                    update_savepwd(wnd);
                    return 1;

                case IDC_SOUND:
                    if (enable_sound) enable_sound=0;
                    else enable_sound=1;
                    update_sound(wnd);
                    return 1;

                case IDC_NEWLIGHT:
                    if (newlight) newlight=0;
                    else newlight=1;
                    update_newlight(wnd);
                    return 1;
                case IDC_LARGETEXT:
                    if (largetext) largetext=0;
                    else largetext=1;
                    update_large(wnd);
                    return 1;
                case IDC_CREATE:
                    ShellExecute(wnd,"open","https://www.intent-software.com/cgi-v3/conflict.cgi?step=1",NULL,NULL,SW_SHOWNORMAL);
                    return 1;
                case IDC_ACCOUNT:
                    ShellExecute(wnd,"open","https://www.intent-software.com/cgi-v3/conflict.cgi?step=0",NULL,NULL,SW_SHOWNORMAL);
                    return 1;
                case IDC_WEBSITE:
                    ShellExecute(wnd,"open","http://www.astonia.com/",NULL,NULL,SW_SHOWNORMAL);
                    return 1;
            }
            return FALSE;
    }
    return FALSE;
}

int win_start(void) {
    return DialogBox(instance,MAKEINTRESOURCE(IDD_STARTUP),NULL,start_dlg_proc);
}

// main

int PASCAL WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow) {
    int ret;
    int width,height;
    char buf[80];
    extern int x_offset,y_offset,x_max,y_max;
    struct hostent *he;

    errorfp=fopen("moac.log","a");
    if (!errorfp) errorfp=stderr;

    load_options();

    if ((ret=parse_cmd(lpCmdLine))!=0) return fail("ill command (%d)",ret);

    // set some stuff
    if (!*username || !*password) quickstart=0;

    // set instance
    instance=hInstance;

    // next init (only once)
    if (net_init()==-1) {
        MessageBox(NULL,"Can't Initialize Windows Networking Libraries.","Error",MB_APPLMODAL|MB_OK|MB_ICONSTOP);
        return -1;
    }

    if (!quickstart && win_start()) goto done;

    switch (opt_res) {
        case IDC_RES1200:	width=1200; height=900; x_offset=y_offset=0; break;
        case IDC_RES1600:	width=1600; height=1200; x_offset=y_offset=0; break;
        case IDC_RES2400:	width=2400; height=1800; x_offset=y_offset=0; break;
        case IDC_RES800:
        default:		width=800; height=600; x_offset=y_offset=0; break;
    }
    x_max=x_offset+800;
    y_max=y_offset+600;

    if (developer_server) {
        he=gethostbyname("astonia.dyndns.org");
        if (he) base_server=target_server=ntohl(*(unsigned long *)(*he->h_addr_list));
        else {
            base_server=target_server=(192<<24)|(168<<16)|(42<<8)|(2<<0);
            fail("gethostbyname4a failed, using default");
        }
    } else

#ifndef DEVELOPER
        if (backup_server==0) {
            he=gethostbyname("login.astonia.com");
            if (he) base_server=target_server=ntohl(*(unsigned long *)(*he->h_addr_list));
            else {
                base_server=target_server=(195<<24)|(90<<16)|(31<<8)|(34<<0);
                fail("gethostbyname4 failed, using default");
            }
        } else if (backup_server==1) {
            he=gethostbyname("login2.astonia.com");
            if (he) base_server=target_server=ntohl(*(unsigned long *)(*he->h_addr_list));
            else {
                base_server=target_server=(195<<24)|(50<<16)|(166<<8)|(1<<0);
                fail("gethostbyname5 failed, using default");
            }
        } else {
            he=gethostbyname("login3.astonia.com");
            if (he) base_server=target_server=ntohl(*(unsigned long *)(*he->h_addr_list));
            else {
                base_server=target_server=(195<<24)|(50<<16)|(166<<8)|(1<<0);
                fail("gethostbyname6 failed, using default");
            }
        }
#endif
    note("Using login server at %u.%u.%u.%u",(target_server>>24)&255,(target_server>>16)&255,(target_server>>8)&255,(target_server>>0)&255);

    // init random
    rrandomize();

    sprintf(buf,"Astonia 3 v%d.%d.%d",(VERSION>>16)&255,(VERSION>>8)&255,(VERSION)&255);
    if (dd_init(width,height)==-1) {
        char buf[512];
        dd_exit();

        sprintf(buf,"Can't Initialize SDL\nPlease make sure you have DirectX and the latest drivers\nfor your graphics card installed.");
        MessageBox(NULL,buf,"Error",MB_APPLMODAL|MB_OK|MB_ICONSTOP);

        net_exit();
        return -1;
    }

    sprintf(buf,"Astonia 3 v%d.%d.%d",(VERSION>>16)&255,(VERSION>>8)&255,(VERSION)&255);
    if (!sdl_init(800,600,buf)) {
        dd_exit();
        net_exit();
        return -1;
    }

#ifdef DOSOUND
    init_sound();
#endif

    if (largetext) {
        extern int namesize;

        namesize=0;
        dd_set_textfont(1);
    }

    if (main_init()==-1) {
        dd_exit();
        net_exit();
        MessageBox(NULL,"Can't Initialize Program","Error",MB_APPLMODAL|MB_OK|MB_ICONSTOP);
        return -1;
    }
    update_user_keys();

    main_loop();

    main_exit();
    dd_exit();

    list_mem();

    if (panic_reached) MessageBox(NULL,panic_reached_str,"recursion panic",MB_APPLMODAL|MB_OK|MB_ICONSTOP);
    if (xmemcheck_failed) MessageBox(NULL,memcheck_failed_str,"memory panic",MB_APPLMODAL|MB_OK|MB_ICONSTOP);
done:
    net_exit();
    return 0;
}

