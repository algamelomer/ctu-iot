#include <WiFi.h>
#include <RemoteXY.h>
#include <ESP32Servo.h>
#include <Keypad.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>

////////////////////////////////////////////////////
// REMOTEXY
////////////////////////////////////////////////////

#define REMOTEXY_MODE__WIFI_POINT
#define REMOTEXY_WIFI_SSID "omar"
#define REMOTEXY_WIFI_PASSWORD "omar5040"
#define REMOTEXY_SERVER_PORT 6377

#pragma pack(push, 1)
uint8_t RemoteXY_CONF[] = {
  255,1,0,0,0,36,0,19,0,0,0,0,31,1,106,200,1,1,1,0,
  1,30,80,46,46,0,1,31,79,112,101,110,0
};

struct {
  uint8_t open_button;
  uint8_t connect_flag;
} RemoteXY;

#pragma pack(pop)

////////////////////////////////////////////////////
// SERVO
////////////////////////////////////////////////////

Servo doorServo;
#define SERVO_PIN 26

////////////////////////////////////////////////////
// KEYPAD (YOUR WORKING PINS)
////////////////////////////////////////////////////

const byte ROWS = 4;
const byte COLS = 3;

char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

byte rowPins[ROWS] = {13, 12, 14, 27};
byte colPins[COLS] = {32, 33, 25};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

////////////////////////////////////////////////////
// RFID RC522
////////////////////////////////////////////////////

#define SS_PIN 5
#define RST_PIN 22
MFRC522 rfid(SS_PIN, RST_PIN);

String validUID = "A1 B2 C3 D4";  // change later

////////////////////////////////////////////////////
// OLED
////////////////////////////////////////////////////

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

////////////////////////////////////////////////////
// SYSTEM VARIABLES
////////////////////////////////////////////////////

String password = "7897";
String input = "";

bool doorOpen = false;
int wrongAttempts = 0;

////////////////////////////////////////////////////
// SETUP
////////////////////////////////////////////////////

void setup() {
  Serial.begin(115200);

  RemoteXY_Init();

  // Servo
  doorServo.setPeriodHertz(50);
  doorServo.attach(SERVO_PIN, 500, 2400);
  doorServo.write(0);

  // RFID
  SPI.begin();
  rfid.PCD_Init();

  // OLED
  Wire.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();

  showMsg("System Ready");
}

////////////////////////////////////////////////////
// LOOP
////////////////////////////////////////////////////

void loop() {
  RemoteXY_Handler();

  handleKeypad();
  handleRFID();

  if (RemoteXY.open_button == 1) {
    openDoor("REMOTE");
    RemoteXY.open_button = 0;
  }
}

////////////////////////////////////////////////////
// KEYPAD
////////////////////////////////////////////////////

void handleKeypad() {
  char key = keypad.getKey();

  if (key) {
    Serial.println(key);

    if (key == '#') {
      checkPassword();
      input = "";
    }
    else if (key == '*') {
      input = "";
    }
    else {
      input += key;
    }
  }
}

////////////////////////////////////////////////////
// PASSWORD CHECK
////////////////////////////////////////////////////

void checkPassword() {
  if (input == password) {
    openDoor("KEYPAD");
    wrongAttempts = 0;
  } else {
    wrongAttempts++;
    showMsg("WRONG PASSWORD");

    if (wrongAttempts >= 3) {
      showMsg("LOCKED");
      delay(10000);
      wrongAttempts = 0;
    }
  }
}

////////////////////////////////////////////////////
// RFID
////////////////////////////////////////////////////

void handleRFID() {
  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;

  String uid = "";

  for (byte i = 0; i < rfid.uid.size; i++) {
    uid += String(rfid.uid.uidByte[i], HEX);
    uid += " ";
  }

  uid.toUpperCase();

  if (uid == validUID) {
    openDoor("RFID");
  } else {
    showMsg("RFID DENIED");
  }

  rfid.PICC_HaltA();
}

////////////////////////////////////////////////////
// OPEN DOOR
////////////////////////////////////////////////////

void openDoor(String method) {

  if (doorOpen) return;

  doorOpen = true;

  showMsg("ACCESS GRANTED\n" + method);

  doorServo.write(90);
  delay(3000);
  doorServo.write(0);

  showMsg("DOOR CLOSED");

  doorOpen = false;
}

////////////////////////////////////////////////////
// OLED DISPLAY
////////////////////////////////////////////////////

void showMsg(String msg) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.println(msg);
  display.display();
}
