/*
 * Part of Astonia Client (c) Daniel Brockhaus. Please read license.txt.
 */

#define SV_SCROLL_UP		1
#define SV_SCROLL_DOWN		2
#define SV_SCROLL_LEFT		3
#define SV_SCROLL_RIGHT		4
#define SV_SCROLL_LEFTUP	5
#define SV_SCROLL_RIGHTUP	6
#define SV_SCROLL_LEFTDOWN	7
#define SV_SCROLL_RIGHTDOWN	8
#define SV_TEXT			9
#define SV_SETVAL0		10
#define SV_SETVAL1		11
#define SV_SETHP		12
#define SV_SETMANA		13
#define SV_SETITEM		14
#define SV_SETORIGIN		15
#define SV_SETTICK		16
#define SV_SETCITEM		17
#define SV_ACT			18
#define SV_EXIT			19
#define SV_NAME			20
#define SV_SERVER		21
#define SV_CONTAINER		22
#define SV_CONCNT		23
#define SV_ENDURANCE		24
#define SV_LIFESHIELD		25
#define SV_EXP			26
#define SV_EXP_USED		27
#define SV_PRICE		28
#define SV_CPRICE		29
#define SV_GOLD			30
#define SV_LOOKINV		31
#define SV_ITEMPRICE		32
#define SV_CYCLES		33
#define SV_CEFFECT		34
#define SV_UEFFECT		35
#define SV_REALTIME		36
#define SV_SPEEDMODE		37
#define SV_FIGHTMODE		38      // unused in vanilla server
#define SV_CONTYPE		39
#define SV_CONNAME		40
#define SV_LS 		  	41
#define SV_CAT			42
#define SV_LOGINDONE		43
#define SV_SPECIAL		44
#define SV_TELEPORT		45
#define SV_SETRAGE		46
#define SV_MIRROR		47
#define SV_PROF			48
#define SV_PING			49
#define SV_UNIQUE		50
#define SV_MIL_EXP		51
#define SV_QUESTLOG		52
#define SV_PROTOCOL		53
#define SV_RESERVED1    54
#define SV_RESERVED2    55
#define SV_RESERVED3    56
#define SV_RESERVED4    57
#define SV_MOD1         58
#define SV_MOD2         59
#define SV_MOD3         60
#define SV_MOD4         61
#define SV_MOD5         62
#define SV_RESERVED5    63
// 58...62 can be used by amod.dll

#define SV_MAPTHIS		0
#define SV_MAPNEXT		16
#define SV_MAPOFF		32
#define SV_MAPPOS		(16+32)

#define SV_MAP01		64
#define SV_MAP10		128
#define SV_MAP11		(64+128)

#define MAX_INBUF	        0xFFFFF
#define MAX_OUTBUF	        0xFFFFF

#define Q_SIZE	16

struct queue {
    unsigned char buf[16384];
    int size;
};

int record_client(char *filename);
int open_client(char *username,char *password);
int init_network(void);
void exit_network(void);
void bzero_client(int part);

