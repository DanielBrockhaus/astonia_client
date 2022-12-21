/*
 * Part of Astonia Client (c) Daniel Brockhaus. Please read license.txt.
 */

#include <windows.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ddraw.h>
#include <math.h>
#include <stdio.h>
#pragma hdrstop
#include "main.h"
#include "dd.h"
#include "client.h"

DDFONT *fonta_shaded=NULL;
DDFONT *fonta_framed=NULL;

DDFONT *fontb_shaded=NULL;
DDFONT *fontb_framed=NULL;

DDFONT *fontc_shaded=NULL;
DDFONT *fontc_framed=NULL;

float fontdim_a[256];
float fontdim_b[256];
float fontdim_c[256];
float *fontdim=fontdim_a;

void dd_create_font(void);
void dd_init_text(void);
void dd_black(void);

// extern ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern HWND mainwnd;

// helpers

#ifndef irgb_blend
unsigned short int irgb_blend(unsigned short int a,unsigned short int b,int alpha) {
    return IRGB((IGET_R(a)*alpha+IGET_R(b)*(31-alpha))/31,(IGET_G(a)*alpha+IGET_G(b)*(31-alpha))/31,(IGET_B(a)*alpha+IGET_B(b)*(31-alpha))/31);
}
#endif

// direct x basics //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int dd_gamma=8;
int dd_lighteffect=16;
int newlight=0;

int XRES;               // set to indicate the maximal size of the offscreen surface - respective the display mode to set
int YRES;               // set to indicate the maximal size of the offscreen surface - respective the display mode to set

float mouse_scale=1.0f;      // mouse input needs to be scaled by this factor because the display window is stretched

char dderr[256]={""};
const char *DDERR=dderr;

unsigned int R_MASK;
unsigned int G_MASK;
unsigned int B_MASK;

#define RGBM_R5G6B5     0
#define RGBM_X1R5G5B5   1
#define RGBM_B5G6R5     2

const char *rgbmstr[3]={"RGBM_R5G6B5","RGBM_X1R5G5B5","RGBM_B5G6R5"};

int x_offset,y_offset;
int x_max,y_max;

int vc_cnt=0,sc_cnt=0,ap_cnt=0,np_cnt=0,tp_cnt=0,vm_cnt=0,sm_cnt=0;
int vc_time=0,ap_time=0,tp_time=0,sc_time=0,vm_time=0,bless_time=0,vi_time=0,im_time=0;

int rgbm=-1;
int xres;
int yres;

int dd_tick=0;

LPDIRECTDRAW dd=NULL;
LPDIRECTDRAWCLIPPER ddcl=NULL;
LPDIRECTDRAWSURFACE ddbs=NULL;

int clipsx,clipsy,clipex,clipey;
int clipstore[32][4],clippos=0;

static unsigned short *vidptr=NULL;     // for checking only - remove in final release

void dd_push_clip(void) {
    if (clippos>=32) return;

    clipstore[clippos][0]=clipsx;
    clipstore[clippos][1]=clipsy;
    clipstore[clippos][2]=clipex;
    clipstore[clippos][3]=clipey;
    clippos++;
}

void dd_pop_clip(void) {
    if (clippos==0) return;

    clippos--;
    clipsx=clipstore[clippos][0];
    clipsy=clipstore[clippos][1];
    clipex=clipstore[clippos][2];
    clipey=clipstore[clippos][3];
}

void dd_more_clip(int sx,int sy,int ex,int ey) {
    if (sx>clipsx) clipsx=sx;
    if (sy>clipsy) clipsy=sy;
    if (ex<clipex) clipex=ex;
    if (ey<clipey) clipey=ey;
}

void dd_set_clip(int sx,int sy,int ex,int ey) {
    clipsx=sx;
    clipsy=sy;
    clipex=ex;
    clipey=ey;
}

extern int gfx_load_image(IMAGE *image,int sprite);
extern int gfx_init(void);
extern int gfx_exit(void);

static const char* dd_errstr(int err) {
    static char buf[256];

    switch (err) {
        case -1:                                        return "DDERR INTERNAL";
        case DDERR_INVALIDOBJECT:                       return "DDERR_INVALIDOBJECT";
        case DDERR_INVALIDPARAMS:                       return "DDERR_INVALIDPARAMS";
        case DDERR_OUTOFMEMORY:                         return "DDERR_OUTOFMEMORY";
        case DDERR_SURFACEBUSY:                         return "DDERR_SURFACEBUSY";
        case DDERR_SURFACELOST:                         return "DDERR_SURFACELOST";
        case DDERR_WASSTILLDRAWING:                 	return "DDERR_WASSTILLDRAWING";
        case DDERR_INCOMPATIBLEPRIMARY:             	return "DDERR_INCOMPATIBLEPRIMARY";
        case DDERR_INVALIDCAPS:                   	return "DDERR_INVALIDCAPS";
        case DDERR_INVALIDPIXELFORMAT:            	return "DDERR_INVALIDPIXELFORMAT";
        case DDERR_NOALPHAHW:                     	return "DDERR_NOALPHAHW";
        case DDERR_NOCOOPERATIVELEVELSET:         	return "DDERR_NOCOOPERATIVELEVELSET";
        case DDERR_NODIRECTDRAWHW:                	return "DDERR_NODIRECTDRAWHW";
        case DDERR_NOEMULATION:                   	return "DDERR_NOEMULATION";
        case DDERR_NOEXCLUSIVEMODE:               	return "DDERR_NOEXCLUSIVEMODE";
        case DDERR_NOFLIPHW:                      	return "DDERR_NOFLIPHW";
        case DDERR_NOMIPMAPHW:                    	return "DDERR_NOMIPMAPHW";
        case DDERR_NOOVERLAYHW:                   	return "DDERR_NOOVERLAYHW";
        case DDERR_NOZBUFFERHW:                   	return "DDERR_NOZBUFFERHW";
        case DDERR_OUTOFVIDEOMEMORY:              	return "DDERR_OUTOFVIDEOMEMORY";
        case DDERR_PRIMARYSURFACEALREADYEXISTS:   	return "DDERR_PRIMARYSURFACEALREADYEXISTS";
        case DDERR_UNSUPPORTEDMODE:               	return "DDERR_UNSUPPORTEDMODE";
        case DDERR_EXCEPTION:                           return "DDERR_EXCEPTION";
        case DDERR_GENERIC:                             return "DDERR_GENERIC";
        case DDERR_INVALIDRECT:                         return "DDERR_INVALIDRECT";
        case DDERR_NOTFLIPPABLE:                        return "DDERR_NOTFLIPPABLE";
        case DDERR_UNSUPPORTED:                         return "DDERR_UNSUPPORTED";
    }

    sprintf(buf,"DDERR_UNKNOWN(%d,%X,%X)",err,err,DDERR_EXCEPTION);
    return buf;
}

static int dd_error(const char *txt,int err) {
    sprintf(dderr,"%s : %s",txt,dd_errstr(err));
    if (err==DDERR_SURFACELOST) note("%s",dderr);
    else fail("%s",dderr);
    return -1;
}

static char* binstr(char *buf,unsigned int val,int num) {
    int i;
    char *run=buf;

    for (i=num-1; i>=0; i--) if ((1<<i)&val) *run++='1';
        else *run++='0';
    *run=0;

    return buf;
}

static char* ddsdstr(char *buf,DDSURFACEDESC *ddsd) {
    int bpp,pitch;
    char *memstr;

    if (ddsd->ddsCaps.dwCaps&DDSCAPS_LOCALVIDMEM) memstr="localvidmem";
    else if (ddsd->ddsCaps.dwCaps&DDSCAPS_VIDEOMEMORY) memstr="videomemory";
    else if (ddsd->ddsCaps.dwCaps&DDSCAPS_SYSTEMMEMORY) memstr="systemmemory";
    else memstr="funnymemory";

    bpp=ddsd->ddpfPixelFormat.u1.dwRGBBitCount;
    if (bpp) pitch=ddsd->u1.lPitch/(bpp/8);
    else pitch=-1;

    sprintf(buf,"%dx%dx%d %s (pitch=%d) (%08X,%08X,%08X)",
            ddsd->dwWidth,ddsd->dwHeight,bpp,
            memstr,
            pitch,
            ddsd->ddpfPixelFormat.u2.dwRBitMask, // binstr(rbuf,ddsd->ddpfPixelFormat.u2.dwRBitMask,bpp),
            ddsd->ddpfPixelFormat.u3.dwGBitMask, // binstr(gbuf,ddsd->ddpfPixelFormat.u3.dwGBitMask,bpp),
            ddsd->ddpfPixelFormat.u4.dwBBitMask  // binstr(bbuf,ddsd->ddpfPixelFormat.u4.dwBBitMask,bpp)
           );

    return buf;
}

static int dd_vidmembytes(LPDIRECTDRAWSURFACE sur) {
    DDSURFACEDESC ddsd;
    int err;

    bzero(&ddsd,sizeof(ddsd));
    ddsd.dwSize=sizeof(ddsd);
    ddsd.dwFlags=DDSD_ALL;
    if ((err=sur->lpVtbl->GetSurfaceDesc(sur,&ddsd))!=DD_OK) return dd_error("GetSurfaceDesc(ddcs[s])",err);

    if (ddsd.ddsCaps.dwCaps&DDSCAPS_SYSTEMMEMORY) return 0; // if it's not there, it has to be in video memory - works for funnymemory as well

    return ddsd.dwHeight*ddsd.u1.lPitch;
}

void dd_get_client_info(struct client_info *ci) {
    int n;
    DDCAPS caps;
    static MEMORYSTATUS memstat;

    bzero(&caps,sizeof(caps));
    caps.dwSize=sizeof(caps);
    dd->lpVtbl->GetCaps(dd,&caps,NULL);

    bzero(&memstat,sizeof(memstat));
    memstat.dwLength=sizeof(memstat);
    GlobalMemoryStatus(&memstat);

    ci->vidmemtotal=caps.dwVidMemTotal;
    ci->vidmemfree=caps.dwVidMemFree;

    ci->systemtotal=memstat.dwTotalPhys;
    ci->systemfree=memstat.dwAvailPhys;
}

int dd_set_color_key(void) {
    DDCOLORKEY key;
    int err;
    int s;

    note("colorkey=0x%X/0x%X - 0x%X - 0x%X",rgbcolorkey,scrcolorkey,scr2rgb[scrcolorkey],R_MASK|B_MASK);

    key.dwColorSpaceLowValue=scrcolorkey;
    key.dwColorSpaceHighValue=scrcolorkey;

    return 0;
}

int dd_init(int width,int height) {
    DDSURFACEDESC ddsd;
    int err,s,r,flags,freevidmem;
    char buf[1024];

    // create dd
    if ((err=DirectDrawCreate(NULL,&dd,NULL))!=DD_OK) return dd_error("DirectDrawCreate()",err);

    // you can force any screen (and offscreen) size
    if (editor) {
        XRES=width;
        YRES=height;
    } else {
        XRES=800;
        YRES=600;
    }

    // set cooperative level
    if ((err=dd->lpVtbl->SetCooperativeLevel(dd,mainwnd,DDSCL_NORMAL))!=DD_OK) return dd_error("SetCooperativeLevel()",err);

    // create a back surface (offscreen, always using a 16 bit mode)
    note("back surface: %dx%d",XRES,YRES);
    bzero(&ddsd,sizeof(ddsd));
    ddsd.dwSize=sizeof(ddsd);
    ddsd.dwFlags=DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT|DDSD_PIXELFORMAT;
    ddsd.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN|DDSCAPS_SYSTEMMEMORY;
    ddsd.dwWidth=XRES;
    ddsd.dwHeight=YRES;
    ddsd.ddpfPixelFormat.dwSize=sizeof(ddsd.ddpfPixelFormat);
    ddsd.ddpfPixelFormat.dwFlags=DDPF_RGB;
    ddsd.ddpfPixelFormat.u1.dwRGBBitCount=16;

    // RGBM_R5G6B5
    ddsd.ddpfPixelFormat.u2.dwRBitMask=0xF800;
    ddsd.ddpfPixelFormat.u3.dwGBitMask=0x07E0;
    ddsd.ddpfPixelFormat.u4.dwBBitMask=0x001F;
    ddsd.ddpfPixelFormat.u5.dwRGBAlphaBitMask=0;

    if ((err=dd->lpVtbl->CreateSurface(dd,&ddsd,&ddbs,NULL))!=DD_OK) return dd_error("CreateSurface(ddbs)",err);    // create Backsurface

    // do some neccassary clipper stuff
    if ((err=dd->lpVtbl->CreateClipper(dd,0,&ddcl,NULL))!=DD_OK) return dd_error("CreateClipper(ddbs)",err);        // CreateClipper
    if ((err=ddcl->lpVtbl->SetHWnd(ddcl,0,mainwnd))!=DD_OK) return dd_error("SetHWnd(ddcl)",err);                   // Attach Clipper to Window

    // get informations about the back surface
    bzero(&ddsd,sizeof(ddsd));
    ddsd.dwSize=sizeof(ddsd);
    ddsd.dwFlags=DDSD_ALL;
    if ((err=ddbs->lpVtbl->GetSurfaceDesc(ddbs,&ddsd))!=DD_OK) return dd_error("GetSurfaceDesc(ddbs)",err);
    note("ddbs is %s",ddsdstr(buf,&ddsd));
    xres=ddsd.u1.lPitch/2;
    yres=ddsd.dwHeight;

    if (!(ddsd.ddpfPixelFormat.dwFlags&DDPF_RGB)) return dd_error("CANNOT HANDLE PIXEL FORMAT",-1);
    R_MASK=ddsd.ddpfPixelFormat.u2.dwRBitMask;
    G_MASK=ddsd.ddpfPixelFormat.u3.dwGBitMask;
    B_MASK=ddsd.ddpfPixelFormat.u4.dwBBitMask;

    if (R_MASK==0xF800 && G_MASK==0x07E0 && B_MASK==0x001F) { rgbm=RGBM_R5G6B5; }
    else if (R_MASK==0x7C00 && G_MASK==0x03E0 && B_MASK==0x001F) { rgbm=RGBM_X1R5G5B5; }
    else if (R_MASK==0x001F && G_MASK==0x07E0 && B_MASK==0xF800) { rgbm=RGBM_B5G6R5; }
    else return dd_error("CANNOT HANDLE RGB MASK",-1);

    // initialize cache (will initialize color tables, too)
    if (dd_init_cache()==-1) return -1;

    // set the color key of all cache surfaces
    dd_set_color_key();

    // set the clipping to the maximum possible
    clippos=0;
    clipsx=0;
    clipsy=0;
    clipex=XRES;
    clipey=YRES;

    // initialize the gfx loading stuff - TODO: call this in dd_init_cache();
    gfx_init();

    dd_create_font();
    dd_init_text();

    dd_black();

    return 0;
}

