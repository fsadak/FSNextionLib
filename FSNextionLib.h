#ifndef FSNEXTIONLIB_H
#define FSNEXTIONLIB_H

#include <Arduino.h>
#include <functional>
#include <vector>

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
    String getText(const char* component);
    int getNumber(const char* component);
    bool isConnected();
    void listen();
    void onTouch(TouchEventCallback callback);
    void page(const String& pageName);
    String readRawNextionString(long timeout = 250);

    void txt(const String& objname, const String& value);
    String txt(const String& objname);

    void val(const String& objname, int value);
    int val(const String& objname);

    void vis(const String& objname, bool visible);
    void bco(const String& objname, uint16_t color);

    void click(const String& objname);
    void release(const String& objname);
    void refresh(const String& objname);

    void enable(const String& objname, bool state);
    void touch(const String& objname, bool state);

private:
    HardwareSerial& _serial;
    TouchEventCallback _touchCallback = nullptr;

    void endCommand();
    String _readString(long timeout = 250);
};

#endif // FSNEXTIONLIB_H