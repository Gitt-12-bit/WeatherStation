#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_BMP280.h>
#include <DHT.h>
#include <WiFi.h>
#include <time.h>
#include <SPI.h>
#include <Wire.h>
#include "NTP.h"
#include "mqtt.h"
#include <HTTPClient.h>
#include <WiFiClient.h>

// ── PIN CONFIG ─────────────────────────
#define TFT_CS   5
#define TFT_DC   17
#define TFT_RST  16
#define TFT_MOSI 23
#define TFT_MISO 19
#define TFT_SCK  18
#define DHTPIN   4
#define DHTTYPE  DHT22
#define SOIL_PIN 32
#define BMP_SCL  22
#define BMP_SDA  21

// ── COLORS ─────────────────────────────
#define COL_BG       0x0841
#define COL_PANEL    0x1082
#define COL_WHITE    0xFFFF
#define COL_GOOD     0x07E0
#define COL_WARN     0xFFE0
#define COL_BAD      0xF800
#define COL_ACCENT   0x04FF
#define COL_ORANGE   0xFD20
#define COL_BROWN    0x8200
#define COL_DKBROWN  0x4100
#define COL_BLUE     0x035F
#define COL_LTBLUE   0x567F
#define COL_DKGREY   0x39E7
#define COL_MIDGREY  0x528A

// ── WIFI RECONNECT STATE ───────────────
#define WIFI_CHECK_INTERVAL 5000   // check every 5s
#define WIFI_RECONNECT_TIMEOUT 20000 // 20s reconnect attempt window

// ── MQTT CLIENT CONFIG ─────────────────
static const char* pubtopic    = "620171757";
static const char* mqtt_server = "www.yanacreations.com";
static uint16_t    mqtt_port   = 1883;

// ── WIFI ───────────────────────────────
const char* ssid      = "Galaxyy";
const char* backendIP = "10.194.108.105";
enum WiFiState { WIFI_CONNECTED, WIFI_WAITING, WIFI_RECONNECTING };
static WiFiState wifiState        = WIFI_CONNECTED;
static unsigned long lastWifiCheck   = 0;
static unsigned long reconnectStart  = 0;
static unsigned long reconnectDot    = 0;

// ── OBJECTS ────────────────────────────
Adafruit_ILI9341 tft(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCK, TFT_RST, TFT_MISO);
Adafruit_BMP280  bmp(&Wire);   // explicitly use Wire so BMP_SCL/SDA take effect
DHT              dht(DHTPIN, DHTTYPE);

// ── DATA ───────────────────────────────
float temp=0, hum=0, pres=0, heatIndex=0, altitude=0;
int   soil=0;
bool  bmpOK=false;

// ── PUBLISH TASK ───────────────────────
static volatile bool publishPending = false;
static char          publishPayload[160];

// ── FORWARD DECLARATIONS ───────────────
void drawStaticLayout();
void updateValues();
void drawHeader();
void drawCard(int x, int y, int w, int h, uint16_t borderCol);
void drawIconThermo(int cx, int cy, uint16_t col);
void drawIconDrop(int cx, int cy, uint16_t col);
void drawIconGauge(int cx, int cy, uint16_t col);
void drawIconPlant(int cx, int cy, uint16_t col);
void drawIconHeat(int cx, int cy, uint16_t col);
void drawIconAlt(int cx, int cy, uint16_t col);
void readSensors();
void postToBackend();

