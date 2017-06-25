#include <FS.h>                   //this needs to be first, or it all crashes and burns...

#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager

#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson'

#include <PubSubClient.h>

//define your default values here, if there are different values in config.json, they are overwritten.
//length should be max size + 1
char mqtt_server[40] = "openfoggps.com";
char mqtt_port[6] = "1883";
char openfog_token[33] = "Your Username";
//default custom static IP
char static_ip[16] = "192.168.1.55";
char static_gw[16] = "192.168.1.1";
char static_sn[16] = "255.255.255.0";
char line_token[44] = "Line Token ( Option )";
char publishMQTTtopic[50];
char subscribeMQTTtopic[50];

WiFiClient wifiClient;
void callback(char* topic, byte* payload, unsigned int length);
PubSubClient MQTTclient(mqtt_server, atoi(mqtt_port), callback, wifiClient);

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
bool UPDATE_FIRMWARE_FLAG = false;

String fwVersion;


#include <SoftwareSerial.h>
SoftwareSerial SoftSerial(14, 12, false, 256);  //D14 = RX, D12 = TX

#include <livolo.h>
Livolo livolo(5);     //Use 433MHz at GPIO5
char keyID = 0;

String DateTime;
int Date;
int Month;
int Year;
int DOW;
int Hour;
int Minute;
int Second;

bool UsetoReadState = false;

const int Connection = 16;

long WiFiTimer_Cnt = 0;

String IR_Head = "";
String IR_Data = "";

String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
word counter_read;
char RecieveString[100];

bool Recieve_Flag = false;
char RecieveData[100];

bool MQTT_Connect_Flag = false;
bool DoUpdateOnline = true;

//แจ้งเตือนมีการบุกรุก
String message = "%E0%B9%81%E0%B8%88%E0%B9%89%E0%B8%87%E0%B9%80%E0%B8%95%E0%B8%B7%E0%B8%AD%E0%B8%99%E0%B8%A1%E0%B8%B5%E0%B8%81%E0%B8%B2%E0%B8%A3%E0%B8%9A%E0%B8%B8%E0%B8%81%E0%B8%A3%E0%B8%B8%E0%B8%81";

//แจ้งเตือนอุณหภูมิสูงเกินกำหนด
String message2 = "%E0%B9%81%E0%B8%88%E0%B9%89%E0%B8%87%E0%B9%80%E0%B8%95%E0%B8%B7%E0%B8%AD%E0%B8%99%E0%B8%AD%E0%B8%B8%E0%B8%93%E0%B8%AB%E0%B8%A0%E0%B8%B9%E0%B8%A1%E0%B8%B4%E0%B8%AA%E0%B8%B9%E0%B8%87%E0%B9%80%E0%B8%81%E0%B8%B4%E0%B8%99%E0%B8%81%E0%B8%B3%E0%B8%AB%E0%B8%99%E0%B8%94";

//แจ้งเตือนแรงดันแบตเตอรี่ต่ำ
String message3 = "%E0%B9%81%E0%B8%88%E0%B9%89%E0%B8%87%E0%B9%80%E0%B8%95%E0%B8%B7%E0%B8%AD%E0%B8%99%E0%B9%81%E0%B8%A3%E0%B8%87%E0%B8%94%E0%B8%B1%E0%B8%99%E0%B9%81%E0%B8%9A%E0%B8%95%E0%B9%80%E0%B8%95%E0%B8%AD%E0%B8%A3%E0%B8%B5%E0%B9%88%E0%B8%95%E0%B9%88%E0%B8%B3";

//แจ้งเตือน

extern "C" {
#include "MQTT_Linker.h"
#include "OnlineUpdate.h"
}

