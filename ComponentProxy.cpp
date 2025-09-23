#include "ComponentProxy.h"
#include "FSNextionLib.h"

ComponentProxy::ComponentProxy(FSNextionLib* lib, const String& name)
    : _lib(lib), _name(name) {}

ComponentProxy& ComponentProxy::txt(const String& value) {
    _lib->setText(_name.c_str(), value.c_str());
    return *this;
}

String ComponentProxy::txt() {
    return _lib->getText(_name.c_str());
}

ComponentProxy& ComponentProxy::val(int value) {
    _lib->setNumber(_name.c_str(), value);
    return *this;
}

int ComponentProxy::val() {
    return _lib->getNumber(_name.c_str());
}

ComponentProxy& ComponentProxy::vis(bool visible) {
    String cmd = _name + ".vis=" + String(visible ? 1 : 0);
    _lib->sendCommand(cmd.c_str());
    return *this;
}

bool ComponentProxy::vis() {
    String cmd = "get " + _name + ".vis";
    _lib->sendCommand(cmd.c_str());
    String result = _lib->readRawNextionString();
    return result.toInt() == 1;
}

ComponentProxy& ComponentProxy::bco(uint16_t color) {
    String cmd = _name + ".bco=" + String(color);
    _lib->sendCommand(cmd.c_str());
    return *this;
}

int ComponentProxy::bco() {
    String cmd = "get " + _name + ".bco";
    _lib->sendCommand(cmd.c_str());
    String result = _lib->readRawNextionString();
    return result.toInt();
}

void ComponentProxy::cmd(const String& raw) {
    _lib->sendCommand(raw.c_str());
}
