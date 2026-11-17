#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <esp_mac.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

// ================= CONFIGURATION =================

const uint8_t GROUP_ID = 2;
const uint8_t CHANNEL = 1;

// Upload 1: true for sender
// Upload 2: false for receiver
const bool IS_SENDER = true;

// Station MAC of the receiver board
const uint8_t RECEIVER_MAC[6] = {
  0x14, 0x33, 0x5C, 0x04, 0x34, 0xA4
};

const uint8_t BROADCAST_MAC[6] = {
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

const uint16_t PACKETS_PER_MODE = 100;
const unsigned long SEND_INTERVAL_MS = 100;

// ================= PACKET STRUCTURE =================

struct Message {
  uint8_t group;
  uint8_t mode;        // 0 = Broadcast, 1 = Unicast
  uint16_t reserved;
  uint32_t sequence;
};

static_assert(sizeof(Message) == 8, "Unexpected Message structure size");

// ================= GLOBAL VARIABLES =================

QueueHandle_t receiveQueue = nullptr;

bool seen[2][PACKETS_PER_MODE] = {};
uint16_t uniqueCount[2] = {};
uint16_t duplicateCount[2] = {};

uint8_t sendMode = 0;
uint32_t sendSequence = 0;

unsigned long lastSendAt = 0;
unsigned long lastPacketAt = 0;

bool summaryPrinted = false;
bool ready = false;

// ================= HELPER FUNCTIONS =================

void printMacAddress() {
  uint8_t mac[6] = {};
  esp_err_t result = esp_read_mac(mac, ESP_MAC_WIFI_STA);

  if (result != ESP_OK) {
    Serial.printf("MAC read failed: %s\n", esp_err_to_name(result));
    return;
  }

  Serial.printf(
    "Station MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
    mac[0], mac[1], mac[2],
    mac[3], mac[4], mac[5]
  );
}

bool receiverMacIsSet() {
  for (int i = 0; i < 6; i++) {
    if (RECEIVER_MAC[i] != 0) return true;
  }
  return false;
}

bool addPeer(const uint8_t *mac) {
  esp_now_peer_info_t peer = {};

  memcpy(peer.peer_addr, mac, 6);
  peer.channel = CHANNEL;
  peer.ifidx = WIFI_IF_STA;
  peer.encrypt = false;

  esp_err_t result = esp_now_add_peer(&peer);

  if (result != ESP_OK) {
    Serial.printf("Peer registration failed: %s\n", esp_err_to_name(result));
    return false;
  }

  return true;
}

// ================= RECEIVE CALLBACK =================

void onReceive(const esp_now_recv_info_t *info,
               const uint8_t *data,
               int length) {

  if (length != sizeof(Message)) return;

  Message message;
  memcpy(&message, data, sizeof(message));

  if (message.group != GROUP_ID ||
      message.mode > 1 ||
      message.sequence >= PACKETS_PER_MODE) {
    return;
  }

  if (receiveQueue != nullptr) {
    // Non-blocking: the callback never waits for queue space.
    xQueueSend(receiveQueue, &message, 0);
  }
}

// ================= SUMMARY =================

void printSummary() {
  Serial.println("\n========== DELIVERY SUMMARY ==========");

  for (int mode = 0; mode < 2; mode++) {
    float delivery = 100.0f * uniqueCount[mode] / PACKETS_PER_MODE;

    Serial.printf(
      "%s: %u/%u unique, %u duplicates, %.1f%% delivered\n",
      mode == 0 ? "Broadcast" : "Unicast",
      uniqueCount[mode],
      PACKETS_PER_MODE,
      duplicateCount[mode],
      delivery
    );
  }

  Serial.println("======================================");
}

// ================= SETUP =================

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\nESP-NOW Packet Test");
  Serial.println(IS_SENDER ? "Role: SENDER" : "Role: RECEIVER");

  if (!WiFi.mode(WIFI_STA)) {
    Serial.println("ERROR: Wi-Fi station mode failed");
    return;
  }

  delay(100);
  printMacAddress();

  esp_err_t result = esp_wifi_set_channel(CHANNEL, WIFI_SECOND_CHAN_NONE);
  if (result != ESP_OK) {
    Serial.printf("ERROR setting channel: %s\n", esp_err_to_name(result));
    return;
  }

  result = esp_now_init();
  if (result != ESP_OK) {
    Serial.printf("ERROR initializing ESP-NOW: %s\n", esp_err_to_name(result));
    return;
  }

  if (IS_SENDER) {
    if (!receiverMacIsSet()) {
      Serial.println("ERROR: Configure RECEIVER_MAC first");
      return;
    }

    if (!addPeer(BROADCAST_MAC)) return;
    if (!addPeer(RECEIVER_MAC)) return;
  } else {
    receiveQueue = xQueueCreate(32, sizeof(Message));

    if (receiveQueue == nullptr) {
      Serial.println("ERROR: Queue creation failed");
      return;
    }

    result = esp_now_register_recv_cb(onReceive);
    if (result != ESP_OK) {
      Serial.printf("ERROR registering callback: %s\n", esp_err_to_name(result));
      return;
    }
  }

  ready = true;
  Serial.println(IS_SENDER ? "Sender ready" : "Receiver ready");
}

// ================= MAIN LOOP =================

void loop() {
  if (!ready) {
    delay(1000);
    return;
  }

  // -------- SENDER --------
  if (IS_SENDER) {
    if (sendMode >= 2) {
      delay(1000);
      return;
    }

    if (millis() - lastSendAt < SEND_INTERVAL_MS) return;
    lastSendAt = millis();

    Message message = {
      GROUP_ID,
      sendMode,
      0,
      sendSequence
    };

    const uint8_t *destination = (sendMode == 0) ? BROADCAST_MAC : RECEIVER_MAC;

    esp_err_t result = esp_now_send(
      destination,
      reinterpret_cast<const uint8_t *>(&message),
      sizeof(message)
    );

    if (result != ESP_OK) {
      Serial.printf(
        "Send request failed: mode=%u sequence=%lu error=%s\n",
        sendMode,
        static_cast<unsigned long>(sendSequence),
        esp_err_to_name(result)
      );
    }

    sendSequence++;

    if (sendSequence >= PACKETS_PER_MODE) {
      Serial.println(sendMode == 0 ? "Broadcast batch finished"
                                   : "Unicast batch finished");
      sendMode++;
      sendSequence = 0;

      if (sendMode == 1) {
        Serial.println("Waiting 2 seconds before unicast...");
        delay(2000);
      }
    }

    return;
  }

  // -------- RECEIVER --------
  Message message;

  while (xQueueReceive(receiveQueue, &message, 0) == pdTRUE) {
    lastPacketAt = millis();
    summaryPrinted = false;

    if (seen[message.mode][message.sequence]) {
      duplicateCount[message.mode]++;
    } else {
      seen[message.mode][message.sequence] = true;
      uniqueCount[message.mode]++;
    }
  }

  // Print after 3 seconds without a processed packet.
  if (!summaryPrinted &&
      lastPacketAt != 0 &&
      millis() - lastPacketAt >= 3000) {
    printSummary();
    summaryPrinted = true;
  }
}
