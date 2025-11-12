/*
 * Part of Astonia Client (c) Daniel Brockhaus. Please read license.txt.
 *
 * Command (chat) line processing
 *
 * Processes key strokes and executes commands.
 */

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "../../src/astonia.h"
#include "../../src/gui.h"
#include "../../src/gui/_gui.h"
#include "../../src/sdl.h"
#include "../../src/client.h"
#include "../../src/game.h"
#include "../../src/modder.h"

#define MAXCMDLINE	199
#define MAXHIST		20

static char cmdline[MAXCMDLINE+1]={""};
static char *history[MAXHIST];
static int cmdcursor=0,cmddisplay=0,histpos=-1;

void update_user_keys(void) {
    int n;

    for (n=0; n<10; n++) {
        keytab[n].userdef=user_keys[n];
        keytab[n+10].userdef=user_keys[n];
        keytab[n+20].userdef=user_keys[n];
    }
}

char* strcasestr(const char *haystack,const char *needle) {
    const char *ptr;

    for (ptr=needle; *haystack; haystack++) {
        if (toupper(*ptr)==toupper(*haystack)) {
            ptr++;
            if (!*ptr) return (char *)(haystack+(needle-ptr+1));
        } else ptr=needle;
    }
    return NULL;
}
void cmd_version(void) {
    int i;
    char *ptr;

    addline("Client version: %s",client_version());

    for (i=0; i<MAXMOD; i++) {
        ptr=amod_version(i);
        if (ptr) addline("%c-mod version: %s",'A'+i,ptr);
    }
}

int client_cmd(char *buf) {

    if (!strncmp(buf,"#ps ",3)) {
        playersprite_override=atoi(&buf[3]);
        return 1;
    }

    if (!strncmp(buf,"#crash",6) || !strncmp(buf,"/crash",6)) {
        *(volatile int*)0=42;
        return 1;
    }

    if (!strncmp(buf,"#clearmap",7) || !strncmp(buf,"/clearmap",7)) {
        minimap_clear();
        return 1;
    }
    if (!strncmp(buf,"#compactmap",7) || !strncmp(buf,"/compactmap",7)) {
        minimap_compact();
        return 1;
    }

    if (!strncmp(buf,"#col1",5) || !strncmp(buf,"#col2",5) || !strncmp(buf,"#col3",5) ||
        !strncmp(buf,"/col1",5) || !strncmp(buf,"/col2",5) || !strncmp(buf,"/col3",5)) {
        show_color=1;
        show_cur=0;
        show_color_c[0]=map[MAPDX*MAPDY/2].rc.c1;
        show_color_c[1]=map[MAPDX*MAPDY/2].rc.c2;
        show_color_c[2]=map[MAPDX*MAPDY/2].rc.c3;
        return 1;
    }
    if (!strncmp(buf, "#sound ", 7)) {
    	play_sound(atoi(&buf[7]),0,0);
    	return 1;
    }
    if (!strncmp(buf, "#volume ", 8) || !strncmp(buf, "/volume ", 8)) {
    	int new_sound_volume = atoi(&buf[8]);
    	if (new_sound_volume < 0) new_sound_volume = 0;
    	if (new_sound_volume >= 128) new_sound_volume = 128;
    	sound_volume = new_sound_volume;
    	addline("Volume is now at %d", sound_volume);
    	return 1;
    }
    if (!strncmp(buf, "#version", 5) || !strncmp(buf, "/version", 5)) {
        cmd_version();
    	return 1;
    }
    if (!strncmp(buf,"#set ",5) || !strncmp(buf,"/set ",5)) {
        int what,key;
        char *ptr;

        ptr=buf+5;

        while (isspace(*ptr)) ptr++;
        what=atoi(ptr);
        if (what==0) what=9;
        else what--;

        while (isdigit(*ptr)) ptr++;
        while (isspace(*ptr)) ptr++;
        key=toupper(*ptr);

        if (what<0 || what>9) {
            addline("Spell is out of bounds (must be between 0 and 9)");
            return 1;
        }
        if (key<'A' || key>'Z') {
            addline("Key is out of bounds (must be between A and Z)");
            return 1;
        }
        user_keys[what]=key;
        update_user_keys();
        save_options();

        addline("Set key %c for spell %d.",key,what==9?0:what+1);

        return 1;
    }
    if (strcasestr(buf,password)) {
        addline("°c3Sorry, but you are not allowed to say your password. No matter what you're promised, do not give your password to anyone! The only things which happened to players who did are: Loss of all items, lots of negative experience, bad karma and locked characters. If you really, really think you have to tell your password to someone, then I'm sure you'll find a way around this block.");
        return 1;
    }

    return amod_client_cmd(buf);
}

char rem_buf[10][256]={""};
int rem_in=0,rem_out=0;
void cmd_remember(char *ptr) {
    char *start=ptr,*dst;
    char tmp[256];

    if (*ptr!='#' && *ptr!='/') return;
    ptr++;
    if (*ptr!='t' && *ptr!='T') return;
    ptr++;
    if (*ptr==' ') goto do_remember;
    if (*ptr!='e' && *ptr!='E') return;
    ptr++;
    if (*ptr==' ') goto do_remember;
    if (*ptr!='l' && *ptr!='L') return;
    ptr++;
    if (*ptr==' ') goto do_remember;
    if (*ptr!='l' && *ptr!='L') return;
    ptr++;
    if (*ptr==' ') goto do_remember;
    return;

do_remember:
    while (isspace(*ptr)) ptr++;
    while (*ptr && !isspace(*ptr)) ptr++;

    for (dst=tmp; start<=ptr; *dst++=*start++);
    *dst=0;

    if (strcmp(tmp,rem_buf[rem_in])) {
        rem_in=(rem_in+1)%10;
        strcpy(rem_buf[rem_in],tmp);
    }
    rem_out=rem_in;
}

