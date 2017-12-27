------------------------------------------------------------------------------
BlueGigaEmpeg
by Tony Fabris
https://github.com/tfabris/BlueGigaEmpeg
------------------------------------------------------------------------------
A project to use a Silicon Labs BlueGiga WT32i Bluetooth chip, combined with
an Arduino Mega board, to act as an interface between an empeg Car Mk2 (or
Rio Car) MP3 player and a modern Bluetooth-equipped car.

The empeg Car (small "e") player, also sold as the Rio Car, is an amazing
in-dash MP3 player made from 1999-2001 whose features are still, to this day,
better than any other in-car MP3 solution available. However, the empeg Car
didn't have Bluetooth since it wasn't in common use yet at the time.

The purpose of this BlueGigaEmpeg interface is twofold:

1. Bluetooth A2DP connection allows high quality stereo audio from the empeg
   to play on the modern car stereo speakers without needing an AUX input on
   the modern car stereo. The Bluetooth connection is useful because many cars
   no longer have an AUX input available, yet most modern cars now have
   Bluetooth available.

2. Allow empeg Car to receive Bluetooth AVRCP commands such as "next",
   "previous", "pause", etc. from the Bluetooth connection, thus allowing the
   car stereo's touchscreen and steering wheel controls to be able to change
   tracks on the empeg. In addition, the same AVRCP channel allows track title
   and artist information to be displayed on the car stereo's touchscreen.
   It's all accomplished by this code running on the Arduino.

Some modification of the empeg Car is required in order for this to work. Make
sure to perform all modifications listed in this document for this code to
work correctly. See "Checklist" below.

The audio connection works like this:
 Empeg Car ->
 I2S digital outputs from empeg Car ->
 I2S digital inputs on Bluetooth chip ->
 Bluetooth chip paired with factory car stereo via Bluetooth A2DP ->
 Sound from empeg comes out the car speakers.

The data connection for playback controls works like this:
 User presses touchscreen or steering wheel playback controls in car ->
 Control commands are sent from car stereo via Bluetooth AVRCP ->
 Bluetooth chip echoes the AVRCP commands on its serial output ->
 Arduino board receives these serial commands on its serial port ->
 Arduino runs code which interprets these AVRCP commands ->
 Arduino outputs empeg Car serial commands to its attached RS232 ->
 Empeg Car receives the serial commands and performs the action.

The data connection for track title/artist information works like this:
 Empeg Car outputs track metadata messages on its serial port ->
 Arduino board receives track data from the empeg serial port ->
 Arduino sends track data to the Bluetooth chip via its serial port ->
 Bluetooth chip sends track data to the car stereo ->
 Track data such as Title and Artist appear on the car stereo's display.

NOTE: At the time of this writing, I only tested this on a very limited
set of Bluetooth gear. I have tested it on:
   My Honda Accord 2017 factory stereo.
   Kenwood Bluetooth-equipped car stereo in my roommate's car.
   Plantronics Voyager Edge Bluetooth headset.

There may be differences in Bluetooth implementation on other audio gear, and
so there might be bugs using this on your car stereo. This project is open
source on GitHub so that I can accept bug fixes from people with other brands
of Bluetooth gear.


------------------------------------------------------------------------------
Acknowledgments
------------------------------------------------------------------------------
Many thanks to the members of the empegBBS who helped with this so much. The
initial chip suggestion was by BBS member Elperepat, and even though I ended
up not using the particular chip that he suggested (I had trouble with that
chip), it was his suggestion that started me experimenting with Bluetooth. Big
thanks to Shonky who had many helpful suggestions and who pointed out the
WT32i chip that is the one that finally got used in this particular design.
Also particular thanks to Stuart Euans of Eutronix who created the empeg
Display Extender board which was also critical to me being able to implement
this at all. And massive thanks to Mark Lord who assisted me in countless ways
with the design and implementation, taught me a ton of important things, found
my bugs, fixed issues with the Arduino compiler, and pointed out all of my EE
design flaws. And last but not least, thanks to the entire empeg team who made
such a fantastic car MP3 player that we are still doing interesting things
with it, nearly 20 years later.


------------------------------------------------------------------------------
Checklist
------------------------------------------------------------------------------
Implementing this requires some manual labor. Make sure to go through each
step in this checklist. Each step is detailed in its own section, below.

- Bluetooth Chip Firmware Upgrade
- Modify your Arduino compiler for larger buffer size
- Compile and upload the latest version of BlueGigaEmpeg.ino to the Arduino
- Disconnect all tuner modules from all sleds you own
- Modify Empeg Car interior for I2S digital audio connection
- Upgrade the empeg Car's hijack kernel and set "Serial Port Assignment"
- Empeg Car configuration changes
- Set jumpers, switches, and traces
- Connect external hardware connections
- Set Bluetooth PIN code if needed (most likely not needed)
- Apply power and pair Bluetooth
- Modify empeg's power connection to car if needed
- Debug Bluetooth connection if needed


