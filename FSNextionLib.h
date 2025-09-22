#ifndef FS_NEXTION_LIB_H
#define FS_NEXTION_LIB_H

#include <Arduino.h>
#include <functional>

// Callback function type: void func(pageId, componentId, eventType)
// eventType: 0 = Release, 1 = Press
using TouchEventCallback = std::function<void(byte, byte, byte)>;
using SystemEventCallback = std::function<void(byte)>;

class FSNextionLib {
public:
    // Error codes
    enum class ErrorCode {
        SUCCESS,
        TIMEOUT,
        INVALID_RESPONSE,
        BUFFER_OVERFLOW,
        NOT_CONNECTED,
        SERIAL_ERROR
    };

    // Constructor: Takes the HardwareSerial port to which Nextion is connected (e.g., Serial2).
    FSNextionLib(HardwareSerial& serial);

    // Initializes serial communication with Nextion. Can use custom pins for ESP32.
    void begin(long baud = 115200, int8_t rxPin = -1, int8_t txPin = -1);

    // Basic functions
    void sendCommand(const char* cmd);
    void sendCommand(const String& cmd);
    
    // Component control functions
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
    
    // Page control - BELİRSİZLİĞİ ÖNLEMEK İÇİN İSİM DEĞİŞTİRDİK
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
    int getNumber(const char* component);
    
    // Status and connection
    bool isConnected();
    ErrorCode getLastError() const;
    void clearError();
    
    // Debug and configuration
    void setDebug(bool enabled);
    void setTimeout(unsigned long timeoutMs);
    
    // Event handling
    void listen();
    void onTouch(TouchEventCallback callback);
    void onSystemEvent(SystemEventCallback callback);

private:
    HardwareSerial& _serial;
    TouchEventCallback _touchCallback = nullptr;
    SystemEventCallback _systemCallback = nullptr;
    
    ErrorCode _lastError = ErrorCode::SUCCESS;
    bool _debug = false;
    unsigned long _timeout = 500;
    
    void _endCommand();
    bool _waitForTerminator(unsigned long timeout);
    void _handleTouchEvent();
    void _handleStringData();
    void _handleNumericData();
    void _handleSystemEvent(byte eventType);
    void _clearBuffer();
    void _log(const String& message);
    void _log(const char* message);
    bool _readBytes(byte* buffer, size_t length, unsigned long timeout);
};

#endif // FS_NEXTION_LIB_H