void cmd_fetch(char *ptr) {
    if (rem_out!=(rem_in+1)%10) strcpy(ptr,rem_buf[rem_out]);
    rem_out=(rem_out+9)%10;
}

void cmd_proc(int key) {
    if (context_key(key)) return;

    switch (key) {
        case CMD_BACK:	if (cmdcursor<1) break;
            memmove(cmdline+cmdcursor-1,cmdline+cmdcursor,MAXCMDLINE-cmdcursor);
            cmdline[MAXCMDLINE-1]=0;
            cmdcursor--;
            break;
        case CMD_DELETE:
            memmove(cmdline+cmdcursor,cmdline+cmdcursor+1,MAXCMDLINE-cmdcursor-1);
            cmdline[MAXCMDLINE-1]=0;
            break;

        case CMD_LEFT:	if (cmdcursor>0) cmdcursor--;
            break;

        case CMD_RIGHT:	if (cmdcursor<MAXCMDLINE-1) {
                if (cmdline[cmdcursor]==0) cmdline[cmdcursor]=' ';
                cmdcursor++;
            }
            break;

        case CMD_HOME:	cmdcursor=0;
            break;

        case CMD_END:	for (cmdcursor=MAXCMDLINE-2; cmdcursor>=0; cmdcursor--) if (cmdline[cmdcursor]) break; cmdcursor++;
            break;

        case CMD_UP:
            if (histpos<MAXHIST-1 && history[histpos+1]) histpos++;
            else break;
            bzero(cmdline,sizeof(cmdline));
            strcpy(cmdline,history[histpos]);
            for (cmdcursor=MAXCMDLINE-2; cmdcursor>=0; cmdcursor--) if (cmdline[cmdcursor]) break;
            cmdcursor++;
            break;

        case CMD_DOWN:	if (histpos>0) histpos--;
            else { bzero(cmdline,sizeof(cmdline)); cmdcursor=0; histpos=-1; break; }
            bzero(cmdline,sizeof(cmdline));
            strcpy(cmdline,history[histpos]);
            for (cmdcursor=MAXCMDLINE-2; cmdcursor>=0; cmdcursor--) if (cmdline[cmdcursor]) break;
            cmdcursor++;
            break;

        case CMD_RETURN:
        case 13:
            if (!client_cmd(cmdline) && cmdline[0]) cmd_text(cmdline);
            cmd_remember(cmdline);
            if (history[MAXHIST-1]) xfree(history[MAXHIST-1]);
            memmove(history+1,history,sizeof(history)-sizeof(history[0]));
            history[0]=xstrdup(cmdline,MEM_TEMP);
            cmdcursor=cmddisplay=0; histpos=-1;
            bzero(cmdline,sizeof(cmdline));
            break;

        case 9:		bzero(cmdline,sizeof(cmdline));
            cmd_fetch(cmdline);
            for (cmdcursor=MAXCMDLINE-2; cmdcursor>=0; cmdcursor--) if (cmdline[cmdcursor]) break;
            cmdcursor++;
            break;

        default:	if (key<32 || key>127) { /* addline("%d",key); */ break; }
            if (cmdcursor<MAXCMDLINE-1) {
                memmove(cmdline+cmdcursor+1,cmdline+cmdcursor,MAXCMDLINE-cmdcursor-1);
                cmdline[cmdcursor++]=key;
            }
            break;
    }
}

void cmd_add_text(char *buf,int typ) {

    context_key_set(1);

    while (*buf) cmd_proc(*buf++);
}

void display_cmd(void) {
    int n,x,tmp;

    if (!context_key_isset()) return;

    if (cmdcursor<cmddisplay) cmddisplay=0;

    for (x=0,n=cmdcursor; n>cmddisplay; n--) {
        x+=dd_char_len(cmdline[n]);
        if (x>dotx(DOT_TX2)-dotx(DOT_TXT)-16) {
            cmddisplay=n;
            break;
        }
    }
    if (context_key_enabled()) {
        dd_shaded_rect(dotx(DOT_TXT)-1,doty(DOT_TX2)-2,dotx(DOT_TX2)+1,doty(DOT_TX2)+8,IRGB(15,15,15),95);
    }

    for (x=0,n=cmddisplay; n<MAXCMDLINE; n++) {
        if (cmdline[n]) tmp=dd_drawtext_char(dotx(DOT_TXT)+x,doty(DOT_TX2)-1,cmdline[n],IRGB(31,31,31));
        else tmp=0;
        if (n==cmdcursor) {
            if (cmdline[n]) dd_shaded_rect(dotx(DOT_TXT)+x-1,doty(DOT_TX2)-2,dotx(DOT_TXT)+x+tmp+1,doty(DOT_TX2)+8,0xffe0,95);
            else            dd_shaded_rect(dotx(DOT_TXT)+x  ,doty(DOT_TX2)-2,dotx(DOT_TXT)+x+4    ,doty(DOT_TX2)+8,0xffe0,95);
        }
        x+=tmp;
        if (x>dotx(DOT_TX2)-dotx(DOT_TXT)-8) break;
    }
}

void cmd_reset(void) {
    bzero(cmdline,sizeof(cmdline)); cmdcursor=0; histpos=-1;
}