//=================================================================================================
void setup() {

        // put your setup code here, to run once:

//        pinMode(FactoryReset, INPUT);
        
        pinMode(Connection, OUTPUT);
        digitalWrite(Connection, HIGH);
  
        Serial.begin(115200);
        Serial.println();

        SoftSerial.begin(57600);

//        EEPROM.begin(512);
        
        //clean FS, for testing
        //SPIFFS.format();
      
        //read configuration from FS json
        Serial.println("mounting FS...");
      
        if (SPIFFS.begin()) {
          Serial.println("mounted file system");
          if (SPIFFS.exists("/config.json")) {
            //file exists, reading and loading
            Serial.println("reading config file");
            File configFile = SPIFFS.open("/config.json", "r");
            if (configFile) {
              Serial.println("opened config file");
              size_t size = configFile.size();
              // Allocate a buffer to store contents of the file.
              std::unique_ptr<char[]> buf(new char[size]);
      
              configFile.readBytes(buf.get(), size);
              DynamicJsonBuffer jsonBuffer;
              JsonObject& json = jsonBuffer.parseObject(buf.get());
              json.printTo(Serial);
              if (json.success()) {
                Serial.println("\nparsed json");
      
                strcpy(mqtt_server, json["mqtt_server"]);
                strcpy(mqtt_port, json["mqtt_port"]);
                strcpy(openfog_token, json["openfog_token"]);
                strcpy(line_token, json["line_token"]);
      
                if(json["ip"]) {
                  Serial.println("setting custom ip from config");
                  //static_ip = json["ip"];
                  strcpy(static_ip, json["ip"]);
                  strcpy(static_gw, json["gateway"]);
                  strcpy(static_sn, json["subnet"]);
                  //strcat(static_ip, json["ip"]);
                  //static_gw = json["gateway"];
                  //static_sn = json["subnet"];
                  Serial.println(static_ip);
      /*            Serial.println("converting ip");
                  IPAddress ip = ipFromCharArray(static_ip);
                  Serial.println(ip);*/
                } else {
                  Serial.println("no custom ip in config");
                }
              } else {
                Serial.println("failed to load json config");
              }
            }
          }
        } else {
          Serial.println("failed to mount FS");
        }
        //end read
        Serial.println(static_ip);
        Serial.println(openfog_token);
        Serial.println(mqtt_server);
        Serial.println(line_token);
      
      
        // The extra parameters to be configured (can be either global or just in the setup)
        // After connecting, parameter.getValue() will get you the configured value
        // id/name placeholder/prompt default length
        WiFiManagerParameter custom_mqtt_server("server", "MQTT Server", mqtt_server, 40);
        WiFiManagerParameter custom_mqtt_port("port", "MQTT Port", mqtt_port, 5);
        WiFiManagerParameter custom_openfog_token("openfog", "Your username", openfog_token, 34);
        WiFiManagerParameter custom_line_token("line token", "Line Token ( Option )", line_token, 43);
      
        //WiFiManager
        //Local intialization. Once its business is done, there is no need to keep it around
        WiFiManager wifiManager;
      
        //set config save notify callback
        wifiManager.setSaveConfigCallback(saveConfigCallback);
      
        //set static ip
        IPAddress _ip,_gw,_sn;
        _ip.fromString(static_ip);
        _gw.fromString(static_gw);
        _sn.fromString(static_sn);
      
        wifiManager.setSTAStaticIPConfig(_ip, _gw, _sn);
        
        //add all your parameters here
        wifiManager.addParameter(&custom_mqtt_server);
        wifiManager.addParameter(&custom_mqtt_port);
        wifiManager.addParameter(&custom_openfog_token);
        //wifiManager.addParameter(&custom_line_token);                       // Fixed
      
        //reset settings - for testing
        //wifiManager.resetSettings();
      
        //set minimu quality of signal so it ignores AP's under that quality
        //defaults to 8%
        wifiManager.setMinimumSignalQuality();
        
        //sets timeout until configuration portal gets turned off
        //useful to make it all retry or go to sleep
        //in seconds
        //wifiManager.setTimeout(120);
      
        //fetches ssid and pass and tries to connect
        //if it does not connect it starts an access point with the specified name
        //here  "AutoConnectAP"
        //and goes into a blocking loop awaiting configuration
        if (!wifiManager.autoConnect("HomeControlAP", "password")) {
        
          Serial.println("failed to connect and hit timeout");
          delay(3000);
          //reset and try again, or maybe put it to deep sleep
          ESP.reset();
          delay(5000);
        }
      
        //if you get here you have connected to the WiFi
        Serial.println("connected...");
      
        //read updated parameters
        strcpy(mqtt_server, custom_mqtt_server.getValue());
        strcpy(mqtt_port, custom_mqtt_port.getValue());
        strcpy(openfog_token, custom_openfog_token.getValue());
        //strcpy(line_token, custom_line_token.getValue());         // Fixed
      
        //save the custom parameters to FS
        if (shouldSaveConfig) {
              Serial.println("saving config");
              DynamicJsonBuffer jsonBuffer;
              JsonObject& json = jsonBuffer.createObject();
              json["mqtt_server"] = mqtt_server;
              json["mqtt_port"] = mqtt_port;
              json["openfog_token"] = openfog_token;
              json["line_token"] = line_token;
          
              json["ip"] = WiFi.localIP().toString();
              json["gateway"] = WiFi.gatewayIP().toString();
              json["subnet"] = WiFi.subnetMask().toString();
          
              File configFile = SPIFFS.open("/config.json", "w");
              if (!configFile) {
                Serial.println("failed to open config file for writing");
              }
          
              json.prettyPrintTo(Serial);
              json.printTo(configFile);
              configFile.close();
              //end save
        }
      
        Serial.println("local ip");
        Serial.println(WiFi.localIP());
        Serial.println(WiFi.gatewayIP());
        Serial.println(WiFi.subnetMask());



  strcpy(subscribeMQTTtopic, openfog_token);
  //strcat(subscribeMQTTtopic, "/F1DC132D34B6AAFM");
  strcat(subscribeMQTTtopic, "/F1DC132D34B6AAF");


  strcpy(publishMQTTtopic, openfog_token);
  //strcat(publishMQTTtopic, "/F1DC132D34B6AAFB");
  strcat(publishMQTTtopic, "/F1DC132D34B6AAF");

  Serial.print("subscribeMQTTtopic =");
  Serial.println(subscribeMQTTtopic);

  Serial.print("publishMQTTtopic =");
  Serial.println(publishMQTTtopic);

  reconnect();

  //wait a bit before starting the main loop
  delay(1000);

  //getTime();  //Sync Time to DS1302

  delay(1000);

  digitalWrite(Connection, LOW);
}

