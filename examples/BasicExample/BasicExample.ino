#include <Arduino.h>
#include "FSNextionLib.h" // Include your new library

// Create an object using the serial port (Serial2) to which the Nextion display is connected.
// Default Serial2 pins for ESP32: RX = 16, TX = 17
FSNextionLib myNextion(Serial2);

/**
 * @brief Callback function that will be called when a touch event is received from Nextion.
 * 
 * @param pageId The ID of the page where the event occurred.
 * @param componentId The ID of the component that triggered the event.
 * @param eventType The type of the event (0: Release, 1: Press).
 */
void handleTouchEvent(byte pageId, byte componentId, byte eventType) {
  Serial.print("Event Received! -> ");
  Serial.print("Page: ");
  Serial.print(pageId);
  Serial.print(", Component: ");
  Serial.print(componentId);
  Serial.print(", Event: ");
  Serial.println(eventType == 1 ? "Press" : "Release");

  // Example: When the button with ID 1 on page 0 is released...
  if (pageId == 0 && componentId == 1 && eventType == 0) { // eventType 0 = Release 1 = Press
    Serial.println("Custom Button (ID:1) released! Writing text to screen...");
    myNextion.setText("t0", "Button 1 Active");
  }
}

void setup() {
  // Start serial communication with the computer for debugging
  Serial.begin(115200);
  Serial.println("System starting...");

  // Initialize communication with Nextion
  myNextion.begin(115200);

  // Wait until the Nextion display is found
  Serial.println("Searching for Nextion display...");
  while (!myNextion.isConnected()) {
    Serial.println("Nextion not found. Retrying in 2 seconds...");
    delay(2000);
  }

  Serial.println("Nextion found and active!");

  // Register our event listener function to the library.
  myNextion.onTouch(handleTouchEvent);

  Serial.println("Event listener ready. Waiting for events from Nextion...");
  myNextion.setText("t0", "Ready");
}

void loop() {
  // In the main loop, only call the listen() function.
  // This does not block the loop and processes data from Nextion instantly.
  myNextion.listen();

  // You can add other non-blocking (delay-free) code here.
}
