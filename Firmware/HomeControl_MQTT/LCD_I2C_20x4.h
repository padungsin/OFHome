//////////////////////////////////////////////////////////////////////////////
// This is the simple CCS program for I2C (PCF8574T) lcd module 
// Auther: Pumrin S.
// Pin map: 
// PCF8574T > 20x4 LCD
//   P0     >   RS
//   P1     >   R/W
//   P2     >   E
//   P3     >   NC
//   P4     >   D4
//   P5     >   D5
//   P6     >   D6
//   P7     >   D7
// Note: The SCL and SDA pins should be pull-up resister allway. 
//////////////////////////////////////////////////////////////////////////////

#use I2C (master, sda = PIN_B4, scl = PIN_D3)

#define LCDADDR        0x70 // default slave address
#define ON             0x08 
#define OFF            0x00

#define LCD_SET_BIT(x)    fetch_data(x)

byte LCD_ADDR=0x4e; //I2C slave address for Funduino LCD module 

//*** START LCD Command ***
#define  LCD_CLEAR_SCREEN   0B00000001
#define  LCD_RETURN_HOME    0B00000010

//Entry mode set (BIT2=1)
#define  LCD_ENTRY_MODE     0B00000100
#define  INCREMENTS_LCD     0B00000010
#define  DECREMENTS_LCD     0B00000000
#define  NO_SCROLL_LCD      0B00000000
#define  SCROLL_LCD         0B00000001

//Display ON/OFF (BIT3=1)
#define  DISPLAY_LCD        0B00001000
#define  LCD_ON             0B00000100
#define  LCD_OFF            0B00000000
#define  LCD_CURSOR_ON      0B00000010
#define  LCD_CURSOR_OFF     0B00000000
#define  LCD_BLINK_ON       0B00000001
#define  LCD_BLINK_OFF      0B00000000

//Scroll Display/Shift Cursor (BIT4=1)
#define  LCD_SCROLL_SHIFT   0B00010000
#define  LCD_SHIFT_CURSOR   0B00000000
#define  LCD_SHIFT_DISPLAY  0B00001000
#define  LCD_SHIFT_LEFT     0B00000000
#define  LCD_SHIFT_RIGHT    0B00000100

//Function set (BIT5=1)
#define  LCD_FUNCTION_SET   0B00100000
#define  LCD4BIT_MODE       0B00000000
#define  LCD8BIT_MODE       0B00010000
#define  LCD_LINE1          0B00000000
#define  LCD_LINE2          0B00001000
#define  LCD_FONT_5X7       0B00000000
#define  LCD_FONT_5X10      0B00000100

//Move To CGRAM Address (BIT6=1)
#define  LCD_CGRAM_ADDRESS  0B01000000

//Move To DDRAM Address (BIT7=1)
#define  LCD_DDRAM_ADDRESS  0B10000000

//*** END LCD Command ***

// Prepare x,y address byte
//line1=80h, line2=C0h line3=94h, line4=D4h
//   1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20
//1 80 81 82 83 84 85 86 87 88 89 8A 8B 8C 8D 8E 8F 90 91 92 93
//2 C0 C1 C2 C3 C4 C5 C6 C7 C8 C9 CA CB CC CD CE CF D0 D1 D2 D3
//3 94 95 96 97 98 99 9A 9B 9C 9D 9E 9F A0 A1 A2 A3 A4 A5 A6 A7
//4 D4 D5 D6 D7 D8 D9 DA DB DC DE DF E0 E1 E2 E3 E4 E5 E6 E7 E8

byte line_1[20]={0x80,0x81,0x82,0x83,0x84,
                 0x85,0x86,0x87,0x88,0x89,
                 0x8A,0x8B,0x8C,0x8D,0x8E, 
                 0x8F,0x90,0x91,0x92,0x93};
