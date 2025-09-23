#ifndef FSNEXTIONLIB_H
#define FSNEXTIONLIB_H

#include <Arduino.h>
#include <functional>
#include <vector>
#include "ComponentProxy.h"

struct NextionComponent {
    byte pageId;
    byte componentId;
    String name;
    String type;
};

using TouchEventCallback = std::function<void(byte, byte, byte)>;

class FSNextionLib {
public:
    FSNextionLib(HardwareSerial& serial);

    void begin(long baud = 115200, int8_t rxPin = -1, int8_t txPin = -1);
    void sendCommand(const char* cmd);
    void setText(const char* component, const char* txt);
    void setNumber(const char* component, int value);
    String getText(const char* component);
    int getNumber(const char* component);
    bool isConnected();
    void listen();
    void onTouch(TouchEventCallback callback);
    void page(const String& pageName);
    String readRawNextionString(long timeout = 250);

    ComponentProxy operator[](const String& name);
    ComponentProxy operator[](const char* name);

private:
    HardwareSerial& _serial;
    TouchEventCallback _touchCallback = nullptr;

    void endCommand();
    String _readString(long timeout = 250);
};

#endif
