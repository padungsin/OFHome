int1 MQTTDataReadyFlag = false;






int timerEnable[21] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int timerSwitchNumber[21] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
char timerDeviceType[21] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
char timerControlMode[21] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//int timerSecondDuration[21] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//int timerMonth[21] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//int timerDate[21] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int timerHour[21] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int timerMinute[21] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int timerSecond[21] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//0-255 bit 7 is recurrent flag
int timerRecurrent[21] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};






//replace above
char *esp8266Ptr;
char *esp8266Index;
int1 esp8266Reading = false;
//======================================================================================================
//
#INT_RDA2
void SerialInt2()
{   
	//get character from ESP8266 Module
	char esp8266Char = getchar(ESP8266);

	//if get $ then start read command for processing
	if(esp8266Char == '$'){
		//pointer need to be reset before start storing
		esp8266Index = esp8266Ptr;
		*esp8266Index = esp8266Char;
		esp8266Index++;
		esp8266Reading = true;
	}else if(esp8266Reading == true){
		*esp8266Index = esp8266Char;
		esp8266Index++;
		if(esp8266Char == '*'){
			*esp8266Index = 0;
			esp8266Reading = false;
			MQTTDataReadyFlag = true;
		}
	}
} 

//========================================================================================================
// 
void ClearESP8266_Receive()
{
	esp8266Index = esp8266Ptr;
	MQTTDataReadyFlag = false;
}

