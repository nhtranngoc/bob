#include <ArduinoJson.h>              // https://github.com/bblanchon/ArduinoJson needs version v6 or above
#include <WiFi.h>                     // Built-in
#include <HTTPClient.h>
#include <WebServer.h>
#include <EEPROM.h>

#include "display.h"
#include "secrets.h"

StaticJsonDocument<256> jsonBuffer;
WebServer server(80);

#define WIFI_TIMEOUT 30000 // Trying to connect for 30s before switching on AP mode
#define uS_TO_S_FACTOR 1000000ULL  //Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP  60*60*24        //Time ESP32 will go to sleep (in seconds)

String esid;
String epass = "";
int statusCode;
String st;
String content;

void createWebServer();

bool isWifiConnected(void)
{
  int c = 0;
  //Serial.println("Waiting for Wifi to connect");
  while ( c < 20 ) {
    if (WiFi.status() == WL_CONNECTED)
    {
      return true;
    }
    delay(500);
    Serial.print("*");
    c++;
  }
  Serial.println("");
  Serial.println("Connect timed out, opening AP");
  return false;
}

void clearEEPROM() {
  Serial.println("Clearing EEPROM");
  for (int i = 0; i < 512; ++i) {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
  Serial.println("Cleared");
}

void startServer() {
  Serial.println("");
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi connected");
  }
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("SoftAP IP: ");
  Serial.println(WiFi.softAPIP());
  createWebServer();
  // Start the server
  server.begin();
  Serial.println("Server started");
}

void updateAPList() {
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      //Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
  }
  Serial.println("");
  st = "<ol>";
  for (int i = 0; i < n; ++i) {
    // Print SSID and RSSI for each network found
    st += "<li>";
    st += WiFi.SSID(i);
    st += " (";
    st += WiFi.RSSI(i);

    st += ")";
    //st += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
    st += "</li>";
  }
  st += "</ol>";
}

void setupAP(void) {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  updateAPList();
  delay(100);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  Serial.println("Initializing_softap_for_wifi credentials_modification");
  startServer();
  Serial.println("over");
}

void createWebServer() {
  {
    server.on("/", []() {
      updateAPList();
      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
      content = "<!DOCTYPE HTML>\r\n<html>";
      content += "<style type=\"text/css\">body{margin:40px auto;max-width:650px;line-height:1.6;font-size:18px;color:#444;padding:0 10px}h1,h2,h3{line-height:1.2}</style>";
      content +="<h2>Update yo wifi here</h2>";
      content += "<button onClick=\"window.location.reload();\">Refresh</button>";
      content += "<p>";
      content += st;
      content += "</p><form method='get' action='setting'><label>SSID: </label><input name='ssid' length=32><input name='pass' length=64><input type='submit'></form>";
      content += "</html>";
      server.send(200, "text/html", content);
    });

    server.on("/setting", []() {
      String qsid = server.arg("ssid");
      String qpass = server.arg("pass");
      if (qsid.length() > 0 && qpass.length() > 0) {
        Serial.println("clearing eeprom");
        clearEEPROM();
        Serial.println(qsid);
        Serial.println("");
        Serial.println(qpass);
        Serial.println("");

        Serial.println("writing eeprom ssid:");
        for (int i = 0; i < qsid.length(); ++i)
        {
          EEPROM.write(i, qsid[i]);
          Serial.print("Wrote: ");
          Serial.println(qsid[i]);
        }
        Serial.println("writing eeprom pass:");
        for (int i = 0; i < qpass.length(); ++i)
        {
          EEPROM.write(32 + i, qpass[i]);
          Serial.print("Wrote: ");
          Serial.println(qpass[i]);
        }
        EEPROM.commit();

        content = "{\"Success\":\"saved to eeprom... reset to boot into new wifi\"}";
        statusCode = 200;
        ESP.restart();
      } else {
        content = "{\"Error\":\"404 not found\"}";
        statusCode = 404;
        Serial.println("Sending 404");
      }
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(statusCode, "application/json", content);

    });
  }
}

void wifiSetup() {
  // Retrieve WiFi details from EEPROM
  Serial.println();
  Serial.println("Disconnecting current wifi connection");
  WiFi.disconnect();
  EEPROM.begin(512); //Initialasing EEPROM
  delay(10);
  Serial.println();
  Serial.println("Reading EEPROM ssid");

  for (int i = 0; i < 32; ++i) {
    esid += char(EEPROM.read(i));
  }
  Serial.println();
  Serial.print("SSID: ");
  Serial.println(esid);
  Serial.println("Reading EEPROM password");

  for (int i = 32; i < 96; ++i) {
    epass += char(EEPROM.read(i));
  }
  Serial.print("PASS: ");
  Serial.println(epass);

  // Try and connect to WiFi
  WiFi.begin(esid.c_str(), epass.c_str());
  Serial.print("Connecting to SSID ");
  Serial.println(esid);

  // If doesn't work, spin up server
    if ((WiFi.status() == WL_CONNECTED)) {
    for (int i = 0; i < 10; i++) {
      Serial.print("Successfully connected to ");
      Serial.println(esid);
      delay(100);
    }
  }

  if (isWifiConnected()) {
    Serial.println("Connected!");
    return;
  } else {
    Serial.println("Not connected, turning on AP");
    startServer();
    setupAP();
  }

  Serial.println();
  Serial.println("Waiting.");

  while ((WiFi.status() != WL_CONNECTED)) {
    Serial.print(".");
    delay(100);
    server.handleClient();
  }
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

void queryBurger() {
  // Grab total burger count, generate a random burger ID
  String payload = sendQuery("/api/rest/burgercount");
  deserializeJson(jsonBuffer, payload);
  const uint16_t burgerCount = jsonBuffer["botd_aggregate"]["aggregate"]["count"];
  const uint16_t randomBurgerId = random(burgerCount) + 1;
  Serial.println(randomBurgerId);

  // Retrieve burger with ID
  char randomBurgerQuery[50];
  sprintf(randomBurgerQuery, "/api/rest/burgerbyid/%d", randomBurgerId);
  Serial.println(randomBurgerQuery);
  payload = sendQuery(randomBurgerQuery);
  deserializeJson(jsonBuffer, payload);
}

void setup() {
  Serial.begin(115200);
  EEPROM.begin(512); //Initialasing EEPROM
  wifiSetup();

  queryBurger(); // and store in global jsonBuffer
  const char *burger = jsonBuffer["botd"][0]["name"];
  const char *tag = "";
  if (!jsonBuffer["botd"][0]["tag"].isNull()) {
    tag = jsonBuffer["botd"][0]["tag"];
  }
  const char *price = jsonBuffer["botd"][0]["price"];

  initDisplay();
  drawBotd(burger, tag, price);

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
	Serial.println("Setup ESP32 to sleep for " + String(TIME_TO_SLEEP) +
	" Seconds");

	//Go to sleep now
	esp_deep_sleep_start();
}

void loop() {
  // Like me, this will never run
}
