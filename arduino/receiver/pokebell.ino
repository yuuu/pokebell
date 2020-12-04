#include <string.h>
#include <stdio.h>

#include <M5Stack.h>
#define CONSOLE Serial
#define MODEM Serial2
 
#define TINY_GSM_MODEM_UBLOX
#include <TinyGsmClient.h> 
TinyGsm modem(MODEM);
TinyGsmClient ctx(modem);

#include <Arduino_JSON.h>
#include <PubSubClient.h>
PubSubClient MqttClient(ctx);
const char *THING_NAME = "m5stack";
const char *PUB_TOPIC = "/devices/m5stack/events";
const char *SUB_TOPIC = "pokebell";
 
#define __VERSION__   "1.0.0"
#define LOOP_INTERVAL (6000)

void callback(char* topic, byte* payload, unsigned int length) {
  String buf_t = String(topic);
  payload[length] = '\0'; /* https://hawksnowlog.blogspot.com/2017/06/convert-byte-array-to-string.html */
  String buf_p = String((char*) payload);

  CONSOLE.print("Incoming topic: ");
  CONSOLE.println(buf_t);
  CONSOLE.print("Payload>");
  CONSOLE.println(buf_p);
  M5.Lcd.clear(BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(0,0);

  JSONVar json = JSON.parse((char*) payload);
  if(json.hasOwnProperty("message")){
    M5.Lcd.println("メッセージを受信しました:");
    M5.Lcd.println((const char*)json["message"]);
  }
}

void cellular_disconnect() {
  MqttClient.disconnect();
  modem.restart();
}

void cellular_connect() {
  M5.Lcd.clear(BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(0,0);
  
  CONSOLE.print(F("modem.restart()"));
  M5.Lcd.print(F("modem.restart()"));
  modem.restart();
  CONSOLE.println(F("Ok"));
  M5.Lcd.println(F("Ok"));

  CONSOLE.print(F("waitForNetwork()"));
  M5.Lcd.print(F("waitForNetwork()"));
  while (!modem.waitForNetwork()) CONSOLE.print(".");
  CONSOLE.println(F("Ok"));
  M5.Lcd.println(F("Ok"));

  CONSOLE.print(F("gprsConnect(soracom.io)"));
  M5.Lcd.print(F("gprsConnect(soracom.io)"));
  modem.gprsConnect("soracom.io", "sora", "sora");
  CONSOLE.println(F("Ok"));
  M5.Lcd.println(F("Ok"));

  CONSOLE.print(F("isNetworkConnected()"));
  M5.Lcd.print(F("isNetworkConnected()"));
  while (!modem.isNetworkConnected()) CONSOLE.print(".");
  CONSOLE.println(F("Ok"));
  M5.Lcd.println(F("Ok"));

  CONSOLE.print(F("My IP addr: "));
  M5.Lcd.print(F("My IP addr: "));
  IPAddress ipaddr = modem.localIP();
  CONSOLE.println(ipaddr);
  M5.Lcd.println(ipaddr);
}

void mqtt_disconnect() {
  CONSOLE.println(MqttClient.state());
  MqttClient.disconnect();
}

void mqtt_connect() {
  CONSOLE.print("ThingName(mqtt_id): ");
  CONSOLE.println(THING_NAME);
  MqttClient.setServer("beam.soracom.io", 1883);
  MqttClient.setCallback(callback);
  if (!MqttClient.connect(THING_NAME)) {
    CONSOLE.println(MqttClient.state());
  }
  MqttClient.subscribe(SUB_TOPIC);
}

void connection_check_and_reconnect() {
  if (!MqttClient.connected()) {
    mqtt_disconnect();
    cellular_disconnect();
    cellular_connect();
    mqtt_connect();
  }
}

void setup() {
  delay(500);
  Serial.begin(115200);
  M5.begin();
  M5.Lcd.clear(BLACK);
  M5.Lcd.setTextColor(WHITE);
  String f20 = "genshin-regular-20pt";
  M5.Lcd.loadFont(f20, SD);
  CONSOLE.println();
  CONSOLE.println(__VERSION__);
  M5.Lcd.println(__VERSION__);
  MODEM.begin(115200, SERIAL_8N1, 16, 17); // 3G MODULE
  cellular_connect();
  mqtt_connect();
}

void loop() {
  M5.update();
  connection_check_and_reconnect();
  /* Implement for Loop */
  CONSOLE.println("loop");
  char payload[512];
  sprintf(payload, "{\"uptime\":%lu}", millis()/1000); /* for example */
  MqttClient.publish(PUB_TOPIC, payload);

  unsigned long next = millis();
  while (millis() < next + LOOP_INTERVAL) {
      MqttClient.loop();
  }
}