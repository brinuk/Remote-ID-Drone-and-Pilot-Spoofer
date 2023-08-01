/*
 * Author: B J lambert
 * 
 * Reads and decodes data from a BN-880 or similar GPS module
 * Connect BN-880 TX to Arduino RX (pin0)***REMOVE WHEN PROGRAMMING THE ARDUINO***
 *                GND to Arduino GND
 *                VCC to Arduino 5V
 * decoder.begin(); and decoder.readRawData() must be run to collect GPS data
 * The baud rate is fixed at 9600
 * Note the sentences from the BN-880 start with GN and not GP as per the NMEA standard
 * To save variable memory the DATA_BUFFER_SIZE in the BN880Decoder.h file could be reduced in size from 250 bytes.
 * Reducing this may slow or stop the aquisition of the NMEA sentence.
 * With DATA_BUFFER_SIZE set to 250 bytes the aquistion of a GNGGA sentence is every 1-2 seconds.       
*/

#include "Arduino.h"
#include "BN880Decoder.h"

/*-----( Declare objects )-----*/


BN880Decoder::BN880Decoder()
{
  
}

//Start up everything
void BN880Decoder::begin()
{
  // start the serial communication with the host computer
    Serial.begin(GPSBAUD);
    Serial.println(F( "Comms with Arduino started at 9600 -- waiting for GPS data..."));
	while (Serial.available() <= 0){};//wait for GPS data
	Serial.println(F("GPS streaming data"));
	

}

//Stream raw gps data to data buffer
void BN880Decoder::readRawData()//reads gps
{
	int bufferPointer = 0;
	char rxData;
	bool started = false;
	while (bufferPointer < DATA_BUFFER_SIZE)
		{	
			if(Serial.available() > 0)
				{
					rxData = char(Serial.read());
					if(rxData == START_SENTENCE)
						{ 
							started = true;
						}
					if(started)
						{
							dataBuffer[bufferPointer] = rxData;
							bufferPointer++;
						}
				}
		}
}

//print contents of data buffer
void BN880Decoder::printDataBuffer()
{
	int bufferPointer = 0;
	while (bufferPointer < DATA_BUFFER_SIZE)
		{	
			if(dataBuffer[bufferPointer] == START_SENTENCE)
						{ 
							Serial.println();
						}
					Serial.print(dataBuffer[bufferPointer]);
					bufferPointer++;
		}
}

//returns the start index value of a string
int BN880Decoder::getSentenceIndex(String type, int from)
{
	int index;
	index = (String(dataBuffer)).indexOf(type, from);
	return index;
}

//returns a sentence less $ or an empty string. Type is sentence name eg GNGGA
String BN880Decoder::getSentence(String type)
{
	int firstIndex = 0;
	int secondIndex = 0;
	String sentence = "";
	firstIndex = getSentenceIndex(type, 0);//find correct sentence
	secondIndex = getSentenceIndex("*", firstIndex);//find end of sentence
	for(int n = firstIndex; n <= secondIndex+2; n++)//get full sentence
		{
			if(n < (DATA_BUFFER_SIZE -3))//dont go past end of buffer
				{
					sentence += dataBuffer[n];
				}
		}
	if(sentence.startsWith(type))
		{
			return sentence;
		}
	else
		{
			return "";
		}

}


