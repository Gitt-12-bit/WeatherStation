import os
import paho.mqtt.client as mqtt
from .config import Config

class MqttClientWrapper:
    def __init__(self):
        self.client = mqtt.Client()
        self.host = getattr(Config, "MQTT_HOST", "127.0.0.1")  # fallback localhost
        self.port = int(getattr(Config, "MQTT_PORT", 1883))
        # Optional: set username/password if needed
        self.username = getattr(Config, "MQTT_USERNAME", None)
        self.password = getattr(Config, "MQTT_PASSWORD", None)

        if self.username and self.password:
            self.client.username_pw_set(self.username, self.password)

        # Try connecting immediately
        try:
            self.client.connect(self.host, self.port)
            print(f"MQTT connected to {self.host}:{self.port}")
        except Exception as e:
            print(f"MQTT connection failed: {e}")

Mqtt = MqttClientWrapper()