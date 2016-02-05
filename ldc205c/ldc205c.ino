/*
  A Controller for the Thorlabs LDC205C series equipment.
  
  Main Functions:
    * ENABLE/DISABLE Laser -> Pin 13 + LED
    * SET Laser Current    -> Pin 11 (Changed to 31 kHz)
    * GET Laser Current    -> A0
  LDC205C -> k=50mA/V
  
  Commands from Cobolt Gen5.
  
  Power fit: P=AI+b
  B (y-intercept) = 2,158048764451327e+01 +/- 8,999878654085594e-02
  A (slope) = 6,979088703376868e-01 +/- 1,292424325833072e-03
*/

// Please clone DHTlib from: https://github.com/RobTillaart/Arduino.git
// Copy DHTlib to ~/sketchbook/libraries.
#include <dht.h>
dht DHT;

unsigned long serialNumber = 302432729;
float time;

float A=0.69790887;
float B=21.5804876;

float kLDC500 = 50.0; // mA/V

const int laserREM = 13;
const int laserMod = 11;
const int laserCTL = A0;
const int dht22Pin =  8;

String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete

void setup() {
  pinMode(laserREM, OUTPUT);
  pinMode(laserMod, OUTPUT);
  // Change PWM Frequency for Timer2.  f=~31kHZ
  // Copied the relevant part from here: http://playground.arduino.cc/Code/PwmFrequency
  TCCR2B = TCCR2B & 0b11111000 | 0x01;
  Serial.begin(115200);
  // reserve 200 bytes for the inputString:
  inputString.reserve(200);
}

void setCurrent(float i) {
  // Lock to Max.
  if (i > 100.00) { i = 110.00; }
  int iDigit = i*1.535;
  analogWrite(laserMod, iDigit);
  Serial.println("OK\r");
}

float getVoltage() {
  int ctlOut = analogRead(laserCTL);
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  return ctlOut*0.005; // voltage
}

float getCurrent() {
  return getVoltage()*kLDC500;
}

float getPower() {
  float p = (getCurrent()-B)/A;
  if ( p > 0 ) return p;
  else return 0.0;
}

float toFloat(String s) {
  char carray[s.length() + 1];            //determine size of the array
  s.toCharArray(carray, sizeof(carray));  //put readStringinto an array
  float floatNumber = atof(carray);       //convert the array into a float
  return floatNumber;
}

void loop() {
  // get any incoming bytes:
  if (stringComplete) {
  // Status of 4 LEDs
    if (inputString.startsWith("leds?")) {
      // POWER ON + LASER ON + LASER LOCK + ERROR = 1 + 2 + 4 + 8
      int leds = 1+digitalRead(laserREM)*6;
      Serial.println(leds);
  //  Laser ENABLE/DISABLE/STATE
    } else if (inputString.startsWith("l")) {
      if (inputString.substring(1,2) == "0") {
        digitalWrite(laserREM,LOW);
        Serial.println("OK\r");
      } else if (inputString.substring(1,2) == "1") {
        digitalWrite(laserREM,HIGH);
        Serial.println("OK\r");
      } else if (inputString.substring(1,2) == "?") {
        Serial.println(digitalRead(laserREM));
      }
    //  Drive Current GET
    } else if (inputString.startsWith("i?")) {
      Serial.println(getCurrent());
    //  Drive Current SET
    } else if (inputString.startsWith("slc")) {
      setCurrent(toFloat(inputString.substring(3)));
    //  Power GET/SET
    } else if (inputString.startsWith("p")) {
      if ((inputString.substring(1,2) == "?") || (inputString.substring(1,3) == "a?")) {
        Serial.println(getPower());
      } else {
        float fp = toFloat(inputString.substring(1));
        float fpi;
        if ( fp < 1.0 )  fpi = A*fp*1000.0+B;
        else             fpi = A*fp+B;
        if (fpi > 100.0) fpi = 100.0;
        setCurrent(fpi);
      }
    // DHT22 Sensor TEMP/HUM
    } else if (inputString.startsWith("d")) {
      int chk = DHT.read22(dht22Pin);
      if (inputString.substring(1,3) == "t?") {
        Serial.println(DHT.temperature, 1);
      } else if (inputString.substring(1,3) == "h?") {
        Serial.println(DHT.humidity, 1);
      } else if (inputString.substring(1,2) == "?")  {
        if (chk == DHTLIB_OK) Serial.println("1");
        else Serial.println("0");
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
// Interlock State, no real code behind
    } else if (inputString.startsWith("ilk?")) {
      Serial.println("0");
// Operating fault GET, no real code behind
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
 hardware serial RX. This routine is run between each
 time loop() runs, so using delay inside loop can delay
 response. Multiple bytes of data may be available.
 */
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read(); 
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a carriage return (ASCII 13),
    // set a flag so the main loop can do something about it:
    if (inChar == '\r') {
      stringComplete = true;
    } 
  }
}
