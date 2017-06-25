int1 WIFI_Flag = false;


//replace above
char *wifiPtr;
char *wifiIndex;
int1 wifiReading = false;

//======================================================================================================
//												Receive Data from U2 Process
#INT_RDA
void SeriaInput()
{
  
	//get character from wifi Module
	char wifiChar = getchar();

	//if get $ then start read command for processing
	if(wifiChar == '$' || wifiChar == '&'){
		//pointer need to be reset before start storing
		wifiIndex = wifiPtr;
		*wifiIndex = wifiChar;
		wifiIndex++;
		wifiReading = true;
	}else if(wifiReading == true){
		*wifiIndex = wifiChar;
		wifiIndex++;
		if(wifiChar == '*'){
			*wifiIndex = 0;
			wifiReading = false;
			WaitingWireless_Flag = false;
			WIFI_Flag = true;
		}
	}

} 

//========================================================================================================
// 
void Clear_Receive()
{
	wifiIndex = wifiPtr;
	WIFI_Flag = false;

}
//========================================================================================================
//
void WiFi_Receive()
{

	int correct_data;								// สำหรับการคัดกรองข้อมูลที่ถูกต้องใส่ไว้ใน Memory
	int datastart;


	char tempCommand[COMMAND_SIZE], delimeter[2];
	char *command;

//for general switch
	int8 switchNumber;
	int8 switchControlMode;

	char *ptr;
	char *searchResultPtr;
	char *uuid;

	//Response from slave
	char *responseCommandSwitch = "$MQSWCSR";
	char *nofifySlaveStatus = "$MQSWSTATUSSN";
	char *slaveCheckLink = "$SCHECKLINK";

	char switchType;
	int switchStatus;

	int success;

	correct_data = 0;
	datastart = 0;


	//fprintf(ESP8266,"WIFI CMD: %s*\n", wifiPtr);


	strcpy(tempCommand, wifiPtr);               // copy the string to temp string
   	strcpy(delimeter,";"); 

	

	//remove *
	searchResultPtr = strchr(tempCommand, '*');
	if(searchResultPtr != NULL){
		searchResultPtr[0]=0;
	}

	command = strtok(tempCommand,delimeter);
	if(command != 0){

		if(strcmp(command, slaveCheckLink) == 0){
			switchNumber = atoi(strtok(0,delimeter));
			fprintf(WIFI,"%sS;OK*",command);

			//debug
			//fprintf(ESP8266,"%sS;OK*",command);
	
			ptr = NULL;
			Clear_Receive();
			return;
		}

		////$MQSWCSR;switchNumber;switchControlMode;success;uuid
		if(strcmp(command, responseCommandSwitch) == 0){

			switchNumber = atoi(strtok(0,delimeter));
			waitingSlaveCounter[switchNumber-1] = 0;
	
			ptr = strtok(0,delimeter);
			switchControlMode = ptr[0];
			success = atoi(strtok(0,delimeter));
			uuid = strtok(0,delimeter);
			
			//fprintf(WIFI,"Switch: %u Response: %u\n",switchNumber, success);

			if(success == 1){
				if(switchControlMode == SWITCH_ON){
					wirelessSwitchStatus[switchNumber-1] = 1;
				}else if(switchControlMode == SWITCH_OFF){
					wirelessSwitchStatus[switchNumber-1] = 0;
				}else if(switchControlMode == SWITCH_TOGGLE){
					wirelessSwitchStatus[switchNumber-1] = 0;
				}
			}
			//$MQSWCR;switchNumber;switchControlMode;success;uuid
			fprintf(ESP8266,"$MQSWCR;%u;W;%c;%u;%s*", switchNumber, switchControlMode, success, uuid);
			write_eeprom(saveWirelessSwitchStatusAddress[switchnumber-1],wirelessSwitchStatus[switchnumber-1]);
		
	
			ptr = NULL;
			Clear_Receive();
			return;
		}

	////$MQSWSTATUSSN;switchType;switchNumber;status*
		if(strcmp(command, nofifySlaveStatus) == 0){
			ptr = strtok(0,delimeter);
			switchType = ptr[0];
			switchNumber = atoi(strtok(0,delimeter));
			switchStatus = atoi(strtok(0,delimeter));

			if(switchType == 'L'){
				livoloSwitchStatus[switchNumber-1] = switchStatus;
				write_eeprom(saveLivoloSwitchStatusAddress[switchnumber-1],livoloSwitchStatus[switchnumber-1]);
			}else if(switchType == 'W'){
				wirelessSwitchStatus[switchNumber-1] = switchStatus;
				write_eeprom(saveWirelessSwitchStatusAddress[switchnumber-1],wirelessSwitchStatus[switchnumber-1]);
			}
	
			//fprintf(WIFI,"Switch Notify: %u Status: %u\n",switchNumber, switchStatus[switchNumber-1]);
			//$MQSWCR;switchNumber;switchControlMode;success;uuid
			fprintf(ESP8266,"$MQSWSTATUSN;%c;%u;%u*", switchType, switchNumber, switchStatus);
			
	
			ptr = NULL;
			Clear_Receive();
			return;
		}

 

	}

	Clear_Receive();

}
//========================================================================================================