//========================================================================================================
void ProcessMQTTData()
{
	fprintf(WIFI,"CMD: %s\n", esp8266Ptr);

	char *WIFIStatus = "$ESP8266WIFISTATUS";
	char *MQStatus = "$ESP8266MQSTATUS";
	//$MQTT,IP192.168.1.201*

	char *statusSuccess = "SUCCESS";
	char *statusFailure = "FAILURE";



	char *accountInfo = "$ACRINFO";


	char *commandSwitch = "$MQSWC";
	char *commandSetTimer = "$MQTMSET";
	char *commandClearTimer = "$MQTMCLR";
	char *commandReadTimer = "$MQTMSTATUS";




	char *commandStatus = "$MQSWSTATUS";
	char *commandUpdate = "$MQSYSUPFW";
	char *commandSyncTime = "$MQTTPSYNRTC";


	char *commandIRSwitch = "$XPORTIRSLWR";
	char *commandIRSend = "$MQTTYIRSEND";
	char *commandIRRead = "$XPORTIRREAD";
	char *commandClearIR = "$XPORTIRSCLR";

	char *commandSecurityLock = "$MQTTYSCLOCK";
	char *commandSecurityUnlock = "$MQTTYUNLOCK";
	char *commandSecurityLoad = "$MQTTYSCLOAD";
	char *commandSetSecurity = "$MQTTYSETSECURE";




	char tempCommand[COMMAND_SIZE], delimeter[2];
	char *command;

//for general switch
	int8 switchNumber;
	int8 requestSwitchNumber;
	char deviceType;
	char switchControlMode;
//tmp sw status
	char *allSwStatus = "";

//for timer
	//int secondDuration;
	int monthSet;
	int dateSet;
	int hourSet;
	int minuteSet;
	int recurrent;
	char recurrentStr[7];
	char* recurrentDay;
	int setId;

	char *ptr;
	char *searchResultPtr;
	char *uuid;
	




	//on boot
	//$MQTTPSYNRTC:13:11:2016:1:12:56:38:*
	
	
	strcpy(tempCommand, esp8266Ptr);               // copy the string to temp string
   	strcpy(delimeter,";"); 

	//remove *
	searchResultPtr = strchr(tempCommand, '*');
	if(searchResultPtr != NULL){
		searchResultPtr[0]=0;
	}


	
	command = strtok(tempCommand,delimeter);


	if(command != 0){


		if(strcmp(command, MQStatus) == 0){
			ptr = strtok(0,delimeter);
			if(strcmp(ptr, statusSuccess) == 0){
				lcd_goto_xy(0,1);
				display("MQTT    ");
		
			    lcd_goto_xy(0,2);
		     	display("                ");
		
			 	Boot_Flag = true;
				MQTTConnect_Flag = false;

			}else{
				lcd_goto_xy(0,1);
				display("FAIL    ");
		
			    lcd_goto_xy(0,2);
		     	display("                ");
			}

			ptr = NULL;
			ClearESP8266_Receive();
			return;
		}

		if(strcmp(command, WIFIStatus) == 0){
			ptr = strtok(0,delimeter);
			if(strcmp(ptr, statusSuccess) == 0){
				lcd_goto_xy(0,2);
				display("                ");
		
			 	Boot_Flag = true;
			}else{
				lcd_goto_xy(5,1);
				display("E2");
		
				lcd_goto_xy(0,2);
				display("WiFi Signal Loss");
		
			 	Boot_Flag = false;
			}
			ptr = NULL;
			ClearESP8266_Receive();
			return;
		}


		//NEW $MQSWC;1;F;UUID;*
		if(strcmp(command, commandSwitch) == 0){
			requestSwitchNumber = atoi(strtok(0,delimeter));
	
			//ptr = strtok(0,delimeter);
			//deviceType = ptr[0];
			ptr = strtok(0,delimeter);
			switchControlMode = ptr[0];
		
			uuid = strtok(0,delimeter);

			if(requestSwitchNumber <= sizeof(internalSwitchStatus)){
				switchNumber = requestSwitchNumber;
				deviceType = INTERNAL;
			}else if(requestSwitchNumber > sizeof(internalSwitchStatus) && requestSwitchNumber <= sizeof(internalSwitchStatus) + sizeof(wirelessSwitchStatus)){
				switchNumber = requestSwitchNumber - sizeof(internalSwitchStatus);
				deviceType = WIRELESS;
			}else{
				switchNumber = requestSwitchNumber - sizeof(internalSwitchStatus) - sizeof(wirelessSwitchStatus);
				deviceType = LIVOLO;
			}
			
		
		
			//farward command to wifi and wait for response
			if(deviceType == WIRELESS){
				fprintf(WIFI,"%sS;%u;%c;%s*", command, switchNumber, switchControlMode, uuid);
				waitingSlaveCounter[switchNumber-1] = 1;
				uuids[switchNumber-1] = uuid;
			}else if(deviceType == LIVOLO){
				fprintf(ESP8266,"%sL;%u;%c;%s*", command, switchNumber, switchControlMode, uuid);
				if(livoloSwitchStatus[switchNumber-1] == 0){
					livoloSwitchStatus[switchNumber-1] = 1;
				}else{
					livoloSwitchStatus[switchNumber-1] = 0;
				}
				//fprintf(ESP8266,"%sR;%u;%c;%c;1;%s*", command, switchNumber, deviceType ,switchControlMode, uuid);
				write_eeprom(saveLivoloSwitchStatusAddress[switchnumber-1],livoloSwitchStatus[switchnumber-1]);
			}else{

				
				fprintf(WIFI,"Switch Internal: %u\n",switchNumber);
				fprintf(WIFI,"Control Mode: %c\n",switchControlMode);
				
				if(switchControlMode == SWITCH_ON){
					output_high(internalSwitch[switchNumber-1]);
					delay_ms(200);
					internalSwitchStatus[switchNumber-1] = 1;
					
				}else if(switchControlMode == SWITCH_OFF){
					output_low(internalSwitch[switchNumber-1]);
					delay_ms(200);
					internalSwitchStatus[switchNumber-1] = 0;
				
				}else if(switchControlMode == SWITCH_TOGGLE){
					output_high(internalSwitch[switchNumber-1]);
					delay_ms(2000);
					output_low(internalSwitch[switchNumber-1]);
					internalSwitchStatus[switchNumber-1] = 0;
				}
	
				//$MQSWCR;switchNumber;switchControlMode;success;uuid
				fprintf(ESP8266,"%sR;%u;%c;1;%s*", command, requestSwitchNumber ,switchControlMode, uuid);
				write_eeprom(saveInternalSwitchStatusAddress[switchnumber-1],internalSwitchStatus[switchnumber-1]);
			}
	
	
			ptr = NULL;
			ClearESP8266_Receive();
			return;
		}
	

		//$MQSWSTATUS;uuid*
		if(strcmp(command, commandStatus) == 0){
			uuid = strtok(0,delimeter);
			delay_ms(200);

			strcpy(allSwStatus, command);
//			ptr = allSwStatus + strlen(allSwStatus);
//			ptr[0] = 'R';
//			ptr[1] = 0;
			
			

//			stringLength = strlen(allSwStatus);
//			allSwStatus[stringLength] = 'R';
			fprintf(ESP8266,"%sR", command);
			
			
			fprintf(WIFI,"Size %u\n",sizeof(internalSwitchStatus)); 
			//fprintf(ESP8266,";I;%u", sizeof(internalSwitchStatus));
			for(i=0; i<sizeof(internalSwitchStatus); i++){

				itoa(internalSwitchStatus[i], 10, ptr);
				fprintf(ESP8266,";%s", ptr);
				ptr = NULL;

			}

			fprintf(WIFI,"Size %u\n",sizeof(wirelessSwitchStatus)); 
			//fprintf(ESP8266,";W;%u", sizeof(wirelessSwitchStatus));
			for(i=0; i<sizeof(wirelessSwitchStatus); i++){

				itoa(wirelessSwitchStatus[i], 10, ptr);
				fprintf(ESP8266,";%s", ptr);
				ptr = NULL;

			}

			fprintf(WIFI,"Size %u\n",sizeof(livoloSwitchStatus)); 
//			fprintf(ESP8266,";L;%u", sizeof(livoloSwitchStatus));
			for(i=0; i<sizeof(livoloSwitchStatus); i++){

				//itoa(livoloSwitchStatus[i], 10, ptr);
				//fprintf(ESP8266,";%s", ptr);
				//ptr = NULL;
				fprintf(ESP8266,";0");

			}


			fprintf(ESP8266,";%s*", uuid);


			ptr = NULL;
			ClearESP8266_Receive();
			return;
		}

		//============================================================================================================================          
		//$MQTMSET;id;1;T;20;M;d;H;m;1;0-127;uuid*
		//id=99 for new timer
		if(strcmp(command, commandSetTimer) == 0){
			setId = atoi(strtok(0,delimeter));

			requestSwitchNumber = atoi(strtok(0,delimeter));
			ptr = strtok(0,delimeter);
			//deviceType = ptr[0];
			ptr = strtok(0,delimeter);
			switchControlMode = ptr[0];
		
			//secondDuration = atoi(strtok(0,delimeter));
			//monthSet = atoi(strtok(0,delimeter));
			//dateSet = atoi(strtok(0,delimeter));
			hourSet = atoi(strtok(0,delimeter));
			minuteSet = atoi(strtok(0,delimeter));
			recurrent = atoi(strtok(0,delimeter));
			recurrentDay = strtok(0,delimeter);

			//last command
			uuid = strtok(0,delimeter);


			if(requestSwitchNumber <= sizeof(internalSwitchStatus)){
				switchNumber = requestSwitchNumber;
				deviceType = INTERNAL;
			}else if(requestSwitchNumber > sizeof(internalSwitchStatus) && requestSwitchNumber <= sizeof(internalSwitchStatus) + sizeof(wirelessSwitchStatus)){
				switchNumber = requestSwitchNumber - sizeof(internalSwitchStatus);
				deviceType = WIRELESS;
			}else{
				switchNumber = requestSwitchNumber - sizeof(internalSwitchStatus) - sizeof(wirelessSwitchStatus);
				deviceType = LIVOLO;
			}


			
	

			timerEnable[setId] = 1;
			timerSwitchNumber[setId] = switchNumber;
			timerDeviceType[setId] = deviceType;
			timerControlMode[setId] = switchControlMode;
//			timerSecondDuration[setId] = secondDuration;
//			timerMonth[setId] = monthSet;
//			timerDate[setId] = dateSet;
			timerHour[setId] = hourSet;
			timerMinute[setId] = minuteSet;
			

			
			if(recurrent == 1){
				
				bit_set(timerRecurrent[setId],7);

				//sunday
				if(recurrentDay[0] == '1'){
					bit_set(timerRecurrent[setId],6);
				}else{
					bit_clear(timerRecurrent[setId],6);
				}
				
				//monday
				if(recurrentDay[1] == '1'){
					bit_set(timerRecurrent[setId],5);
				}else{
					bit_clear(timerRecurrent[setId],5);
				}

				//tuesday
				if(recurrentDay[2] == '1'){
					bit_set(timerRecurrent[setId],4);
				}else{
					bit_clear(timerRecurrent[setId],4);
				}

				//wednesday
				if(recurrentDay[3] == '1'){
					bit_set(timerRecurrent[setId],3);
				}else{
					bit_clear(timerRecurrent[setId],3);
				}

				//thursday
				if(recurrentDay[4] == '1'){
					bit_set(timerRecurrent[setId],2);
				}else{
					bit_clear(timerRecurrent[setId],2);
				}

				//friday
				if(recurrentDay[5] == '1'){
					bit_set(timerRecurrent[setId],1);
				}else{
					bit_clear(timerRecurrent[setId],1);
				}

				//saturday
				if(recurrentDay[6] == '1'){
					bit_set(timerRecurrent[setId],0);
				}else{
					bit_clear(timerRecurrent[setId],0);
				}
			
			}else{
				timerRecurrent[setId] = 0;
			}
			
			write_eeprom (timerEnableEEPROM[setId], timerEnable[setId]);
			write_eeprom (timerSwitchNumberEEPROM[setId], timerSwitchNumber[setId]);
			write_eeprom (timerControlModeEEPROM[setId], timerControlMode[setId]);
//			write_eeprom (timerSecondDurationEEPROM[setId], timerSecondDuration[setId]);
//			write_eeprom (timerMonthEEPROM[setId], timerMonth[setId]);
//			write_eeprom (timerDateEEPROM[setId], timerDate[setId]);
			write_eeprom (timerHourEEPROM[setId], timerHour[setId]);
			write_eeprom (timerMinuteEEPROM[setId], timerMinute[setId]);
			write_eeprom (timerRecurrentEEPROM[setId], timerRecurrent[setId]);

			delay_ms(200);

			lastTrigDate[setId] = 0;

			ptr = strchr (activeTimerSwitch,  setId+1);
			if(ptr == 0){
				ptr = activeTimerSwitch + strlen(activeTimerSwitch);
				ptr[0] = setId+1;
				ptr[1] = 0;
			}

			//fprintf(ESP8266,"$ACTIVE TIMER ADDED id: %u\nSwitch Number: %u*", ptr[0], switchNumber);
			ptr = NULL;
			
	//$MQTMSETR;id;1;T;20;M;d;H;m;1;1111111;success;uuid*

	        fprintf(ESP8266,"%sR;%u;%u;%u;%c;%u;%u;%u;%s;1;%s*", command, setId, timerEnable[setId], requestSwitchNumber, switchControlMode, hourSet, minuteSet, recurrent, recurrentDay, uuid);
			
	
			ClearESP8266_Receive();
			return;
	
		}


		//$MQTMCLR;id;uuid*
		//id=99 for new timer
		if(strcmp(command, commandClearTimer) == 0){
			setId = atoi(strtok(0,delimeter));
			uuid = strtok(0,delimeter);


			ptr = strchr (activeTimerSwitch,  setId+1);
			if(ptr == 0){
				fprintf(ESP8266,"%sR;%u;0;%s*", command, setId, uuid);
				ClearESP8266_Receive();
				return;
			}
			while(ptr[0] != 0){
				ptr[0] = ptr[1];
				ptr++;
			}
			ptr[0] = 0;
			ptr = NULL;
			
			fprintf(ESP8266,"%sR;%u;1;%s*", command, setId, uuid);
			ClearESP8266_Receive();
			return;
			
		}

		//$MQTMSTATUS;id;uuid*
		//id=99 for new timer
		if(strcmp(command, commandReadTimer) == 0){
			setId = atoi(strtok(0,delimeter));
			uuid = strtok(0,delimeter);
			
			recurrent = bit_test(timerRecurrent[setId], 7);
			itoa(timerRecurrent[setId],2, recurrentDay);
			ptr = recurrentDay + 1;
			//itoa(x,10, string);


			if(timerDeviceType[setId] == INTERNAL){
				switchNumber = timerSwitchNumber[setId];
			
			}else if(timerDeviceType[setId] == WIRELESS){
				switchNumber = timerSwitchNumber[setId] + sizeof(internalSwitchStatus);
				
			}else{
				switchNumber = timerSwitchNumber[setId] + sizeof(internalSwitchStatus) + sizeof(wirelessSwitchStatus);
			}
	
	        fprintf(ESP8266,"%sR;%u;%u;%u;%c;%u;%u;%u;%s;1;%s*", command, setId ,timerEnable[setId], switchNumber, timerControlMode[setId], timerHour[setId], timerMinute[setId], recurrent, ptr, uuid);
			ptr = NULL;
			ClearESP8266_Receive();
			return;
			
		}



	    //===========================================================================================================================
		//Online Update Firmware
		//============================================================================================================================
	    if(strcmp(command, commandUpdate) == 0){
		
     		lcd_goto_xy(0,1);
     		display("Process OTA ....");

			lcd_goto_xy(0,2);
     		display("Upload New F/W  ");

			fprintf(WIFI,"Upgrade New Firmware Process\r\n");
			write_eeprom (UPGRADE_ADDRESS, 0x31);				// Set Upgrade New Firmware
			delay_ms(100);
			Reset_CPU();
		}


	}






//============================================================================================================================
	//$MQTTPSYNRTC + : ( ตัวแบ่งข้อมูล ) + วัน : เดือน : ปี : DOW* : ชั่วโมง : นาที : วินาที( ซึ่งทุกๆ ข้อมูลต้องคั่นด้วยตัวแบ่งข้อมูลทุกครั้ง ) ==========================================
	//$MQTTPSYNRTC:23:10:2016:1:10:58:34:**

	searchResultPtr = strstr(esp8266Ptr, commandSyncTime);
    if(searchResultPtr != NULL){
		strcpy(tempCommand, esp8266Ptr);               // copy the string to temp string
   		strcpy(delimeter,":"); 
		ptr = strtok(tempCommand,delimeter);
		if(ptr==0){
			ClearESP8266_Receive();
			return;
		}

		day = atoi(strtok(0,delimeter));
		mth = atoi(strtok(0,delimeter));
		year = atoi(strtok(0,delimeter));
		dow = atoi(strtok(0,delimeter));
		hour = atoi(strtok(0,delimeter));
		min = atoi(strtok(0,delimeter));
		sec = atoi(strtok(0,delimeter));
		rtc_set_datetime(day,mth,year,dow,hour,min,sec);
		ptr = NULL;
		ClearESP8266_Receive();

		UpdateRTC_flag = false;

		return;	
	}





	//%$ACCINFO*
	searchResultPtr = strstr(esp8266Ptr, accountInfo);
	if(searchResultPtr != NULL){
		delay_ms(100);
		fprintf(ESP8266,"$ACR,%2.2f,%u,%u,*",voltage,temp,HomeLock_Flag);
		ClearESP8266_Receive();
		return;
	}

	ClearESP8266_Receive();
}

