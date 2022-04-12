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
// #include <painlessMesh.h>

//$ Mesh Configuration
// #define MESH_PREFIX "ALiVe_MESH"
// #define MESH_PASSWORD "TmlhdCBzZWthbGkgYW5kYSBtZW5kZWNvZGUgaW5pIC1NZXJ6YQ=="
// #define MESH_PORT 5555

//$ Access Point Configuration
#define WIFI_SSID "ALiVe_AP"
#define WIFI_PASS "LeTS_ALiVe"

//*Mesh Configuration
// Scheduler userScheduler;
// painlessMesh mesh;
// int nodeNumber = 1;

int sensorReading;
String reading, prevReading;
unsigned long previousMillis = 0;
const long interval = 3468;

WebSocketsClient webSocket;

DynamicJsonDocument data(1024);
DynamicJsonDocument receivedData(1024);

void sendData();
void webSocketEvent(WStype_t type, uint8_t* payload, size_t length);
// Task taskSendMessage(TASK_SECOND * 1, TASK_FOREVER, &sendMessage);

//$ Needed for painless mesh library
// void receivedCallback(uint32_t from, String &msg);
// void newConnectionCallback(uint32_t nodeId);
// void changedConnectionCallback();
// void nodeTimeAdjustedCallback(int32_t offset);

void setup() {
  Serial.begin(115200);
  // mesh.setDebugMsgTypes(ERROR | STARTUP);

  pinMode(A0, INPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  webSocket.begin("192.168.5.1", 80, "/ws");
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);

  // mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
  // mesh.onReceive(&receivedCallback);
  // mesh.onNewConnection(&newConnectionCallback);
  // mesh.onChangedConnections(&changedConnectionCallback);
  // mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  // userScheduler.addTask(taskSendMessage);
  // taskSendMessage.enable();
}

void loop() {
  // mesh.update();
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    Serial.println("Sending Data");

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

    if (reading != prevReading) {
      sendData();
    }
  }
  prevReading = reading;

  webSocket.loop();
}

void sendData() {
  data["from"] = "moisture-sensor";
  data["to"] = "center";
  data["data"] = reading;
  String msg;
  serializeJson(data, msg);
  webSocket.sendTXT(msg);
  Serial.println("Data sent!");
  // mesh.sendBroadcast(msg);
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

//$ Needed for painless mesh library
// void receivedCallback(uint32_t from, String &msg) {
//   Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
//   deserializeJson(receivedData, msg);
//   if (data["from"].as<String>() == "center" &&
//       data["to"].as<String>() == "lamp-1") {
//     if (data["condition"].as<String>() == "true") {
//       plugCondition = true;
//       dimmer.setState(ON);
//     } else {
//       plugCondition = false;
//       dimmer.setState(OFF);
//     }
//   }
//   sendMessage();
// }

// void newConnectionCallback(uint32_t nodeId) {
//   Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
// }

// void changedConnectionCallback() { Serial.printf("Changed connections\n"); }

// void nodeTimeAdjustedCallback(int32_t offset) {
//   Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),
//   offset);
// }