------------------------------------------------------------------------------
Resources
------------------------------------------------------------------------------
Purchase:
Empeg Car player:           http://empegbbs.com/ubbthreads.php/forums/11/1/For_Sale
Arduino MEGA 2560 R3 Board: https://www.amazon.com/gp/product/B01H4ZLZLQ       
BetzTechnik WT32i Breakout: http://www.betztechnik.ca/store/p3/WT32i_breakout_board.html
BlueGigaEmpeg Interface:    TBA (I plan to make my interface board available)

Bluetooth information, schematics, and command references:
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
ClassOfDevice generators:   http://bluetooth-pentest.narod.ru/software/bluetooth_class_of_device-service_generator.html
                            http://www.ampedrftech.com/cod.htm
MAX232 circuit for Arduino: https://www.avrprogrammers.com/articles/max232-arduino
TI MAX232E datasheet:       http://www.ti.com/lit/ds/symlink/max232e.pdf
BlueGiga Forum:             https://www.silabs.com/community/wireless/bluetooth
Empeg BBS thread:           http://empegbbs.com/ubbthreads.php/topics/370217

Software:
Arduino software:           https://www.arduino.cc/
Hijack Kernel for empeg:    http://empeg-hijack.sourceforge.net/
Tony's Empeg Logo Editor:   http://empegbbs.com/ubbthreads.php/ubb/download/Number/7067/filename/logoedit17.zip

Upgrading firmware on the WT32i:
Firmware Update Guide:      https://www.silabs.com/documents/login/user-guides/UG216.pdf
Page containing firmware:   https://www.silabs.com/documents/login/data-sheets/WT32i-DataSheet.pdf
Link to firmware zip file:  https://www.silabs.com/documents/login/software/iWRAP-Firmware-Releases.zip
Prolific PL2303 USB driver: http://www.prolific.com.tw/US/ShowProduct.aspx?p_id=225&pcid=41
FTDI USB driver:            http://www.ftdichip.com/FTDrivers.htm

Parts used in BlueGigaEmpeg interface board:
Pololu 5v 5a V.Reg #2851:   https://www.pololu.com/product/2851
TI MAX232E:                 https://www.digikey.com/product-detail/en/texas-instruments/MAX232EIN/296-27963-5-ND/1511027
RS-232 connector:           https://www.digikey.com/product-detail/en/assmann-wsw-components/A-DS-09-A-KG-T2S/AE10968-ND/1241804
Molex tuner connector:      https://www.digikey.com/product-detail/en/molex-llc/0039295083/WM3923-ND/356037
Molex connector screws: 2x  https://www.digikey.com/product-detail/en/b-f-fastener-supply/MPMS-003-0008-PH/H743-ND/274954
LED:                        https://www.digikey.com/product-detail/en/cree-inc/C503B-BCN-CV0Z0461/C503B-BCN-CV0Z0461-ND/1922945
Reset button:               https://www.digikey.com/product-detail/en/e-switch/TL1105LF250Q/EG2506-ND/378972
Male conn. headers:         https://www.digikey.com/product-detail/en/3m/961236-6404-AR/3M9466-36-ND/2071927
                        3x  https://www.digikey.com/product-detail/en/3m/961110-6404-AR/3M9457-10-ND/2071896
                        4x  https://www.digikey.com/product-detail/en/3m/961108-6404-AR/3M9457-08-ND/7104637
Female conn. header asst:   https://www.amazon.com/gp/product/B074GQ9LKY/ref=oh_aui_detailpage_o01_s00
Ceramic Capacitor asst:     https://www.amazon.com/gp/product/B071VVTC7Z/ref=oh_aui_detailpage_o04_s00
10k ohm resistors:      8x  https://www.amazon.com/gp/product/B0185FIOTA/ref=oh_aui_detailpage_o02_s00
4.7k ohm resistors:     3x  https://www.amazon.com/gp/product/B0185FC5OK/ref=oh_aui_detailpage_o02_s00
100 ohm resistor:           https://www.digikey.com/product-detail/en/stackpole-electronics-inc/CF18JT100R/CF18JT100RCT-ND/2022718
1N914 diode:                https://www.digikey.com/product-detail/en/on-semiconductor/1N914/1N914FS-ND/978749