int dd_exit(void) {
    int s,left;

    // removed - slow!
    //gfx_exit();
    //dd_exit_cache();

    if (ddbs) {
        left=ddbs->lpVtbl->Release(ddbs);
        ddbs=NULL;
        // note("released ddbs. %d references left",left);
    }

    if (ddcl) {
        left=ddcl->lpVtbl->Release(ddcl);
        ddcl=NULL;
        // note("released ddcl. %d references left",left);
    }

    if (dd) {
        dd->lpVtbl->RestoreDisplayMode(dd);
        left=dd->lpVtbl->Release(dd);
        dd=NULL;
        // note("released dd. %d references left",left);
    }

    if (left==12345678) return left; // grrr

    return 0;
}

void* dd_lock_surface(LPDIRECTDRAWSURFACE surface) {
    DDSURFACEDESC ddsd;
    int err;

    bzero(&ddsd,sizeof(ddsd));
    ddsd.dwSize=sizeof(ddsd);
    if ((err=surface->lpVtbl->Lock(surface,NULL,&ddsd,DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT,NULL))!=DD_OK) { dd_error("Lock()",err); return NULL; }

    return ddsd.lpSurface;
}

int dd_unlock_surface(LPDIRECTDRAWSURFACE surface) {
    int err;

    if ((err=surface->lpVtbl->Unlock(surface,NULL))!=DD_OK) return dd_error("Unlock()",err);

    return 0;
}

HDC dd_get_dc(LPDIRECTDRAWSURFACE surface) {
    int err;
    HDC dc;

    if ((err=surface->lpVtbl->GetDC(surface,&dc))!=DD_OK) { dd_error("dd_get_dc()",err); return NULL; }

    return dc;
}

int dd_release_dc(LPDIRECTDRAWSURFACE surface,HDC dc) {
    int err;

    if ((err=surface->lpVtbl->ReleaseDC(surface,dc))!=DD_OK) return dd_error("dd_release_dc()",err);

    return 0;
}

void* dd_lock_ptr(void) {
    return dd_lock_surface(ddbs);
}

int dd_unlock_ptr(void) {
    return dd_unlock_surface(ddbs);
}


#define MAXTEXTER   1024
struct texter {
    int x,y;
    char *text;
    int color;
    int flags;
};

int textcnt=0;
struct texter texter[MAXTEXTER];
int newtext=0;

void texter_add(int x, int y,int color,int flags,const char *text) {
    int r,g,b;

    if (textcnt>=MAXTEXTER) return;

    r=(int)((((color>>11)&31)/31.0f)*255.0f);
    g=(int)((((color>>5) &63)/63.0f)*255.0f);
    b=(int)((((color)    &31)/31.0f)*255.0f);


    texter[textcnt].x=x;
    texter[textcnt].y=y;
    texter[textcnt].color=(r)|(g<<8)|(b<<16);
    texter[textcnt].flags=flags;
    texter[textcnt].text=strdup(text);
    textcnt++;
}

void dd_flip(void) {
    HDC srcdc;
    HDC tgtdc;
    HDC tmpdc;
    HBITMAP bm;
    RECT r;
    int xs,ys;

    srcdc=dd_get_dc(ddbs);
    tgtdc=GetDC(mainwnd);

    GetClientRect(mainwnd,&r);
    xs=r.right-r.left;
    ys=r.bottom-r.top;

    if (newtext) {
        tmpdc=CreateCompatibleDC(tgtdc);
        bm=CreateCompatibleBitmap(tgtdc,xs,ys);
        SelectObject(tmpdc,bm);
    } else tmpdc=tgtdc;

    if (xs!=XRES && ys!=YRES) {

        mouse_scale=1.0*xs/XRES;

        if (mouse_scale!=2.0f && mouse_scale!=3.0f) {
            if (srcdc) SetStretchBltMode(srcdc,HALFTONE);
            if (tmpdc) SetStretchBltMode(tmpdc,HALFTONE);
        } else {
            if (srcdc) SetStretchBltMode(srcdc,COLORONCOLOR);
            if (tmpdc) SetStretchBltMode(tmpdc,COLORONCOLOR);
        }
        if (srcdc && tmpdc) StretchBlt(tmpdc,0,0,xs,ys,srcdc,0,0,XRES,YRES,SRCCOPY);
    } else {
        if (srcdc && tmpdc) BitBlt(tmpdc,0,0,XRES,YRES,srcdc,0,0,SRCCOPY);
        mouse_scale=1.0;
    }

    dd_release_dc(ddbs,srcdc);

    if (newtext) {
        static HFONT wfonts=NULL,wfontm=NULL,wfontb=NULL;
        int n,x,y,flags;
        char *text;

        if (!wfonts) {
            ABC abc;
            int n;

            #define fontname "Small"
            wfonts=CreateFont(10*mouse_scale,0,0,0,FW_DONTCARE,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_OUTLINE_PRECIS,CLIP_DEFAULT_PRECIS,ANTIALIASED_QUALITY,VARIABLE_PITCH,TEXT(fontname));
            wfontm=CreateFont(11*mouse_scale,0,0,0,FW_DONTCARE,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_OUTLINE_PRECIS,CLIP_DEFAULT_PRECIS,ANTIALIASED_QUALITY,VARIABLE_PITCH,TEXT(fontname));
            wfontb=CreateFont(15*mouse_scale,0,0,0,FW_DONTCARE,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_OUTLINE_PRECIS,CLIP_DEFAULT_PRECIS,ANTIALIASED_QUALITY,VARIABLE_PITCH,TEXT(fontname));

            SelectObject(tmpdc,wfontm);
            for (n=32; n<128; n++) {
                GetCharABCWidths(tmpdc,n,n,&abc);
                fontdim_a[n]=(abc.abcA+abc.abcB+abc.abcC)/mouse_scale;
            }
            SelectObject(tmpdc,wfonts);
            for (n=32; n<128; n++) {
                GetCharABCWidths(tmpdc,n,n,&abc);
                fontdim_b[n]=(abc.abcA+abc.abcB+abc.abcC)/mouse_scale;
            }
            SelectObject(tmpdc,wfontb);
            for (n=32; n<128; n++) {
                GetCharABCWidths(tmpdc,n,n,&abc);
                fontdim_c[n]=(abc.abcA+abc.abcB+abc.abcC)/mouse_scale;
            }
        }
        SetBkMode(tmpdc,TRANSPARENT);

        for (n=0; n<textcnt; n++) {
            x=(texter[n].x)*mouse_scale;
            y=(texter[n].y-2)*mouse_scale;
            text=texter[n].text;
            flags=texter[n].flags;

            if (flags&DD_SMALL) SelectObject(tmpdc,wfonts);
            else if (flags&DD_LARGE) SelectObject(tmpdc,wfontb);
            else SelectObject(tmpdc,wfontm);


            if (flags&DD_CENTER) SetTextAlign(tmpdc,TA_CENTER|TA_TOP);
            else if (flags&DD_RIGHT) SetTextAlign(tmpdc,TA_RIGHT|TA_TOP);
            else SetTextAlign(tmpdc,TA_LEFT|TA_TOP);

            if (flags&(DD_SHADE|DD_FRAME)) {
                SetTextColor(tmpdc,0);
                TextOut(tmpdc,x-1,y,text,strlen(text));
                TextOut(tmpdc,x,y-1,text,strlen(text));
                TextOut(tmpdc,x+1,y,text,strlen(text));
                TextOut(tmpdc,x,y+1,text,strlen(text));
            }

            SetTextColor(tmpdc,texter[n].color);
            TextOut(tmpdc,x,y,text,strlen(text));
            free(text);
        }
        textcnt=0;

        if (tgtdc && tmpdc) BitBlt(tgtdc,0,0,xs,ys,tmpdc,0,0,SRCCOPY);

        SelectObject(tmpdc,NULL);
        DeleteObject(bm);
        DeleteDC(tmpdc);
        ReleaseDC(mainwnd,tgtdc);
    }
}

int dd_islost(void) {
    int s;

    if (ddbs->lpVtbl->IsLost(ddbs)!=DD_OK) return 1; // ddbs->lpVtbl->Restore(ddbs); else ok++;

    return 0;
}

int dd_restore(void) {
    int s;
    static int cacheflag=0;

    if (ddbs->lpVtbl->IsLost(ddbs)!=DD_OK) if (ddbs->lpVtbl->Restore(ddbs)!=DD_OK) return -1;

    if (cacheflag) { dd_reset_cache(0,0,1); cacheflag=0; }

    dd_black();

    return 0;
}

// cache ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int vget(int tx,int ty,int tdx,int tdy) {
    int val;

    val=tx+ty*tdx;
    PARANOIA(if (val<0 || val>tdx*tdy) paranoia("vget: val(%d) to large (tx=%d ty=%d tdx=%d tdy=%d",tx,ty,tdx,tdy); )

    return val;
}


#define SIDX_NONE       -1              // still not set (invalid position) set in videocache[v].sidx
#define SPRITE_NONE     -1
#define IIDX_NONE       -1

struct imagecache {
    int prev;                       // last used list
    int next;


    int hnext,hprev;        // hash link

    int sprite;                     // if sprite==SPRITE_NONE this entry is free

    IMAGE image;
};

typedef struct imagecache IMAGECACHE;

struct apix {
    unsigned char x;
    unsigned char y;
    unsigned char a;
    unsigned short int rgb;
};

typedef struct apix APIX;

struct systemcache {
    int prev;                       // last used list
    int next;

    // fx
    int sprite;                     // if sprite==SPRITE_NONE this entry is free

    signed char sink;
    unsigned char scale;
    char cr,cg,cb,light,sat;
    unsigned short c1,c2,c3,shine;

    unsigned char freeze;
    unsigned char grid;

    // light
    char ml,ll,rl,ul,dl;

    // hash links
    int hnext,hprev;

    // primary
    unsigned short xres;            // x resolution in pixels
    unsigned short yres;            // y resolution in pixels
    unsigned char tdx;              // x-resolution in tiles
    unsigned char tdy;              // y-resolution in tiles
    short xoff;                     // offset to blit position
    short yoff;                     // offset to blit position
    short int size;

    unsigned short *rgb;            // colors in internal screen format
    short acnt;                     // number of alpha pixel
    APIX *apix;                     // alpha pixel

    // vc access
    unsigned int used;
    unsigned int lost;
    unsigned int tick;
};

typedef struct systemcache SYSTEMCACHE;

int max_imagecache=0;
int max_systemcache=0;

IMAGECACHE *imagecache;
SYSTEMCACHE *systemcache;

int sidx_best,sidx_last;
int iidx_best,iidx_last;

unsigned short rgbcolorkey=IRGB(31,0,31);
unsigned short rgbcolorkey2=IRGB(16,0,16);
unsigned short scrcolorkey;
unsigned short *rgb2scr=NULL;
unsigned short *scr2rgb=NULL;
#define MAX_RGBFX_LIGHT                 16
unsigned short **rgbfx_light;           // 16 light values (0-14=light 15=bright)
unsigned short **rgbfx_freeze;          // DDFX_MAX_FREEZE light values (0 is empty, no freeze - take care empty=NULL)

#define IMA_HASH_SIZE	1024
#define SYS_HASH_SIZE	1024
static int *ima_hash;   // [IMA_HASH_SIZE];
static int *sys_hash;   // [SYS_HASH_SIZE],sys_hash_init=0;


int sc_miss=0,sc_hit=0,sc_maxstep=0;
int vc_miss=0,vc_hit=0,vc_unique=0,vc_unique24=0,sc_blits=0;

int dd_init_cache(void) {
    int scr,rgb,r,g,b,s,i,v,x,y;
    int tr,tg,tb;

    // imagecache
    max_imagecache=1024;
    imagecache=xcalloc(max_imagecache*sizeof(IMAGECACHE),MEM_GLOB);
    for (i=0; i<max_imagecache; i++) {
        imagecache[i].sprite=SPRITE_NONE;
        imagecache[i].prev=i-1;
        imagecache[i].next=i+1;
        imagecache[i].hnext=-1;
        imagecache[i].hprev=-1;
    }
    imagecache[0].prev=IIDX_NONE;
    imagecache[max_imagecache-1].next=IIDX_NONE;
    iidx_best=0;
    iidx_last=max_imagecache-1;

    ima_hash=xmalloc(IMA_HASH_SIZE*sizeof(int),MEM_GLOB);
    for (i=0; i<IMA_HASH_SIZE; i++) ima_hash[i]=-1;

    // systemcache
    max_systemcache=1024*10;
    systemcache=xcalloc(max_systemcache*sizeof(SYSTEMCACHE),MEM_GLOB);
    for (i=0; i<max_systemcache; i++) {
        systemcache[i].sprite=SPRITE_NONE;
        systemcache[i].prev=i-1;
        systemcache[i].next=i+1;
        systemcache[i].hnext=-1;
        systemcache[i].hprev=-1;
    }
    systemcache[0].prev=SIDX_NONE;
    systemcache[max_systemcache-1].next=SIDX_NONE;
    sidx_best=0;
    sidx_last=max_systemcache-1;

    sys_hash=xmalloc(SYS_HASH_SIZE*sizeof(int),MEM_GLOB);
    for (i=0; i<SYS_HASH_SIZE; i++) sys_hash[i]=-1;


    // tables
    rgb2scr=xmalloc(65536*sizeof(unsigned short),MEM_GLOB);
    scr2rgb=xmalloc(65536*sizeof(unsigned short),MEM_GLOB);

    for (scr=0; scr<65536; scr++) {

        if (rgbm==RGBM_R5G6B5) {
            // rrrrrggggggbbbbb
            r=(scr>>11)&0x1F;
            g=(scr>>6)&0x1F;
            b=(scr>>0)&0x1F;
        } else if (rgbm==RGBM_X1R5G5B5) {
            // xrrrrrgggggbbbbb
            r=(scr>>10)&0x1F;
            g=(scr>>5)&0x1F;
            b=(scr>>0)&0x1F;
        } else if (rgbm==RGBM_B5G6R5) {
            // bbbbbggggggrrrrr
            r=(scr>>0)&0x1F;
            g=(scr>>6)&0x1F;
            b=(scr>>11)&0x1F;
        } else return -1;

        scr2rgb[scr]=IRGB(r,g,b);
        rgb2scr[IRGB(r,g,b)]=scr;
    }

    scrcolorkey=R_MASK|B_MASK;
    scr2rgb[scrcolorkey]=IRGB(31,31,0);
    rgb2scr[rgbcolorkey]=scrcolorkey;

    // light
    rgbfx_light=xcalloc(MAX_RGBFX_LIGHT*sizeof(unsigned short int *),MEM_GLOB);
    for (i=0; i<16; i++) rgbfx_light[i]=xcalloc(65536*sizeof(unsigned short),MEM_GLOB);
    for (rgb=0; rgb<32768; rgb++) rgbfx_light[0][rgb]=IRGB(min(31,2*IGET_R(rgb)+4),min(31,2*IGET_G(rgb)+4),min(31,2*IGET_B(rgb)+4));
    for (i=1; i<=15; i++) {
        for (rgb=0; rgb<32768; rgb++) {

            r=IGET_R(rgb);
            g=IGET_G(rgb);
            b=IGET_B(rgb);


            {
                static int lightmulti[16]={0,2,4,8,16,18,20,22,24,26,27,28,29,30,31,32};
                int lm=lightmulti[i],le=dd_lighteffect;

                if (newlight) le=i;

                r=min(31,(lm+le)*(r*dd_gamma/8)/(32+le));
                g=min(31,(lm+le)*(g*dd_gamma/8)/(32+le));
                b=min(31,(lm+le)*(b*dd_gamma/8)/(32+le));
            }

            if (r<0 || r>31 || g<0 || g>31 || b<0 || b>31) paranoia("some ill rgbfx_light here r=%d g=%d b=%d i=%d",r,g,b,i);

            rgbfx_light[i][rgb]=IRGB(r,g,b);
        }
    }

    // freeze
    rgbfx_freeze=xcalloc(DDFX_MAX_FREEZE*sizeof(unsigned short int *),MEM_GLOB);
    for (i=1; i<DDFX_MAX_FREEZE; i++) rgbfx_freeze[i]=xcalloc(65536*sizeof(unsigned short),MEM_GLOB);
    for (i=1; i<DDFX_MAX_FREEZE; i++) {
        for (rgb=0; rgb<32768; rgb++) {

            r=IGET_R(rgb);
            g=IGET_G(rgb);
            b=IGET_B(rgb);

            r=min(31,r+31*i/(3*DDFX_MAX_FREEZE-1));
            g=min(31,g+31*i/(3*DDFX_MAX_FREEZE-1));
            b=min(31,b+31*3*i/(3*DDFX_MAX_FREEZE-1));

            if (r<0 || r>31 || g<0 || g>31 || b<0 || b>31) paranoia("some ill rgbfx_light here r=%d g=%d b=%d i=%d",r,g,b,i);

            rgbfx_freeze[i][rgb]=IRGB(r,g,b);
        }
    }


    return 0;
}

