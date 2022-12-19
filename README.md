# DGNSS (Differential Global Navigational Satellite System)
## SCRIPTS:
   baseStationSetup - outputs RAW_RXM data to SD card for submission to AUSPOS for PPP; yet to be tested irl.
   
   baseStationFunction - set location of base station and enable output of RTCM3 sentences.
   
   roverFunction - enables rover module to recieve RTCM3 sentences, process and output long, lat and alt; Yet to be tested irl.
   
   ##### Libraries:
      1. Sparkfun UBlox GNAA Arduino Library - https://github.com/sparkfun/SparkFun_u-blox_GNSS_Arduino_Library.git
         
            - IMPORTANT NOTE: change line 7587 to -> packetCfg.len = 40;
            
      2. WiFi - Add via Arduino Library Manager; Only for NTRIP communications
      
      3. Wire - enables verification of I2C connection
   
## TODO:
   - [ ] Test Rover Function Script irl
   - [x] Fine tune baseStationFunction to be compatable with AUSPOS ECEF input.
   - [ ] Explore NTRIP - transmission of RTCM3 sentences via IP

         - NTRIP Casters - security?, price?, compatability?
   - [ ] Containment of Rover & Base Station Modules

## READING

   PPP - https://geoscienceaustralia.github.io/ginan/ppp.html
   
   RTK vs DGNSS - https://geoscienceaustralia.github.io/ginan/rtk.html
   
# SETTING UP THE BASE STATION (RADIO LINK)

## HARDWARE
##### Equipment:
  1. ZED-F9P
  2. Sik Telemetry Radio
  3. Arduino
##### Wiring:
  Arduino 2 ZED (I2C)
  
    ZED 5V --> Arduino 5V (can also be 3.3V)
    
    ZED GND --> Arduino GND (cant also be 3.3V)
    
    ZED SCL --> Arduino SCL (Clock Line)
    
    ZED SDA --> Arduino SDA (Data Line)
    
  ZED 2 Telemetry Radio (UART2)
  
    ZED TX2 --> Radio RX  ----
                              | - NOT A TYPO - Meant to be opposite.
    ZED RX2 --> Radio TX  ----
    
    ZED GND --> Radio GND
    
    ZED 3.3V --> Radio 5V (Wire to ZED 5V if more range required)
## FUNCTIONALITY
The DGNSS base station is static. Its location is set via PPP ECEF coordinates.

Using these coordinates it will determine the error in the GNSS location it currently recieves.

This error is then sent in the form of RTCM3 sentences to the rover over radio connected to the ZED's UART2 port. 

## SOFTWARE SETUP

  upload baseStationFunction.ino script 2 Arduino.
  
##### VERIFICATION
  Script should indicate that base station has entered functionality mode '5' (i.e. TIME MODE)
  
  RTCM3 values should be printing to Arduino Serial. 



# SETTING UP THE ROVER (RADIO LINK) - currently setup using U-Center.

## HARDWARE
##### Equipment:
  1. ZED-F9P
  2. Sik Telemetry Radio
  3. USB 2 USB-C Cable
  4. U-Center (windows software only)
##### Wiring:
  U-Center 2 ZED (I2C)
  
    Connect using USB 2 USB-C cable
    
  ZED 2 Telemetry Radio (UART2)
  
    ZED TX2 --> Radio RX  ----
                              | - NOT A TYPO - Meant to be opposite.
    ZED RX2 --> Radio TX  ----
    
    ZED GND --> Radio GND
    
    ZED 3.3V --> Radio 5V (Wire to ZED 5V if more range required)
## FUNCTIONALITY
The DGNSS rover is mobile.

It receives RTCM3 sentences from the base station.

It applies the error correction from the RTCM3 sentences to accurately find its position (within ~14mm). 

The rover will then output lattitude and longitude and whatever other data is required.

## U-CENTER SETUP

  *OPEN MESSAGE VIEW WINDOW*

  STEP ONE: RESET ALL SETTINGS
    
    UBX - CFG - CFG --> select 'Reset to Default Settings' and press send.
    
  STEP TWO: CONFIGURE TO RECEIVE RTCM3 SENTENCES
  
    UBX - CFG - PRT
    
      Select port drop down menu
      
      Select UART2 Port
      
      Set protocol in to RTCM3
      
      Set protocol out to none
      
      Set baud-rate to 57600
      
   STEP THREE: SAVE ROVER CONFIGURATION
   
     UBX - CFG - CFG --> select 'Save this Configuration' and press send.
     
   ROVER SHOULD NOW BE FUNCTIONAL
   
   STEP FOUR: VERIFY RTCM3 SENTENCES ARE BEING RECEIVED
   
    UBX - RCV - RTCM --> This should display all RTCM3 messages coming in and whether they are used or not. 
    
   STEP FIVE: VERIFY FIX MODE & ACCURACY
   
    Fix Mode: UBX - NAV - PVT --> Fix Mode should be RTK Fix
    
    Accuracy: UBX - NAV - HPOPECEF --> accuracy estimate should be ~0.014m
    
    
  



  
   
