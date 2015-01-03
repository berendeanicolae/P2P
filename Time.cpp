#include "Time.h"

//gettimeofday sys/time.h

/**
    Return the number of miliseconds since the program started.
    For now it only has +-1000 miliseconds precision.
*/

time_t getTicks(){
    return time(0);
}

time_t getTicksSince(time_t since){
    return time(0)-since;
}