void dd_exit_cache(void) {
    int i,s;

    // tables
    xfree(rgb2scr);
    rgb2scr=NULL;
    xfree(scr2rgb);
    scr2rgb=NULL;

    // freeze
    if (rgbfx_freeze) {
        for (i=1; i<DDFX_MAX_FREEZE; i++) xfree(rgbfx_freeze[i]);
        xfree(rgbfx_freeze);
        rgbfx_freeze=NULL;
    }

    // light
    if (rgbfx_light) {
        for (i=0; i<16; i++) xfree(rgbfx_light[i]);
        xfree(rgbfx_light);
        rgbfx_light=NULL;
    }

    // systemcache
    for (i=0; i<max_systemcache; i++) {
        xfree(systemcache[i].rgb);
        xfree(systemcache[i].apix);
    }
    xfree(systemcache);
    systemcache=NULL;
    max_systemcache=0;

    xfree(sys_hash);
    sys_hash=NULL;

    // imagecache
    for (i=0; i<max_imagecache; i++) {
        xfree(imagecache[i].image.rgb);
        xfree(imagecache[i].image.a);
    }
    xfree(imagecache);
    imagecache=NULL;
    max_imagecache=0;

    xfree(ima_hash);
    ima_hash=NULL;

    return;
}

int dd_reset_cache(int reset_image,int reset_system,int reset_video) {
    int i,iidx,sidx,vidx;
    int vmax,v;

    note("reset%s%s%s",reset_image?" image_cache":"",reset_system?" system_cache":"",reset_video?" video_cache":"");

    // imagecache
    if (reset_image) {
        for (iidx=0; iidx<max_imagecache; iidx++) {
            if (imagecache[iidx].sprite==SPRITE_NONE) continue;
            imagecache[iidx].sprite=SPRITE_NONE;
            imagecache[iidx].hnext=-1;
            imagecache[iidx].hprev=-1;
        }

        for (i=0; i<IMA_HASH_SIZE; i++) ima_hash[i]=-1;
    }

    // systemcache
    if (reset_system) {
        for (sidx=0; sidx<max_systemcache; sidx++) {

            if (systemcache[sidx].sprite==SPRITE_NONE) continue;

            systemcache[sidx].sprite=SPRITE_NONE;
        }

        for (i=0; i<SYS_HASH_SIZE; i++) sys_hash[i]=-1;
    }

    return 0;
}

static void ic_best(int iidx) {
    PARANOIA(if (iidx==IIDX_NONE) paranoia("ic_best(): iidx=IIDX_NONE"); )
    PARANOIA(if (iidx>=max_imagecache) paranoia("ic_best(): iidx>max_imagecache (%d>=%d)",iidx,max_imagecache); )

    if (imagecache[iidx].prev==IIDX_NONE) {

        PARANOIA(if (iidx!=iidx_best) paranoia("ic_best(): iidx should be best"); )

        return;
    } else if (imagecache[iidx].next==IIDX_NONE) {

        PARANOIA(if (iidx!=iidx_last) paranoia("ic_best(): iidx should be last"); )

        iidx_last=imagecache[iidx].prev;
        imagecache[iidx_last].next=IIDX_NONE;
        imagecache[iidx_best].prev=iidx;
        imagecache[iidx].prev=IIDX_NONE;
        imagecache[iidx].next=iidx_best;
        iidx_best=iidx;

        return;
    } else {
        imagecache[imagecache[iidx].prev].next=imagecache[iidx].next;
        imagecache[imagecache[iidx].next].prev=imagecache[iidx].prev;
        imagecache[iidx].prev=IIDX_NONE;
        imagecache[iidx].next=iidx_best;
        imagecache[iidx_best].prev=iidx;
        iidx_best=iidx;
        return;
    }
}

static void ic_last(int iidx) {
    PARANOIA(if (iidx==IIDX_NONE) paranoia("ic_last(): iidx=IIDX_NONE"); )
    PARANOIA(if (iidx>=max_imagecache) paranoia("ic_last(): iidx>max_imagecache (%d>=%d)",iidx,max_imagecache); )

    if (imagecache[iidx].next==IIDX_NONE) {

        PARANOIA(if (iidx!=iidx_last) paranoia("ic_last(): iidx should be last"); )

        return;
    } else if (imagecache[iidx].prev==IIDX_NONE) {

        PARANOIA(if (iidx!=iidx_best) paranoia("ic_last(): iidx should be best"); )

        iidx_best=imagecache[iidx].next;
        imagecache[iidx_best].prev=IIDX_NONE;
        imagecache[iidx_last].next=iidx;
        imagecache[iidx].prev=iidx_last;
        imagecache[iidx].next=IIDX_NONE;
        iidx_last=iidx;
    } else {
        imagecache[imagecache[iidx].prev].next=imagecache[iidx].next;
        imagecache[imagecache[iidx].next].prev=imagecache[iidx].prev;
        imagecache[iidx].prev=iidx_last;
        imagecache[iidx].next=IIDX_NONE;
        imagecache[iidx_last].next=iidx;
        iidx_last=iidx;
    }
}

static void sc_best(int sidx) {
    PARANOIA(if (sidx==SIDX_NONE) paranoia("sc_best(): sidx=SIDX_NONE"); )
    PARANOIA(if (sidx>=max_systemcache) paranoia("sc_best(): sidx>max_systemcache (%d>=%d)",sidx,max_systemcache); )

    if (systemcache[sidx].prev==SIDX_NONE) {

        PARANOIA(if (sidx!=sidx_best) paranoia("sc_best(): sidx should be best"); )

        return;
    } else if (systemcache[sidx].next==SIDX_NONE) {

        PARANOIA(if (sidx!=sidx_last) paranoia("sc_best(): sidx should be last"); )

        sidx_last=systemcache[sidx].prev;
        systemcache[sidx_last].next=SIDX_NONE;
        systemcache[sidx_best].prev=sidx;
        systemcache[sidx].prev=SIDX_NONE;
        systemcache[sidx].next=sidx_best;
        sidx_best=sidx;

        return;
    } else {
        systemcache[systemcache[sidx].prev].next=systemcache[sidx].next;
        systemcache[systemcache[sidx].next].prev=systemcache[sidx].prev;
        systemcache[sidx].prev=SIDX_NONE;
        systemcache[sidx].next=sidx_best;
        systemcache[sidx_best].prev=sidx;
        sidx_best=sidx;
        return;
    }
}

static void sc_last(int sidx) {
    PARANOIA(if (sidx==SIDX_NONE) paranoia("sc_last(): sidx=SIDX_NONE"); )
    PARANOIA(if (sidx>=max_systemcache) paranoia("sc_last(): sidx>max_systemcache (%d>=%d)",sidx,max_systemcache); )

    if (systemcache[sidx].next==SIDX_NONE) {

        PARANOIA(if (sidx!=sidx_last) paranoia("sc_last(): sidx should be last"); )

        return;
    } else if (systemcache[sidx].prev==SIDX_NONE) {

        PARANOIA(if (sidx!=sidx_best) paranoia("sc_last(): sidx should be best"); )

        sidx_best=systemcache[sidx].next;
        systemcache[sidx_best].prev=SIDX_NONE;
        systemcache[sidx_last].next=sidx;
        systemcache[sidx].prev=sidx_last;
        systemcache[sidx].next=SIDX_NONE;
        sidx_last=sidx;
    } else {
        systemcache[systemcache[sidx].prev].next=systemcache[sidx].next;
        systemcache[systemcache[sidx].next].prev=systemcache[sidx].prev;
        systemcache[sidx].prev=sidx_last;
        systemcache[sidx].next=SIDX_NONE;
        systemcache[sidx_last].next=sidx;
        sidx_last=sidx;
    }
}

static void sc_blit_apix(int sidx,int scrx,int scry,int grid,int freeze) {
    int i,start;
    unsigned short int src,dst,r,g,b,a;
    unsigned short int *ptr;
    APIX *apix;
    int acnt,xoff,yoff,x,y,m;

    apix=systemcache[sidx].apix;
    acnt=systemcache[sidx].acnt;
    xoff=systemcache[sidx].xoff;
    yoff=systemcache[sidx].yoff;

    start=GetTickCount();

    if ((vidptr=ptr=dd_lock_surface(ddbs))==NULL) return;

    for (i=0; i<acnt; i++,apix++) {
        if (grid==DDFX_LEFTGRID) { if ((xoff+apix->x+yoff+apix->y)&1) continue; } else if (grid==DDFX_RIGHTGRID) {  if ((xoff+apix->x+yoff+apix->y+1)&1) continue; }

        x=scrx+apix->x;
        y=scry+apix->y;
        if (x<clipsx || y<clipsy || x>=clipex || y>=clipey) continue;

        a=apix->a;
        src=apix->rgb;
        if (freeze) src=rgbfx_freeze[freeze][src];

        m=(x+x_offset)+(y+y_offset)*xres;

        dst=scr2rgb[ptr[m]];

        r=(a*IGET_R(src)+(31-a)*IGET_R(dst))/31;
        g=(a*IGET_G(src)+(31-a)*IGET_G(dst))/31;
        b=(a*IGET_B(src)+(31-a)*IGET_B(dst))/31;

        if (x+y*xres>=xres*yres || x+y*xres<0) {
            note("PANIC #2");
            continue;
        }

        ptr[m]=rgb2scr[IRGB(r,g,b)];
    }

    dd_unlock_surface(ddbs);

    ap_time+=GetTickCount()-start;
    ap_cnt+=acnt;
}

static int ic_load(int sprite) {
    int iidx,start;
    int amax,nidx,pidx,v;
    IMAGE *image;

    start=GetTickCount();

    if (sprite>MAXSPRITE || sprite<0) {
        note("illegal sprite %d wanted in ic_load",sprite);
        return IIDX_NONE;
    }

    for (iidx=ima_hash[sprite%IMA_HASH_SIZE]; iidx!=-1; iidx=imagecache[iidx].hnext) {
        if (imagecache[iidx].sprite!=sprite) continue;

        ic_best(iidx);

        // remove from old pos
        nidx=imagecache[iidx].hnext;
        pidx=imagecache[iidx].hprev;

        if (pidx==-1) ima_hash[sprite%IMA_HASH_SIZE]=nidx;
        else imagecache[pidx].hnext=imagecache[iidx].hnext;

        if (nidx!=-1) imagecache[nidx].hprev=imagecache[iidx].hprev;

        // add to top pos
        nidx=ima_hash[sprite%IMA_HASH_SIZE];

        if (nidx!=-1) imagecache[nidx].hprev=iidx;

        imagecache[iidx].hprev=-1;
        imagecache[iidx].hnext=nidx;

        ima_hash[sprite%IMA_HASH_SIZE]=iidx;

        return iidx;
    }

    // find free "iidx=rrand(max_imagecache);"
    iidx=iidx_last;

    // delete
    if (imagecache[iidx].sprite!=SPRITE_NONE) {
        nidx=imagecache[iidx].hnext;
        pidx=imagecache[iidx].hprev;

        if (pidx==-1) ima_hash[imagecache[iidx].sprite%IMA_HASH_SIZE]=nidx;
        else imagecache[pidx].hnext=imagecache[iidx].hnext;

        if (nidx!=-1) imagecache[nidx].hprev=imagecache[iidx].hprev;

        imagecache[iidx].sprite=SPRITE_NONE;
    }

    // build
    if (gfx_load_image(&imagecache[iidx].image,sprite)) { ic_last(iidx); fail("ic_load: load_image(%u,%u) failed",iidx,sprite); return IIDX_NONE; }

    // init
    imagecache[iidx].sprite=sprite;
    ic_best(iidx);

    nidx=ima_hash[sprite%IMA_HASH_SIZE];

    if (nidx!=-1) imagecache[nidx].hprev=iidx;

    imagecache[iidx].hprev=-1;
    imagecache[iidx].hnext=nidx;

    ima_hash[sprite%IMA_HASH_SIZE]=iidx;

    im_time+=GetTickCount()-start;

    return iidx;
}

struct prefetch {
    int attick;
    int sprite;
    signed char sink;
    unsigned char scale,cr,cg,cb,light,sat;
    unsigned short c1,c2,c3,shine;
    char ml,ll,rl,ul,dl;
    unsigned char freeze;
    unsigned char grid;
};

#define MAXPRE (16384)
static struct prefetch pre[MAXPRE];
static int pre_in=0,pre_out=0;

static int sc_load(int sprite,int sink,int freeze,int grid,int scale,int cr,int cg,int cb,int light,int sat,int c1,int c2,int c3,int shine,int ml,int ll,int rl,int ul,int dl,int checkonly,int isprefetch);

