#pragma once
#include "Arduino.h"

class Timer {
  private:
    unsigned long long start_time;
    unsigned long long end_time;
    int limit;    

  public:
    int limit_default = 8*60;
    Timer();
    Timer(int end_time);
    unsigned long fromStart();
    unsigned long fromEnd();
    int getEnd();
    void reset();
    void reset(int end_time);
    bool isEnd();
    void setEnd(int end_time);

};
