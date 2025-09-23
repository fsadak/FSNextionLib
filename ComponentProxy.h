#ifndef COMPONENT_PROXY_H
#define COMPONENT_PROXY_H

#include <Arduino.h>

class FSNextionLib; // Forward declaration

class ComponentProxy {
public:
    ComponentProxy(FSNextionLib* lib, const String& name);

    ComponentProxy& txt(const String& value);
    String txt();

    ComponentProxy& val(int value);
    int val();

    ComponentProxy& vis(bool visible);
    bool vis();

    ComponentProxy& bco(uint16_t color);
    int bco();

    void cmd(const String& raw);

private:
    FSNextionLib* _lib;
    String _name;
};

#endif
