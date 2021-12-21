#include <ArduinoJson.h>              // https://github.com/bblanchon/ArduinoJson needs version v6 or above
#include <WiFi.h>                     // Built-in

#include "display.h"

void setup() {
  const char burgerName[] = "It's fun to eat at the RyeMCA Burger";
  const char tagLine[] = "(Comes on Rye with avo";
  const char price[] = "$5.95";

  Serial.begin(115200);
  initDisplay();
  drawBotd(burgerName, tagLine, price);
}

void loop() { // this will never run!
}
