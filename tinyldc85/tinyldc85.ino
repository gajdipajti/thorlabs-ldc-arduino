/*
  A Controller for the Thorlabs LDC205C/LDC500 series equipment using an ATTiny85.
  LDC205C: https://www.thorlabs.com/thorproduct.cfm?partnumber=LDC205C
  LDC500:  https://www.thorlabs.com/thorproduct.cfm?partnumber=LDC500
  ATTiny85 source for Arduino IDE integration: https://github.com/SpenceKonde/ATTinyCore
  
  Main Functions:
    * ENABLE/DISABLE Laser  -> PIN_B2 + LED
    * SET Laser Current     -> PIN_B4 (Change to 32 MHz, RC filtered)
    * GET Laser1 Current    -> A3  -> i?
    * GET AVR Internal Temperature -> t?
  LDC205C -> k=50mA/V
  
  Commands Cobolt Gen5 style + minor changes.
  
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
  Note if you need a faster response time, comment out the unwanted code.
*/

unsigned int serialNumber = 490; // Just 405nm+attiny85. Just don't think about it.
float versionNumber = 3.0;       // ATtiny85 version
float time;

// Temperature calibration
const float At=2.5;
const float Bt=-120.0;

// 405nm laser in the LDC205
const float A=0.69790887;
const float B=21.5804876;

// They both use the same modulation coefficient, but it is better to keep them separated.
const float kLDC205 = 50.0;     // mA/V
const float iConst = 1.535;     // for single laser driver

const int laserREM = PIN_B2;
const int laserMod = PIN_B4;
const int laser1CTL = A3;       // 405nm laser in the LDC205

boolean interlock = false;      // for future interlock feature

String inputString = "";        // a string to hold incoming data
boolean stringComplete = false; // whether the string is complete

void setup() {
  pinMode(laserREM, OUTPUT);    // Set laser ON/OFF to output
  pinMode(laserMod, OUTPUT);    // Set the PWM pin to output
  pinMode(laser1CTL, INPUT);    // Set ADC pin to input

  digitalWrite(laserREM, HIGH); // turn the LED on (HIGH is the voltage level)
  
  // Serial.begin(115200);      // Works with 16 MHz (PPL) clock source. It is noisy.
  Serial.begin(9600);           // Recommended for 8 MHz internal oscillator.
  inputString.reserve(128);     // reserve 128 bytes for the inputString, we use small commands.
  
  digitalWrite(laserREM, LOW);  // turn the LED off by making the voltage LOW
}

void setCurrent(float i) {
  // Lock to Max.
  if (i > 100.00) { i = 110.00; }
  int iDigit = i*iConst;
  analogWrite(laserMod, iDigit);
  Serial.println("OK\r");
}

float getVoltage() {
  int ctlOut = analogRead(laser1CTL);     // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  return ctlOut*0.005; // voltage
}

float getCurrent() {
  return getVoltage()*kLDC205;
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
  if (stringComplete) {                         // get any incoming bytes:
  // Status of 4 LEDs
    if (inputString.startsWith("leds?")) {      // POWER ON + LASER ON + LASER LOCK + ERROR = 1 + 2 + 4 + 8
      int leds = 1+digitalRead(laserREM)*6;
      Serial.println(leds);
    } else if (inputString.startsWith("l")) {   //  Laser ENABLE/DISABLE/STATE
      switch (inputString.charAt(1)) {
        case '0':
          digitalWrite(laserREM,LOW);
          Serial.println("OK\r"); break;
        case '1':
          digitalWrite(laserREM,HIGH);
          Serial.println("OK\r"); break;
        case '?':
          Serial.println(digitalRead(laserREM)); break;
        default:
          digitalWrite(laserREM,!digitalRead(laserREM));
          Serial.println("OK\r"); break;
      }
    } else if (inputString.startsWith("i?")) {  //  Drive Current GET
      Serial.println(getCurrent());
    } else if (inputString.startsWith("slc")) { //  Drive Current SET
      setCurrent(toFloat(inputString.substring(3)));
    } else if (inputString.startsWith("p")) {   //  Power GET/SET
      if ((inputString.substring(1,2) == "?") || (inputString.substring(1,3) == "a?")) {
        Serial.println(getPower());
      } else {
        float fp = toFloat(inputString.substring(1));
        float fpi;
        if ( fp < 1.0 )  fpi = 0; // If power set to zero, set the current to zero.
        else             fpi = A*fp+B;
        if (fpi > 100.0) fpi = 100.0;
        setCurrent(fpi);
      }
    } else if (( inputString.startsWith("sn?") ) || (inputString.substring(3,6) == "sn?") ) {   // Serial Number
      Serial.println(serialNumber);
    } else if (inputString.startsWith("ver?")) {  // Version Number
      Serial.println(versionNumber);
    } else if (inputString.startsWith("hrs?")) {  // Working minutes
      time = millis()/60000.0;
      Serial.println(time);
    } else if (inputString.startsWith("ilk?")) {  // Interlock State, no real code behind
      (interlock) ? Serial.println("1") : Serial.println("0");
    } else if (inputString.startsWith("f?")) {    // Operating fault GET, no hardware implementation behind
      Serial.println("0");
    } else if (inputString.startsWith("?"))  {    // Are you there? Returns Ok (undocumented Cobolt command)
      Serial.println("OK");
    } else if (inputString.startsWith("t?")) {    // Just print the internal temperature's ADC value, and a crude calibration.
      Serial.println(analogRead(ADC_TEMPERATURE)*At+Bt);  
    } else {
      Serial.print("Syntax Error: ");
      Serial.println(inputString);
    }
    // clear the string:
    inputString = "";
    stringComplete = false;
  }
  serialEvent();
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
