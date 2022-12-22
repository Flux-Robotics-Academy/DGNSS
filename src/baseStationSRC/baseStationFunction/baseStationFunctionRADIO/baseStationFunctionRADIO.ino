//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-==-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//                             BASE STATION FUNCTIONALITY
//=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//This script will allow a GPS module to function as a base station in a DGNSS System
  //Enter ECEF coordinates gathered from setupBaseStation script
  //Enable the output of RTCM messages
  //FUNTCIONALITY( NTRIP):
  //  - ZED-F9P communicates with ESP32 via I2C
  //  - ESP32 connect via wifi to NTRIP Caster
  //FUNCTIONALITY(RADIO):
  // - Radio connected to ZED-F9P via UART2
  // - ZED connected to Arduino via I2C
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//                       Libraries + Relevant Definitions

//When connecting ZED via I2C
#include <Wire.h>

#include <SparkFun_u-blox_GNSS_Arduino_Library.h> 
SFE_UBLOX_GNSS baseStation;

//===========================================================================================================
//                         Global Variables   

//Manually Enter From PPP output
  int ecefX = -392753937;
  int ecefX_HP = -22;
  int ecefY = 346187061;
  int ecefY_HP = 49;
  int ecefZ = -363072508;
  int ecefZ_HP = -98;
//Verification Variables
  bool success = true ;
  bool response1 = true;
  bool response2 = true;
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//                            SETTING UP MODULE AS A BASE STATION                               
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void setup(){
  Serial.begin(115200);
  while (!Serial); //Wait for user to open terminal
  Serial.println(F("---------Creating DGNSS Base Station----------"));
  //-----------------------------------------------
  //Begin Transmission of RTCM data over I2C - gets Data from ZED to ARD
  Wire.begin();
  Wire.setClock(400000); 
  if (baseStation.begin() == false) //Check for Valid I2C Wiring 
  {
    Serial.println(F("Invalid Wiring between ZED-F9P and Controller"));
    while (1);
  }

  //baseStation.factoryDefault(); delay(5000); //Uncomment to reset module to factory defaults

  
  baseStation.setI2COutput(COM_TYPE_UBX | COM_TYPE_NMEA | COM_TYPE_RTCM3); // Ensure RTCM3 is enabled --> output RTCM messages from ZED to controller
  //Set PPP of base station using ECEF coordinates from baseStationSetup script.
  success &= baseStation.setStaticPosition(ecefX, ecefX_HP, ecefY, ecefY_HP, ecefZ, ecefZ_HP);
  if (!success) Serial.println(F("At least one call to setStaticPosition failed!"));
  baseStation.setNavigationFrequency(1); //Set output in Hz. RTCM rarely benefits from >1Hz. (i.e. 1 RTCM message per second)
  baseStation.setSerialRate(57600, COM_PORT_UART2); //Set BaudRate of UART2 output to communicate with Rover
  baseStation.saveConfigSelective(VAL_CFG_SUBSEC_IOPORT); //Save the current settings to flash and BBR --> reinstate current configuration upon bootup


  //Enable RTCM Messages-------------------------------------------------------
    //RTCM output over UART2
    response1 &= baseStation.enableRTCMmessage(UBX_RTCM_1005, COM_PORT_UART2, 1); //Enable message 1005 to output through I2C port, message every second; Stationary RTK reference ARP
    response1 &= baseStation.enableRTCMmessage(UBX_RTCM_1074, COM_PORT_UART2, 1); // GPS MSM7
    response1 &= baseStation.enableRTCMmessage(UBX_RTCM_1084, COM_PORT_UART2, 1);
    response1 &= baseStation.enableRTCMmessage(UBX_RTCM_1094, COM_PORT_UART2, 1);
    response1 &= baseStation.enableRTCMmessage(UBX_RTCM_1124, COM_PORT_UART2, 1); // GLONASS MSM7
    response1 &= baseStation.enableRTCMmessage(UBX_RTCM_1230, COM_PORT_UART2, 10);
    //RTCM output over I2C (i.e. To ARD)
    response2 &= baseStation.enableRTCMmessage(UBX_RTCM_1005, COM_PORT_I2C, 1); //Enable message 1005 to output through I2C port, message every second ----|
    response2 &= baseStation.enableRTCMmessage(UBX_RTCM_1074, COM_PORT_I2C, 1);                                                                            
    response2 &= baseStation.enableRTCMmessage(UBX_RTCM_1084, COM_PORT_I2C, 1);
    response2 &= baseStation.enableRTCMmessage(UBX_RTCM_1094, COM_PORT_I2C, 1);
    response2 &= baseStation.enableRTCMmessage(UBX_RTCM_1124, COM_PORT_I2C, 1);
    response2 &= baseStation.enableRTCMmessage(UBX_RTCM_1230, COM_PORT_I2C, 10); //Enable message every 10 seconds
    
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

  success &= baseStation.setStaticPosition(ecefX, ecefX_HP, ecefY, ecefY_HP, ecefZ, ecefZ_HP);
  if (!success) Serial.println(F("At least one call to setStaticPosition failed!"));


  //------CONFIRMATION OF BASE STATION SETUP------

  int fix = baseStation.getFixType();
  if (fix != 5){
    Serial.println(F("BASE STATION COULD NOT ENTER FIX TYPE 5"));
    while(1); //Freeze
  } 

  Serial.println(F("Base Station Available To Transmit!"));
  //Serial1.println(F("Base Station Available To Transmit!"));


  while (Serial.available()) Serial.read(); //Clear any latent chars in serial buffer
  Serial.println(F("PRESS ANY KEY TO BEGIN TRANSMITTING RTCM DATA"));
  while (Serial.available() == 0) ; //Wait for user to press a key
  Serial.println("=================NOW TRANSMITTING==============");
  //Serial1.println("=================NOW TRANSMITTING==============");

  //Begin transmitting RTCM Messages from UART2 Port
  baseStation.setUART2Output(COM_TYPE_UBX | COM_TYPE_RTCM3);

  
}
          
void loop(){

//----------RADIO TRANSMISSION CODE--------------------------------
    baseStation.checkUblox(); //Calls processRTCM whenever new Data is available
    delay(250);
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//    PROCESSRTCM()
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  This function is called within the checkUblox function
//  Alter its functionality depending on how RTCM data is to be transmitted
//  Current configured to output RTCM over serial radio extension.
void SFE_UBLOX_GNSS::processRTCM(uint8_t incoming)
{
  
  //Ouput RTCM data to Serial Terminal 4 debugging--------------
    if (baseStation.rtcmFrameCounter % 16 == 0) Serial.println();
  Serial.print(F(" "));
  if (incoming < 0x10) Serial.print(F("0"));
  Serial.print(incoming, HEX);
  //------------------------------------------------------------
}

////=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

