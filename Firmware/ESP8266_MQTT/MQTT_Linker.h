//===================================================================================================================================================================================
// Function sendButton(unsigned int remoteID, byte keycode)
// Livilo remoteID 6400
// keycode  7 bit
// Keycodes Livolo remote: #1: 0, #2: 96, #3: 120, #4: 24, #5: 80, #6: 48, #7: 108, #8: 12, #9: 72; #10: 40, #OFF: 106
void controlLivolo(int livoloKey) {
  Serial.print(">>");
  Serial.println(livoloKey);
  switch (livoloKey) {
    case 1:
      livolo.sendButton(6400, 0);
      Serial.println("key1");
      break;
    case 2:
      livolo.sendButton(6400, 96);
      Serial.println("key2");
      break;
    case 3:
      livolo.sendButton(6400, 120);
      Serial.println("key3");
      break;

    case 4:
      livolo.sendButton(6400, 24);
      Serial.println("key4");
      break;

    case 5:
      livolo.sendButton(6400, 80);
      Serial.println("key5");
      break;

    case 6:
      livolo.sendButton(6400, 48);
      Serial.println("key6");
      break;

    case 7:
      livolo.sendButton(6400, 108);
      Serial.println("key7");
      break;

    case 8:
      livolo.sendButton(6400, 12);
      Serial.println("key8");
      break;

    case 9:
      livolo.sendButton(6400, 72);
      Serial.println("key9");
      break;

    case 10:
      livolo.sendButton(6400, 40);
      Serial.println("key10");
      break;
      
    case 99:  //OFF All
      livolo.sendButton(6400, 106);
      //delay(10);
      Serial.println("All Off");
      break;
  }

//  MQTTclient.publish(MQTTtopic, "WireLess Switch Active");
}
//===================================================================================================================================================================================


void MQTT_Receive(char getRecieveString[])
{



    //$MQSWCL;Key
  if(getRecieveString[0] = '$' && getRecieveString[1] == 'M' && getRecieveString[2] == 'Q' && getRecieveString[3] == 'S' && getRecieveString[4] == 'W' && getRecieveString[5] == 'C' && getRecieveString[6] == 'L' )
  {
      String mcuString = String(getRecieveString);
      String command = mcuString.substring(0, mcuString.indexOf(';'));
      mcuString = mcuString.substring(mcuString.indexOf(';')+1);


      int livoloKey = mcuString.substring(0, mcuString.indexOf('*')).toInt();
      controlLivolo(livoloKey);
  }else  if(getRecieveString[0] = '$' && getRecieveString[1] == 'B' && getRecieveString[2] == 'O' && getRecieveString[3] == 'X' && getRecieveString[4] == 'R' && getRecieveString[5] == 'T' && getRecieveString[6] == 'C' )  {
    //$BOXRTC*
      Serial.println("Request Update RTC from Internet"); 
      delay(500);
      
      getTime();
  }else if(getRecieveString[0] = '$' && getRecieveString[1] == 'B' && getRecieveString[2] == 'O' && getRecieveString[3] == 'X' && getRecieveString[4] == 'M' && getRecieveString[5] == 'Q' && getRecieveString[6] == 'T' && getRecieveString[7] == 'T'){
    //$BOXMQTT*
      Serial.println("MQTT Request to check Connection status"); 
      //delay(500);
      if(MQTT_Connect_Flag)
      {
          Serial.println("Sending: $ESP8266MQSTATUS;SUCCESS*"); 
          delay(500);
     
          
          SoftSerial.print("$ESP8266MQSTATUS;SUCCESS*");
//          SoftSerial.print(WiFi.localIP());
//          SoftSerial.print("*");
          delay(500);
      }
      else{
          SoftSerial.print("$ESP8266MQSTATUS;FAILURE*");
      }
  } else if(getRecieveString[0] = '$') {
        
        MQTTclient.publish(publishMQTTtopic, getRecieveString);
 
  }





  
  

}
//======================================================================================================================================================

