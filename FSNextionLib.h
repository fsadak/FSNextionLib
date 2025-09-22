#ifndef FS_NEXTION_LIB_H
#define FS_NEXTION_LIB_H

#include <Arduino.h>
#include <functional>
#include <vector>
#include <map>

// Callback function type: void func(pageId, componentId, eventType)
// eventType: 0 = Release, 1 = Press
using TouchEventCallback = std::function<void(byte, byte, byte)>;
using SystemEventCallback = std::function<void(byte)>;
using ComponentListCallback = std::function<void(bool success)>;

// Component structure
struct NextionComponent {
    byte pageId;
    byte componentId;
    String name;
    String type;
    
    NextionComponent(byte pId, byte cId, const String& n, const String& t) 
        : pageId(pId), componentId(cId), name(n), type(t) {}
};

class FSNextionLib {
public:
    // Error codes
    enum class ErrorCode {
        SUCCESS,
        TIMEOUT,
        INVALID_RESPONSE,
        BUFFER_OVERFLOW,
        NOT_CONNECTED,
        SERIAL_ERROR,
        COMPONENT_NOT_FOUND
    };

    // Constructor: Takes the HardwareSerial port to which Nextion is connected (e.g., Serial2).
    FSNextionLib(HardwareSerial& serial);

    // Initializes serial communication with Nextion. Can use custom pins for ESP32.
    void begin(long baud = 115200, int8_t rxPin = -1, int8_t txPin = -1);

    // Basic functions
    void sendCommand(const char* cmd);
    void sendCommand(const String& cmd);
    
    // Component control functions - Orijinal fonksiyonlar
    void setText(const char* component, const char* txt);
    void setText(const String& component, const String& txt);
    void setNumber(const char* component, int value);
    void setNumber(const String& component, int value);
    void setVisible(const char* component, bool visible);
    void setEnabled(const char* component, bool enabled);
    void setBackgroundColor(const char* component, uint32_t color);
    void setFontColor(const char* component, uint32_t color);
    void setProgress(const char* component, byte value);
    void setGauge(const char* component, uint16_t value);
    
    // YENİ: Akıllı component fonksiyonları
    void setTextById(byte pageId, byte componentId, const char* txt);
    void setTextByName(const char* componentName, const char* txt);
    void setNumberById(byte pageId, byte componentId, int value);
    void setNumberByName(const char* componentName, int value);
    
    // Page control
    void setPageById(byte pageId);
    void setPageByName(const char* pageName);
    void setPageByName(const String& pageName);
    
    // System commands
    void sleep(bool enable);
    void wake();
    void reset();
    void setBrightness(byte brightness);
    
    // Read functions
    String getText(const char* component);
    String getTextById(byte pageId, byte componentId);
    String getTextByName(const char* componentName);
    int getNumber(const char* component);
    int getNumberById(byte pageId, byte componentId);
    int getNumberByName(const char* componentName);
    
    // Component list management
    void requestComponentList(); // Nextion'dan component listesi ister
    bool isComponentListLoaded() const;
    void setComponentListCallback(ComponentListCallback callback);
    
    // Component lookup
    bool componentExists(const char* componentName) const;
    bool componentExists(byte pageId, byte componentId) const;
    NextionComponent* getComponentByName(const char* componentName);
    NextionComponent* getComponentById(byte pageId, byte componentId);
    std::vector<NextionComponent*> getComponentsByPage(byte pageId);
    std::vector<NextionComponent*> getComponentsByType(const char* type);
    
    // Status and connection
    bool isConnected();
    ErrorCode getLastError() const;
    void clearError();
    
    // Debug and configuration
    void setDebug(bool enabled);
    void setTimeout(unsigned long timeoutMs);
    void printComponentList(); // Debug için component listesini yazdırır
    
    // Event handling
    void listen();
    void onTouch(TouchEventCallback callback);
    void onSystemEvent(SystemEventCallback callback);

private:
    HardwareSerial& _serial;
    TouchEventCallback _touchCallback = nullptr;
    SystemEventCallback _systemCallback = nullptr;
    ComponentListCallback _componentListCallback = nullptr;
    
    // Component management
    std::vector<NextionComponent> _components;
    std::map<String, NextionComponent*> _nameMap; // Hızlı lookup için
    std::map<uint16_t, NextionComponent*> _idMap; // pageId<<8 | componentId için
    bool _componentListLoaded = false;
    bool _waitingForComponentList = false;
    String _componentListBuffer = "";
    
    ErrorCode _lastError = ErrorCode::SUCCESS;
    bool _debug = false;
    unsigned long _timeout = 500;
    
    void _endCommand();
    bool _waitForTerminator(unsigned long timeout);
    void _handleTouchEvent();
    void _handleStringData();
    void _handleNumericData();
    void _handleSystemEvent(byte eventType);
    void _handleComponentList(const String& data);
    void _parseComponentLine(const String& line);
    String _mapComponentType(const String& nextionType);
    void _clearBuffer();
    void _log(const String& message);
    void _log(const char* message);
    bool _readBytes(byte* buffer, size_t length, unsigned long timeout);
    uint16_t _makeComponentKey(byte pageId, byte componentId);
};

#endif // FS_NEXTION_LIB_H