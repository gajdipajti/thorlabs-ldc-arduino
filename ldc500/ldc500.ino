/*
  Laser Diode Controller Controller
  
  Functions:
    * ENABLE/DISABLE Laser -> Pin 13 + LED
    * SET Laser Current    -> Pin 11
    * GET Laser Current    -> A0
  LDC500 -> k=50mA/V  
*/

const int laserPin = 13;
const int laserIn  = 11;
const int laserOut = A0;


void setup() {
  pinMode(laserPin, OUTPUT);
  Serial.begin(9600);
  // reserve 200 bytes for the inputString:
  inputString.reserve(200);
}

void loop() {
  // get any incoming bytes:
  if (stringComplete) {
    Serial.println(inputString); 
    // clear the string:
    inputString = "";
    stringComplete = false;
  }
  if (isAlpha()) {
    digitalWrite(laserPin,HIGH);
    delay(100);
    Serial.println("ON\n");
  } else {
    digitalWrite(laserPin,LOW);
    delay(100);
    Serial.println("OFF\n");
  }  
  
  int CTLOut = analogRead(laserOut);
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  float voltage = CTLOut*(10.0 / 1023.0);
  float LaserCurrent = voltage/(10.0/500.0);
  Serial.println(LaserCurrent);
 
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
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    } 
  }
}


