/*
  An artisan DAC. Based on the LDC500sh code
  This version needs a custom electronics after pin9 (see github readme, or sim link), and also an opto-relay for the solenoid.
  Code under GPLv2 license. Author: gajdost
  
  Main Functions:
    * Laser Shutter         -> Pin 13 + !LED
    * SET Laser Current     -> Pin 9 (Changed to 31 kHz)
    * GET Laser1 Current    -> A5  -> i?
  
  LDC205C, LDC500 -> k=50mA/V (conversion factor when reading current)
  
  Commands Cobolt Gen5 style+ 
*/

#include <OneWire.h>
#include <DallasTemperature.h>

// Variables and constants fot the GUI
unsigned int serialNumber = 999; // Just a check in the GUI.
float versionNumber = 3.0;         // Initial version.
float time;
bool dallasPresent = true;                // store dallas sensor status

const int pwmLimit = 255;

// Define used pins.
const int laserSH = 13;      // shutter
const int laserMod = 9;      // DAC
const int OneWireBus = 12;   // Data wire for OneWire
// const int laser1CTL = A5;    // ADC from external source

// Configure OneWire
OneWire oneWire(OneWireBus);
DallasTemperature sensors(&oneWire);
DeviceAddress insideThermometer;

// For serial communication.
String inputString = "";         // a string to hold incoming data
bool stringComplete = false;  // whether the string is complete

// The initial setup, that will run every time when we connect to the Arduino via serial.
void setup() {
  Serial.begin(115200);
  pinMode(laserSH, OUTPUT);
  pinMode(laserMod, OUTPUT);
  digitalWrite(laserSH, LOW);
//  while (!Serial) {
//    ; // wait for serial port to connect. Needed for native USB
//  }
  // Change PWM Frequency for Timer2.  f=~31kHZ
  // Copied the relevant part from here: http://playground.arduino.cc/Code/PwmFrequency
  TCCR1B = TCCR1B & 0b11111000 | 0x01;
  inputString.reserve(128);   // reserve 128 bytes for the inputString:
  digitalWrite(laserSH, HIGH);

  // OneWire setup;
  if (!sensors.getAddress(insideThermometer, 0)) {
    Serial.println("Unable to find address for Device 0");
    dallasPresent = false;
  }
  sensors.setResolution(insideThermometer, 10);
}

void setDirect(float i) {
  int iDigit = i;
  analogWrite(laserMod, constrain(iDigit, 0, 255));
  Serial.println("OK\r");
}

float getDirect() {
  return analogRead(laserMod);
}

void printTemperature(DeviceAddress deviceAddress)
{
  float tempC = sensors.getTempC(deviceAddress);
  Serial.println(tempC);
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
      switch (inputString.charAt(1)) {
        case '0':
          digitalWrite(laserSH,LOW);
          Serial.println("OK\r"); break;
        case '1':
          digitalWrite(laserSH,HIGH);
          Serial.println("OK\r"); break;
        case '?':
          Serial.println(digitalRead(laserSH)); break;
        default:
          digitalWrite(laserSH,!digitalRead(laserSH));
          Serial.println("OK\r"); break;
      }
    // Get Output Voltage
    } else if (inputString.startsWith("gpv?")) {
      Serial.println(getDirect());
    // SET PWM directly
    } else if (inputString.startsWith("spv")) {
      setDirect(toFloat(inputString.substring(3)));
    } else if (inputString.startsWith("d")) {
      if (inputString.substring(1,2) == "?")
      Serial.println(analogRead(laserMod));
      else
      analogWrite(laserMod, toFloat(inputString.substring(1)));
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
    } else if (inputString.startsWith("t?")) {
      if (dallasPresent) {
        // call sensors.requestTemperatures() to issue a global temperature 
        // request to all devices on the bus
        sensors.requestTemperatures(); // Send the command to get temperatures
        printTemperature(insideThermometer); // Use a simple function to print out the data
      } else {
        Serial.println("NaN");
      }
    } else if (inputString.startsWith("?")) {
      // Are you there? Returns Ok (undocumented Cobolt command)
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
