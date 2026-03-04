#! /usr/bin/env python
from app import app, Config

# Try importing MQTT, but handle if it fails
try:
    from app import Mqtt
    mqtt_available = True
except ImportError:
    print("Warning: MQTT module not available. Skipping MQTT startup.")
    mqtt_available = False

if __name__ == "__main__":

    # START MQTT CLIENT IF AVAILABLE
    if mqtt_available:
        try:
            Mqtt.client.loop_start()
        except Exception as e:
            print(f"Failed to start MQTT client: {e}")

    # HOST AND PORT CONFIG
    host = getattr(Config, "FLASK_RUN_HOST", "0.0.0.0")  # LAN-accessible host
    port = getattr(Config, "FLASK_RUN_PORT", 8080)       # Match frontend proxy

    # Fix for localhost string
    if isinstance(host, str) and host.lower() == "localhost":
        host = "0.0.0.0"

    print(f"Starting Flask on {host}:{port} (Debug={getattr(Config,'FLASK_DEBUG', True)})")

    # RUN FLASK APP
    app.run(debug=getattr(Config,'FLASK_DEBUG', True), host=host, port=port)