------------------------------------------------------------------------------
Bluetooth Chip Firmware Upgrade
------------------------------------------------------------------------------
Make sure the Bluetooth WT32i chip+board is updated to the latest firmware
(links in "Resources" above). You can run the Windows-based upgrader tool with
a USB micro cable plugged into the "UART" port on the dev board. At the time
of this writing, here were the steps I took:

 - Make sure that jumper JP4 on the BetzTechnik WT32i Bluetooth board is
   connected. You will need to cut this trace after you're done upgrading. It
   will need to be temporarily reconnected any time you do firmware updates to
   the WT32i chip.

 - Make sure the Bluetooth breakout board is fully disconnected from the
   Arduino and all of its related electronics modules which are part of this
   assembly. Upgrade will not work if it is connected to Arduino serial port.

 - Download and unzip the firmware zip file linked in "Resources", above.

 - Plug in the USB cable to the Bluetooth board in the "UART" port.

 - Look in Windows Device Manager to see if the USB cable has made a serial
   port appear as a device.

 - If it has a little yellow boo boo icon instead of a serial port, then...

 - Install the necessary drivers for the USB-serial connection to the dev
   board. These will be a Prolific brand driver if you are using the BlueGiga
   dev board, or an FTDI brand driver if you are using the BetzTechnik dev
   board. Links to these drivers are in the list of resources, above.

 - Now, plugging in the dev board into the PC with the USB cable makes a
   serial port appear in the Windows Device Manager.

 - Once that's sorted, run the "SerialDFU.exe" tool found in one of the
   unzipped folders.

 - Make sure the "Get Device Type" button works and shows your device
   information.

 - Have it upgrade your chip with the most recent WT32i file among the
   unzipped files. The full path is:
      iWRAP_Firmware_Releases
        Firmware
         DFU
          SerialDFU     (SerialDFU.exe is the upgrader tool in this folder)
           DFU_Images
             WT32i
               ai-6.2.0-1122_aptxll.bc5.dfu        (firmware file)

 - Note: When upgrading, there is a checkbox on the screen in the
   SerialDFU.exe utility which says "Factory Restore All". Make sure to CHECK
   that checkbox when doing the firmware upgrade.

 - After the upgrade is successful, disconnect everything and cut the trace
   JP4 on the BetzTechnik WT32i Bluetooth board.


------------------------------------------------------------------------------
Modify your Arduino compiler for larger buffer size
------------------------------------------------------------------------------
For this code to work you must increase the size of the serial port buffers in
the Arduino compiler, otherwise some of the track titles will not work. The
symptom will be that you switch songs on the empeg, and the track title on the
car stereo screen does not change to the new song title every time.

To fix the issue, you must edit one of the header files in the Arduino
compiler libraries, and then you must compile and upload your sketch from your
local PC using the local Arduino compiler and uploader. Arduino offers an
online web editor for uploading code sketches, but the web editor won't work
for this project because it doesn't have the capability of changing the header
code to increase the size of the serial port buffer.

The file that you need to edit will be the same on all operating systems, but
the location of the file will be different depending on which OS you're using.
The approximate location will be somewhere around this location, but the
install location will vary:

  (install location)/hardware/arduino/avr/cores/arduino/HardwareSerial.h

On a Macintosh computer, the file is harder to find. It is located in the
following place on a Macintosh:

  /Applications/Arduino.app/Contents/Java/hardware/arduino/avr/cores/arduino/HardwareSerial.h

Or, more specifically, in the Macintosh Finder, navigate to:

  Macintosh HD -> /Applications/Arduino

Ctrl-click on the application file "Arduino" and select "Show Package
Contents". Then navigate to:

  Contents/Java/hardware/arduino/avr/cores/arduino/HardwareSerial.h

Ctrl-click on the file "HardwareSerial.h" and select "Open With", and choose
your favorite quick text editor program to edit the file with it.

Regardless of which operating system you are doing this with, once you have
HardwareSerial.h open, locate the following code section:

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

First, check to make sure this section of code is getting compiled at all.
Insert the following line immediately before the line which says
"#define SERIAL_RX_BUFFER_SIZE 64"

   #error This line of code has been reached successfully

Now, in your Arduino compiler program, attempt to verify/compile the
BlueGigaEmpeg.ino sketch. The compilation step should fail and it should
clearly show the error message that you just inserted. Now you know that the
header file is the correct header file and that your compiler is picking it up
and using it.

Now remove the error line from the header file so that it will build again.

Now, instead, edit the lines "#define SERIAL_TX_BUFFER_SIZE 64" and
"#define SERIAL_RX_BUFFER_SIZE 64" and change them to this instead:

   #define SERIAL_TX_BUFFER_SIZE 128
       ...    ...    ...
   #define SERIAL_RX_BUFFER_SIZE 256

Save the file.


------------------------------------------------------------------------------
Compile and upload the latest version of BlueGigaEmpeg.ino to the Arduino
------------------------------------------------------------------------------
Using the Arduino development software, compile and upload "BlueGigaEmpeg.ino"
to the Arduino Mega board. Use the Arduino Serial Monitor feature, set to
115200 BPS, to observe the serial port output from the Arduino. It should list
the RX and TX buffer sizes near the beginning of the output and indicate
whether they are good or not.


------------------------------------------------------------------------------
Disconnect all tuner modules from all sleds you own
------------------------------------------------------------------------------
The BlueGigaEmpeg uses the tuner module connector on the empeg Car docking
sled for a special digital audio interface. Damage may occur if you connect a
tuner module once the digital audio interface modification is made. To prevent
problems, make sure to disconnect any tuner modules from any docking sleds
that you own. After making the modification below, do not use a tuner module
any more.


------------------------------------------------------------------------------
Modify Empeg Car interior for I2S digital audio connection
------------------------------------------------------------------------------
You will be modifying the interior of your empeg Car so that there can be
three wires coming out the docking connector which carry digital audio data
("I2S" aka "IIS") and can connect to the BlueGigaEmpeg unit. This is done by
hijacking three of the wires originally used for the FM Tuner Module on its
Molex connector. This tuner module connector will instead plug into the
BlueGigaEmpeg electronic assembly, which contains the Bluetooth chip and the
Arduino.