//returns returns contents of a GNGGA sentence
bool BN880Decoder::getContentsGNGGA(int scanCount)
{
	int firstIndex = 0;
	int secondIndex = 0;
	sentence = getSentence("GNGGA");
	String time = "";
	if(sentence != "" && NMEAchecksum(sentence, sentence.length()))//sentence ok
		{
			//***********parse time************
			firstIndex = sentence.indexOf(",", 0);//find correct sentence
			secondIndex = sentence.indexOf(",", firstIndex + 1);
			if(firstIndex > -1 && secondIndex > -1)//found "," for time
				{
					time = sentence.substring(firstIndex + 1, secondIndex);
					hours = time.substring(0,2);
					minutes = time.substring(2,4);
					seconds = time.substring(4,6);
				}
			//********parse latitude**********
			firstIndex = secondIndex;//start at next ","
			secondIndex = sentence.indexOf(",", firstIndex + 1);//get next index of ","
			if(firstIndex > -1 && secondIndex > -1)//found "," for latitude
				{
					String latitudeStr = sentence.substring(firstIndex + 1, secondIndex);//get latitude
					float latint = stringToFloat(latitudeStr.substring(0,2));//get integer bit of latitude as a float
					float latdec = stringToFloat(latitudeStr.substring(2,10));//get decimal bit of latitude as a float
					latitude = latint + latdec/60;//get total latitude as a float
				}
			//***********parse hemisphere N/S**********
			firstIndex = secondIndex;//start at next ","
			secondIndex = sentence.indexOf(",", firstIndex + 1);//get next index of ","
			if(firstIndex > -1 && secondIndex > -1)//found "," for hemisphere N/S
				{
					hemisphereNS = sentence.substring(firstIndex + 1, secondIndex);//get hemisphere N/S
				}
			//*********parse longitude***********
			firstIndex = secondIndex;//start at next ","
			secondIndex = sentence.indexOf(",", firstIndex + 1);//get next index of ","
			if(firstIndex > -1 && secondIndex > -1)//found "," for longitude
				{
					String longitudeStr = sentence.substring(firstIndex + 1, secondIndex);//get longitude
					float lonint = stringToFloat(longitudeStr.substring(0,3));//get integer bit of longitude as a float
					float londec = stringToFloat(longitudeStr.substring(3,11));//get decimal bit of longitude as a float
					longitude = lonint + londec/60;//get total longitude as a float
				}
			//***********parse hemisphere E/W**********
			firstIndex = secondIndex;//start at next ","
			secondIndex = sentence.indexOf(",", firstIndex + 1);//get next index of ","
			if(firstIndex > -1 && secondIndex > -1)//found "," for hemisphere E/W
				{
					hemisphereEW = sentence.substring(firstIndex + 1, secondIndex);//get hemisphere E/W
				}
			//***********parse GPS fix**********
			firstIndex = secondIndex;//start at next ","
			secondIndex = sentence.indexOf(",", firstIndex + 1);//get next index of ","
			if(firstIndex > -1 && secondIndex > -1)//found "," for GPS fix
				{
					gpsFix = sentence.substring(firstIndex + 1, secondIndex);//get hemisphere E/W
				}

			//*********parse satellites***********
			firstIndex = secondIndex;//start at next ","
			secondIndex = sentence.indexOf(",", firstIndex + 1);//get next index of ","
			if(firstIndex > -1 && secondIndex > -1)//found "," for number of satellites
				{
					String satellitesStr = sentence.substring(firstIndex + 1, secondIndex);//get satelites
					satellites = stringToFloat(satellitesStr.substring(0,2));//get string as a float
				}
			//*********parse HDOP***********
			firstIndex = secondIndex;//start at next ","
			secondIndex = sentence.indexOf(",", firstIndex + 1);//get next index of ","
			if(firstIndex > -1 && secondIndex > -1)//found "," for HDOPs
				{
					String hdopStr = sentence.substring(firstIndex + 1, secondIndex);//get HDOP
					hdop = stringToFloat(hdopStr.substring(0,4));//get string HDOP as a float
				}
			//*********parse altitude***********
			firstIndex = secondIndex;//start at next ","
			secondIndex = sentence.indexOf(",", firstIndex + 1);//get next index of ","
			if(firstIndex > -1 && secondIndex > -1)//found "," for altitude
				{
					String altitudeStr = sentence.substring(firstIndex + 1, secondIndex);//get altitude
					altitude = stringToFloat(altitudeStr.substring(0,5));//get string altitude as a float
				}
			//*********parse height of geoid***********
			firstIndex = secondIndex;//start at next ","
			secondIndex = sentence.indexOf(",", firstIndex + 1);//get next index of ","
			firstIndex = secondIndex;//start at next ","
			secondIndex = sentence.indexOf(",", firstIndex + 1);//get next index of ","
			if(firstIndex > -1 && secondIndex > -1)//found "," for geoid height
				{
					String geoidHeightStr = sentence.substring(firstIndex + 1, secondIndex);//get geoid height
					geoidHeight = stringToFloat(geoidHeightStr.substring(0,4));//get string geoid height as a float
				}
			//distance between two points measured every scanCount scans
					if(distanceCount == 0)
						{
							lat1 = latitude;//save start lat and lon
							lon1 = longitude;
							distance = 0.0;//set distance to zero
						}
					distanceCount++;
					//lat2 = latitude;
					//lon2 = longitude;
					if(distanceCount >= scanCount)
						{
							distance = distanceApart(lat1, lon1, latitude, longitude);//calculated moved distance
							distanceCount = 0;
						}	
						

			return true;
		}
	else
		{
			return false;
		}
	

}

