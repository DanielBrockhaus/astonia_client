/*
 * Part of Astonia Client (c) Daniel Brockhaus. Please read license.txt.
 */

struct quicks {
    int mn[9];      // 0 for invalid neighbours
    int qi[9];      // maxqick for invalid neighbours
    int mapx;
    int mapy;
    int cx;
    int cy;
};

typedef struct quicks QUICK;

extern int nocut;
extern unsigned int now;
extern int playersprite_override;
extern int mapaddx,mapaddy;
extern int mapoffx,mapoffy;
extern int mapaddx,mapaddy;   // small offset to smoothen walking

extern int itmsel;                     // mn
extern int chrsel;                     // mn
extern int mapsel;                     // mn

extern unsigned short int healthcolor,manacolor,endurancecolor,shieldcolor;
extern unsigned short int whitecolor,lightgraycolor,graycolor,darkgraycolor,blackcolor;
extern unsigned short int lightredcolor,redcolor,darkredcolor;
extern unsigned short int lightgreencolor,greencolor,darkgreencolor;
extern unsigned short int lightbluecolor,bluecolor,darkbluecolor;
extern unsigned short int lightorangecolor,orangecolor,darkorangecolor;
extern unsigned short int textcolor;
extern int update_skltab;

extern int teleporter;
extern int show_tutor;
extern char tutor_text[1024];
extern int show_look;
extern int vendor;

void mtos(int mapx,int mapy,int *scrx,int *scry);
void stom(int scrx,int scry,int *mapx,int *mapy);
void set_mapoff(int cx,int cy,int mdx,int mdy);
void set_mapadd(int addx,int addy);
void update_user_keys(void);

int main_init(void);
int main_loop(void);
void main_exit(void);

