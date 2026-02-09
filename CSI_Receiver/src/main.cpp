/**
 * ESP32 CSI RECEIVER
 * Role: Capture CSI data and stream to Python backend via UDP
 * Location: Opposite side of bed (1-2 meters from patient)
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include "esp_wifi.h"
#include "esp_wifi_types.h"

// ========== NETWORK CONFIGURATION ==========
const char* WIFI_SSID = "likhith";
const char* WIFI_PASSWORD = "12345678";

// Python backend server (Laptop running Python script)
const char* SERVER_IP = "192.168.137.84";        // Change to your laptop's IP
const uint16_t SERVER_PORT = 5005;

// ========== CSI CONFIGURATION ==========
#define CSI_SUBCARRIERS 64                    // ESP32 supports 64 subcarriers
#define CSI_BUFFER_SIZE 128

// ========== GLOBALS ==========
WiFiUDP udp;
uint32_t csiPacketCount = 0;
bool wifiConnected = false;

// ========== CSI DATA STRUCTURE (Simplified) ==========
typedef struct {
  uint32_t timestamp;
  uint8_t mac[6];
  int8_t rssi;
  uint8_t rate;
  uint8_t sig_mode;
  uint8_t channel;
  uint16_t len;                               // CSI data length
  int8_t data[CSI_BUFFER_SIZE];               // Amplitude values
} csi_packet_t;

// ========== CSI CALLBACK FUNCTION ==========
void csi_rx_callback(void *ctx, wifi_csi_info_t *data) {
  if (!wifiConnected || data == NULL) return;
  
  csiPacketCount++;
  
  // Prepare simplified CSI packet
  csi_packet_t packet;
  packet.timestamp = millis();
  memcpy(packet.mac, data->mac, 6);
  packet.rssi = data->rx_ctrl.rssi;
  packet.rate = data->rx_ctrl.rate;
  packet.sig_mode = data->rx_ctrl.sig_mode;
  packet.channel = data->rx_ctrl.channel;
  packet.len = data->len;
  
  // Extract CSI amplitude (simplified - real CSI is complex I/Q)
  // This is a MOCK extraction - actual ESP-IDF library required for real parsing
  for (int i = 0; i < CSI_BUFFER_SIZE && i < data->len; i++) {
    packet.data[i] = data->buf[i];            // Raw CSI buffer
  }
  
  // Send via UDP to Python backend
  udp.beginPacket(SERVER_IP, SERVER_PORT);
  udp.write((uint8_t*)&packet, sizeof(packet));
  udp.endPacket();
  
  // Debug output every 50 packets
  if (csiPacketCount % 50 == 0) {
    Serial.printf("[CSI] Packets: %lu | RSSI: %d dBm\n", csiPacketCount, packet.rssi);
  }
}

// ========== SETUP ==========
void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("\n=== CSI RECEIVER INITIALIZING ===");
  
  // Connect to Transmitter's AP
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  Serial.print("Connecting to AP");
  uint8_t attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    Serial.printf("\nConnected! IP: %s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println("\nFAILED TO CONNECT!");
    while(1) delay(1000);
  }
  
  // Initialize UDP
  udp.begin(SERVER_PORT);
  
  // Enable CSI collection
  wifi_csi_config_t csi_config = {
    .lltf_en = true,                          // Enable Long Training Field
    .htltf_en = true,                         // Enable HT-LTF
    .stbc_htltf2_en = false,
    .ltf_merge_en = false,
    .channel_filter_en = false,
    .manu_scale = false,
    .shift = 0
  };
  
  esp_wifi_set_csi_config(&csi_config);
  esp_wifi_set_csi_rx_cb(&csi_rx_callback, NULL);
  esp_wifi_set_csi(true);
  
  Serial.println("=== CSI RECEIVER READY ===\n");
}

// ========== LOOP ==========
void loop() {
  // Check Wi-Fi connection health
  if (WiFi.status() != WL_CONNECTED) {
    wifiConnected = false;
    Serial.println("[ERROR] Wi-Fi disconnected! Reconnecting...");
    WiFi.reconnect();
    delay(5000);
  }
  
  delay(100);
}
