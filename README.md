# FSNextionLib

A simple, lightweight, and non-blocking Arduino library for interfacing with Nextion HMI displays, built for the PlatformIO ecosystem.

This library was collaboratively developed with Google's Gemini.

## ⚠️ Disclaimer

This library is currently under active development. While functional, the API may change, and it should be used with caution in production environments.

## Features

- **Event-Driven:** Uses a non-blocking `listen()` model with callbacks to handle touch events without using `delay()`.
- **Easy Get/Set:** Simple methods to get and set values of text and number components.
- **Custom Pins:** Supports custom RX/TX pins for ESP32 hardware serial ports.
- **Connection Check:** Includes a robust `isConnected()` function to verify that the display is active before proceeding.
- **Lightweight:** Minimal dependencies and a straightforward design.

## Wiring

To connect your Nextion display to an ESP32, use one of the available Hardware Serial ports. This library was tested with `Serial2`.

### Default Pins (ESP32)

| ESP32 Pin      | Nextion Pin |
|----------------|-------------|
| GND            | GND         |
| 5V / 3.3V      | +5V         |
| GPIO 17 (TX2)  | RX          |
| GPIO 16 (RX2)  | TX          |

**Important:** The TX pin of the ESP32 goes to the RX pin of the Nextion, and the RX pin of the ESP32 goes to the TX pin of the Nextion.

## Installation

This library is designed for PlatformIO.

1.  Clone or download this repository.
2.  Place the entire `FSNextionLib` folder into the `lib/` directory of your PlatformIO project.
3.  PlatformIO's Library Dependency Finder will automatically detect and include the library in your project.

## API Reference

### `FSNextionLib(HardwareSerial& serial)`

**Description:** The library constructor.

-   `serial`: The `HardwareSerial` port your Nextion is connected to (e.g., `Serial1`, `Serial2`).

### `begin(long baud = 115200, int8_t rxPin = -1, int8_t txPin = -1)`

**Description:** Initializes the serial communication with the Nextion display. For ESP32, custom RX and TX pins can be specified.

-   `baud`: The baud rate configured in your Nextion project. Defaults to `115200`.
-   `rxPin` (Optional, ESP32 only): The RX pin for the serial communication.
-   `txPin` (Optional, ESP32 only): The TX pin for the serial communication.

**Usage:**
```cpp
// Use default pins for Serial2 (GPIO 16, 17)
myNextion.begin(115200);

// Use custom pins for Serial2 (e.g., RX=26, TX=27)
myNextion.begin(115200, 26, 27);
```

### `isConnected()`

**Description:** Checks if the display is connected and responsive.

-   **Returns:** `true` if a response is received from the display, `false` otherwise.

### `setText(const char* component, const char* txt)`

**Description:** Sets the `.txt` attribute of a component.

-   `component`: The name of the component (e.g., `"t0"`).
-   `txt`: The text to set.

### `setNumber(const char* component, int value)`

**Description:** Sets the `.val` attribute of a component.

-   `component`: The name of the component (e.g., `"n0"`, `"h0"`).
-   `value`: The integer value to set.

### `getText(const char* component)`

**Description:** Reads the `.txt` attribute from a component.

-   `component`: The name of the component (e.g., `"t0"`).
-   **Returns:** A `String` containing the component's text. Returns an empty string on timeout or error.

### `getNumber(const char* component)`

**Description:** Reads the `.val` attribute from a component.

-   `component`: The name of the component (e.g., `"n0"`, `"h0"`).
-   **Returns:** An `int` containing the component's value. Returns `-1` on timeout or error.

### `onTouch(TouchEventCallback callback)`

**Description:** Registers a callback function to be executed when a touch event occurs.

-   `callback`: The function to call. It must match the signature: `void myFunc(byte pageId, byte componentId, byte eventType)`.

### `listen()`

**Description:** The core of the event system. This function must be called continuously in your main `loop()` to process incoming events from the Nextion.

## Example Usage

The following example demonstrates how to use the event listener to handle button presses without blocking the main loop.

**Nextion Setup:**

1.  Create a button on Page 0.
2.  Set its component ID to `3`.
3.  In the "Touch Release Event" tab for the button, check the "Send Component ID" box.

**Code (`src/main.cpp`):**

```cpp
#include <Arduino.h>
#include "FSNextionLib.h" // Include your new library

// Create an object using the serial port (Serial2) to which the Nextion display is connected.
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

  // Example: When the button with ID 3 on page 0 is released...
  if (pageId == 0 && componentId == 3 && eventType == 0) { // eventType 0 = Release
    Serial.println("Custom Button (ID:3) released! Writing text to screen...");
    myNextion.setText("t0", "Button 3 Active");
  }
}

void setup() {
  // Start serial communication with the computer for debugging
  Serial.begin(115200);
  Serial.println("System starting...");

  // Initialize communication with Nextion using default pins for Serial2 (RX=16, TX=17)
  myNextion.begin(115200);
  
  // -- OR --
  
  // Initialize with custom pins (e.g., RX=26, TX=27)
  // myNextion.begin(115200, 26, 27);

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
```