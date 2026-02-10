#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

// ================= CONFIG =================

// WiFi AP
const char* AP_SSID     = "Nothing";
const char* AP_PASSWORD = "bablu123";

IPAddress AP_IP(192, 168, 4, 1);
IPAddress AP_GATEWAY(192, 168, 4, 1);
IPAddress AP_SUBNET(255, 255, 255, 0);

// Receiver ESP32 IP (CHANGE LAST NUMBER TO MATCH RECEIVER)
IPAddress RECEIVER_IP(172, 18, 181, 167);
const uint16_t UDP_PORT = 4210;

// Send rate
const uint16_t SEND_INTERVAL_MS = 20;

// LED
#define STATUS_LED 2

WiFiUDP udp;

unsigned long lastSend = 0;
unsigned long lastSecond = 0;
uint32_t packetsSent = 0;

// ================= SETUP =================
void setup() {
    Serial.begin(115200);
    delay(1000);

    pinMode(STATUS_LED, OUTPUT);
    digitalWrite(STATUS_LED, LOW);

    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(AP_IP, AP_GATEWAY, AP_SUBNET);

    if (!WiFi.softAP(AP_SSID, AP_PASSWORD)) {
        while (1) delay(1000);
    }

    udp.begin(UDP_PORT);

    lastSend = millis();
    lastSecond = millis();
}

// ================= LOOP =================
void loop() {
    unsigned long now = millis();

    if (WiFi.softAPgetStationNum() > 0 &&
        now - lastSend >= SEND_INTERVAL_MS) {

        lastSend = now;

        char payload[32];
        snprintf(payload, sizeof(payload), "BEACON:%lu", packetsSent);

        udp.beginPacket(RECEIVER_IP, UDP_PORT);
        udp.write((uint8_t*)payload, strlen(payload));

        if (udp.endPacket()) {
            packetsSent++;
            if (packetsSent % 25 == 0) {
                digitalWrite(STATUS_LED, !digitalRead(STATUS_LED));
            }
        }
    }

    if (now - lastSecond >= 1000) {
        lastSecond = now;
        Serial.print("Packets Sent: ");
        Serial.print(packetsSent);
        Serial.print(" | Stations: ");
        Serial.println(WiFi.softAPgetStationNum());
    }

    delay(1);
}