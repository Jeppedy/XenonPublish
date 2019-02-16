/*
 * Project XenonPublish
 * Description:
 * Author:
 * Date:
 */

#include <DS18B20.h>

#define ONE_WIRE_BUS D4  // Data wire is plugged into pin 2 on the Arduino

const int      MAXRETRY          = 4;

int boardLed = D7; // This is the LED that is already on your device.
int iterCount;

DS18B20 sensors(ONE_WIRE_BUS, true);

//---------------------------------------------------------
void blinkLED( int LEDPin, int times ) {
  for( int i=0; i < times; i++) {
    if( i > 0 )
      delay( 200 );
    digitalWrite(LEDPin, HIGH);
    delay( 150 );
    digitalWrite(LEDPin, LOW);
  }
}

double getTemp(){
  double   celsius;
  double   fahrenheit;
  float _temp;
  int   i = 0;

  do {
    _temp = sensors.getTemperature();
  } while (!sensors.crcCheck() && MAXRETRY > i++);

  if (i < MAXRETRY) {
    celsius = _temp;
    fahrenheit = sensors.convertToFahrenheit(_temp);
  }
  else {
    celsius = fahrenheit = 999;
    Serial.println("Invalid reading");
  }
  return fahrenheit ;
}

//---------------------------------------------------------
void setup() {
  Serial.begin(14400);
 
  pinMode(boardLed, OUTPUT);
  pinMode(ONE_WIRE_BUS, INPUT);

  Particle.process();

  Particle.publish("AppVer", "XenonPublish-v1", 60, PRIVATE);

  blinkLED( boardLed, 5 );

  sensors.setResolution(TEMP_11_BIT);   // max = 12
}

void loop() {
  digitalWrite(boardLed, HIGH);
  Mesh.publish("PUBSUB", "xenon-pub");

  float temp1=0;
  float temp2=0;
  float temp3=0;
  char outBuffer[32] ; 


  //sensors.requestTemperatures(); // Send the command to get temperatures
  //temp1 = sensors.getTempFByIndex(1);  // Garage
  //temp2 = sensors.getTempFByIndex(0);  // Freezer
  temp1 = (float)getTemp();
  temp2 = 0;
  temp3 = 0; //Voltage
  
  if ( ++iterCount > 999 ) iterCount = 0;

  sprintf(outBuffer, "%2X,%03d,%04u,%04u,%04u",
    0xAA,    //    configs.nodeid & 0xff,
    iterCount,
    (int16_t)(temp1*10),
    (int16_t)(temp2*10),
    (int16_t)(temp3*10)
    );

  Serial.printf("outBuffer: %s len: %d \n",outBuffer, strlen(outBuffer));
  Mesh.publish("OutBuff", outBuffer);

  digitalWrite(boardLed, LOW);

  delay(10000);

}