//Main Loop
//============================================================================
void loop() {

//  // put your main code here, to run repeatedly:
//  //reconnect if connection is lost
//  if (!MQTTclient.connected() && WiFi.status() == 3) {
//    reconnect();
//  }
//
//  //maintain MQTT connection
//  MQTTclient.loop();
//
//  SoftSerialRead();
//
//  if (stringComplete) {
//    Serial.print("Complete Recieve: ");
//    Serial.println(RecieveString);
//
//    MQTT_Receive(RecieveString);
//
//    for (int i = 0; i < 50; i++)
//    {
//      RecieveString[i] = 0xFF;
//    }
//
//    // clear the string:
//    inputString = "";
//    stringComplete = false;
//  }
//
//  //  int j = 0;
//  //  while (SoftSerial.available() > 0) {
//  //    while(SoftSerial.read() != "\n")
//  //    {
//  //      RecieveData[j] = SoftSerial.read();
//  //      j++;
//  //      Recieve_Flag = true;
//  //    }
//  //  }
//  //
//  //  if(Recieve_Flag)
//  //  {
//  //      Serial.print("j = ");
//  //      Serial.println(j);
//  //
//  //      Serial.write(RecieveData);
//  //      //client.publish(MQTTtopic, RecieveData);
//  //      if(RecieveData[0] == 'A' && RecieveData[1] == 'L' && RecieveData[2] == 'E' && RecieveData[3] == 'R' && RecieveData[4] == 'T' && RecieveData[5] == '1')
//  //      {
//  //        Serial.println("\tHomeControl Alert!!!");
//  //        Line_Notify(message);
//  //      }
//  //
//  //      if(RecieveData[0] == 'P' && RecieveData[1] == 'U' && RecieveData[2] == 'B')
//  //      {
//  //        client.publish(MQTTtopic, RecieveData);
//  //      }
//  //
//  //      ClsRecieveData();
//  //
//  //      Recieve_Flag = false;
//  //  }
//  //  else{
//  //
//  //
//  //  }
//
//  //  while (Serial.available() > 0) {
//  //      SoftSerial.write(Serial.read());
//  //  }
//  OnlineUpdateFirmware();
//  CheckWiFiConnection();
//
//  //MUST delay to allow ESP8266 WIFI functions to run
//  delay(10);


                // put your main code here, to run repeatedly:
                OnlineUpdateFirmware();
   
                //reconnect if connection is lost
                if (!MQTTclient.connected() && WiFi.status() == 3) {reconnect();}
                
                //maintain MQTT connection
                MQTTclient.loop();
            
                        SoftSerialRead();
                        if (stringComplete) {
                             MQTT_Receive(RecieveString);
                            
//                            for(int i = 0; i< 50; i++)
//                            {
//                              RecieveString[i] = 0xFF;
//                            }
                            
                            stringComplete = false;
                        }
            
            
              CheckWiFiConnection();
            
              //MUST delay to allow ESP8266 WIFI functions to run
              delay(10); 
 

//  SoftSerial.print("$ESP8266MQSTATUS;SUCCESS*");
//  delay(5000); 

}
//==========================================================================
//End Main Loop

