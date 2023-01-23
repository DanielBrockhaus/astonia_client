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

extern unsigned int now;
extern int playersprite_override;
extern int mapaddx,mapaddy;

extern unsigned short int healthcolor,manacolor,endurancecolor,shieldcolor;
extern unsigned short int whitecolor,lightgraycolor,graycolor,darkgraycolor,blackcolor;
extern unsigned short int lightredcolor,redcolor,darkredcolor;
extern unsigned short int lightgreencolor,greencolor,darkgreencolor;
extern unsigned short int lightbluecolor,bluecolor,darkbluecolor;
extern unsigned short int lightorangecolor,orangecolor,darkorangecolor;
extern unsigned short int textcolor;


void mtos(int mapx,int mapy,int *scrx,int *scry);
void stom(int scrx,int scry,int *mapx,int *mapy);
void set_mapoff(int cx,int cy,int mdx,int mdy);
void set_mapadd(int addx,int addy);
void update_user_keys(void);


