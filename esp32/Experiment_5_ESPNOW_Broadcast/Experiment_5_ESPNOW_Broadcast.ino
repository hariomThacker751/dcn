#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>  

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

typedef struct struct_message {
  char message[64];
} struct_message;

struct_message myData;

void Sent(const wifi_tx_info_t *tx_info, esp_now_send_status_t status) {
  Serial.print("Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Failed");
}

void Recv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len) {
  struct_message receivedData;
  if (len >= (int)sizeof(receivedData)) len = sizeof(receivedData) - 1;  // safety
  memcpy(&receivedData, incomingData, len);
  receivedData.message[sizeof(receivedData.message)-1] = '\0';

  Serial.print("Received message: ");
  Serial.println(receivedData.message);
}

void setup() {
  Serial.begin(115200);
  delay(200);

  WiFi.mode(WIFI_STA);
  Serial.print("My MAC: ");
  Serial.println(WiFi.macAddress());


  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register callbacks
  esp_now_register_send_cb(Sent);
  esp_now_register_recv_cb(Recv);

  // Add broadcast peer
  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 1;              // 0 = current WiFi channel (we set it to 1 above)
  peerInfo.encrypt = false;
  peerInfo.ifidx = WIFI_IF_STA;      // IMPORTANT on v3.x

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
}

void loop() {
  // Prepare the message
  snprintf(myData.message, sizeof(myData.message), "Hi, How are you?");

  // Send broadcast
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t*)&myData, sizeof(myData));
  if (result == ESP_OK) {
    Serial.println("Message sent (queued)");
  } else {
    Serial.print("Error sending the message, err=");
    Serial.println(result);
  }

  delay(5000);
}