void SoftSerialRead()
{
  while (SoftSerial.available()) {
    // get the new byte:
    char inChar = (char)SoftSerial.read();
    if (inChar == '$')
    {
      counter_read = 0;
      RecieveString[counter_read] = '$';
    }
    RecieveString[counter_read] = inChar;
    counter_read++;

    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '*') {
      //RecieveString[counter_read] = '\n';
      RecieveString[counter_read] = 0;
      stringComplete = true;
    }
  }
}
//==========================================================================

void ClsRecieveData()
{
  for (int k = 0; k <= 100; k++)
  {
    RecieveData[k] = 0xFF;
  }
}

//==========================================================================

String getTime() {
  WiFiClient client;
  //  while (!!!client.connect("openfog.net", 80))

  while (!!!client.connect("google.com", 80)) {
    Serial.println("connection failed, retrying...");
  }

  client.print("HEAD / HTTP/1.1\r\n\r\n");

  while (!!!client.available()) {
    yield();
  }

  while (client.available()) {
    if (client.read() == '\n') {
      if (client.read() == 'D') {
        if (client.read() == 'a') {
          if (client.read() == 't') {
            if (client.read() == 'e') {
              if (client.read() == ':') {
                client.read();
                DateTime = client.readStringUntil('\r');
                client.stop();

                Serial.println(DateTime);
                if (DateTime[0] == 'S' && DateTime[1] == 'u')
                {
                  DOW = 1;
                }

                if (DateTime[0] == 'M' && DateTime[1] == 'o')
                {
                  DOW = 2;
                }

                if (DateTime[0] == 'T' && DateTime[1] == 'u')
                {
                  DOW = 3;
                }

                if (DateTime[0] == 'W' && DateTime[1] == 'e')
                {
                  DOW = 4;
                }

                if (DateTime[0] == 'T' && DateTime[1] == 'h')
                {
                  DOW = 5;
                }

                if (DateTime[0] == 'F' && DateTime[1] == 'r')
                {
                  DOW = 6;
                }

                if (DateTime[0] == 'S' && DateTime[1] == 'a')
                {
                  DOW = 7;
                }

                Serial.print("Date Of Week : ");
                Serial.println(DOW);

                if (DateTime[6] != ' ')
                {
                  Date = ((DateTime[5] - 48) * 10) + ( DateTime[6] - 48 );
                  Serial.print("Date : ");
                  Serial.println(Date);

                  if (DateTime[8] == 'J' && DateTime[9] == 'a' && DateTime[10] == 'n')
                  {
                    Month = 1;
                  }

                  if (DateTime[8] == 'F' && DateTime[9] == 'e' && DateTime[10] == 'b')
                  {
                    Month = 2;
                  }

                  if (DateTime[8] == 'M' && DateTime[9] == 'a' && DateTime[10] == 'r')
                  {
                    Month = 3;
                  }

                  if (DateTime[8] == 'A' && DateTime[9] == 'p' && DateTime[10] == 'r')
                  {
                    Month = 4;
                  }

                  if (DateTime[8] == 'M' && DateTime[9] == 'a' && DateTime[10] == 'y')
                  {
                    Month = 5;
                  }

                  if (DateTime[8] == 'J' && DateTime[9] == 'u' && DateTime[10] == 'n')
                  {
                    Month = 6;
                  }

                  if (DateTime[8] == 'J' && DateTime[9] == 'u' && DateTime[10] == 'l')
                  {
                    Month = 7;
                  }

                  if (DateTime[8] == 'A' && DateTime[9] == 'u' && DateTime[10] == 'g')
                  {
                    Month = 8;
                  }

                  if (DateTime[8] == 'S' && DateTime[9] == 'e' && DateTime[10] == 'p')
                  {
                    Month = 9;
                  }

                  if (DateTime[8] == 'O' && DateTime[9] == 'c' && DateTime[10] == 't')
                  {
                    Month = 10;
                  }

                  if (DateTime[8] == 'N' && DateTime[9] == 'o' && DateTime[10] == 'v')
                  {
                    Month = 11;
                  }

                  if (DateTime[8] == 'D' && DateTime[9] == 'e' && DateTime[10] == 'c')
                  {
                    Month = 12;
                  }

                  Serial.print("Month : ");
                  Serial.println(Month);

                  Year = ((DateTime[12] - 48) * 1000) + ((DateTime[13] - 48) * 100) + ((DateTime[14] - 48) * 10) + DateTime[15] - 48;
                  Serial.print("Year : ");
                  Serial.println(Year);

                }
                else {  //รอทดสอบว่าจะเกิด Case นี้เหรอเปล่า
                  Date = DateTime[5] - 48;
                }

                Hour = ((DateTime[17] - 48 ) * 10) + ( DateTime[18] - 48 );
                Hour = Hour + 7;
                switch (Hour)
                {
                  case 24: Hour = 0; break;
                  case 25: Hour = 1; break;
                  case 26: Hour = 2; break;
                  case 27: Hour = 3; break;
                  case 28: Hour = 4; break;
                  case 29: Hour = 5; break;
                  case 30: Hour = 6; break;
                  case 31: Hour = 7; break;
                }

                Serial.print("Hour : ");
                Serial.println(Hour);

                Minute = ((DateTime[20] - 48 ) * 10) + ( DateTime[21] - 48 );
                Serial.print("Minute : ");
                Serial.println(Minute);

                Second = ((DateTime[23] - 48 ) * 10) + ( DateTime[24] - 48 );
                Serial.print("Second : ");
                Serial.println(Second);

                //$XPORTSYNRTC:" + mDate + ":" + mMonth + ":" + mYear + ":" + mDOW + ":"+ mHour + ":"+ mMinute + ":" + mSec + ":*";
                SoftSerial.print("$MQTTPSYNRTC:");
                SoftSerial.print(Date);
                SoftSerial.print(":");
                SoftSerial.print(Month);
                SoftSerial.print(":");
                SoftSerial.print(Year);
                SoftSerial.print(":");
                SoftSerial.print(DOW);
                SoftSerial.print(":");
                SoftSerial.print(Hour);
                SoftSerial.print(":");
                SoftSerial.print(Minute);
                SoftSerial.print(":");
                SoftSerial.print(Second);
                SoftSerial.print(":**");

                return DateTime;
              }
            }
          }
        }
      }
    }
  }

}

