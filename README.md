# thorlabs-ldc-arduino
 * Arduino controller for Thorlabs LDC205.
 * http://www.thorlabs.de/newgrouppage9.cfm?objectgroup_id=10&pn=LDC205C
 * Serial communication examples are used
 * The String->float conversion: http://forum.arduino.cc/index.php?topic=179666.msg1331654#msg1331654
 * LP Filter: http://sim.okawa-denshi.jp/en/PWMtool.php
 * PWM speedup from here: http://playground.arduino.cc/Code/PwmFrequency

# Start Here:
 1. Clone the repository to your sketchbook folder.
 2. Connect the BNC connectors:
   1. LD REM to pin 13.
   2. MOD IN to pin 11.
   3. CTLOUT to pin a0
 3. Calculate the Low-pass filter R&C values from here: http://sim.okawa-denshi.jp/en/PWMtool.php
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

# Cobolt Laser Documentation
 * http://www.cobolt.se/wp-content/uploads/2014/10/Owners-Manual-05-01_140611.pdf
 * Look for communication commands.

# Currently used R&C values:
 * R=5k3 Ohm
 * C=105