static void publishTask(void* pv) {
  for (;;) {
    if (publishPending) {
      publishPending = false;
      postToBackend();
      mqttPublish("weatherstation/data", publishPayload);
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

// ── MQTT CALLBACK ──────────────────────
void mqttOnMessage(const char* topic, byte* payload, unsigned int len) {
  Serial.println("[MQTT] Message received");
}

// ───────────────────────────────────────
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n========================");
  Serial.println("BOOT START");
  Serial.println("========================");

  // ── I2C INIT — must come before bmp.begin() ──
  Wire.begin(BMP_SDA, BMP_SCL);   // SDA=21, SCL=22

  // ── TFT INIT ──
  Serial.println("[STEP] TFT INIT");
  tft.begin(40000000);
  tft.setRotation(0);
  tft.fillScreen(COL_BG);
  tft.setTextSize(1);
  tft.setTextColor(COL_ACCENT, COL_BG);
  tft.setCursor(60, 150);
  tft.print("WEATHER STATION");
  tft.setCursor(80, 165);
  tft.setTextColor(COL_DKGREY, COL_BG);
  tft.print("booting...");
  Serial.println("[OK] TFT INITIALIZED");

  // ── DHT INIT ──
  Serial.println("[STEP] DHT INIT");
  dht.begin();
  delay(2000);
  Serial.println("[OK] DHT READY");

  // ── BMP INIT ──
  Serial.println("[STEP] BMP INIT (I2C SDA=21 SCL=22)");
  if (!bmp.begin(0x76)) {
    Serial.println("[ERROR] BMP NOT FOUND");
    bmpOK = false;
  } else {
    Serial.println("[OK] BMP CONNECTED");
    bmpOK = true;
  }

  // ── WIFI ──
  Serial.println("[STEP] WIFI CONNECT");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; i++) {
    Serial.printf("[WIFI] %s (RSSI:%d)\n", WiFi.SSID(i).c_str(), WiFi.RSSI(i));
  }
  WiFi.begin(ssid);
  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 40) {
    delay(500);
    Serial.print(".");
    retry++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[OK] WIFI CONNECTED");
    Serial.print("[INFO] IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n[ERROR] WIFI FAILED");
  }

  // ── NTP + MQTT ──
  ntpBegin();
  mqttBegin();
  xTaskCreatePinnedToCore(publishTask, "publish", 8192, NULL, 1, NULL, 0);
  Serial.println("[OK] SYSTEM READY");

  // ── DRAW STATIC LAYOUT ONCE ──
  readSensors();
  drawStaticLayout();
  updateValues();
}

// ───────────────────────────────────────
void postToBackend() {
  if (WiFi.status() != WL_CONNECTED) return;
  WiFiClient client;
  HTTPClient http;
  http.setTimeout(2000);
  String url = "http://10.194.108.105:8080/api/weather/update";
  Serial.printf("[HTTP] Connecting to: %s\n", url.c_str());
  http.begin(client, url);
  http.addHeader("Content-Type", "application/json");
  char payload[256];
  snprintf(payload, sizeof(payload),
    "{\"id\":\"620171757\",\"temp\":%.1f,\"hum\":%.1f,\"pres\":%.1f,\"soil\":%d,\"heatIndex\":%.1f,\"altitude\":%.1f}",
    temp, hum, pres, soil, heatIndex, altitude
  );
  int httpCode = http.POST(payload);
  Serial.printf("[HTTP] Response Code: %d\n", httpCode);
  http.end();
}