void pre_add(int attick,int sprite,signed char sink,unsigned char freeze,unsigned char grid,unsigned char scale,char cr,char cg,char cb,char light,char sat,int c1,int c2,int c3,int shine,char ml,char ll,char rl,char ul,char dl) {
    if ((pre_in+1)%MAXPRE==pre_out) return; // buffer is full

    if (sprite>MAXSPRITE || sprite<0) {
        note("illegal sprite %d wanted in pre_add",sprite);
        return;
    }
    if (sc_load(sprite,sink,freeze,grid,scale,cr,cg,cb,light,sat,c1,c2,c3,shine,ml,ll,rl,ul,dl,1,1)) return;    // already in systemcache

    pre[pre_in].attick=attick;
    pre[pre_in].sprite=sprite;
    pre[pre_in].sink=sink;
    pre[pre_in].freeze=freeze;
    pre[pre_in].grid=grid;
    pre[pre_in].scale=scale;
    pre[pre_in].cr=cr;
    pre[pre_in].cg=cg;
    pre[pre_in].cb=cb;
    pre[pre_in].light=light;
    pre[pre_in].sat=sat;
    pre[pre_in].c1=c1;
    pre[pre_in].c2=c2;
    pre[pre_in].c3=c3;
    pre[pre_in].shine=shine;
    pre[pre_in].ml=ml;
    pre[pre_in].ll=ll;
    pre[pre_in].rl=rl;
    pre[pre_in].dl=dl;
    pre[pre_in].ul=ul;

    //note("add_pre: %d %d %d %d %d %d",pre[pre_in].sprite,pre[pre_in].ml,pre[pre_in].ll,pre[pre_in].rl,pre[pre_in].ul,pre[pre_in].dl);

    pre_in=(pre_in+1)%MAXPRE;
}

int pre_do(int curtick) {
    while (pre[pre_out].attick<curtick && pre_in!=pre_out) pre_out=(pre_out+1)%MAXPRE;

    if (pre_in==pre_out) return 0;  // prefetch buffer is empty

    // load into systemcache
    sc_load(pre[pre_out].sprite,
            pre[pre_out].sink,
            pre[pre_out].freeze,
            pre[pre_out].grid,
            pre[pre_out].scale,
            pre[pre_out].cr,
            pre[pre_out].cg,
            pre[pre_out].cb,
            pre[pre_out].light,
            pre[pre_out].sat,
            pre[pre_out].c1,
            pre[pre_out].c2,
            pre[pre_out].c3,
            pre[pre_out].shine,
            pre[pre_out].ml,
            pre[pre_out].ll,
            pre[pre_out].rl,
            pre[pre_out].ul,
            pre[pre_out].dl,0,1);
    //note("pre_do: %d %d %d %d %d %d",pre[pre_out].sprite,pre[pre_out].ml,pre[pre_out].ll,pre[pre_out].rl,pre[pre_out].ul,pre[pre_out].dl);

    pre_out=(pre_out+1)%MAXPRE;

    if (pre_in>=pre_out) return pre_in-pre_out;
    else return MAXPRE+pre_in-pre_out;
}

static unsigned short shine_pix(unsigned short irgb,unsigned short shine) {
    double r,g,b;

    if (irgb==rgbcolorkey) return irgb;

    r=IGET_R(irgb)/15.5;
    g=IGET_G(irgb)/15.5;
    b=IGET_B(irgb)/15.5;

    r=((r*r*r*r)*shine+r*(100-shine))/200;
    g=((g*g*g*g)*shine+g*(100-shine))/200;
    b=((b*b*b*b)*shine+b*(100-shine))/200;

    if (r>1) r=1;
    if (g>1) g=1;
    if (b>1) b=1;

    irgb=IRGB((int)(r*31),(int)(g*31),(int)(b*31));

    if (irgb==rgbcolorkey) return irgb-1;
    else return irgb;
}

#define REDCOL		(0.40)
#define GREENCOL	(0.70)
#define BLUECOL		(0.70)

static int colorize_pix(unsigned short irgb,unsigned short c1v,unsigned short c2v,unsigned short c3v) {
    double rf,gf,bf,m,str,rm,gm,bm,rv,gv,bv;
    double c1=0,c2=0,c3=0;
    double shine=0;
    int r,g,b;

    if (irgb==rgbcolorkey) return irgb;

    rf=IGET_R(irgb)/32.0;
    gf=IGET_G(irgb)/32.0;
    bf=IGET_B(irgb)/32.0;

    m=max(max(rf,gf),bf)+0.000001;
    rm=rf/m; gm=gf/m; bm=bf/m;

    // channel 1: green max
    if (c1v && gm>0.99 && rm<GREENCOL && bm<GREENCOL) {
        c1=gf-max(bf,rf);
        if (c1v&0x8000) shine+=gm-max(rm,bm);

        gf-=c1;
    }

    m=max(max(rf,gf),bf)+0.000001;
    rm=rf/m; gm=gf/m; bm=bf/m;

    // channel 2: blue max
    if (c2v && bm>0.99 && rm<BLUECOL && gm<BLUECOL) {
        c2=bf-max(rf,gf);
        if (c2v&0x8000) shine+=bm-max(rm,gm);

        bf-=c2;
    }

    m=max(max(rf,gf),bf)+0.000001;
    rm=rf/m; gm=gf/m; bm=bf/m;

    // channel 3: red max
    if (c3v && rm>0.99 && gm<REDCOL && bm<REDCOL) {
        c3=rf-max(gf,bf);
        if (c3v&0x8000) shine+=rm-max(gm,bm);

        rf-=c3;
    }

    // sanity
    rf=max(0,rf);
    gf=max(0,gf);
    bf=max(0,bf);

    // collect
    r=min(31,
          2*c1*IGET_R(c1v)+
          2*c2*IGET_R(c2v)+
          2*c3*IGET_R(c3v)+
          rf*31);
    g=min(31,
          2*c1*IGET_G(c1v)+
          2*c2*IGET_G(c2v)+
          2*c3*IGET_G(c3v)+
          gf*31);
    b=min(31,
          2*c1*IGET_B(c1v)+
          2*c2*IGET_B(c2v)+
          2*c3*IGET_B(c3v)+
          bf*31);

    irgb=IRGB(r,g,b);

    if (shine>0.1) irgb=shine_pix(irgb,(int)(shine*50));

    if (irgb==rgbcolorkey) return irgb-1;
    else return irgb;
}

static IMAGE* ic_merge(IMAGE *a,IMAGE *b) {
    IMAGE *c;
    int x,y;
    int x1,x2,y1,y2;
    int a1,rgb1;
    int a2,rgb2;

    c=xmalloc(sizeof(IMAGE),MEM_IC);
    c->xres=max(a->xres,b->xres);
    c->yres=max(a->yres,b->yres);
    c->xoff=min(a->xoff,b->xoff);
    c->yoff=min(a->yoff,b->yoff);

    x1=c->xoff-a->xoff;
    x2=c->xoff-b->xoff;
    y1=c->yoff-a->yoff;
    y2=c->yoff-b->yoff;

    //note("cxres=%d, cyres=%d, axres=%d, ayres=%d, bxres=%d, byres=%d, x1=%d, x2=%d, y1=%d, y2=%d",c->xres,c->yres,a->xres,a->yres,b->xres,b->yres,x1,x2,y1,y2);

    c->a=xmalloc(c->xres*c->yres*sizeof(unsigned char),MEM_IC);
    c->rgb=xmalloc(c->xres*c->yres*sizeof(unsigned short),MEM_IC);

    for (y=0; y<c->yres; y++) {
        for (x=0; x<c->xres; x++) {
            if (x+x1<0 || x+x1>=a->xres || y+y1<0 || y+y1>=a->yres) {
                a1=rgb1=0;
            } else {
                a1=a->a[(x+x1)+(y+y1)*a->xres];
                rgb1=a->rgb[(x+x1)+(y+y1)*a->xres];
            }

            if (x+x2<0 || x+x2>=b->xres || y+y2<0 || y+y2>=b->yres) {
                a2=rgb2=0;
            } else {
                a2=b->a[(x+x2)+(y+y2)*b->xres];
                rgb2=b->rgb[(x+x2)+(y+y2)*b->xres];
            }

            c->a[x+y*c->xres]=max(a1,a2);

            if (IGET_R(rgb2)>1 ||
                IGET_G(rgb2)>1 ||
                IGET_B(rgb2)>1) c->rgb[x+y*c->xres]=rgb2;
            else c->rgb[x+y*c->xres]=rgb1;
        }
    }

    return c;
}

static unsigned short colorbalance(unsigned short irgb,char cr,char cg,char cb,char light,char sat) {
    int r,g,b,grey;

    if (irgb==rgbcolorkey) return irgb;

    r=IGET_R(irgb)*8;
    g=IGET_G(irgb)*8;
    b=IGET_B(irgb)*8;

    // lightness
    if (light) {
        r+=light; g+=light; b+=light;
    }

    // saturation
    if (sat) {
        grey=(r+g+b)/3;
        r=((r*(20-sat))+(grey*sat))/20;
        g=((g*(20-sat))+(grey*sat))/20;
        b=((b*(20-sat))+(grey*sat))/20;
    }

    // color balancing
    cr*=0.75; cg*=0.75; cg*=0.75;

    r+=cr; g-=cr/2; b-=cr/2;
    r-=cg/2; g+=cg; b-=cg/2;
    r-=cb/2; g-=cb/2; b+=cb;

    if (r<0) r=0;
    if (g<0) g=0;
    if (b<0) b=0;

    if (r>255) { g+=(r-255)/2; b+=(r-255)/2; r=255; }
    if (g>255) { r+=(g-255)/2; b+=(g-255)/2; g=255; }
    if (b>255) { r+=(b-255)/2; g+=(b-255)/2; b=255; }

    if (r>255) r=255;
    if (g>255) g=255;
    if (b>255) b=255;

    irgb=IRGB(r/8,g/8,b/8);

    if (irgb==rgbcolorkey) return irgb-1;
    else return irgb;
}

