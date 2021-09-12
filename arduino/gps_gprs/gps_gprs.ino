#include <SoftwareSerial.h>

// Parse state enumeration
enum _parseState {
  PS_DETECT_MSG_TYPE,

  PS_IGNORING_COMMAND_ECHO,

  PS_HTTPACTION_TYPE,
  PS_HTTPACTION_RESULT,
  PS_HTTPACTION_LENGTH,

  PS_HTTPREAD_LENGTH,
  PS_HTTPREAD_CONTENT
};

// constants
// secs between scans for track request changes
const unsigned int trackRequestScanTimeout = 15UL;

const bool _enableDebug = false;

// global variables
SoftwareSerial ssGprs(7, 8);
SoftwareSerial ssGps(7, 8);
byte parseState = PS_DETECT_MSG_TYPE;
char buffer[120];
byte pos = 0;
String gpsData = "";
String latitude = "";
String longitude = "";
String extra = "";
int contentLength = 0;

// forward declarations
void getGps();
String sendGpsData(String command, const int timeout);
void resetBuffer();
void sendGprs(const char* msg, int waitMs);
void checkTrackRequest();
void parseATText(byte b);

String _requestAPI = "http://trackstr.losito.com.ph/firebaseTrackRequest.php";
String _sendAPI = "http://trackstr.losito.com.ph/firebaseLocationUpdate.php";

void setup() {

  // the Serial port of Arduino baud rate.
  Serial.begin(9600);
  
  Serial.print(F("Trackstr. "));
  Serial.print(F(" Compiled: "));
  Serial.print(F(__DATE__));
  Serial.print(F(" "));
  Serial.print(F(__TIME__));
  Serial.print(F("."));
  Serial.println();

  // GPRS baud rate
  ssGprs.begin(9600);

  // GPS baud rate
  ssGps.begin(9600);
  ssGps.listen();

}

void loop() {
  
  unsigned long timeout = 0UL;

  getGps();

  // ever <timeout> secs,  check for track request changes
  if (millis() > timeout) {
    ssGprs.listen();
    checkTrackRequest();    
    ssGps.listen();

    // determine next time to check for SMS
    timeout = millis() + trackRequestScanTimeout * 1000UL;
  }

}

void checkTrackRequest() {

  sendGprs("AT+SAPBR=3,1,\"APN\",\"internet.globe.com.ph\"", 0);  
  sendGprs("AT+SAPBR=1,1",3000);
  sendGprs("AT+HTTPINIT", 0);  
  sendGprs("AT+HTTPPARA=\"CID\",1", 0);
  sendGprs("AT+HTTPPARA=\"URL\",\"" + _requestAPI + "?tracker_imei=865067023403739\"", 0);
  sendGprs("AT+HTTPACTION=0", 0);

  while(ssGprs.available()) {
    parseATText(ssGprs.read());
  }
  
}

void getGps() {
   
   sendGpsData("AT+CGNSPWR=1", 1000); 
   gpsData = sendGpsData("AT+CGNSINF", 1000);

}

String sendGpsData(String command, const int timeout) {
  
    String response = "";    
    ssGps.println(command); 
    long int time = millis();   
    while((time+timeout) > millis())
    {
      while(ssGps.available())
      {       
        char c = ssGps.read();
        // uncomment for debugging
        Serial.print(c);
        response += c;
      }  
    }    
    
    Serial.println(response);
    
    return response;
    
}

void resetBuffer() {
  
  memset(buffer, 0, sizeof(buffer));
  pos = 0;
  
}

void sendGprs(const char* msg, int waitMs) {
  ssGprs.println(msg);

  waitMs = (waitMs != 0) ? waitMs : 500;
  delay(waitMs);
  while(ssGprs.available()) {
    parseATText(ssGprs.read());
  }
}

