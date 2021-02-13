/*
  A Controller for the Thorlabs LDC205C/LDC500 series equipment.
  This version needs a custom electronics after pin10 (see github readme, or sim link), and also an opto-relay for the solenoid.
  LDC205C: https://www.thorlabs.com/thorproduct.cfm?partnumber=LDC205C
  LDC500:  https://www.thorlabs.com/thorproduct.cfm?partnumber=LDC500
  Code under GPLv2 license. Author: gajdost
  
  Main Functions:
    * Laser Shutter         -> Pin 13 + !LED
    * SET Laser Current     -> Pin 9 (Changed to 31 kHz)
    * GET Laser1 Current    -> A5  -> i?
  
  LDC205C, LDC500 -> k=50mA/V (conversion factor when reading current)
  
  Commands Cobolt Gen5 style+ 
*/
// Variables and constants fot the GUI
unsigned int serialNumber = 637; // Just a check in the GUI.
float versionNumber = 2.1;       // Code for different laser with shutter.

// Modulation coeff. from datasheet.
float kLDC = 50.0; // mA/V

// Inverse Current fit: PWM = iA0*I^2 + iB0*I + iC0
const float iC0 = -3.63883;
const float iB0 =  0.83821;
const float iA0 = -0.00063;
const int pwmLimit = 170;   // Hard limit, just in case.

// 637nm laser in the LDC500: P = pA1*I*I + pB1*I + pC1
const float pA1 = -0.00082;
const float pB1 =  0.97591;
const float pC1 = -60.3816;

// Define used pins.
const int laserSH = 13;
const int laserMod = 9;
const int laser1CTL = A5;    // 637nm laser in the LDC500

// For serial communication.
String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete

// The initial setup, that will run every time when we connect to the Arduino via serial.
void setup() {
  Serial.begin(115200);
  pinMode(laserSH, OUTPUT);
  pinMode(laserMod, OUTPUT);
  pinMode(laser1CTL, INPUT);
//  while (!Serial) {
//    ; // wait for serial port to connect. Needed for native USB
//  }
  // Change PWM Frequency for Timer2.  f=~31kHZ
  // Copied the relevant part from here: http://playground.arduino.cc/Code/PwmFrequency
  TCCR1B = TCCR1B & 0b11111000 | 0x01;
  inputString.reserve(200);   // reserve 200 bytes for the inputString:
  digitalWrite(laserSH, HIGH);
}

void setCurrent(float i) {
  int iDigit = iA0*i*i + iB0*i + iC0;
  analogWrite(laserMod, constrain(iDigit, 0, 170));
  Serial.println("OK\r");
}

float getVoltage() {
  int ctlOut = analogRead(laser1CTL);
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  return ctlOut*0.005; // voltage
}

float getCurrent() {
  return getVoltage()*kLDC;
}

float getPower() {
  float c = getCurrent();
  float p = pA1*c*c + pB1*c + pC1;
  if ( p > 0 ) return p; else return 0.0;
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
      Serial.println(1+digitalRead(laserSH)*6);
    // Laser ENABLE/DISABLE/STATE (shutter actually)
    } else if (inputString.startsWith("l")) {
      if (inputString.substring(1,2) == "1") {
        digitalWrite(laserSH,LOW);
        Serial.println("Laser ON\r");
      } else if (inputString.substring(1,2) == "0") {
        digitalWrite(laserSH,HIGH);
        Serial.println("Laser OFF\r");
      } else if (inputString.substring(1,2) == "?") {
        Serial.println(!digitalRead(laserSH));
      }
    // Drive Current GET
    } else if (inputString.startsWith("i?")) {
      Serial.println(getCurrent());
    // Drive Current SET
    } else if (inputString.startsWith("slc")) {
      setCurrent(toFloat(inputString.substring(3)));
    } else if (inputString.startsWith("d")) {
      if (inputString.substring(1,2) == "?")
      Serial.println(analogRead(laserMod));
      else
      analogWrite(laserMod, toFloat(inputString.substring(1)));
    // Power GET/SET
    } else if (inputString.startsWith("p")) {
      if ((inputString.substring(1,2) == "?") || (inputString.substring(1,3) == "a?")) {
        Serial.println(getPower());
      } 
    // Serial Number
    } else if (( inputString.startsWith("sn?") ) || (inputString.substring(3,6) == "sn?") ) {
      Serial.println(serialNumber);
    // Version Number
    } else if (inputString.startsWith("ver?")) {
      Serial.println(versionNumber);
    // Working seconds
    } else if (inputString.startsWith("hrs?")) {
      long timeNow = millis();
      int hrs = timeNow/3600000;
      float mnt = (timeNow % 3600000) / 60000.0;
      Serial.print(hrs);
      Serial.print(":");
      if (mnt<10) Serial.print("0"); // leading zero for minutes
      Serial.println(mnt);
    // Are you there? Returns Ok (undocumented Cobolt command)
    } else if (inputString.startsWith("?")) {
      Serial.println("OK");
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
