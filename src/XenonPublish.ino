/*
 * Project XenonPublish
 * Description:
 * Author:
 * Date:
 */

//#include <DS18B20.h>

const boolean debug = false ;
const int ONE_WIRE_BUS = D4;  // Data wire is plugged into pin 2 on the Arduino
const int boardLed = D7; // This is the LED that is already on your device.
const int MAXRETRY = 3;
const char appVer[] = "v2.3-XenonPublish" ;

int iterCount;
int crcErrCount = 0;
int failedTempRead = 0;
int meshNotReady = 0;
char devShortID[4+1];
unsigned long old_time = 0;
const unsigned long DELAYINTERVAL = 5*60*1000;

DS18B20 sensors(ONE_WIRE_BUS, true);

//---------------------------------------------------------
void setup() {
  Serial.begin(14400);
 
  selectExternalMeshAntenna() ;

  pinMode(boardLed, OUTPUT);
  pinMode(ONE_WIRE_BUS, INPUT);

  // Particle.variable("devShortID", devShortID );
  Particle.variable("iterCount", iterCount );
  Particle.variable("crcErrCount", crcErrCount );
  Particle.variable("badTempRead", failedTempRead );
  Particle.variable("meshNotReady", meshNotReady );

  publishSafe("AppVer", appVer, PRIVATE);

  // Device Name Identification
  String devFullID = System.deviceID() ;
  Serial.printf("Full DevID: %s\n", devFullID.c_str() );
  devFullID.substring(strlen(devFullID)-4, strlen(devFullID)).toUpperCase().toCharArray(devShortID,4+1) ;

  sensors.setResolution(TEMP_11_BIT);   // max = 12
}

boolean publishSafe(const char *eventName, const char *data, PublishFlags flags) {
  boolean rtn = false;
  if ( Particle.connected() ) {
    //rtn = Particle.publish( eventName, data, flags );
  }
  else {
    Serial.println("Publish was skipped-disconnected");
  }
  return rtn;
}

void loop() {
  if(millis() - old_time >= DELAYINTERVAL || old_time == 0 ) {
    digitalWrite(boardLed, HIGH);
      publishTemp() ;
    digitalWrite(boardLed, LOW);
    old_time = millis();
  }
}

//---------------------------------------------------------

void selectExternalMeshAntenna() {
  #if (PLATFORM_ID == PLATFORM_ARGON)
    digitalWrite(ANTSW1, 1);
    digitalWrite(ANTSW2, 0);
  #elif (PLATFORM_ID == PLATFORM_BORON)
    digitalWrite(ANTSW1, 0);
  #else
    digitalWrite(ANTSW1, 0);
    digitalWrite(ANTSW2, 1);
  #endif
}

void publishTemp( void ) {
  float temp1=0;
  float temp2=0;
  float temp3=0;
  char outBuffer[32+1] ; 

  temp1 = (float)getTemp();
  temp2 = 0;
  temp3 = 0; //Voltage
  
  if( temp1 == 999 || temp2 == 999 || temp3 == 999) {
    char errMsg[128+1] ;
    sprintf(errMsg, "Invalid temp found! [%f] [%f] [%f]", temp1, temp2, temp3 ) ;
    Serial.println( errMsg );
    if( debug ) publishSafe("Error", errMsg, PRIVATE );
    failedTempRead++ ;
  }
  Particle.process() ;

  if ( ++iterCount > 999 ) iterCount = 0;

  sprintf(outBuffer, "%c%c,%03d,%04u,%04u,%04u",
    devShortID[2], devShortID[3],
    iterCount,
    (int16_t)(temp1*10),
    (int16_t)(temp2*10),
    (int16_t)(temp3*10)
    );

  if( Mesh.ready() ) {
    Mesh.publish("temps", outBuffer);
  } else {
    Serial.println("Publish was skipped-disconnected");
    meshNotReady++ ;
  }
  Serial.printf("outBuffer: %s len: %d \n",outBuffer, strlen(outBuffer));
  if( debug ) publishSafe("tempDBG", outBuffer, PRIVATE);
}

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
    tempOut = 999;
  }
  return tempOut ;
}
