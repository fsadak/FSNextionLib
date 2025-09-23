#ifndef FS_NEXTION_LIB_H
#define FS_NEXTION_LIB_H

#include <Arduino.h>
#include <functional>
#include <vector>

// Struct to hold information about a single Nextion component
struct NextionComponent {
    byte pageId;
    byte componentId;
    String name;
    String type;
};

// Callback function type: void func(pageId, componentId, eventType)
// eventType: 0 = Release, 1 = Press
using TouchEventCallback = std::function<void(byte, byte, byte)>;

class FSNextionLib {
public:
    std::vector<NextionComponent> components;

    FSNextionLib(HardwareSerial& serial);

    void begin(long baud = 115200, int8_t rxPin = -1, int8_t txPin = -1);
    void sendCommand(const char* cmd);
    void setText(const char* component, const char* txt);
    void setNumber(const char* component, int value);
    bool isConnected();
    String getText(const char* component);
    int getNumber(const char* component);
    void listen();
    void onTouch(TouchEventCallback callback);
    String readRawNextionString(long timeout = 250);

private:
    HardwareSerial& _serial;
    TouchEventCallback _touchCallback = nullptr;

    void endCommand();
    String _readString(long timeout = 250);
};

#endif // FS_NEXTION_LIB_H