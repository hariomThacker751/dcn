# Data Communication & Networking Lab

Lab experiment code: Scilab programs and ESP32 (Arduino) sketches.

## Scilab (`scilab/`)

| Exp | File | Aim |
|-----|------|-----|
| 2 | `Exp2_Bit_Stuffing.sce` | Bit stuffing and de-stuffing with `01111110` frame flags |
| 4 | `Exp4_Hamming_Code.sce` | (7,4) Hamming code: encode, detect and correct a single-bit error |

Run in Scilab (tested on 2025.0.0): open the file in SciNotes and press Execute, or
`exec('Exp2_Bit_Stuffing.sce', -1)` from the console.

## ESP32 (`esp32/`)

| Exp | Sketch | Aim |
|-----|--------|-----|
| 5 | `Experiment_5_ESPNOW_Broadcast` | Send and receive messages between two ESP32s using ESP-NOW broadcast |
| 6 | `Experiment_6_WiFi_Survey` | Scan nearby Wi-Fi networks and print channel, RSSI and SSID |
| 7 | `Experiment_7_Unicast_Broadcast` | ESP-NOW unicast vs broadcast, printing source and destination MAC |
| 8 | `Experiment_8_Packet_Delivery_Ratio` | Send 100 broadcast + 100 unicast packets and compute packet delivery ratio |

Tools: Arduino IDE 2.3.x, esp32 board package by Espressif Systems v3.3.x, two ESP32 WROOM boards.

Notes:
- Exp 7 and 8 need the other board's station MAC. Upload once, read `My MAC` / `Station MAC`
  from the Serial Monitor (115200 baud), and put it in `PEER_MAC` / `RECEIVER_MAC`.
- Exp 8: upload with `IS_SENDER = true` to one board and `IS_SENDER = false` to the other.
- Both boards use Wi-Fi channel 1.