byte line_2[20]={0xC0,0xC1,0xC2,0xC3,0xC4,
                 0xC5,0xC6,0xC7,0xC8,0xC9,
                 0xCA,0xCB,0xCC,0xCD,0xCE,
                 0xCF,0xD0,0xD1,0xD2,0xD3};
byte line_3[20]={0x94,0x95,0x96,0x97,0x98,
                 0x99,0x9A,0x9B,0x9C,0x9D,
                 0x9E,0x9F,0xA0,0xA1,0xA2,
                 0xA3,0xA4,0xA5,0xA6,0xA7};  
byte line_4[20]={0xD4,0xD5,0xD6,0xD7,0xD8,
                 0xD9,0xDA,0xDB,0xDC,0xDE,
                 0xDF,0xE0,0xE1,0xE2,0xE3,
                 0xE4,0xE5,0xE6,0xE7,0xE8};

//Transmittion data
void transceiver(unsigned char data)
   {
        
        i2c_start();
        i2c_write(LCD_ADDR); //the slave addresse
        i2c_write(data);    
        i2c_stop();
   }

//Clocking the LCD's enable pin during transmit data
void fetch_data(unsigned char data)
   {
        data=data|0b00000100;//set pin E is a 1
        transceiver(data);
        delay_ms(1);
        data=data-4;//toggle E back to 0
        transceiver(data);
        delay_ms(1);

   }

void lcd_init()
{
   //Request works on the command by set the RS = 0 R/W = 0 write
        LCD_SET_BIT(0x00);
        LCD_SET_BIT(0x10);
        LCD_SET_BIT(0x00);
        LCD_SET_BIT(0x00);
        LCD_SET_BIT(0x10);
           //First state in 8 bit mode
        LCD_SET_BIT(0x30);
        LCD_SET_BIT(0x30);
           //Then set to 4-bit mode
        LCD_SET_BIT(0x30);
        LCD_SET_BIT(0x20);
           //mode 4 bits, 2 lines, characters 5 x 7 (28 h)
        LCD_SET_BIT(0x20);
        LCD_SET_BIT(0x80);
           //no need cursor on (0Ch)
        LCD_SET_BIT(0x00);
        LCD_SET_BIT(0xC0);
           //the cursor moves to the left (06 h)
        LCD_SET_BIT(0x00);
        LCD_SET_BIT(0x60);
           //clears the display
        LCD_SET_BIT(0x00);
        LCD_SET_BIT(0x10);
}

void lcd_clear()
{
    LCD_SET_BIT(0x00);
    LCD_SET_BIT(0x10);
}

// Need the backlight lid.
void lcd_backlight(byte state)
{
  LCD_SET_BIT(0x00);
  LCD_SET_BIT(state);
}

//Display the character on LCD screen.
void display(char in_data)
{
        char data;
        data=in_data&0xF0;
        data=data+1; //set RS pin to 1
        fetch_data(data);
        data=in_data&0x0F;
        data=data<<4;
        data=data+1; //set RS pin to 1
        fetch_data(data);

}

//Make the x/y pointer 
void lcd_goto_xy(byte x, byte y)
{
  byte ptr1, ptr2;
  
  fetch_data(0x10);
  fetch_data(0x00);

  switch(y)
  {
   case 1:
          ptr1=line_1[x]&0xF0;// stamp the high bit
          ptr2=line_1[x]&0x0F;// stamp the low bit
        
     break;

   case 2:
          ptr1=line_2[x]&0xF0;
          ptr2=line_2[x]&0x0F;
         
     break;

   case 3:
          ptr1=line_3[x]&0xF0;
          ptr2=line_3[x]&0x0F;
          
     break;

   case 4:
          ptr1=line_4[x]&0xF0;
          ptr2=line_4[x]&0x0F;
         
     break;

   default:
          fetch_data(0x80);
          fetch_data(0x00);
     break;
     
  }
          ptr2=ptr2<<4;
          fetch_data(ptr1);
          fetch_data(ptr2);          
} 