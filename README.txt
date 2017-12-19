----------------------------------------------------------------------------
BlueGigaEmpeg
by Tony Fabris
----------------------------------------------------------------------------
A project to use a Silicon Labs BlueGiga WT32i bluetooth chip, combined with
an Arduino Mega board with an added RS232 port, to act as an intermediary
between the empeg Car stereo and a modern car. The purpose is twofold:

1. Bluetooth A2DP connection allows high quality stereo audio from the
   empeg Car outputs to play on the modern car stereo speakers without
   needing an AUX input on the modern car stereo (many cars no longer
   have an AUX input available any more).

2. Allow empeg Car to recieve commands such as "next", "previous", "pause" etc.
   from the bluetooth connection, thus allowing the car's touchscreen and
   steering wheel controls to be able to change tracks on the empeg.
   This is accomplished by custom code running on the Arduino (this code).

The audio connection works like this:
 Empeg Car ->
 I2S digital outputs from empeg Car ->
 I2S digital inputs on bluetooth chip ->
 Bluetooth chip paired with factory car stereo via Bluetooth A2DP ->
 Sound from empeg comes out the car speakers.

The data connection works like this:
 Car stereo user presses touchscreen controls or steering wheel controls for |<<, >>|, pause, play, stop, etc. ->
 Controls are sent to bluetooth chip from car stereo via Bluetooth AVRCP ->
 Bluetooth chip, by default, echoes the AVRCP commands on its serial output ->
 Arduino board receives these serial commands on its serial port ->
 Arduino runs code which interprets these AVRCP commands and turns them into empeg-Car-compatible commands ->
 Arduino outputs these empeg-Car serial commands to its attached RS232, via another serial port ->
 Empeg Car receives the serial commands and performs the action.
 And also...
 Empeg Car outputs track metadata messages on its serial port ->
 Arduino board receives track information from the empeg serial port ->
 Arduino sends track information to the bluetooth chip via its serial port ->
 Bluetooth chip sends track information to the car stereo ->
 Track data such as Title and Artist appear on the car stereo's display.

NOTE: At the time of this writing I only tested this on a very limited
set of bluetooth gear. I have tested it on:
   My Honda Accord 2017 factory stereo.
   Kenwood bluetooth-equipped car stereo in my roommate's car.
   Plantronics Voyager Edge bluetooth headset.
There may be differences in bluetooth implementation on other audio gear,
and so there might be bugs using this on your car stereo. Your mileage may
vary! :-)

Many thanks to the members of the empegBBS who helped with this so much. The
initial chip suggestion was by BBS member Elperepat, and even though I ended
up not using the particular chip that he suggested (I had trouble with that
chip), it was his suggestion that started me experimenting with bluetooth.
Big thanks to Shonky who had many helpful suggestions and who pointed out
the WT32i chip that is the one that finally got used in this particular
design. Also particular thanks to Stuart Euans of Eutronix who created the
empeg Display Extender board which was also critical to me being able to
implement this at all. And massive thanks to Mark Lord who assisted me in
countless ways with the design and implementation, taught me a ton of
important things, found my bugs, fixed issues with the Arduino compiler,
and pointed out all of my EE design flaws. And last but not least, thanks
to the entire empeg team who made such a fantastic car MP3 player that we
are still doing interesting things with it, nearly 20 years later.


----------------------------------------------------------------------------
Reference materials, resources, and purchase links:
----------------------------------------------------------------------------
Purchase:
Arduino MEGA 2560 R3 Board: https://www.amazon.com/gp/product/B01H4ZLZLQ       
MAX232 circuit for Arduino: https://www.avrprogrammers.com/articles/max232-arduino
                            http://justanotherlanguage.org/content/building-max232-circuit-serial-port-communication
