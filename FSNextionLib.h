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
    // A vector to store discovered components
    std::vector<NextionComponent> components;

    // Constructor: Takes the HardwareSerial port to which Nextion is connected (e.g., Serial2).
    FSNextionLib(HardwareSerial& serial);

    // Initializes serial communication with Nextion. Can use custom pins for ESP32.
    void begin(long baud = 115200, int8_t rxPin = -1, int8_t txPin = -1);

    // A basic function to send commands to Nextion.
    void sendCommand(const char* cmd);

    // Changes the content of a text component.
    void setText(const char* component, const char* txt);

    // Changes the value of a number component.
    void setNumber(const char* component, int value);

    // Checks if the Nextion display is connected and active.
    bool isConnected();

    // Reads the text (.txt) value of a component.
    String getText(const char* component);

    // Reads the numeric (.val) value of a component.
    int getNumber(const char* component);

    // Listens for incoming Nextion events. Should be called continuously in loop().
    void listen();

    // Registers a function to be called when a touch event occurs.
    void onTouch(TouchEventCallback callback);

    // Triggers and parses the component discovery process.
    bool discoverComponents();
    bool isComponentDiscoveryAvailable() const { return _componentDiscoveryEnabled; }

private:
    HardwareSerial& _serial; // Serial port reference to be used for Nextion.
    TouchEventCallback _touchCallback = nullptr; // Registered callback function

    void endCommand();
    String _readString(long timeout = 250); // Private helper to read string responses
    bool _componentDiscoveryEnabled = false;
    NextionComponent* getComponentByName(const String& name);
};

#endif // FS_NEXTION_LIB_H
