#include <SoftwareSerial.h>
#include <math.h>

//**********ENTER IP ADDRESS OF SERVER******************//

#define HOST_IP     "10.94.187.105"
#define HOST_PORT   "8080"
#define route       "api/update"
#define idNumber    "620171757"

#define SSID        "MonaConnect"
#define password    ""

#define stay        100

//**********PIN DEFINITIONS******************//

#define espRX         10
#define espTX         11
#define espTimeout_ms 300

// Ultrasonic Sensor Pins
#define trigPin 6
#define echoPin 7

// Tank Constants (FROM LAB MANUAL)
#define SENSOR_HEIGHT 94.5
#define MAX_HEIGHT    77.763
#define DIAMETER      61.5

SoftwareSerial esp(espRX, espTX);

// Function Prototypes
float getRadar();
float calculateWaterHeight(float radar);
float calculateReserve(float waterheight);
float calculatePercentage(float waterheight);

void setup(){

  Serial.begin(115200);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  espInit();
}

void loop() {
  float radar       = getRadar();
  float waterheight = calculateWaterHeight(radar);
  float reserve     = calculateReserve(waterheight);
  float percentage  = calculatePercentage(waterheight);

  // Arduino Nano does not support %f in snprintf - use dtostrf() instead
  char s_radar[10], s_waterheight[10], s_reserve[10], s_percentage[10];
  dtostrf(radar,       1, 2, s_radar);
  dtostrf(waterheight, 1, 2, s_waterheight);
  dtostrf(reserve,     1, 2, s_reserve);
  dtostrf(percentage,  1, 2, s_percentage);

  char message[200];
  snprintf(message, sizeof(message),
    "{\"id\":\"%s\",\"type\":\"ultrasonic\",\"radar\":%s,\"waterheight\":%s,\"reserve\":%s,\"percentage\":%s}",
    idNumber, s_radar, s_waterheight, s_reserve, s_percentage
  );

  Serial.println("---- Sensor Reading ----");
  Serial.print("Radar (distance):  "); Serial.print(s_radar);       Serial.println(" in");
  Serial.print("Water Height:      "); Serial.print(s_waterheight);  Serial.println(" in");
  Serial.print("Reserve:           "); Serial.print(s_reserve);      Serial.println(" Gal");
  Serial.print("Percentage:        "); Serial.print(s_percentage);   Serial.println(" %");
  Serial.print("JSON: ");              Serial.println(message);
  Serial.println("------------------------");

  espUpdate(message);

  delay(1000);
}

//####################################################################
//#                        UTIL FUNCTIONS                            #
//####################################################################

// Measure distance using HC-SR04
float getRadar(){

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);

  float distance = duration * 0.0133 / 2.0;  // convert to inches

  return distance;
}

// Calculate water height
float calculateWaterHeight(float radar){

  float waterheight = SENSOR_HEIGHT - radar;

  if(waterheight < 0) waterheight = 0;

  return waterheight;
}

// Calculate reserve (volume in US Gallons)
float calculateReserve(float waterheight){

  float radius = DIAMETER / 2.0;

  float volume_cubic_inches = PI * radius * radius * waterheight;

  float gallons = volume_cubic_inches / 231.0;

  return gallons;
}

// Calculate percentage
float calculatePercentage(float waterheight){

  float percentage = (waterheight / MAX_HEIGHT) * 100.0;

  return percentage;
}

//####################################################################
//#                        ESP FUNCTIONS                             #
//####################################################################

void espSend(char command[] ){
    esp.print(command);
    delay(300);
    while(esp.available()){ Serial.println(esp.readString());}
}

void espUpdate(char mssg[]){
    char espCommandString[80] = {0};  // increased from 50
    char post[512]            = {0};  // increased from 290

    snprintf(espCommandString, sizeof(espCommandString),
    "AT+CIPSTART=\"TCP\",\"%s\",%s\r\n",HOST_IP,HOST_PORT);

    espSend(espCommandString);
    delay(stay);

    snprintf(post, sizeof(post),
    "POST /%s HTTP/1.1\r\nHost: %s\r\nContent-Type: application/json\r\nContent-Length: %d\r\nConnection: close\r\n\r\n%s\r\n\r\n",
    route, HOST_IP, strlen(mssg), mssg);

    snprintf(espCommandString, sizeof(espCommandString),
    "AT+CIPSEND=%d\r\n", strlen(post));

    espSend(espCommandString);
    delay(stay);

    Serial.println(post);
    espSend(post);
    delay(stay);

    espSend("AT+CIPCLOSE\r\n");
    delay(stay);
}

void espInit(){
    char connection[100] = {0};

    esp.begin(115200);
    Serial.println("Initializing");

    esp.println("AT");
    delay(1000);

    esp.println("AT+CWMODE=1");
    delay(1000);

    while(esp.available()){ Serial.println(esp.readString());}

    snprintf(connection, sizeof(connection),
    "AT+CWJAP=\"%s\",\"%s\"\r\n",SSID,password);

    esp.print(connection);

    delay(3000);

    if(esp.available()){ Serial.print(esp.readString());}

    Serial.println("\nFinish Initializing");
}
 

