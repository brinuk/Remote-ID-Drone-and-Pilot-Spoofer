/* Lolin Wemos R2 RemoteID and Pilot spoofer
 *  Use in conjunction with the OpenDronID app on mobile phone, for quicker refresh set WIFI Throttling to off
 *  THIS IS FOR PROOF OF CONCEPT ONLY, may not be legal to use in your country.
 *  Heavily adapted from https://github.com/sxjack/uav_electronic_ids
 *  and https://github.com/jjshoots/RemoteIDSpoofer
 *  Generates 16 pilots and 16 drones at random location around the actual pilot
 *  Uses BN220 GPS module for pilot location
 *  Uses Wemos R2 module
 * Connect BN-220   TX to Arduino RX ***REMOVE WHEN PROGRAMMING THE ARDUINO***
 *                  RX to Arduino TX
 *                  GND to Arduino GND
 *                  VCC to Arduino 5V
 * decoder.begin(); and decoder.readRawData() must be run to collect GPS data
 * decoder.getContentsGNGGA() decodes the GPS data
 * The baud rate is fixed at 9600
*/

#include "spoofer.h"
#include "BN880Decoder.h"
#include <SPI.h>

/*-----( Declare Constants and Pin Numbers )-----*/
#define DEBUG 0 //for print statements on set DEBUG to 1
#if(DEBUG == 1)
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#define debugArg(x,y) Serial.print(x,y)
#define debuglnArg(x,y) Serial.println(x,y)
#else
#define debug(x)
#define debugln(x)
#define debugArg(x,y)
#define debuglnArg(x,y)
#endif

#define LED_PIN 9
#define MIN_SATS 5
#define SCANS_DIST_MEASURED 0 //Always set to zero


/*-----( Declare objects )-----*/
BN880Decoder decoder;


/*-----( Declare Variables )-----*/
const int num_spoofers = 16;
static Spoofer spoofers[num_spoofers];
double latSigned = 0.0;
double lonSigned = 0.0;



void setup()
{
  //get GPS data and pilot location
  decoder.begin();
  debugln(F("Waiting for a good GPS sentence..."));
  delay(2000);
  while(!goodGpsFix())
    {  
    }
  // to print data set #define DEBUG to 1
        debug(F("Time "));
        debug(decoder.hours);
        debug(F(":"));
        debug(decoder.minutes);
        debug(F(":"));
        debug(decoder.seconds);
        debugln();
        debug(F("Latitude "));
        if(decoder.hemisphereNS == "N")
          {
            debugArg(decoder.latitude,6);
            latSigned = decoder.latitude;
          }
        else
          {
            debugArg(-decoder.latitude,6);
            latSigned = -decoder.latitude;
          }
        debugln(" " + decoder.hemisphereNS);
        debug(F("Longitude "));
        if(decoder.hemisphereEW == "E")
          {
            debugArg(decoder.longitude,7);
            lonSigned = decoder.longitude;
          }
        else
          {
            debugArg(-decoder.longitude,7);
            lonSigned = -decoder.longitude;
          }
        debugln(" " + decoder.hemisphereEW);
        debug(F("GPS fix "));
        if(decoder.gpsFix == "0")
          {
            debugln(F("Bad"));
          }
        else
          {
            debugln(F("Good"));
          }
        debug(F("Satellites "));
        debuglnArg(decoder.satellites,0);
        debug(F("Horizontal dilution of precision "));
        debuglnArg(decoder.hdop,2);
        debug(F("Altitude "));
        debugArg(decoder.altitude, 1);
        debugln(F(" metres"));
        debug(F("Height of geoid above WGS84 ellipsoid "));
        debugArg(-decoder.geoidHeight, 1);
        debugln(F(" metres"));
        debugln(F("****************"));

        //set pilot latitude longitude in spoofer
        spoofers[num_spoofers].pilotLat = latSigned; 
        spoofers[num_spoofers].pilotLong = lonSigned; 
      
    
}
// run spoofer
void loop() {
  for (int i = 0; i < num_spoofers; i++) {
    spoofers[i].update();
    delay(200 / num_spoofers);
  }
}

//check status of gps fix
    bool goodGpsFix()
      {
        bool fixGood = false;
        decoder.readRawData();
        decoder.getContentsGNGGA(SCANS_DIST_MEASURED);
        if(decoder.satellites >= MIN_SATS && decoder.gpsFix != "0")
          {
            fixGood = true;
          }
        return fixGood;
     }
