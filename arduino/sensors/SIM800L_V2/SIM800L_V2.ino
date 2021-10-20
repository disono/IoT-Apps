#include <SoftwareSerial.h>

SoftwareSerial mySerial(D5, D6); // RX, TX 

bool isSent = false;

void setup()  
{
  // Open serial communication
  Serial.begin(9600);

  // set the data rate for the SoftwareSerial port
  mySerial.begin(9600);

  Serial.println("Initializing..."); 
  delay(1000);

  Serial.println("Testing SIM800L module");
  Serial.println();
  Serial.print("Sizeof(mySerial) = "); 
  Serial.println(sizeof(mySerial));
  Serial.println();
}

void loop() {
  if( mySerial.available() ) {
    char c = mySerial.read(); 
    Serial.print(c);
  }
  
  if(Serial.available()) {
    String Arsp = Serial.readString();

    Serial.println("Serial available");
    Serial.println(Arsp);
    mySerial.println(Arsp);
    Serial.println("Serial available end");
  }

  if (isSent == false) {
    isSent = true;

    mySerial.println("AT"); //Once the handshake test is successful, it will back to OK
    updateSerial();

    mySerial.println("AT+CMGF=1"); // Configuring TEXT mode
    updateSerial();
    mySerial.println("AT+CMGS=\"+630000000000\"");//change ZZ with country code and xxxxxxxxxxx with phone number to sms
    updateSerial();
    mySerial.print("sample text"); //text content
    updateSerial();
    mySerial.write(26);
  }
}

void updateSerial() {
  delay(500);
  
  while (Serial.available()) {
    mySerial.write(Serial.read());// Forward what Serial received to Software Serial Port
  }
  
  while(mySerial.available()) {
    Serial.write(mySerial.read()); // Forward what Software Serial received to Serial Port
  }
}