BetzTechnik WT32i Breakout: http://www.betztechnik.ca/store/p3/WT32i_breakout_board.html
Pololu 5v 5a V.Reg #2851:   https://www.pololu.com/product/2851
Molex tuner connector:      https://www.digikey.com/product-detail/en/molex-llc/0039295083/WM3923-ND/356037
RS-232 connector:           https://www.digikey.com/product-detail/en/assmann-wsw-components/A-DS-09-A-KG-T2S/AE10968-ND/1241804

Software, datasheets, and command references (may need to create an account at the Silicon Labs web site):
Bluetooth AVRCP specs:      https://www.bluetooth.org/docman/handlers/DownloadDoc.ashx?doc_id=292286
Command reference:          https://www.silabs.com/documents/login/reference-manuals/iWRAP6-API-RM.pdf
AVRCP command reference:    https://www.silabs.com/documents/login/application-notes/AN986.pdf
Dev Board User Guide:       https://www.silabs.com/documents/login/user-guides/UG215.pdf
Dev Board Data Sheet:       https://www.silabs.com/documents/login/data-sheets/WT32i-DataSheet.pdf
Dev Board Reference Design: https://www.silabs.com/documents/login/reference-designs/Reference-Design-DKWT32i.pdf
Dev Board Full Schematic:   https://www.silabs.com/documents/login/reference-designs/DKWT32i-v2.2.zip
BetzTechnik Schematic:      http://www.betztechnik.ca/uploads/2/5/6/7/25674051/wt32i.pdf
Pololu V.Reg #2851 Pinout:  https://a.pololu-files.com/picture/0J5850.1200.jpg
Arduino Mega Pin Map:       https://www.arduino.cc/en/uploads/Hacking/Atmega168PinMap2.png
Arduino Mega Standalone:    https://www.arduino.cc/en/Main/Standalone
BlueGiga Forum:             https://www.silabs.com/community/wireless/bluetooth

Upgrading firmware on the WT32i:
Firmware Update Guide:      https://www.silabs.com/documents/login/user-guides/UG216.pdf
Page containing firmware:   https://www.silabs.com/documents/login/data-sheets/WT32i-DataSheet.pdf
Link to firmware zip file:  https://www.silabs.com/documents/login/software/iWRAP-Firmware-Releases.zip
Prolific PL2303 USB driver: http://www.prolific.com.tw/US/ShowProduct.aspx?p_id=225&pcid=41
FTDI USB driver:            http://www.ftdichip.com/FTDrivers.htm

ClassOfDevice generators:    http://bluetooth-pentest.narod.ru/software/bluetooth_class_of_device-service_generator.html
                             http://www.ampedrftech.com/cod.htm

Hijack Kernel for empeg:    http://empeg-hijack.sourceforge.net/
----------------------------------------------------------------------------

