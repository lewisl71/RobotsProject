#include <Wire.h>
#include <U8glib.h>
#include "RTClib.h"

// OLED Display and RTC
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE);
RTC_DS1307 rtc;

// Pins for sensors and relays
const int NUM_SENSORS = 4;
int sensorPins[NUM_SENSORS] = {A0, A1, A2, A3};
int relayPins[NUM_SENSORS] = {6, 8, 9, 10};
int pumpPin = 4;
int buttonPin = 12;

// Sensor values
int sensorValues[NUM_SENSORS] = {0};

// State flags
bool pumpState = false;
bool relayStates[NUM_SENSORS] = {false};

// Thresholds
const int DRY_THRESHOLD = 30; // Below this, soil is considered dry
const int WET_THRESHOLD = 55; // Above this, soil is considered wet

void setup() {
  Serial.begin(9600);
  Wire.begin();
  rtc.begin();

  // Pin setup
  for (int i = 0; i < NUM_SENSORS; i++) {
    pinMode(sensorPins[i], INPUT);
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], LOW); // Ensure all relays are initially OFF
  }
  pinMode(pumpPin, OUTPUT);
  digitalWrite(pumpPin, LOW); // Ensure the pump is initially OFF
  pinMode(buttonPin, INPUT);

  // Display initialization
  u8g.firstPage();
  do {
    u8g.setFont(u8g_font_6x10);
    u8g.drawStr(10, 30, "System Initializing...");
  } while (u8g.nextPage());
  delay(2000);
}

void loop() {
  readSensors();
  controlRelaysAndPump();
  displayStatus();
}

// Function to read all sensor values
void readSensors() {
  for (int i = 0; i < NUM_SENSORS; i++) {
    sensorValues[i] = map(analogRead(sensorPins[i]), 600, 360, 0, 100);
    sensorValues[i] = max(sensorValues[i], 0); // Ensure no negative values
    Serial.print("Sensor ");
    Serial.print(i);
    Serial.print(" Value: ");
    Serial.println(sensorValues[i]);
  }
}

// Function to control relays and pump based on sensor values
void controlRelaysAndPump() {
  bool anyRelayActive = false;

  for (int i = 0; i < NUM_SENSORS; i++) {
    if (sensorValues[i] < DRY_THRESHOLD) { 
      // Soil is dry, turn ON relay
      digitalWrite(relayPins[i], HIGH);
      relayStates[i] = true;
      anyRelayActive = true;
    } else if (sensorValues[i] > WET_THRESHOLD) { 
      // Soil is wet, turn OFF relay
      digitalWrite(relayPins[i], LOW);
      relayStates[i] = false;
    }
  }

  // Pump control
  if (anyRelayActive && !pumpState) {
    digitalWrite(pumpPin, HIGH); // Turn ON the pump
    pumpState = true;
    Serial.println("Pump ON");
  } else if (!anyRelayActive && pumpState) {
    digitalWrite(pumpPin, LOW); // Turn OFF the pump
    pumpState = false;
    Serial.println("Pump OFF");
  }
}

// Function to display the current status on the OLED
void displayStatus() {
  u8g.firstPage();
  do {
    u8g.setFont(u8g_font_6x10);
    u8g.drawStr(0, 10, "Soil Moisture Levels:");

    for (int i = 0; i < NUM_SENSORS; i++) {
      char buffer[20];
      snprintf(buffer, sizeof(buffer), "Sensor %d: %d%%", i + 1, sensorValues[i]);
      u8g.drawStr(0, 20 + i * 10, buffer);
    }

    u8g.drawStr(0, 60, pumpState ? "Pump: ON" : "Pump: OFF");
  } while (u8g.nextPage());
}