IMPORTANT: Tuner module will no longer work after this modification. If you
have any tuner modules, disconnect all of them from any and all sleds that you
own. Damage may occur to your empeg and your tuner module if you dock your
modified empeg to any tuner module after you make this modification.

Disassemble your empeg player by carefully removing the fascia, lid and drive
tray. Refer to the empeg FAQ for disassembly instructions. You must do the
disassembly carefully so as not to damage the empeg. You should theoretically
be able to do this without disconnecting the IDE cable, but it might be easier
if you do. You should not need to remove the display board as long as you are
careful not to break any of the components sticking off the back of the
display board.

Locate 5 blank IIS pads on the front left side of the empeg motherboard.
Pads are outlined in a white silkscreen rectangle with the letters "IIS" next
to the outline. White silkscreen rectangle will have one corner of it with a
small diagonal "snip" to the corner which will indicate pad number 1 of the
set of 5 pads. This diagonal snip will be towards the back of the empeg, and
the remaining pads continue towards the front of the empeg. Pads are not
individually labeled with numbers, but understand that they are logically
numbered 1 2 3 4 5 starting from the back and going towards the front of the
empeg. The pinouts of these five pads are:

    1 = IISC  aka  SCK     aka "serial clock"
    2 = IISW  aka  WS      aka "word select"
    3 = GND   aka  Ground  same as chassis ground of empeg
    4 = IISD1 aka  SD      aka "serial data" (1 of 2)
    5 = IISD2 aka  SD      aka "serial data" (2 of 2)

Carefully solder some small jumper wires to pads 1,2, and 4, and you must keep
track of which wires are soldered to which pads.

When soldering, make sure the jumper wires and the solder are flat to the
board instead of sticking upwards. The disk drive tray gets close to that
location when mounted, so make sure they don't have a chance to contact the
tray. Take a look at Stu's instructions for his digital sound board for the
empeg, which utilizes these same three connections on the IIS pads for an
example of how to solder these three wires correctly:

       http://www.eutronix.com/media/dig-cxxxx_manual.pdf

After soldering, cover the solder points with tape or some other insulator to
prevent them from shorting out on the drive tray.

Locate the two white connectors on the back part of the empeg motherboard
which connect two groups of small colored wires to the docking connector
assembly. Locate the rightmost of the two connectors nearest the Ethernet port
on the back of the empeg. Unplug this connector, and using a tiny flat tool
such as the tip of a jeweler's screwdriver, lift up the small white tabs and
gently disengage and pull out the rightmost three pins nearest the Ethernet
connector to free up those wires. On my unit, these wires were colored, going
left to right, yellow+green, red, and brown, though your unit may differ. The
brown wire is the one on the end closest to the Ethernet connector, the red
wire is the second from the end closest to the Ethernet connector, and the
yellow+green wire is the third closest to the Ethernet connector.

You should now have three loose wires inside the empeg, coming out of the
docking connector assembly, with small pin connectors on the ends of them.
Carefully solder and shrink-tube your three jumper wires directly to these pin
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

Empeg IIS pads   Int. Empeg wires*    Int. wht Conn pos**    Sled Tuner Plug   BlueGigaEmpeg Tuner Plug    Bluetooth Chip   Usage on BlueGigaEmpeg Assembly
1 IISC            Yellow+Green         Third from end         7 Purple          7  SCK                      30  PCM_CLK      Serial Clock
2 IISW            Brown wire           Right end              2 Grey            2  WS                       29  PCM_SYNC     Word Select
4 IISD1           Red wire             Second from end        1 Pink            1  SDIN                     27  PCM_IN       Serial Data
                                                              4 Black           4  GND                          GND          Universal Ground
                                                              8 Blue            8  12vPower                                  Power to voltage regulator

*(These were the colors inside my empeg, yours may differ. These are the interior wires connecting the docking sled connector to the motherboard.)
**(Original positions of these wires on the white connector near the Ethernet plug, now disconnected from that plug.)

Finally, make sure that the variable "digitalAudio", which is a boolean flag
in the Arduino code sketch in this project, is set to "true" so that the
digital audio input on the WT32i chip is used, instead of the default analog
input.


------------------------------------------------------------------------------
Upgrade the empeg Car's hijack kernel and set "Serial Port Assignment"
------------------------------------------------------------------------------

Install the latest Hijack Kernel onto the empeg Car player if it is not
already installed (link to Hijack in "Resources", above). Make sure it is
Hijack version 524 or later.

To install the Hijack kernel, run Tony's Empeg Logo Editor, with the empeg
connected to your Windows computer's RS-232 serial port (or an RS-232 adapter)
and use the "Kernel Flash Utility" on its menu. Link to Tony's Empeg Logo
Editor is in "Resources", above. Make sure to read its documentation.

Once installed, open up the Hijack kernel settings on the player via a
longpress on the rotary encoder dial. Change "Serial port assignment" to
"Player uses serial port" and follow the instructions to reboot the player.