----------------------------------------------------------------------------
Bluetooth Dev Board Firmware Upgrade
----------------------------------------------------------------------------
Make sure the Bluetooth WT32i chip+board is updated to the latest firmware
(links in "Reference Materials" above). You can run the Windows-based upgrader
tool with a USB micro cable plugged into the "UART" port on the dev board.
At the time of this writing, here were the steps I took:

 - Make sure that jumper JP4 on the BetzTechnik WT32i bluetooth board is connected.
   You will need to cut this trace after you're done upgrading. It will need to be
   temporarily reconnected any time you do firmware updates to the WT32i chip.
 - Make sure the Bluetooth breakout board is fully disconnected from the Arduino
   and all of its related electronics modules which are part of this assembly.
   Upgrade will not work if it is connected to Arduino serial port.
 - Download and unzip the firmware zip file linked in "Reference Materials" above.
 - Plug in the USB cable to the bluetooth board in the "UART" port.
 - Look in Windows device manager to see if the USB cable has made a serial port.
 - If it has a little yellow boo boo icon instead of a serial port, then...
 - Install the necessary drivers for the USB-serial connection to the dev board.
   These will be a Prolific brand driver if you are using the BlueGiga dev board,
   or an FTDI brand driver if you are using the BetzTechnik dev board.
 - Now, plugging in the dev board into the PC with the USB cable makes a serial port
   appear in the Windows Device Manager.
 - Once that's sorted, run the "SerialDFU.exe" tool found in one of the unzipped folders.
 - Make sure the "Get Device Type" button works and shows your device information.
 - Have it upgrade your chip with the most recent WT32i file among the unzipped files.
   The full path is:
      iWRAP_Firmware_Releases
        Firmware
         DFU
          SerialDFU     (SerialDFU.exe is the upgrader tool in this folder)
           DFU_Images
             WT32i
               ai-6.2.0-1122_aptxll.bc5.dfu        (firmware file)
 - Note: When upgrading, there is a checkbox on the screen in the SerialDFU.exe
   utility which says "Factory Restore All". Make sure to CHECK that checkbox when doing
   the firmware upgrade.
 - After the upgrade is successful, disconnect everything and cut the trace JP4
   on the BetzTechnik WT32i bluetooth board.

 ----------------------------------------------------------------------
 IMPORTANT - Modifying the Arduino compiler for larger buffer size
 ----------------------------------------------------------------------
 For this code to work you must increase the size of the serial port
 buffers in the Arduino compiler, otherwise some of the track
 titles will not work. The symptom will be that you switch songs on
 the empeg, and the track title on the car stereo screen does not
 change to the new song title.
 
 To fix the issue, you must edit one of the header files in the
 Arduino compiler libraries, and then you must compile and upload
 your sketch from your local PC using the local Arduino compiler and
 uploader (the online web editor will not work for this).
 
 The file that you need to edit will be the same on all operating
 systems, but the location of the file will be different depending
 on which OS you're using. The approximate location will be somewhere
 around this location, but the install location will vary:
 
    (install location)/hardware/arduino/avr/cores/arduino/HardwareSerial.h
 
 On a Macintosh computer, the file is harder to find. It is located
 in the following place on a Macintosh:
 
    /Applications/Arduino.app/Contents/Java/hardware/arduino/avr/cores/arduino/HardwareSerial.h
 
 Or, more specifically, in the Macintosh Finder, navigate to:
 
    Macintosh HD -> /Applications/Arduino
 
 Ctrl-click on the application file "Arduino" and select "Show Package
 Contents". Then navigate to:
 
    Contents/Java/hardware/arduino/avr/cores/arduino/HardwareSerial.h
 
 Ctrl-click on the file "HardwareSerial.h" and select "Open With" and
 choose your favorite quick text editor program to edit the file with it.
 Locate the following code section:
 
           #if !defined(SERIAL_TX_BUFFER_SIZE)
           #if ((RAMEND - RAMSTART) < 1023)
           #define SERIAL_TX_BUFFER_SIZE 16
           #else
           #define SERIAL_TX_BUFFER_SIZE 64
           #endif
           #endif
           #if !defined(SERIAL_RX_BUFFER_SIZE)
           #if ((RAMEND - RAMSTART) < 1023)
           #define SERIAL_RX_BUFFER_SIZE 16
           #else
           #define SERIAL_RX_BUFFER_SIZE 64
           #endif
           #endif
 
 First, check to make sure this section of code is getting compiled
 at all. Insert the following line immediately before the line which
 says "#define SERIAL_RX_BUFFER_SIZE 64"
 
     #error This line of code has been reached successfully
 
 Now, in your Arduino compiler program, attempt to verify/compile and
 upload your sketch to your Arduino. The compilation step should fail
 and it should clearly show the error message that you just inserted.
 Now you know that the header file is the correct header file and that
 your compiler is picking it up and using it.  
 
 Now remove the error line from the header file so that it will build.
 
 Now, instead, edit the lines "#define SERIAL_TX_BUFFER_SIZE 64" and
 "#define SERIAL_RX_BUFFER_SIZE 64" and change them to this instead:
 
     #define SERIAL_TX_BUFFER_SIZE 128
         ...    ...    ...
     #define SERIAL_RX_BUFFER_SIZE 256
 
 Save the file, compile and upload to your Arduino, and you're done.
 

