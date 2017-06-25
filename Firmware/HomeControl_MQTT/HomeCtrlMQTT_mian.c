#include <18F46K22.h>
//#fuses HS,NOWDT,PROTECT,NODEBUG  // fuses  configurados          //for 18f452 ,NOBROWNOUT ,PUT
#device ADC=10                    // Set ADC 10 Bit

// ทำการ Setup fuse bit  เพื่อกำหนดการเริ่มต้นของ MCU
#FUSES NOWDT                     //No Watch Dog Timer
#FUSES WDT128                    //Watch Dog Timer uses 1:128 Postscale
#FUSES HSH                        //HS + 4x PLL
#FUSES NOPROTECT                 //Code not protected from reading
//#FUSES NOOSCSEN                  //Oscillator switching is disabled, main oscillator is source
#FUSES NOBROWNOUT                //No brownout reset
//#FUSES BORV20                   z //Brownout reset at 2.0V
#FUSES NOPUT                     //No Power Up Timer
#FUSES NOCPD                     //No EE protection
#FUSES STVREN                    //Stack full/underflow will cause reset
#FUSES NODEBUG                   //No Debug mode for ICD
//#FUSES LVP                     //Low voltage prgming, B3(PIC16) or B5(PIC18) used for I/O
#FUSES NOWRT                     //Program memory not write protected
#FUSES NOWRTD                    //Data EEPROM not write protected
#FUSES NOWRTB                    //Boot block not write protected
#FUSES NOCPB                     //No Boot Block code protection
#FUSES NOWRTC                    //configuration not registers write protected
#FUSES NOEBTR                    //Memory not protected from table reads
#FUSES NOEBTRB                   //Boot block not protected from table reads*/

#use delay(clock=20000000,RESTART_WDT)           // oscillator for Boot Loader - กำหนดคล็อกของ MCU ให้ทำงานที่ 20 MHz

#priority int_TIMER1,rda,rtcc,int_TIMER2,ad,rda2,int_TIMER3

//#use delay(clock=20000000)
#use rs232(baud=9600 ,xmit=pin_C6,rcv=pin_C7,stream=WIFI,errors)			// กำหนด Boudrate สำหรับ RS-232
#use rs232(baud=57600 ,xmit=pin_D6,rcv=pin_D7,stream=ESP8266,errors)			// 

/* ------------------------------------------------------------------------- */
/* map reset vector and interrupt vector                                     */
/* 0x000-0x3FF is used by the bootloader. The bootloader maps the original   */
/* reset vector (0x000) to 0x400 and the interrupt vector (0x008) to 0x408.  */
/* ------------------------------------------------------------------------- */
#build (reset=0x400, interrupt=0x408)										// เริ่มต้น reste vector ให้ไปที่ address 0x400 และอินเตอร์รัพเริ่มที่ address 0x408
/* ------------------------------------------------------------------------- */
/* reserve boot block area                                                   */
/* This memory range is used by the bootloader, so the application must not  */
/* use this area.                                                            */
/* ------------------------------------------------------------------------- */
#org 0, 0x3FF {}															//เริ่มทำงานในส่วนโปรแกรม เมมโมรี่ที่ แอดเดรส 0 ไปจนถึง 0x3FF

#include <stdio.h>
#include <stdlib.h>

#include "HCMQTT_Header.h"
#include "LCD_I2C_20x4.h"
#include "DS1302.c"								//นำเข้า ไลบารี DS1302

#include "IRRemote.c"							//นำเข้า ไลบารี่ IR Remot

#include "WiFi_linker.c"						//นำเข้า ไลบารี่ WiFi Linker
#include "MQTT_linker.c"						//นำเข้า ไลบารี่ MQTT Linker

#include "Timer.c"								//นำเข้า ไลบารี่ Timer
#include "touch.c" 

#include "AccessoryService.c"
#include "ISR_Process.c"

void main()
{
	 set_tris_a(0b00001111);					// A0 = BATT
     set_tris_b(0b10000001);                    // B4 = OP4,B2 = I/O, B1 = XPORT_RESET, B0 = IR Remote
   	 set_tris_c(0b10000010);					// RC7 = O/P, RC6 = I/P , C1 Remote LED
     set_tris_d(0b10000000);                    // D
     set_tris_e(0b00000100);					// E2 = Connection, E1 = ESP8266_Reset, E0 = WiFi_Reset

	 write_eeprom (UPGRADE_ADDRESS, 0x39);		// Clear Upgrade New Firmware
	 write_eeprom (FWVERSIONADDR1, '0'); 
	 write_eeprom (FWVERSIONADDR2, '0');
	 write_eeprom (FWVERSIONADDR3, '.');
	 write_eeprom (FWVERSIONADDR4, '9');
	 write_eeprom (FWVERSIONADDR5, '0');
	 

     //setup_adc_ports(0);
     //setup_adc(ADC_CLOCK_INTERNAL);

     int_count=INTS_PER_SECOND; 
     setup_counters( RTCC_INTERNAL, RTCC_DIV_256); 
     setup_timer_0(RTCC_INTERNAL | RTCC_DIV_256 | RTCC_8_BIT); //Timer 0 for System Clock

	 enable_interrupts(INT_RTCC);

	 /*/ IR Remote ==========================
	 setup_counters(RTCC_INTERNAL,RTCC_DIV_1);    // get high
	 setup_timer_1(T1_INTERNAL|T1_DIV_BY_1);

	 ext_int_edge(H_TO_L);      // init interrupt triggering for button press
	 enable_interrupts(INT_EXT);// turn on interrupts

	//======================================*/

	 //set_timer3(0);
	 //setup_timer_3 (T3_INTERNAL|T3_DIV_BY_1);	// 	เลข 1 คือให้ over flow 1 รอบ

	 enable_interrupts(INT_RDA);
	 clear_interrupt(INT_RDA);

	 enable_interrupts(INT_RDA2);              	// enable RDA 2 interrupts
	 clear_interrupt(INT_RDA2);

	

	 enable_interrupts(GLOBAL);                 // enable global ints


	 output_high(WIFI_RESET);
     output_high(ESP8266_RESET);

	 output_low(SIREN);

  	 delay_ms(20);
  	 lcd_init();

	 rtc_init();															// Innitail ตัวเรียลไทม์คล็อค IC เบอร์ DS1302

     lcd_goto_xy(0,2);
     display("HomeControl V4.0");

	 fprintf(WIFI,"\r\nHome Control Plus - MQTT V00.90\r\n");
	 HardResetESP8266();

     Last_Switch_State();													//โหลด สถานะสวิตซ์ ครั้งก่อนหน้านี้
	 //Load_IR_DATA();

 

	 while(1)
	 {


				if(WIFI_Flag)
				{
					WiFi_Receive();											//เรียกฟังก์ชั่นอ่านข้อมูลจาก Xport
				}
				else if(MQTTDataReadyFlag)
				{
					ProcessMQTTData();
				}	
				else{
					Show_RTC();												//เรียกฟังก์ชั่นอ่านสัญญาณนาฬิกา
					//Check_Timer();	
					Check_Connecttion();
					/*if(Boot_Flag)
					{
						ReadTemp_DS1820();
						CheckBattery();
						CheckIntrusion();
					}*/
				}

	 }
}