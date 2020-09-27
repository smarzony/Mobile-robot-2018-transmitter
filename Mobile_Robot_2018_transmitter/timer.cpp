#include "timer.h"

Timer::Timer()
{
    this->start_time = millis();
}

Timer::Timer(int end_time)
{
    this->start_time = millis();
    this->limit = end_time;
    this->end_time = this->start_time + (unsigned long)(this->limit) * 1000;
}

unsigned long Timer::fromStart()
{
    return millis() - this->start_time;
}

unsigned long Timer::fromEnd()
{
    if (this->end_time > 0)
        return millis() - this->end_time;
    else
        return 0;
}

int Timer::getEnd()
{
    return this->limit;
}

void Timer::reset()
{
    this->start_time = millis();
    this->end_time = 0;
}

void Timer::reset(int end_time)
{
    this->limit = end_time;
    this->start_time = millis();
    this->end_time = this->start_time + (unsigned long)(this->limit) * 1000;
}

bool Timer::isEnd()
{
    if (millis() > this->end_time && this->end_time > 0)
    return true;
    else
    return false;
}

void Timer::setEnd(int end_time)
{
    this->limit = end_time;
    this->end_time = this->start_time + (unsigned long)(this->limit) * 1000;
}

