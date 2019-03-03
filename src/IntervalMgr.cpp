#include <application.h>
#include <IntervalMgr.h>

IntervalMgr::IntervalMgr( uint32_t intervalIn ) {  
        // _lastRun = 0;
        _interval = intervalIn ;
        // Serial.printf("Set interval to: %ld\n", intervalIn ) ;
    }

boolean IntervalMgr::isTimeToRun( boolean autoMarkAsRun) {
    if(millis() - _lastRun >= _interval || _lastRun == 0 ) {
        // Serial.printf("It's time!  Curr Millis %ld: Last Run %ld: Interval %ld\n", millis(), _lastRun, _interval ) ;
        if( autoMarkAsRun ) markAsRun(); 
        // Serial.printf("Curr Millis %ld: Last Run %ld\n", millis(), _lastRun ) ;
        return true ;
    }
    else {
        return false;
    }
}

void IntervalMgr::markAsRun() {
    // Serial.println("Marking as having run");
    // Serial.printf("Curr Millis %ld: Last Run %ld\n", millis(), _lastRun ) ;
    _lastRun = millis();
}
