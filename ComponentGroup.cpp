#include "ComponentGroup.h"

void ComponentGroup::add(ComponentProxy proxy) {
    _components.push_back(proxy);
}

ComponentGroup& ComponentGroup::txt(const String& value) {
    for (auto& c : _components) c.txt(value);
    return *this;
}

ComponentGroup& ComponentGroup::val(int value) {
    for (auto& c : _components) c.val(value);
    return *this;
}

ComponentGroup& ComponentGroup::vis(bool visible) {
    for (auto& c : _components) c.vis(visible);
    return *this;
}

ComponentGroup& ComponentGroup::click() {
    for (auto& c : _components) c.click();
    return *this;
}

ComponentGroup& ComponentGroup::release() {
    for (auto& c : _components) c.release();
    return *this;
}

ComponentGroup& ComponentGroup::refresh() {
    for (auto& c : _components) c.refresh();
    return *this;
}