//returns a string as a float
float BN880Decoder::stringToFloat(String input)
{
	char array1[10];//set up temp char array
	input.toCharArray(array1,8);//put decimal part into char array
	float result = atof(array1);//change string to float
	return result;
}


//calulates and checks the NMEA checksum and returns true or false
bool BN880Decoder::NMEAchecksum( String input, int cnt )
{
		
	char buf[cnt];//set up temp char array
	input.toCharArray(buf,cnt);//put string into char array
    char Character;
    int Checksum = 0;
    int i;              // loop counter

    //foreach(char Character in sentence)
    for (i=0;i<cnt;++i)
    {
        Character = buf[i];
        switch(Character)
        {
            case '$':
                // Ignore the dollar sign
                break;
            case '*':
                // Stop processing before the asterisk
                i = cnt;
                continue;
            default:
                // Is this the first value for the checksum?
                if (Checksum == 0)
                {
                    // Yes. Set the checksum to the value
                    Checksum = Character;
                }
                else
                {
                    // No. XOR the checksum with this character's value
                    Checksum = Checksum ^ Character;
                }
                break;
        }
    }
		String result = String(Checksum, HEX);//convert int to HEX string
		result.toUpperCase();//converts to upper case
		int firstIndex = 0;
		firstIndex = input.indexOf("*", 0);//find start of CRC
		String crc = sentence.substring(firstIndex+1, firstIndex+3);//get CRC
		if(crc == result)//check if CRC correct
			{
				return true;
			}
		else
			{
				return false;
			}

} // NEMA_Checksum end

//Returns distance (m) between two GPS coordinates
float BN880Decoder::distanceApart(float lat1, float lon1, float lat2, float lon2)
	{
        
        float dlat1=lat1*(PI/180);
        float dlong1=lon1*(PI/180);
        float dlat2=lat2*(PI/180);
        float dlong2=lon2*(PI/180);

        float dLong=dlong1-dlong2;
        float dLat=dlat1-dlat2;

        float aHarv= pow(sin(dLat/2.0),2.0)+cos(dlat1)*cos(dlat2)*pow(sin(dLong/2),2);
        float cHarv=2*atan2(sqrt(aHarv),sqrt(1.0-aHarv));
        //earth's radius from wikipedia varies between 6,356.750 km — 6,378.135 km (˜3,949.901 — 3,963.189 miles)
        //The IUGG value for the equatorial radius of the Earth is 6378.137 km (3963.19 mile)
        const float earth=6378.137;//I am doing miles, just change this to radius in kilometers to get distances in km
        float dist = earth*cHarv*1000;//result in metres
		if(dist > 1000000)
			{
				dist = 0.0;//set to zero if calculation error
			}
		return dist;
	}