//=========================================================================================================
void callback(char* topic, byte* payload, unsigned int length) {

  //convert topic to string to make it easier to work with
  String topicStr = topic;

  //Print out some debugging info

  Serial.print("Incomming command: ");
  
  String PayloadString = String((char *)payload);
  PayloadString = PayloadString.substring(0, PayloadString.indexOf('*')+1);
  Serial.println(PayloadString);




  // if(payload[0] == '$' && payload[1] == 'M' && payload[2] == 'Q' && payload[3] == 'T' && payload[4]=='T' && payload[5]=='Y' && payload[6]=='U'
  //&& payload[7] == 'P' && payload[9] == 'F' && payload[10] == 'W' && payload[11]=='*'){

  if (PayloadString.startsWith("$MQSYS")) {

    //$MQSYSUPFWWI
    //$MQSYSUPFWNN
    if (PayloadString.startsWith("$MQSYSUPFW")) {

      //parameter firmware version

      char fwchar [5] = {'0','0', '.', '0', '0'};
      fwchar[0] = payload[12];
      fwchar[1] = payload[13];
      fwchar[2] = payload[14];
      fwchar[3] = payload[15];
      fwchar[4] = payload[16];

      fwVersion = String(fwchar);

      //$MQTTYUPIFW*  //Upgrade Initial Firmware
      //if(payload[8] == 'I' ){
      if (payload[10] == 'I') {
        MQTTclient.publish(publishMQTTtopic, "+++ Upload Initail Firmware +++");
        delay(500);

        UPDATE_FIRMWARE_FLAG = true;
        delay(2000);                   //wait MCU Reboot
      }

      //$MQTTYUPNFW* //Upgrad New Firmware
      //if(payload[8] == 'N'){
      if (payload[10] == 'N') {
        MQTTclient.publish(publishMQTTtopic, "+++ Upload New Firmware Version +++");
        delay(500);

        SoftSerial.print("$MQSYSUPFW*");
        UPDATE_FIRMWARE_FLAG = true;
        delay(2000);                   //wait MCU Reboot
      }

    } else if (PayloadString.startsWith("$MQSYSW")) {

      MQTTclient.publish(publishMQTTtopic, "ConfigureWiFi");
      delay(500);

      //WiFiManager
      //                  //Local intialization. Once its business is done, there is no need to keep it around
      //                  WiFiManager wifiManager;

      WiFi.disconnect(true);

      delay(500);
      SoftSerial.print("$MQTTPRESTR*");

    } else if (PayloadString.startsWith("$MQSYSR") ) {

      delay(500);
      SoftSerial.print("$MQTTPRESTR*");
    } else if (PayloadString.startsWith("$MQSYST")) {
      MQTTclient.publish(publishMQTTtopic, "++ HomeControl Connection OK ++");
      delay(500);

    }

    payload = NULL;
  }else {
    SoftSerial.print(PayloadString);
  }



  WiFiTimer_Cnt = 0;
}


