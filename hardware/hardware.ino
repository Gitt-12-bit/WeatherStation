//##################################################################################################################
//##                                      ELET2415 DATA ACQUISITION SYSTEM CODE                                   ##
//##################################################################################################################

// IMPORT ALL REQUIRED LIBRARIES
#include <rom/rtc.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <time.h>

//IMPORT IMAGES
#include "lockclose.h"
#include "lockopen.h"

#ifndef _WIFI_H
#include <WiFi.h>
#include <HTTPClient.h>
#endif

#ifndef STDLIB_H
#include <stdlib.h>
#endif

#ifndef STDIO_H
#include <stdio.h>
#endif

#ifndef ARDUINO_H
#include <Arduino.h>
#endif

// DEFINE VARIABLES
uint8_t currentDigit = 1;
bool lockState = false;

uint8_t d1 = 0;
uint8_t d2 = 0;
uint8_t d3 = 0;
uint8_t d4 = 0;

// Hardware Pin Definitions
#define BTN1 14
#define BTN2 27
#define BTN3 26
#define POT_PIN 34
#define TFT_CS 5
#define TFT_DC 17
#define TFT_RST 16

// Screen Dimensions Constants
#define SCREEN_WIDTH 240
#define BOX_W 45
#define BOX_H 50
#define BOX_GAP 8

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

// IMPORT FONTS FOR TFT DISPLAY
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>

// MQTT CLIENT CONFIG
static const char* pubtopic      = "620171757";
static const char* subtopic[]    = {"620171757_sub", "/elet2415"};
static const char* mqtt_server   = "www.yanacreations.com";
static uint16_t    mqtt_port     = 1883;

// WIFI CREDENTIALS - UPDATE THESE TO MATCH YOUR NETWORK
const char* ssid     = "MonaConnect";
const char* password = "";

// BACKEND IP - UPDATE THIS TO YOUR COMPUTER'S LOCAL IPv4 ADDRESS
// Run 'ipconfig' in Windows terminal and look for IPv4 Address under your WiFi adapter
const char* backendIP = "10.94.187.105";  

// TASK HANDLES
TaskHandle_t xButtonCheckeHandle = NULL;

// FUNCTION DECLARATIONS
void drawStaticUI(void);
void drawDigitBox(uint16_t x, uint8_t number, bool selected);
void updateDigits(void);
void checkPasscode(void);
void showLockState(void);
void vButtonCheck(void* pvParameters);
void syncNTP(void);

//##################################################################################################################

void setup() {
  Serial.begin(115200);

  // Configure GPIO pins
  pinMode(BTN1, INPUT_PULLUP);
  pinMode(BTN2, INPUT_PULLUP);
  pinMode(BTN3, INPUT_PULLUP);

  // Initialise TFT display
  tft.begin();
  tft.setRotation(0);  // Portrait mode: 240x320
  tft.fillScreen(ILI9341_BLACK);

  // Show connecting message on TFT while WiFi connects
  tft.setFont(&FreeSansBold9pt7b);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(30, 160);
  tft.print("Connecting WiFi...");

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Sync time via NTP (UTC-5 for EST, adjust offset for your timezone)
  // UTC offset in seconds: EST = -5*3600, AST (Jamaica) = -5*3600
  syncNTP();

  // Draw the main UI
  drawStaticUI();
  updateDigits();
  showLockState();

  // Start button check task on core 1
  xTaskCreatePinnedToCore(vButtonCheck, "Button Check", 4000, (void*)1, 2, &xButtonCheckeHandle, 1);
}

void loop() {
  uint16_t raw      = analogRead(POT_PIN);
  uint8_t mappedVal = map(raw, 0, 4095, 0, 9);

  bool changed = false;
  if (currentDigit == 1 && d1 != mappedVal) { d1 = mappedVal; changed = true; }
  if (currentDigit == 2 && d2 != mappedVal) { d2 = mappedVal; changed = true; }
  if (currentDigit == 3 && d3 != mappedVal) { d3 = mappedVal; changed = true; }
  if (currentDigit == 4 && d4 != mappedVal) { d4 = mappedVal; changed = true; }

  if (changed) updateDigits();

  vTaskDelay(100 / portTICK_PERIOD_MS);
}

