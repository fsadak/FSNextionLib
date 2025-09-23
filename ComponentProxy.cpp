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
    String cmd = "vis " + _name + "," + String(visible ? 1 : 0);
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

ComponentProxy& ComponentProxy::click() {
    String cmd = "click " + _name + ",1";
    _lib->sendCommand(cmd.c_str());
    return *this;
}

ComponentProxy& ComponentProxy::release() {
    String cmd = "click " + _name + ",0";
    _lib->sendCommand(cmd.c_str());
    return *this;
}

ComponentProxy& ComponentProxy::refresh() {
    String cmd = "ref " + _name;
    _lib->sendCommand(cmd.c_str());
    return *this;
}

ComponentProxy& ComponentProxy::enable(bool state) {
    String cmd = "tsw " + _name + "," + String(state ? 1 : 0);
    _lib->sendCommand(cmd.c_str());
    return *this;
}

ComponentProxy& ComponentProxy::set(int value) {
    String cmd = _name + ".val=" + String(value);
    _lib->sendCommand(cmd.c_str());
    return *this;
}

ComponentProxy& ComponentProxy::update(int value) {
    String cmd = _name + ".val=" + String(value);
    _lib->sendCommand(cmd.c_str());
    return *this;
}

ComponentProxy& ComponentProxy::font(uint8_t fontId) {
    String cmd = _name + ".font=" + String(fontId);
    _lib->sendCommand(cmd.c_str());
    return *this;
}

ComponentProxy& ComponentProxy::style(uint8_t styleId) {
    String cmd = _name + ".style=" + String(styleId);
    _lib->sendCommand(cmd.c_str());
    return *this;
}