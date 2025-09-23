#include "FSNextionLib.h"

// Constructor
FSNextionLib::FSNextionLib(HardwareSerial& serial) : _serial(serial) {}

// Initializes communication. Can use custom pins for ESP32.
void FSNextionLib::begin(long baud, int8_t rxPin, int8_t txPin) {
    #if defined(ESP32)
        if (rxPin > -1 && txPin > -1) {
            _serial.begin(baud, SERIAL_8N1, rxPin, txPin);
        } else {
            _serial.begin(baud);
        }
    #else
        // For non-ESP32 boards, ignore pin arguments as begin() doesn't support them.
        _serial.begin(baud);
    #endif
}

// Sends command termination characters
void FSNextionLib::endCommand() {
    _serial.write(0xFF);
    _serial.write(0xFF);
    _serial.write(0xFF);
}

// General command sending function
void FSNextionLib::sendCommand(const char* cmd) {
    _serial.print(cmd);
    endCommand();
}

// Writes text to a text component
void FSNextionLib::setText(const char* component, const char* txt) {
    String cmd = String(component) + ".txt=\"" + String(txt) + "\"";
    _serial.print(cmd);
    endCommand();
}

// Writes a number to a number component
void FSNextionLib::setNumber(const char* component, int value) {
    String cmd = String(component) + ".val=" + String(value);
    _serial.print(cmd);
    endCommand();
}

// Checks if the Nextion display is connected and active.
bool FSNextionLib::isConnected() {
    // Clear any old data from the serial buffer
    while (_serial.available()) {
        _serial.read();
    }

    // Send a universal command that will force a response.
    // This command asks if the device is in sleep mode and always produces a response.
    _serial.print("get sleep");
    endCommand();

    // Check if any response arrives within a short time.
    long startTime = millis();
    while (millis() - startTime < 300) { // 300ms timeout is sufficient
        if (_serial.available() > 0) {
            // Response received, clear the buffer and return success.
            while (_serial.available()) {
                _serial.read();
            }
            return true; // Any response is proof of connection.
        }
    }

    // If it times out, it means there is no connection.
    return false;
}

// Reads the text (.txt) value of a component.
String FSNextionLib::getText(const char* component) {
    String cmd = "get " + String(component) + ".txt";
    sendCommand(cmd.c_str());

    String response = "";
    long startTime = millis();
    while (millis() - startTime < 500) { // 500ms timeout
        if (_serial.available()) {
            char c = _serial.read();
            if (c == 0x70) { // Text response start byte
                // Read until the terminator (0xFF 0xFF 0xFF) is found
                while (millis() - startTime < 500) {
                    if (_serial.available()) {
                        char nextChar = _serial.read();
                        if (nextChar == 0xFF) { // First byte of the terminator
                            _serial.read(); // Read the second
                            _serial.read(); // Read the third
                            return response; // Return the read text
                        } else {
                            response += nextChar;
                        }
                    }
                }
            }
        }
    }
    return ""; // Return an empty string on timeout or error
}

// Reads the numeric (.val) value of a component.
int FSNextionLib::getNumber(const char* component) {
    String cmd = "get " + String(component) + ".val";
    sendCommand(cmd.c_str());

    long startTime = millis();
    while (millis() - startTime < 500) { // 500ms timeout
        if (_serial.available()) {
            char c = _serial.read();
            if (c == 0x71) { // Numeric response start byte
                // Read the 4-byte (little-endian) numeric value
                if (_serial.available() >= 7) { // 4 bytes of data + 3 bytes of terminator
                    byte buffer[4];
                    _serial.readBytes(buffer, 4);
                    // Clear the terminator from the buffer
                    for(int i=0; i<3; i++) _serial.read();
                    // Combine the value and return
                    return buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);
                }
            }
        }
    }
    return -1; // Return -1 on timeout or error
}

// Registers the callback function for the touch event.
void FSNextionLib::onTouch(TouchEventCallback callback) {
    _touchCallback = callback;
}

// Listens for incoming data and processes events.
void FSNextionLib::listen() {
    // Check if there is enough data for a full event message (usually 7 bytes)
    if (_serial.available() < 7) {
        return;
    }

    // Read the first incoming byte
    byte firstByte = _serial.read();

    // If it's not a touch event (0x65), ignore it for now.
    if (firstByte != 0x65) {
        // Code for other event types (0x70, 0x71, etc.) can be added here.
        // For now, let's clear the buffer and exit to prevent incorrect accumulation.
        while(_serial.available()) {
            _serial.read();
        }
        return;
    }

    // If it's a touch event, read the rest of the data
    byte pageId = _serial.read();
    byte componentId = _serial.read();
    byte eventType = _serial.read(); // 0 = Release, 1 = Press

    // Clear the 3 terminator bytes from the buffer
    _serial.read();
    _serial.read();
    _serial.read();

    // If a callback function has been registered, call it.
    if (_touchCallback) {
        _touchCallback(pageId, componentId, eventType);
    }
}

bool FSNextionLib::discoverComponents() {
    components.clear();
    sendCommand("tm0.en=1");

    String buffer = "";
    bool collecting = false;

    while (true) {
        if (_serial.available()) {
            byte b = _serial.read();
            buffer += (char)b;

            if (buffer.endsWith("\xFF\xFF\xFF")) {
                buffer.remove(buffer.length() - 3);
                buffer.trim();

                if (buffer == "component list begin") {
                    collecting = true;
                } else if (buffer == "component list end") {
                    break;
                } else if (collecting) {
                    int idx1 = buffer.indexOf(',');
                    int idx2 = buffer.indexOf(',', idx1 + 1);
                    int idx3 = buffer.indexOf(',', idx2 + 1);

                    if (idx1 > -1 && idx2 > -1 && idx3 > -1) {
                        NextionComponent comp;
                        comp.pageId = buffer.substring(0, idx1).toInt();
                        comp.componentId = buffer.substring(idx1 + 1, idx2).toInt();
                        comp.name = buffer.substring(idx2 + 1, idx3);
                        comp.type = buffer.substring(idx3 + 1);
                        components.push_back(comp);
                    }
                }

                buffer = "";
            }
        }
    }

    sendCommand("tm0.en=0");
    return !components.empty();
}