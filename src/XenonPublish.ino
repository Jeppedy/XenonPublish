/*
 * Project XenonPublish
 * Description:
 * Author:
 * Date:
 */

#include <DS18B20.h>
#include <IntervalMgr.h>

const boolean debug = true ;
const int ONE_WIRE_BUS_1 = D4; 
const int ONE_WIRE_BUS_2 = D5; 
const int ONE_WIRE_BUS_3 = D6; 
const int boardLed = D7; // This is the LED that is already on your device.
const int MAXRETRY = 3;
const char appVer[] = "v3.0-XenonPublish" ;

#define TEMPPUB_DELAY 1*60*1000
#define PANICCHECK_DELAY 3*60*1000

int iterCount;
int crcErrCount = 0;
int failedTempRead = 0;
int meshNotReady = 0;
char devShortID[4+1];

DS18B20 tempSensor1(ONE_WIRE_BUS_1, true);
DS18B20 tempSensor2(ONE_WIRE_BUS_2, true);
DS18B20 tempSensor3(ONE_WIRE_BUS_3, true);

//---------------------------------------------------------
void setup() {
  Serial.begin(14400);
 
  selectExternalMeshAntenna() ;

  pinMode(boardLed, OUTPUT);
  digitalWrite(boardLed, LOW);

  pinMode(ONE_WIRE_BUS_1, INPUT);
  pinMode(ONE_WIRE_BUS_2, INPUT);
  pinMode(ONE_WIRE_BUS_3, INPUT);

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

  tempSensor1.setResolution(TEMP_11_BIT);   // max = 12
  tempSensor2.setResolution(TEMP_11_BIT);   // max = 12
  tempSensor3.setResolution(TEMP_11_BIT);   // max = 12
}

boolean publishSafe(const char *eventName, const char *data, PublishFlags flags) {
  boolean rtn = false;
  if ( Particle.connected() )
    rtn = Particle.publish( eventName, data, flags );
  else
    Serial.println("Publish was skipped-disconnected");
  return rtn;
}

void loop() {
  static IntervalMgr tempTimer( TEMPPUB_DELAY ) ;
  static IntervalMgr panicTimer( PANICCHECK_DELAY ) ;

  if( debug ) publishSafe("DBG", "Publish Temps", PRIVATE );
  if( tempTimer.isTimeToRun()  ) publishTemp() ;
  if( debug ) publishSafe("DBG", "Check for Particle Cloud", PRIVATE );
  if( panicTimer.isTimeToRun() ) checkForDisconnectPanic() ;

  if( debug ) publishSafe("DBG", "End of Loop ", PRIVATE );
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

double getTemp( DS18B20 sensor, int maxRetries = 5 ){
  double tempOut = 999;
  float _temp;
  int   i = 0;

  do {
//    Serial.println("Obtaining reading");
    _temp = sensor.getTemperature();
    if(!sensor.crcCheck()) {
      crcErrCount++ ;
    } else {
      break ;
    }
  } while ( maxRetries > ++i);

  if (i < maxRetries) {
    tempOut = sensor.convertToFahrenheit(_temp);
  }
  else {
    Serial.println("Invalid reading");
    tempOut = 999;
  }
  return tempOut ;
}

boolean publishTemp( void ) {
  boolean rtn = true ;
  float temp1=0;
  float temp2=0;
  float temp3=0;
  char outBuffer[32+1] ; 

  temp1 = (float)getTemp( tempSensor1 );
  temp2 = (float)getTemp( tempSensor2 );
  temp3 = 0; //Voltage
  
  if( temp1 == 999 || temp2 == 999 || temp3 == 999) {
    char errMsg[128+1] ;
    sprintf(errMsg, "Invalid temp found! [%f] [%f] [%f]", temp1, temp2, temp3 ) ;
    Serial.println( errMsg );
    if( debug ) publishSafe("Error", errMsg, PRIVATE );
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

  Serial.printf("outBuffer: %s len: %d \n",outBuffer, strlen(outBuffer));

  if( Mesh.ready() ) {
    Mesh.publish("temps", outBuffer);
    if( debug ) publishSafe("DBG", outBuffer, PRIVATE);
  } else {
    Serial.println("Publish was skipped-Mesh not connected");
    meshNotReady++ ;
    rtn = false ;
  }

  return( rtn ) ;
}

void blinkLED( int LEDPin, int times ) {
  for( int i=0; i < times; i++) {
    if( i > 0 )
      delay( 300 );
    digitalWrite(LEDPin, HIGH);
    delay( 250 );
    digitalWrite(LEDPin, LOW);
  }
}

void checkForDisconnectPanic() {
  static IntervalMgr disconnectTimer( 3*60*1000 ) ;

  if (Particle.connected()) {
    digitalWrite(boardLed, LOW);
    disconnectTimer.markAsRun() ;  // we are connected, so reset the timer
  } else {
    digitalWrite(boardLed, HIGH);
    Serial.printlnf("[%s] Particle Cloud is not accessible", Time.timeStr().c_str() );
    // publishSafe("DBG", "No Cloud", PRIVATE);  // Might be completely worthless, as no Cloud connection exists!

    if ( disconnectTimer.isTimeToRun() ) {
      blinkLED( boardLed, 12 ) ;
      Serial.printlnf("[%s] Resetting, to regain access to Particle Cloud", Time.timeStr().c_str() );
      Serial.flush() ;
      delay(100);

      // we have been disconnected for too long, so let's reset everything!
      #if Wiring_Wifi
        Wifi.off();
        delay(1000);
      #endif
      System.reset();
    }
  }
}

