#include "FSNextionLib.h"

// Constructor
FSNextionLib::FSNextionLib(HardwareSerial& serial) : _serial(serial) {}

// Component key oluşturma - ARTIK STATIC
uint16_t FSNextionLib::_makeComponentKey(byte pageId, byte componentId) {
    return (pageId << 8) | componentId;
}

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
    
    // Nextion'un boot olmasını bekle (3 saniye)
    delay(3000);
    
    // Otomatik olarak component listesi iste
    requestComponentList();
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

// Logging function - ARTIK CONST
void FSNextionLib::_log(const String& message) const {
    if (_debug) {
        Serial.println("[FSNextion] " + message);
    }
}

void FSNextionLib::_log(const char* message) const {
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

// Page control
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

// Component listesi isteği gönder
void FSNextionLib::requestComponentList() {
    _waitingForComponentList = true;
    _componentListBuffer = "";
    _components.clear();
    _nameMap.clear();
    _idMap.clear();
    _componentListLoaded = false;
    
    _log("Requesting component list from Nextion...");
}

// Component listesi yüklü mü?
bool FSNextionLib::isComponentListLoaded() const {
    return _componentListLoaded;
}

// Component listesi callback'i
void FSNextionLib::setComponentListCallback(ComponentListCallback callback) {
    _componentListCallback = callback;
}

// Component type mapping
String FSNextionLib::_mapComponentType(const String& nextionType) {
    // Nextion component type'larını standartlaştır
    if (nextionType == "button" || nextionType.startsWith("b")) return "button";
    if (nextionType == "text" || nextionType.startsWith("t")) return "text";
    if (nextionType == "number" || nextionType.startsWith("n")) return "number";
    if (nextionType == "gauge" || nextionType.startsWith("g")) return "gauge";
    if (nextionType == "progress" || nextionType.startsWith("j")) return "progress";
    if (nextionType == "slider" || nextionType.startsWith("z")) return "slider";
    return nextionType;
}

// Component listesi satırını parse et
void FSNextionLib::_parseComponentLine(const String& line) {
    // Format: "pageId, componentId, name, type"
    // Örnek: "0, 1, b0, button" veya "0, 2, t0, text"
    
    int firstComma = line.indexOf(',');
    int secondComma = line.indexOf(',', firstComma + 1);
    int thirdComma = line.indexOf(',', secondComma + 1);
    
    if (firstComma == -1 || secondComma == -1 || thirdComma == -1) {
        _log("Invalid component line: " + line);
        return;
    }
    
    byte pageId = line.substring(0, firstComma).toInt();
    byte componentId = line.substring(firstComma + 1, secondComma).toInt();
    String name = line.substring(secondComma + 1, thirdComma);
    name.trim();
    String type = line.substring(thirdComma + 1);
    type.trim();
    
    // Component type'ını standartlaştır
    type = _mapComponentType(type);
    
    // Component oluştur ve listeye ekle
    NextionComponent comp(pageId, componentId, name, type);
    _components.push_back(comp);
    
    // Lookup map'lerini güncelle
    _nameMap[name] = &_components.back();
    _idMap[_makeComponentKey(pageId, componentId)] = &_components.back();
    
    _log("Component added: Page=" + String(pageId) + 
         ", ID=" + String(componentId) + 
         ", Name=" + name + 
         ", Type=" + type);
}

// Component listesi verisini işle
void FSNextionLib::_handleComponentList(const String& data) {
    if (data == "component list begin") {
        _componentListBuffer = "";
        _components.clear();
        _nameMap.clear();
        _idMap.clear();
        _componentListLoaded = false;
        _waitingForComponentList = true;
        _log("Component list transmission started");
        return;
    }
    
    if (data == "component list end") {
        _componentListLoaded = true;
        _waitingForComponentList = false;
        
        _log("Component list loaded. Total components: " + String(_components.size()));
        
        // Callback çağır
        if (_componentListCallback) {
            _componentListCallback(true);
        }
        return;
    }
    
    // Component listesi devam ediyor, satırı parse et
    if (_waitingForComponentList) {
        _parseComponentLine(data);
    }
}

// Component lookup fonksiyonları - CONST VERSİYONLARI EKLENDİ
bool FSNextionLib::componentExists(const char* componentName) const {
    return _nameMap.find(String(componentName)) != _nameMap.end();
}

bool FSNextionLib::componentExists(byte pageId, byte componentId) const {
    return _idMap.find(_makeComponentKey(pageId, componentId)) != _idMap.end();
}

NextionComponent* FSNextionLib::getComponentByName(const char* componentName) {
    auto it = _nameMap.find(String(componentName));
    return (it != _nameMap.end()) ? it->second : nullptr;
}

const NextionComponent* FSNextionLib::getComponentByName(const char* componentName) const {
    auto it = _nameMap.find(String(componentName));
    return (it != _nameMap.end()) ? it->second : nullptr;
}

NextionComponent* FSNextionLib::getComponentById(byte pageId, byte componentId) {
    auto it = _idMap.find(_makeComponentKey(pageId, componentId));
    return (it != _idMap.end()) ? it->second : nullptr;
}

const NextionComponent* FSNextionLib::getComponentById(byte pageId, byte componentId) const {
    auto it = _idMap.find(_makeComponentKey(pageId, componentId));
    return (it != _idMap.end()) ? it->second : nullptr;
}

std::vector<NextionComponent*> FSNextionLib::getComponentsByPage(byte pageId) {
    std::vector<NextionComponent*> result;
    for (auto& comp : _components) {
        if (comp.pageId == pageId) {
            result.push_back(&comp);
        }
    }
    return result;
}

std::vector<const NextionComponent*> FSNextionLib::getComponentsByPage(byte pageId) const {
    std::vector<const NextionComponent*> result;
    for (const auto& comp : _components) {
        if (comp.pageId == pageId) {
            result.push_back(&comp);
        }
    }
    return result;
}

std::vector<NextionComponent*> FSNextionLib::getComponentsByType(const char* type) {
    std::vector<NextionComponent*> result;
    String typeStr(type);
    for (auto& comp : _components) {
        if (comp.type == typeStr) {
            result.push_back(&comp);
        }
    }
    return result;
}

std::vector<const NextionComponent*> FSNextionLib::getComponentsByType(const char* type) const {
    std::vector<const NextionComponent*> result;
    String typeStr(type);
    for (const auto& comp : _components) {
        if (comp.type == typeStr) {
            result.push_back(&comp);
        }
    }
    return result;
}

// YENİ: Akıllı component fonksiyonları
void FSNextionLib::setTextById(byte pageId, byte componentId, const char* txt) {
    if (_componentListLoaded) {
        NextionComponent* comp = getComponentById(pageId, componentId);
        if (comp && (comp->type == "text" || comp->type == "button")) {
            setText(comp->name.c_str(), txt);
            return;
        }
    }
    // Fallback: direct command
    String compName = "p[" + String(pageId) + "].b[" + String(componentId) + "]";
    setText(compName.c_str(), txt);
}

void FSNextionLib::setTextByName(const char* componentName, const char* txt) {
    if (_componentListLoaded) {
        NextionComponent* comp = getComponentByName(componentName);
        if (comp && (comp->type == "text" || comp->type == "button")) {
            setText(comp->name.c_str(), txt);
            return;
        }
    }
    // Fallback: direct command
    setText(componentName, txt);
}

void FSNextionLib::setNumberById(byte pageId, byte componentId, int value) {
    if (_componentListLoaded) {
        NextionComponent* comp = getComponentById(pageId, componentId);
        if (comp && (comp->type == "number" || comp->type == "gauge" || comp->type == "progress")) {
            setNumber(comp->name.c_str(), value);
            return;
        }
    }
    // Fallback: direct command
    String compName = "p[" + String(pageId) + "].b[" + String(componentId) + "]";
    setNumber(compName.c_str(), value);
}

void FSNextionLib::setNumberByName(const char* componentName, int value) {
    if (_componentListLoaded) {
        NextionComponent* comp = getComponentByName(componentName);
        if (comp && (comp->type == "number" || comp->type == "gauge" || comp->type == "progress")) {
            setNumber(comp->name.c_str(), value);
            return;
        }
    }
    // Fallback: direct command
    setNumber(componentName, value);
}

// YENİ: Akıllı read fonksiyonları
String FSNextionLib::getTextById(byte pageId, byte componentId) {
    if (_componentListLoaded) {
        NextionComponent* comp = getComponentById(pageId, componentId);
        if (comp && (comp->type == "text" || comp->type == "button")) {
            return getText(comp->name.c_str());
        }
    }
    // Fallback
    String compName = "p[" + String(pageId) + "].b[" + String(componentId) + "]";
    return getText(compName.c_str());
}

String FSNextionLib::getTextByName(const char* componentName) {
    if (_componentListLoaded) {
        NextionComponent* comp = getComponentByName(componentName);
        if (comp && (comp->type == "text" || comp->type == "button")) {
            return getText(comp->name.c_str());
        }
    }
    // Fallback
    return getText(componentName);
}

int FSNextionLib::getNumberById(byte pageId, byte componentId) {
    if (_componentListLoaded) {
        NextionComponent* comp = getComponentById(pageId, componentId);
        if (comp && (comp->type == "number" || comp->type == "gauge" || comp->type == "progress")) {
            return getNumber(comp->name.c_str());
        }
    }
    // Fallback
    String compName = "p[" + String(pageId) + "].b[" + String(componentId) + "]";
    return getNumber(compName.c_str());
}

int FSNextionLib::getNumberByName(const char* componentName) {
    if (_componentListLoaded) {
        NextionComponent* comp = getComponentByName(componentName);
        if (comp && (comp->type == "number" || comp->type == "gauge" || comp->type == "progress")) {
            return getNumber(comp->name.c_str());
        }
    }
    // Fallback
    return getNumber(componentName);
}

// Debug için component listesini yazdır - CONST VERSİYON
void FSNextionLib::printComponentList() {
    printComponentList();
}

void FSNextionLib::printComponentList() const {
    if (!_componentListLoaded) {
        _log("Component list not loaded yet");
        return;
    }
    
    Serial.println("=== NEXTION COMPONENT LIST ===");
    for (const auto& comp : _components) {
        Serial.printf("Page: %d, ID: %d, Name: %s, Type: %s\n", 
                     comp.pageId, comp.componentId, comp.name.c_str(), comp.type.c_str());
    }
    Serial.println("==============================");
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
    _clearBuffer();
}

void FSNextionLib::_handleNumericData() {
    // Placeholder for numeric data handling  
    _clearBuffer();
}

void FSNextionLib::_handleSystemEvent(byte eventType) {
    if (_systemCallback) {
        _systemCallback(eventType);
    }
    _clearBuffer();
}

// Main event listener
void FSNextionLib::listen() {
    while (_serial.available()) {
        // String veri geliyor mu kontrol et (component listesi için)
        if (_serial.available() > 5) {
            String data = _serial.readStringUntil('\n');
            data.trim();
            
            if (data.length() > 0) {
                _log("Received: " + data);
                
                // Component listesi ile ilgili veri mi?
                if (data.startsWith("component list") || _waitingForComponentList) {
                    _handleComponentList(data);
                    continue;
                }
            }
        }
        
        // Binary event'leri işle
        if (_serial.available() >= 7) {
            byte firstByte = _serial.peek();
            
            switch (firstByte) {
                case 0x65: // Touch event
                    _serial.read();
                    if (_serial.available() >= 6) {
                        _handleTouchEvent();
                    }
                    break;
                    
                case 0x70: // String data
                case 0x71: // Numeric data
                    _serial.read();
                    _clearBuffer();
                    break;
                    
                case 0x86: // Sleep
                case 0x87: // Wakeup
                    _serial.read();
                    _handleSystemEvent(firstByte);
                    break;
                    
                default:
                    _serial.read();
                    _lastError = ErrorCode::INVALID_RESPONSE;
                    break;
            }
        } else {
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