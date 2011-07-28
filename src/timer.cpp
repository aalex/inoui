#include "timer.h"

Timer::Timer()
{
    timer_ = g_timer_new();
    start();
}

double Timer::elapsed()
{
    gulong ms;
    return (double) g_timer_elapsed(timer_, &ms);
}

void Timer::start()
{
    g_timer_start(timer_);
}

Timer::~Timer()
{
    g_timer_destroy(timer_);
}
