/*
 * Project XenonPublish
 * Description:
 * Author:
 * Date:
 */

#include <DS18B20.h>

#define ONE_WIRE_BUS D4  // Data wire is plugged into pin 2 on the Arduino

const int MAXRETRY = 5;

int boardLed = D7; // This is the LED that is already on your device.
int iterCount;
int crcErrCount = 0;
int failedTempRead = 0;
char devShortID[4+1];

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
  double tempOut = 999;
  float _temp;
  int   i = 0;

  do {
//    Serial.println("Obtaining reading");
    _temp = sensors.getTemperature();
    if(!sensors.crcCheck()) {
      crcErrCount++ ;
    } else {
      break ;
    }
  } while ( MAXRETRY > i++);

  if (i < MAXRETRY) {
    tempOut = sensors.convertToFahrenheit(_temp);
  }
  else {
    Serial.println("Invalid reading");
  }
  return tempOut ;
}

//---------------------------------------------------------
void setup() {
  Serial.begin(14400);
 
  pinMode(boardLed, OUTPUT);
  pinMode(ONE_WIRE_BUS, INPUT);

  Particle.variable("devShortID", devShortID );
  Particle.variable("iterCount", iterCount );
  Particle.variable("crcErrCount", crcErrCount );
  Particle.variable("badTempRead", failedTempRead );

  Particle.publish("AppVer", "XenonPublish-v1", 60, PRIVATE);

  // Device Name Identification
  String devFullID = System.deviceID() ;
  Serial.printf("Full DevID: %s\n", devFullID.c_str() );
  devFullID.substring(strlen(devFullID)-4, strlen(devFullID)).toUpperCase().toCharArray(devShortID,4+1) ;
  //Serial.println( devShortID );

  sensors.setResolution(TEMP_11_BIT);   // max = 12

  // blinkLED( boardLed, 5 );
}

void loop() {
  digitalWrite(boardLed, HIGH);

  float temp1=0;
  float temp2=0;
  float temp3=0;
  char outBuffer[32] ; 

  temp1 = (float)getTemp();
  temp2 = 0;
  temp3 = 0; //Voltage
  
  if( temp1 == 999 || temp2 == 999 || temp3 == 999) {
    Serial.printlnf("Invalid temp found! [%f] [%f] [%f]", temp1, temp2, temp3);
    failedTempRead++ ;
  }

  if ( ++iterCount > 999 ) iterCount = 0;

  sprintf(outBuffer, "%c%c,%03d,%04u,%04u,%04u",
    devShortID[2], devShortID[3],
    iterCount,
    (int16_t)(temp1*10),
    (int16_t)(temp2*10),
    (int16_t)(temp3*10)
    );

  Mesh.publish("temps", outBuffer);
  Serial.printf("outBuffer: %s len: %d \n",outBuffer, strlen(outBuffer));
  // Particle.publish("tempDBG", outBuffer, 60, PRIVATE);

  digitalWrite(boardLed, LOW);

  delay(10 * 1000);
}