void handleWiFi(unsigned long now) {
  switch (wifiState) {

    case WIFI_CONNECTED:
      // Periodically verify we're still up
      if (now - lastWifiCheck >= WIFI_CHECK_INTERVAL) {
        lastWifiCheck = now;
        if (WiFi.status() != WL_CONNECTED) {
          Serial.println("[WIFI] Lost connection — starting reconnect");
          WiFi.disconnect();
          delay(100);             // one-time tiny delay, not in a loop
          WiFi.begin(ssid);
          reconnectStart = now;
          reconnectDot   = now;
          wifiState      = WIFI_RECONNECTING;
        }
      }
      break;

    case WIFI_RECONNECTING:
      // Print a dot every 500ms so serial shows progress (non-blocking)
      if (now - reconnectDot >= 500) {
        reconnectDot = now;
        Serial.print(".");
      }

      if (WiFi.status() == WL_CONNECTED) {
        // ── Success ──
        Serial.println("\n[WIFI] Reconnected!");
        Serial.print("[INFO] IP: "); Serial.println(WiFi.localIP());
        ntpBegin();
        mqttBegin();
        lastWifiCheck = now;
        wifiState     = WIFI_CONNECTED;

      } else if (now - reconnectStart >= WIFI_RECONNECT_TIMEOUT) {
        // ── Timed out — wait before retrying ──
        Serial.println("\n[WIFI] Reconnect timed out — will retry in 5s");
        WiFi.disconnect();
        lastWifiCheck = now;      // reuse check timer as the cooldown
        wifiState     = WIFI_WAITING;
      }
      break;

    case WIFI_WAITING:
      // Cooldown period before next attempt
      if (now - lastWifiCheck >= WIFI_CHECK_INTERVAL) {
        Serial.println("[WIFI] Retrying connection...");
        WiFi.begin(ssid);
        reconnectStart = now;
        reconnectDot   = now;
        wifiState      = WIFI_RECONNECTING;
      }
      break;
  }
}

// ───────────────────────────────────────

void loop() {
  static unsigned long lastSensor  = 0;
  static unsigned long lastPublish = 0;
  unsigned long now = millis();

  // ── NON-BLOCKING WIFI WATCHDOG ─────────────────────────
  handleWiFi(now);

  // ── SENSOR READ (always runs, WiFi-independent) ─────────
  if (now - lastSensor >= 2000) {
    lastSensor = now;
    readSensors();
    updateValues();   // TFT keeps updating — WiFi/MQTT dots show current status
  }

  // ── PUBLISH (skipped gracefully when offline) ───────────
  if (now - lastPublish >= 5000) {
    lastPublish = now;
    if (WiFi.status() == WL_CONNECTED) {    // only queue when actually online
      snprintf(publishPayload, sizeof(publishPayload),
        "{\"id\":\"620171757\",\"temp\":%.1f,\"hum\":%.1f,\"pres\":%.1f,\"soil\":%d,\"heatIndex\":%.1f,\"altitude\":%.1f}",
        temp, hum, pres, soil, heatIndex, altitude
      );
      publishPending = true;
    }
  }
}

// ───────────────────────────────────────
void readSensors() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (!isnan(h) && !isnan(t)) {
    hum  = h;
    temp = t;
    float hi = -8.78469475556
               + 1.61139411    * t
               + 2.33854883889 * h
               - 0.14611605    * t * h
               - 0.01230809296 * t * t
               - 0.01642482778 * h * h
               + 0.00221733    * t * t * h
               + 0.00072546    * t * h * h
               - 0.00000358    * t * t * h * h;
    heatIndex = (t >= 27 && h >= 40) ? hi : t;
  }
  if (bmpOK) {
    pres     = bmp.readPressure() / 100.0;
    altitude = bmp.readAltitude(1013.25);
  }
  int raw = analogRead(SOIL_PIN);
  soil = constrain(map(raw, 3400, 900, 0, 100), 0, 100);
  Serial.printf("[SENSOR] T:%.1f HI:%.1f H:%.1f P:%.1f ALT:%.1f S:%d\n",
                temp, heatIndex, hum, pres, altitude, soil);
}

// ── (All drawing functions unchanged below) ──────────────────────────────────

