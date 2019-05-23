// dht11.ino - print humidity and temperature using DHT11 sensor

const int tempPin = 8;
const int flamePin = 6;
const int flameLedPin = 13;
const int COAnaloguePin = A0;
int temperature = -1;
int humidity = -1;
int flameDetected;

void setup() 
{
   Serial.begin(115200);
   pinsInit()
}

void pinsInit()
{
   pinMode( flamePin, INPUT );
   pinMode( LED, OUTPUT );
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
