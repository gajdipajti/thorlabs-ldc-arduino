/*
  A Controller for the Thorlabs LDC205C/LDC500 series equipment.
  LDC205C: https://www.thorlabs.com/thorproduct.cfm?partnumber=LDC205C
  LDC500:  https://www.thorlabs.com/thorproduct.cfm?partnumber=LDC500
  
  Main Functions:
    * ENABLE/DISABLE Laser  -> Pin 13 + LED
    * SET Laser Current     -> Pin 11 (Changed to 31 kHz)
    * GET Laser1 Current    -> A0  -> i?
    * GET Laser2 Current    -> A1  -> j?
  LDC205C -> k=50mA/V
  
  Commands Cobolt Gen5 style + minor changes for the second laser and the DHT22 sensor.
  
  Power fit: P=AI+b
  B1 (y-intercept) = 2,158048764451327e+01 +/- 8,999878654085594e-02
  A1 (slope) = 6,979088703376868e-01 +/- 1,292424325833072e-03
  
  Undocumented commands:
  rtec1drv?
  rtec1t?
  ra?
  glm?
  mev?
  gfv?
  gfd?
  gfbd?
  gcn?
  
  Non-cobolt commands for the second laser (+1):
  j?  -> i?
  q?  -> p?
  
  Note if you need a faster response time, comment out the unwanted code.
*/
// Please clone DHTlib from: https://github.com/RobTillaart/Arduino.git
// Copy DHTlib to ~/sketchbook/libraries.
// If you need help with this, please contact me!
#include <dht.h>    //DHT
dht DHT;            //DHT
// NOTE: If you don't want to use a DHT sensor, just comment out the DHT parts

unsigned int serialNumber = 855; // Just 405+450. Just don't think about it.
float versionNumber = 2.0;       // Merge DHT code, and the second laser.
float time;

// 405nm laser in the LDC205
float A=0.69790887;
float B=21.5804876;

// 450nm laser in the LDC500
float C=0.69790887;
float D=21.5804876;

// They both use the same modulation coefficient, but it is better to keep them separated.
float kLDC205 = 50.0; // mA/V
float kLDC500 = 50.0; // mA/V

const int laserREM = 13;
const int laserMod = 11;
const int laser1CTL = A0;    // 405nm laser in the LDC205
const int laser2CTL = A1;    // 450nm laser in the LDC500
const int dht22Pin =  8;

boolean interlock = false;   // for future interlock feature

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

float getVoltage1() {
  int ctlOut = analogRead(laser1CTL);
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  return ctlOut*0.005; // voltage
}

float getVoltage2() {
  int ctlOut = analogRead(laser2CTL);
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  return ctlOut*0.005; // voltage
}

float getCurrent1() {
  return getVoltage1()*kLDC205;
}

float getCurrent2() {
  return getVoltage2()*kLDC500;
}

float getPower1() {
  float p = (getCurrent1()-B)/A;
  if ( p > 0 ) return p;
  else return 0.0;
}

float getPower2() {
  float p = (getCurrent1()-D)/C;
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
//  Drive Current GET1
    } else if (inputString.startsWith("i?")) {
      Serial.println(getCurrent1());
//  Drive Current GET2
    } else if (inputString.startsWith("j?")) {
      Serial.println(getCurrent2());
//  Drive Current SET
    } else if (inputString.startsWith("slc")) {
      setCurrent(toFloat(inputString.substring(3)));
//  Power GET/SET
    } else if (inputString.startsWith("p")) {
      if ((inputString.substring(1,2) == "?") || (inputString.substring(1,3) == "a?")) {
        Serial.println(getPower1());
      } else {
        float fp = toFloat(inputString.substring(1));
        float fpi;
        if ( fp < 1.0 )  fpi = A*fp*1000.0+B;
        else             fpi = A*fp+B;
        if (fpi > 100.0) fpi = 100.0;
        setCurrent(fpi);
      }
//  Power GET2
    } else if (inputString.startsWith("q")) {
      if ((inputString.substring(1,2) == "?") || (inputString.substring(1,3) == "a?")) {
        Serial.println(getPower1());
      }
// DHT22 Sensor TEMP/HUM
    } else if (inputString.startsWith("d")) {            //DHT
      int chk = DHT.read22(dht22Pin);                    //DHT
      if (inputString.substring(1,3) == "t?") {          //DHT
        Serial.println(DHT.temperature, 1);              //DHT
      } else if (inputString.substring(1,3) == "h?") {   //DHT
        Serial.println(DHT.humidity, 1);                 //DHT
      } else if (inputString.substring(1,2) == "?")  {   //DHT
        if (chk == DHTLIB_OK) Serial.println("1");       //DHT
        else Serial.println("0");                        //DHT
      } else {                                           //DHT
        Serial.print("Syntax Error: ");                  //DHT
        Serial.println(inputString);                     //DHT
      }                                                  //DHT
// Serial Number
    } else if (( inputString.startsWith("sn?") ) || (inputString.substring(3,6) == "sn?") ) {
      Serial.println(serialNumber);
// Version Number
    } else if (inputString.startsWith("ver?")) {
      Serial.println(versionNumber);
// Working seconds
    } else if (inputString.startsWith("hrs?")) {
      time = millis()/60000.0;
      Serial.println(time);
// Interlock State, no real code behind
    } else if (inputString.startsWith("ilk?")) {
      (interlock) ? Serial.println("1") : Serial.println("0");
// Operating fault GET, no hardware implementation behind
    } else if (inputString.startsWith("f?")) {
      Serial.println("0");
// Are you there? Returns Ok (undocumented Cobolt command)
    } else if (inputString.startsWith("?")) {
      Serial.println("OK");
// Cobolt controller commands (some are undocumented)
    } else if (inputString.startsWith("@cob")) {
      if (inputString.substring(4,5) == "0") {
        digitalWrite(laserREM,LOW);
        Serial.println("OK\r");
      } else if (inputString.substring(4,5) == "1") {
        // Laser ON after interlock.
        digitalWrite(laserREM,HIGH);
        Serial.println("OK\r");
      } else if (inputString.substring(4,7) == "as?") {
        // Get autostart enable state.
        Serial.println("0");
      } else if (inputString.substring(4,9) == "asdr?") {
        // Get direct input enable state.
        Serial.println("0");
      } else if (inputString.substring(4,9) == "asks?") {
        // Get key switch state.
        Serial.println("1");
      } else if (inputString.substring(4,9) == "asky?") {
        // Get key switch enable state.
        Serial.println("1");
      } else {
        Serial.print("Syntax Error: ");
        Serial.println(inputString);
      }    
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