----------------------------------------------------------------------------
Hardware Connection information (internal board connections):
----------------------------------------------------------------------------
Arduino must be the "Mega" model with three hardware serial ports built in.
I tried it with an "Uno" model using software serial ports, and it had problems
where characters sometimes got dropped, and it could not keep up with the data
rates that I wanted.

Arduino serial port 1 goes to the empeg's RS-232 port, attached via a MAX232 chip
circuit (see resources above for schematic). This is necessary because you can't
connect an actual RS-232 cable directly to an Arduino, the voltage and signaling are
different. So it must go through the MAX232 circuit.

Arduino serial port 2 (RX2/TXT) goes to the bluetooth chip's serial port. This is at
TTL level so the Arduino and the bluetooth chip can connect almost directly with wires
or traces instead of needing to go through a MAX232 circuit. However, the TX wire from
the Arduino is running at 5v TTL, and the BlueGiga chip runs at 3v (and our assembly
will actually have it nominally at 2.5v), you must run the Arduino TX2 output through
a simple voltage divider to step 5v TTL from Arduino down to 2.5v for the BlueGiga chip.
This is a simple circuit with just 2 resistors. More details below.


Arduino and RS-232 port, critical connections:

You must roll your own RS-232 circuit, connecting the Arduino hardware serial port 1 TX1/RX1
pins to a MAX232 as directed by the schematic linked above. Leave the option open to flip them into
a crossover configuration just in case they don't work straight. I can never keep track of which
way they're supposed to go, I just get it working correctly on my board and then don't touch it.
Give it 5 volts from the 5v rail, according to the MAX232 schematic in the resources linked above.
(There will also be capacitors in that schematic that must be correctly placed too.)

Empeg tuner connector is used to supply power to the Arduino and the rest of the assembly.
Blue wire on tuner connecter connects to the voltage input pin on the 12v-to-5v step-down transformer
power supply, and the black wire on the tuner connector connects to the ground input pin on the
step-down power supply.

Arduino "5v" pin is connected to the 5v output rail of the 12v-to-5v step-down transformer power
supply circuit.

Grounding - Multiple Arduino GND pins connected to the output ground rail of the 5v power supply. 

Arduino and Pair mode indicator LED - Arduino pin 50 digital I/O pin, connected to +LED through
resistor (current of resistor determined by calculator and LED values), then -LED connect to GND.
Resistor current calculator: http://led.linear1.org/1led.wiz
Example: If using a blue LED with a 3.2v forward voltage and 20ma current, and the Arduino
analog power supply from the analog pins will be 5 volts, then use a 100 ohm resistor for this value.

Arduino and Button - Arduino pin 52 digital I/O pin, connected to one of the ground legs of button.
Other ground leg of button goes through 10k pulldown resistor to ground. One of the + legs of
the button connects to +5v. Follow examples on the internet of how to implement
a simply temporary pushbutton on an Arduino: https://www.arduino.cc/en/Tutorial/Button

BlueGiga Bluetooth WT32i chip+board, critical connections:

Bluetooth chip+board "VBus" or "5v" power pin connected to 5v rail from the step-down power supply.

Bluetooth chip+Board "Gnd" pin connected to the ground rail. Additional GND pins are good too,
to make sure solid ground is achieved. 

Bluetooth chip+board "RX/TX" pins connected to the "TX2/RX2" pins (serial port 2) of Arduino
Note: This is a crossover connection, i.e., RX from the Arduino is connected to TX on the dev board.
On the line that runs from the Arduino TX2 pin to the RX pin on the Bluetooth chip, add a voltage
divider circut by using two 10k resistors in the following configuration:

Connect one of the 10K resistors to the TX2 pin from the Arduino, connect the other 10K resistor to
GND, and connect the other leads of each resistor to each other. The RX-input of the WT32i then
gets connected to that same point (where the two resistors are tied together). Example schematic:

    ARDUINO -----VVVVV----+----VVVVV-----GND
    TX2         10Kohm    |    10Kohm
                          |
                       WT32i Rx

