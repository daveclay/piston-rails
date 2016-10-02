#include "Arduino.h"
#include "Impactor.h"

#define CAPTURE_TIME 10
#define FORCE_THRESHOLD 500
#define OFF_THRESHOLD 200
#define NUMBER_OF_TRIGGER_SAMPLES_THRESHOLD 2
#define NUMBER_OF_OFF_SAMPLES_THRESHOLD 2
#define NOTE_OFF_DELAY 10

Impactor::Impactor(int _pin) {
  fsrAnalogPin = _pin;
}

Note Impactor::hit() {
  Note note;
  note.velocity = -1;
  fsrReading = 1024 - analogRead(fsrAnalogPin);
  currentTime = millis();

  if (fsrReading > FORCE_THRESHOLD) {
    triggerFsrReading = fsrReading;
    numberOfTriggeringSamples++;
    if (lastReadingTime < 0) {
      lastReadingTime = currentTime;
    }
    velocity = min(1024, map(fsrReading, FORCE_THRESHOLD, 1024, 0, 1024));
    digitalWrite(13, HIGH);
  }

  if (!noteIsOn && velocity > 0) {
    if ((currentTime - lastReadingTime) > CAPTURE_TIME) {
      if (numberOfTriggeringSamples < NUMBER_OF_TRIGGER_SAMPLES_THRESHOLD) {
        reset();
      } else {
        noteIsOn = true;
        noteOnTime = currentTime;

        note.on = true;
        note.velocity = velocity;
        return note;
        /*
          Serial.print("ON\t\t\tVelocity = ");
          Serial.print(velocity);
          Serial.print("\t triggerFsrReading ");
          Serial.print(triggerFsrReading);
          Serial.print("\t numberOfTriggeringSamples ");
          Serial.println(numberOfTriggeringSamples);
        */
      }
    }
  } else if (noteIsOn) {
    if (fsrReading <= OFF_THRESHOLD) {
      numberOfOffSamples++;
      if (numberOfOffSamples > NUMBER_OF_OFF_SAMPLES_THRESHOLD && (currentTime - noteOnTime) > NOTE_OFF_DELAY) {
        /*
          Serial.print("\t\tOFF\tnumberOfOffSamples ");
          Serial.println(numberOfOffSamples);
        */
        reset();
        note.velocity = 0;
        note.on = false;
        return note;
      }
    }
  }

  note.velocity = -1;
  return note;
}

void Impactor::reset() {
  digitalWrite(13, LOW);
  noteIsOn = false;
  lastReadingTime = -1;
  velocity = 0;
  triggerFsrReading = 0;
  numberOfTriggeringSamples = 0;
  numberOfOffSamples = 0;
}


