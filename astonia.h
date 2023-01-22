
void display_game(void);

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
#define MAX_DOT         25

int dotx(int didx);
int doty(int didx);

int do_display_questlog(int nr);