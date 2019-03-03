#ifndef _INTERVALMGR_H_
#define _INTERVALMGR_H_

#include <application.h>

class IntervalMgr {
public:
    IntervalMgr( uint32_t intervalIn ) ;

    boolean isTimeToRun( boolean autoMarkAsRun = true) ;
    void markAsRun() ;

private:
    uint32_t _interval ;
    uint32_t _lastRun ;
} ;

#endif

