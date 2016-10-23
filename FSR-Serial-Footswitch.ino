#include "Impactor.h"
#include <Wire.h> // Include the I2C library (required)
#include <SparkFunSX1509.h> // Include SX1509 library

// Arduino Pins (not SX1509!)
const byte ARDUINO_INT_PIN = 2; // SX1509 int output to D2
const byte SX1509_ADDRESS = 0x3E;  // SX1509 I2C address

const byte FINGER_PIN = 3;

SX1509 io; // Create an SX1509 object

Impactor kick(0);
Impactor snare(1);
int handle = 0;
int finger = 0;
Note kickNote;
Note snareNote;

long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 50;    // the debounce time; increase if the output flickers
int currentPatchPressed = -1;
int lastPatchSent = 0;
volatile bool footswitchButtonPressed = false;
int reading = HIGH;


void kickOn(int velocity) {
  Serial.println("T,ON,KICK," + String(velocity) + "," + String(handle) + "," + String(finger));
}

void kickOff() {
  Serial.println("T,OFF,KICK");
}

void snareOn(int velocity) {
  Serial.println("T,ON,SNARE," + String(velocity) + "," + String(handle) + "," + String(finger));
}

void snareOff() {
  Serial.println("T,OFF,SNARE");
}


void setup(void) {
  Serial.begin(9600);
  pinMode(13, OUTPUT);
  pinMode(FINGER_PIN, INPUT_PULLUP);
  setupSX1509();
}

void loop(void) {
  handle = 1023 - analogRead(2);
  finger = 1 - digitalRead(FINGER_PIN);

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

  if (footswitchButtonPressed) // If the button() ISR was executed
  {
    // read io.interruptSource() find out which pin generated
    // an interrupt and clear the SX1509's interrupt output.
    unsigned int intStatus = io.interruptSource();
    for (int i = 0; i < 8; i++) {
      if (intStatus & 1 << i) {
        currentPatchPressed = i;
      }
    }

    if (lastPatchSent != currentPatchPressed) {
      Serial.println("PATCH," + String(currentPatchPressed + 1));
      lastPatchSent = currentPatchPressed;
      currentPatchPressed = -1;
    }

    allOtherPinsTo(lastPatchSent + 8, LOW);
    io.digitalWrite(lastPatchSent + 8, HIGH);

    footswitchButtonPressed = false; // Clear the buttonPressed flag
  }
}

void setupSX1509()
{
  digitalWrite(13, LOW); // Start it as low
  // Call io.begin(<I2C address>) to initialize the I/O
  // expander. It'll return 1 on success, 0 on fail.
  if (!io.begin(SX1509_ADDRESS))
  {
    Serial.println("Failed to communicate.");
    while (1) ;
  }

  // Call io.pinMode(<pin>, <mode>) to set any SX1509 pin as
  // either an INPUT, OUTPUT, INPUT_PULLUP, or ANALOG_OUTPUT
  // The SX1509 has built-in debounce features, so a single
  // button-press doesn't accidentally create multiple ints.
  // Use io.debounceTime(<time_ms>) to set the GLOBAL SX1509
  // debounce time.
  // <time_ms> can be either 0, 1, 2, 4, 8, 16, 32, or 64 ms.
  io.debounceTime(1); // Set debounce time

  for (int i = 0; i < 8; i++) {
    io.pinMode(i + 8, OUTPUT);

    io.pinMode(i, INPUT_PULLUP);
    io.enableInterrupt(i, FALLING);
    // After configuring the debounce time, use
    // debouncePin(<pin>) to enable debounce on an input pin.
    io.debouncePin(i); // Enable debounce
  }

  // Don't forget to configure your Arduino pins! Set the
  // Arduino's interrupt input to INPUT_PULLUP. The SX1509's
  // interrupt output is active-low.
  pinMode(ARDUINO_INT_PIN, INPUT_PULLUP);

  // Attach an Arduino interrupt to the interrupt pin. Call
  // the button function, whenever the pin goes from HIGH to
  // LOW.
  attachInterrupt(digitalPinToInterrupt(ARDUINO_INT_PIN), footswitchButton, FALLING);

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

// button() is an Arduino interrupt routine, called whenever
// the interrupt pin goes from HIGH to LOW.
void footswitchButton() {
  footswitchButtonPressed = true; // Set the buttonPressed flag to true
  // We can't do I2C communication in an Arduino ISR. The best
  // we can do is set a flag, to tell the loop() to check next
  // time through.
}

