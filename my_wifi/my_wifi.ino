#include <WiFi.h>
#include "esp_event.h"

void WiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info) {
  switch(event) {
    case ARDUINO_EVENT_WIFI_STA_START:
      Serial.println("STA started");
      break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
      Serial.println("Connected to AP");
      break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      Serial.print("Got IP: ");
      Serial.println(WiFi.localIP());
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      Serial.print("Disconnected from AP, reason: ");
      Serial.println(info.wifi_sta_disconnected.reason);  // <-- correct
      break;
    default:
      Serial.print("Other event: ");
      Serial.println(event);
      break;
  }
}

const char* ssid = "VM3870323";
String ssidToFind = "VM3870323";
const char* password = "xd
NEW SKETCH
xjLgyyhaun5Wds";

//const char* ssid = "AndroidAP9244";
//String ssidToFind = "AndroidAP9244";
//const char* password = "wneu3093";

void setup() {
  // put your setup code here, to run once:
  delay(2000);
  Serial.begin(115200);
  delay(2000);
  WiFi.onEvent(WiFiEvent); 
  for (int i = 0; i < 50; i++) {
    Serial.println();
  }
  wifi_mode_t wifi_mode = WiFi.getMode();
  Serial.print("WiFi.getMode : ");
  Serial.println(wifi_mode); //0 = WIFI_OFF

  //check wi-fi stack (driver) initiliaztion is okay
  WiFi.mode(WIFI_STA);
  delay(200);
  wifi_mode = WiFi.getMode();
  Serial.print("WiFi.getMode : ");
  Serial.println(wifi_mode); // 1 = WIFI_STA

  //scan for networks
  int n = WiFi.scanNetworks();
  if (n == 0) {
    Serial.println("No networks found");
  } else {
    Serial.printf("%d networks found:\n", n);
    String enc;

    for (int i = 0; i < n; ++i) {
      switch (WiFi.encryptionType(i)) {
        case WIFI_AUTH_OPEN: enc = "Open"; break;
        case WIFI_AUTH_WEP: enc = "WEP"; break;
        case WIFI_AUTH_WPA_PSK: enc = "WPA"; break;
        case WIFI_AUTH_WPA2_PSK: enc = "WPA2"; break;
        case WIFI_AUTH_WPA_WPA2_PSK: enc = "WPA/WPA2"; break;
        case WIFI_AUTH_WPA2_ENTERPRISE: enc = "WPA2 Enterprise"; break;
        case WIFI_AUTH_WPA3_PSK: enc = "WPA3"; break;
        case WIFI_AUTH_WPA2_WPA3_PSK: enc = "WPA2/WPA3"; break;
        default: enc = "Unknown"; break;
      }
      Serial.printf("%d: %s (RSSI (signal strength): %d dBm) %s\n", 
        i + 1, 
        WiFi.SSID(i).c_str(), 
        WiFi.RSSI(i), 
        enc);
    }
  }
  
  delay(4000);
  for (int i = 0; i < n; ++i) {
    if (WiFi.SSID(i) == ssidToFind) {
      Serial.print("Trying to connect to Network : ");
      Serial.print(ssid);
      Serial.print(" Password : ");
      Serial.println(password);
      WiFi.begin(ssid, password);
      //WiFi.begin(ssid); 

      unsigned long startTime = millis();  // record the start time
      const unsigned long timeout = 6000;

      while (WiFi.status() != WL_CONNECTED) {
        if (millis() - startTime >= timeout) {
          Serial.println("Connection timed out!");
          break;  // exit the loop after 6 seconds
        } 
        Serial.println(WiFi.status());
        delay(1000);
      }

      if (WiFi.status() == 6 ) {
        Serial.println("Disconnected, reason: ");
        //Serial.println(WiFi.reasonCode());
      }
    }
  }
}

void loop() {
  delay(5000);
  // put your main code here, to run repeatedly:
  Serial.println("Going to sleep...");
  esp_deep_sleep_start();
}