void parseATText(byte b) {

  buffer[pos++] = b;

  if (pos >= sizeof(buffer)) {
    // just to be safe    
    resetBuffer();
  }

  // Detailed debugging
  if (_enableDebug == true) {
     Serial.println();
     Serial.print("state = ");
     Serial.print("b = ");
     Serial.println(b);
     Serial.print("pos = ");
     Serial.println(pos);
     Serial.print("buffer = ");
     Serial.println(buffer);
  }

  switch (parseState) 
  {
    case PS_DETECT_MSG_TYPE: 
    {
        if (b == '\n' ) {
          resetBuffer();
        } else {        
          if (pos == 3 && strcmp(buffer, "AT+") == 0 ) {
            parseState = PS_IGNORING_COMMAND_ECHO;
          } else if (b == ':' ) {
            // Serial.print("Checking message type: ");
            // Serial.println(buffer);
            
            if (strcmp(buffer, "+HTTPACTION:") == 0 ) {
              Serial.println("Received HTTPACTION");
              parseState = PS_HTTPACTION_TYPE;
            } else if (strcmp(buffer, "+HTTPREAD:") == 0 ) {
              Serial.println("Received HTTPREAD");            
              parseState = PS_HTTPREAD_LENGTH;
            }
            
            resetBuffer();
          }
        }
    }
    break;
  
    case PS_IGNORING_COMMAND_ECHO:
    {
        if (b == '\n' ) {
          Serial.print("Ignoring echo: ");
          Serial.println(buffer);
          parseState = PS_DETECT_MSG_TYPE;
          resetBuffer();
        }
    }
    break;
  
    case PS_HTTPACTION_TYPE:
    {
        if (b == ',' ) {
          Serial.print("HTTPACTION type is ");
          Serial.println(buffer);
          parseState = PS_HTTPACTION_RESULT;
          resetBuffer();
        }
    }
    break;
  
    case PS_HTTPACTION_RESULT:
    {
        if (b == ',' ) {
          Serial.print("HTTPACTION result is ");
          Serial.println(buffer);
          parseState = PS_HTTPACTION_LENGTH;
          resetBuffer();
        }
    }
    break;
  
    case PS_HTTPACTION_LENGTH:
    {
        if (b == '\n') {
          Serial.print("HTTPACTION length is ");
          Serial.println(buffer);
          
          // now request content
          ssGprs.print("AT+HTTPREAD=0,");
          ssGprs.println(buffer);
          
          parseState = PS_DETECT_MSG_TYPE;
          resetBuffer();
        }
    }
    break;
  
    case PS_HTTPREAD_LENGTH:
    {
        if (b == '\n' ) {
          contentLength = atoi(buffer);
          Serial.print("HTTPREAD length is ");
          Serial.println(contentLength);
          
          Serial.print("HTTPREAD content: ");
          
          parseState = PS_HTTPREAD_CONTENT;
          resetBuffer();
        }
    }
    break;
  
    case PS_HTTPREAD_CONTENT:
    {
        // for this demo I'm just showing the content bytes in the serial monitor
        Serial.write(b);
        
        contentLength--;
        
        if (contentLength <= 0 ) {
          // all content bytes have now been read
  
          // Serial.println("\n\n");
          // Serial.print(gpsData);        
          
          if (String(buffer).equals("YES")) {
  
            gpsData.trim();
            gpsData.remove(0,46);
            int commaIndex = gpsData.indexOf(',');
            int secondCommaIndex = gpsData.indexOf(',', commaIndex+1);
            latitude = gpsData.substring(0, commaIndex);
            longitude = gpsData.substring(commaIndex+1, secondCommaIndex);
            extra = gpsData.substring(secondCommaIndex+1); //To the end of the string
            
            Serial.println("\n\n");
            Serial.print("Lat: ");
            Serial.println(latitude);
            Serial.print("Lng: ");
            Serial.println(longitude);
            Serial.print("Excess: ");
            Serial.println(extra);          
          
            Serial.println("\n");
            Serial.println("Request is YES, send coordinates.");
            ssGprs.println("AT+HTTPTERM");
            ssGprs.println("AT+SAPBR=3,1,\"APN\",\"internet.globe.com.ph\"");
            ssGprs.println("AT+SAPBR=1,1");
            delay(3000);
            
            ssGprs.println("AT+HTTPINIT");
            ssGprs.println("AT+HTTPPARA=\"CID\",1");
            ssGprs.println("AT+HTTPPARA=\"URL\",\"" + _sendAPI + "?tracker_imei=865067023403739&lat="+ latitude +"&lng=" + longitude + "122.0392434\"");
            ssGprs.println("AT+HTTPACTION=0");
            ssGprs.println("AT+HTTPACTION=0");
            ssGprs.println("AT+HTTPACTION=0");
            ssGprs.println("AT+HTTPREAD");
            ssGprs.println("AT+HTTPTERM");
            gpsData = "";
            latitude = "";
            longitude = "";
            extra = "";
            
          } else {
            Serial.println("\n");
            Serial.println("Request is NO, back to listening.");
          }
          
          parseState = PS_DETECT_MSG_TYPE;
          resetBuffer();
  
          Serial.println("\n\n\n");
  
          ssGprs.println("AT+HTTPTERM");
        }
      }
      break;
  }
}
