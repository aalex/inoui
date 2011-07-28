#ifndef __TIMER_H__
#define __TIMER_H__
// FIXME: should include glib/glib.h
#include <clutter/clutter.h>

/**
 * Timer to time duration of stuff.
 *
 * Warning: you must start() this timer after clutter or GTK is running, otherwise it returns invalid time.
 */
class Timer
{
    public:
        Timer();
        ~Timer();
        /**
         * Returns how many seconds is elapsed since started.
         */
        double elapsed();
        /**
         * Resets the timer and starts counting.
         */
        void start();

    private:
        GTimer *timer_;
};

#endif

