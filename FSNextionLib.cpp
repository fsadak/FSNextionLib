#include "FSNextionLib.h"

// Constructor
FSNextionLib::FSNextionLib(HardwareSerial& serial) : _serial(serial) {}

// Initializes communication
void FSNextionLib::begin(long baud, int8_t rxPin, int8_t txPin) {
    #if defined(ESP32)
        if (rxPin > -1 && txPin > -1) {
            _serial.begin(baud, SERIAL_8N1, rxPin, txPin);
        } else {
            _serial.begin(baud);
        }
    #else
        _serial.begin(baud);
    #endif
    
    // Wait for serial to initialize
    delay(100);
    _clearBuffer();
    
    _log("Nextion library initialized");
}

// Sends command termination characters
void FSNextionLib::_endCommand() {
    _serial.write(0xFF);
    _serial.write(0xFF);
    _serial.write(0xFF);
}

// Clears the serial buffer
void FSNextionLib::_clearBuffer() {
    while (_serial.available()) {
        _serial.read();
    }
}

// Logging function
void FSNextionLib::_log(const String& message) {
    if (_debug) {
        Serial.println("[FSNextion] " + message);
    }
}

void FSNextionLib::_log(const char* message) {
    if (_debug) {
        Serial.print("[FSNextion] ");
        Serial.println(message);
    }
}

// Wait for terminator sequence
bool FSNextionLib::_waitForTerminator(unsigned long timeout) {
    unsigned long startTime = millis();
    byte terminatorCount = 0;
    
    while (millis() - startTime < timeout) {
        if (_serial.available()) {
            byte b = _serial.read();
            if (b == 0xFF) {
                terminatorCount++;
                if (terminatorCount == 3) {
                    return true;
                }
            } else {
                terminatorCount = 0;
            }
        }
    }
    return false;
}

// Read bytes with timeout
bool FSNextionLib::_readBytes(byte* buffer, size_t length, unsigned long timeout) {
    unsigned long startTime = millis();
    size_t bytesRead = 0;
    
    while (bytesRead < length && millis() - startTime < timeout) {
        if (_serial.available()) {
            buffer[bytesRead++] = _serial.read();
        }
    }
    
    return bytesRead == length;
}

// General command sending function
void FSNextionLib::sendCommand(const char* cmd) {
    _serial.print(cmd);
    _endCommand();
    _log("Command sent: " + String(cmd));
}

void FSNextionLib::sendCommand(const String& cmd) {
    sendCommand(cmd.c_str());
}

// Component control functions
void FSNextionLib::setText(const char* component, const char* txt) {
    String cmd = String(component) + ".txt=\"" + String(txt) + "\"";
    sendCommand(cmd);
}

void FSNextionLib::setText(const String& component, const String& txt) {
    setText(component.c_str(), txt.c_str());
}

void FSNextionLib::setNumber(const char* component, int value) {
    String cmd = String(component) + ".val=" + String(value);
    sendCommand(cmd);
}

void FSNextionLib::setNumber(const String& component, int value) {
    setNumber(component.c_str(), value);
}

void FSNextionLib::setVisible(const char* component, bool visible) {
    String cmd = String(component) + ".vis=" + String(visible ? 1 : 0);
    sendCommand(cmd);
}

void FSNextionLib::setEnabled(const char* component, bool enabled) {
    String cmd = String(component) + ".ena=" + String(enabled ? 1 : 0);
    sendCommand(cmd);
}

void FSNextionLib::setBackgroundColor(const char* component, uint32_t color) {
    String cmd = String(component) + ".bco=" + String(color);
    sendCommand(cmd);
}

void FSNextionLib::setFontColor(const char* component, uint32_t color) {
    String cmd = String(component) + ".pco=" + String(color);
    sendCommand(cmd);
}

void FSNextionLib::setProgress(const char* component, byte value) {
    String cmd = String(component) + ".val=" + String(value);
    sendCommand(cmd);
}

void FSNextionLib::setGauge(const char* component, uint16_t value) {
    String cmd = String(component) + ".val=" + String(value);
    sendCommand(cmd);
}

// Page control - YENİ İSİMLER
void FSNextionLib::setPageById(byte pageId) {
    String cmd = "page " + String(pageId);
    sendCommand(cmd);
}

void FSNextionLib::setPageByName(const char* pageName) {
    String cmd = "page " + String(pageName);
    sendCommand(cmd);
}

void FSNextionLib::setPageByName(const String& pageName) {
    setPageByName(pageName.c_str());
}

// System commands
void FSNextionLib::sleep(bool enable) {
    sendCommand(enable ? "sleep=1" : "sleep=0");
}

void FSNextionLib::wake() {
    sleep(false);
}

void FSNextionLib::reset() {
    sendCommand("rest");
}

void FSNextionLib::setBrightness(byte brightness) {
    String cmd = "dim=" + String(brightness);
    sendCommand(cmd);
}

