#include "Impactor.h"
#include <Wire.h> // Include the I2C library (required)
#include <SparkFunSX1509.h> // Include SX1509 library

Impactor kick(0);
Impactor snare(1);
int handle = 0;
Note kickNote;
Note snareNote;

long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 50;    // the debounce time; increase if the output flickers
int currentPatchPressed = 0;
int lastPatchSent = 0;

int reading = HIGH;

SX1509 io; // Create an SX1509 object

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
  setupSX1509();
  for (int i = 12; i >= 5; i--) {
    pinMode(i, INPUT_PULLUP);
  }
}

void loop(void) {
  handle = 1023 - analogRead(2);

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

  for (int i = 0; i < 8; i++) {
    reading = io.digitalRead(i);
    if (reading == 0 && lastPatchSent != i) {
      lastDebounceTime = millis();
      currentPatchPressed = i;
    }
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (currentPatchPressed != lastPatchSent) {
      Serial.print("PATCH,");
      Serial.println(currentPatchPressed);
      lastPatchSent = currentPatchPressed;
    }
  }

  allOtherPinsTo(lastPatchSent + 8, LOW);
  io.digitalWrite(lastPatchSent + 8, HIGH);
}

void setupSX1509()
{
  digitalWrite(13, LOW); // Start it as low
  // Call io.begin(<I2C address>) to initialize the I/O
  // expander. It'll return 1 on success, 0 on fail.
  if (!io.begin(0x3E))
  {
    // If we failed to communicate, turn the pin 13 LED on
    digitalWrite(13, HIGH);
    while (1)
      ; // And loop forever.
  }

  // Call io.pinMode(<pin>, <mode>) to set any SX1509 pin as
  // either an INPUT, OUTPUT, INPUT_PULLUP, or ANALOG_OUTPUT

  for (int i = 0; i < 8; i++) {
    io.pinMode(i + 8, OUTPUT);
    io.pinMode(i, INPUT_PULLUP);
  }

  // Blink the LED a few times before we start:
  for (int i = 0; i < 5; i++)  {
    // Use io.digitalWrite(<pin>, <LOW | HIGH>) to set an
    // SX1509 pin either HIGH or LOW:
    allPinsTo(HIGH);
    delay(100);
    allPinsTo(LOW);
    delay(100);
  }
}

void allOtherPinsTo(int pin, int value) {
  for (int i = 8; i < 16; i++) {
    if (i != pin) {
      io.digitalWrite(i, value);
    }
  }
}

void allPinsTo(int value) {
  allOtherPinsTo(-1, value);
}

