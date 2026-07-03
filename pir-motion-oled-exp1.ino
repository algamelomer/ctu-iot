/////////////////////////////////////////////
// Smart Home Security System - ESP32
// WiFi Point + PIR + Latched Alarm + STOP Button + OLED (FIXED)
/////////////////////////////////////////////

#define REMOTEXY_MODE__WIFI_POINT

#include <WiFi.h>
#include <RemoteXY.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/////////////////////////////////////////////
// WiFi config
/////////////////////////////////////////////

#define REMOTEXY_WIFI_SSID "omar"
#define REMOTEXY_WIFI_PASSWORD "omar5040"
#define REMOTEXY_SERVER_PORT 6377

/////////////////////////////////////////////
// OLED config
/////////////////////////////////////////////

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/////////////////////////////////////////////
// RemoteXY GUI
/////////////////////////////////////////////

#pragma pack(push, 1)
uint8_t RemoteXY_CONF[] = {
  255,1,0,1,0,37,0,19,0,0,0,0,31,1,106,200,1,1,2,0,
  1,29,93,46,46,0,1,31,0,74,39,71,26,12,13,2,30,37,64,83,
  116,111,112,0
};

struct {
  uint8_t button_01;
  uint8_t connect_flag;
} RemoteXY;

#pragma pack(pop)

/////////////////////////////////////////////
// Pins
/////////////////////////////////////////////

#define PIR_PIN 27
#define BUZZER_PIN 26
#define LED_PIN 2

bool alarmActive = false;

/////////////////////////////////////////////
// TIMERS (NON-BLOCKING)
/////////////////////////////////////////////

unsigned long lastAlarmToggle = 0;
bool alarmState = false;

unsigned long lastOledUpdate = 0;

/////////////////////////////////////////////

void setup() {
  RemoteXY_Init();

  pinMode(PIR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_PIN, LOW);

  /////////////////////////////////////////////
  // OLED INIT
  /////////////////////////////////////////////
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    while (true);
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 20);
  display.println("System");
  display.println("Ready");
  display.display();
  delay(1500);
}

/////////////////////////////////////////////

void loop() {
  RemoteXY_Handler();

  //////////////////////////////////////
  // PIR triggers alarm (latched)
  //////////////////////////////////////
  if (digitalRead(PIR_PIN) == HIGH) {
    alarmActive = true;
  }

  //////////////////////////////////////
  // STOP button
  //////////////////////////////////////
  if (RemoteXY.button_01 == 1) {
    alarmActive = false;
    RemoteXY.button_01 = 0;
  }

  //////////////////////////////////////
  // ALARM logic (NON BLOCKING)
  //////////////////////////////////////
  if (alarmActive) {
    runAlarm();
  } else {
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
  }

  //////////////////////////////////////
  // OLED update (smooth)
  //////////////////////////////////////
  updateOLED();
}

/////////////////////////////////////////////
// NON-BLOCKING ALARM
/////////////////////////////////////////////

void runAlarm() {
  if (millis() - lastAlarmToggle > 200) {
    lastAlarmToggle = millis();
    alarmState = !alarmState;

    digitalWrite(BUZZER_PIN, alarmState);
    digitalWrite(LED_PIN, alarmState);
  }
}

/////////////////////////////////////////////
// OLED UPDATE (optimized)
/////////////////////////////////////////////

void updateOLED() {
  if (millis() - lastOledUpdate < 300) return;
  lastOledUpdate = millis();

  display.clearDisplay();

  if (alarmActive) {
    display.setTextSize(2);
    display.setCursor(0, 10);
    display.println("MOTION!");

    display.setTextSize(1);
    display.setCursor(0, 40);
    display.println("ALERT ACTIVE");
  } else {
    display.setTextSize(2);
    display.setCursor(20, 20);
    display.println("SAFE");
  }

  display.display();
}