void drawStaticLayout() {
  tft.fillScreen(COL_BG);
  drawHeader();
  drawCard(4,   48,  114, 86, COL_ORANGE);
  drawCard(122, 48,  114, 86, COL_LTBLUE);
  drawCard(4,   138, 114, 86, COL_GOOD);
  drawCard(122, 138, 114, 86, COL_WARN);
  drawCard(4,   228, 114, 86, COL_BAD);
  drawCard(122, 228, 114, 86, COL_ACCENT);
  tft.setTextSize(1);
  tft.setTextColor(COL_DKGREY, COL_PANEL);
  tft.setCursor(10,  54);  tft.print("TEMP");
  tft.setCursor(128, 54);  tft.print("HUMIDITY");
  tft.setCursor(10,  144); tft.print("PRESSURE");
  tft.setCursor(128, 144); tft.print("SOIL");
  tft.setCursor(10,  234); tft.print("HEAT IDX");
  tft.setCursor(128, 234); tft.print("ALTITUDE");
  drawIconThermo(22,  108, COL_ORANGE);
  drawIconDrop(  140, 108, COL_LTBLUE);
  drawIconGauge( 22,  198, COL_GOOD);
  drawIconPlant( 140, 198, COL_WARN);
  drawIconHeat(  22,  288, COL_BAD);
  drawIconAlt(   140, 288, COL_ACCENT);
}

void updateValues() {
  uint16_t tCol = (temp < 18) ? COL_LTBLUE : (temp < 28) ? COL_ORANGE : COL_BAD;
  tft.fillRect(44, 63, 70, 36, COL_PANEL);
  tft.setTextSize(2); tft.setTextColor(tCol, COL_PANEL);
  tft.setCursor(44, 65); tft.print(temp, 1);
  tft.setTextSize(1); tft.setTextColor(COL_WHITE, COL_PANEL); tft.print("C");
  tft.fillRect(44, 92, 68, 9, COL_PANEL);
  tft.setTextSize(1); tft.setTextColor(tCol, COL_PANEL); tft.setCursor(44, 93);
  if      (temp < 18) tft.print("COLD   ");
  else if (temp < 28) tft.print("COMFORT");
  else                tft.print("HOT    ");
  tft.fillRect(8, 128, 106, 3, COL_BG);
  tft.fillRect(8, 128, constrain((int)map((int)temp,0,50,0,106),0,106), 3, tCol);

  uint16_t hCol = (hum < 30) ? COL_WARN : (hum < 70) ? COL_LTBLUE : COL_BLUE;
  tft.fillRect(162, 63, 68, 36, COL_PANEL);
  tft.setTextSize(2); tft.setTextColor(hCol, COL_PANEL);
  tft.setCursor(162, 65); tft.print(hum, 1);
  tft.setTextSize(1); tft.setTextColor(COL_WHITE, COL_PANEL); tft.print("%");
  tft.fillRect(162, 92, 68, 9, COL_PANEL);
  tft.setTextSize(1); tft.setTextColor(hCol, COL_PANEL); tft.setCursor(162, 93);
  if      (hum < 30) tft.print("DRY    ");
  else if (hum < 70) tft.print("NORMAL ");
  else               tft.print("HUMID  ");
  tft.fillRect(126, 128, 106, 3, COL_BG);
  tft.fillRect(126, 128, constrain((int)map((int)hum,0,100,0,106),0,106), 3, hCol);

  uint16_t pCol = !bmpOK ? COL_DKGREY : (pres<1000)?COL_WARN:(pres<1020)?COL_GOOD:COL_LTBLUE;
  tft.fillRect(44, 153, 72, 40, COL_PANEL);
  tft.setTextSize(2); tft.setTextColor(pCol, COL_PANEL); tft.setCursor(44, 155);
  if (bmpOK) tft.print((int)pres); else tft.print("N/A");
  tft.setTextSize(1); tft.setTextColor(COL_WHITE, COL_PANEL); tft.print(" hPa");
  tft.fillRect(44, 178, 68, 9, COL_PANEL);
  tft.setTextSize(1); tft.setTextColor(pCol, COL_PANEL); tft.setCursor(44, 179);
  if (!bmpOK)           tft.print("NO SENS");
  else if (pres < 1000) tft.print("LOW    ");
  else if (pres < 1020) tft.print("NORMAL ");
  else                  tft.print("HIGH   ");

  uint16_t sCol = (soil<20)?COL_BAD:(soil<60)?COL_WARN:COL_GOOD;
  tft.fillRect(162, 153, 68, 36, COL_PANEL);
  tft.setTextSize(2); tft.setTextColor(sCol, COL_PANEL); tft.setCursor(162, 155);
  tft.print(soil);
  tft.setTextSize(1); tft.setTextColor(COL_WHITE, COL_PANEL); tft.print("%");
  tft.fillRect(162, 182, 68, 9, COL_PANEL);
  tft.setTextSize(1); tft.setTextColor(sCol, COL_PANEL); tft.setCursor(162, 183);
  if      (soil < 20) tft.print("DRY    ");
  else if (soil < 60) tft.print("MOIST  ");
  else                tft.print("WET    ");
  tft.fillRect(126, 218, 106, 3, COL_BG);
  tft.fillRect(126, 218, constrain((int)map(soil,0,100,0,106),0,106), 3, sCol);

  uint16_t hiCol = (heatIndex<27)?COL_GOOD:(heatIndex<32)?COL_WARN:(heatIndex<41)?COL_ORANGE:COL_BAD;
  tft.fillRect(44, 243, 70, 36, COL_PANEL);
  tft.setTextSize(2); tft.setTextColor(hiCol, COL_PANEL); tft.setCursor(44, 245);
  tft.print(heatIndex, 1);
  tft.setTextSize(1); tft.setTextColor(COL_WHITE, COL_PANEL); tft.print("C");
  tft.fillRect(44, 272, 68, 9, COL_PANEL);
  tft.setTextSize(1); tft.setTextColor(hiCol, COL_PANEL); tft.setCursor(44, 273);
  if      (heatIndex < 27) tft.print("COMFORT");
  else if (heatIndex < 32) tft.print("CAUTION");
  else if (heatIndex < 41) tft.print("EXTREME");
  else                     tft.print("DANGER ");

  uint16_t aCol = bmpOK ? COL_ACCENT : COL_DKGREY;
  tft.fillRect(162, 243, 72, 40, COL_PANEL);
  tft.setTextSize(2); tft.setTextColor(aCol, COL_PANEL); tft.setCursor(162, 245);
  if (bmpOK) tft.print((int)altitude); else tft.print("N/A");
  tft.setTextSize(1); tft.setTextColor(COL_WHITE, COL_PANEL); tft.print(" m");
  tft.fillRect(162, 268, 68, 9, COL_PANEL);
  tft.setTextSize(1); tft.setTextColor(aCol, COL_PANEL); tft.setCursor(162, 269);
  if (!bmpOK)               tft.print("NO SENS");
  else if (altitude < 0)    tft.print("BELOW SL");
  else if (altitude < 500)  tft.print("LOW    ");
  else if (altitude < 2000) tft.print("MID    ");
  else                      tft.print("HIGH   ");

  uint16_t wCol = (WiFi.status() == WL_CONNECTED) ? COL_GOOD : COL_BAD;
  tft.fillCircle(206, 13, 5, wCol);
  uint16_t mCol = mqttConnected() ? COL_GOOD : COL_WARN;
  tft.fillCircle(228, 13, 5, mCol);
}

