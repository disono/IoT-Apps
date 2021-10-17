#define SHOCK_PIN 2

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);       // on-board LED, usually pin 13
  pinMode(SHOCK_PIN, INPUT);          // shock sensor pin set to input
}

void loop() {
  if (digitalRead(SHOCK_PIN)) {       // shock detected?
    // shock detected with pull-down resistor
    digitalWrite(LED_BUILTIN, HIGH);  // switch LED on
    delay(2000);                      // leave LED on for period
  }
  else {
    // shock not detected with pull-down resistor
    digitalWrite(LED_BUILTIN, LOW);   // switch LED off
  }
}
