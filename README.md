# Two projects inside one github repository:
 * Coherent Sapphire 488mW Laser GUI written in LabView
 * Thorlabs LDC Arduino controller and a GUI written in LabView

## Coherent Sapphire 488mW GUI:

[Our team](http://titan.physx.u-szeged.hu/~adoptim/) got a Coherent Sapphire 488nm laser with the LP USB CDRH Driver Unit. This means, it can be only controlled via RS-232. So I wrote a similar GUI as the Arduino LDC205 one, but with advanced internals (STATES). It can be used with any Coherent Lasers as far as I know, because during startup it will check for minimum and maximum laser power values.

### Changelog:
 * Initial release with states.
 * It searches for a config file: _Documents\LabVIEW Data\LaserControlSettings.ini_
 * There is also a build.

## thorlabs-ldc-arduino the arduino code for a laser driver.
 * Arduino UNO R3 controller for Thorlabs LDC205, and LDC500.
   * You can split the MOD IN output to control both lasers with the same modulation. (We don't use both lasers at the same time.)
 * http://www.thorlabs.de/newgrouppage9.cfm?objectgroup_id=10&pn=LDC205C
 * Serial communication examples are used
 * The String->float conversion: http://forum.arduino.cc/index.php?topic=179666.msg1331654#msg1331654
 * LP Filter: http://sim.okawa-denshi.jp/en/PWMtool.php
 * PWM speedup from here: http://playground.arduino.cc/Code/PwmFrequency
 * DHTlib, get it from here: http://playground.arduino.cc//Main/DHTLib

### Sort of documentation:
 * LabView GUI for LDC205 ver 1.0: https://gajdicookbook.wordpress.com/2016/02/07/189/
   * Rewritten to a Cobolt subVI free version. Updated to ver 2.0.
   * The Project and a build is also included.
 * Controller: https://gajdicookbook.wordpress.com/2015/02/16/arduino-controller-for-thorlabs-ldc200c-series/

### Changelog for the thorlabs-ldc-arduino & GUI:
 * Merged the DHT22 temperature code, from the branch.
 * Added a second _read only_ output for the LDC500 (we got a new laser, which has to be controlled).
   * I updated the serial number to 855 (405+450). And the version to 2.0.
 * De-blobbed the previous LabView GUI, and included in this repository.
 * It searches for a config file: _Documents\LabVIEW Data\LaserControlSettings.ini_
 * There is also a build.

# Start Here for the Arduino Controller:
 1. Clone the repository to your sketchbook folder.
 2. Connect the BNC connectors:
   * LD REM to pin 13. (Same as the L Led on the board)
   * MOD IN to pin 11. (You can split the output.)
   * CTLOUT1 to pin A0. (first laser)
   * CTLOUT2 to pin A1. (second laser, read only mode)
 3. Calculate the Low-pass filter R&C values from here: http://sim.okawa-denshi.jp/en/PWMtool.php
 4. Burn it to your Arduino UNO.
 5. Optionally connect a DHT22 sensor.

# Communication with the Controller: 115200 8N1
## Send Command: Carridge Return (\r)
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

 ## Second laser, just readout
 * j? -> GET laser current (float [mA])
 * q?, qa? -> GET laser power (float [W])

 ## DHT22 Communication:
 * d? -> GET sensor status (0: error, 1: ok)
 * dt? -> GET temperature (float)
 * dh? -> GET humidity (float)

# Manufacturer Documentation
 * Cobolt: http://www.cobolt.se/wp-content/uploads/2016/06/D0106-B_Manual-Cobolt-05-01-Series_June_2016.pdf
   * Look for communication commands.
 * Thorlabs: https://www.thorlabs.com/thorproduct.cfm?partnumber=LDC205C
 * Thorlabs: https://www.thorlabs.com/thorproduct.cfm?partnumber=LDC500
 * Coherent: I have the Operator's Manual on paper.

# Currently used R&C values:
 * R=5k3 Ohm
 * C=105

# Issues with Arduino Leonardo:
 * The board does not handle SerialEvents. [FIX](http://forum.arduino.cc/index.php?topic=150558.msg1131262#msg1131262)
 * [The board does not have Timer2](http://provideyourown.com/2012/arduino-leonardo-versus-uno-whats-new/):
   * Timer4 can be used, but a different board design and pins are needed.

# Graphical User Interface in LabView 14.

![](./UI.png)