void drawHeader() {
  tft.fillRect(0, 0, 240, 44, COL_PANEL);
  tft.fillRect(0, 0, 4, 44, COL_ACCENT);
  tft.setTextSize(2);
  tft.setTextColor(COL_WHITE, COL_PANEL); tft.setCursor(12, 6); tft.print("WEATHER");
  tft.setTextColor(COL_ACCENT, COL_PANEL); tft.print(" STN");
  tft.setTextSize(1); tft.setTextColor(COL_DKGREY, COL_PANEL);
  tft.setCursor(194, 24); tft.print("WiFi");
  tft.setCursor(220, 24); tft.print("MQ");
}

void drawCard(int x, int y, int w, int h, uint16_t borderCol) {
  tft.fillRoundRect(x, y, w, h, 6, COL_PANEL);
  tft.drawRoundRect(x, y, w, h, 6, borderCol);
}

void drawIconThermo(int cx, int cy, uint16_t col) {
  tft.drawRoundRect(cx-3, cy-18, 6, 22, 2, col);
  tft.fillRect(cx-1, cy-8, 2, 10, col);
  tft.fillCircle(cx, cy+6, 5, col);
  tft.drawCircle(cx, cy+6, 5, col);
  tft.drawFastHLine(cx+3, cy-14, 3, COL_MIDGREY);
  tft.drawFastHLine(cx+3, cy-8,  3, COL_MIDGREY);
  tft.drawFastHLine(cx+3, cy-2,  3, COL_MIDGREY);
}

