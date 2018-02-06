BlueGigaEmpeg
==============================================================================
by Tony Fabris

https://github.com/tfabris/BlueGigaEmpeg

A project to use a Silicon Labs BlueGiga WT32i Bluetooth chip, combined with
an Arduino Mega board, to act as an interface between an empeg Car Mk2/2a MP3
player and a modern Bluetooth-equipped car.

------------------------------------------------------------------------------
The empeg Car (small "e") player, also sold as the Rio Car, is an amazing
in-dash MP3 player made from 1999-2001 whose features are still, nearly 20
years later, better than any other in-car MP3 solution available. However, the
empeg Car didn't have Bluetooth since it wasn't in common use yet at the time.

The purpose of this BlueGigaEmpeg interface is twofold:

1. Bluetooth A2DP connection allows high quality stereo audio from the empeg
   Car player to play on the modern car stereo speakers without needing an AUX
   input on the car stereo. The Bluetooth connection is useful because many
   cars no longer have an AUX input available, yet most modern cars now have
   Bluetooth available.

2. Allow empeg Car to receive Bluetooth AVRCP commands such as "next",
   "previous", "pause", etc. from the Bluetooth connection, thus allowing the
   car stereo's touchscreen and steering wheel controls to be able to change
   tracks on the empeg. In addition, the same AVRCP channel allows track title
   and artist information to be displayed on the car stereo's touchscreen.

Some modification of the empeg Car is required in order for this to work. Make
sure to perform all modifications listed in this document for this to work
correctly. Follow the "Checklist" section of this document to make sure all
steps are performed.

NOTE: At the time of this writing, I only tested this on a very limited set of
Bluetooth gear. I have tested it on:
- My Honda Accord 2017 factory stereo.
- Kenwood Bluetooth-equipped car stereo in my roommate's car.
- Plantronics Voyager Edge Bluetooth headset.
- Onkyo home stereo with Bluetooth input.

There may be differences in Bluetooth implementation on other audio gear, and
so there might be bugs using this on your car stereo. This project is open
source on GitHub so that I can accept bug reports and code fixes from people
with other brands of Bluetooth gear.


Acknowledgments
==============================================================================
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


Checklist
==============================================================================
Implementing this requires some manual labor. Make sure to go through each
step in this checklist. Each step is detailed in its own section, below.

