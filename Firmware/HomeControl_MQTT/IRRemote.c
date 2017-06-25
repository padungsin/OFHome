#define IR_TX 		PIN_C1

#define IR_INPUT 	PIN_B0         // the IR-Decoder has to be connected to an interrupt pin 
#define IR_SIGNAL (!input(IR_INPUT))  // invert signal from IR-Receiver 

#DEFINE IRTXON      set_tris_C(0b10000000);
#DEFINE IRTXOFF     set_tris_C(0b10000010);

int32 IR_Data;
int32 IR_Code;

int1 IR_Input_State = FALSE;
unsigned long bitcount;
int PULSE_MAX = 50;
int PULSE = 0;

int1 DoneLowState = true;	
int1 DoneHighState = true;
int1 DoneSignalLow = true;
int1 DoneHighWide = true;
int1 DoneHighNarrow = true;

int16 WaitHeadLowEdge;
int16 WaitHeadHighEdge;
int16 WaitSignalLow;
int16 WaitHighWide;
int16 WaitHighNarrow;

int1 Waiting_IR = true;
int1 Detect_IR_Flag = false;
int16 Waiting_IR_Count = 0;
int IR_Switch_CH;

long y;

int16 highEdge,lowEdge;
int16 HeadLowEdge,HeadHighEdge;

int16 Signal_high_wide;
int16 Signal_high_narrow;
int16 Signal_low;

#define REM_ADDRESS_DEFINE 61
int16 REM_ADDRESS;		//Strat at @61

int IR_Data_Read[5];
int IR_LOAD_DATA[33];
int32 IR_CH[8];

void ir_get_fall(void);
void IR_Read();
void IR_Send();

