# FSNextionLib
A simple, lightweight, and non-blocking Arduino library for interfacing with Nextion HMI displays—built for the PlatformIO ecosystem and designed for clarity, speed, and ease of use.

# Disclaimer
This library is under active development. While fully functional, the API may evolve. Use with caution in production environments.

# Features
- **Event-Driven:** Non-blocking listen() model with callback support—no delay() required.
- **Intuitive API:** Simple methods like txt(), val(), click, vis() for direct component control.
- **Custom Pins:** Supports RX/TX remapping for ESP32 hardware serial ports.
- **Connection Check:** Robust isConnected() method to verify display readiness.
- **Lightweight:** Minimal dependencies and fast compile times.

# Wiring
To connect your Nextion display to an ESP32, use one of the available hardware serial ports. This library was tested with ***Serial2***.

### Default Pins (ESP32)

| ESP32 Pin      | Nextion Pin |
|----------------|-------------|
| GND            | GND         |
| 5V / 3.3V      | +5V         |
| GPIO 17 (TX2)  | RX          |
| GPIO 16 (RX2)  | TX          |

**Important:** TX from ESP32 goes to RX on Nextion, and RX from ESP32 goes to TX on Nextion.

# Installation
This library is designed for PlatformIO.
1. 	Clone or download this repository.
2. 	Place the ***FSNextionLib*** folder into your project’s ***lib/*** directory or add ***lib_deps = 	https://github.com/fsadak/FSNextionLib.git*** to your platformio.ini file.
3. 	PlatformIO will automatically detect and include the library.

# API Overview
**Initialization**
<code>
FSNextionLib myNextion(Serial2); // Use Serial2 or any HardwareSerial

void setup() {
  myNextion.begin(115200);           // Default pins (GPIO 16, 17)
  // myNextion.begin(115200, 26, 27); // Optional custom pins
}
</code>

# Connection Check
<code>
if (myNextion.isConnected()) {
  Serial.println("Nextion is ready!");
}
</code>

# Component Control

## Text
<code>
myNextion.txt("t0", "Hello World");
String currentText = myNextion.txt("t0");
</code>

## Number
<code>
myNextion.val("n0", 42);
int currentValue = myNextion.val("n0");
</code>

## Visibility & Color
<code>
myNextion.vis("b0", false);      // Hide component
myNextion.vis("b0", true);       // Show component
myNextion.bco("b0", 63488);      // Set background color (RED)
</code>

## Touch Simulation
<code>
myNextion.click("b0");           // Simulate Touch Press
myNextion.release("b0");         // Simulate Touch Release
</code>

## Refresh
<code>
myNextion.refresh("b0");         // Redraw component
</code>

## Enable/Disable Touch Events
Nextion components respond to user interaction through Touch Press and Touch Release events. These can be programmatically enabled or disabled using the tsw command.

FSNextionLib provides two intuitive methods:
<code>
myNextion.enable("b0", true);   // Enables touch events for 'b0'
myNextion.touch("b0", false);   // Disables touch events for 'b0'
</code>

# What's the difference?
Both methods send the same command (tsw b0,1 or tsw b0,0), but they serve different semantic purposes:
• enable() is used to control whether a component is functionally active—ideal for disabling buttons until a condition is met.
• touch() is used to suppress or allow touch responsiveness directly—useful during transitions or animations.

Note: Disabling touch does not affect visibility. A component may still be visible but unresponsive.

## Page Navigation
<code>
myNextion.page("main");         // Switch to page 'main'
</code>

## Touch Event Listener
Register a callback to handle touch events:
<code>
myNextion.onTouch([](byte pageId, byte componentId, byte eventType) {
  Serial.printf("Touch: page=%d, component=%d, event=%d\n", pageId, componentId, eventType);
});
</code>

Call listen() inside your main loop:
<code>
void loop() {
  myNextion.listen(); // Non-blocking event processing
}
</code>

## Touch Event Listener (Filtered by Page and/or Component)
You can restrict the touch event handler to only respond to events from a specific page and/or component. For example, to handle events only from page ID 1, use the following pattern:

<code>
myNextion.onTouch([](byte pageId, byte componentId, byte eventType) {
  if (pageId != 1) return; // Ignore events from other pages

  Serial.printf("Touch on Page 1 → Component: %d, Event: %s\n",
                componentId,
                eventType == 1 ? "Press" : "Release");

  if (componentId == 2 && eventType == 0) { //1 = Press, 0 = Release
    myNextion.txt("t0", "Button on Page 1 Released");
  }
});
</code>

# Example Project

## Nextion Setup:
1. 	Create a button on Page 0 with ID 1.
2. 	Enable "Send Component ID" in its Touch Release Event.
3. 	Create a textbox named t0.

## Code:
```cpp
#include <Arduino.h>
#include "FSNextionLib.h"

FSNextionLib myNextion(Serial2);

void handleTouchEvent(byte pageId, byte componentId, byte eventType) {
  if (pageId == 0 && componentId == 1 && eventType == 0) {
    myNextion.txt("t0", "Button 1 Active");
  }
}

void setup() {
  Serial.begin(115200);
  myNextion.begin(115200);

  while (!myNextion.isConnected()) {
    Serial.println("Waiting for Nextion...");
    delay(2000);
  }

  myNextion.onTouch(handleTouchEvent);
  myNextion.txt("t0", "Ready");
}

void loop() {
  myNextion.listen();
}

### Feedback & Contributions
Pull requests and suggestions are welcome! Whether you're optimizing performance, adding features, or improving documentation—your input helps make FSNextionLib better for everyone.