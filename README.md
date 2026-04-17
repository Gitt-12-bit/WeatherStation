# IoT Weather Station with Remote Monitoring

## Description
The aim of this project is to design and implementation of an IoT-based Weather Station with remote monitoring capabilities. The system integrates an embedded hardware unit, a backend server, and a web-based graphical user interface (GUI) to display, and analyze the environmental data collected via the sensors.

The system is divided into three major components: Hardware, Backend, and Frontend.

---

## Hardware Component
The hardware system is responsible for:

- Reading and processing environmental data from sensors:
  - Air Temperature  
  - Humidity  
  - Atmospheric Pressure  
  - Altitude  
  - Heat Index  
  - Soil Moisture  

- Displaying real-time sensor data on a TFT display
- Transmitting sensor data via WiFi using MQTT

### Hardware Components
- ESP32 
- BMP280 Sensor (Pressure & Altitude)  
- DHT Sensor (Temperature, Heat Index & Humidity)  
- Soil Moisture Sensor (Soil Moisture)
- TFT Display (ILI9341)  
- Breadboard prototype / Final PCBed System

---

## Backend Component (Flask API + MongoDB Database)

The backend system is responsible for:

- Receiving sensor data from the hardware (MQTT / HTTP)
- Processing and storing the data in a MongoDB database
- Providing API endpoints for data access
- Facilitating communication between hardware and frontend

---

## Frontend Component (Web GUI)

The frontend system is responsible for:

- Providing a user-friendly graphical interface
- Fetching data from backend through APIs
- Displaying real-time and historical data
- Rendering charts and tables
- Auto-refreshing data to show realtime updates
- Providing unit conversion abilities on database data

---

## Hardware Setup

Download and install Arduino IDE:  
https://www.arduino.cc/en/software  

### Required Libraries
Install the following libraries via the Arduino IDE:

1. Adafruit GFX Library  
2. Adafruit ILI9341  
3. Adafruit BMP280  
4. DHT Sensor Library  
5. WiFi (ESP32)  
6. PubSubClient  
7. ArduinoJson  

---

## Backend Setup (Flask API)

### Navigate to backend folder
```sh
cd backend
```

### Create virtual environment
Windows:
```sh
python -m venv env
```

Linux:
```sh
python3 -m venv env
```

### Activate virtual environment
Windows:
```sh
.\env\Scripts\activate
```

Linux:
```sh
source env/bin/activate
```

### Install dependencies
```sh
pip install -r requirements.txt
```

### Create `.env` file
Create a `.env` file in the backend folder and include:

```env
FLASK_RUN_HOST=<your-ip-address>
```

Also include:
- Database credentials  
- MQTT broker configuration  

### Run Flask API
Windows:
```sh
py run.py
```

Linux:
```sh
python3 run.py
```

---

## Frontend Setup (Vue + Vuetify + Vite)

### Navigate to frontend folder
```sh
cd frontend
```

### Install dependencies
```sh
npm install
```

### Run development server
```sh
npm run dev
```

### Build for production
```sh
npm run build
```

---

### 🔧 Modify `vite.config.js`
```js
proxy: {
  '^/api*': { 
    target: 'http://<your-ip-address>:8080/',
    changeOrigin: false,
  },   
}
```

---

## System Workflow

1. Sensors collect environmental data  
2. ESP32 processes the data  
3. Data is displayed on the TFT screen  
4. Data is transmitted via WiFi using MQTT  
5. Backend receives and stores data in the database  
6. Frontend fetches data via API  
7. GUI displays real-time and historical data  

---

## Key Features

- Real-time environmental monitoring  
- Wireless communication (WiFi + MQTT)  
- Real-time TFT display of data
- Web-based dashboard with charts and tables  
- Auto-refresh on webpage
- Database storage and retrieval  
- Clean and modular architecture  

---

## Final System

The completed system includes:

- Fully assembled and packaged Weather Station
- Functional backend and database  
- Interactive web-based GUI  
- Complete IoT pipeline:
  Sensors → ESP32 → MQTT → Backend → Database → API → Frontend  
---

## Deliverables

- Breadboard Prototype (Sensors + TFT)  
- Data Transmission System (WiFi + MQTT)  
- Backend API and Database  
- Web-Based GUI  
- Fully Packaged PCB System  
- PowerPoint Presentation on Product   
- Final Technical Report  

---
