#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

const uint8_t GROUP_ID = 1;
const uint8_t CHANNEL = 1;

// Replace with the other ESP32's station MAC address
const uint8_t PEER_MAC[6] = {0xD4, 0x8A, 0xFC, 0x9F, 0x31, 0x5C};
const uint8_t BROADCAST_MAC[6] = {255, 255, 255, 255, 255, 255};

struct Message {
  uint8_t group;
  uint8_t kind;      // 0 = broadcast, 1 = unicast
  uint32_t sequence;
};

uint32_t nextSequence = 1;
bool peerReady = false;

void printMac(const uint8_t *mac) {
  for (int i = 0; i < 6; i++) {
    if (i > 0) Serial.print(':');
    if (mac[i] < 16) Serial.print('0');
    Serial.print(mac[i], HEX);
  }
}

bool peerMacIsSet() {
  for (int i = 0; i < 6; i++) {
    if (PEER_MAC[i] != 0) return true;
  }
  return false;
}

void onReceive(const esp_now_recv_info_t *info, const uint8_t *data, int length) {
  if (length != sizeof(Message)) return;
  Message message;
  memcpy(&message, data, sizeof(message));
  if (message.group != GROUP_ID || message.kind > 1) return;
  Serial.print(message.kind == 0 ? "Broadcast" : "Unicast");
  Serial.printf(" message %lu from ", static_cast<unsigned long>(message.sequence));
  printMac(info->src_addr);
  Serial.print(" to ");
  printMac(info->des_addr);
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  delay(500);
  WiFi.mode(WIFI_STA);
  Serial.print("My MAC: ");
  Serial.println(WiFi.macAddress());

  if (esp_wifi_set_channel(CHANNEL, WIFI_SECOND_CHAN_NONE) != ESP_OK) {
    Serial.println("Could not set Wi-Fi channel");
    return;
  }
  if (esp_now_init() != ESP_OK) {
    Serial.println("Could not initialize ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(onReceive);

  // peer 1: broadcast address
  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, BROADCAST_MAC, 6);
  peer.channel = CHANNEL;
  peer.ifidx = WIFI_IF_STA;
  if (esp_now_add_peer(&peer) != ESP_OK) {
    Serial.println("Could not add broadcast peer");
    return;
  }
  if (!peerMacIsSet()) {
    Serial.println("Set PEER_MAC to the other board's MAC and upload again");
    return;
  }

  // peer 2: the other board (unicast)
  memcpy(peer.peer_addr, PEER_MAC, 6);
  if (esp_now_add_peer(&peer) != ESP_OK) {
    Serial.println("Could not add unicast peer");
    return;
  }
  peerReady = true;
}

void loop() {
  if (!peerReady) {
    delay(1000);
    return;
  }

  Message broadcastMessage = {GROUP_ID, 0, nextSequence};
  Message unicastMessage = {GROUP_ID, 1, nextSequence};

  esp_err_t broadcastResult = esp_now_send(BROADCAST_MAC,
      reinterpret_cast<uint8_t *>(&broadcastMessage), sizeof(broadcastMessage));
  Serial.printf("Broadcast %lu queued: %s\n",
                static_cast<unsigned long>(nextSequence),
                broadcastResult == ESP_OK ? "yes" : "no");

  delay(100);

  esp_err_t unicastResult = esp_now_send(PEER_MAC,
      reinterpret_cast<uint8_t *>(&unicastMessage), sizeof(unicastMessage));
  Serial.printf("Unicast %lu queued: %s\n",
                static_cast<unsigned long>(nextSequence),
                unicastResult == ESP_OK ? "yes" : "no");

  nextSequence++;
  delay(5000);
}
