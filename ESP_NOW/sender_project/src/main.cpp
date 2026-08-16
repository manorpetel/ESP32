#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

// Use the receiver's MAC address for a direct ESP-NOW peer.
constexpr uint8_t peerAddress[] = {0x8C, 0x94, 0xDF, 0x70, 0xD4, 0x9C};
constexpr uint8_t espNowChannel = 1;

// Button pin definitions
const int SAFETY_PIN = 32; // GPIO32 -> "safety"
const int LAUNCH_PIN = 33; // GPIO33 -> "launch"

// Track last known states to detect changes
int lastSafetyState = HIGH;
int lastLaunchState = HIGH;
// Track whether a Launch On was sent (so we only send Launch Off if it was active)
bool launchSent = false;

void OnDataSent(const uint8_t *macAddr, esp_now_send_status_t status) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);

  Serial.print("Send status to ");
  Serial.print(macStr);
  Serial.print(": ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "success" : "FAILED");
  if (status != ESP_NOW_SEND_SUCCESS) {
    Serial.println("ERROR: Send failed!");
  }
}

// Helper to send a text message via ESP-NOW and print debug
void sendMessage(const String &msg) {
  Serial.print("Sending: ");
  Serial.print(msg);
  Serial.println(")");

  // Create payload buffer (no null terminator)
  size_t len = msg.length();
  uint8_t payload[len];
  memcpy(payload, msg.c_str(), len);

  esp_err_t result = esp_now_send(peerAddress, payload, len);
  if (result != ESP_OK) {
    Serial.print("esp_now_send error: ");
    Serial.println(result);
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("=== SENDER DEBUG START ===");

  // Configure button pins with internal pull-ups
  pinMode(SAFETY_PIN, INPUT_PULLUP);
  pinMode(LAUNCH_PIN, INPUT_PULLUP);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  WiFi.setSleep(false);
  
  // Start WiFi radio (required before setting channel)
  WiFi.begin();
  delay(100);

  String localMac = WiFi.macAddress();
  Serial.print("Sender MAC: ");
  Serial.println(localMac);

  char peerMacStr[18];
  snprintf(peerMacStr, sizeof(peerMacStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           peerAddress[0], peerAddress[1], peerAddress[2], peerAddress[3], peerAddress[4], peerAddress[5]);
  Serial.print("Receiver MAC: ");
  Serial.println(peerMacStr);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  Serial.println("Adding peer...");
  
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, peerAddress, 6);
  peerInfo.channel = espNowChannel;
  peerInfo.encrypt = false;

  esp_err_t addPeerResult = esp_now_add_peer(&peerInfo);
  if (addPeerResult != ESP_OK) {
    Serial.print("Failed to add peer, error code: ");
    Serial.println(addPeerResult);
    Serial.println("Attempting to continue anyway...");
  } else {
    Serial.println("Peer added successfully!");
  }

  // Now set channel AFTER esp_now_init()
  esp_err_t chResult = esp_wifi_set_channel(espNowChannel, WIFI_SECOND_CHAN_NONE);
  Serial.print("esp_wifi_set_channel result: ");
  Serial.println(chResult);

  // Read initial button states
  lastSafetyState = digitalRead(SAFETY_PIN);
  lastLaunchState = digitalRead(LAUNCH_PIN);

  Serial.print("Initial Safety state: ");
  Serial.println(lastSafetyState == LOW ? "PRESSED (LOW)" : "RELEASED (HIGH)");
  Serial.print("Initial Launch state: ");
  Serial.println(lastLaunchState == LOW ? "PRESSED (LOW)" : "RELEASED (HIGH)");

  Serial.println("Sender ready");
  Serial.println("Ready to get buttons inputs and send messages via ESP-NOW.");
}

void loop() {
  static String inputBuffer = "";

  // Handle Serial text commands as before
  while (Serial.available() > 0) {
    char c = Serial.read();

    if (c == '\r' || c == '\n') {
      inputBuffer.trim();

      if (inputBuffer == "HIGH" || inputBuffer == "LOW") {
        sendMessage(inputBuffer);
      } else if (inputBuffer.length() > 0) {
        Serial.print("Please type HIGH or LOW. You typed: ");
        Serial.println(inputBuffer);
      }

      inputBuffer = "";
    } else {
      inputBuffer += c;
    }
  }

  // Check buttons and send messages on state changes
  int currentSafety = digitalRead(SAFETY_PIN);
  if (currentSafety != lastSafetyState) {
    // Simple debounce: wait and re-read
    delay(50);
    currentSafety = digitalRead(SAFETY_PIN);
    if (currentSafety != lastSafetyState) {
      lastSafetyState = currentSafety;
      if (currentSafety == LOW) {
        // Button pressed -> GPIO goes LOW
        Serial.println("Safety button pressed -> sending 'Safety Off'");
        sendMessage("Safety Off");
      } else {
        // Button released -> GPIO goes HIGH
        Serial.println("Safety button released -> sending 'Safety On'");
        sendMessage("Safety On");
        // If safety is re-enabled while a Launch was active, force it off
        if (launchSent) {
          Serial.println("Safety re-enabled while Launch was active -> sending 'Launch Off'");
          sendMessage("Launch Off");
          launchSent = false;
        }
      }
    }
  }

  int currentLaunch = digitalRead(LAUNCH_PIN);
  if (currentLaunch != lastLaunchState) {
    // Simple debounce: wait and re-read
    delay(50);
    currentLaunch = digitalRead(LAUNCH_PIN);
    if (currentLaunch != lastLaunchState) {
      // Handle press/release based on safety state
      if (currentLaunch == LOW) {
        // Button pressed -> GPIO goes LOW
        if (currentSafety == LOW) {
          Serial.println("Launch button pressed -> Safety is OFF -> sending 'Launch On'");
          sendMessage("Launch On");
          launchSent = true;
        } else {
          Serial.println("Launch button pressed -> Safety is ON -> Launch blocked (no message sent)");
        }
      } else {
        // Button released -> GPIO goes HIGH
        if (launchSent) {
          Serial.println("Launch button released -> sending 'Launch Off'");
          sendMessage("Launch Off");
          launchSent = false;
        } else {
          Serial.println("Launch button released (no active Launch to turn off)");
        }
      }

      lastLaunchState = currentLaunch;
    }
  }

  delay(10);
}