static void sc_make_slow(SYSTEMCACHE *sc,IMAGE *image,signed char sink,unsigned char freeze,unsigned char grid,unsigned char scale,char cr,char cg,char cb,char light,char sat,unsigned short c1v,unsigned short c2v,unsigned short c3v,unsigned short shine,char ml,char ll,char rl,char ul,char dl) {
    int x,y,amax,xm;
    double ix,iy,low_x,low_y,high_x,high_y,dbr,dbg,dbb,dba;
    unsigned short int irgb,*sptr;
    unsigned char a;
    int a_r,a_g,a_b,b_r,b_g,b_b,r,g,b,da,db;

    if (image->xres==0 || image->yres==0) scale=100;    // !!! needs better handling !!!

    if (scale!=100) {
        sc->xres=ceil((double)(image->xres-1)*scale/100.0);
        sc->yres=ceil((double)(image->yres-1)*scale/100.0);
        sc->size=sc->xres*sc->yres;

        sc->xoff=floor(image->xoff*scale/100.0+0.5);
        sc->yoff=floor(image->yoff*scale/100.0+0.5);
    } else {
        sc->xres=(int)image->xres; //*scale/100;
        sc->yres=(int)image->yres; //*scale/100;
        sc->size=sc->xres*sc->yres;
        sc->xoff=image->xoff;
        sc->yoff=image->yoff;
    }

    if (sink) sink=min(sink,max(0,sc->yres-4));

    sc->rgb=xrealloc(sc->rgb,sc->xres*sc->yres*sizeof(unsigned short int),MEM_SC);

    amax=sc->acnt;
    sc->acnt=0;

    for (y=0; y<sc->yres; y++) {
        for (x=0; x<sc->xres; x++) {

            if (scale!=100) {
                ix=x*100.0/scale;
                iy=y*100.0/scale;

                high_x=ix-floor(ix);
                high_y=iy-floor(iy);
                low_x=1-high_x;
                low_y=1-high_y;

                irgb=image->rgb[floor(ix)+floor(iy)*image->xres];

                if (irgb==rgbcolorkey) {
                    dbr=0;
                    dbg=0;
                    dbb=0;
                    dba=0;
                } else {
                    if (c1v || c2v || c3v) irgb=colorize_pix(irgb,c1v,c2v,c3v);
                    dbr=IGET_R(irgb)*low_x*low_y;
                    dbg=IGET_G(irgb)*low_x*low_y;
                    dbb=IGET_B(irgb)*low_x*low_y;
                    if (!image->a) dba=31*low_x*low_y;
                    else dba=image->a[floor(ix)+floor(iy)*image->xres]*low_x*low_y;
                }

                irgb=image->rgb[ceil(ix)+floor(iy)*image->xres];

                if (irgb==rgbcolorkey) {
                    dbr+=0;
                    dbg+=0;
                    dbb+=0;
                    dba+=0;
                } else {
                    if (c1v || c2v || c3v) irgb=colorize_pix(irgb,c1v,c2v,c3v);
                    dbr+=IGET_R(irgb)*high_x*low_y;
                    dbg+=IGET_G(irgb)*high_x*low_y;
                    dbb+=IGET_B(irgb)*high_x*low_y;
                    if (!image->a) dba+=31*high_x*low_y;
                    else dba+=image->a[ceil(ix)+floor(iy)*image->xres]*high_x*low_y;
                }

                irgb=image->rgb[floor(ix)+ceil(iy)*image->xres];

                if (irgb==rgbcolorkey) {
                    dbr+=0;
                    dbg+=0;
                    dbb+=0;
                    dba+=0;
                } else {
                    if (c1v || c2v || c3v) irgb=colorize_pix(irgb,c1v,c2v,c3v);
                    dbr+=IGET_R(irgb)*low_x*high_y;
                    dbg+=IGET_G(irgb)*low_x*high_y;
                    dbb+=IGET_B(irgb)*low_x*high_y;
                    if (!image->a) dba+=31*low_x*high_y;
                    else dba+=image->a[floor(ix)+ceil(iy)*image->xres]*low_x*high_y;
                }

                irgb=image->rgb[ceil(ix)+ceil(iy)*image->xres];

                if (irgb==rgbcolorkey) {
                    dbr+=0;
                    dbg+=0;
                    dbb+=0;
                    dba+=0;
                } else {
                    if (c1v || c2v || c3v) irgb=colorize_pix(irgb,c1v,c2v,c3v);
                    dbr+=IGET_R(irgb)*high_x*high_y;
                    dbg+=IGET_G(irgb)*high_x*high_y;
                    dbb+=IGET_B(irgb)*high_x*high_y;

                    if (!image->a) dba+=31*high_x*high_y;
                    else dba+=image->a[ceil(ix)+ceil(iy)*image->xres]*high_x*high_y;
                }


                irgb=IRGB(((int)dbr),((int)dbg),((int)dbb));
                a=((int)dba);

                if (a>31) { note("oops: %d %d %d %d (%.2f %.2f %.2f %.2f) (%d,%d of %d,%d) (%.2f, %.2f of %d,%d)",
                                 a,(int)dbr,(int)dbg,(int)dbb,low_x,low_y,high_x,high_y,
                                 x,y,sc->xres,sc->yres,
                                 ix,iy,image->xres,image->yres); }
            } else {
                irgb=image->rgb[x+y*image->xres];
                if (c1v || c2v || c3v) irgb=colorize_pix(irgb,c1v,c2v,c3v);

                if (irgb==rgbcolorkey) a=0;
                else if (!image->a) a=31;
                else a=image->a[x+y*image->xres];
            }

            if ((cr || cg || cb || light || sat) && irgb!=rgbcolorkey) irgb=colorbalance(irgb,cr,cg,cb,light,sat);
            if (shine) irgb=shine_pix(irgb,shine);

            sptr=&sc->rgb[x+y*sc->xres];

            if (ll!=ml || rl!=ml || ul!=ml || dl!=ml) {
                int r1,r2,r3,r4,r5;
                int g1,g2,g3,g4,g5;
                int b1,b2,b3,b4,b5;
                int v1,v2,v3,v4,v5;
                int div;

                if (y<10+(20-abs(20-x))/2) {
                    if (x/2<20-y) {
                        v2=-(x/2-(20-y))+1;
                        r2=IGET_R(rgbfx_light[ll][irgb]);
                        g2=IGET_G(rgbfx_light[ll][irgb]);
                        b2=IGET_B(rgbfx_light[ll][irgb]);
                    } else v2=0;
                    if (x/2>20-y) {
                        v3=(x/2-(20-y))+1;
                        r3=IGET_R(rgbfx_light[rl][irgb]);
                        g3=IGET_G(rgbfx_light[rl][irgb]);
                        b3=IGET_B(rgbfx_light[rl][irgb]);
                    } else v3=0;
                    if (x/2>y) {
                        v4=(x/2-y)+1;
                        r4=IGET_R(rgbfx_light[ul][irgb]);
                        g4=IGET_G(rgbfx_light[ul][irgb]);
                        b4=IGET_B(rgbfx_light[ul][irgb]);
                    } else v4=0;
                    if (x/2<y) {
                        v5=-(x/2-y)+1;
                        r5=IGET_R(rgbfx_light[dl][irgb]);
                        g5=IGET_G(rgbfx_light[dl][irgb]);
                        b5=IGET_B(rgbfx_light[dl][irgb]);
                    } else v5=0;
                } else {
                    if (x<10) {
                        v2=(10-x)*2-2;
                        r2=IGET_R(rgbfx_light[ll][irgb]);
                        g2=IGET_G(rgbfx_light[ll][irgb]);
                        b2=IGET_B(rgbfx_light[ll][irgb]);
                    } else v2=0;
                    if (x>10 && x<20) {
                        v3=(x-10)*2-2;
                        r3=IGET_R(rgbfx_light[rl][irgb]);
                        g3=IGET_G(rgbfx_light[rl][irgb]);
                        b3=IGET_B(rgbfx_light[rl][irgb]);
                    } else v3=0;
                    if (x>20 && x<30) {
                        v5=(10-(x-20))*2-2;
                        r5=IGET_R(rgbfx_light[dl][irgb]);
                        g5=IGET_G(rgbfx_light[dl][irgb]);
                        b5=IGET_B(rgbfx_light[dl][irgb]);
                    } else v5=0;
                    if (x>30 && x<40) {
                        v4=(x-30)*2-2;
                        r4=IGET_R(rgbfx_light[ul][irgb]);
                        g4=IGET_G(rgbfx_light[ul][irgb]);
                        b4=IGET_B(rgbfx_light[ul][irgb]);
                    } else v4=0;
                }

                //addline("v1=%d, v2=%d, v3=%d, v4=%d, v5=%d",v1,v2,v3,v4,v5);

                v1=20-(v2+v3+v4+v5)/2;
                r1=IGET_R(rgbfx_light[ml][irgb]);
                g1=IGET_G(rgbfx_light[ml][irgb]);
                b1=IGET_B(rgbfx_light[ml][irgb]);

                div=v1+v2+v3+v4+v5;

                r=(r1*v1+r2*v2+r3*v3+r4*v4+r5*v5)/div;
                g=(g1*v1+g2*v2+g3*v3+g4*v4+g5*v5)/div;
                b=(b1*v1+b2*v2+b3*v3+b4*v4+b5*v5)/div;

                irgb=IRGB(r,g,b);

            } else  irgb=rgbfx_light[ml][irgb];

            if (sink) {
                if (sc->yres-sink<y) a=0;
            }

            if (freeze) irgb=rgbfx_freeze[freeze][irgb];
            if (grid==DDFX_LEFTGRID) { if ((sc->xoff+x+sc->yoff+y)&1) a=0; }
            if (grid==DDFX_RIGHTGRID) {  if ((sc->xoff+x+sc->yoff+y+1)&1) a=0; }

            if (a==31) {
                *sptr=rgb2scr[irgb];
            } else if (a==0) {
                *sptr=scrcolorkey;  //rgbcolorkey;
            } else {
                if (sc->acnt==amax) {
                    amax+=64;
                    sc->apix=xrealloc(sc->apix,amax*sizeof(APIX),MEM_SC);
                }

                sc->apix[sc->acnt].x=x;
                sc->apix[sc->acnt].y=y;
                sc->apix[sc->acnt].a=a;
                sc->apix[sc->acnt].rgb=irgb;

                sc->acnt++;

                *sptr=scrcolorkey;  //rgbcolorkey;
            }
        }
    }

    sc->apix=xrealloc(sc->apix,sc->acnt*sizeof(APIX),MEM_SC);
}

#pragma argsused
static void sc_make_fast(SYSTEMCACHE *sc,IMAGE *image,signed char sink,unsigned char freeze,unsigned char grid,unsigned char scale,char cr,char cg,char cb,char light,char sat,unsigned short c1v,unsigned short c2v,unsigned short c3v,unsigned short shine,char ml,char ll,char rl,char ul,char dl) {
    int x,y,amax,xm,pos;
    unsigned short int irgb;
    unsigned char a;
    int need_colorize=0;
    int need_colorbalance=0;

    sc->xres=(int)image->xres;
    sc->yres=(int)image->yres;
    sc->size=sc->xres*sc->yres;
    sc->xoff=image->xoff;
    sc->yoff=image->yoff;

    sc->rgb=xrealloc(sc->rgb,sc->xres*sc->yres*sizeof(unsigned short int),MEM_SC);

    amax=sc->acnt;
    sc->acnt=0;

    if (c1v || c2v || c3v) need_colorize=1;
    if (cr || cg || cb || light || sat) need_colorbalance=1;

    for (y=pos=0; y<sc->yres; y++) {
        for (x=0; x<sc->xres; x++,pos++) {

            irgb=image->rgb[pos];

            if (irgb==rgbcolorkey) {
                sc->rgb[pos]=scrcolorkey;
                continue;
            } else {
                if (!image->a) a=31;
                else a=image->a[pos];

                if (need_colorize) irgb=colorize_pix(irgb,c1v,c2v,c3v);
                if (need_colorbalance) irgb=colorbalance(irgb,cr,cg,cb,light,sat);
                if (shine) irgb=shine_pix(irgb,shine);

                irgb=rgbfx_light[ml][irgb];
                if (freeze) irgb=rgbfx_freeze[freeze][irgb];
            }

            if (a==31) {
                sc->rgb[pos]=rgb2scr[irgb];
            } else if (a==0) {
                sc->rgb[pos]=scrcolorkey;   //rgbcolorkey;
            } else {
                if (sc->acnt==amax) {
                    amax+=64;
                    sc->apix=xrealloc(sc->apix,amax*sizeof(APIX),MEM_SC);
                }

                sc->apix[sc->acnt].x=x;
                sc->apix[sc->acnt].y=y;
                sc->apix[sc->acnt].a=a;
                sc->apix[sc->acnt].rgb=irgb;

                sc->acnt++;

                sc->rgb[pos]=scrcolorkey;   //rgbcolorkey;
            }
        }
    }

    sc->apix=xrealloc(sc->apix,sc->acnt*sizeof(APIX),MEM_SC);
}

static void sc_make(SYSTEMCACHE *sc,IMAGE *image,signed char sink,unsigned char freeze,unsigned char grid,unsigned char scale,char cr,char cg,char cb,char light,char sat,unsigned short c1v,unsigned short c2v,unsigned short c3v,unsigned short shine,char ml,char ll,char rl,char ul,char dl) {
    int start;

    start=GetTickCount();

    if (scale!=100 || ll!=ml || rl!=ml || ul!=ml || dl!=ml || grid || sink) {
        sc_make_slow(sc,image,sink,freeze,grid,scale,cr,cg,cb,light,sat,c1v,c2v,c3v,shine,ml,ll,rl,ul,dl);
        //printf("slow: %d %d %d %d\n",scale!=100,ll!=ml || rl!=ml || ul!=ml || dl!=ml,grid,sink);
    } else sc_make_fast(sc,image,sink,freeze,grid,scale,cr,cg,cb,light,sat,c1v,c2v,c3v,shine,ml,ll,rl,ul,dl);

    sc_time+=GetTickCount()-start;
    sm_cnt++;
}

int sys_hash_key2(int sprite,int ml,int cr,int cg,int cb,int light) {
    return ((sprite)+(ml<<4)+(cr<<8)+(cg<<12)+(cb<<16)+(light<<0))&(SYS_HASH_SIZE-1);
}

int sys_hash_key(int sidx) {
    return sys_hash_key2(systemcache[sidx].sprite,
                         systemcache[sidx].ml,
                         systemcache[sidx].cr,
                         systemcache[sidx].cg,
                         systemcache[sidx].cb,
                         systemcache[sidx].light);
}


static int sc_load(int sprite,int sink,int freeze,int grid,int scale,int cr,int cg,int cb,int light,int sat,int c1,int c2,int c3,int shine,int ml,int ll,int rl,int ul,int dl,int checkonly,int isprefetch) {
    int sidx,iidx,v,vmax,step,pidx,nidx;

    if (sprite>MAXSPRITE || sprite<0) {
        note("illegal sprite %d wanted in sc_load",sprite);
        return SIDX_NONE;
    }

    for (sidx=sys_hash[sys_hash_key2(sprite,ml,cr,cg,cb,light)],step=0; sidx!=-1; sidx=systemcache[sidx].hnext,step++) {
        if (systemcache[sidx].sprite!=sprite) continue;
        if (systemcache[sidx].sink!=sink) continue;
        if (systemcache[sidx].freeze!=freeze) continue;
        if (systemcache[sidx].grid!=grid) continue;
        if (systemcache[sidx].scale!=scale) continue;
        if (systemcache[sidx].cr!=cr) continue;
        if (systemcache[sidx].cg!=cg) continue;
        if (systemcache[sidx].cb!=cb) continue;
        if (systemcache[sidx].light!=light) continue;
        if (systemcache[sidx].sat!=sat) continue;
        if (systemcache[sidx].c1!=c1) continue;
        if (systemcache[sidx].c2!=c2) continue;
        if (systemcache[sidx].c3!=c3) continue;
        if (systemcache[sidx].shine!=shine) continue;
        if (systemcache[sidx].ml!=ml) continue;
        if (systemcache[sidx].ll!=ll) continue;
        if (systemcache[sidx].rl!=rl) continue;
        if (systemcache[sidx].ul!=ul) continue;
        if (systemcache[sidx].dl!=dl) continue;

        if (checkonly) return 1;

        sc_hit++;
        sc_best(sidx);

        sc_maxstep+=step;

        // remove from old pos
        nidx=systemcache[sidx].hnext;
        pidx=systemcache[sidx].hprev;

        if (pidx==-1) sys_hash[sys_hash_key(sidx)]=nidx;
        else systemcache[pidx].hnext=systemcache[sidx].hnext;

        if (nidx!=-1) systemcache[nidx].hprev=systemcache[sidx].hprev;

        // add to top pos
        nidx=sys_hash[sys_hash_key(sidx)];

        if (nidx!=-1) systemcache[nidx].hprev=sidx;

        systemcache[sidx].hprev=-1;
        systemcache[sidx].hnext=nidx;

        sys_hash[sys_hash_key(sidx)]=sidx;

        return sidx;
    }

    if (checkonly) return 0;

    if (!isprefetch) {
        sc_miss++;
    }

    // find free "sidx=rrand(max_systemcache);"
    sidx=sidx_last;

    // delete
    if (systemcache[sidx].sprite!=SPRITE_NONE) {

        //printf("PAAAAANIC!\n");

        nidx=systemcache[sidx].hnext;
        pidx=systemcache[sidx].hprev;

        if (pidx==-1) sys_hash[sys_hash_key(sidx)]=nidx;
        else systemcache[pidx].hnext=systemcache[sidx].hnext;

        if (nidx!=-1) systemcache[nidx].hprev=systemcache[sidx].hprev;

        systemcache[sidx].sprite=SPRITE_NONE;
    }

    // build
    iidx=ic_load(sprite);
    if (iidx==IIDX_NONE) { sc_last(sidx); return SIDX_NONE; }

    systemcache[sidx].sprite=sprite;    // dup
    sc_make(&systemcache[sidx],&imagecache[iidx].image,sink,freeze,grid,scale,cr,cg,cb,light,sat,c1,c2,c3,shine,ml,ll,rl,ul,dl);
    //if (systemcache[sidx].xres>TILESIZEDX || systemcache[sidx].yres>TILESIZEDY) addline("res=%d,%d",systemcache[sidx].xres,systemcache[sidx].yres);

    // init
    systemcache[sidx].sprite=sprite;
    systemcache[sidx].sink=sink;
    systemcache[sidx].freeze=freeze;
    systemcache[sidx].grid=grid;
    systemcache[sidx].scale=scale;
    systemcache[sidx].cr=cr;
    systemcache[sidx].cg=cg;
    systemcache[sidx].cb=cb;
    systemcache[sidx].light=light;
    systemcache[sidx].sat=sat;
    systemcache[sidx].c1=c1;
    systemcache[sidx].c2=c2;
    systemcache[sidx].c3=c3;
    systemcache[sidx].shine=shine;
    systemcache[sidx].ml=ml;
    systemcache[sidx].ll=ll;
    systemcache[sidx].rl=rl;
    systemcache[sidx].ul=ul;
    systemcache[sidx].dl=dl;

    nidx=sys_hash[sys_hash_key(sidx)];

    if (nidx!=-1) systemcache[nidx].hprev=sidx;

    systemcache[sidx].hprev=-1;
    systemcache[sidx].hnext=nidx;

    sys_hash[sys_hash_key(sidx)]=sidx;

    systemcache[sidx].tdx=(systemcache[sidx].xres+TILESIZEDX-1)/TILESIZEDX;
    systemcache[sidx].tdy=(systemcache[sidx].yres+TILESIZEDY-1)/TILESIZEDY;

    systemcache[sidx].lost=1;

    sc_best(sidx);

    return sidx;
}