Bluetooth chip+Board three I2S pins PCM_CLK, PCM_SYNC, PCM_IN connected to empeg IISC, IISW, IISD1
via special modification to the empeg the tuner connector, as described in the section below titled
"Empeg Car Interior Modification for Digital I2S connection". These will each need to be coming in
through some resistors arranged in a voltage divider configuration, with a 4.7k and a 10k resistor
for each one of the three lines. Example of one line:

    EMPEG -----VVVVV----+----VVVVV-----GND
    IISC      4.7Kohm   |    10Kohm
                        |
                  WT32i PCM_CLK

Experimental Reset Line:
Bluetooth chip+board RST (reset) pin connected to Pin 51 of the Arduino board. This is working for
Mark Lord, but my chip fried immediately after trying this, so I consider this to be a risk. Perhaps
this needs to go through a diode and/or a voltage divider (similar to the other voltage dividers
described above) in order to work. Mark Lord is still investigating.

Self jumpers and switches on the Bluetooth chip+board, the BetzTechnik WT32i Breakout board V2:

  IMPORTANT: Must cut the jumper at "J4 FTDI_+5v" but only do this AFTER successfully
  updating the chip's firmware to the latest version. Cutting the jumper shuts off
  the the onboard UART and prevents the UART and the Ardino pin-to-pin serial connection
  from arguing with each other. This prevents errors on the serial port which cause the
  chip to reboot randomly.

  "Smd_2_pole_switch" set to the left or up position, to put it into always-on mode.
  The red LED on the breakout board will blink randomly when power is applied.
  Symptoms of this switch being set wrong will be: LED on the breakout board
  blinks steady when power is applied instead of randomly, and there is no serial
  port communication, and you need to press the reset button on the board to start up the module.


----------------------------------------------------------------------------
Hardware Connection information (external):
----------------------------------------------------------------------------
Debugging console to Arduino (during software development): USB cable
connected from computer to Arduino USB port.

Power to Arduino: For debugging mode, the USB cable from the computer can
power the Arduino and the bluetooth board. When connected to the empeg,
supply power to the 12v power input for the power supply (implemented
as a 12v->5v DC-DC power converter as part of this assembly) via the
Tuner connector.

Note: Do not try to use direct 12v from the car to power the Arduino into the
Arduino board's power 5.5mm barrel plug connector, because the Arduino can't
actually handle direct 12v power from the car despite what its specs say.
Instead, use the 12v-to-5v step-down DC-DC transformer included as part
of the final design, with 12v +/- connections supplied and connected to the
tuner connector.

Power to BlueGiga board+chip: The BlueGiga breakpout board gets its power from
the 5v rail that comes from the 12v-to-5v step-down power supply.

RS-232 serial port connector connected to the serial port on the empeg Car player
via its car docking sled.

Audio: connects to I2S data coming from the empeg over modified wires on the
tuner connector. More details about that below.


----------------------------------------------------------------------------
Special note about empeg Car power connection:
----------------------------------------------------------------------------
I might recommend that, for this installation, you wire up the empeg to your
car differently than it would normally otherwise be wired up.

This design is intended to be used in such a way so that the empeg is
not the primary stereo in the system. The empeg becomes one of the bluetooth
inputs into a modern car stereo. So the empeg sleep mode, the empeg "always
on" power connection, and the way this bluetooth module gets its power from
the empeg tuner connector, all combine to cause some interesting problem with
power state transitions. At least they did in my car, your mileage may vary.

I found that if I connected the empeg to my car via the regular method (i.e.
the ignition wire to the orange wire on the sled, and constant 12v power to
the yellow wire on the sled) then there were certain unusual states that it
could get into, where I might sometimes shut off my car but the empeg would
come back up out of sleep mode and play tracks silently to an unconnnected
bluetooth module.

