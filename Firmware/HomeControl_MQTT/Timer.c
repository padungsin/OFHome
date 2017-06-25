

int16 timerTrigger = 5;

int1 timerProcessing[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
void Show_RTC()
{
	rtc_get_date( day, mth, year, dow);
	rtc_get_time( hour, min, sec );

	itoa(hour,10, hour_Display);									//แปลง int ให้เป็น String

	hour_Display[0] = hour/10 + 48;
	hour_Display[1] = hour%10 + 48;

	min_Display[0] = min/10 + 48;
	min_Display[1] = min%10 + 48;

    sec_Display[0] = sec/10 + 48;
    sec_Display[1] = sec%10 + 48;

	day_Display[0] = day/10 + 48;
	day_Display[1] = day%10 + 48;

	mth_Display[0] = mth/10 + 48;
	mth_Display[1] = mth%10 + 48;

	year_Display[0] = year/10 + 48;
	year_Display[1] = year%10 + 48;

	lcd_goto_xy(6,1);
	//lcd_goto_xy(8,1);
	display(dow+48);
	display(":");

	display(hour_Display[0]);
	display(hour_Display[1]);
	display(":");
	display(min_Display[0]);
	display(min_Display[1]);
	display(":");
	display(sec_Display[0]);
	display(sec_Display[1]);

	/*
	display("  ");
	display(day_Display[0]);
	display(day_Display[1]);
	display("/");
	display(mth_Display[0]);
	display(mth_Display[1]);
	*/
}


//======================================================================================================
//
void Check_Timer()
{

	int16 currentTrigDate = (year-2000)*10000 + mth*100 + day;

	
	int switchNumber;
	char deviceType;
	char *ptr;
	char *ptr2;
	int i;
	int1 movePtr = 0;
	char switchControlMode;
	//int secondDuration;


	if(timerTrigger > 0){
		timerTrigger--;
		return;
	}	

	timerTrigger = 7;

	ptr = activeTimerSwitch;
	
	//fprintf(WIFI,"PTR IS %u \n",ptr[0]);
	//fprintf(WIFI,"activeTimerSwitch IS %u \n",activeTimerSwitch[0]);

	while(ptr[0] != 0){

		
		i = ptr[0] - 1;
		//fprintf(WIFI,"CHECKING TIMER FOR %u \n",ptr[0]);

		if(timerProcessing[i] == 1){
			continue;
		}

		movePtr = 1;
		//fprintf(WIFI,"ON:%u:%u OFF:%u:%u",timerOnHour[i], timerOnMinute[i],timerOffHour[i], timerOffMinute[i]);
/*
		//check date if not reach trig date then skip
		if(timerMonth[i]*100 + timerDate[i] > mth*100 + day){
			ptr++;
			continue;
		}
*/
		//fprintf(WIFI,"TIMER %u Date check pass\n",ptr[0]);


		//if not recurrent (one time) do not check day
		if(bit_test(timerRecurrent[i], 7)){
			//check day
			//sun = 1
			if(dow == 1 && !bit_test(timerRecurrent[i], 6)){
				ptr++;
				continue;
			}
	
			if(dow == 2 && !bit_test(timerRecurrent[i],5)){
				ptr++;
				continue;
			}
	
			if(dow == 3 && !bit_test(timerRecurrent[i],4)){
				ptr++;
				continue;
			}
	
			if(dow == 4 && !bit_test(timerRecurrent[i],3)){
				ptr++;
				continue;
			}
	
			if(dow == 5 && !bit_test(timerRecurrent[i],2)){
				ptr++;
				continue;
			}
	
			if(dow == 6 && !bit_test(timerRecurrent[i],1)){
				ptr++;
				continue;
			}
	
			if(dow == 7 && !bit_test(timerRecurrent[i],0)){
				ptr++;
				continue;
			}
		}
		
		
		//fprintf(WIFI,"TIMER %u Day of Week to run pass\n",ptr[0]);


		//check time
		if(timerHour[i] != hour || timerMinute[i] != min || timerSecond[i] != sec){
			ptr++;
			continue;
		}


		

		//fprintf(WIFI,"TIMER %u Time to run pass\n",ptr[0]);

		//check trig
		if(lastTrigDate[i] > currentTrigDate){
			ptr++;
			continue;
		}
		
		
			
		switchNumber = timerSwitchNumber[i];
		deviceType = timerDeviceType[i];
		switchControlMode = timerControlMode[i];
//		secondDuration = timerSecondDuration[i];

		//fprintf(WIFI,"Time to %c Switch %u \n",switchControlMode, switchNumber);


		if(deviceType == WIRELESS){		
			fprintf(WIFI,"$MQSWCS;%u;%c;MASTER*", switchNumber, switchControlMode);
			//WaitingWireless_Flag = true;
			waitingSlaveCounter[switchNumber-1] = 1;
		}else if(deviceType == LIVOLO){
			fprintf(ESP8266,"$MQSWCL;%u;%c;MASTER*", switchNumber, switchControlMode);
			if(livoloSwitchStatus[i] == 0){
				livoloSwitchStatus[i] = 1;
			}else{
				livoloSwitchStatus[i] = 0;
			}
			write_eeprom(saveLivoloSwitchStatusAddress[i], livoloSwitchStatus[i]);
		}else{

			if(switchControlMode == SWITCH_ON){
				output_high(internalSwitch[switchNumber-1]);
				delay_ms(200);
				internalSwitchStatus[switchNumber-1] = 1;
				
			}else if(switchControlMode == SWITCH_OFF){
				output_low(internalSwitch[switchNumber-1]);
				delay_ms(200);
				internalSwitchStatus[switchNumber-1] = 0;
				fprintf(ESP8266,"$MQSWSTATUSN;%u;1*", switchNumber);
			}else if(switchControlMode == SWITCH_TOGGLE){
				output_high(internalSwitch[switchNumber-1]);
				delay_ms(200);
				output_low(internalSwitch[switchNumber-1]);
				internalSwitchStatus[switchNumber-1] = 0;
				fprintf(ESP8266,"$MQSWSTATUSN;%u;0*", switchNumber);

			}
		
			write_eeprom(saveInternalSwitchStatusAddress[i], internalSwitchStatus[i]);
				
	
			
		}
		

		timerProcessing[i] = 0;
		
		if(bit_test(timerRecurrent[i],7) == 0){
			//fprintf(WIFI,"Remove Switch %u From timer Queue\n", switchNumber);
			timerEnable[i] = 0;
			
			//remove from active list
			ptr2 = strchr (activeTimerSwitch,  i+1);
			while(ptr2[0] != 0){
				ptr2[0] = ptr2[1];
				ptr2++;
			}
			ptr2[0] = 0;
			ptr2 = NULL;
			movePtr = 0;
		}

		currentTrigDate = lastTrigDate[i];
		write_eeprom (timerEnableEEPROM[i], timerEnable[i]); 

		//move to next Active


		if(movePtr == 1){
			ptr++;
		}
		
	}


}
// End Program
//==================================================================================================================================================================