------------------------------------------------------------------------------
Empeg Car configuration changes
------------------------------------------------------------------------------
For best results, empeg Car player software version should be 2.00 final or
2.00 Beta 13. I have tested this with version 2.00 Beta 13. I do not if it
will work the same on 3.0 Alpha version of empeg Car software. See the empeg
Car FAQ for more information on upgrading the empeg Car player software.

Using the Emplode or Jemplode software, edit the empeg Car's config.ini as
follows.

If you are using Emplode on Windows, you must first edit the Microsoft Windows
registry before it will allow you to alter the config.ini. First, run
REGEDIT.EXE in windows and locate the following key:

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

... then restart the Emplode software. Emplode will now have a menu option
which allows you to edit the config.ini on the player.

Change the settings in config,ini as follows (adding new section headers such
as [Hijack] only if that section does not already exist):

 [Hijack]
 suppress_notify=2

 [Output]
 notify=1

 [Serial]
 car_rate=115200

Make sure to synchronize with the player after changing the config.ini
settings.


------------------------------------------------------------------------------
Set jumpers, switches, and traces
------------------------------------------------------------------------------
These instructions are for the the BetzTechnik WT32i Breakout board V2.

Cut the jumper at "J4 FTDI_+5v" but only do this AFTER successfully updating
the chip's firmware to the latest version. Cutting the jumper shuts off the
the onboard UART and prevents the UART and the Arduino pin-to-pin serial
connection from arguing with each other. This prevents errors on the serial
port which cause the chip to reboot randomly.

The BlueGigaEmpeg has an option to have a software reset line built into the
assembly. The next instruction depends on whether this reset line is being
used or not. This behavior is controlled by the flag performResetLinePhysical,
located in BlueGigaEmpeg.ino. Set this flag to true to use the reset line, and
remember to re-upload the sketch to the Arduino if you have changed the value.

If reset line is used:
"Smd_2_pole_switch" set to the right or down position, to put it into battery
power mode. The red LED on the breakout board will blink slowly and steadily
when power is applied, until the Arduino sketch boots up and uses the reset
line to turn on the Bluetooth chip. After the Bluetooth chip is started, then
the red LED on the breakout board will blink randomly.

If reset line is not used:
"Smd_2_pole_switch" set to the left or up position, to put it into always-on
mode. The red LED on the breakout board will blink randomly when power is
applied. Symptoms of this switch being set wrong will be: LED on the breakout
board blinks steady when power is applied instead of randomly, and there is no
serial port communication, and you need to press the reset button on the board
to start up the module.


------------------------------------------------------------------------------
Connect external hardware connections
------------------------------------------------------------------------------
BetzTechnik WT32i Breakout board V2, plug into BlueGigaEmpeg circuit board.

Arduino Mega board, plug into BlueGigaEmpeg circuit board.

RS-232 serial port from the empeg Car player docking sled, connect to the
serial port on the BlueGigaEmpeg module.

Tuner module connector from the empeg Car player docking sled, connect to the
tuner connector on the BlueGigaEmpeg module. This plug carries 12v power to
the BlueGigaEmpeg module and also carries I2S audio data.

Power the empeg using 12 volts DC through its car docking sled. During bench
testing in the house, you can use the 12v AC adapter power supply that comes
with the empeg. The BlueGigaEmpeg module will get its power from the empeg's
tuner module connector, and it should work equally well with either sled
power or AC adapter power.

Debugging console to Arduino - Only used for uploading the latest Arduino code
to the BlueGigaEmpeg module, or for monitoring the serial output and/or
debugging the connection. Use a standard USB cable connected from a computer
to the Arduino USB port if you need to do this.


Notes:

Power to Arduino: For debugging mode, the USB cable from the computer can
power the Arduino and the Bluetooth board. When connected to the empeg, power
comes from the tuner connector. Both can be connected at the same time, though
I do not recommend leaving them connected that way for long periods of time.

Startup order: If using the USB cable for debugging mode, the order of
connecting the cables and applying power are important. See the section titled
"Debug Bluetooth Connection if needed" for more details.

Arduino power connector: The Arduino has a 5.5mm barrel plug connector for
power. Do not try to use direct 12v from the car to power this plug. The
Arduino can't actually handle direct 12v power from the car, despite what its
specs might say. Car 12v power can briefly fluctuate much higher than 12v.
Instead, apply 12v power to the empeg and let the empeg supply 12 volts to the
BlueGigaEmpeg module through the tuner connector, which will go into a Pololu
12v-to-5v step-down power regulator located on the BlueGigaEmpeg module.


------------------------------------------------------------------------------
Set Bluetooth PIN code if needed (most likely not needed)
------------------------------------------------------------------------------
BlueGigaEmpeg does not usually require a PIN code to successfully pair. Before
trying to modify the PIN code, simply try pairing first.

Some stereos will prompt with a set of digits on the screen and ask you to
confirm those digits on the Bluetooth device with a Yes/No prompt. This is not
the same thing as a PIN code. In those cases, the BlueGigaEmpeg will try to
automatically answer this prompt with a "yes" answer so not user interaction
will be needed.

