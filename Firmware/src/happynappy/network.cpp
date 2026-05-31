#include "network.h"

#define MQTT_MAX_PACKET_SIZE 4096
#include <WiFi.h>
#include <PubSubClient.h>

#include "app_state.h"
#include "config.h"

bool stopSampling = false;

namespace {

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

void mqttCallback(char *topic, byte *payload, unsigned int length) {
  Serial.print("\n🚨 Received message on topic: ");
  Serial.println(topic);

  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("Message: ");
  Serial.println(message);

  if (String(topic) == "esp32/wake_alert") {
    Serial.println("\n=====================================");
    Serial.println("🚨 WAKE UP ALERT! 🚨");
    Serial.println("=====================================");
    buzzMotor();  // Activate vibration motor
  } else if (String(topic) == "esp32/control") {
    if (message == "STOP_SAMPLING") {
      stopSampling = true;
      Serial.println("🛑 Received STOP command - stopping data collection");
    }
  }
}

}  // namespace

void networkSetup() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(kWifiSsid, kWifiPass);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("WiFi connected, IP: ");
  Serial.println(WiFi.localIP());

  mqttClient.setServer(kMqttHost, kMqttPort);
  mqttClient.setBufferSize(4096);  // Explicitly set buffer size
  mqttClient.setCallback(mqttCallback);
}

void networkEnsureConnected() {
  if (!mqttClient.connected()) {
    while (!mqttClient.connected()) {
      String clientId = "esp32-max30102-" + String((uint32_t)ESP.getEfuseMac(), HEX);
      if (mqttClient.connect(clientId.c_str())) {
        Serial.println("MQTT connected");
        mqttClient.subscribe("esp32/wake_alert");
        mqttClient.subscribe("esp32/control");
        Serial.println("🔔 Subscribed to wake alerts and control messages");
      } else {
        Serial.print("MQTT connect failed, rc=");
        Serial.print(mqttClient.state());
        Serial.println(" retrying in 2s");
        delay(2000);
      }
    }
  }
  mqttClient.loop();
}

bool publishSamples(const long *samples, uint16_t count) {
  String payload;
  payload.reserve(3600);
  payload += "{\"ir\":[";
  for (uint16_t i = 0; i < count; ++i) {
    payload += String(samples[i]);
    if (i + 1 < count) {
      payload += ',';
    }
  }
  payload += "],\"sample_rate\":";
  payload += String(kSampleRateHz);
  payload += '}';

  return mqttClient.publish(kMqttTopic, payload.c_str());
}