static int sc_blit2(DDFX *ddfx,int sidx,int scrx,int scry);
int dd_copysprite_fx(DDFX *ddfx,int scrx,int scry) {
    int sidx;

    PARANOIA(if (!ddfx) paranoia("dd_copysprite_fx: ddfx=NULL"); )
    PARANOIA(if (ddfx->light<0 || ddfx->light>16) paranoia("dd_copysprite_fx: ddfx->light=%d",ddfx->light); )
    PARANOIA(if (ddfx->freeze<0 || ddfx->freeze>=DDFX_MAX_FREEZE) paranoia("dd_copysprite_fx: ddfx->freeze=%d",ddfx->freeze); )

    sidx=sc_load(ddfx->sprite,
                 ddfx->sink,
                 ddfx->freeze,
                 ddfx->grid,
                 ddfx->scale,
                 ddfx->cr,
                 ddfx->cg,
                 ddfx->cb,
                 ddfx->clight,
                 ddfx->sat,
                 ddfx->c1,
                 ddfx->c2,
                 ddfx->c3,
                 ddfx->shine,
                 ddfx->ml,
                 ddfx->ll,
                 ddfx->rl,
                 ddfx->ul,
                 ddfx->dl,0,0);

    if (sidx==SIDX_NONE) return 0;

    // note("sprite=%d xoff=%d yoff=%d",imagecache[iidx].sprite,imagecache[iidx].image.xoff,imagecache[iidx].image.yoff);

    // shift position according to align
    if (ddfx->align==DD_OFFSET) {
        scrx+=systemcache[sidx].xoff;
        scry+=systemcache[sidx].yoff;
    } else if (ddfx->align==DD_CENTER) {
        scrx-=systemcache[sidx].xres/2;
        scry-=systemcache[sidx].yres/2;
    }

    // add the additional cliprect
    if (ddfx->clipsx!=ddfx->clipex || ddfx->clipsy!=ddfx->clipey) {
        dd_push_clip();
        if (ddfx->clipsx!=ddfx->clipex) dd_more_clip(scrx-systemcache[sidx].xoff+ddfx->clipsx,clipsy,scrx-systemcache[sidx].xoff+ddfx->clipex,clipey);
        if (ddfx->clipsy!=ddfx->clipey) dd_more_clip(clipsx,scry-systemcache[sidx].yoff+ddfx->clipsy,clipex,scry-systemcache[sidx].yoff+ddfx->clipey);
    }

    // blit it
    sc_blit2(ddfx,sidx,scrx,scry);

    // remove additional cliprect
    if (ddfx->clipsx!=ddfx->clipex || ddfx->clipsy!=ddfx->clipey) dd_pop_clip();

    return 1;
}

void dd_copysprite_callfx(int sprite,int scrx,int scry,int light,int ml,int grid,int align) {
    DDFX ddfx;

    bzero(&ddfx,sizeof(DDFX));

    ddfx.sprite=sprite;
    if (light<1000) ddfx.light=DDFX_NLIGHT;
    ddfx.grid=grid;
    ddfx.align=align;

    ddfx.ml=ddfx.ll=ddfx.rl=ddfx.ul=ddfx.dl=ml;
    ddfx.sink=0;
    ddfx.scale=100;
    ddfx.cr=ddfx.cg=ddfx.cb=ddfx.clight=ddfx.sat=0;
    ddfx.c1=ddfx.c2=ddfx.c3=ddfx.shine=0;

    dd_copysprite_fx(&ddfx,scrx,scry);
}

void dd_copysprite(int sprite,int scrx,int scry,int light,int align) {
    DDFX ddfx;

    bzero(&ddfx,sizeof(DDFX));

    ddfx.sprite=sprite;
    ddfx.light=DDFX_NLIGHT;
    ddfx.align=align;

    ddfx.ml=ddfx.ll=ddfx.rl=ddfx.ul=ddfx.dl=light;
    ddfx.sink=0;
    ddfx.scale=100;
    ddfx.cr=ddfx.cg=ddfx.cb=ddfx.clight=ddfx.sat=0;
    ddfx.c1=ddfx.c2=ddfx.c3=ddfx.shine=0;

    dd_copysprite_fx(&ddfx,scrx,scry);
}

void dd_copysprite_callfx_old(int sprite,int scrx,int scry,int fx,int align) {
    DDFX ddfx;
    int light;

    bzero(&ddfx,sizeof(DDFX));

    ddfx.sprite=sprite;

    light=fx%16;
    if (light==FX_BRIGHT) ddfx.light=0;
    else ddfx.light=15-light;

    ddfx.ml=ddfx.ll=ddfx.rl=ddfx.ul=ddfx.dl=ddfx.light;
    ddfx.sink=0;
    ddfx.scale=100;
    ddfx.cr=ddfx.cg=ddfx.cb=ddfx.clight=ddfx.sat=0;
    ddfx.c1=ddfx.c2=ddfx.c3=ddfx.shine=0;

    switch (fx/STARTSLICE) {
        case 3: ddfx.grid=DDFX_LEFTGRID; break;
        case 2: ddfx.grid=DDFX_RIGHTGRID; break;
        case 1: addline("aha"); break;
        default: ddfx.grid=0; break;
    }
    // ddfx.grid=0;
    ddfx.align=align;
    dd_copysprite_fx(&ddfx,scrx,scry);
}

void dd_rect(int sx,int sy,int ex,int ey,unsigned short int color) {
    int x,y,err;
    unsigned short *ptr;
    RECT rc;
    DDBLTFX bltfx;

    if (sx<clipsx) sx=clipsx;
    if (sy<clipsy) sy=clipsy;
    if (ex>clipex) ex=clipex;
    if (ey>clipey) ey=clipey;

    if (sx>ex || sy>ey) return;

    if ((ex-sx)*(ey-sy)>100) {  // large rect? use hardware then
        bzero(&bltfx,sizeof(bltfx));
        bltfx.dwSize=sizeof(bltfx);
        bltfx.u5.dwFillColor=color;

        rc.left=sx+x_offset;
        rc.top=sy+y_offset;
        rc.right=ex+x_offset;
        rc.bottom=ey+y_offset;

        if ((err=ddbs->lpVtbl->Blt(ddbs,&rc,NULL,NULL,DDBLT_COLORFILL|DDBLT_WAIT,&bltfx))!=DD_OK) {
            char buf[80];
            sprintf(buf,"dd_rect(): %d,%d -> %d,%d (%d,%d)",sx,sy,ex,ey,xres,yres);
            dd_error(buf,err);
        }
    } else {
        if ((vidptr=ptr=dd_lock_surface(ddbs))==NULL) return;

        ptr+=(sx+x_offset)+(sy+y_offset)*xres;

        for (y=sy; y<ey; y++,ptr+=xres-(ex-sx)) {
            for (x=sx; x<ex; x++,ptr++) {
                if (ptr-vidptr>=xres*yres || ptr<vidptr) {
                    note("PANIC #3");
                    ptr=vidptr;
                }
                *ptr=color;
            }
        }

        dd_unlock_surface(ddbs);
    }
}

void dd_black(void) {
    int err;
    RECT rc;
    DDBLTFX bltfx;
    char buf[256];


    bzero(&bltfx,sizeof(bltfx));
    bltfx.dwSize=sizeof(bltfx);
    bltfx.u5.dwFillColor=0;

    rc.left=0;
    rc.top=0;
    rc.right=XRES;  //xres;
    rc.bottom=YRES; //yres;

    if ((err=ddbs->lpVtbl->Blt(ddbs,&rc,NULL,NULL,DDBLT_COLORFILL|DDBLT_WAIT,&bltfx))!=DD_OK) {
        //sprintf(buf,"dd_black(ddbs,%d,%d):",xres,yres);
        dd_error(buf,err);
    }
}

void dd_shaded_rect(int sx,int sy,int ex,int ey) {
    int x,y,r,g,b;
    unsigned short *ptr,col;

    if (sx<clipsx) sx=clipsx;
    if (sy<clipsy) sy=clipsy;
    if (ex>clipex) ex=clipex;
    if (ey>clipey) ey=clipey;

    if ((vidptr=ptr=dd_lock_surface(ddbs))==NULL) return;

    ptr+=(sx+x_offset)+(sy+y_offset)*xres;

    for (y=sy; y<ey; y++,ptr+=xres-(ex-sx)) {
        for (x=sx; x<ex; x++,ptr++) {
            if (ptr-vidptr>=xres*yres || ptr<vidptr) {
                note("PANIC #3");
                ptr=vidptr;
            }
            col=*ptr;
            col=scr2rgb[col];

            r=IGET_R(col);
            g=IGET_G(col);
            b=IGET_B(col);
            r=min(31,r+16);
            g=min(31,g+16);

            col=IRGB(r,g,b);
            col=rgb2scr[col];

            *ptr=col;
        }
    }

    dd_unlock_surface(ddbs);
}

void dd_line(int fx,int fy,int tx,int ty,unsigned short col) {
    unsigned short *ptr,val;
    int dx,dy,x,y,rx,ry;

    if (fx<clipsx) fx=clipsx;
    if (fy<clipsy) fy=clipsy;
    if (fx>=clipex) fx=clipex-1;
    if (fy>=clipey) fy=clipey-1;

    if (tx<clipsx) tx=clipsx;
    if (ty<clipsy) ty=clipsy;
    if (tx>=clipex) tx=clipex-1;
    if (ty>=clipey) ty=clipey-1;

    fx+=x_offset; tx+=x_offset;
    fy+=y_offset; ty+=y_offset;

    dx=(tx-fx);
    dy=(ty-fy);

    if (dx==0 && dy==0) return;

    if (abs(dx)>abs(dy)) { dy=dy*1024/abs(dx); dx=dx*1024/abs(dx); } else { dx=dx*1024/abs(dy); dy=dy*1024/abs(dy); }

    x=fx*1024+512;
    y=fy*1024+512;

    if ((vidptr=ptr=dd_lock_surface(ddbs))==NULL) return;

    rx=x/1024; ry=y/1024;
    while (1) {
        if (rx+ry*xres>=xres*yres || rx+ry*xres<0) {
            note("PANIC #4B");
            break;
        }
        ptr[rx+ry*xres]=col;

        x+=dx;
        y+=dy;
        rx=x/1024; ry=y/1024;
        if (rx==tx && ry==ty) break;
    }

    dd_unlock_surface(ddbs);
}

void dd_line2(int fx,int fy,int tx,int ty,unsigned short col,unsigned short *ptr) {
    unsigned short val;
    int dx,dy,x,y,rx,ry;

    if (fx<clipsx) fx=clipsx;
    if (fy<clipsy) fy=clipsy;
    if (fx>=clipex) fx=clipex-1;
    if (fy>=clipey) fy=clipey-1;

    if (tx<clipsx) tx=clipsx;
    if (ty<clipsy) ty=clipsy;
    if (tx>=clipex) tx=clipex-1;
    if (ty>=clipey) ty=clipey-1;

    fx+=x_offset; tx+=x_offset;
    fy+=y_offset; ty+=y_offset;

    dx=(tx-fx);
    dy=(ty-fy);

    if (dx==0 && dy==0) return;

    if (abs(dx)>abs(dy)) { dy=dy*1024/abs(dx); dx=dx*1024/abs(dx); } else { dx=dx*1024/abs(dy); dy=dy*1024/abs(dy); }

    x=fx*1024+512;
    y=fy*1024+512;

    rx=x/1024; ry=y/1024;
    while (1) {
        if (rx+ry*xres>=xres*yres || rx+ry*xres<0) {
            note("PANIC #4B");
            break;
        }
        ptr[rx+ry*xres]=col;

        x+=dx;
        y+=dy;
        rx=x/1024; ry=y/1024;
        if (rx==tx && ry==ty) break;
    }
}

void dd_display_strike(int fx,int fy,int tx,int ty) {
    int mx,my;
    int dx,dy,d,l;
    unsigned short col;

    dx=abs(tx-fx);
    dy=abs(ty-fy);

    mx=(fx+tx)/2+15-rrand(30);
    my=(fy+ty)/2+15-rrand(30);

    if (dx>=dy) {
        for (d=-4; d<5; d++) {
            l=(4-abs(d))*4;
            col=rgb2scr[IRGB(l,l,31)];
            dd_line(fx,fy,mx,my+d,col);
            dd_line(mx,my+d,tx,ty,col);
        }
    } else {
        for (d=-4; d<5; d++) {
            l=(4-abs(d))*4;
            col=rgb2scr[IRGB(l,l,31)];
            dd_line(fx,fy,mx+d,my,col);
            dd_line(mx+d,my,tx,ty,col);
        }
    }
}



void dd_draw_curve(int cx,int cy,int nr,int size,unsigned short col) {
    unsigned short *ptr;
    int n,x,y;

    col=rgb2scr[col];

    if ((vidptr=ptr=dd_lock_surface(ddbs))==NULL) return;

    for (n=nr*90; n<nr*90+90; n+=4) {
        x=sin(n/360.0*M_PI*2)*size+cx;
        y=cos(n/360.0*M_PI*2)*size*2/3+cy;

        if (x<clipsx) continue;
        if (y<clipsy) continue;
        if (x>=clipex) continue;
        if (y+10>=clipey) continue;

        x+=x_offset; y+=y_offset;

        ptr[x+y*xres]=col;

        ptr[x+y*xres]=col;
        ptr[x+y*xres+xres*5]=col;
        //ptr[x+y*xres-xres*5]=col;
        ptr[x+y*xres+xres*10]=col;
        //ptr[x+y*xres-xres*10]=col;
    }

    dd_unlock_surface(ddbs);
}

void dd_display_pulseback(int fx,int fy,int tx,int ty) {
    int mx,my;
    int dx,dy,d,l;
    unsigned short col;

    dx=abs(tx-fx);
    dy=abs(ty-fy);

    mx=(fx+tx)/2+15-rrand(30);
    my=(fy+ty)/2+15-rrand(30);

    if (dx>=dy) {
        for (d=-4; d<5; d++) {
            l=(4-abs(d))*4;
            col=rgb2scr[IRGB(l,31,l)];
            dd_line(fx,fy,mx,my+d,col);
            dd_line(mx,my+d,tx,ty,col);
        }
    } else {
        for (d=-4; d<5; d++) {
            l=(4-abs(d))*4;
            col=rgb2scr[IRGB(l,31,l)];
            dd_line(fx,fy,mx+d,my,col);
            dd_line(mx+d,my,tx,ty,col);
        }
    }
}

// text

int dd_textlength(int flags,const char *text) {
    DDFONT *font;
    float *dim;
    float x;
    const char *c;

    if (flags&DD_SMALL) { font=fontb; dim=fontdim_b; }
    else if (flags&DD_BIG) { font=fontc; dim=fontdim_c; }
    else { font=fonta; dim=fontdim_a; }

    for (x=0,c=text; *c && *c!=DDT; c++) if (newtext) x+=dim[*c]; else x+=font[*c].dim;

    return (int)(x+0.5f);
}