If this happens to you, then I recommend connecting it like this:
- Car power 12v accessory power -> Connect to yellow "memory" wire on empeg.
- Connect that same 12v accessory power to orange "ignition" wire on empeg.
- Do not connect your car's constant 12v power to any part of the empeg.
- Connect your car's illumination wire to the empeg's illum. sense wire.
- Connect ground to ground of course.
That should be all you need. No audio or amplifier connections because the
bluetooth takes care of that now. No cellphone mute wire because the
bluetooth takes care of that now. No amp remote wire because there is no
amp. No FM tuner or aux connection, etc, etc... 

In this situation, the empeg does not have a 12v constant power concept
at all and will not go into sleep mode when you turn off the ignition.
However it, and the bluetooth, will always power on and off at the same
time with the car's headunit and nothing will get confused. You might lose
the clock/date/time information on the empeg sometimes but your modern car
likely has a perfectly functional clock of its own.

Finally, you'll notice that the empeg sled connector only supplies power
if the empeg is awake. If you put the empeg into Sleep mode via a long
press on the top button, it will go to sleep but that also means the 
Arduino+bluetooth assembly stops receiving power. This is actually good:
you want to be able to turn the bluetooth on and off and reboot it
sometimes, and it's easy to do that just by sleeping or waking the player
by hand. In my case I leave the player awake most of the time, so that
when I shut off the car ignition, the player fully shuts down, and then
when I start my car again, the empeg starts up in Awake mode and
immediately supplies power to the bluetooth assembly which then autoconnects
to the car stereo head unit.


----------------------------------------------------------------------------
Empeg Car Interior Modification for Digital I2S connection:
----------------------------------------------------------------------------

You will be modifying the interior of your empeg Car so that there can be
three wires coming out the docking connector which carry digital audio 
data ("I2S" aka "IIS") and can connect to this external module. This is done
by hijacking three of the wires originally used for the FM Tuner Module
and its molex connector. This tuner module connector will instead plug
into my electronic assembly which contains the bluetooth chip and the
Arduino.

IMPORTANT: Tuner module will no longer work after this modification.
If you have any tuner modules, disconnect all of them from any and all
sleds that you own. Damage may occur to your empeg and your tuner module
if you dock your modified empeg to any tuner module after you make this
modification.

Disassemble your empeg player by carefully removing the fascia, lid and
drive tray. Refer to the empeg FAQ for disassembly instructions. You must
do the disassembly carefully so as not to damage the empeg. You should
theoretically be able to do this without disconnecting the IDE cable
but it might be easier if you do. You should not need to remove the
display board as long as you are careful not to break any of the
components sticking off the back of the display board.

Locate 5 blank IIS pads on the front right side of the empeg motherboard. 
Pads are outlined in a white silkscreen rectangle with the letters "IIS"
next to the outline. White silkscreen rectangle will have one corner of
it with a small diagonal "snip" to the corner which will indicate pad 
number 1 of the set of 5 pads. This diagonal snip will be towards the back
of the empeg, and the remaining pads continue towards the front of the
empeg. Pads are not individually labeled with numbers, but understand that
they are logically numbered 1 2 3 4 5 starting from the back and going
towards the front of the empeg. The pinouts of these five pads are:
    1 = IISC  aka  SCK     aka "serial clock"
    2 = IISW  aka  WS      aka "word select"
    3 = GND   aka  Ground  same as chassis ground of empeg
    4 = IISD1 aka  SD      aka "serial data" (1 of 2)
    5 = IISD2 aka  SD      aka "serial data" (2 of 2)
Carefully solder some small jumper wires to pads 1,2, and 4, and you must
keep track of which wires are soldered to which pads.

When soldering, make sure the jumper wires and the solder are flat to the
board instead of sticking upwards. The disk drive tray gets close to that
location when mounted, so make sure they don't have a chance to contact
the tray. Take a look at Stu's instructions for his digital sound board
for the empeg, which utilizes these same three connections on the IIS pads
for an example of how to solder these three wires correctly:
       http://www.eutronix.com/media/dig-cxxxx_manual.pdf
