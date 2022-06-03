/*
    PINOUT
      PIN A0 = Analog Sensor
      PIN 14 = LED 1
      PIN 12 = LED 2
*/

//$ Include Library
#include <Arduino.h>
#include <ArduinoJson.h>
#include <WebSocketsClient.h>

//$ Access Point Configuration
#define WIFI_SSID "ALiVe_AP"
#define WIFI_PASS "LeTS_ALiVe"

int sensorReading;
String reading, prevReading;
unsigned long previousMillis = 0;
const long interval = 3468;

WebSocketsClient webSocket;

DynamicJsonDocument data(1024);
DynamicJsonDocument receivedData(1024);

//* Device Name
const String deviceName = "sensor-3";
const String sensorType = "moistureSensor";
const String centerName = "center";

void sendData();
void webSocketEvent(WStype_t type, uint8_t* payload, size_t length);

void setup() {
  Serial.begin(115200);

  pinMode(A0, INPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  webSocket.begin("192.168.5.1", 80, "/ws");
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    Serial.println("Sending Data");

    if (webSocket.isConnected()) {
      Serial.println("WebSocket Connected");
    } else {
      Serial.println("Connecting (Please Connect)");
      webSocket.begin("192.168.5.1", 80, "/ws");
    }

    sensorReading = analogRead(A0);
    Serial.println(sensorReading);

    if (sensorReading <= 430) {
      reading = "Terlalu banyak air";
      Serial.println("Terlalu banyak air");
    } else if (sensorReading > 430 && sensorReading < 350) {
      reading = "Basah";
      Serial.println("Basah");
    } else if (sensorReading >= 350) {
      reading = "Kering";
      Serial.println("Kering");
    }

    // if (reading != prevReading) {
    sendData();
    // }
  }
  prevReading = reading;

  webSocket.loop();
}

void sendData() {
  data["from"] = deviceName;
  data["sensorType"] = sensorType;
  data["to"] = centerName;
  // data["data"] = reading;
  data["data"] = sensorReading;
  String msg;
  serializeJson(data, msg);
  webSocket.sendTXT(msg);
  Serial.println("Data sent!");
}

void webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
  if (type == WStype_TEXT) {
    deserializeJson(receivedData, payload);

    String myData;
    serializeJson(receivedData, myData);
    String from = receivedData["from"].as<String>();
    String to = receivedData["to"].as<String>();
    String condition = receivedData["condition"].as<String>();

    Serial.println(myData);
    Serial.println(from);
    Serial.println(to);
    Serial.println(condition);
  }
}