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
#define LOOP_INTERNAL (50)
#define BEEP_INTERVAL (1600)
#define DISPLAY_INTERVAL (30000)
int received = false;
unsigned long receivedAt = 0;
int beep = false;

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
    receivedAt = millis();
    received = true;
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
  M5.Lcd.clear(BLACK);
}

void loop() {
  M5.update();
  unsigned long start = millis();

  connection_check_and_reconnect();
  CONSOLE.println("loop");

  if ((receivedAt + DISPLAY_INTERVAL) <= start) {
    M5.Lcd.clear(BLACK);
  }
  if (received) {
    if ((receivedAt + BEEP_INTERVAL) <= start) {
      received = false;
      M5.Speaker.mute();
    } else {
      beep = !beep;
      if (beep) {
        M5.Speaker.tone(1000);
      } else {
        M5.Speaker.mute();
      }
    }
  }

  while (millis() < (start + LOOP_INTERNAL)) {
    MqttClient.loop();
  }
}