#define BOOTLOADER_PIC18F458
//#define BOOTLOADER_PIC18F8722


/* ------------------------------------------------------------------------- */
/* PIC18F458 Bootloader                                                      */
/* ------------------------------------------------------------------------- */
//#ifdef BOOTLOADER_PIC18F458

#include <18F46K22.h>
#device adc=10

//#fuses HSH,NOWDT,PROTECT			 // Configuration word
#fuses HSH,NOMCLR,PUT,NOWDT,NOLVP,NOPLLEN,PROTECT,NOBROWNOUT

/*
#FUSES NOWDT                     //No Watch Dog Timer
#FUSES WDT128                    //Watch Dog Timer uses 1:128 Postscale
#FUSES HS                        //HS + 4x PLL
#FUSES NOPROTECT                 //Code not protected from reading
#FUSES NOOSCSEN                  //Oscillator switching is disabled, main oscillator is source
#FUSES NOBROWNOUT                //No brownout reset
//#FUSES BORV20                    //Brownout reset at 2.0V
#FUSES NOPUT                     //No Power Up Timer
#FUSES NOCPD                     //No EE protection
#FUSES STVREN                    //Stack full/underflow will cause reset
#FUSES NODEBUG                   //No Debug mode for ICD
#FUSES LVP                       //Low voltage prgming, B3(PIC16) or B5(PIC18) used for I/O
#FUSES NOWRT                     //Program memory not write protected
#FUSES NOWRTD                    //Data EEPROM not write protected
#FUSES NOWRTB                    //Boot block not write protected
#FUSES NOCPB                     //No Boot Block code protection
#FUSES NOWRTC                    //configuration not registers write protected
#FUSES NOEBTR                    //Memory not protected from table reads
#FUSES NOEBTRB                   //Boot block not protected from table reads*/

#use delay(clock=20000000)

//#endif /* #ifdef BOOTLOADER_PIC18F458 */



/* ------------------------------------------------------------------------- */
/* PIC18F8722 Bootloader                                                     */
/* ------------------------------------------------------------------------- */
#ifdef BOOTLOADER_PIC18F8722

#include <18F8722.h>
#device adc=8

#FUSES NOWDT                     //No Watch Dog Timer
#FUSES WDT128                    //Watch Dog Timer uses 1:128 Postscale
#FUSES H4                        //High speed osc with HW enabled 4X PLL
#FUSES NOPROTECT                 //Code not protected from reading
#FUSES IESO                      //Internal External Switch Over mode enabled
#FUSES NOBROWNOUT                //No brownout reset
#FUSES BORV20                    //Brownout reset at 2.0V
#FUSES NOPUT                     //No Power Up Timer
#FUSES NOCPD                     //No EE protection
#FUSES STVREN                    //Stack full/underflow will cause reset
#FUSES NODEBUG                   //No Debug mode for ICD
#FUSES LVP                       //Low voltage prgming, B3(PIC16) or B5(PIC18) used for I/O
#FUSES NOWRT                     //Program memory not write protected
#FUSES NOCPB                     //No Boot Block code protection
#FUSES NOEBTRB                   //Boot block not protected from table reads
#FUSES NOEBTR                    //Memory not protected from table reads
#FUSES NOWRTD                    //Data EEPROM not write protected
#FUSES NOWRTC                    //configuration not registers write protected
#FUSES NOWRTB                    //Boot block not write protected
#FUSES FCMEN                     //Fail-safe clock monitor enabled
#FUSES LPT1OSC                   //Timer1 configured for low-power operation
#FUSES MCLR                      //Master Clear pin enabled
#FUSES NOXINST                   //Extended set extension and Indexed Addressing mode disabled (Legacy mode)
#FUSES MCU                       //Microcontroller Mode
#FUSES NOWAIT                    //Wait selections unavailable for Table Reads or Table Writes
#FUSES BW16                      //16-bit external bus mode
#FUSES ABW20                     //20-bit Address bus
#FUSES ECCPE                     //Enhanced CCP PWM outpts multiplexed with RE6 thorugh RE3
#FUSES BBSIZ4K                   //4K words Boot Block size

#use delay(clock=40000000)

#endif /* #ifdef BOOTLOADER_PIC18F8722 */
