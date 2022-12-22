//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-==-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//                             BASE STATION FUNCTIONALITY
//=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//This script will allow a GPS module to function as a base station in a DGNSS System
  //Enter ECEF coordinates gathered from setupBaseStation script
  //Enable the output of RTCM messages
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//                       Libraries + Relevant Definitions
//
#include <WiFi.h> 
#include "secrets.h" //Modify secrets.h to suit your wifi network - i.e. Wifi Id and network password
WiFiClient ntripCaster; 

//When connecting ZED via I2C
#include <Wire.h>

#include <SparkFun_u-blox_GNSS_Arduino_Library.h> 
SFE_UBLOX_GNSS baseStation;

//===========================================================================================================
//                         Global Variables   

int RTCMCount = 0;

long lastSentRTCM_ms = 0;           //Time of last data pushed to socket
int maxTimeBeforeHangup_ms = 10000; //If we fail to get a complete RTCM frame after 10s, then disconnect from caster

uint32_t serverBytesSent = 0; //Just a running total
long lastReport_ms = 0;       //Time of last report of bytes sent

//Manually Enter From PPP output
  int ecefX = -392753937;
  int ecefX_HP = -22;
  int ecefY = 346187061;
  int ecefY_HP = 49;
  int ecefZ = -363072508;
  int ecefZ_HP = -98;
//Verification Variables
  bool success = true ;
  bool response = true;
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//                            SETTING UP MODULE AS A BASE STATION                               
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void setup(){
  //Initialise Serial Terminal - for debugging purposes
  Serial.begin(115200);
  while (!Serial); //Wait for user to open terminal
  Serial.println(F("---------Creating DGNSS Base Station----------"));


  //-----UNCOMMENT WHEN USING NTRIP PROTOCOL-----
    Serial.print("Connecting to local WiFi");
    Serial1.print("Connecting to local WiFi");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      Serial.print(".");
      Serial1.print(".");
    }

    Serial.print("\nWiFi connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial1.print("\nWiFi connected with IP: ");
    Serial1.println(WiFi.localIP());
  //-----------------------------------------------
  //Begin Transmission of RTCM data over I2C - gets Data from ZED to ARD
  Wire.begin();
  Wire.setClock(400000); 
  if (baseStation.begin() == false) //Check for Valid I2C Wiring 
  {
    Serial.println(F("Invalid Wiring between ZED-F9P and Controller"));
    while (1);
  }
  
  baseStation.setI2COutput(COM_TYPE_UBX | COM_TYPE_NMEA | COM_TYPE_RTCM3); // Ensure RTCM3 is enabled --> output RTCM messages from ZED to controller
  baseStation.saveConfigSelective(VAL_CFG_SUBSEC_IOPORT); //Save the current settings to flash and BBR --> reinstate current configuration upon bootup
  //Set PPP of base station using ECEF coordinates from baseStationSetup script.
  success &= baseStation.setStaticPosition(ecefX, ecefX_HP, ecefY, ecefY_HP, ecefZ, ecefZ_HP);
  if (!success) Serial.println(F("At least one call to setStaticPosition failed!"));
  baseStation.setNavigationFrequency(1); //Set output in Hz. RTCM rarely benefits from >1Hz. (i.e. 1 RTCM message per second)


  //---------------------MESSAGE CONFIGURATION-----------------------------

  //Enable RTCM Messages-------------------------------------------------------
    //RTCM output over UART2
    //RTCM output over I2C (i.e. To ARD)
    response &= baseStation.enableRTCMmessage(UBX_RTCM_1005, COM_PORT_I2C, 1); //Enable message 1005 to output through I2C port, message every second ----|
    response &= baseStation.enableRTCMmessage(UBX_RTCM_1074, COM_PORT_I2C, 1);                                                                            
    response &= baseStation.enableRTCMmessage(UBX_RTCM_1084, COM_PORT_I2C, 1);
    response &= baseStation.enableRTCMmessage(UBX_RTCM_1094, COM_PORT_I2C, 1);
    response &= baseStation.enableRTCMmessage(UBX_RTCM_1124, COM_PORT_I2C, 1);
    response &= baseStation.enableRTCMmessage(UBX_RTCM_1230, COM_PORT_I2C, 10); //Enable message every 10 seconds
    
    //CHECK Message Configuration has Succeeded/
    if (response1 == true){
      Serial.println(F("RTCM messages enabled"));
    }
    else{
      Serial.println(F("!!RTCM failed to enable!!"));
      while (1); //Freeze
    }
    if (response2 == true) {
      Serial.println(F("RTCM messages enabled"));
    }
    else{
      Serial.println(F("!!RTCM failed to enable!!"));
      while (1); //Freeze
    }

  //------CONFIRMATION OF BASE STATION SETUP------

  int fix = baseStation.getFixType();
  if (fix != 5){
    Serial.println(F("BASE STATION COULD NOT ENTER FIX TYPE 5"));
    while(1); //Freeze
  } 

  Serial.println(F("Base Station Available To Transmit!"));


  while (Serial.available()) Serial.read(); //Clear any latent chars in serial buffer
  Serial.println(F("PRESS ANY KEY TO BEGIN TRANSMITTING RTCM DATA"));
  while (Serial.available() == 0) ; //Wait for user to press a key
  Serial.println("=================NOW TRANSMITTING==============");

}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//                                  TRANSMISSION OF RTCM DATA TO ROVER
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
              