void drawIconDrop(int cx, int cy, uint16_t col) {
  tft.drawLine(cx, cy-14, cx-7, cy+2, col);
  tft.drawLine(cx, cy-14, cx+7, cy+2, col);
  tft.fillCircle(cx, cy+4, 7, col);
}

void drawIconGauge(int cx, int cy, uint16_t col) {
  for (int a = 0; a <= 180; a += 20) {
    float rad = a * 3.14159 / 180.0;
    tft.drawPixel(cx + (int)(10*cos(rad)), cy - (int)(10*sin(rad)), col);
  }
  tft.drawFastHLine(cx-10, cy, 20, col);
  tft.drawLine(cx, cy, cx+5, cy-7, COL_WHITE);
  tft.fillCircle(cx, cy, 2, col);
}

void drawIconPlant(int cx, int cy, uint16_t col) {
  tft.fillRect(cx-6, cy+2, 12, 8, COL_DKBROWN);
  tft.drawRect(cx-6, cy+2, 12, 8, col);
  tft.drawFastVLine(cx, cy-12, 14, col);
  tft.drawLine(cx,   cy-4,  cx-6, cy-10, col);
  tft.drawLine(cx-6, cy-10, cx-6, cy-4,  col);
  tft.drawLine(cx-6, cy-4,  cx,   cy-4,  col);
  tft.drawLine(cx,   cy-8,  cx+6, cy-14, col);
  tft.drawLine(cx+6, cy-14, cx+6, cy-8,  col);
  tft.drawLine(cx+6, cy-8,  cx,   cy-8,  col);
}

void drawIconHeat(int cx, int cy, uint16_t col) {
  tft.drawLine(cx,   cy-14, cx-5, cy-4,  col);
  tft.drawLine(cx-5, cy-4,  cx-7, cy+4,  col);
  tft.drawLine(cx-7, cy+4,  cx,   cy+8,  col);
  tft.drawLine(cx,   cy+8,  cx+7, cy+4,  col);
  tft.drawLine(cx+7, cy+4,  cx+5, cy-4,  col);
  tft.drawLine(cx+5, cy-4,  cx,   cy-14, col);
  tft.drawLine(cx,   cy-6,  cx-3, cy+2,  col);
  tft.drawLine(cx-3, cy+2,  cx,   cy+5,  col);
  tft.drawLine(cx,   cy+5,  cx+3, cy+2,  col);
  tft.drawLine(cx+3, cy+2,  cx,   cy-6,  col);
}

void drawIconAlt(int cx, int cy, uint16_t col) {
  tft.drawLine(cx,    cy-14, cx-10, cy+6,  col);
  tft.drawLine(cx-10, cy+6,  cx+10, cy+6,  col);
  tft.drawLine(cx+10, cy+6,  cx,    cy-14, col);
  tft.drawLine(cx,    cy-14, cx-3,  cy-8,  col);
  tft.drawLine(cx-3,  cy-8,  cx+3,  cy-8,  col);
  tft.drawLine(cx+3,  cy-8,  cx,    cy-14, col);
  tft.drawFastVLine(cx, cy-14, 6, col);
  tft.drawLine(cx, cy-14, cx-2, cy-11, col);
  tft.drawLine(cx, cy-14, cx+2, cy-11, col);
}