
#define INTS_PER_SECOND 38 //Processor and clock specific

int8 int_count=0; 
int8 data = 0;
int WaitingTimer = 0;

//=======================================================================================
#int_rtcc 
clock_isr() { 

   //this block maintains timer variables with a resolution of a ~ 1 s 
   if(--int_count==0) 
   { 
      int_count=INTS_PER_SECOND; //reset 

      if(data == 0) 
      { 
         data = 1; 
		 //output_high(LED);
      } 
      else 
      {
			data = 0; 


	  	for(int i = 0; i < sizeof(waitingSlaveCounter); i++){
			if(waitingSlaveCounter[i] > 0){
				waitingSlaveCounter[i]++;
			}

			if(waitingSlaveCounter[i] > 3){
				fprintf(ESP8266,"$MQSWERR;%u;%s*",i+1, uuids[i]);
				waitingSlaveCounter[i] = 0;
			}
		}

			Request_Update_Info++;
			if(Request_Update_Info == 5)
			{
				if(MQTTConnect_Flag)
				{
					fprintf(ESP8266,"$BOXMQTT*");
				}
			}

			if(Request_Update_Info == 7)
			{
				Request_Update_Info = 0;

				if(UpdateRTC_flag)
				{
					//$BOXRTC*
					fprintf(ESP8266,"$BOXRTC*");
				}
			}

	  }
	



	}

}

//========================================================================================*/