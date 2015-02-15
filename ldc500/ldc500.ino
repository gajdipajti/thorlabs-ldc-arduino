/*
  Laser Diode Controller Controller
  
  Main Functions:
    * ENABLE/DISABLE Laser -> Pin 13 + LED
    * SET Laser Current    -> Pin 11
    * GET Laser Current    -> A0
  LDC500 -> k=50mA/V  
*/

#include <dht.h>
dht DHT;

unsigned long serialNumber = 302432729;
float time;

const int laserPin = 13;
const int laserIn  = 11;
const int laserOut = A0;
const int dht22Pin =  5;

String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete

void setup() {
  pinMode(laserPin, OUTPUT);
  Serial.begin(115200);
  // reserve 200 bytes for the inputString:
  inputString.reserve(200);
}

void loop() {
  // get any incoming bytes:
  if (stringComplete) {
//    Serial.println(inputString); 
//  Laser ENABLE/DISABLE/STATE
    if (inputString.startsWith("l")) {
      if (inputString.substring(1,2) == "0") {
        digitalWrite(laserPin,LOW);
        Serial.println("OK\r");
      } else if (inputString.substring(1,2) == "1") {
        digitalWrite(laserPin,HIGH);
        Serial.println("OK\r");
      } else if (inputString.substring(1,2) == "?") {
        Serial.println(digitalRead(laserPin));
      } else {
        Serial.print("Syntax Error: ");
        Serial.println(inputString);
      }
//  Current GET
    } else if (inputString.startsWith("i?")) {
      int CTLOut = analogRead(laserOut);
      // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
      float voltage = CTLOut*(10.0 / 1023.0);
      float LaserCurrent = voltage/(10.0/500.0);
      Serial.println(LaserCurrent);
//  Power GET/SET
    } else if (inputString.startsWith("p")) {
      if ((inputString.substring(1,2) == "?") || (inputString.substring(1,3) == "a?")) {
        int CTLOut = analogRead(laserOut);
        // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
        float voltage = CTLOut*(10.0 / 1023.0);
        float LaserCurrent = voltage/(10.0/500.0);
        Serial.println(LaserCurrent);
      } else if (inputString.substring(1) == "0") {
        Serial.println(inputString.substring(1));
      } else {
        Serial.print("Syntax Error: ");
        Serial.println(inputString);
      }
//  DHT22 Sensor TEMP/HUM
    } else if (inputString.startsWith("d")) {
        int chk = DHT.read22(dht22Pin);
        switch (chk) {
          case DHTLIB_OK:
            //Serial.print("OK,\t");
            break;
          case DHTLIB_ERROR_CHECKSUM:
            Serial.print("Checksum error,\t");
            break;
          case DHTLIB_ERROR_TIMEOUT:
            Serial.print("Time out error,\t");
            break;
          default:
            Serial.print("Unknown error,\t");
            break;
        }
        if (inputString.substring(1,3) == "t?") {
          Serial.println(DHT.temperature, 1);
        } else if (inputString.substring(1,3) == "h?") {
          Serial.println(DHT.humidity, 1);
        } else if (inputString.substring(1,2) == "?") {
          Serial.print(DHT.humidity, 1);
          Serial.print("; ");
          Serial.println(DHT.temperature, 1);
        } else {
          Serial.print("Syntax Error: ");
          Serial.println(inputString);
        }
// Serial Number
    } else if (inputString.startsWith("sn?")) {
        Serial.println(serialNumber);
// Working seconds
    } else if (inputString.startsWith("hrs?")) {
        time = millis()/60000.0;
        Serial.println(time);
// Interlock State
    } else if (inputString.startsWith("ilk?")) {
        Serial.println("0");
    } else {
        Serial.print("Syntax Error: ");
        Serial.println(inputString);
    }
    // clear the string:
    inputString = "";
    stringComplete = false;
  }
}
/*
  SerialEvent occurs whenever a new data comes in the
 hardware serial RX.  This routine is run between each
 time loop() runs, so using delay inside loop can delay
 response.  Multiple bytes of data may be available.
 */
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read(); 
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a carriage return (ASCII 13),
    //  set a flag so the main loop can do something about it:
    if (inChar == '\r') {
      stringComplete = true;
    } 
  }
}