After soldering, cover the solder points with tape or some other insulator
to prevent them from shorting out on the drive tray.

Locate the two white connectors on the back part of the empeg motherboard 
which connect two groups of small colored wires to the docking connector
assembly. Locate the rightmost of the two connectors nearest the Ethernet
port on the back of the empeg. Unplug this connector and using a tiny flat
tool such as the tip of a jeweler's screwdriver, lift up the small white tabs
and gently disengage and pull out the rightmost three pins nearest the
Ethernet connector to free up those wires. On my unit, these wires were
colored, going left to right, yellow+green, red, and brown, though your unit
may differ. The brown wire is the one on the end closest to the Ethernet
connector, the red wire is the second from the end closest to the Ethernet
connector, and the yellow+green wire is the third closest to the Ethernet
connector.

You should now have three loose wires inside the empeg, coming out of the 
docking connector assembly, with small pin connectors on the ends of them.
Carefully solder and shrink-tube your three jumper wires to directly these pin
connectors. After you have done this, your empeg's IIS pads will, when docked,
now be carried out the back of the empeg, via the docking sled, into the tuner
module molex connector. Solder your jumper wires to the empeg interior wires
in this order:
   IIS Pad 1 (IISC)  jumpered to Yellow+Green wire, originally third from end.
   IIS Pad 2 (IISW)  jumpered to Brown wire, originally the end wire.
   IIS Pad 4 (IISD1) jumpered to Red wire, originally second from end.

Make sure that your jumper wires are carefully tucked down around the left
side of the empeg and out of the way of the disk drive tray assembly when you
reinstall it. 

Final wiring positions and colors:

Empeg IIS pads   My Jumper wires*  Int. Empeg wires**   Int. wht Conn pos***   Sled Tuner Plug   BlueGigaEmpeg Tuner Plug    Bluetooth Chip   Usage on BlueGigaEmpeg Assembly
1 IISC            Yellow            Yellow+Green         Third from end         7 Purple          7  SCK                      30  PCM_CLK      Serial Clock
2 IISW            Orange            Brown wire           Right end              2 Grey            2  WS                       29  PCM_SYNC     Word Select
4 IISD1           Red               Red wire             Second from end        1 Pink            1  SDIN                     27  PCM_IN       Serial Data
                                                                                4 Black           4  GND                          GND          Universal Ground
                                                                                8 Blue            8  12vPower                                  Power to voltage regulator

*(These are just the colors I used for my jumper wires, yours may differ.)
**(These were the colors inside my empeg, yours may differ. These are the interior wires connecting the docking sled connector to the motherbopard.)
***(Original positions of these wires on the white connector near the Ethernet plug, now disconnected from that plug.)

On the bluetooth module side of this connection, all three I2S input lines from
the empeg going into the bluetooth will need to be protected by a voltage divider
circuit which uses two resistors. Example schematic, will need to be repeated for
all three I2S input lines on the bluetooth module side of the circuit:

    EMPEG -----VVVVV----+----VVVVV-----GND
    IISC      4.7Kohm   |    10Kohm
                        |
                  WT32i PCM_CLK

Finally, make sure that the variable "digitalAudio", which is a boolean flag in
the Arduino code sketch in this project, is set to "true" so that the digital
audio input on the WT32i chip is used, instead of the default analog input.


----------------------------------------------------------------------------
Empeg Car Configuration information:
----------------------------------------------------------------------------
I have tested this with empeg Car firmware version 2.00 Beta 13. I do not
know if it will work the same on 3.0 Alpha version of empeg Car firmware.

Using the Emplode or Jemplode software, edit the empeg Car's config.ini.

If you are using Emplode on Windows, you must first edit the Microsoft
Windows registry before it will allow you to alter the config.ini. First,
run REGEDIT.EXE in windows and locate the following key:
   HKEY_CURRENT_USER
     Software
       SONICblue
         emplode
           2.0
             Settings
Inside the "Settings" key, create a new DWORD value named
               allow_edit_config