int dd_textlen(int flags,const char *text,int n) {
    DDFONT *font;
    float x,*dim;
    const char *c;

    if (n<0) return dd_textlength(flags,text);

    if (flags&DD_SMALL) { font=fontb; dim=fontdim_b; }
    else if (flags&DD_BIG) { font=fontc; dim=fontdim_c; }
    else { font=fonta; dim=fontdim_a; }

    for (x=0,c=text; *c && *c!=DDT && n; c++,n--) if (newtext) x+=dim[*c]; else x+=font[*c].dim;

    return (int)(x+0.5f);
}

int dd_drawtext(int sx,int sy,unsigned short int color,int flags,const char *text) {
    unsigned short *ptr,*dst;
    unsigned char *rawrun;
    int x,y,start;
    const char *c;
    DDFONT *font;

    if (newtext) {
        float tmp,*dim,tmp2;

        if (sy<clipsy) return sx;
        if (sy>=clipey) return sx;

        if (flags&DD_SMALL) dim=fontdim_b;
        else if (flags&DD_BIG) dim=fontdim_c;
        else dim=fontdim_a;

        if (flags&DD_CENTER) {
        for (tmp2=0.0f,c=text; *c; c++) tmp2+=dim[*c];
            tmp2=tmp2/2.0f;
        } else if (flags&DD_RIGHT) {
            for (tmp2=0.0f,c=text; *c; c++) x+=dim[*c];
        } else tmp2=0.0f;

        tmp=sx;
        while (*text && *text!=DDT && tmp-tmp2+dim[*text]<clipsx) tmp+=dim[*text++];

        sx=(int)(tmp+0.5f);

        texter_add(sx+x_offset,sy+y_offset,color,flags,text);
        return sx+dd_textlength(flags,text);
    }

    start=GetTickCount();

    if (flags&DD__SHADEFONT) {
        if (flags&DD_SMALL) font=fontb_shaded;
        else if (flags&DD_BIG) font=fontc_shaded;
        else font=fonta_shaded;
    } else if (flags&DD__FRAMEFONT) {
        if (flags&DD_SMALL) font=fontb_framed;
        else if (flags&DD_BIG) font=fontc_framed;
        else font=fonta_framed;
    } else {
        if (flags&DD_SMALL) font=fontb;
        else if (flags&DD_BIG) font=fontc;
        else font=fonta;
    }
    if (!font) return 42;

    if (flags&DD_CENTER) {
        for (x=0,c=text; *c; c++) x+=font[*c].dim;
        sx-=x/2;
    } else if (flags&DD_RIGHT) {
        for (x=0,c=text; *c; c++) x+=font[*c].dim;
        sx-=x;
    }

    if (flags&DD_SHADE) {
        dd_drawtext(sx-1,sy-1,rgb2scr[IRGB(0,0,0)],DT_LEFT|(flags&(DD_SMALL|DD_BIG))|DD__SHADEFONT,text);
    } else if (flags&DD_FRAME) {
        dd_drawtext(sx-1,sy-1,rgb2scr[IRGB(0,0,0)],DT_LEFT|(flags&(DD_SMALL|DD_BIG))|DD__FRAMEFONT,text);
    }

    if (sy>=clipey) return sx;

    if ((vidptr=ptr=dd_lock_surface(ddbs))==NULL) return sx;

    while (*text && *text!=DDT && sx+font[*text].dim<clipsx) sx+=font[*text++].dim;

    while (*text && *text!=DDT) {

        if (*text<0) { note("PANIC: char over limit"); text++; continue; }

        rawrun=font[*text].raw;

        x=sx;
        y=sy;

        dst=ptr+(x+x_offset)+(y+y_offset)*xres;

        while (*rawrun!=255) {

            if (*rawrun==254) {
                y++;
                x=sx;
                rawrun++;
                if (y>=clipey) break;
                dst=ptr+(x+x_offset)+(y+y_offset)*xres;
                continue;
            }

            dst+=*rawrun;
            x+=*rawrun;

            if (x>=clipex) {
                while (*rawrun!=255 && *rawrun!=254) rawrun++;
                continue;
            }

            rawrun++;
            if (x>=clipsx && y>=clipsy) {
                if (dst-vidptr>xres*yres || dst<vidptr) {
                    note("PANIC #5");
                    dst=vidptr;
                }
                *dst=color;
                tp_cnt++;
            }
        }

        if (x>=clipex) break;

        sx+=font[*text++].dim;
    }

    dd_unlock_surface(ddbs);

    tp_time+=GetTickCount()-start;

    return sx;
}

int dd_drawtext_break(int x,int y,int breakx,unsigned short color,int flags,const char *ptr) {
    char buf[256];
    int xp,n;
    float size;

    xp=x;

    while (*ptr) {
        while (*ptr==' ') ptr++;

        for (n=0; n<256 && *ptr && *ptr!=' '; buf[n++]=*ptr++);
        buf[n]=0;

        size=dd_textlength(flags,buf);
        if (xp+size>breakx) {
            xp=x; y+=10;
        }
        dd_drawtext(xp,y,color,flags,buf);
        xp+=size+4;
    }
    return y+10;
}


static void dd_pixel_fast(int x,int y,unsigned short col,unsigned short *ptr) {
    if (x<0 || y<0 || x>=800 || y>=600) return; // !!!! xres yres ???
    if ((x+x_offset)>=xres || (y+y_offset)>=yres) {
        note("PANIC 5b - %d,%d %d,%d %X",x,y,x_offset,y_offset,col);
        return;
    }

    ptr[(x+x_offset)+(y+y_offset)*xres]=col;
    np_cnt++;
}

void dd_pixel(int x,int y,unsigned short col) {
    unsigned short *ptr;

    if ((ptr=dd_lock_surface(ddbs))==NULL) return;

    dd_pixel_fast(x,y,col,ptr);

    dd_unlock_surface(ddbs);
}

int dd_drawtext_fmt(int sx,int sy,unsigned short int color,int flags,const char *format,...) {
    char buf[1024];
    va_list va;

    va_start(va,format);
    vsprintf(buf,format,va);
    va_end(va);

    return dd_drawtext(sx,sy,color,flags,buf);
}


static int bless_init=0;
static int bless_sin[36];
static int bless_cos[36];
static int bless_hight[200];

void dd_draw_bless_pix(int x,int y,int nr,int color,int front,unsigned short *ptr) {
    int sy;

    sy=bless_sin[nr%36];
    if (front && sy<0) return;
    if (!front && sy>=0) return;

    x+=bless_cos[nr%36];
    y=y+sy+bless_hight[nr%200];

    if (x<clipsx || x>=clipex || y<clipsy || y>=clipey) return;

    dd_pixel_fast(x,y,color,ptr);
}

void dd_draw_rain_pix(int x,int y,int nr,int color,int front,unsigned short *ptr) {
    int sy;

    x+=((nr/30)%30)+15;
    sy=((nr/330)%20)+10;
    if (front && sy<0) return;
    if (!front && sy>=0) return;

    y=y+sy-((nr*2)%60)-60;

    if (x<clipsx || x>=clipex || y<clipsy || y>=clipey) return;

    dd_pixel_fast(x,y,color,ptr);
}

void dd_draw_bless(int x,int y,int ticker,int strength,int front) {
    int step,nr;
    double light;
    unsigned short *ptr;
    int start;
    //static int bless_time=0,bless_cnt=0;

    start=GetTickCount();

    if (!bless_init) {
        for (nr=0; nr<36; nr++) {
            bless_sin[nr]=sin((nr%36)/36.0*M_PI*2)*8;
            bless_cos[nr]=cos((nr%36)/36.0*M_PI*2)*16;
        }
        for (nr=0; nr<200; nr++) {
            bless_hight[nr]=-20+sin((nr%200)/200.0*M_PI*2)*20;
        }
        bless_init=1;
    }

    if ((ptr=dd_lock_surface(ddbs))==NULL) return;

    if (ticker>62) light=1.0;
    else light=(ticker)/62.0;

    for (step=0; step<strength*10; step+=17) {
        dd_draw_bless_pix(x,y,ticker+step+0,rgb2scr[IRGB(((int)(24*light)),((int)(24*light)),((int)(31*light)))],front,ptr);
        dd_draw_bless_pix(x,y,ticker+step+1,rgb2scr[IRGB(((int)(20*light)),((int)(20*light)),((int)(28*light)))],front,ptr);
        dd_draw_bless_pix(x,y,ticker+step+2,rgb2scr[IRGB(((int)(16*light)),((int)(16*light)),((int)(24*light)))],front,ptr);
        dd_draw_bless_pix(x,y,ticker+step+3,rgb2scr[IRGB(((int)(12*light)),((int)(12*light)),((int)(20*light)))],front,ptr);
        dd_draw_bless_pix(x,y,ticker+step+4,rgb2scr[IRGB(((int)(8*light)),((int)(8*light)),((int)(16*light)))],front,ptr);
    }
    dd_unlock_surface(ddbs);

    bless_time+=GetTickCount()-start;
}

void dd_draw_potion(int x,int y,int ticker,int strength,int front) {
    int step,nr;
    double light;
    unsigned short *ptr;

    if (!bless_init) {
        for (nr=0; nr<36; nr++) {
            bless_sin[nr]=sin((nr%36)/36.0*M_PI*2)*8;
            bless_cos[nr]=cos((nr%36)/36.0*M_PI*2)*16;
        }
        for (nr=0; nr<200; nr++) {
            bless_hight[nr]=-20+sin((nr%200)/200.0*M_PI*2)*20;
        }
        bless_init=1;
    }

    if ((ptr=dd_lock_surface(ddbs))==NULL) return;

    if (ticker>62) light=1.0;
    else light=(ticker)/62.0;

    for (step=0; step<strength*10; step+=17) {
        dd_draw_bless_pix(x,y,ticker+step+0,rgb2scr[IRGB(((int)(31*light)),((int)(24*light)),((int)(24*light)))],front,ptr);
        dd_draw_bless_pix(x,y,ticker+step+1,rgb2scr[IRGB(((int)(28*light)),((int)(20*light)),((int)(20*light)))],front,ptr);
        dd_draw_bless_pix(x,y,ticker+step+2,rgb2scr[IRGB(((int)(24*light)),((int)(16*light)),((int)(16*light)))],front,ptr);
        dd_draw_bless_pix(x,y,ticker+step+3,rgb2scr[IRGB(((int)(20*light)),((int)(12*light)),((int)(12*light)))],front,ptr);
        dd_draw_bless_pix(x,y,ticker+step+4,rgb2scr[IRGB(((int)(16*light)),((int)(8*light)),((int)(8*light)))],front,ptr);
    }

    dd_unlock_surface(ddbs);
}

void dd_draw_rain(int x,int y,int ticker,int strength,int front) {
    int step;
    double light;
    unsigned short *ptr;

    if ((ptr=dd_lock_surface(ddbs))==NULL) return;

    for (step=-(strength*100); step<0; step+=237) {
        dd_draw_rain_pix(x,y,-ticker+step+0,rgb2scr[IRGB(31,24,16)],front,ptr);
        dd_draw_rain_pix(x,y,-ticker+step+1,rgb2scr[IRGB(24,16,8)],front,ptr);
        dd_draw_rain_pix(x,y,-ticker+step+2,rgb2scr[IRGB(16,8,0)],front,ptr);
    }

    dd_unlock_surface(ddbs);
}

void dd_create_letter(unsigned char *rawrun,int sx,int sy,int val,char letter[16][16]) {
    int x=sx,y=sy;

    while (*rawrun!=255) {
        if (*rawrun==254) {
            y++;
            x=sx;
            rawrun++;
            continue;
        }

        x+=*rawrun++;

        letter[y][x]=val;
    }
}

char* dd_create_rawrun(char letter[16][16]) {
    char *ptr,*fon,*last;
    int x,y,step;

    last=fon=ptr=xmalloc(256,MEM_TEMP);

    for (y=3; y<16; y++) {
        step=0;
        for (x=3; x<16; x++) {
            if (letter[y][x]==2) {
                *ptr++=step; last=ptr;
                step=1;
            } else step++;
        }
        *ptr++=254;
    }
    ptr=last;
    *ptr++=255;

    fon=xrealloc(fon,ptr-fon,MEM_GLOB);
    return fon;
}

void create_shade_font(DDFONT *src,DDFONT *dst) {
    char letter[16][16];
    int x,y,c;

    for (c=0; c<128; c++) {
        bzero(letter,sizeof(letter));
        dd_create_letter(src[c].raw,4,5,2,letter);
        dd_create_letter(src[c].raw,5,4,2,letter);
        dd_create_letter(src[c].raw,4,4,1,letter);
        dst[c].raw=dd_create_rawrun(letter);
        dst[c].dim=src[c].dim;
    }
}

void create_frame_font(DDFONT *src,DDFONT *dst) {
    char letter[16][16];
    int x,y,c;

    for (c=0; c<128; c++) {
        bzero(letter,sizeof(letter));
        dd_create_letter(src[c].raw,5,4,2,letter);
        dd_create_letter(src[c].raw,3,4,2,letter);
        dd_create_letter(src[c].raw,4,5,2,letter);
        dd_create_letter(src[c].raw,4,3,2,letter);
        dd_create_letter(src[c].raw,5,5,2,letter);
        dd_create_letter(src[c].raw,5,3,2,letter);
        dd_create_letter(src[c].raw,3,5,2,letter);
        dd_create_letter(src[c].raw,3,3,2,letter);
        dd_create_letter(src[c].raw,4,4,1,letter);
        dst[c].raw=dd_create_rawrun(letter);
        dst[c].dim=src[c].dim;
    }
}

void dd_create_font(void) {
    if (fonta_shaded) return;

    fonta_shaded=xmalloc(sizeof(DDFONT)*128,MEM_GLOB); create_shade_font(fonta,fonta_shaded);
    fontb_shaded=xmalloc(sizeof(DDFONT)*128,MEM_GLOB); create_shade_font(fontb,fontb_shaded);
    fontc_shaded=xmalloc(sizeof(DDFONT)*128,MEM_GLOB); create_shade_font(fontc,fontc_shaded);

    fonta_framed=xmalloc(sizeof(DDFONT)*128,MEM_GLOB); create_frame_font(fonta,fonta_framed);
    fontb_framed=xmalloc(sizeof(DDFONT)*128,MEM_GLOB); create_frame_font(fontb,fontb_framed);
    fontc_framed=xmalloc(sizeof(DDFONT)*128,MEM_GLOB); create_frame_font(fontc,fontc_framed);

}

DDFONT *textfont=fonta;
int textdisplay_dy=10;