In rare cases where a special code is is needed, there is a place in the
"BlueGigaEmpeg.ino" source code file where you can modify the PIN to something
other than "0000". Find the line in the code that defines the variable
"btPinCodeString". That line contains the "0000" default PIN code. Change it
to your correct PIN code for your stereo and re-upload the sketch to the
Arduino. It can accept PIN codes up to 16 digits long.


------------------------------------------------------------------------------
Apply power and pair Bluetooth
------------------------------------------------------------------------------
Apply power to the empeg, and the BlueGigaEmpeg module will receive power. 

The first time you use the BlueGigaEmpeg, you will need to pair it with your
Bluetooth car stereo. The BlueGigaEmpeg has a RESET/PAIR button for this
purpose. After you have successfully paired with your car stereo, you should
not need to pair it again, it should automatically reconnect each time you
start your car.

When you press the RESET/PAIR button on the BlueGigaEmpeg, it does the
following:

 - Erases all previous Bluetooth pairings from the BlueGigaEmpeg.
 
 - Resets the module to default settings as defined in the Arduino code.

 - Looks for Bluetooth devices to pair with for about 30 seconds and pairs
   with the first one that it sees.

 - The blue RESET/PAIR LED is lit during this pairing mode, and will turn
   off once pairing is complete or once the 30 seconds has run out.

NOTE: Though this is counterintuitive: Sometimes you don't press that
RESET/PAIR button at all. Sometimes, instead, you do the pairing when the
RESET/PAIR LED is *not* lit.

There are some stereos which need you to initiate pairing solely on the car
stereo's touch screen. Mine is one of these types of stereos. In my case, for
initial pairing with my car stereo the first time, I don't press the
RESET/PAIR button at all. I just go to my car's touch screen, tell it to pair
with the Bluetooth device named "empeg Car", and it works. After that, it
reconnects automatically each time I get in the car and start it up (though it
takes a moment to connect).

Some Bluetooth stereos *do* need you to press that RESET/PAIR button on the
BlueGigaEmpeg, which should light the LED for ~30 seconds, during which time
you should also put your stereo into pair mode.

If one method doesn't work for you, try the other method. For example this
sequence will work for many stereos and headsets:

 - Press the RESET/PAIR button, and while the LED is still lit, then
   initiate pairing on your Bluetooth stereo via its pairing feature.

 - If that doesn't work, then wait for the RESET/PAIR LED on the
   BlueGigaEmpeg to go dark, and then, without touching the RESET/PAIR
   button, initiate pairing from your stereo via its pairing feature.

Car and empeg should be playing nicely together now, assuming everything else
is working correctly. Audio from the empeg comes out of your car stereo's
speakers, the stereo's track change controls will change tracks on the empeg,
and the track titles will appear on the car stereo's screen.


------------------------------------------------------------------------------
Modify empeg's power connection to car if needed
------------------------------------------------------------------------------
I might recommend that, for this installation, you wire up the empeg to your
car differently than it would otherwise would be.

This design is intended to be used in such a way so that the empeg is not the
primary stereo in the system. The empeg becomes one of the Bluetooth inputs
into a modern car stereo. So the empeg's sleep mode, the empeg's "memory"
power connection, and the way this Bluetooth module gets its power from the
empeg tuner connector, all combine to cause some interesting problems with
power state transitions. At least they did in my car, your mileage may vary.

I found that if I connected the empeg to my car via the regular method (i.e.
the ignition wire to the orange ignition wire on the sled, and constant 12v
power to the yellow memory wire on the sled) then there were certain unusual
states that it could get into, where I might sometimes shut off my car but the
empeg would come back up out of sleep mode and play tracks silently to an
unconnected Bluetooth module. I was worried that this might drain my car's
battery.

If this happens to you, then I recommend connecting power to the empeg like
this instead:

- Yellow "memory" wire on Empeg: Connect to car power 12v accessory power.
- Orange "ignition" wire on Empeg: Connect that same 12v accessory power.
- Do not connect your car's constant 12v power to any part of the empeg.
- Connect your car's illumination wire to the empeg's "lights on" wire.
- Connect ground to ground of course.
- Tuner module and serial plug connectors go to the BlueGigaEmpeg module.

That should be all you need. No audio or amplifier connections because the
Bluetooth takes care of that now. No cellphone mute wire because the Bluetooth
takes care of that now. No amp remote wire because there is no amp connected
directly to the empeg. No aux connection to the empeg, etc.

In this situation, the empeg does not have a 12v constant power concept at all
and will not go into sleep mode when you turn off the ignition. However it,
and the Bluetooth, will always power on and off at the same time as the car's
headunit, and nothing will get confused. You might lose the clock/date/time
information on the empeg sometimes, but your modern car likely has a perfectly
functional clock of its own.

Finally, you'll notice that the empeg sled connector only supplies power if
the empeg is awake. If you put the empeg into Sleep mode via a long press on
the top button, it will go to sleep but that also means the Arduino+Bluetooth
assembly stops receiving power. This is actually good: you want to be able to
turn the Bluetooth on and off and reboot it sometimes, and it's easy to do
that just by sleeping and waking the player by hand. In my case I leave the
player awake all of the time, so that when I shut off the car ignition, the
player fully shuts down, and then when I start my car again, the empeg starts
up in Awake mode and immediately supplies power to the Bluetooth assembly
which then autoconnects to the car stereo head unit.


