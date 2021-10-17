#include <SoftwareSerial.h>
#include <TinyGPS.h>

// lat,lon; // create variable for latitude and longitude object
float lat = 0,lon = 0; // create variable for latitude and longitude object 
SoftwareSerial gpsSerial(D1, D2); // rx, tx
TinyGPS gps; // create gps object
#include <TinyGPSPlus.h>
TinyGPSPlus gpsP;

void setup(){
  Serial.begin(9600); // connect serial
  gpsSerial.begin(9600); // connect gps sensor
  Serial.println("Init...");
}
 
void loop() {
  while(gpsSerial.available() > 0) { // check for gps data

  byte gpsData = gpsSerial.read();
  // Serial.write(gpsData);

  if (gpsP.encode(gpsData)) {
    if (gpsP.location.isUpdated()) {
      float latP = gpsP.location.lat();
      float lngP = gpsP.location.lng();
      Serial.println(String(latP, 6) + "," + String(lngP, 6));
    }      
  }
      
  if(gps.encode(gpsSerial.read())) { 
      gps.f_get_position(&lat,&lon); // get latitude and longitude
      // display position
    }
  }
  
  String latitude = String(lat,6);
  String longitude = String(lon,6);
  // Serial.println(latitude + "," + longitude);
  delay(1000);
  
}