- [Prerequisites](#prerequisites)
- [Modify your Arduino compiler for larger buffer size](#modify-your-arduino-compiler-for-larger-buffer-size)
- [Compile and upload the latest version of BlueGigaEmpeg.ino to the Arduino](#compile-and-upload-the-latest-version-of-bluegigaempegino-to-the-arduino)
- [Disconnect all tuner modules from all sleds you own](#disconnect-all-tuner-modules-from-all-sleds-you-own)
- [Modify empeg Car interior for I2S digital audio connection](#modify-empeg-car-interior-for-i2s-digital-audio-connection)
- [Empeg Car configuration changes](#empeg-car-configuration-changes)
- [Upgrade the empeg Car's Hijack kernel and set "Serial Port Assignment"](#upgrade-the-empeg-cars-hijack-kernel-and-set-serial-port-assignment)
- [Connect external hardware connections](#connect-external-hardware-connections)
- [Apply power and pair Bluetooth](#apply-power-and-pair-bluetooth)
- [Set Bluetooth PIN code if needed (most likely not needed)](#set-bluetooth-pin-code-if-needed-most-likely-not-needed)
- [Test AVRCP behavior and set serial port crossover jumpers if needed](#test-avrcp-behavior-and-set-serial-port-crossover-jumpers-if-needed)
- [Modify empeg's power connection to car if needed](#modify-empegs-power-connection-to-car-if-needed)
- [Debug Bluetooth connection if needed](#debug-bluetooth-connection-if-needed)


Prerequisites
==============================================================================

Make sure you have all of these things before working with the BlueGigaEmpeg:

Purchase:
 - Empeg Mk2 or Rio Car player   http://empegbbs.com/ubbthreads.php/forums/11/1/For_Sale
 - BlueGigaEmpeg Interface       tfabris@gmail.com

Download software (Windows PC or Windows VM required for some of these items):
 - Hijack Kernel for empeg       http://empeg-hijack.sourceforge.net
 - Tony's Empeg Logo Editor      http://empegbbs.com/ubbthreads.php/ubb/download/Number/7067/filename/logoedit17.zip
 - Arduino standalone IDE        https://www.arduino.cc/en/Main/Software
 - BlueGigaEmpeg sketch          https://github.com/tfabris/BlueGigaEmpeg

May be needed later if debugging the WT32i chip directly:
 - Windows FTDI USB driver       http://www.ftdichip.com/FTDrivers.htm
 - WT32i Firmware Upgrade        https://www.silabs.com/documents/login/software/iWRAP-Firmware-Releases.zip

###  Important:

Do not connect the BlueGigaEmpeg module to the Empeg until after completing
the step in this document titled "Modify Empeg Car interior for I2S digital
audio connection". Also, once the I2S modification has been completed, do
not connect the Empeg to any tuner module. Damage may occur if these
instructions are not followed.

You must already be comfortable with safely dismantling and repairing your
empeg Car in order to safely make this modification. I take no responsibility
for damage incurred while you are dismantling your empeg Car player.

Be sure you are capable of safely making the internal modification to the
empeg Car player as described in the section of this document titled "Modify
Empeg Car interior for I2S digital audio connection". 


Modify your Arduino compiler for larger buffer size
==============================================================================
To update to the latest BlueGigaEmpeg firmware on the Arduino, you must
increase the size of the serial port buffers in the Arduino compiler,
otherwise there will be intermittent errors such as the track titles will
sometimes not work. The symptom will be that you switch songs on the empeg,
and the track title on the car stereo screen does not correctly change to the
new song title every time.

To fix the issue, you must edit one of the header files in the Arduino
compiler libraries, and then you must compile and upload your sketch from your
local PC to the Arduino board using the standalone version of the Arduino IDE.
The link to the standalone Arduino IDE is in the "Prerequisites" section of
this document. Make sure to download the standalone Arduino IDE; do not use
the Arduino web editor. The web editor won't work for this project because it
doesn't have the capability of changing the header code to increase the size
of the serial port buffer.

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
HardwareSerial.h open, locate the following code lines:

             #if !defined(SERIAL_TX_BUFFER_SIZE)
             #if ((RAMEND - RAMSTART) < 1023)
             #define SERIAL_TX_BUFFER_SIZE 16
             #else
    -->      #define SERIAL_TX_BUFFER_SIZE 64
             #endif
             #endif
             #if !defined(SERIAL_RX_BUFFER_SIZE)
             #if ((RAMEND - RAMSTART) < 1023)
             #define SERIAL_RX_BUFFER_SIZE 16
             #else
    -->      #define SERIAL_RX_BUFFER_SIZE 64
             #endif
             #endif

Now edit the lines indicated above, and change them to this instead:

       #define SERIAL_TX_BUFFER_SIZE 128
       
       #define SERIAL_RX_BUFFER_SIZE 256

In other words, you are making the larger of the two possible transmit buffer
sizes even larger (changing 64 to 128) and the larger of the two possible
receive buffers even larger (changing 64 to 256).

Note: Leave the "SERIAL_xx_BUFFER_SIZE 16" lines alone. Only modify the ones
that were originally set to "64" and increase them as described above.

Save the file.


Compile and upload the latest version of BlueGigaEmpeg.ino to the Arduino
==============================================================================
Uploading code to the Arduino may be done with the BlueGigaEmpeg module fully
assembled. However do not plug the BlueGigaEmpeg module into the empeg Car
tuner connector until after the I2S modification has been completed, as
described elsewhere in this document.

Obtain the BlueGigaEmpeg Arduino sketch from GitHub, linked in the
"Prerequisites" section of this document. Unzip it onto a folder named
"BlueGigaEmpeg" on your computer's hard disk.

Connect the USB cable from the computer to the Arduino USB connector. This is
the USB connector exposed on the end of the BlueGigaEmpeg enclosure. The
correct USB cable was supplied with the BlueGigaEmpeg, it is an old style
A-to-B connector.

Using the Arduino IDE, open the BlueGigaEmpeg.ino project and compile and
upload it to the Arduino Mega board.

Use the Arduino Serial Monitor feature (built into the Arduino IDE), set to
115200 BPS, and observe the serial port output from the Arduino. It should
list the RX and TX buffer sizes near the beginning of the output and indicate
whether they are good or not.


Disconnect all tuner modules from all sleds you own
==============================================================================
The BlueGigaEmpeg re-purposes the tuner module connector on the empeg Car
docking sled for a special digital audio interface. Damage may occur if you
connect a tuner module once the digital audio interface modification is made.

To prevent problems, make sure to disconnect any tuner modules from any
docking sleds that you own. After making the I2S wiring modification to the
empeg, do not use a tuner module any more.

Do not connect the BlueGigaEmpeg module to the empeg Car sled until the I2S
modifications have been completed. Those modifications are described in the
next step, below.


Modify empeg Car interior for I2S digital audio connection
==============================================================================
To perform this step, you must be comfortable with disassembling the empeg Car
player and removing the disk drive tray in such a way as to not cause damage
to the player. In particular there is risk to the components on the back side
of the display board, and to the IDE header connector on the empeg
motherboard. Refer to the empeg FAQ for details on how to disassemble the
player.

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

Refer to the annotated photograph in this repository named "I2S Wiring
Modification Photo.jpg" to help understand the wiring instructions below.

Photo:
![Wiring Modification](https://github.com/tfabris/BlueGigaEmpeg/blob/master/I2S%20Wiring%20Modification%20Photo.jpg)

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

Carefully solder three supplied jumper wires to pads 1,2, and 4, and you must
keep track of which wires are soldered to which pads.

When soldering, make sure the jumper wires and the solder joints are flat to
the board instead of sticking upwards. The disk drive tray gets close to that
location when mounted, so make sure they don't have a chance to contact the
tray. After soldering, cover the solder points with tape or some other
insulator to prevent them from shorting out on the drive tray.

Locate the two white connectors on the back part of the empeg motherboard
which connect two groups of small colored wires to the docking connector
assembly. Locate the rightmost of the two connectors nearest the Ethernet port
on the back of the empeg. Unplug this connector, and using a tiny flat tool
such as the tip of a jeweler's screwdriver, lift up the small white tabs and
gently disengage and pull out the pins. Pull out only the rightmost three pins
nearest the Ethernet connector to free up those three wires. These wires are
colored, going from left to right, yellow+green, red, and brown. The brown
wire is the one on the right end closest to the Ethernet connector, the red
wire is the second from the end, and the yellow+green wire is the third from
the end.

With the three wires disconnected from the white connector, plug the white
connector back into place with the remainder of its wires still connected.

You should now have three wires inside the empeg, each with one end still
connected to the docking connector assembly, and their other ends loose with
small pin connectors on the ends of them. Carefully solder and shrink-tube
your three jumper wires directly to these pin connectors in the exact order
described below. After you have done this, your empeg's IIS pads will, when
docked, now be carried out the back of the empeg, via the docking sled, into
the tuner module Molex connector. Solder your jumper wires to the empeg
interior wires in this order:

    IIS Pad 1 (IISC)  jumpered to Yellow+Green wire, originally third from end.
    IIS Pad 2 (IISW)  jumpered to Brown wire, originally the end wire.
    IIS Pad 4 (IISD1) jumpered to Red wire, originally second from end.

Make sure that your jumper wires are carefully tucked down around the left
side of the empeg and out of the way of the disk drive tray assembly as shown
in the photo linked above.

Glue down your jumper wires near the I2S pads so that they do not wiggle and
rip away the I2S pads. Some empeg Car units will have a chip at that location
that you can glue onto, others will have a blank area of the PCB in that
location that you can glue onto. Use RTV silicone or cyanoacrylate superglue
to glue them down. Use a small amount and make sure the blob of glue does not
"stick up" since the disk drive tray gets very close to that location.

After the glue is fully dry, carefully reassemble the player.

Final wiring positions and colors:

| Empeg IIS pads |  Int. Empeg wires * |  Int. wht Conn pos **  | Sled Tuner Plug   |
|----------------|---------------------|------------------------|-------------------|
|    1 IISC      |      Yellow+Green   |      Third from end    |     7 Purple      |
|    2 IISW      |      Brown wire     |      Right end         |     2 Grey        |
|    4 IISD1     |      Red wire       |      Second from end   |     1 Pink        |
|                |                     |                        |     4 Black       |
|                |                     |                        |     8 Blue        |
    
|   BlueGigaEmpeg Tuner Plug |   Bluetooth Chip |  Usage                            |
|----------------------------|------------------|-----------------------------------|
|    7  SCK                  |    30  PCM_CLK   |   Serial Clock                    |
|    2  WS                   |    29  PCM_SYNC  |   Word Select aka Sync            |
|    1  SDIN                 |    27  PCM_IN    |   Serial Data aka PCM Audio Data  |
|    4  GND                  |        GND       |   Universal Ground                |
|    8  12vPower             |                  |   Power to voltage regulator      |

    *  (These are the interior wires connecting the docking sled connector to the
       motherboard.)
    
    ** (Original positions of these wires on the white connector near the Ethernet
       plug, now disconnected from that connector.)


Empeg Car configuration changes
==============================================================================
For best results, empeg Car player software version should be 2.00 final or
2.00 Beta 13. I have tested this with version 2.00 Beta 13. I do not know if
it will work on 3.0 Alpha version of empeg Car software, but theoretically it
should work. See the empeg Car FAQ for more information on upgrading the empeg
Car player software.

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
                  (Inside the "Settings" key, create a new DWORD value named:)
                    allow_edit_config
                      (with a value of)
                        1              

Then restart the Emplode software. Emplode will now have a menu option
which allows you to edit the config.ini on the player.

Change the settings in config,ini as follows (adding new section headers such
as [Hijack] only if that section does not already exist):

    [hijack]
    suppress_notify=2
    
    [output]
    notify=1
    
    [serial]
    car_rate=115200

These settings are case-sensitive. Make sure to synchronize with the player
after changing the config.ini settings.


Upgrade the empeg Car's Hijack kernel and set "Serial Port Assignment"
==============================================================================
Install the latest Hijack Kernel onto the empeg Car player if it is not
already installed. Link to the Hijack kernel is found in the "Prerequisites"
section of this document. Make sure it is Hijack version 524 or later.

In order to install the latest Hijack Kernel onto the empeg Car, it must be
connected to your Windows computer's RS-232 serial port, or to a USB-to-RS-232
adapter cable if your computer does not have an RS-232 serial port. Make sure
the serial port drivers are installed and that you can see the empeg bootup
messages correctly through a serial terminal program.

To install the Hijack kernel, run Tony's Empeg Logo Editor. Link to the Logo
Editor program is found in the "Prerequisites" section of this document. On
the File menu of the logo editor program, select "Kernel Flash Utility" and
follow the instructions.

Once Hijack is installed, open up the Hijack kernel settings on the player via
a long-press on the player's rotary encoder dial. Change "Serial port
assignment" to "Player uses serial port" and follow the on-screen instructions
to reboot the player.


Connect external hardware connections
==============================================================================
Do not connect the BlueGigaEmpeg module to the empeg car's tuner connector
unless the I2S wiring modification, described elsewhere in this document, has
been completed.

If the I2S wiring modification has been completed, connect the tuner module
connector from the empeg Car player docking sled to the tuner connector on the
BlueGigaEmpeg module. This plug carries 12v power to the BlueGigaEmpeg module
and also carries I2S audio data.

Connect the RS-232 serial port from the empeg Car player docking sled to the
serial port on the BlueGigaEmpeg module. This plug carries the play/pause/next
control commands from the BlueGigaEmpeg to the empeg Car player software, and
also carries the track metadata (title/artist/etc) from the empeg Car player
software to the BlueGigaEmpeg module.

Power the empeg using 12 volts DC through its car docking sled. Alternatively,
during bench testing in the house, you can connect the empeg's 12v AC adapter
power supply to the empeg's AC adapter input. The BlueGigaEmpeg module will
get its power from the empeg's tuner module connector, and it should work
equally well with either sled power or AC adapter power.

The USB connector on the Arduino, which is accessible from the outside of the
BlueGigaEmpeg module enclosure, is normally left disconnected. It is only used
for debugging, and for uploading the latest Arduino code to the BlueGigaEmpeg
module. See the section titled "Debug Bluetooth Connection if needed" for more
details on how to use the connector for debugging.

Arduino power connector: The Arduino has a 5.5mm barrel plug connector for
power. This connector is not exposed on the outside of the BlueGigaEmpeg
enclosure. Do not use this connector. Power the BlueGigaEmpeg module only
from the tuner plug via the empeg itself.


Apply power and pair Bluetooth
==============================================================================
Apply power to the empeg, and the BlueGigaEmpeg module will receive power. 
Make sure the empeg is not in "sleep" mode, power to the tuner module
connector is turned off in sleep mode.

The first time you use the BlueGigaEmpeg, you will need to pair it with your
Bluetooth car stereo. You might also wish to re-pair again after editing the
BlueGigaEmpeg firmware or the WT32i firmware. The BlueGigaEmpeg has a
RESET/PAIR button for this purpose.

###  Procedure:

- If your car stereo contains a feature that lists existing Bluetooth
  pairings, and if there is an existing entry for "empeg Car" from an earlier
  pairing, make sure to delete that earlier pairing because its security key
  will be different when you press the RESET/PAIR button in the next step.

- Press the RESET/PAIR button on the BlueGigaEmpeg. Its LED should light up
  blue for about 30 seconds or so.

- On the car stereo, initiate pairing mode while the blue LED on the
  BlueGigaEmpeg module is lit. Look for the device "empeg Car" and pair it.

- If the pairing doesn't work the first time, also try initiating pairing from
  the car stereo AFTER the blue LED on the BlueGigaEmpeg module goes out.

Car and empeg should be playing nicely together now, assuming everything else
is working correctly. Audio from the empeg comes out of your car stereo's
speakers, the stereo's track change controls will change tracks on the empeg,
and the track titles will appear on the car stereo's screen. If there is audio
but the track change controls and track titles do not work, see the section
titled "Test AVRCP behavior and set serial port crossover jumpers if needed",
located elsewhere in this document.

After you have successfully paired with your car stereo, you should not need
to pair it again each time you get in the car. It should automatically
reconnect each time you start your car.

Some car stereos will let you pair your phone and the BlueGigaEmpeg at the
same time. My Honda allows me to do this. When pairing, the touch screen lets
me choose whether the paired device is a "phone" device or a "music" device. I
choose to pair my phone as a phone, and the empeg Car as a music device. Then,
each time I start my car, they both pair up correctly. I can use the steering
wheel controls to initiate a speakerphone call which automatically mutes and
pauses the music from the empeg.

###  Technical details and troubleshooting:

When you press the RESET/PAIR button on the BlueGigaEmpeg, it does the
following:

 - Erases all previous Bluetooth pairings from the BlueGigaEmpeg.
 
 - Resets the module to default settings as defined in the Arduino code.

 - Looks for Bluetooth devices to pair with for about 30 seconds, and pairs
   automatically with the first one that it sees.

 - The blue RESET/PAIR LED is lit during this pairing mode, and will turn
   off once either pairing is complete or once the 30 seconds has run out.

Though this is counterintuitive: Sometimes you don't press that RESET/PAIR
button at all. Sometimes, instead, you do the pairing when the RESET/PAIR LED
is not lit. There are some stereos which need you to initiate pairing solely
on the car stereo's touch screen. My Honda is one of these types of stereos.
In my case, for initial pairing with my car stereo the first time, I don't
press the RESET/PAIR button at all except for the purpose of erasing previous
pairings. I just go to my car's touch screen, tell it to pair with the
Bluetooth device named "empeg Car", and it works. After that, it reconnects
automatically each time I get in the car and start it up.

Some Bluetooth stereos still need you to press that RESET/PAIR button on the
BlueGigaEmpeg, which should light the LED for about 30 seconds, during which
time you should also put your stereo into pair mode.

If one method doesn't work for you, try the other method. For example this
sequence will work for many stereos and headsets:

 - Press the RESET/PAIR button, and while the LED is still lit, then
   initiate pairing on your Bluetooth stereo via its pairing feature.

 - If that doesn't work, then wait for the RESET/PAIR LED on the
   BlueGigaEmpeg to go dark, and then, without touching the RESET/PAIR
   button, initiate pairing from your stereo via its pairing feature.

One final note about the pairing process: Paired devices share a set of
security keys with each other. Pressing the RESET/PAIR button erases those
keys from the WT32i chip. If your car stereo has the option to remember a list
of multiple different Bluetooth devices (my Honda has this) then you must
remember to delete the empeg Car from the list if you have pressed the
RESET/PAIR button and are re-pairing a second time.


Set Bluetooth PIN code if needed (most likely not needed)
==============================================================================
BlueGigaEmpeg does not usually require a PIN code to successfully pair. Before
trying to modify the PIN code, simply try pairing first.

Some stereos will prompt with a random set of digits on the screen and ask you
to confirm those digits on the Bluetooth device with a Yes/No prompt. This is
not the same thing as a PIN code. In those cases, the BlueGigaEmpeg will try
to automatically answer this prompt with a "yes" answer so that no user
interaction will be needed.

In rare cases where a special PIN code is is needed, there is a place in the
"BlueGigaEmpeg.ino" source code file where you can modify the PIN to something
other than "0000". Find the line in the code that defines the variable
"btPinCodeString". That line contains the "0000" default PIN code. Change it
to your correct PIN code for your stereo and re-upload the sketch to the
Arduino. It can accept PIN codes up to 16 digits long.


Test AVRCP behavior and set serial port crossover jumpers if needed
==============================================================================
If you are lucky, the empeg will respond to commands from your car stereo's
controls for play, pause, next track, etc., and will display track titles on
its screen. This is done via the Bluetooth Audio Video Remote Control Profile
("AVRCP"), and the serial port connection to the empeg Car. If this is already
working fine, then skip this step and go on to the next step.

However, if the Bluetooth pairs successfully and streams music, but you are
unable to play/pause/next the empeg via the car's stereo's controls, and the
track titles don't show up on the car stereo's screen, then try changing the
crossover jumper block on the BlueGigaEmpeg module as described below.

These jumpers exist because I have seen empeg sleds with different wiring on
the RS-232 plug. Some are wired straight, some are wired crossover. The jumper
block allows you to wire the BlueGigaEmpeg board as straight or crossover to
compensate for this. 

To change this, open the BlueGigaEmpeg enclosure with a 2.5mm hex tool and
lift out the board assembly. Look near the button and LED, and you'll see the
RS-232 Crossover jumper block, with silkscreened instructions on the board to
describe how to change the setting. Set the RS-232 Crossover jumper block to
the opposite setting.

Place the assembly into the BlueGigaEmpeg enclosure so that the RESET/PAIR
button and the LED fit into the holes on the enclosure. If the LED got bent,
straighten it carefully so the LED fits into the hole.

When reassembling the BlueGigaEmpeg enclosure, make sure to align the small
notch in one end of the lid with the tuner module connector. The notch is
there to make room for the release tab on the tuner module connector. And be
careful not to overtighten the screws.


Modify empeg's power connection to car if needed
==============================================================================
I might recommend that, for this installation, you wire up the empeg to your
car differently than it would otherwise would be.

This design is intended to be used in such a way so that the empeg is not the
primary stereo in the system. The empeg becomes one of the Bluetooth inputs
into a modern car stereo. So the empeg's sleep mode, the empeg's "memory"
power connection, and the way this Bluetooth module gets its power from the
empeg tuner connector, all combine to cause some interesting problems with
power state transitions. At least they did in my car, your mileage may vary.

I found that if I connected the empeg to my car via the regular method (i.e.
car ignition wire to the orange ignition wire on the sled, and constant 12v
power to the yellow memory wire on the sled) then there were certain unusual
states that it could get into. Sometimes I would shut off my car, but the
empeg would come back up out of sleep mode and play tracks silently to an
unconnected Bluetooth module. I was worried that this might drain my car's
battery.

If this happens to you, then I recommend connecting power to the empeg like
this instead:

- Yellow "memory" wire on empeg: Connect to car power 12v accessory power.
- Orange "ignition" wire on empeg: Connect to that same 12v accessory power.
- Do not connect your car's constant 12v power to any part of the empeg.
- White "lights on" wire on empeg: Connect to car dash illumination power.
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

You'll notice that the empeg sled connector only supplies power if the empeg
is awake. If you put the empeg into sleep mode via a long press on the top
button, it will go to sleep, but that also means the Arduino+Bluetooth
assembly stops receiving power. This is actually good: you want to be able to
turn the Bluetooth on and off and reboot it sometimes, and it's easy to do
that just by sleeping and waking the player by hand. Most of the time I leave
the player awake, so that when I shut off the car ignition, the player fully
shuts down, and then when I start my car again, the empeg starts up in "awake"
mode and immediately supplies power to the Bluetooth assembly which then
autoconnects to the car stereo head unit.


Debug Bluetooth connection if needed
==============================================================================
Everything hopefully will work perfectly, but there may be bugs in the Arduino
code because this was not tested with a wide range of Bluetooth gear.

If you need to debug the connection, here are some helpful tips.

###  Power up sequence (startup order) for Arduino debugging:

The USB cable from the computer will power the Arduino (and also the Bluetooth
board because it is connected to the Arduino through the BlueGigaEmpeg
interface board), but when the BlueGigaEmpeg assembly is connected to the
empeg, power comes from the empeg tuner connector. Both can be connected at
the same time during debugging. If using the USB cable for debugging mode, the
order of connecting the cables and applying power are important.

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
power to the empeg (or before waking the empeg from sleep). Meaning if I am
debugging in the car, I should connect the USB cable first and then turn on
the ignition or wake the empeg from sleep. That way, the Serial Monitor
program can find a port to connect to. If I am debugging on the test bench, I
should connect the USB cable first, then connect the AC adapter to the empeg
or wake the empeg from sleep.

When using the Arduino Serial Monitor program, set it to 115200 baud, with
either "Newline" or "Both NL and CR" as line endings. If you are using a
different serial terminal program, set it to 115200 8n1 and turn on local
echo.

###  Sending a bug report:

- Use the Arduino Serial Monitor or other serial terminal as described above.

- Reproduce the issue so that the communication shows up on the serial output
  while the issue is occurring.

- After the issue is reproduced, pause the empeg player so that additional
  timestamp reports no longer show up (screen stops scrolling constantly).

- Type the "set" and "list" commands into the serial monitor so that their
  results show up on the screen.

- "Select all" in the serial monitor main window and copy and paste the
  contents into a text document and attach it to the bug report.
  
- Submit bug reports by creating an account at GitHub if you don't already
  have one, then go to https://github.com/tfabris/BlueGigaEmpeg/issues and
  press "New Issue".

###  Variables in the Arduino sketch:

There are several flag variables in the BlueGigaEmpeg.ino sketch file, defined
at the top of the code, which can be modified if you need them when debugging.
Change them as you see fit, and re-upload the sketch to the Arduino. Remember
to change them back to their default values when you are done debugging, as
some of them can slow down the responses on the BlueGigaEmpeg and cause the
Bluetooth communication to miss things.

Refer to the code comments accompanying these flags to understand them:

      EmpegSendCommandDebug
      displayEmpegSerial
      displayTracksOnSerial

Useful debugging commands in the BlueGiga WT32i iWrap command language:

Typing any of these into the Arduino Serial Monitor should send these commands
to the Bluetooth module directly. Many will echo a response. Module will say
SYNTAX ERROR if there is a problem with any command.

      SET
      Displays Bluetooth module's current settings.

      SET BT PAIR
      Displays Bluetooth module's current list of paired devices.

      SET BT PAIR *
      Deletes all saved Bluetooth pairings on the Bluetooth module. The asterisk
      is important, it's the part that does the deleting.

      SET CONTROL RECONNECT *
      Turns off automatic reconnection attempts on the Bluetooth module until the
      next time it pairs up with something.

      RESET
      Reboots the Bluetooth module, saving current settings. Does not erase any
      existing saved Bluetooth pairings nor change any settings.

      SET RESET
      Resets the Bluetooth chip itself to its factory defaults. Does not remove
      existing pairings, those are still kept. Note: you should restart the
      Arduino after using "SET RESET" in order to have it apply its customized
      default settings to the Bluetooth chip again.

See the iWrap 6.x command reference and the AVRCP command reference documents,
linked in "Resources" elsewhere in this document, for more details on the
incredibly rich and complex command language of this Bluetooth chip.

A note about response ordering in the debug console:

Sometimes the responses from the Bluetooth module do not show up on the screen
immediately, because the main input loop for the Arduino code might not have
caught up and read all the responses from the serial port yet. So, for
example, a bad command might be issued several lines back, but the "SYNTAX
ERROR" response doesn't appear until later. So don't always assume that any
two command/response pairs are absolutely connected just because they appear
next to each other on the debug console screen.


Hardware interface information and notes (internal board connections)
==============================================================================
The BlueGigaEmpeg interface board implements everything described below. The
information below is just for my personal reference and you should not need to
do any of the connections below yourself, unless you are developing your own
interface board.

Arduino must be the "Mega" model with three hardware serial ports built in. I
tried it with an "Uno" model using software serial ports, and it had problems
where characters sometimes got dropped, and it could not keep up with the data
rates that I wanted.

###  Arduino and RS-232 port, critical connections:

Arduino serial port 1 (Tx1/Rx1) connects to the empeg's RS-232 port, but it
must go through an RS-232 converter circuit based on a MAX232 chip. This is
necessary because you can't connect an actual RS-232 cable directly to an
Arduino, the voltage and signaling are different. So it must go through the
MAX232. The full schematic for this circuit is linked in the "Resources"
section of this document, and the pinouts are also listed below.

The RX/TX crossover configurations for the MAX232 and the RS-232 plug on the
BlueGigaEmpeg can be confusing. The empeg Car sled serial port may or may not
be already wired into a crossover configuration. I have seen both cases. There
are jumpers on the BlueGigaEmpeg PCB to swap the RX/TX configuration to the
serial port as needed.

I am using a TI MAX232E for this implementation. The connections for the
MAX232 are as follows:

    MAX232 Pin 1 and 3 aka C1+/C1- - Bridge with a 2.2uf ceramic capacitor
    MAX232 Pin 4 and 5 aka C2+/C2- - Bridge with a 2.2uf ceramic capacitor
    MAX232 Pin 2 aka V+ - Connect to Arduino +5v via a 2.2uf ceramic capacitor
    MAX232 Pin 6 aka V- - Connect to GND via a 2.2uf ceramic capacitor
    MAX232 Pin 11 aka TTL-I1 - Connect to Arduino Mega Board 18 aka Tx1 
    MAX232 Pin 12 aka TTL-O1 - Connect to Arduino Mega Board 19 axa Rx1
    MAX232 Pin 13 aka 232-I1 - Connect to RS-232 plug pin 3 aka Tx via jumpers
    MAX232 Pin 14 aka 232-O1 - Connect to RS-232 plug pin 2 aka Rx via jumpers
    MAX232 Pin 15 aka GND - Connect directly to GND
    MAX232 Pin 16 aka VCC - Connect directly Arduino +5v output.
    MAX232 Pin 16 aka VCC - Also connect to 0.1uf ceramic capacitor which
                            then connects to GND. (Pin 16 is two connections,
                            one to Arduino +5v and then also to the smoothing
                            capacitor which bridges across 5v and GND. Place
                            this cap as close to pin 16 as possible.)

Empeg tuner connector is used to supply power to the Pololu step-down
transformer power supply, and from there, on to the rest of the assembly. Blue
wire on tuner connector connects to the voltage input pin on Pololu, and the
black wire on the tuner connector connects to the Pololu ground input pin.

Arduino "Vin" pin is connected to the 7.5v output rail from the Pololu step-
down transformer power supply circuit.

Grounding: Multiple Arduino GND pins connected to the output ground rail of
the Pololu power supply.

Arduino and Pair mode indicator LED: Arduino pin 50 digital I/O pin, connected
to +LED through resistor. Current of resistor determined by online LED
resistor calculator and LED values. Then -LED connect to GND. Example: If
using a blue LED with a 3.2v forward voltage and 20ma current, and the Arduino
analog power supply from the analog pins will be 5 volts, then use a 100 ohm
resistor for this value. Online resistor current calculator:
http://led.linear1.org/1led.wiz

Arduino and Button: Arduino pin 52 digital I/O pin, connected to one of the
ground legs of button. This same line (or the other ground leg of the button)
also goes through 10k pulldown resistor to ground. One of the + legs of the
button connects to +5v coming from the Arduino 5v pin. Follow examples on the
Internet of how to implement a simple temporary pushbutton on an Arduino:
https://www.arduino.cc/en/Tutorial/Button

###  BlueGiga Bluetooth WT32i chip+board, critical connections:

BetzTechnik board JP4 FTDI UART Enable jumper is cut after applying firmware
update.

Bluetooth chip+board "5v" power pin NOT connected to anything.

Bluetooth chip+board "3v3" or "VDD_IO" pin connected to the 3v output coming
from the Arduino.

Bluetooth chip+board "ENA" or "VREG_ENA" pin connected to the 3v output coming
from the Arduino.

Bluetooth chip+board "BATT" pin connected to the 3v output coming from the
Arduino.

Bluetooth chip+Board "Gnd" pin connected to the ground rail.

Bluetooth chip+board serial port "RX/TX" pins connected to the "TX2/RX2" pins
(serial port 2) of Arduino. Note: This is a crossover connection, i.e., RX
from the Arduino is connected to TX on the Bluetooth chip and vice-versa.
Since this is at TTL level, you don't need to go through a MAX232 circuit.
However, the Arduino is running at 5v TTL, and the BlueGiga chip runs at 3.3v.
So you must voltage match them. Decrease the voltage for the Arduino TX2
output, by running it through a simple 50% voltage divider to step 5v TTL from
Arduino down to 2.5v for the BlueGiga chip. The BlueGiga chip's transmit pin
can stay at 3.3v and the Arduino will still read it just fine, so no changes
are needed for that line. Schematic:

    ARDUINO TX2-----VVVVV----+----VVVVV-----GND
                   10Kohm    |    10Kohm
                             |
                             +--------------WT32i Rx


    ARDUINO RX2-----------------------------WT32i Tx


Bluetooth chip+Board three I2S pins PCM_CLK, PCM_SYNC, PCM_IN connected to
empeg IISC, IISW, IISD1 via a special modification to the empeg tuner module
connector, as described in the section of this document titled "Modify Empeg
Car interior for I2S digital audio connection".

Each one of the three I2S pins will need to be coming in through a set of
resistors arranged in a voltage divider configuration, two 10k resistors for
each one of the three lines. Example of one line (repeat a total of three
times, one for each of the I2S connections):

    EMPEG IISC-----VVVVV----+----VVVVV-----GND
                  10Kohm    |    10Kohm                    (3x) 
                            |
                            +--------------WT32i PCM_CLK



Bluetooth Chip Firmware Upgrade
==============================================================================
Note: This is only for my personal reference. This should only be needed if
you are getting a replacement board directly from BetzTechnik, or you are
performing some type of maintenance or debugging.

The firmware upgrade is done with a USB-A-to-micro-USB cable.

Steps:

 - Make sure the Bluetooth breakout board is fully separated and disconnected
   from the BlueGigaEmpeg board.

 - Make sure that jumper JP4 on the BetzTechnik WT32i Bluetooth board is
   connected. It comes connected by default, and then we cut it after
   upgrading the firmware. It needs to be connected any time you do firmware
   updates to the WT32i chip.

 - The firmware upgrade must be performed from a Windows computer, since the
   upgrading software is Windows software. Virtualized Windows works, too, for
   example, I was successful with doing this in Parallels on a Mac computer.

 - Download and unzip the "iWRAP-Firmware-Releases.zip" file linked in the
   "Prerequisites" section of this document.

 - Connect your Windows computer to the Bluetooth device via a USB micro cable
   connected to the "UART" port on the BetzTechnik WT32i breakout board. (If
   you are running a Windows VM, you may need to assign this device to the
   USB of the virtual machine. For example, if running Parallels on Mac, it
   will prompt you to do so when you plug it in while Parallels is running.)

 - Run Windows Device Manager. This can be found in the Windows control panel,
   by searching from the Start menu, or by running the program "DEVMGMT.MSC".

-  Look in Windows Device Manager to see if the USB connection has made a new
   serial port appear as a device under the heading "Ports (COM & LPT)".

 - If there is no new serial port there, look for a little yellow boo boo icon
   in the USB section instead. If there is a little yellow boo boo icon
   instead of a serial port, then you must install the necessary FTDI drivers
   for the USB-serial connection to the board. Links to these drivers are in
   the "Prerequisites" section of this document.

 - Now, plugging in the Bluetooth breakout board into the PC with the USB
   micro cable makes a serial port appear in the Windows Device Manager.

 - In Windows Device Manager, right click on the serial port and select
   "Properties". Set the speed of the serial port to 115200 bps with 8 data
   bits, no parity, 1 stop bit, no flow control (aka "8N1").

 - Once that's sorted, run the "SerialDFU.exe" tool found in one of the
   folders of the unzipped "iWRAP-Firmware-Releases.zip" file. In the folder
   of unzipped files, the full path is:
   
       iWRAP_Firmware_Releases
         Firmware
          DFU
           SerialDFU
            SerialDFU.exe

 - Note: The serial port settings in the SerialDFU tool will show "8E1" for
   its BCSP settings instead of "8N1". There will be two sets of serial port
   settings on the screen. The first one will say "8E1" and the second one
   will say "8N1". This is OK and must remain that way for the upgrade
   process to work. 

 - Make sure the "Get Device Type" button works, and shows your device
   information for your WT32i chip.

 - Select the options to upgrade your chip with the most recent WT32i file
   among the unzipped files. Do this by pressing the "Browse" button on the
   "Select DFU file" field and browsing to the correct ".dfu" file found here:
   
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

 - Make sure the upgrade completes successfully. Note that you might need to
   press the reset button or change the position of the power switch on the
   board to succeed.

 - After the upgrade is successful, unplug the USB cable from the board and
   computer, and close the Serial DFU Tool software.

 - Make sure to cut JP4 as described in the next step.


Set jumpers and switches, and modify Bluetooth board
==============================================================================
Note: This is only for my personal reference. This is only needed if you have
received a replacement board from BetzTechnik or you are doing other
maintenance work.

On the BetzTechnik WT32i Breakout board:

Cut the jumper at "JP4 FTDI_+5v" on the BetzTechnik board, but only do this
AFTER successfully updating the chip's firmware to the latest version. Cutting
the jumper shuts off power to the the BetzTechnik onboard UART, which prevents
the UART and the Arduino pin-to-pin serial connection from arguing with each
other. This prevents errors on the serial port which would cause the chip to
reboot randomly.

JP4 is made of two exposed solder pads, plus a very tiny bridge trace between
them. Cut this tiny bridge trace with a sharp X-Acto knife. Magnification will
be required to see this properly.

IMPORTANT: Use extreme care when cutting JP4. Make sure not to peel up the
pads. There are two traces running to the USB side of the JP4 pad and if you
peel up the pad or the traces, your board will no longer work correctly.

On the BetzTechnik board, set the "Smd_2_pole_switch" to the "down" position
(looking at the board so that the board's silkscreen name is readable) which
is the "off" position for this switch. This disconnects the BATT voltage from
the 2.5v linear voltage regulator on the board. The BlueGigaEmpeg assembly
will be supplying power directly to the chip at 3v and will not use the BATT
power from that regulator.


Solder headers onto the Bluetooth board
==============================================================================
Note: This is only for my personal reference. This is only needed if you have
received a replacement board from BetzTechnik or you are doing other
maintenance work.

If you received the BetzTechnik board separately, you will need to solder the
22-pin female headers onto it so that it can press fit onto the BlueGigaEmpeg
circuit board. 

IMPORTANT: The 22-pin female headers are inserted onto the BOTTOM SIDE of the
BetzTechnik WT32i Bluetooth Breakout board, and they are through-hole
components, so the soldering happens on the top side of the board. If you do
this backwards then the pinouts will be backwards and you will damage the
connected components.

Best procedure for soldering the headers to make sure they fit:

- Plug the loose 22-pin female headers onto the 22-pin male headers already
  existing on the BlueGigaEmpeg board.

- Place the BetzTechnik board onto the 22-pin headers so that their soldering
  pins protrude through the holes in the BetzTechnik board.

- Double check that the labeled connections all match up: check the silkscreen
  printing on the BlueGigaEmpeg board with the silkscreen labels on the
  BetzTechnik board and make sure everything matches up.

- Make sure the BetzTechnik board is fully pressed down flat against the
  headers and that there are no gaps.

- With the board pressed into its final position, solder the end header pins
  in place, i.e., the four end pins, one on each end of each header on each
  side of the board. You're "tacking down" the ends of the headers to the Betz
  board to make sure they are in the correct position.

- Double check that the Betz board is still fully down flat onto the headers
  and that everything is still fully fitted and flat, and that there are no
  gaps.

- With the header end pins tacked down and everything verified, then solder
  the next pins in from the ends. Now you should have eight pins total
  soldered, two on each end of the header rows. This should be enough to hold
  the headers in their final positions temporarily.

- Now carefully pull apart the Betz board from the BlueGigaEmpeg board,
  separating the male headers from the female headers. Female headers should
  still be held in place well against the Betz board.

- Now fully solder all 44 header pins on the Betz board.


Assemble the BlueGigaEmpeg module
==============================================================================
Note: This is only for my personal reference. This is only needed if you have
received a replacement board from BetzTechnik or you are doing other
maintenance work.

Press fit the Arduino Mega board onto the BlueGigaEmpeg circuit board by
turning it face down and aligning all of the pins. You will notice that not
all pins on the Arduino are used, some of the headers are left unconnected on
purpose. Still, it should be clear which pins connect to which headers, based
on the silkscreen printing on the BlueGigaEmpeg circuit board and the pin
positions. The arduino should fit only one way. Make sure the pins go all the
way in correctly, and that no pins are bent and that there are no fitting
problems.

Press fit the BetzTechnik board onto the BlueGigaEmpeg board fully, making
sure the corresponding pins are all lined up correctly according to the
silkscreen printing on the boards. Make sure the pins go all the way in
correctly, and that no pins are bent and that there are no fitting problems.

Now you should have the BlueGigaEmpeg board, the BetzTechnik WT32i Breakout
board, and the Arduino Mega board, all sandwiched together and fully fitted.

Place the assembly into the BlueGigaEmpeg enclosure so that the RESET/PAIR
button and the LED fit into the holes on the enclosure. If the LED got bent in
shipping and handling, straighten it carefully so the LED fits into the hole.

Place the lid atop the BlueGigaEmpeg enclosure with the screw holes aligned.

IMPORTANT: On the lid of the BlueGigaEmpeg enclosure, there is a small notch
on one end. Make sure to align that notch with the Molex tuner connector on
the board. The notch is there to make room for the release tab on the tuner
connector.

Screw the four supplied screws into place using a 2.5mm hex tool. The four
screws are the same kind of M3 hex bolts that hold the empeg fascia in place
(I thought that would be a nice tribute), so you should already have a 2.5mm
hex tool on hand. The screws may be a tight fit at first, if the enclosure is
freshly printed. Tighten them down far enough to hold the lid snugly in place
but do not overtighten, which would strip the plastic threads.

Do not attach the BlueGigaEmpeg assembly to the empeg Car sled until after
the I2S modifications are complete before attaching. The I2S modifications
are described elsewhere in this document.


Resources
==============================================================================
```
Purchase:
Empeg Car player:           http://empegbbs.com/ubbthreads.php/forums/11/1/For_Sale
Arduino MEGA 2560 R3 Board: https://www.amazon.com/gp/product/B01H4ZLZLQ       
BetzTechnik WT32i Breakout: http://www.betztechnik.ca/store/p3/WT32i_breakout_board.html
BlueGigaEmpeg Interface:    http://www.empegbbs.com or direct email to tfabris@gmail.com

Download software:
Arduino IDE:                https://www.arduino.cc/en/Main/Software
BlueGigaEmpeg sketch:       https://github.com/tfabris/BlueGigaEmpeg
Hijack Kernel for empeg:    http://empeg-hijack.sourceforge.net/
Tony's Empeg Logo Editor:   http://empegbbs.com/ubbthreads.php/ubb/download/Number/7067/filename/logoedit17.zip
WT32i Firmware Upgrade:     https://www.silabs.com/documents/login/software/iWRAP-Firmware-Releases.zip
FTDI USB driver:            http://www.ftdichip.com/FTDrivers.htm

Bluetooth information, schematics, and command references:
Bluetooth AVRCP specs:      https://www.bluetooth.org/docman/handlers/DownloadDoc.ashx?doc_id=292286
Command reference:          https://www.silabs.com/documents/login/reference-manuals/iWRAP6-API-RM.pdf
AVRCP command reference:    https://www.silabs.com/documents/login/application-notes/AN986.pdf
Dev Board User Guide:       https://www.silabs.com/documents/login/user-guides/UG215.pdf
Dev Board Data Sheet:       https://www.silabs.com/documents/login/data-sheets/WT32i-DataSheet.pdf
Dev Board Reference Design: https://www.silabs.com/documents/login/reference-designs/Reference-Design-DKWT32i.pdf
Dev Board Full Schematic:   https://www.silabs.com/documents/login/reference-designs/DKWT32i-v2.2.zip
BetzTechnik Schematic:      http://www.betztechnik.ca/uploads/2/5/6/7/25674051/wt32i.pdf
Pololu V.Reg #2853 Pinout:  https://a.pololu-files.com/picture/0J5850.1200.jpg
Arduino Mega Pin Map:       https://www.arduino.cc/en/uploads/Hacking/Atmega168PinMap2.png
Arduino Mega Standalone:    https://www.arduino.cc/en/Main/Standalone
ClassOfDevice generators:   http://bluetooth-pentest.narod.ru/software/bluetooth_class_of_device-service_generator.html
                            http://www.ampedrftech.com/cod.htm
MAX232 circuit for Arduino: https://www.avrprogrammers.com/articles/max232-arduino
TI MAX232E datasheet:       http://www.ti.com/lit/ds/symlink/max232e.pdf
BlueGiga Forum:             https://www.silabs.com/community/wireless/bluetooth
Empeg BBS thread:           http://empegbbs.com/ubbthreads.php/topics/370217
GitHub Markdown:            https://github.com/adam-p/markdown-here/wiki/Markdown-Cheatsheet
                            https://guides.github.com/features/mastering-markdown/

Upgrade firmware on the WT32i:
Firmware Update Guide:      https://www.silabs.com/documents/login/user-guides/UG216.pdf
Page containing firmware:   https://www.silabs.com/documents/login/data-sheets/WT32i-DataSheet.pdf
Link to firmware zip file:  https://www.silabs.com/documents/login/software/iWRAP-Firmware-Releases.zip
Prolific PL2303 USB driver: http://www.prolific.com.tw/US/ShowProduct.aspx?p_id=225&pcid=41
FTDI USB driver:            http://www.ftdichip.com/FTDrivers.htm

Parts list used in BlueGigaEmpeg interface board:
BlueGigaEmpeg PCB:             tfabris@gmail.com
Pololu 7.5v V.Reg #2853:       https://www.pololu.com/product/2853
TI MAX232E:                    https://www.digikey.com/product-detail/en/texas-instruments/MAX232EIN/296-27963-5-ND/1511027
RS-232 connector:              https://www.digikey.com/product-detail/en/assmann-wsw-components/A-DS-09-A-KG-T2S/AE10968-ND/1241804
Molex tuner connector:         https://www.digikey.com/product-detail/en/molex-llc/0039295083/WM3923-ND/356037
Molex connector screws:  Qty:2 https://www.digikey.com/product-detail/en/b-f-fastener-supply/MPMS-003-0008-PH/H743-ND/274954
M3 hex bolts (for case): Qty:4 https://www.amazon.com/gp/product/B00SN36C6M
LED:                           https://www.digikey.com/product-detail/en/cree-inc/C503B-BCN-CV0Z0461/C503B-BCN-CV0Z0461-ND/1922945
Reset button:                  https://www.digikey.com/product-detail/en/e-switch/TL1105LF250Q/EG2506-ND/378972
Jumpers 0.10 pitch:      Qty:2 https://www.digikey.com/product-detail/en/te-connectivity-amp-connectors/2-382811-0/2-382811-0-ND/1864296
Male conn. headers:  3x2       https://www.digikey.com/product-detail/en/molex-llc/0010897060/WM9874-ND/3068084
                    18x2       https://www.digikey.com/product-detail/en/3m/961236-6404-AR/3M9466-36-ND/2071927
                    22x1 Qty:2 https://www.digikey.com/product-detail/en/3m/961122-6404-AR/3M9457-22-ND/2071905
                    10x1       https://www.digikey.com/product-detail/en/3m/961110-6404-AR/3M9457-10-ND/2071896
                     8x1 Qty:2 https://www.digikey.com/product-detail/en/3m/961108-6404-AR/3M9457-08-ND/7104637
Fem. conn. headers  22x1 Qty:2 https://www.digikey.com/product-detail/en/3m/929850-01-22-RA/929850E-01-22-ND/1094203
2.2uf Ceramic Capacitor: Qty:4 https://www.amazon.com/gp/product/B071VVTC7Z/ref=oh_aui_detailpage_o04_s00
0.1uf Ceramic Capacitor        https://www.amazon.com/gp/product/B071VVTC7Z/ref=oh_aui_detailpage_o04_s00
10k ohm resistors:       Qty:9 https://www.amazon.com/gp/product/B0185FIOTA/ref=oh_aui_detailpage_o02_s00
100 ohm resistor:              https://www.digikey.com/product-detail/en/stackpole-electronics-inc/CF18JT100R/CF18JT100RCT-ND/2022718
Jumper Wires:            Qty:3 (lying around)
```
