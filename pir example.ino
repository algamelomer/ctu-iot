int PIR_PIN = 27;

void setup() {
  Serial.begin(115200);
  pinMode(PIR_PIN, INPUT);
  Serial.println("PIR Sensor Test Starting...");
  delay(2000);
}

void loop(){
  int pirValue = digitalRead(PIR_PIN);

  if(pirValue == HIGH) {
    Serial.println("Motion detected!");
  } else {
    Serial.println("No motion");
  }

  delay(500);
}
