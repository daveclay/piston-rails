#include "Impactor.h"

Impactor kick(0);
Impactor snare(1);
int handle = 0;
Note kickNote;
Note snareNote;

int lastPatchSent = -1;
long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 50;    // the debounce time; increase if the output flickers
int currentPatchPressed = -1;


void kickOn(int velocity) {
  Serial.print("T,ON,KICK");
  Serial.print(",");
  Serial.print(velocity);
  Serial.print(",");
  Serial.println(handle);
}

void kickOff() {
  Serial.println("T,OFF,KICK");
}

void snareOn(int velocity) {
  Serial.print("T,ON,SNARE");
  Serial.print(",");
  Serial.print(velocity);
  Serial.print(",");
  Serial.println(handle);
}

void snareOff() {
  Serial.println("T,OFF,SNARE");
}


void setup(void) {
  Serial.begin(9600);
  pinMode(13, OUTPUT);
  for (int i = 12; i >= 5; i--) {
    pinMode(i, INPUT_PULLUP);
  }
}

void loop(void) {
  handle = analogRead(2);

  kickNote = kick.hit();
  if (kickNote.velocity > -1) {
    if (kickNote.on) {
      kickOn(kickNote.velocity);
    } else {
      kickOff();
    }
  }

  snareNote = snare.hit();
  if (snareNote.velocity > -1) {
    if (snareNote.on) {
      snareOn(snareNote.velocity);
    } else {
      snareOff();
    }
  }

  for (int i = 12; i >= 5; i--) {
    int reading = digitalRead(i);
    if (reading == 0 && lastPatchSent != i) {
      lastDebounceTime = millis();
      currentPatchPressed = i;
    }
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (currentPatchPressed != lastPatchSent) {
      Serial.print("PATCH,");
      Serial.println(13 - currentPatchPressed);
      lastPatchSent = currentPatchPressed;
    }
  }
}

