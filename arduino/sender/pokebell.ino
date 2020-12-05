#include <M5Stack.h>
#include <Wire.h>
#include "SparkFun_Qwiic_Keypad_Arduino_Library.h"

#define RX 16
#define TX 17
#define BAUDRATE 9600

KEYPAD keypad1;

typedef enum {
  START,
  SEND,
  NONE,
  MAX_COMMAND
} Command;

typedef enum {
  WAITING,
  INPUTTING,
  MAX_INPUT_STATUS
} InputStatus;
InputStatus status = WAITING;

void StartAction();
void NoneAction();
void SendAction();
void InputAction();
void (*action[MAX_INPUT_STATUS][MAX_COMMAND])() = {
  { StartAction, NoneAction, NoneAction },
  { NoneAction, SendAction, InputAction },
};

typedef struct CommandBuffer {
  int len;
  char buff[8];
} CommandBuffer;
CommandBuffer command = {0};

typedef struct MessageBuffer {
  int len;
  char buff[32];
} MessageBuffer;
MessageBuffer message = {0};

char button = 0;

void setup(void)
{
  M5.begin();
  M5.Power.begin();

  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(2);

  Serial.begin(9600);
  Serial.println("Qwiic KeyPad Example");

  if (keypad1.begin() == false) {
    Serial.println("Keypad does not appear to be connected. Please check wiring. Freezing...");
    while (1);
  }
  Serial.print("Initialized. Firmware Version: ");
  Serial.println(keypad1.getVersion());
  Serial.println("Press a button: * to do a space. # to go to next line.");

  Serial2.begin(BAUDRATE, SERIAL_8N1, RX, TX);

  printWelcomeMessage();
}

void loop(void)
{
  keypad1.updateFIFO();
  button = keypad1.getButton();

  if (button == -1) {
    Serial.println("No keypad detected");
    delay(1000);
  }
  else if (button != 0) {
    tone(button);
    enqueueCommand(&command, button);
    (action[status][whichCommand(&command)])();
  }

  delay(25);
}

// 入力文字をコマンドバッファにエンキュー
void enqueueCommand(CommandBuffer* command, char button) {
  if (command->len >= 4) {
    for(int i = 0 ; i < (command->len - 1) ; i++) {
      command->buff[i] = command->buff[i + 1];
    }
    command->buff[command->len - 1] = button;
  } else {
    command->buff[command->len] = button;
    command->len++;
  }
}

// コマンド判定
Command whichCommand(CommandBuffer* command) {
  char* b = command->buff;
  if ((b[0] == '*') && (b[1] == '2') && (b[2] == '*') && (b[3] == '2')) {
    resetCommand(command);
    return START;
  }
  for (int i = 0 ; i < (command->len - 1) ; i++) {
    if ((b[i] == '#') && (b[i + 1] == '#')) {
      resetCommand(command);
      return SEND;
    }
  }
  return NONE;
}

// コマンドリセット
void resetCommand(CommandBuffer* command) {
  command->len = 0;
  for (int i = 0 ; i < 4 ; i ++) {
    command->buff[i] = 0;
  }
}

// 入力文字をメッセージバッファにエンキュー
void enqueueMessage(MessageBuffer* message, char button) {
  if (message->len < 24) {
    message->buff[message->len] = button;
    message->len++;
  }
}

// メッセージリセット
void resetMessage(MessageBuffer* message) {
  message->len = 0;
  for (int i = 0 ; i < 24 ; i++) {
    message->buff[i] = 0;
  }
}

// アクション: なし
void NoneAction() {
  return;
}

// アクション: 入力開始
void StartAction() {
  M5.Lcd.clear();
  M5.Lcd.setCursor(0, 0);

  Serial.println("INPUT");
  M5.Lcd.println("INPUT");

  resetCommand(&command);
  resetMessage(&message);
  status = INPUTTING;
}

// アクション: 入力
void InputAction() {
  if ((button == '#') || (button == '*')) {
    return;
  }
  enqueueMessage(&message, button);
}

// アクション: 送信
void SendAction() {
  if ((message.len % 2) == 1) {
    message.buff[message.len - 1] = 0;
  }
  Serial.printf("Send: %s\n", message.buff);
  M5.Lcd.printf("Send: %s\n", message.buff);

  Send(message.buff);
  delay(3000);

  printWelcomeMessage();
  resetCommand(&command);
  resetMessage(&message);
  status = WAITING;
}

void Send(char* payload) {
  char command[64];
  sprintf(command, "AT$SF=%s\r\n", payload);
  Serial2.write(command);
  while(!Serial2.available());
  String response = Serial2.readStringUntil('\n');
  Serial.println(response);
  M5.Lcd.println(response);
}

// 入力文字を出力(デバッグ用)
void printButton(char button) {
  Serial.print(button);
  M5.Lcd.printf("%c", button);
}

// Welcome message.
void printWelcomeMessage() {
  M5.Lcd.clear();
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.println("Type '*2*2' to\nsend a message.");
}

int lowTone(char button) {
  switch (button) {
    case '1':
    case '2':
    case '3':
      return 697;
    case '4':
    case '5':
    case '6':
      return 770;
    case '7':
    case '8':
    case '9':
      return 852;
    case '*':
    case '0':
    case '#':
      return 941;
  }
}

void tone(char button) {
  M5.Speaker.tone(lowTone(button), 100);
  delay(100);
  M5.Speaker.mute();
}
