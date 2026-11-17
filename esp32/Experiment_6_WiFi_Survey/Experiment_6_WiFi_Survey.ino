#include <Arduino.h>
#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  delay(500);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  Serial.println("ESP32 Wi-Fi survey");
}

void loop() {
  Serial.println("Scanning...");
  int networkCount = WiFi.scanNetworks(false, true);
  if (networkCount < 0) {
    Serial.println("Scan failed");
  } else {
    Serial.printf("Found %d network(s)\n", networkCount);
    for (int i = 0; i < networkCount; i++) {
      String ssid = WiFi.SSID(i);
      if (ssid.length() == 0) ssid = "<hidden>";
      Serial.printf("%2d  Channel %2d  RSSI %4d dBm  %s\n", i + 1, WiFi.channel(i), WiFi.RSSI(i), ssid.c_str());
    }
  }
  WiFi.scanDelete();
  Serial.println();
  delay(15000);
}
