/*
 * Project XenonPublish
 * Description:
 * Author:
 * Date:
 */

int boardLed = D7; // This is the LED that is already on your device.

// setup() runs once, when the device is first turned on.
void setup() {

  pinMode(boardLed, OUTPUT);

  Particle.process();

  int on_duration = 150 ;
  int blink_delay = 200 ;
  for( int i=0; i<3; i++) {
    digitalWrite(boardLed,HIGH);
    delay( on_duration );
    digitalWrite(boardLed,LOW);

    delay( blink_delay );
  }

  Particle.publish("AppVer", "XenonPublish-v1", 60, PRIVATE);
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  Mesh.publish("PUBSUB", "xenon-pub");

  delay(5000);

}