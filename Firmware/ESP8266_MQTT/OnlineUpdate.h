// Remote site information
const char http_site[] = "ofofficial.dyndns.org";
const int http_port = 80;

long number_of_colon = 0;
long buffer_count = 0;
long ACK_Count = 0;
long Fail_Count = 0;

bool UploadSuccess_Flag = false;

//================================================================================================
 // Perform an HTTP GET request to a remote page
bool getPage() { 
  
  // Attempt to make a connection to the remote server
  if ( !wifiClient.connect(http_site, http_port) ) {
    return false;
  }

  String dir = "/homecontrol/" + fwVersion + "/";
  Serial.println("Downloading " + dir + "\n");
  
  // Make an HTTP GET request
  wifiClient.println("GET " + dir + "HomeCtrlMQTT_mian.hex HTTP/1.1");
  wifiClient.print("Host: ");
  wifiClient.println(http_site);
  wifiClient.println("Connection: close");
  wifiClient.println();
  
  return true;
}

//================================================================================================
void OnlineUpdateFirmware()
 {

    int Blink_Count = 0;
    
    if(UPDATE_FIRMWARE_FLAG)
    {
            WiFiTimer_Cnt = 0;
      
            Serial.println("Starting Upload Firmware ===> MCU");
            // Attempt to connect to website
            if ( !getPage() ) {
              Serial.println("GET request failed");
            }

            number_of_colon = 0;
            ACK_Count = 0;
            DoUpdateOnline = true;
            inputString = "";
            
            while(DoUpdateOnline)
            {
                  // If there are incoming bytes, print them
                  if ( wifiClient.available() ) 
                  {
                        char c = wifiClient.read();
                        //Serial.print(SourceChar);
                        if(c == ':')
                        {
                           number_of_colon++;
                        }
                        if(number_of_colon >= 13)
                        { 
                              if(c != '\r' && c != ';')
                              {
                                 inputString += c;
                              }
                              if(c == '\n'){
                                 SoftSerial.print(inputString);
                                 int ACK = SoftSerial.read();
                                 Serial.println(ACK); 
                                 Serial.println(" , "); 
                                 if(ACK == 6)     //Receive ACK
                                 {
                                    ACK_Count++;
                                    Blink_Count++;
                                    UploadSuccess_Flag = true;
                                    Serial.println(ACK_Count); 
                                 }
                                 else if(ACK == 21)
                                 {
                                    DoUpdateOnline = false;
                                    UPDATE_FIRMWARE_FLAG = true;      //ReProgram Again
                                 }
                                 else if(ACK == -1)     //No Data Coming
                                 {  
                                      MQTTclient.publish(publishMQTTtopic, "Upload Firmware Fail !!!!");
                                 }

                                 delay(10);                        
                                 inputString = "";

                                 if(Blink_Count > 5)
                                 {
                                       digitalWrite(Connection, HIGH);
                                 }
                                 if(Blink_Count > 10)
                                 {
                                       Blink_Count = 0;
                                       digitalWrite(Connection, LOW);
                                 }
                              }
                        }
                        
                        if(c == ';')
                        {
                          DoUpdateOnline = false;
                        }

//                          char c = client.read();
//                          Serial.print(c);
                  }
                  
                  // If the server has disconnected, stop the client and WiFi
                  if ( !wifiClient.connected() ) {    
                      wifiClient.stop();       
                      DoUpdateOnline = false;
                  }
            }

            if(UploadSuccess_Flag)
            {
                UPDATE_FIRMWARE_FLAG = false;
                Serial.println("Upload Firmware Success ++++"); 
                MQTTclient.publish(publishMQTTtopic, "Upload Firmware Success ++++");      //Cannot Publish because MCU Reset ESP8266
            }
            else{
                UPDATE_FIRMWARE_FLAG = false;      //ReProgram Again
                Serial.println("Upload Firmware Fail !!!!"); 
                MQTTclient.publish(publishMQTTtopic, "Upload Firmware Fail !!!!");
            }

//            Serial.println();  
//            Serial.println("Update Firmware Finished !!!"); 
            Serial.print("number_of_colon = "); 
            Serial.println(number_of_colon - 13);
            Serial.print("ACK_Count = "); 
            Serial.println(ACK_Count);
            Serial.print("Fail_Count = "); 
            Serial.println(Fail_Count);

            wifiClient.stop();     

    }
 }
//================================================================================================