------------------------------------------------------------------------------
Debug Bluetooth connection if needed
------------------------------------------------------------------------------
Everything hopefully will work perfectly, but there may be bugs in the Arduino
code because this was not tested with a wide range of Bluetooth gear.

If you need to debug the connection, here are some helpful tips.

Power up sequence for Arduino debugging:

Your mileage may vary, but on my system there is an interesting trick with
debugging via the Arduino USB cable and the Arduino Serial Monitor.

When everything is connected together (empeg, Arduino, Bluetooth), the
assembly and all devices which are part of the assembly will get their power
off of the 12v power coming off the tuner connector on the empeg sled. This
means that if you plug all this stuff in, it's already powered by the time you
try to attach your USB debug cable to the Arduino.

On my computer system, if the Arduino is already externally powered when I
connect my debug cable, then my computer cannot find its USB UART and is
unable to connect to the debug port.

So to successfully debug via the Arduino USB debug cable and use the Arduino
Serial Monitor program, I must connect the USB cable FIRST, before applying
power to the empeg. Meaning if I am debugging in the car, I should connect the
USB cable first and then turn on the ignition, that way the Serial Monitor
program can find a port to connect to. If I am debugging on the test bench,
I should connect the USB cable first, then connect the AC adapter to the
empeg.

When using the Arduino Serial Monitor program, set it to 115200 baud, with
either "Newline" or "Both NL and CR" as line endings. If you are using a
different serial terminal program, set it to 115200 8n1 and turn on local
echo.

Variables in the Arduino sketch:

There are several flag variable in the BlueGigaEmpeg.ino sketch file, defined
at the top of the code, which can be modified if you need them when debugging.
Change them as you see fit, and re-upload the sketch to the Arduino. Remember
to change them back to their default values when you are done debugging, as
some of them can slow down the responses on the BlueGigaEmpeg and cause the
Bluetooth communication to miss things.

Refer to the code comments accompanying these flags to understand them:
  EmpegSendCommandDebug
  displayEmpegSerial
  logLineByLine
  outputMillis
  displayTracksOnSerial

Useful debugging commands on BlueGiga module:

Typing any of these into the Arduino Serial Monitor should send these commands
to the Bluetooth module directly. Many will echo a response. Module will say
SYNTAX ERROR if there is a problem with any command.

  SET
  Displays Bluetooth module's current settings.

  SET BT PAIR
  Displays Bluetooth module's current list of paired devices.

  SET BT PAIR *
  Deletes all saved Bluetooth pairings on the Bluetooth module.

  RESET
  Reboots the Bluetooth module, saving current settings. Does not erase any
  existing saved bluetooth pairings nor change any settings.

  SET RESET
  Resets the Bluetooth chip itself to its factory defaults. Does not remove
  existing pairings, those are still kept. Note: you should restart the
  Arduino after using "SET RESET" in order to have it apply its customized
  default settings to the Bluetooth chip again.

A note about response ordering in the debug console:

Sometimes the responses from the Bluetooth module do not show up on the screen
immediately, because the main input loop for the Arduino code might not have
caught up and read all the responses from the serial port yet. So, for
example, a bad command might be issued several lines back, but the "SYNTAX
ERROR" response doesn't appear until later. So don't always assume that any
two command/response pairs are absolutely connected just because they appear
next to each other on the debug console screen.


------------------------------------------------------------------------------
Hardware interface information and notes (internal board connections)
------------------------------------------------------------------------------
There will be a BlueGigaEmpeg interface circuit board made available. It will
implement everything described below. The information below is just for my
personal reference and you should not need to do any of the connections below
yourself, unless you are developing your own interface board.

Arduino must be the "Mega" model with three hardware serial ports built in. I
tried it with an "Uno" model using software serial ports, and it had problems
where characters sometimes got dropped, and it could not keep up with the data
rates that I wanted.

Arduino and RS-232 port, critical connections:

Arduino serial port 1 (Tx1/Rx1) connects to the empeg's RS-232 port, but it
must go through an RS-232 converter circuit based on a MAX232 chip. This is
necessary because you can't connect an actual RS-232 cable directly to an
Arduino, the voltage and signaling are different. So it must go through the
MAX232. The full schematic for this circuit is linked in "Resources" above,
and the pinouts are also listed below.