int dd_drawtext_char(int sx,int sy,int c,unsigned short int color) {
    unsigned short *ptr,*dst;
    unsigned char *rawrun;
    int x,y,start;

    if (c>127 || c<0) return 0;

    if (newtext) {
        char text[2]={0,0};
        text[0]=c;
        texter_add(sx,sy,color,0,text);
        return fontdim[c];
    }

    if ((vidptr=ptr=dd_lock_surface(ddbs))==NULL) return 0;

    rawrun=textfont[c].raw;

    x=sx;
    y=sy;

    dst=ptr+(x+x_offset)+(y+y_offset)*xres;

    while (*rawrun!=255) {
        if (*rawrun==254) {
            y++;
            x=sx;
            rawrun++;
            dst=ptr+(x+x_offset)+(y+y_offset)*xres;
            continue;
        }

        dst+=*rawrun;
        rawrun++;

        if (dst-vidptr>xres*yres || dst<vidptr) {
            note("PANIC #5");
            dst=vidptr;
        }
        *dst=color;
        tp_cnt++;
    }

    dd_unlock_surface(ddbs);

    return textfont[c].dim;
}

int dd_text_len(const char *text) {
    float x;
    const char *c;

    for (x=0,c=text; *c; c++) if (newtext) x+=fontdim[*c]; else x+=textfont[*c].dim;

    return (int)(x+0.5f);
}

int dd_char_len(char c) {
    if (newtext) return fontdim[c]; else return textfont[c].dim;
}


#define MAXTEXTLINES		256
#define MAXTEXTLETTERS		256

#define TEXTDISPLAY_X		230
#define TEXTDISPLAY_Y		438

#define TEXTDISPLAY_DY		(textdisplay_dy)

#define TEXTDISPLAY_SX		396
#define TEXTDISPLAY_SY		150

#define TEXTDISPLAYLINES	(TEXTDISPLAY_SY/TEXTDISPLAY_DY)

int textnextline=0,textdisplayline=0;

struct letter {
    char c;
    unsigned char color;
    unsigned char link;
};

struct letter *text=NULL;

unsigned short palette[256];

void dd_init_text(void) {
    text=xcalloc(sizeof(struct letter)*MAXTEXTLINES*MAXTEXTLETTERS,MEM_GLOB);
    palette[0]=rgb2scr[IRGB(31,31,31)];    // normal white text (talk, game messages)
    palette[1]=rgb2scr[IRGB(16,16,16)];    // dark gray text (now entering ...)
    palette[2]=rgb2scr[IRGB(16,31,16)];    // light green (normal chat)
    palette[3]=rgb2scr[IRGB(31,16,16)];    // light red (announce)
    palette[4]=rgb2scr[IRGB(16,16,31)];    // light blue (text links)
    palette[5]=rgb2scr[IRGB(24,24,31)];    // orange (item desc headings)
    palette[6]=rgb2scr[IRGB(31,31,16)];    // yellow (tells)
    palette[7]=rgb2scr[IRGB(16,24,31)];    // violet (staff chat)
    palette[8]=rgb2scr[IRGB(24,24,31)];    // light violet (god chat)

    palette[9]=rgb2scr[IRGB(24,24,16)];    // chat - auction
    palette[10]=rgb2scr[IRGB(24,16,24)];    // chat - grats
    palette[11]=rgb2scr[IRGB(16,24,24)];    // chat	- mirror
    palette[12]=rgb2scr[IRGB(31,24,16)];    // chat - info
    palette[13]=rgb2scr[IRGB(31,16,24)];    // chat - area
    palette[14]=rgb2scr[IRGB(16,31,24)];    // chat - v2, games
    palette[15]=rgb2scr[IRGB(24,31,16)];    // chat - public clan
    palette[16]=rgb2scr[IRGB(24,16,31)];    // chat	- internal clan

    palette[17]=rgb2scr[IRGB(31,31,31)];    // fake white text (hidden links)
}

void dd_set_textfont(int nr) {
    extern int namesize;
    int n;

    switch (nr) {
        case 0:	textfont=fonta; textdisplay_dy=10; fontdim=fontdim_a; break;
        case 1:	textfont=fontc; textdisplay_dy=12; fontdim=fontdim_c; break;
    }
    bzero(text,MAXTEXTLINES*MAXTEXTLETTERS*sizeof(struct letter));
    textnextline=textdisplayline=0;
}

void dd_display_text(void) {
    int n,m,rn,x,y,pos;
    char buf[256],*bp;
    unsigned short lastcolor;

    for (n=textdisplayline,y=TEXTDISPLAY_Y; y<=TEXTDISPLAY_Y+TEXTDISPLAY_SY-TEXTDISPLAY_DY; n++,y+=TEXTDISPLAY_DY) {
        rn=n%MAXTEXTLINES;

        x=TEXTDISPLAY_X;
        pos=rn*MAXTEXTLETTERS;

        bp=buf;
        for (m=0; m<MAXTEXTLETTERS; m++,pos++) {
            if (text[pos].c==0) break;

            if (text[pos].c>0 && text[pos].c<32) continue;

            if (bp==buf) {
                lastcolor=text[pos].color;
            }
            if (lastcolor!=text[pos].color) {
                *bp=0;
                x=dd_drawtext(x,y,palette[lastcolor],0,buf);
                bp=buf; lastcolor=text[pos].color;
            }
            *bp++=text[pos].c;
        }
        if (bp!=buf) {
            *bp=0;
            dd_drawtext(x,y,palette[lastcolor],0,buf);
        }
    }
}

void dd_add_text(char *ptr) {
    int n,m,pos,color=0,link=0;
    float x=0.0f,tmp;
    char buf[256];

    pos=textnextline*MAXTEXTLETTERS;
    bzero(text+pos,sizeof(struct letter)*MAXTEXTLETTERS);

    while (*ptr) {


        while (*ptr==' ') ptr++;
        while (*ptr=='°') {
            ptr++;
            switch (*ptr) {
                case 'c':	tmp=atoi(ptr+1);
                    if (tmp==18) link=0;
                    else if (tmp!=17) { color=tmp; link=0; }
                    if (tmp==4 || tmp==17) link=1;
                    ptr++;
                    while (isdigit(*ptr)) ptr++;
                    break;
                default:	ptr++; break;
            }
        }
        while (*ptr==' ') ptr++;

        n=0;
        while (*ptr && *ptr!=' ' && *ptr!='°' && n<49) buf[n++]=*ptr++;
        buf[n]=0;

        if (x+(tmp=dd_text_len(buf))>=TEXTDISPLAY_SX) {
            if (textdisplayline==(textnextline+(MAXTEXTLINES-TEXTDISPLAYLINES))%MAXTEXTLINES) textdisplayline=(textdisplayline+1)%MAXTEXTLINES;
            textnextline=(textnextline+1)%MAXTEXTLINES;
            pos=textnextline*MAXTEXTLETTERS;
            bzero(text+pos,sizeof(struct letter)*MAXTEXTLETTERS);
            x=tmp;

#define INDENT_TEXT
#ifdef INDENT_TEXT
            for (m=0; m<2; m++) {
                text[pos].c=32;
                if (newtext) x+=fontdim[32];
                else x+=textfont[32].dim;
                text[pos].color=color;
                text[pos].link=link;
                pos++;
            }
#endif

        } else x+=tmp;

        //printf("adding [%s] at pos %d (nextline=%d)\n",buf,pos,textnextline);

        for (m=0; m<n; m++,pos++) {
            text[pos].c=buf[m];
            text[pos].color=color;
            text[pos].link=link;
        }
        text[pos].c=32;
        if (newtext) x+=fontdim[32];
        else x+=textfont[32].dim;
        text[pos].color=color;
        text[pos].link=link;

        pos++;
    }
    pos=(pos+MAXTEXTLETTERS*MAXTEXTLINES-1)%(MAXTEXTLETTERS*MAXTEXTLINES);
    if (text[pos].c==32) {
        text[pos].c=0;
        text[pos].color=0;
        text[pos].link=0;
        //printf("erase space at %d\n",pos);
    }

    if (textdisplayline==(textnextline+(MAXTEXTLINES-TEXTDISPLAYLINES))%MAXTEXTLINES) textdisplayline=(textdisplayline+1)%MAXTEXTLINES;

    textnextline=(textnextline+1)%MAXTEXTLINES;
    if (textnextline==textdisplayline) textdisplayline=(textdisplayline+1)%MAXTEXTLINES;
    //textdisplayline=(textnextline+(MAXTEXTLINES-TEXTDISPLAYLINES))%MAXTEXTLINES;
}

int dd_text_init_done(void) {
    return text!=NULL;
}

int dd_scantext(int x,int y,char *hit) {
    int n,m,pos,panic=0,tmp=0;
    float dx;

    if (x<TEXTDISPLAY_X || y<TEXTDISPLAY_Y) return 0;
    if (x>TEXTDISPLAY_X+TEXTDISPLAY_SX) return 0;
    if (y>TEXTDISPLAY_Y+TEXTDISPLAY_SY) return 0;

    n=(y-TEXTDISPLAY_Y)/TEXTDISPLAY_DY;
    n=(n+textdisplayline)%MAXTEXTLINES;

    for (pos=n*MAXTEXTLETTERS,dx=m=0; m<MAXTEXTLETTERS && text[pos].c; m++,pos++) {
        if (text[pos].c>0 && text[pos].c<32) { dx=((int)text[pos].c)*12+TEXTDISPLAY_X; continue; }

        if (newtext) dx+=fontdim[text[pos].c];
        else dx+=textfont[text[pos].c].dim;

        if (dx+TEXTDISPLAY_X>x) {
            if (text[pos].link) {   // link palette color
                while ((text[pos].link || text[pos].c==0) && panic++<5000) {
                    pos--;
                    if (pos<0) pos=MAXTEXTLETTERS*MAXTEXTLINES-1;
                }

                pos++;
                if (pos==MAXTEXTLETTERS*MAXTEXTLINES) pos=0;
                while ((text[pos].link || text[pos].c==0) && panic++<5000 && tmp<80) {
                    if (tmp>0 && text[pos].c==' ' && hit[tmp-1]==' ');
                    else if (text[pos].c) hit[tmp++]=text[pos].c;
                    pos++;
                }
                if (tmp>0 && hit[tmp-1]==' ') hit[tmp-1]=0;
                else hit[tmp]=0;
                return 1;
            }
            return 0;
        }
    }
    return 0;
}

void dd_text_lineup(void) {
    int tmp;

    tmp=(textdisplayline+MAXTEXTLINES-1)%MAXTEXTLINES;
    if (tmp!=textnextline) textdisplayline=tmp;
}

void dd_text_pageup(void) {
    int n,tmp;

    for (n=0; n<TEXTDISPLAYLINES; n++) {
        tmp=(textdisplayline+MAXTEXTLINES-1)%MAXTEXTLINES;
        if (tmp==textnextline) break;
        textdisplayline=tmp;
    }
}

void dd_text_linedown(void) {
    int tmp;

    tmp=(textdisplayline+1)%MAXTEXTLINES;
    if (tmp!=(textnextline+MAXTEXTLINES-TEXTDISPLAYLINES+1)%MAXTEXTLINES) textdisplayline=tmp;
}

void dd_text_pagedown(void) {
    int n,tmp;

    for (n=0; n<TEXTDISPLAYLINES; n++) {
        tmp=(textdisplayline+1)%MAXTEXTLINES;
        if (tmp==(textnextline+MAXTEXTLINES-TEXTDISPLAYLINES+1)%MAXTEXTLINES) break;
        textdisplayline=tmp;
    }
}

// blit a systemcache entry to backsurface without touching the video cache
void sc_blit3(SYSTEMCACHE *sc,int sx,int sy) {
    int x,y,addx=0,addy=0,sstep,dstep,dx,dy;
    unsigned short *ptr,col,*dst,*src;

    dx=sc->xres; dy=sc->yres;

    if (sx<clipsx) { addx=clipsx-sx; dx-=addx; sx=clipsx; }
    if (sy<clipsy) { addy=clipsy-sy; dy-=addy; sy=clipsy; }
    if (sx+dx>=clipex) dx=clipex-sx;
    if (sy+dy>=clipey) dy=clipey-sy;

    if (dy<=0 || dx<=0) return;

    if ((ptr=dd_lock_surface(ddbs))==NULL) return;

    src=sc->rgb+addx+addy*sc->xres;
    sstep=sc->xres-dx;
    dst=ptr+(sx+x_offset)+(sy+y_offset)*xres;
    dstep=xres-dx;

    for (y=0; y<dy; y++) {
        for (x=0; x<dx; x++) {
            col=*src++;

            if (col==scrcolorkey) { dst++; continue; }

            *dst++=col;
        }
        src+=sstep;
        dst+=dstep;
    }

    dd_unlock_surface(ddbs);
}

// blit a systemcache entry to backsurface without touching the video cache
void sc_blit4(SYSTEMCACHE *sc,int scrx,int scry,int sx,int sy,int dx,int dy) {
    int x,y,addx=0,addy=0,sstep,dstep;
    unsigned short *ptr,col,*dst,*src;

    if (sx+scrx<clipsx) { addx=clipsx-(sx+scrx); dx-=addx; sx+=addx; }
    if (sy+scry<clipsy) { addy=clipsy-(sy+scry); dy-=addy; sy+=addy; }
    if (sx+scrx+dx>=clipex) dx=clipex-(sx+scrx);
    if (sy+scry+dy>=clipey) dy=clipey-(sy+scry);

    if (dy<=0 || dx<=0) return;

    if ((ptr=dd_lock_surface(ddbs))==NULL) return;

    src=sc->rgb+addx+sx+(addy+sy)*sc->xres;
    sstep=sc->xres-dx;
    dst=ptr+(sx+x_offset+scrx)+(sy+y_offset+scry)*xres;
    dstep=xres-dx;

    for (y=0; y<dy; y++) {
        for (x=0; x<dx; x++) {

            col=*src++;

            if (col==scrcolorkey) { dst++; continue; }

            *dst++=col;
        }
        src+=sstep;
        dst+=dstep;
    }

    dd_unlock_surface(ddbs);
}

// blit a systemcache entry to the screen
static int sc_blit2(DDFX *ddfx,int sidx,int scrx,int scry) {
    SYSTEMCACHE *sc;
    int v,tx,ty,use,size;
    struct vid_cache *vcl,*vc;

    PARANOIA(if (sidx==SIDX_NONE) paranoia("sc_blit: sidx==SIDX_NONE"); )
    PARANOIA(if (sidx>=max_systemcache) paranoia("sc_blit: sidx>=max_systemcache (%d>=%d)",sidx,max_systemcache); )

    // easy use of sc
    sc=&systemcache[sidx];

    sc_blit3(sc,scrx,scry);
    sc_blits++;

    // draw the alpha pixels
    if (sc->acnt) sc_blit_apix(sidx,scrx,scry,ddfx->grid,ddfx->freeze);

    sc->tick=dd_tick;
    sc->used++;

    sc_cnt++;

    return 0;
};