#int_EXT 
EXT_isr() 
{ 
	ir_get_fall();

	if(lowEdge > 20000 )
	{
//    	output_high(LED2);

	    HeadLowEdge = lowEdge;
		IR_Input_State = TRUE;
		disable_interrupts(INT_EXT);

		IR_Read();
	}
} 
//======================================================================================
void ir_get_fall(void
{
		set_timer1(0);
		while(IR_SIGNAL);          // wait for signal to go low 
		lowEdge = get_timer1();
}
/*/======================================================================================
void ir_get_rise(void)
{
		set_timer1(0);
		while(!IR_SIGNAL);          // wait for signal to go high    // RC2, CCP1, pin 17	
		highEdge = get_timer1();

}*/

//=======================================================================================
void Load_IR_DATA()
{
	REM_ADDRESS = REM_ADDRESS_DEFINE ;

	for(int y=0; y<8; y++)
	{
		for(i=0; i<4; i++)
		{
			IR_LOAD_DATA[( y*4 ) + i ] = read_eeprom (REM_ADDRESS + ( y*4 ) + i);
			//fprintf(WIFI,"\r\nIR_LOAD_DATA[%u] = read_eeprom (%lu) ,Data( %u )",(( y*4 ) + i), REM_ADDRESS + ( y*4 ) + i ,IR_LOAD_DATA[( y*4 ) + i ]);		//Debug
		}
	}

	for(i=0; i<8; i++)
	{
		IR_CH[i] = make32(IR_LOAD_DATA[( i*4 )+3],IR_LOAD_DATA[( i*4 )+2],IR_LOAD_DATA[( i*4 )+1],IR_LOAD_DATA[( i*4 )]);
		delay_ms(10);
		//fprintf(WIFI,"\n\rIR_CH[%u] = %lu",i,IR_CH[i]);		//Debug
	}

}

//=======================================================================================
void IR_Save()
{
	fprintf(WIFI,"\r\nIR_Switch_CH = %u",IR_Switch_CH);
	REM_ADDRESS = REM_ADDRESS_DEFINE + ( IR_Switch_CH - 1 ) * 4; 

	for(i=0; i<4; i++)
	{
		write_eeprom (REM_ADDRESS + i, IR_Data_Read[i]);
		delay_ms(10);
		//fprintf(WIFI,"\r\nIR_Save REM_ADDRESS = %lu , IR_Data_Read[%u] = %u",REM_ADDRESS + i,i,IR_Data_Read[i]);
	}

	Detect_IR_Flag = false;

	Load_IR_DATA();
}
//=======================================================================================
void Clear_IR_Save()
{
	for(int y=0; y<8; y++)
	{
		for(i=0; i<4; i++)
		{
			write_eeprom (REM_ADDRESS_DEFINE + (y*4) + i, 0xFF);
			delay_ms(10);
			//fprintf(WIFI,"\r\nREM_ADDRESS = %lu , IR_Data_Read[%u] = 0xFF",((REM_ADDRESS + y*4)+i),i);		//Debug
		}
	}
		
	reset_cpu();
}

//=======================================================================================
void IR_Read()
{
			set_timer1(0);
			while(!IR_SIGNAL);          // wait for signal to go high    // RC2, CCP1, pin 17
			HeadHighEdge = get_timer1();

			PULSE = 0;
			for(bitcount=0;bitcount<PULSE_MAX;bitcount++)
			{
				ir_get_fall();	
				Signal_low = lowEdge;

				set_timer1(0);
				while(!IR_SIGNAL)
				{
					if(get_timer1() > 57000)          // wait for signal to go high    // RC2, CCP1, pin 17
					{
						goto StopRead;	
					}
				}
				
				highEdge = get_timer1();
				if(highEdge > 5000)
				{
					bit_set(IR_Data, bitcount);
					Signal_high_wide = highEdge;
				}
				else if(highEdge < 5000)
				{
					bit_clear(IR_Data, bitcount);
					Signal_high_narrow = highEdge;	
				}

				PULSE++;

			}
StopRead: delay_us(1);
		  delay_us(1);

		  	IR_Data_Read[3] = make8(IR_Data,3); 
			IR_Data_Read[2] = make8(IR_Data,2); 
			IR_Data_Read[1] = make8(IR_Data,1); 
			IR_Data_Read[0] = make8(IR_Data,0); 

			Detect_IR_Flag = true;

			fprintf(WIFI,"\n\rIR_Data = %lu\n",IR_Data);		//DEBUG
			fprintf(WIFI,"$MQTTYIRSEND,%lu,%lu,%lu,%lu,%lu,%u,1b",HeadLowEdge,HeadHighEdge,Signal_high_wide,Signal_high_narrow,Signal_low,PULSE);

			//enable_interrupts(INT_EXT);// turn on interrupts

}
//=======================================================================================
void IR_Decode_Data()
{
	

			Detect_IR_Flag = false;
			IR_Input_State = FALSE;
			enable_interrupts(INT_EXT);// turn on interrupts

}

//=======================================================================================
void LowSignalGen()
{
			DoneSignalLow = true;
			set_timer1(0);
			while(DoneSignalLow)
			{
				IRTXON;								// Enable IR TX
				if(get_timer1() >= Signal_low)
				{
					WaitSignalLow = get_timer1();
					DoneSignalLow = false;
				}
			}
}
//=======================================================================================
void HighWideGen()
{
			DoneHighWide = true;
			set_timer1(0);
			while(DoneHighWide)	
			{
				IRTXOFF;
				if(get_timer1() >= Signal_high_wide)
				{
					WaitHighWide = get_timer1();
					DoneHighWide = false;
				}
			}
}
//=======================================================================================
void HighNarrowGen()
{
			DoneHighNarrow = true;
			set_timer1(0);
			while(DoneHighNarrow)	
			{
				IRTXOFF;
				if(get_timer1() >= Signal_high_narrow)
				{
					WaitHighNarrow = get_timer1();
					DoneHighNarrow = false;
				}
			}
}
//=======================================================================================
void IR_Send()
{
		      setup_ccp2(CCP_PWM);
		      set_pwm2_duty(50);   //50% Duty Cycle
		      setup_timer_2(T2_DIV_BY_1,134,1);//37100 Hz

			  DoneLowState = true;	
			  set_timer1(0);		  
              while(DoneLowState)
			  { 
				IRTXON; 
				if(get_timer1() >= HeadLowEdge)
				{
					//fprintf(WIFI,"\r\nWait_HeadLowEdge = %lu",get_timer1());
					WaitHeadLowEdge = get_timer1();
					DoneLowState = false;
				}
			  }
			  
			  DoneHighState = true;
			  set_timer1(0);				
			  while(DoneHighState)
			  { 
				IRTXOFF; 
				if(get_timer1() >= HeadHighEdge)
				{
					WaitHeadHighEdge = get_timer1();
					DoneHighState = false;
				}
			  }

			  for(bitcount=0; bitcount<PULSE; bitcount++)
			  {			 
				 if(bit_test(IR_code,bitcount))		//Signal_Bit 1
				 {
				    LowSignalGen();
					HighWideGen();
				 }
				 else
				 {
				    LowSignalGen();
					HighNarrowGen();
				 }	 
			  }

			  // stop bit *****
		      LowSignalGen();

			  IRTXOFF;

			  //fprintf(WIFI,"\r\nWaitHeadLowEdge = %lu",WaitHeadLowEdge);
			  //fprintf(WIFI,"\r\nWaitHeadHighEdge = %lu",WaitHeadHighEdge);
			  //fprintf(WIFI,"\r\nWaitHighWide = %lu",WaitHighWide);
			  //fprintf(WIFI,"\r\nWaitHighNarrow = %lu",WaitHighNarrow);
			  //fprintf(WIFI,"\r\nWaitSignalLow = %lu",WaitSignalLow);

}
//======================================================================================