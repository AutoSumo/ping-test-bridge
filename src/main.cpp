#include <Arduino.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <WebSocketsServer.h>
#include "connection-details.h"
#include <esp_wifi.h>
#include <esp_task_wdt.h>

WebSocketsServer server = WebSocketsServer(8080);
TaskHandle_t wsTask;

char buffer[1024];

void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
	switch(type) {
		case WStype_DISCONNECTED:
			Serial.println("Disconnection from client!");
			break;
		case WStype_CONNECTED:
			Serial.println("Connection from client!");
			break;
		case WStype_TEXT:
            // Send text data data over serial
            Serial.println((char*) payload);
            Serial.flush();
			break;
		case WStype_BIN:
            Serial.print("Got binary message of length ");
            Serial.println(length);
			break;
		case WStype_ERROR:			
            Serial.println("Websocket error!");
            break;
		case WStype_FRAGMENT_TEXT_START:
		case WStype_FRAGMENT_BIN_START:
		case WStype_FRAGMENT:
		case WStype_FRAGMENT_FIN:
		case WStype_PING:
		case WStype_PONG:
			break;
	}

}

void websocketTask(void *pvParameters) {
    // Connect to websocket server
    Serial.print("WS running on core ");
    Serial.println(xPortGetCoreID());

    server.onEvent(onWebSocketEvent);
    server.begin();

    while(true) {
        server.loop();
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("");
    Serial.println("Starting ping test bridge...");

    // Start wifi
    esp_wifi_set_ps(WIFI_PS_NONE);
    WiFi.mode(WIFI_MODE_AP);
    WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);

    Serial.print("ESP32 running access point \"");
    Serial.print(WIFI_SSID);
    Serial.print("\" with ip: ");
    Serial.println(WiFi.softAPIP());

    // Start websocket
    xTaskCreatePinnedToCore(websocketTask, "WS Task", 10000, NULL, 0, &wsTask, 0);
}

void loop() {
    delay(1);
    if(Serial.available()) {
        size_t read = Serial.readBytesUntil('\n', buffer, 1024);
        server.broadcastTXT(buffer, read);
    }
}