#include <ArduinoJson.h>              // https://github.com/bblanchon/ArduinoJson needs version v6 or above
#include <WiFi.h>                     // Built-in
#include <HTTPClient.h>

#include "display.h"
#include "secrets.h"

StaticJsonDocument<256> jsonBuffer;

void wifiSetup() {
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP()); 
}

String sendQuery(const char *path) {
  //Check WiFi connection status
  if(WiFi.status()== WL_CONNECTED){
    HTTPClient http;

    String serverPath = serverName + path;
    
    // Your Domain name with URL path or IP address with path
    http.begin(serverPath.c_str());
    
    // Send HTTP GET request
    int httpResponseCode = http.GET();
    
    if (httpResponseCode>0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      return http.getString();
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    // Free resources
    http.end();
  }
  else {
    Serial.println("WiFi Disconnected");
  }

  return "";
}

void setup() {
  Serial.begin(115200);
  wifiSetup();

  String payload = sendQuery("/api/rest/burgercount");
  deserializeJson(jsonBuffer, payload);
  const uint16_t burgerCount = jsonBuffer["botd_aggregate"]["aggregate"]["count"];
  const uint16_t randomBurgerId = random(burgerCount) + 1;
  Serial.println(randomBurgerId);

  char randomBurgerQuery[50];
  sprintf(randomBurgerQuery, "/api/rest/burgerbyid/%d", randomBurgerId);
  Serial.println(randomBurgerQuery);
  payload = sendQuery(randomBurgerQuery);
  deserializeJson(jsonBuffer, payload);

  const char *burger = jsonBuffer["botd"][0]["name"];
  const char *tag = "";
  if (!jsonBuffer["botd"][0]["tag"].isNull()) {
    tag = jsonBuffer["botd"][0]["tag"];
  }
  const char *price = jsonBuffer["botd"][0]["price"];

  initDisplay();
  drawBotd(burger, tag, price);
}

void loop() { // this will never run!
}