// Checks if the Nextion display is connected and active.
bool FSNextionLib::isConnected() {
    _clearBuffer();
    _lastError = ErrorCode::SUCCESS;

    sendCommand("connect");
    
    unsigned long startTime = millis();
    while (millis() - startTime < 300) {
        if (_serial.available() > 0) {
            // Check for valid response (any response is good)
            _clearBuffer();
            return true;
        }
    }
    
    _lastError = ErrorCode::NOT_CONNECTED;
    return false;
}

// Reads the text (.txt) value of a component.
String FSNextionLib::getText(const char* component) {
    _lastError = ErrorCode::SUCCESS;
    
    if (!isConnected()) {
        _lastError = ErrorCode::NOT_CONNECTED;
        return "";
    }
    
    String cmd = "get " + String(component) + ".txt";
    sendCommand(cmd.c_str());

    String response = "";
    unsigned long startTime = millis();
    
    while (millis() - startTime < _timeout) {
        if (_serial.available()) {
            byte firstByte = _serial.read();
            
            if (firstByte == 0x70) { // Text response start byte
                // Read text until terminator
                while (millis() - startTime < _timeout && response.length() < 256) {
                    if (_serial.available()) {
                        char c = _serial.read();
                        if (c == 0xFF) {
                            // Check for terminator sequence
                            if (_waitForTerminator(100)) {
                                _lastError = ErrorCode::SUCCESS;
                                return response;
                            }
                        } else {
                            response += c;
                        }
                    }
                }
                break;
            }
        }
    }
    
    _lastError = ErrorCode::TIMEOUT;
    return "";
}

// Reads the numeric (.val) value of a component.
int FSNextionLib::getNumber(const char* component) {
    _lastError = ErrorCode::SUCCESS;
    
    if (!isConnected()) {
        _lastError = ErrorCode::NOT_CONNECTED;
        return -1;
    }
    
    String cmd = "get " + String(component) + ".val";
    sendCommand(cmd.c_str());

    unsigned long startTime = millis();
    
    while (millis() - startTime < _timeout) {
        if (_serial.available()) {
            byte firstByte = _serial.read();
            
            if (firstByte == 0x71) { // Numeric response start byte
                byte data[4];
                if (_readBytes(data, 4, 100)) {
                    if (_waitForTerminator(100)) {
                        int value = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
                        _lastError = ErrorCode::SUCCESS;
                        return value;
                    }
                }
                break;
            }
        }
    }
    
    _lastError = ErrorCode::TIMEOUT;
    return -1;
}

// Event handlers
void FSNextionLib::_handleTouchEvent() {
    byte data[6];
    if (_readBytes(data, 6, 100)) {
        // Check terminator
        if (data[3] == 0xFF && data[4] == 0xFF && data[5] == 0xFF) {
            if (_touchCallback) {
                _touchCallback(data[0], data[1], data[2]);
            }
        } else {
            _lastError = ErrorCode::INVALID_RESPONSE;
        }
    }
}

void FSNextionLib::_handleStringData() {
    // Placeholder for string data handling
    _clearBuffer(); // Clear unknown string data for now
}

void FSNextionLib::_handleNumericData() {
    // Placeholder for numeric data handling  
    _clearBuffer(); // Clear unknown numeric data for now
}

void FSNextionLib::_handleSystemEvent(byte eventType) {
    if (_systemCallback) {
        _systemCallback(eventType);
    }
    _clearBuffer(); // Clear the event data
}

// Main event listener
void FSNextionLib::listen() {
    while (_serial.available() >= 1) {
        byte firstByte = _serial.peek(); // Peek without removing
        
        switch (firstByte) {
            case 0x65: // Touch event
                _serial.read(); // Remove from buffer
                if (_serial.available() >= 6) {
                    _handleTouchEvent();
                }
                break;
                
            case 0x70: // String data
            case 0x71: // Numeric data
                _serial.read();
                // These are typically responses to get commands, handled separately
                _clearBuffer();
                break;
                
            case 0x86: // Sleep
            case 0x87: // Wakeup
                _serial.read();
                _handleSystemEvent(firstByte);
                break;
                
            default:
                // Unknown data, clear one byte and continue
                _serial.read();
                _lastError = ErrorCode::INVALID_RESPONSE;
                break;
        }
    }
}

// Callback registration
void FSNextionLib::onTouch(TouchEventCallback callback) {
    _touchCallback = callback;
}

void FSNextionLib::onSystemEvent(SystemEventCallback callback) {
    _systemCallback = callback;
}

// Configuration functions
void FSNextionLib::setDebug(bool enabled) {
    _debug = enabled;
    _log("Debug " + String(enabled ? "enabled" : "disabled"));
}

void FSNextionLib::setTimeout(unsigned long timeoutMs) {
    _timeout = timeoutMs;
}

// Error handling
FSNextionLib::ErrorCode FSNextionLib::getLastError() const {
    return _lastError;
}

void FSNextionLib::clearError() {
    _lastError = ErrorCode::SUCCESS;
}