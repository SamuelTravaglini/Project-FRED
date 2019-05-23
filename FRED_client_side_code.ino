#include <RadioHead.h>
#include <RHDatagram.h>
#include <RH_RF95.h>
#include <RH_Serial.h>

#include <SPI.h>
#include <TinyGPS.h>
#include <SoftwareSerial.h>



/*
 * Curtin University Technology Project
 * Project: Fire Risk Evaluation Drone
 * Author: Samuel Travaglini 16990582
 * 
 * This code is to read values from the DHT11, MQ-7, and YG1006 sensors as well as the GPS module 
 * and send them to the server using lora wireless.
 * 
 * This project uses libraries for the needed functionality.
 */

const int tempPin = 8;
const int flamePin = 6;
const int flameLedPin = 13;
const int COAnaloguePin = A0;
int temperature = -1;
int humidity = -1;
int flameDetected;

//Lora + GPS
RH_RF95 rf95;
TinyGPS gps;
SoftwareSerial serial_software(3, 4);
String datastring="";
String datastring1="";
char databuf[100];
uint8_t dataoutgoing[100];
char gps_lon[20]={"\0"};  
char gps_lat[20]={"\0"}; 

void setup() 
{
   Serial.begin(115200);
   pinsInit()

   serial_software.begin(115200);

   if ( !r95.init() )
   {
      Serial.println( "Init failed." );
   }

   Serial.println();
}

void pinsInit()
{
   pinMode( flamePin, INPUT );
   pinMode( LED, OUTPUT );
}

void loop() 
{

   //Display temperature and humidty
   displayDHT11()
   delay(2000); // ms

   //Carbon Monoxide Level Display
   displayMQ7()
   delay(2000); // ms
   
   //Display Flame Detection
   displayFlame();
   delay(2000); // ms

   //Get GPS data and send via lora wireless
   gpsData();
   
}

int readDHT11() 
{
   uint8_t recvBuffer[5];
   uint8_t cnt = 7;
   uint8_t idx = 0;
   
   for(int i = 0; i < 5; i++)
   {
      recvBuffer[i] = 0;
   }

   // Start communications with LOW pulse
   pinMode(tempPin, OUTPUT);
   digitalWrite(tempPin, LOW);
   delay(18);
   digitalWrite(tempPin, HIGH);
   delayMicroseconds(40); //
   pinMode(tempPin, INPUT);
   pulseIn(tempPin, HIGH); //
   
  // Read data packet
   unsigned int timeout = 10000; // loop iterations
   
   for (int i=0; i<40; i++) //
   {
      timeout = 10000;
      while(digitalRead(tempPin) == LOW) 
      {
         if (timeout == 0)
         {
            return -1;
         }
         timeout--;
      }

      unsigned long t = micros(); //
      timeout = 10000;
   
      while(digitalRead(tempPin) == HIGH) 
      { 
         if (timeout == 0) return -1;
         timeout--;
      }
   
      if ((micros() - t) > 40)
      {
         recvBuffer[idx] |= (1 << cnt); //
      }
   
      if (cnt == 0) // next byte?
      {
         cnt = 7; // restart at MSB
         idx++; // next byte!
      }
      else
      { 
         cnt--;
      }
   }

   humidity = recvBuffer[0]; // % //
   temperature = recvBuffer[2]; // C
   uint8_t sum = recvBuffer[0] + recvBuffer[2];
   
   if(recvBuffer[4] != sum)
   { 
      return -2; //
   }
   
   return 0;
}


void displayDHT11()
{
   int ret = readDHT11();

   //Display DHT11 (Temperature and Humidity) Reading
   if(ret != 0)
   { 
      Serial.println(ret);
   }
   Serial.print("Humidity: "); 
   Serial.print(humidity); Serial.println(" %");
   Serial.print("Temperature: "); 
   Serial.print(temperature); Serial.println(" C");
   //End of DHT11 values
}

void displayMQ7()
{
   int COVal;

   COVal = analogueRead(0); // Read from analogue pin A0
   Serial.print("Carbon Monoxide Level: "); 
   Serial.println(val, DEC);
}

void displayFlame()
{
   flameDetected = digitalRead( flamePin );

   if( flameDetected == 1 )
   {
      Serial.println( "** WARNING Flame Detected!! **" );
      Serial.println( flameDetected );
      digitalWrite( LED, HIGH );
      delay( 200 );
      digitalWrite( LED, LOW );
      delay( 200 );
   }
   else
   {
      Serial.prinln( "No flame detected" );
      Serial.println( flameDetected );
      digitalWrite( LED, LOW );
   }

   delay( 1000 );
}

void gpsData()
{
   serial_software.println("Sending to rf95_server");
   bool newData = false;
   unsigned long chars;
   unsigned short sentences, failed;

   for ( unsigned long start = millis(); millis() - start < 1000; )
   {
      while ( Serial.available() )
      {
         char c = Serial.read();
         //Serial.write(c);
         if ( gps.encode(c) )
         {
            newData = true;
         }
      }
   }

   //Retrieve GPS Data
   if ( newData )
   {
       float flat, flon;
       unsigned long age;

       gps.f_get_position( &flat, &flon, &age );
       serial_software.print( "LAT=" );
       serial_software.print( flatt == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6 );
       
       serial_software.print( "LON=" );
       serial_software.print( flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6 );

       serial_software.print( "SAT=" );
       serial_software.print( gps.satellites() == TinyGPS::GPS_INVALID_SATELLITES ? 0 : gps.satellites() );

       serial_software.print( "PREC=" );
       serial_software.print( gps.hdop() == TinyGPS::GPS_INVALID_HDOP ? 0 : gps.hdop() );

       flat = TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 ; flat, 6;
       flon = TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6;

       datastring +=dtostrf( flat, 0, 6, gps_lat );
       datastring1 +=dtostrf( flon, 0, 6, gps_lon );
       serial_software.println( strcat( strcat( gps_lon,","), gps_lat ) );
       strcpy( gps_lat, gps_lon );

       serial_software.println( gps_lat );
       strcpy( (char *)dataoutgoing, gps_lat );

       //Send Data
       rf95.send( dataoutgoing, sizeof(dataoutgoing) );

       uint8_t indatabuf[RH_RF95_MAX_MESSAGE_LEN];
       uint8_t len = sizeof( indatabuf );

       if ( rf95.waitAvailableTimeout(3000) );
       {
          if ( rf95.recv( indatabuf, &len ) )
          {
             serial_software.print( "Reply: " );
             serial_software.println( (char *)indatabuf );
          }
          else
          {
             serial_software.println( "Receiver Failed." );
          }
       }
       else
       {
          serial_software.println( "No reply. Check server side." );
       }

       delay(400);
   }

   gps.stats( &chars, &sentences, &failed );                                                                                                                                                                                                                                                                                                                                                                          
   ss.print( " CHARS=" );
   ss.print( chars );
   ss.print( " SENTENCES=" );
   ss.print( sentences );
   ss.print( " CSUM ERR=" );
   ss.println( failed );
   
   if ( chars == 0 )
   {
      ss.println("** No characters received from GPS: check wiring **");
   }
}
