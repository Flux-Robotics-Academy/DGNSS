//=-=-=-=-=-=-=-=-=-ROVER FUNCTION=-=-=-=-=-=-=-=-=-=-=-
//This scipt will enable the rover to recieve RTCM data and then output longitude and latitiude considering the RTCM data.
//FUNCTIONALITY:
//    1. Pass in RTCM Data - either using NTRIP or Radio - integrate code from NTRIP client
//    2. Output Long and Lat - integrate code from getLongLatMoreSerial
//Modify secrets.h to the correct port and host for the relevant NTRIP caster
//NEED to workout Method for outputting data to a python script - via USB? via Serial (Radio? I2C???)

#include <SparkFun_u-blox_GNSS_Arduino_Library.h> //http://librarymanager/All#SparkFun_u-blox_GNSS
SFE_UBLOX_GNSS rover;

//The ESP32 core has a built in base64 library but not every platform does
//We'll use an external lib if necessary.
//#include "base64.h" //Built-in ESP32 library for encoding binary into base64 strings and the reverse operation.


//Global variables
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
long lastReceivedRTCM_ms = 0; //5 RTCM messages take approximately ~300ms to arrive at 115200bps
int maxTimeBeforeHangup_ms = 10000; //If we fail to get a complete RTCM frame after 10s, then disconnect from caster
int fix = 10; //10 = unitialised
int carrierSolution = 5; //unitialised
bool diffCheck = false;
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=


void setup() {

  Serial.begin(115200);
  Serial.println(F("==========ROVER BASE=========="));


  Wire.begin(); //Start I2C
  if (rover.begin() == false){ //Connect to the Ublox module using Wire port
    Serial.println(F("u-blox GPS not detected at default I2C address. Please check wiring. Freezing."));
    while (1);
  }
  Serial.println(F("u-blox module connected"));

  //rover.factoryDefault(); delay(5000);

  //Configure Input & Output Ports
  rover.setSerialRate(57600, COM_PORT_UART2);
  rover.setI2COutput(COM_TYPE_UBX); //Turn off NMEA noise
  rover.setPortInput(COM_PORT_I2C, COM_TYPE_UBX | COM_TYPE_NMEA | COM_TYPE_RTCM3); //Be sure RTCM3 input is enabled. UBX + RTCM3 is not a valid state.
  rover.setPortInput(COM_PORT_UART2, COM_TYPE_RTCM3); //Be sure RTCM3 input is allowed to be recieved through UART2
  rover.setNavigationFrequency(1); //Set output in Hz
  rover.setAutoPVT(true); //Tell the GNSS to "send" each solution

  fix = rover.getFixType();    
  Serial.print("System Fix Type: ");
  Serial.print(fix);
  Serial.println(" "); 

  carrierSolution = rover.getCarrierSolutionType();
  Serial.print("Carrier Solution Type: ");
  Serial.print(carrierSolution);
  Serial.println(" "); 

}

void loop() {
  
  //    SEND RTCM CORRECTION DATA OVER RADIO
  // Radio --(serial)--> Arduino --(I2C)--> ZED-F9P
  //==========================================================

    //Import data from telemetry radio to arduino - over serial 
    //export data from arduino to the Zed-f9p - over I2C

    if (Serial1.available()){
      beginRadio();
      while (Serial1.available()) Serial1.read(); //Empty buffer of any newline chars
    }
    if (rover.getPVT()){
      sendGNSSData();
    }

    delay(500);
} 

void beginRadio(){
      long rtcmCount = 0;

      uint8_t rtcmData[512 * 4]; //Most incoming data is around 500 bytes but may be larger
      rtcmCount = 0;

      while (Serial1.available()){
      rtcmData[rtcmCount++] = Serial1.read();
        if (rtcmCount == sizeof(rtcmData)) break;
          }

          if (rtcmCount > 0)
          {
            lastReceivedRTCM_ms = millis();

            //Push RTCM to GNSS module over I2C - probably dont need this id data not coming from an NTRIP caster --> Look into it more
            rover.pushRawData(rtcmData, rtcmCount, false);
            Serial.print(F("RTCM pushed to ZED: "));
            Serial.println(rtcmCount);
          }

          //Send Warning if we don't have new data for 10s
        if (millis() - lastReceivedRTCM_ms > maxTimeBeforeHangup_ms)
        {
          Serial.println(F("WARNING: RTCM timeout."));
          return;
        }

        
    

  }
        
  void sendGNSSData(){

    Serial.println("-----NEW TRANSMISSION------------------");

    long latitude = rover.getLatitude();
    //float latitudeADJ = latitude/10000000;
    Serial.print(F(" Lat: "));
    Serial.print(latitude);
    Serial.println(" ");

    long longitude = rover.getLongitude();
    //float longitudeADJ = longitude/10000000;
    Serial.print(F(" Long: "));
    Serial.print(longitude);
    Serial.print(F(" (degrees * 10^-7)"));
    Serial.println(" ");

    long altitude = rover.getAltitude();
    Serial.print(F(" Alt: "));
    Serial.print(altitude);
    Serial.print(F(" (mm)"));
    Serial.println(" ");

    long accuracy = rover.getPositionAccuracy(); //Indoor = ~10m; //Outdoor + Clear Sky = ~15mm
    Serial.print(F(" 3D Positional Accuracy: "));
    Serial.print(accuracy);
    Serial.println(F(" (mm)"));

    //Should be 4 ideally - also maybe 3
    int fix = rover.getFixType();
    Serial.print(F("System Fix Type: "));
    Serial.print(fix);
    Serial.println(" ");

    //This should enter 1 or 2 --> preferably 2
    carrierSolution = rover.getCarrierSolutionType();
    Serial.print("Carrier Solution Type: ");
    Serial.print(carrierSolution);
    Serial.println(" ");

    diffCheck = rover.getDiffSoln();
    if (diffCheck == true){
      Serial.println("Differential Correction NOT Applied");
    }
    else{
      Serial.println("Differential Correction Applied");
    }


    Serial.println();
  }






