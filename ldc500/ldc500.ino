/*
  Laser Diode Controller Controller
  
  Main Functions:
    * ENABLE/DISABLE Laser -> Pin 13 + LED
    * SET Laser Current    -> Pin 11
    * GET Laser Current    -> A0
  LDC500 -> k=50mA/V
  
  Linear Regression of dataset: Table1_1, using function: A*x+B
  Weighting Method: No weighting
  From x = 1,083000000000000e+01 to x = 1,127000000000000e+02
  B (y-intercept) = 2,158048764451327e+01 +/- 8,999878654085594e-02
  A (slope) = 6,979088703376868e-01 +/- 1,292424325833072e-03
*/

#include <dht.h>
dht DHT;

unsigned long serialNumber = 302432729;
float time;

float A=0.69790887;
float B=21.5804876;

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

void setCurrent(float i) {
  // 5/256 = 0,01953125
  int iDigit = 0;
  analogWrite(laserPin, iDigit);
}

void loop() {
  // get any incoming bytes:
  if (stringComplete) {
//    Serial.println(inputString); 
// Status of 4 LEDs
    if (inputString.startsWith("leds?")) {
      // POWER ON + LASER ON + LASER LOCK + ERROR = 1 + 2 + 4 + 8
      int leds = 1+digitalRead(laserPin)*6;
      Serial.println(leds);
//  Laser ENABLE/DISABLE/STATE
    } else if (inputString.startsWith("l")) {
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
//  Drive Current GET
    } else if (inputString.startsWith("i?")) {
      int CTLOut = analogRead(laserOut);
      // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
      float voltage = CTLOut*(10.0 / 1023.0);
      float LaserCurrent = voltage/(10.0/500.0);
      Serial.println(LaserCurrent);
//  Drive Current SET
    } else if (inputString.startsWith("slc")) {
      char icarray[inputString.substring(3).length() + 1];           //determine size of the array
      inputString.substring(3).toCharArray(icarray, sizeof(icarray)); //put readStringinto an array
      float fi = atof(icarray); //convert the array into a float
      setCurrent(fi);
//  Power GET/SET
    } else if (inputString.startsWith("p")) {
      if ((inputString.substring(1,2) == "?") || (inputString.substring(1,3) == "a?")) {
        int CTLOut = analogRead(laserOut);
        // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
        float voltage = CTLOut*(10.0 / 1023.0);
        float LaserCurrent = voltage/(10.0/500.0);
        Serial.println(LaserCurrent);
      } else {
        char pcarray[inputString.substring(1).length() + 1]; //determine size of the array
        inputString.substring(1).toCharArray(pcarray, sizeof(pcarray)); //put readStringinto an array
        float fp = atof(pcarray); //convert the array into a float
        float fpi;
        if ( fp < 1.0 )  fpi = A*fp*1000.0+B;
        else             fpi = A*fp+B;
        if (fpi > 100.0) fpi = 100.0;
        setCurrent(fpi);
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
// Operating fault GET
    } else if (inputString.startsWith("f?")) {
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