//####################################################################
//#                          NTP SYNC                                #
//####################################################################
void syncNTP(void) {
  // UTC-5 = Jamaica/EST. Change first argument if in a different timezone.
  // Format: configTime(gmtOffset_sec, daylightOffset_sec, ntpServer)
  configTime(-5 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  Serial.print("Syncing NTP time");
  struct tm timeinfo;
  int retries = 0;
  while (!getLocalTime(&timeinfo) && retries < 20) {
    Serial.print(".");
    delay(500);
    retries++;
  }

  if (retries < 20) {
    Serial.println();
    Serial.println("NTP time synced successfully!");
    Serial.printf("Current time: %02d:%02d:%02d\n",
                  timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  } else {
    Serial.println();
    Serial.println("NTP sync failed - continuing without time sync.");
  }
}

//####################################################################
//#                       UI LAYOUT (LAB SCREEN)                     #
//####################################################################
void drawStaticUI(void) {
  tft.fillScreen(ILI9341_BLACK);

  // Title
  tft.setFont(&FreeSansBold9pt7b);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(55, 30);
  tft.print("REMOTE LOCK");

  // Horizontal rule
  tft.drawFastHLine(20, 40, 200, ILI9341_DARKGREY);
}

//####################################################################
//#                         DIGIT DRAWING                            #
//####################################################################
void drawDigitBox(uint16_t x, uint8_t number, bool selected) {
  uint16_t color = selected ? ILI9341_YELLOW : ILI9341_BLUE;

  tft.fillRoundRect(x, 250, BOX_W, BOX_H, 4, color);

  tft.setFont(&FreeSansBold18pt7b);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(x + 12, 287);
  tft.print(number);
}

void updateDigits(void) {
  // Total width = (4 * BOX_W) + (3 * BOX_GAP) = 180 + 24 = 204
  // Center: (240 - 204) / 2 = 18
  int startX = 18;

  drawDigitBox(startX,                        d1, currentDigit == 1);
  drawDigitBox(startX + (BOX_W + BOX_GAP),    d2, currentDigit == 2);
  drawDigitBox(startX + 2 * (BOX_W + BOX_GAP), d3, currentDigit == 3);
  drawDigitBox(startX + 3 * (BOX_W + BOX_GAP), d4, currentDigit == 4);
}

//####################################################################
//#                        PASSCODE CHECK                            #
//####################################################################
void checkPasscode(void) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected - cannot check passcode.");
    return;
  }

  WiFiClient client;
  HTTPClient http;

  // Build the backend URL using the configured IP
  String url = "http://";
  url += backendIP;
  url += ":8080/api/check/combination";

  http.begin(client, url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  // Build passcode string from individual digits
  char message[30];
  sprintf(message, "passcode=%d%d%d%d", d1, d2, d3, d4);
  Serial.print("Checking passcode: ");
  Serial.println(message);

  int httpResponseCode = http.POST(message);
  Serial.print("HTTP Response code: ");
  Serial.println(httpResponseCode);

  if (httpResponseCode > 0) {
    String received = http.getString();
    Serial.print("Response: ");
    Serial.println(received);

    lockState = (received.indexOf("complete") != -1);
    showLockState();
  } else {
    Serial.println("HTTP POST failed.");
  }

  http.end();
}

//####################################################################
//#                        LOCK STATE DISPLAY                        #
//####################################################################
void showLockState(void) {
  // Clear the image and status text area
  tft.fillRect(0, 60, 240, 180, ILI9341_BLACK);

  int lockX = (SCREEN_WIDTH - 104) / 2;

  if (lockState) {
    tft.drawRGBBitmap(lockX, 60, lockopen, 104, 97);
    tft.setFont(&FreeSansBold9pt7b);
    tft.setTextColor(ILI9341_GREEN);
    tft.setCursor(45, 190);
    tft.print("ACCESS GRANTED");
  } else {
    tft.drawRGBBitmap(lockX, 60, lockclose, 104, 103);
    tft.setFont(&FreeSansBold9pt7b);
    tft.setTextColor(ILI9341_RED);
    tft.setCursor(50, 190);
    tft.print("ACCESS DENIED");
  }
}

//####################################################################
//#                        BUTTON CHECK TASK                         #
//####################################################################
void vButtonCheck(void* pvParameters) {
  for (;;) {
    // BTN1 - cycle through digits 1→2→3→4→1
    if (digitalRead(BTN1) == LOW) {
      currentDigit = (currentDigit % 4) + 1;
      updateDigits();
      vTaskDelay(300 / portTICK_PERIOD_MS);
    }

    // BTN2 - validate passcode against backend
    if (digitalRead(BTN2) == LOW) {
      checkPasscode();
      vTaskDelay(300 / portTICK_PERIOD_MS);
    }

    // BTN3 - reset lock to closed/denied state
    if (digitalRead(BTN3) == LOW) {
      lockState = false;
      showLockState();
      vTaskDelay(300 / portTICK_PERIOD_MS);
    }

    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}
