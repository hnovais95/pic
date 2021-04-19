#define RS RE2
#define EN RE1
#define D4 RD4
#define D5 RD5
#define D6 RD6
#define D7 RD7

typedef struct
{
    char line0 [16];
    char line1 [16];
} lcd;

void Lcd_SetBit(char data_bit);
void Lcd_Cmd(char a);
void Lcd_Clear();
void Lcd_Set_Cursor(char a, char b);
void Lcd_Start();
void Lcd_Print_Char(char data);
void Lcd_Print_String(char *a);
void Show_Display(lcd lcd);
