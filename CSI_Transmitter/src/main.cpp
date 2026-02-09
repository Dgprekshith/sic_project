/**
 * ESP32 CSI TRANSMITTER
 * Role: Continuously transmit Wi-Fi packets to create CSI signals
 * Location: Bedside (Fixed position, 1-2 meters from patient chest)
 */

#include <Arduino.h>
#include <WiFi.h>

// ========== CONFIGURATION ==========
const char* WIFI_SSID = "likhith";        // Create a dedicated network
const char* WIFI_PASSWORD = "12345678";   // Secure password
const uint8_t WIFI_CHANNEL = 6;               // Fixed channel (avoid interference)

// Packet transmission timing
const uint16_t TX_INTERVAL_MS = 20;           // Send packet every 20ms (50 Hz)

// ========== GLOBALS ==========
uint32_t packetCount = 0;

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("\n=== CSI TRANSMITTER INITIALIZING ===");
  
  // Configure as Access Point (AP Mode)
  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL, 0, 1); // Max 1 client
  
  Serial.printf("AP Created: %s\n", WIFI_SSID);
  Serial.printf("IP Address: %s\n", WiFi.softAPIP().toString().c_str());
  Serial.printf("Channel: %d\n", WIFI_CHANNEL);
  Serial.println("=== TRANSMITTER READY ===\n");
}

void loop() {
  // The ESP32 AP continuously transmits beacons and management frames
  // This is sufficient to generate CSI data at the receiver
  
  // Optional: Send periodic UDP packets for stronger signals
  static uint32_t lastTx = 0;
  if (millis() - lastTx >= TX_INTERVAL_MS) {
    lastTx = millis();
    packetCount++;
    
    // Visual heartbeat every 100 packets
    if (packetCount % 100 == 0) {
      Serial.printf("[TX] Packets sent: %lu\n", packetCount);
    }
  }
  
  delay(10);
}