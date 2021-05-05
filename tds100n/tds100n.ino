// Include the libraries we need
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 2              // Data wire is plugged into port 2 on the Arduino

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);// Pass our oneWire reference to Dallas Temperature.
DeviceAddress insideThermometer;    // arrays to hold device address

// For serial communication.
String inputString = "";            // a string to hold incoming data
boolean stringComplete = false;     // whether the string is complete

bool streamMode = false;            // enable stream mode
// Wait mode
short waitPeriod = 1000;
unsigned long startWait = 0;

void setup(void)
{
  
  Serial.begin(115200);             // start serial port

  // locate devices on the bus
  sensors.begin();
  // Serial.print("Found ");
  // Serial.print(sensors.getDeviceCount(), DEC);
  // Serial.println(" devices.");

  // report parasite power requirements
  // Serial.print("Parasite power is: "); 
  // if (sensors.isParasitePowerMode()) Serial.println("ON");
  // else Serial.println("OFF");
  
  // Assign address manually. The addresses below will beed to be changed
  // to valid device addresses on your bus. Device address can be retrieved
  // by using either oneWire.search(deviceAddress) or individually via
  // sensors.getAddress(deviceAddress, index)
  // Note that you will need to use your specific address here
  //insideThermometer = { 0x28, 0x1D, 0x39, 0x31, 0x2, 0x0, 0x0, 0xF0 };

  // Method 1:
  // Search for devices on the bus and assign based on an index. Ideally,
  // you would do this to initially discover addresses on the bus and then 
  // use those addresses and manually assign them (see above) once you know 
  // the devices on your bus (and assuming they don't change).
  if (!sensors.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0"); 
  
  // method 2: search()
  // search() looks for the next device. Returns 1 if a new address has been
  // returned. A zero might mean that the bus is shorted, there are no devices, 
  // or you have already retrieved all of them. It might be a good idea to 
  // check the CRC to make sure you didn't get garbage. The order is 
  // deterministic. You will always get the same devices in the same order
  //
  // Must be called before search()
  //oneWire.reset_search();
  // assigns the first address found to insideThermometer
  //if (!oneWire.search(insideThermometer)) Serial.println("Unable to find address for insideThermometer");

  // show the addresses we found on the bus
  // Serial.print("Device 0 Address: "); printAddress(insideThermometer); Serial.println();

  // set the resolution to 10 bit (Each Dallas/Maxim device is capable of several different resolutions)
  sensors.setResolution(insideThermometer, 10);
 
  // Serial.print("Device 0 Resolution: "); Serial.print(sensors.getResolution(insideThermometer), DEC); Serial.println();
}

// function to print the temperature for a device
void printTemperature(DeviceAddress deviceAddress)
{
  float tempC = sensors.getTempC(deviceAddress);
  Serial.println(tempC);
}
/*
 * Main function. It will request the tempC from the sensors and display on Serial.
 */
void loop(void)
{ 
  if (stringComplete) {
    if (inputString.startsWith("t?")) {
      // call sensors.requestTemperatures() to issue a global temperature 
      // request to all devices on the bus
      sensors.requestTemperatures(); // Send the command to get temperatures
  
      // It responds almost immediately. Let's print out the data
      printTemperature(insideThermometer); // Use a simple function to print out the data
    } else if (inputString.startsWith("stream")) { streamMode = !streamMode;   // Change operation mode.

    }
    // clear the string:
    inputString = "";
    stringComplete = false;
  } else if (streamMode) {
    // Experimental stream mode, for testing
    // https://dzone.com/articles/arduino-using-millis-instead-of-delay
    startWait = millis();
    sensors.requestTemperatures();
    printTemperature(insideThermometer);
    while(abs(millis() - startWait) < waitPeriod) {
        // Do nothing
        // If there is a rollover, immediately restart
    } 
  }
}

// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

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