void loop(){
// ----------------NTRIP PROTOCOL CODE----------------------------------------------------------
  if (Serial.available())
    beginServing();

  Serial.println(F("Press any key to start serving"));

  delay(1000);
    
}

void beginServing()
{
  Serial.println("Begin transmitting to caster. Press any key to stop");
  delay(10); //Wait for any serial to arrive
  //WANT TO CHANGE THIS A DONT WANT TO PRINT ALL DATA TO THE ARDUINO SERIAL TERMINAL//
  while (Serial.available())
    Serial.read(); //Flush

//WANT TO CONNECT TO THE NTRIP CASTER BEFORE FIRST SERIAL MESSAGE ARRIVES  
  while (Serial.available() == 0)
  {
    //Connect if we are not already
    if (ntripCaster.connected() == false)
    {
      Serial.printf("Opening socket to %s\n", casterHost);

      if (ntripCaster.connect(casterHost, casterPort) == true) //Attempt connection
      {
        Serial.printf("Connected to %s:%d\n", casterHost, casterPort);

        const int SERVER_BUFFER_SIZE = 512; //Look into wh
        char serverRequest[SERVER_BUFFER_SIZE];

        snprintf(serverRequest,
                 SERVER_BUFFER_SIZE,
                 "SOURCE %s /%s\r\nSource-Agent: NTRIP SparkFun u-blox Server v1.0\r\n\r\n",
                 mountPointPW, mountPoint);

        Serial.println(F("Sending server request:"));
        Serial.println(serverRequest);
        ntripCaster.write(serverRequest, strlen(serverRequest));

        //Wait for response
        unsigned long timeout = millis();
        while (ntripCaster.available() == 0)
        {
          if (millis() - timeout > 5000)
          {
            Serial.println("Caster timed out!");
            ntripCaster.stop();
            return;
          }
          delay(10);
        }

        //Check reply
        bool connectionSuccess = false;
        char response[512];
        int responseSpot = 0;
        while (ntripCaster.available())
        {
          response[responseSpot++] = ntripCaster.read();
          if (strstr(response, "200") > 0) //Look for 'ICY 200 OK'
            connectionSuccess = true;
          if (responseSpot == 512 - 1)
            break;
        }
        response[responseSpot] = '\0';

        if (connectionSuccess == false)
        {
          Serial.printf("Failed to connect to Caster: %s", response);
          return;
        }
      } //End attempt to connect
      else
      {
        Serial.println("Connection to host failed");
        return;
      }
    } //End connected == false

    if (ntripCaster.connected() == true)
    {
      delay(10);
      while (Serial.available())
        Serial.read(); //Flush any endlines or carriage returns

      lastReport_ms = millis();
      lastSentRTCM_ms = millis();

      //This is the main sending loop. We scan for new ublox data but processRTCM() is where the data actually gets sent out.
      while (1)
      {
        if (Serial.available())
          break;

        baseStation.checkUblox(); //See if new data is available. Process bytes as they come in.

        //=-=-=-=-=-=-=-=-=CLOSING INTERNET PORT=-=--=-=-=-=-=-=-=-=-=-=-=-=-
        //Close socket if we don't have new data for 10s
        //RTK2Go will ban your IP address if you abuse it. See http://www.rtk2go.com/how-to-get-your-ip-banned/
        //So let's not leave the socket open/hanging without data
        if (millis() - lastSentRTCM_ms > maxTimeBeforeHangup_ms)
        {
          Serial.println("RTCM timeout. Disconnecting...");
          ntripCaster.stop();
          return;
        }

        delay(10);

        //Report some statistics every 250 //IS 250 seconds wuick enough in terms of rover speed
        if (millis() - lastReport_ms > 250)
        {
          lastReport_ms += 250;
          Serial.printf("Total sent: %d\n", serverBytesSent);
        }
      }
    }

    delay(10);
  }

  Serial.println("User pressed a key");
  Serial.println("Disconnecting...");
  ntripCaster.stop();

  delay(10);
  while (Serial.available())
    Serial.read(); //Flush any endlines or carriage returns

}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//    PROCESSRTCM()
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  This function is called within the checkUblox function
//  Alter its functionality depending on how RTCM data is to be transmitted
//  Current configured to output RTCM over serial radio extension.
void SFE_UBLOX_GNSS::processRTCM(uint8_t incoming)
{
  
  //SEND RTCM DATA USING NTRIP--------------------------------
  if (ntripCaster.connected() == true){
    ntripCaster.write(incoming); //Send this byte to socket
    serverBytesSent++;
    lastSentRTCM_ms = millis();
  }
  //-----------------------------------------------------------
  //Ouput RTCM data to Serial Terminal 4 debugging--------------
    if (baseStation.rtcmFrameCounter % 16 == 0) Serial.println();
  Serial.print(F(" "));
  if (incoming < 0x10) Serial.print(F("0"));
  Serial.print(incoming, HEX);
  //------------------------------------------------------------
}

////=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

