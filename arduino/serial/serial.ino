// digital pin 2
int pushButton = 2;

String incomingByte = "";

void setup() {
  pinMode(pushButton, INPUT);
  
  Serial.begin(9600);

  Serial.println("Welcome");
}
int i = 0;

void loop() {
  // read the input pin
  int buttonState = digitalRead(pushButton);

  if (buttonState == HIGH) {
    Serial.println("This is data. This is data. This is data. This is data. This is data. This is data. This is data. This is data. This is data. This is data.");
  } else {
    // Serial.write(0);
  }

  if (Serial.available()) {
    // read the incoming byte:
    incomingByte = Serial.readString();
  
    // say what you got:
    Serial.println(incomingByte);
  }
  
  delay(100);
}
