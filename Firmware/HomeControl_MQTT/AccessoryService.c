
#ifndef DS1820_PIN 
#define DS1820_PIN PIN_C0
#endif 

#define TOUCH_PIN DS1820_PIN 
#include "touch.c" 

//============================================================================
//
/************************************************************ ReadTemp_DS1820 */
//Description : Read Temperature (use RB0 read temp)
void ReadTemp_DS1820(void)
{

		byte i, buffer[9];
		
		if (touch_present()) { // get present (reset) (2)
				touch_write_byte(0xCC); // Skip ROM (3)
				touch_write_byte (0x44); // Start Conversion
				delay_ms(20); // delay 200 ms (4)
				touch_present(); // get present (reset) (5)
				touch_write_byte(0xCC); // Skip ROM (6)
				touch_write_byte (0xBE); // Read Scratch Pad
				
				for(i=0; i<9;i++) // read 9 bytes (7)
				buffer[i] = touch_read_byte();
		}
		temp = (buffer[1]<<4)|(buffer[0]>>4); // Temperature (8)
		raw_temp = buffer[1];
		raw_temp <<= 8;
		raw_temp += buffer[0];

		temp_Display[0] = temp/10 + 48;
		temp_Display[1] = temp%10 + 48;

		//temperature = ds1820_read(); 

		lcd_goto_xy(8,2);				//Line 3
        printf(display," T:%u ",temp);
	    //display(" T:");
	    //display(temp_Display[0]);
        //display(temp_Display[1]);
		//printf(display,"T:%3.1f ", temperature); 
		display(223);
        display("C  ");
}

//==========================================================================

void CheckBattery()
{
		long adc;
			
		set_adc_channel(0);									//RA1_ANALOG
        delay_us(10);

		adc = read_adc();                         			// Read a value from the ADC

		voltage = (float)adc*0.0169;						// Defaul 0.0186   Debug 0.02185

		lcd_goto_xy(0,2);
		printf(display,"B:%2.2fV",voltage);

}

//==========================================================================

void CheckIntrusion()
{
//check alert flag

}
//==========================================================================