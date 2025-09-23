#include <Arduino.h>
#include "FSNextionLib.h"

FSNextionLib myNextion(Serial2);

void handleTouchEvent(byte pageId, byte componentId, byte eventType) {
  if (pageId == 0 && componentId == 1 && eventType == 0) {
    myNextion.txt("t0", "Button 1 Active");
  }
}

void setup() {
  Serial.begin(115200);
  myNextion.begin(115200);

  while (!myNextion.isConnected()) {
    Serial.println("Waiting for Nextion...");
    delay(2000);
  }

  myNextion.onTouch(handleTouchEvent);
  myNextion.txt("t0", "Ready");
}

void loop() {
  myNextion.listen();
}