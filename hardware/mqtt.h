#ifndef _MQTT_H
#define _MQTT_H
#include <WiFi.h>
#include <PubSubClient.h>

static const char* BROKER_IP   = "www.yanacreations.com";
static const uint16_t BROKER_PORT = 1883;
static WiFiClient wifiClient;
static PubSubClient mqttClient(wifiClient);

void mqttOnMessage(const char* topic, byte* payload, unsigned int len);

bool mqttConnected() {
  return mqttClient.connected();
}

static void mqttConnectTask(void* pv) {
  for (;;) {
    if (WiFi.status() == WL_CONNECTED && !mqttClient.connected()) {
      char id[32];
      sprintf(id, "ESP32_%lu", millis());
      if (mqttClient.connect(id)) {
        Serial.println("[MQTT] Connected");
      } else {
        Serial.println("[MQTT] Failed");
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}

static void mqttLoopTask(void* pv) {
  for (;;) {
    if (mqttClient.connected())
      mqttClient.loop();
    vTaskDelay(pdMS_TO_TICKS(300));
  }
}

void mqttBegin() {
  mqttClient.setServer(BROKER_IP, BROKER_PORT);
  mqttClient.setCallback(mqttOnMessage);
  xTaskCreatePinnedToCore(mqttConnectTask, "mqttConn", 8192, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(mqttLoopTask,    "mqttLoop", 4096, NULL, 2, NULL, 0);
  Serial.println("[MQTT] Tasks started");
}

bool mqttPublish(const char* topic, const char* payload) {
  if (!mqttClient.connected()) return false;
  return mqttClient.publish(topic, payload);
}

#endif