//==================================================================================================================================================================
void Last_Switch_State()
{
			   //===================================================================
	disable_interrupts(GLOBAL);                 // enable global ints
			
	int timerEnableSize = sizeOf(timerEnable);
	char *ptr;

	//load internal switch state
	for(int i = 0; i < sizeOf(saveInternalSwitchStatusAddress); i++){
		internalSwitchStatus[i] = read_eeprom (saveInternalSwitchStatusAddress[i]);

		if(internalSwitchStatus[i] > 1){
			internalSwitchStatus[i] = 0;
		}

		if(internalSwitchStatus[i] == 1){
			output_high(internalSwitch[i]);
		}else{
			output_low(internalSwitch[i]);
		}
	
	}

	//load wireless switch state
	for(int i = 0; i < sizeOf(saveWirelessSwitchStatusAddress); i++){
		wirelessSwitchStatus[i] = read_eeprom (saveWirelessSwitchStatusAddress[i]);

		if(wirelessSwitchStatus[i] > 1){
			wirelessSwitchStatus[i] = 0;
		}

		if(wirelessSwitchStatus[i] == 1){
			fprintf(WIFI,"$MQSWCS;%u;N;MASTER*", i+1);
			
		}else{
			fprintf(WIFI,"$MQSWCS;%u;F;MASTER*", i+1);
		}
		waitingSlaveCounter[i] = 1;
	
	}			

	//load livolo switch state
	for(int i = 0; i < sizeOf(saveLivoloSwitchStatusAddress); i++){
		livoloSwitchStatus[i] = read_eeprom (saveLivoloSwitchStatusAddress[i]);

		if(livoloSwitchStatus[i] > 1){
			livoloSwitchStatus[i] = 0;
		}

	}	

	ptr = activeTimerSwitch;
	for(int i = 0; i < sizeof(timerEnableEEPROM); i++){

		timerEnable[i]  = read_eeprom (timerEnableEEPROM[i]);
		if(timerEnable[i] > 1){
			timerEnable[i] = 0;
		}
		
		if(timerEnable[i] == 1){
		
			timerSwitchNumber[i]  = read_eeprom (timerSwitchNumberEEPROM[i]); 
			timerDeviceType[i]  = read_eeprom (timerDeviceTypeEEPROM[i]); 
			timerControlMode[i]  = read_eeprom (timerControlModeEEPROM[i]);
//			timerSecondDuration[i]  = read_eeprom (timerSecondDurationEEPROM[i]);
//			timerMonth[i]  = read_eeprom (timerMonthEEPROM[i]);
//			timerDate[i]  = read_eeprom (timerDateEEPROM[i]); 
			timerHour[i]  = read_eeprom (timerHourEEPROM[i]);
			timerMinute[i]  = read_eeprom (timerMinuteEEPROM[i]); 
			timerRecurrent[i]  = read_eeprom (timerRecurrentEEPROM[i]); 
			
			ptr[0] = i+1;
			ptr++;
		}
		if(i == timerEnableSize-1){
			ptr[0] = 0;
		}

	}


	enable_interrupts(GLOBAL);                 // enable global ints
   //===================================================================					
			
}

//==================================================================================
void HardResetESP8266()
{
	output_low(ESP8266_RESET);
	delay_ms(500);
	output_high(ESP8266_RESET);
}

//==================================================================================
void Check_Connecttion()
{
	if(input(ESP8266_Connect))
	{	
		Boot_Flag = false;

		connection_cnt++;	
		lcd_goto_xy(0,2);
		display("WiFi Loss ");
		printf(display,"%lu    ",connection_cnt);	
		if(connection_cnt > 999)
		{
			connection_cnt = 0;
			lcd_goto_xy(5,1);
			display("E1 ");
			HardResetESP8266();

			lcd_goto_xy(0,2);
			display("Reset WiFi      ");
			fprintf(WIFI,"Reset WiFi\n");
		}
	}
	else{
		connection_cnt = 0;
		Boot_Flag = true;
	}
}

//==================================================================================
