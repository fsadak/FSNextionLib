#ifndef COMPONENT_GROUP_H
#define COMPONENT_GROUP_H

#include <Arduino.h>
#include <vector>
#include "ComponentProxy.h"

class ComponentGroup {
public:
    void add(ComponentProxy proxy);

    ComponentGroup& txt(const String& value);
    ComponentGroup& val(int value);
    ComponentGroup& vis(bool visible);
    ComponentGroup& click();
    ComponentGroup& release();
    ComponentGroup& refresh();

private:
    std::vector<ComponentProxy> _components;
};

#endif