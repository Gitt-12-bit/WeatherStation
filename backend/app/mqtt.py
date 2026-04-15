import os
import json
from time import time
import paho.mqtt.client as mqtt
from .config import Config


class MqttClientWrapper:
    def __init__(self):
        self.client = mqtt.Client()
        self.host = getattr(Config, "MQTT_HOST", "127.0.0.1")
        self.port = int(getattr(Config, "MQTT_PORT", 1883))

        self.username = getattr(Config, "MQTT_USERNAME", None)
        self.password = getattr(Config, "MQTT_PASSWORD", None)
        if self.username and self.password:
            self.client.username_pw_set(self.username, self.password)

        # Bind callbacks
        self.client.on_connect    = self._on_connect
        self.client.on_message    = self._on_message
        self.client.on_disconnect = self._on_disconnect

        # Try connecting
        try:
            self.client.connect(self.host, self.port)
            self.client.loop_start()   # non-blocking background loop
            print(f"[MQTT] Connected to {self.host}:{self.port}")
        except Exception as e:
            print(f"[MQTT] Connection failed: {e}")

    # ── CALLBACKS ──────────────────────────────────────────────────

    def _on_connect(self, client, userdata, flags, rc):
        if rc == 0:
            print("[MQTT] Broker connected")
            # Subscribe to the weather station topic
            client.subscribe("weatherstation/data")
            print("[MQTT] Subscribed to weatherstation/data")
        else:
            print(f"[MQTT] Connect failed with code {rc}")

    def _on_disconnect(self, client, userdata, rc):
        print(f"[MQTT] Disconnected (rc={rc})")

    def _on_message(self, client, userdata, msg):
        """
        Receives JSON published by the ESP32 on weatherstation/data
        Expected payload: {"temp": 27.5, "hum": 77.1, "pres": 991.0, "soil": 24}
        Saves to MongoDB weather collection with a Unix timestamp added.
        """
        try:
            payload = json.loads(msg.payload.decode("utf-8"))
            print(f"[MQTT] Received on {msg.topic}: {payload}")

            # Add Unix timestamp (same pattern as radar/lab4)
            payload["timestamp"] = int(time())

            # Lazy import to avoid circular import at startup
            from .functions import DB
            db = DB(Config)
            result = db.insert_weather_data(payload)

            if result:
                print(f"[MQTT] Saved to MongoDB — id: {result.inserted_id}")
            else:
                print("[MQTT] MongoDB insert returned None")

        except json.JSONDecodeError as e:
            print(f"[MQTT] Bad JSON payload: {e}")
        except Exception as e:
            print(f"[MQTT] Message handler error: {e}")

    # ── PUBLISH HELPER ─────────────────────────────────────────────

    def publish(self, topic, payload):
        try:
            self.client.publish(topic, payload)
        except Exception as e:
            print(f"[MQTT] Publish error: {e}")


Mqtt = MqttClientWrapper()