with a value of
               1
then restart the Emplode software. Emplode will now have a menu
option which allows you to edit the config.ini on the player.
Change the settings as follows (adding sections if necessary):

 [Hijack]
 suppress_notify=2

 [Output]
 notify=1

 [Serial]
 car_rate=115200

Make sure to synchronize with the player after changing the config.ini settings.

Install the latest Hijack Kernel onto the empeg Car player if it is not already
installed (link to Hijack in "Resources", above). Make sure it is Hijack version
524 or later.

Open up the Hijack kernel settings on the player via a longpress on the knob.
Change "Serial port assignment" to "Player uses serial port" and follow the
instructions to reboot the player.


----------------------------------------------------------------------------
Notes about debugging via the Arduino USB connection - Startup sequence
----------------------------------------------------------------------------
Your mileage may vary, but on my system there is an interesting trick with
debugging via the Arduino USB cable and the Arduino Serial Monitor.

When everything is connected together (empeg, Arduino, bluetooth), the assembly
and all devices which are part of the assembly will get their power off of the
12v power coming off the tuner connector on the empeg sled. This means that
if you plug all this stuff in, it's already powered by the time you try to 
attach your USB debug cable to Arduino. 

On my computer system, if the Arduino is already externally powered when I
connect my debug cable, then my computer cannot find its USB UART and is
unable to connect to the debug port. 

So to successfully debug via the Arduino USB debug cable and use the Arduino
Serial Monitor program, I must connect the USB cable *first*, before applying
power to the empeg. Meaning if I am debugging in the car, I should connect
the USB cable first and then turn on the ignition, that way the Serial
Monitor program can find a port to connect to.

When using the Arduino Serial Monitor program, set it to 115200 baud,
with either "Newline" or "Both NL and CR" as line endings. If you are using a
different serial terminal program, set it to 115200 8n1 and turn on local
echo.

----------------------------------------------------------------------------
Notes about pairing and bluetooth PIN codes
----------------------------------------------------------------------------
This device implements a RESET/PAIR button which does the following:

 - Erases all previous pairings

 - Looks for devices to pair with for 30 seconds and pairs with
   the first one that it sees.

 - The RESET/PAIR LED is lit during this pairing mode.

But, this is counterintuitive: Sometimes you don't press that RESET/PAIR
button at all, and instead you pair when the RESET/PAIR LED is *not* lit.

There are some stereos which need you to initiate pairing solely on the
car stereo's touch screen. Mine is one of these types of stereos. In my
case, for initial pairing with my car stereo the first time, I don't
press the RESET/PAIR button at all. I just go to my car's touch screen,
tell it to pair with the bluetooth device named "empeg Car", and it works.
After that, it reconnects automatically each time I get in the car and
start it up (though it takes a moment to connect).

Some stereos and headsets *do* need you to press that RESET/PAIR button,
which should light the LED for ~30 seconds, and then you should put your
headset or stereo into pair mode while the LED is still lit.

If one method doesn't work for you, try the other method. For example
this sequence will work for many stereos and headsets:

 - Press the RESET/PAIR button, and while the LED is still lit,
   then initiate pairing at your stereo or headset via its
   pairing feature.

 - If that doesn't work, then wait for the RESET/PAIR LED to go dark
   and then, without touching the RESET/PAIR button, try to initiate
   pairing with your stereo or headset via its pairing feature.

Special note about bluetooth PIN codes:

The code defaults to Secure Simple Pairing in "Just Works" mode, which
does not require a PIN code to successfully pair. It will fall back to
using a PIN code if the Secure Simple Pairing doesn't work. This should
be good for most devices. However the PIN code defaults to "0000", and
though that should work with most devices, if your pin code happens to
be different from "0000", then you should change the code in this sketch.
Find the line in the code that defines the variable "btPinCodeString".
That line contains the "0000" default PIN code. Change it to your correct
PIN code for your device. It can accept PIN codes up to 16 digits long.

