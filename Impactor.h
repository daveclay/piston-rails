#include "Arduino.h"

#ifndef Impactor_h
#define Impactor_h

struct Note {
  int velocity;
  boolean on;
};

class Impactor {
  public:
    Impactor(int _pin);
    Note hit();
  private:
    int fsrAnalogPin;
    int fsrReading;      // the analog reading from the FSR resistor divider
    int triggerFsrReading;
    boolean noteIsOn;
    long numberOfTriggeringSamples;
    long numberOfOffSamples;
    long currentTime = 0;
    int velocity;
    long lastReadingTime = -1;
    long noteOnTime = 0;

    void reset();
};

#endif