//=========================================================================================================
void CheckWiFiConnection()
{
  WiFiTimer_Cnt++;
  if (WiFiTimer_Cnt > 18000)   //Waiting For WiFi Signal For 3 Minute
  {
    WiFiTimer_Cnt = 0;
    if (WiFi.status() != WL_CONNECTED) {
      SoftSerial.print("$ESP8266WIFISTATUS;FAILURE*");
    }

    if (WiFi.status() == WL_CONNECTED)
    {
      SoftSerial.print("$ESP8266WIFISTATUS;SUCCESS*");
    }
  }

}

//=========================================================================================================
void reconnect() {

  //attempt to connect to the wifi if connection is lost
  if (WiFi.status() != WL_CONNECTED) {

    //loop while we wait for connection
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    //print out some more debug once connected
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }

  //make sure we are connected to WIFI before attemping to reconnect to MQTT
  if (WiFi.status() == WL_CONNECTED) {
    // Loop until we're reconnected to the MQTT server
    while (!MQTTclient.connected()) {
      Serial.print("Attempting MQTT connection...");

      // Generate client name based on MAC address and last 8 bits of microsecond counter
      String clientName;
      clientName += "esp8266-";
      uint8_t mac[6];
      WiFi.macAddress(mac);
      clientName += macToStr(mac);

      //if connected, subscribe to the topic(s) we want to be notified about
      if (MQTTclient.connect((char*) clientName.c_str())) {

        Serial.println("\tMQTT Connected  ");
        MQTTclient.subscribe(subscribeMQTTtopic);
        //SoftSerial.print("$ESP8266MQSTATUS;SUCCESS*");
        MQTT_Connect_Flag = true;
      }

      //otherwise print failed for debugging
      else {

        Serial.println("\tMQTT Connection Failed.");
        //SoftSerial.print("$ESP8266MQSTATUS;FAILURE*");
        MQTT_Connect_Flag = false;
        abort();
      }
    }
  }
}
//=========================================================================================================

//generate unique name from MAC addr
String macToStr(const uint8_t* mac) {

  String result;

  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);

    if (i < 5) {
      result += ':';
    }
  }

  return result;
}
//=========================================================================================================
//#define LINE_TOKEN "<LINE ACCESS TOKEN>"

void Line_Notify(String message) {
  WiFiClientSecure client;

  if (!client.connect("notify-api.line.me", 443)) {
    Serial.println("connection failed");
    return;
  }

  String req = "";
  String reqAuthen = "";
  String req2 = "";
  req += "POST /api/notify HTTP/1.1\r\n";
  req += "Host: notify-api.line.me\r\n";
  reqAuthen += "Authorization: Bearer "; // + LineAccessToken + "\r\n";
  req2 += "Cache-Control: no-cache\r\n";
  req2 += "User-Agent: ESP8266\r\n";
  req2 += "Content-Type: application/x-www-form-urlencoded\r\n";
  req2 += "Content-Length: " + String(String("message=" + message).length()) + "\r\n";
  req2 += "\r\n";
  req2 += "message=" + message;

  //==========================================
  //Debug
  Serial.print(req);
  Serial.print(reqAuthen);
  for (int i = 0; i <= 42; i++)
  {
    Serial.print(line_token[i]);
  }
  Serial.print("\r\n");
  Serial.print(req2);

  Serial.print("\r\n");
  //===========================================

  client.print(req);
  client.print(reqAuthen);
  for (int i = 0; i <= 42; i++)
  {
    client.print(line_token[i]);
  }
  client.print("\r\n");
  client.print(req2);

  delay(20);

  Serial.println("-------------");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
    Serial.println(line);
  }
  Serial.println("-------------");
}

