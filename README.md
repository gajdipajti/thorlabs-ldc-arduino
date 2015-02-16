# thorlabs-ldc-arduino
 * Arduino controller for Thorlabs LDC500.
 * http://www.thorlabs.de/newgrouppage9.cfm?objectgroup_id=10&pn=LDC205C
 * Serial communication and DHT examples are used.
 * The String->float conversion: http://forum.arduino.cc/index.php?topic=179666.msg1331654#msg1331654

# Start Here:
 1. Clone the repository to your sketchbook folder.
 2. Connect the BNC connectors:
   1. LD REM to pin 13.
   2. MOD IN to pin 11.
   3. CTLOUT to pin a0
 3. Optionally, connect a DHT22 sensor to pin 5.
 4. Burn it to your Arduino.

# Communication with the Controller:
## Serial: 115200 8N1
### Send Command: Carridge Return (\r)
 * l? -> GET laser status (0: disabled, 1: enabled)
 * l[0,1] -> DISABLE/ENABLE laser
 * i? -> GET laser current (float [mA])
 * p?, pa? -> GET laser power (float [W])
 * slc[0..100] -> SET laser current
 * p[0.0001..0.1] -> SET laser power
 * hrs? -> GET uptime in minutes (float)
 * sn? -> GET serial (int)
 * ilk? -> GET interlock state (always 0)
 * f? -> GET operation fault state (always 0)
 * leds? -> GET led status (1: Power on, 7: Laser on)

# DHT Commands: 
 * d? -> GET humidity & temperature (float; float)
 * dh? -> GET humidity (float)
 * dt? -> GET temperature (float)

# DHTlib from 
 * https://github.com/RobTillaart/Arduino/tree/master/libraries/DHTlib
 * http://playground.arduino.cc/Main/LibraryList
 * https://learn.adafruit.com/dht/connecting-to-a-dhtxx-sensor

# Cobolt Laser Documentation
 * http://www.cobolt.se/wp-content/uploads/2014/10/Owners-Manual-05-01_140611.pdf
 * Look for communication commands.
