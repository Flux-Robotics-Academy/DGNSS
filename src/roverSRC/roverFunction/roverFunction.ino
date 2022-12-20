//=-=-=-=-=-=-=-=-=-ROVER FUNCTION=-=-=-=-=-=-=-=-=-=-=-
//This scipt will enable the rover to recieve RTCM data and then output longitude and latitiude considering the RTCM data.
//FUNCTIONALITY:
//    1. Pass in RTCM Data - either using NTRIP or Radio - integrate code from NTRIP client
//    2. Output Long and Lat - integrate code from getLongLatMoreSerial
//Modify secrets.h to the correct port and host for the relevant NTRIP caster
//NEED to workout Method for outputting data to a python script - via USB? via Serial (Radio? I2C???)

#include <WiFi.h>
#include "secrets.h"

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

  
  //      SETUP ROVER FOR RTCM OVER NTRIP
  // Uncomment if NTRIP communication required
  //=============================================================
  // Serial.print(F("Connecting to local WiFi"));
  // WiFi.begin(ssid, password);
  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(500);
  //   Serial.print(F("."));
  // }
  // Serial.println();

  // Serial.print(F("WiFi connected with IP: "));
  // Serial.println(WiFi.localIP());

  // while (Serial.available()) Serial.read();

  //        SETUP ROVER FOR RTCM OVER RADIO
  //i.e. recieve rtcm correction data over radio (serial)
  //===================================================================

    //Send Long and Lat over Serial
    //S

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
  //          SEND RTCM DATA OVER NTRIP
  //==================================================
    //   if (Serial.available())
    //   {
    //     beginClient();
    //     while (Serial.available()) Serial.read(); //Empty buffer of any newline chars
    //   }

    //   Serial.println(F("Press any key to start NTRIP Client."));

    //   delay(1000);
    // }

    // //Connect to NTRIP Caster, receive RTCM, and push to ZED module over I2C
    // void beginClient()
    // {
    //   WiFiClient ntripClient;
    //   long rtcmCount = 0;

    //   Serial.println(F("Subscribing to Caster. Press key to stop"));
    //   delay(10); //Wait for any serial to arrive
    //   while (Serial.available()) Serial.read(); //Flush

    //   while (Serial.available() == 0)
    //   {
    //     //Connect if we are not already. Limit to 5s between attempts.
    //     if (ntripClient.connected() == false)
    //     {
    //       Serial.print(F("Opening socket to "));
    //       Serial.println(casterHost);

    //       if (ntripClient.connect(casterHost, casterPort) == false) //Attempt connection
    //       {
    //         Serial.println(F("Connection to caster failed"));
    //         return;
    //       }
    //       else
    //       {
    //         Serial.print(F("Connected to "));
    //         Serial.print(casterHost);
    //         Serial.print(F(": "));
    //         Serial.println(casterPort);

    //         Serial.print(F("Requesting NTRIP Data from mount point "));
    //         Serial.println(mountPoint);

    //         const int SERVER_BUFFER_SIZE  = 512;
    //         char serverRequest[SERVER_BUFFER_SIZE];

    //         snprintf(serverRequest, SERVER_BUFFER_SIZE, "GET /%s HTTP/1.0\r\nUser-Agent: NTRIP SparkFun u-blox Client v1.0\r\n",
    //                 mountPoint);

    //         char credentials[512];
    //         if (strlen(casterUser) == 0)
    //         {
    //           strncpy(credentials, "Accept: */*\r\nConnection: close\r\n", sizeof(credentials));
    //         }
    //         else
    //         {
    //           //Pass base64 encoded user:pw
    //           char userCredentials[sizeof(casterUser) + sizeof(casterUserPW) + 1]; //The ':' takes up a spot
    //           snprintf(userCredentials, sizeof(userCredentials), "%s:%s", casterUser, casterUserPW);

    //           Serial.print(F("Sending credentials: "));
    //           Serial.println(userCredentials);

    //           //Encode with ESP32 built-in library
    //           base64 b;
    //           String strEncodedCredentials = b.encode(userCredentials);
    //           char encodedCredentials[strEncodedCredentials.length() + 1];
    //           strEncodedCredentials.toCharArray(encodedCredentials, sizeof(encodedCredentials)); //Convert String to char array
    //           snprintf(credentials, sizeof(credentials), "Authorization: Basic %s\r\n", encodedCredentials);
    //         }
    //         strncat(serverRequest, credentials, SERVER_BUFFER_SIZE);
    //         strncat(serverRequest, "\r\n", SERVER_BUFFER_SIZE);

    //         Serial.print(F("serverRequest size: "));
    //         Serial.print(strlen(serverRequest));
    //         Serial.print(F(" of "));
    //         Serial.print(sizeof(serverRequest));
    //         Serial.println(F(" bytes available"));

    //         Serial.println(F("Sending server request:"));
    //         Serial.println(serverRequest);
    //         ntripClient.write(serverRequest, strlen(serverRequest));

    //         //Wait for response
    //         unsigned long timeout = millis();
    //         while (ntripClient.available() == 0)
    //         {
    //           if (millis() - timeout > 5000)
    //           {
    //             Serial.println(F("Caster timed out!"));
    //             ntripClient.stop();
    //             return;
    //           }
    //           delay(10);
    //         }

    //         //Check reply
    //         bool connectionSuccess = false;
    //         char response[512];
    //         int responseSpot = 0;
    //         while (ntripClient.available())
    //         {
    //           if (responseSpot == sizeof(response) - 1) break;

    //           response[responseSpot++] = ntripClient.read();
    //           if (strstr(response, "200") > 0) //Look for 'ICY 200 OK'
    //             connectionSuccess = true;
    //           if (strstr(response, "401") > 0) //Look for '401 Unauthorized'
    //           {
    //             Serial.println(F("Hey - your credentials look bad! Check you caster username and password."));
    //             connectionSuccess = false;
    //           }
    //         }
    //         response[responseSpot] = '\0';

    //         Serial.print(F("Caster responded with: "));
    //         Serial.println(response);

    //         if (connectionSuccess == false)
    //         {
    //           Serial.print(F("Failed to connect to "));
    //           Serial.print(casterHost);
    //           Serial.print(F(": "));
    //           Serial.println(response);
    //           return;
    //         }
    //         else
    //         {
    //           Serial.print(F("Connected to "));
    //           Serial.println(casterHost);
    //           lastReceivedRTCM_ms = millis(); //Reset timeout
    //         }
    //       } //End attempt to connect
    //     } //End connected == false

    //     if (ntripClient.connected() == true)
    //     {
    //       uint8_t rtcmData[512 * 4]; //Most incoming data is around 500 bytes but may be larger
    //       rtcmCount = 0;

    //       //Print any available RTCM data
    //       while (ntripClient.available())
    //       {
    //         //Serial.write(ntripClient.read()); //Pipe to serial port is fine but beware, it's a lot of binary data
    //         rtcmData[rtcmCount++] = ntripClient.read();
    //         if (rtcmCount == sizeof(rtcmData)) break;
    //       }

    //       if (rtcmCount > 0)
    //       {
    //         lastReceivedRTCM_ms = millis();

    //         //Push RTCM to GNSS module over I2C
    //         rover.pushRawData(rtcmData, rtcmCount, false);
    //         Serial.print(F("RTCM pushed to ZED: "));
    //         Serial.println(rtcmCount);
    //       }
    //     }

    //     //Close socket if we don't have new data for 10s
    //     if (millis() - lastReceivedRTCM_ms > maxTimeBeforeHangup_ms)
    //     {
    //       Serial.println(F("RTCM timeout. Disconnecting..."));
    //       if (ntripClient.connected() == true)
    //         ntripClient.stop();
    //       return;
    //     }

    //     delay(10);
    //   }

    //   Serial.println(F("User pressed a key"));
    //   Serial.println(F("Disconnecting..."));
    //   ntripClient.stop();

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