The RX/TX crossover configurations for the MAX232 and the RS-232 plug on the
BlueGigaEmpeg can be confusing. Partly because the empeg Car sled serial port
plug is already wired into a crossover configuration opposite of the way that
the one built into the player body is wired. So this configuration will only
work for the sled serial port connector, not the player body serial port
connector. I am using a TI MAX232E for this implementation. The connections
for the MAX232 are as follows:

    MAX232 Pin 1 and 3 aka C1+/C1- - Bridge with a 2.2uf ceramic capacitor
    MAX232 Pin 4 and 5 aka C2+/C2- - Bridge with a 2.2uf ceramic capacitor
    MAX232 Pin 2 aka V+ - Connect to Pololu 5v via a 2.2uf ceramic capacitor
    MAX232 Pin 6 aka V- - Connect to GND via a 2.2uf ceramic capacitor
    MAX232 Pin 11 aka TTL-I1 - Connect to Arduino Mega Board 18 aka Tx1 
    MAX232 Pin 12 aka TTL-O1 - Connect to Arduino Mega Board 19 axa Rx1
    MAX232 Pin 13 aka 232-I1 - Connect to RS-232 plug pin 3 aka Tx
    MAX232 Pin 14 aka 232-O1 - Connect to RS-232 plug pin 2 aka Rx
    MAX232 Pin 15 aka GND - Connect directly to GND
    MAX232 Pin 16 aka VCC - Connect directly to Pololu 5v output.
    MAX232 Pin 16 aka VCC - Also connect to 0.1uf ceramic capacitor which
                            then connects to GND. (Pin 16 is two connections,
                            one to 5V and then also to the smoothing capacitor
                            which bridges across 5v and GND. Place this cap as
                            physically close to pin 16 as possible.)

Empeg tuner connector is used to supply power to the Arduino and the rest of
the assembly. Blue wire on tuner connector connects to the voltage input pin
on the 12v-to-5v Pololu step-down transformer power supply, and the black wire
on the tuner connector connects to the ground input pin on the step-down power
supply.

Arduino "5v" pin is connected to the 5v output rail from the Pololu 12v-to-5v
step-down transformer power supply circuit.

Grounding: Multiple Arduino GND pins connected to the output ground rail of
the 5v power supply.

Arduino and Pair mode indicator LED: Arduino pin 50 digital I/O pin, connected
to +LED through resistor. Current of resistor determined by online LED
resistor calculator and LED values. Then -LED connect to GND. Example: If
using a blue LED with a 3.2v forward voltage and 20ma current, and the Arduino
analog power supply from the analog pins will be 5 volts, then use a 100 ohm
resistor for this value. Online resistor current calculator:
http://led.linear1.org/1led.wiz

Arduino and Button - Arduino pin 52 digital I/O pin, connected to one of the
ground legs of button. Other ground leg of button goes through 10k pulldown
resistor to ground. One of the + legs of the button connects to +5v. Follow
examples on the Internet of how to implement a simple temporary pushbutton on
an Arduino: https://www.arduino.cc/en/Tutorial/Button

BlueGiga Bluetooth WT32i chip+board, critical connections:

Bluetooth chip+board "VBus" or "5v" power pin connected to 5v rail from the
step-down power supply.

Bluetooth chip+Board "Gnd" pin connected to the ground rail. Additional GND
pins are good too, to make sure solid ground is achieved.

Bluetooth chip+board serial port "RX/TX" pins connected to the "TX2/RX2" pins
(serial port 2) of Arduino. Note: This is a crossover connection, i.e., RX
from the Arduino is connected to TX on the dev board and vice-versa. Since
this is at TTL level, the Arduino and the Bluetooth chip can connect almost
directly with wires or traces instead of needing to go through a MAX232
circuit. However, the TX wire from the Arduino is running at 5v TTL, and the
BlueGiga chip runs at 3v (and our assembly will actually have it nominally at
2.5v), you must run the Arduino Tx2 output through a simple 50% voltage
divider to step 5v TTL from Arduino down to 2.5v for the BlueGiga chip. This
is a simple circuit with just two 10k resistors. The voltage divider is only
needed on the line that connects Arduino TX2 to the Bluetooth chip's Rx line,
the other line can be a direct connection. Schematic:

    ARDUINO RX2-----------------------------WT32i Tx

    ARDUINO TX2-----VVVVV----+----VVVVV-----GND
                   10Kohm    |    10Kohm
                             |
                             +--------------WT32i Rx

Bluetooth chip+Board three I2S pins PCM_CLK, PCM_SYNC, PCM_IN connected to
empeg IISC, IISW, IISD1 via a special modification to the empeg the tuner
connector, as described in the section of this document titled "Modify Empeg
Car interior for I2S digital audio connection".

Each one of the three I2S pins will need to be coming in through a set of
resistors arranged in a voltage divider configuration, with a 4.7k and a 10k
resistor for each one of the three lines. Example of one line (repeat three
times for each of the I2S connections):

    EMPEG IISC-----VVVVV----+----VVVVV-----GND
                  4.7Kohm   |    10Kohm                    (3x) 
                            |
                            +--------------WT32i PCM_CLK

Reset Line: Bluetooth chip+board RST (reset) pin connected to Pin 51 of the
Arduino board. This is working for Mark Lord, but my chip fried immediately
after trying this without voltage protection, so I consider this to be a risk
if connecting directly. So instead, run it through a 50% voltage divider and a
diode before it goes into the RST pin on the Bluetooth. Example:

    ARDUINO PIN 51-----VVVVV----+----VVVVV-----GND
                      10Kohm    |    10Kohm
                                |
                                |
                                +------>|------WT32i RST
                                   Diode 1N914


