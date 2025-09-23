#include "FSNextionLib.h"

FSNextionLib::FSNextionLib(HardwareSerial& serial) : _serial(serial) {}

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
}

void FSNextionLib::endCommand() {
    _serial.write(0xFF);
    _serial.write(0xFF);
    _serial.write(0xFF);
}

void FSNextionLib::sendCommand(const char* cmd) {
    _serial.print(cmd);
    endCommand();
}

void FSNextionLib::setText(const char* component, const char* txt) {
    String cmd = String(component) + ".txt=\"" + String(txt) + "\"";
    sendCommand(cmd.c_str());
}

void FSNextionLib::setNumber(const char* component, int value) {
    String cmd = String(component) + ".val=" + String(value);
    sendCommand(cmd.c_str());
}

String FSNextionLib::getText(const char* component) {
    String cmd = "get " + String(component) + ".txt";
    sendCommand(cmd.c_str());

    String response = "";
    long startTime = millis();
    while (millis() - startTime < 500) {
        if (_serial.available()) {
            char c = _serial.read();
            if (c == 0x70) {
                while (millis() - startTime < 500) {
                    if (_serial.available()) {
                        char nextChar = _serial.read();
                        if (nextChar == 0xFF) {
                            _serial.read();
                            _serial.read();
                            return response;
                        } else {
                            response += nextChar;
                        }
                    }
                }
            }
        }
    }
    return "";
}

int FSNextionLib::getNumber(const char* component) {
    String cmd = "get " + String(component) + ".val";
    sendCommand(cmd.c_str());

    long startTime = millis();
    while (millis() - startTime < 500) {
        if (_serial.available()) {
            char c = _serial.read();
            if (c == 0x71) {
                if (_serial.available() >= 7) {
                    byte buffer[4];
                    _serial.readBytes(buffer, 4);
                    for (int i = 0; i < 3; i++) _serial.read();
                    return buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);
                }
            }
        }
    }
    return -1;
}

bool FSNextionLib::isConnected() {
    while (_serial.available()) _serial.read();

    sendCommand("get sleep");

    long startTime = millis();
    while (millis() - startTime < 300) {
        if (_serial.available()) {
            while (_serial.available()) _serial.read();
            return true;
        }
    }
    return false;
}

void FSNextionLib::onTouch(TouchEventCallback callback) {
    _touchCallback = callback;
}

void FSNextionLib::listen() {
    if (_serial.available() < 7) return;

    byte firstByte = _serial.read();
    if (firstByte != 0x65) {
        while (_serial.available()) _serial.read();
        return;
    }

    byte pageId = _serial.read();
    byte componentId = _serial.read();
    byte eventType = _serial.read();

    _serial.read(); _serial.read(); _serial.read();

    if (_touchCallback) {
        _touchCallback(pageId, componentId, eventType);
    }
}

String FSNextionLib::readRawNextionString(long timeout) {
    String result = "";
    unsigned long start = millis();
    while (millis() - start < timeout) {
        if (_serial.available()) {
            char c = _serial.read();
            result += c;
        }
    }
    return result;
}

void FSNextionLib::page(const String& pageName) {
    String cmd = "page " + pageName;
    sendCommand(cmd.c_str());
}

void FSNextionLib::txt(const String& objname, const String& value) {
    setText(objname.c_str(), value.c_str());
}

String FSNextionLib::txt(const String& objname) {
    return getText(objname.c_str());
}

void FSNextionLib::val(const String& objname, int value) {
    setNumber(objname.c_str(), value);
}

int FSNextionLib::val(const String& objname) {
    return getNumber(objname.c_str());
}

void FSNextionLib::vis(const String& objname, bool visible) {
    String cmd = "vis " + objname + "," + String(visible ? 1 : 0);
    sendCommand(cmd.c_str());
}

void FSNextionLib::bco(const String& objname, uint16_t color) {
    String cmd = objname + ".bco=" + String(color);
    sendCommand(cmd.c_str());
}

void FSNextionLib::click(const String& objname) {
    String cmd = "click " + objname + ",1";
    sendCommand(cmd.c_str());
}

void FSNextionLib::release(const String& objname) {
    String cmd = "click " + objname + ",0";
    sendCommand(cmd.c_str());
}

void FSNextionLib::refresh(const String& objname) {
    String cmd = "ref " + objname;
    sendCommand(cmd.c_str());
}

void FSNextionLib::enable(const String& objname, bool state) {
    String cmd = "tsw " + objname + "," + String(state ? 1 : 0);
    sendCommand(cmd.c_str());
}

void FSNextionLib::touch(const String& objname, bool state) {
    String cmd = "tsw " + objname + "," + String(state ? 1 : 0);
    sendCommand(cmd.c_str());
}