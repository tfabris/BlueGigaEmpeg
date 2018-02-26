BlueGigaEmpeg
==============================================================================
by Tony Fabris

https://github.com/tfabris/BlueGigaEmpeg

BlueGigaEmpeg is a project to use a Silicon Labs BlueGiga WT32i Bluetooth
chip, combined with an Arduino Mega board, to act as an interface between an
empeg Car Mk2/2a MP3 player and a modern Bluetooth-equipped car.

![Board](https://github.com/tfabris/BlueGigaEmpeg/blob/master/BlueGigaEmpeg%20Board%20Photo.jpg) ![Enclosure](https://github.com/tfabris/BlueGigaEmpeg/blob/master/BlueGigaEmpeg%20Enclosure%20Photo.jpg)


------------------------------------------------------------------------------

Table of Contents
==============================================================================
- [Introduction                                                                    ](#introduction)
- [Installation                                                                    ](#installation)
- [Usage                                                                           ](#usage)
- [Troubleshooting                                                                 ](#troubleshooting)
- [Updating firmware                                                               ](#updating-firmware)
- [Technical Data                                                                  ](#technical-data)

------------------------------------------------------------------------------


Introduction
==============================================================================
- [Project Information                                                             ](#project-information)
- [Caveats                                                                         ](#caveats)
- [Acknowledgments                                                                 ](#acknowledgments)

Project Information
------------------------------------------------------------------------------
The empeg Car (small "e") player, also sold as the Rio Car, is an amazing
in-dash MP3 player made from 1999-2001 whose features are still, nearly 20
years later, better than any other in-car MP3 solution available. However, the
empeg Car didn't have Bluetooth since it wasn't in common use yet at the time.

Modern cars are becoming increasingly difficult to install aftermarket stereos
into, due to the car touchscreen dashboards and the high level of integration
of the stereo system into the other car systems. For owners of the empeg Car
player, it's a problem, because it makes it hard to install our beloved player
into our new cars. This project helps to solve this issue by adding digital
Bluetooth output to the empeg Car player, allowing it to be connected to a
modern car's factory stereo system, and integrate with its controls, in the
same way that you would connect a smartphone to your car.

The BlueGigaEmpeg interface works in two ways:

1. Bluetooth A2DP connection allows high quality digital audio from the empeg
   Car player to play on the modern car's stereo speakers without needing an
   Aux Input on the car stereo. This is useful because many cars no longer have
   an Aux Input available, yet most new cars have Bluetooth. 

2. Bluetooth AVRCP connection allows the empeg Car to receive commands such as
   "next", "previous", "pause", etc. from the Bluetooth connection, allowing
   the car stereo's touchscreen and steering wheel controls to be able to
   change tracks and control playback on the empeg. In addition, the same AVRCP
   channel allows track title and artist information to be displayed on the car
   stereo's touchscreen.

Demo video: https://www.youtube.com/watch?v=XS9U_ALc634

Some modification of the empeg Car is required in order for this to work. The
Molex connector on the empeg docking sled's wiring harness, which is normally
used for an external AM/FM tuner module, must be converted into a digital
audio interface for sending audio data to the BlueGigaEmpeg module. Make sure
to perform all modifications listed in the [Installation](#installation)
section of this document.

Caveats
------------------------------------------------------------------------------
This project only works with the empeg Car Mk2 or the Rio Car player. It will
not work with the empeg Car Mk1 player.

At the time of this writing, I have tested this project on a very limited set
of Bluetooth gear. So far, I have only tested it on:
- My Honda Accord 2017 factory stereo.
- Kenwood Bluetooth-equipped car stereo in my roommate's car.
- Plantronics Voyager Edge Bluetooth headset.
- Onkyo TX-SR33 home stereo with Bluetooth input.
- Etekcity standalone Bluetooth receiver.
- iHome brand toy Stardestroyer with a Bluetooth speaker.

Some of the devices listed above do not have full functionality. For instance,
I cannot get track titles working on the Onkyo, and the Honda does not have
controls for Shuffle, Fast Forward, or Rewind. As far as I can tell, these are
limitations of the devices themselves, not problems with the BlueGigaEmpeg.

There are differences in Bluetooth implementation on various audio gear, and
so there still might be bugs using this on your particular car stereo. This
project is open source on GitHub, so that I can accept bug reports and code
fixes from people with other brands of Bluetooth gear.

This is not intended to replace the empeg front panel user interface. This
only allows you to do things like press Next Track, Pause, and Play on your
car stereo. The full menu and user interface of the empeg is still needed for
all other features of the empeg, such as selecting playlists. So this module
won't let you mount the empeg in your trunk, unless you also have a display
extender kit from Eutronix.com as well, which would allow you to reach the
empeg front panel from the driver's seat. Unfortunately, Stu at Eutronix
isn't selling display extenders any more. I was lucky to get a display
extender several years ago when they were still being sold, so I'm able to
mount my empeg in my trunk with this. But if you don't have one, plan on
still having to mount your empeg where you can get to it.


Acknowledgments
------------------------------------------------------------------------------
Many thanks to the members of the empegBBS who helped with this so much. The
initial chip suggestion was by BBS member Elperepat, and even though I ended
up not using the particular chip that he suggested (I had trouble with that
chip), it was his suggestion that started me experimenting with Bluetooth. Big
thanks to Shonky who had many helpful suggestions and who pointed out the
WT32i chip that is the one that finally got used in this particular design.
Also particular thanks to Stuart Euans of Eutronix who created the empeg
Display Extender board, which was critical to me being able to get my empeg
mounted in my new Honda, since it doesn't have dash space for the whole thing.
Big thanks to Peter Betz of Betztechnik, who made a convenient and perfectly
sized breakout board containing the chip I wanted to work with, and who was
very helpful and responsive in emails, and even made another manufacturing run
with an updated design. And massive thanks to Mark Lord who assisted me in
countless ways with the design and implementation, taught me a ton of
important things, found my bugs, fixed issues with the Arduino compiler,
modified his Hijack kernel to support this project, and pointed out all of my
EE design flaws. And last but not least, thanks to the entire empeg team who
made such a fantastic car MP3 player that we are still doing interesting
things with it, nearly 20 years later.


Installation
==============================================================================
Installing the BlueGigaEmpeg module requires some modifications to your empeg
Car player. Make sure to go through each of the steps linked below.

- [Prerequisites                                                                   ](#prerequisites)
- [Modify empeg Car interior for I2S digital audio connection                      ](#modify-empeg-car-interior-for-i2s-digital-audio-connection)
- [Disconnect all tuner modules from all sleds you own                             ](#disconnect-all-tuner-modules-from-all-sleds-you-own)
- [Empeg Car configuration changes                                                 ](#empeg-car-configuration-changes)
- [Upgrade the empeg Car's Hijack kernel and set "Serial Port Assignment"          ](#upgrade-the-empeg-cars-hijack-kernel-and-set-serial-port-assignment)
- [Modify empeg's power connection to car                                          ](#modify-empegs-power-connection-to-car)
- [Connect external hardware connections                                           ](#connect-external-hardware-connections)
- [Mounting                                                                        ](#mounting)


Prerequisites
------------------------------------------------------------------------------
Make sure you have all of these things before working with the BlueGigaEmpeg:

####  Purchase:
| Item                        | URL                                                     |
|:----------------------------|:--------------------------------------------------------|
| Empeg Mk2 or Rio Car player | http://empegbbs.com/ubbthreads.php/forums/11/1/For_Sale |
| BlueGigaEmpeg Interface     | tfabris@gmail.com                                       |

####  Download software (Windows PC or Windows VM required for some of these items):
| Item                              | URL                                                                                 |
|:----------------------------------|:------------------------------------------------------------------------------------|
| Hijack Kernel for empeg           | http://empeg-hijack.sourceforge.net                                                 |
| Tony's Empeg Logo Editor          | http://empegbbs.com/ubbthreads.php/ubb/download/Number/7067/filename/logoedit17.zip |
| Arduino standalone IDE            | https://www.arduino.cc/en/Main/Software                                             |
| BlueGigaEmpeg GitHub project      | https://github.com/tfabris/BlueGigaEmpeg                                            |
| BlueGigaEmpeg.ino direct download | https://github.com/tfabris/BlueGigaEmpeg/archive/master.zip                         |

####  Important:

The BlueGigaEmpeg module works by connecting to the Molex connector on the
empeg Car's docking sled wiring harness. This connector was originally
intended for the tuner module, and the empeg must be modified so that the
tuner module connector becomes a digital audio connection instead.

Do not connect the BlueGigaEmpeg module to the tuner module connector until
after completing the step in this document titled ["Modify empeg Car interior
for I2S digital audio connection"](#i2s). Also, once the I2S modification has
been completed, do not connect the empeg to any tuner module. Damage may occur
if these instructions are not followed.

Be sure you are capable of safely making internal modifications to the empeg
Car player, and your are able to solder and shrink-tube electronic components.
I take no responsibility for damage incurred while you are dismantling and
modifying your empeg Car player.


<a name="i2s"></a>

Modify empeg Car interior for I2S digital audio connection
------------------------------------------------------------------------------
To perform this step, you must be comfortable with disassembling the empeg Car
player and removing the disk drive tray in such a way as to not cause damage
to the player. In particular, there is risk to the components on the back side
of the display board, and to the IDE header connector on the empeg
motherboard. Use extreme caution when disassembling the empeg player.

You will be modifying the interior of your empeg Car so that there can be
three wires coming out the docking connector which carry digital audio data
("I2S" aka "IIS") and can connect to the BlueGigaEmpeg unit. This is done by
hijacking three of the wires originally used for the empeg tuner module on its
Molex connector. This tuner module connector will instead plug into the
BlueGigaEmpeg electronic assembly, which contains the Bluetooth chip and the
Arduino.

The empeg tuner module will no longer work after this modification, and damage
may occur if you connect one your player. If you have a tuner module, now is a
good time to disconnect it from the sled and put it away.

####  Disassembly of empeg Car:

Partially disassemble your empeg player by carefully removing the fascia, lid
and drive tray, as described below. You must do the disassembly gently, so as
not to damage the empeg. Make sure not to break any of the components sticking
off the back of the display board as you do this.

- Use a 2.5mm hex tool to carefully remove four hex bolts from the fascia.

- Gently remove fascia, buttons, rotary control, and colored lens.

- Extend the hinged pullout handle during the next step. This lowers the
  keeper hooks on the top of the player, which keep it hooked into the sled
  and which will also interfere with the lid removal if they aren't lowered.
  If you have trouble removing the lid in the next step, first double check
  that the handle is in the extended position.

- Remove lid by sliding it forward and upward over the tabs. This may be
  tricky on some players, requiring some gentle levering against the tabs with
  a screwdriver. You shouldn't need to bend or force anything, but it may be a
  bit of a tight fit as you remove it. Use caution not to slip and damage the
  components on the display board. As shown in this photo:
  ![Lid removal](https://github.com/tfabris/BlueGigaEmpeg/blob/master/Lid%20removal.jpg)

- Do not remove the display board.

- Remove four Pozidriv screws on the outside sides of the player which are
  holding the disk drive tray in place. Be careful not to strip the screw
  heads by using the wrong size or type of screwdriver. Make sure to select a
  screwdriver which fits the heads of the screws good and tight. Information
  about Pozidriv screws can be found here:
  https://en.wikipedia.org/wiki/List_of_screw_drives#Pozidriv

- Gently lift out the drive tray. Make sure not to bump any of the
  components on the back of the display board. There are some particularly
  fragile components there, and it's really important that you don't bump
  them, or your empeg display will stop working entirely. They are really easy
  to hit, too, so be super careful.

- When lifting out the drive tray, take note of how the IDE cable is folded
  and routed so that you can put it back exactly as you found it. 

- Do not disconnect the IDE cable. You should be able to set the drive tray
  gently aside, without disconnecting the cable, and also without putting
  any stress on the cable.

####  Jumper the I2S pads to the docking connector wires:

Refer to the annotated photograph "I2S Wiring Modification Photo.jpg" to help
understand the wiring instructions below.

I2S Wiring Modification Photo.jpg:
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

Carefully solder the three supplied jumper wires to pads 1,2, and 4, and you
must keep track of which wires are soldered to which pads.

When soldering, make sure the jumper wires and the solder joints are flat to
the board instead of sticking upwards. The disk drive tray gets close to that
location when mounted, so make sure they don't have a chance to contact the
tray.

After soldering, cover the solder points with tape or some other insulator to
prevent them from shorting out on the drive tray.

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
small pin connectors on the ends of them.

Carefully solder and shrink-tube your three jumper wires directly to these pin
connectors in the exact order described below. Shrink tubing has been
supplied with the BlueGigaEmpeg.

After you have done this, your empeg's IIS pads will, when docked, now be
carried out the back of the empeg, via the docking sled, into the tuner module
Molex connector. Solder your jumper wires to the empeg interior wires in this
order:

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
to glue them down. Use a small amount of glue, and make sure the blob of glue
does not "stick up", since the disk drive tray gets very close to that
location.

After the glue is fully dry, carefully reassemble the player. Reassemble in
the reverse order of disassembly. Make sure to put everything back where you
found it, in particular, make sure the IDE cable is folded and routed the same
way as it was when you disassembled the player.

When reassembling, be careful not to overtighten the hex bolts which hold the
fascia in place, they should only be finger-tight. If you overtighten them,
you may crack the fascia and/or prevent the buttons from working correctly.

Final wiring positions and colors:
                                                                                                                                                                
| Empeg IIS pads | Int. empeg wires * | Int. wht Conn pos **  | Sled Tuner Plug | BlueGigaEmpeg Tuner Plug | Bluetooth Chip | Usage                            |
|----------------|--------------------|-----------------------|-----------------|--------------------------|----------------|----------------------------------|
|  1 IISC        |  Yellow+Green wire |  Third from end       |  7 Purple       |  7  SCK                  |  30  PCM_CLK   |  Serial Clock                    |
|  2 IISW        |  Brown wire        |  Right end            |  2 Grey         |  2  WS                   |  29  PCM_SYNC  |  Word Select aka Sync            |
|  4 IISD1       |  Red wire          |  Second from end      |  1 Pink         |  1  SDIN                 |  27  PCM_IN    |  Serial Data aka PCM Audio Data  |
|                |                    |                       |  4 Black        |  4  GND                  |      GND       |  Universal Ground                |
|                |                    |                       |  8 Blue         |  8  12vPower             |                |  Power to voltage regulator      |

    *  (These are the interior wires connecting the docking sled connector to the
       motherboard.)
    
    ** (Original positions of these wires on the white connector near the Ethernet
       plug, now disconnected from that connector.)


Disconnect all tuner modules from all sleds you own
------------------------------------------------------------------------------
After completing the I2S wiring modification, the external empeg tuner module
will no longer work. Also, damage may occur if you connect a tuner module to a
modified empeg.

To prevent problems, make sure to disconnect any tuner modules from any
docking sleds that you own, and pack them away somewhere safe. After making
the I2S wiring modification to the empeg, do not use a tuner module any more.

Label your empeg with the supplied stickers, which tell your future selves
not to use this particular empeg with tuner modules any more.


Empeg Car configuration changes
------------------------------------------------------------------------------
The following configuration changes are required for BlueGigaEmpeg to work
correctly.

####  Player firmware version:

I have tested the BlueGigaEmpeg with version 2.00 of the empeg Car software.
It is designed to work with version 2.00 Beta 13 or version 2.00 Final. I have
not tested it with the 3.0 Alpha version of the empeg Car software, but
theoretically it should work. See the empeg Car FAQ for more information on
upgrading the empeg Car player software if needed.

#### Edit the config.ini on the player:

Using the Emplode or Jemplode software, edit the empeg Car's config.ini as
follows.

If you are using Emplode on Windows, you must first edit the Microsoft Windows
registry before it will allow you to alter the config.ini. First, run
REGEDIT.EXE in Windows and locate the following key:

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
as [hijack] only if that section does not already exist):

    [hijack]
    suppress_notify=2
    
    [output]
    notify=1
    
    [serial]
    car_rate=115200

These settings are case-sensitive. Make sure to synchronize with the player
after changing the config.ini settings.


<a name="hijack"></a>

Upgrade the empeg Car's Hijack kernel and set "Serial Port Assignment"
------------------------------------------------------------------------------
The latest version of Hijack is required in order for BlueGigaEmpeg to work
correctly. Install the latest Hijack Kernel onto the empeg Car player if it is
not already installed. Link to the Hijack kernel is found in the
[Prerequisites](#prerequisites) section of this document. Make sure it is
Hijack version 524 or later.

In order to install the latest Hijack Kernel onto the empeg Car, you must
connect the empeg car directly to your Windows computer's RS-232 serial port,
or to a USB-to-RS-232 adapter cable if your computer does not have an RS-232
serial port. Make sure the serial port drivers are installed and that you can
see the empeg bootup messages correctly through a serial terminal program.

To install the Hijack kernel, run Tony's Empeg Logo Editor. Link to the Logo
Editor program is found in the [Prerequisites](#prerequisites) section of this
document. On the File menu of the logo editor program, select "Kernel Flash
Utility" and follow the instructions.

Once Hijack is installed, open up the Hijack kernel settings on the player via
a long-press on the player's rotary encoder dial. Change "Serial port
assignment" to "Player uses serial port" and follow the on-screen instructions
to reboot the player.


<a name="power"></a>

Modify empeg's power connection to car
------------------------------------------------------------------------------
I recommend that for this installation, you connect the empeg's wiring harness
to your car differently than if it were a standard car installation. It might
not be required for your installation, but it was required for mine, so doing
this now could save you time and hassle later.

This BlueGigaEmpeg is designed to turn the empeg into a secondary Bluetooth 
input into a modern car stereo, rather than being the car's main stereo 
system. In this situation, the empeg's sleep mode, the empeg's "memory" power 
connection, the way the BlueGigaEmpeg module gets its power from the empeg 
tuner connector, and the way the car stereo issues Bluetooth commands when 
you shut off the ignition, all combine to cause some interesting problems with
power state transitions.

I found that if I connected the empeg to my car's power via the regular method
(i.e. car accessory power to the orange ignition wire on the sled, and car
constant 12v power to the yellow memory wire on the sled), then there were
certain unusual states that it could get into. Sometimes I would shut off my
car, but the empeg would come back up out of sleep mode and play tracks
silently to an unconnected Bluetooth module, draining my car's battery.

To prevent this problem from happening to you, I recommend connecting power
between the car and the empeg like this instead:

- Orange "ignition" wire from empeg: Connect to car 12v accessory power.
  - "Acessory Power" is the power that is only on when your car ignition is
    on, or your key is set to the "accessory" position so that your car
    stereo is turned on. Or the pushbutton equivalent of that, if your car
    doesn't have a physical key.

- Yellow "memory" wire from empeg: *Also* connect to car 12v accessory power
  (instead of the usual constant power).

- Do *not* connect your car's constant 12v power to any part of the empeg.

- White "lights on" wire from empeg: Connect to car dash illumination power.

- Black "ground" wire from empeg: Connect to car ground.

- Tuner module and serial plug connectors: Connect to BlueGigaEmpeg module.

That should be all you need. No analog audio or amplifier connections, because
the Bluetooth takes care of that now. No cellphone mute wire, because the
Bluetooth takes care of that now. No amp remote wire, because there is no amp
connected directly to the empeg.

When connected this way, the empeg does not receive a 12v constant power
source at all, and will completely lose all power when you turn off the
ignition, rather than going into sleep mode. The empeg and its attached
BlueGigaEmpeg module will always power on and off at the same time as the car
stereo, and nothing will get confused about power state. You might lose the
date/time information on the empeg sometimes, but your modern car likely has a
perfectly functional clock of its own. The empeg is already designed to save
its playback position when it unexpectedly loses power, so it will continue to
work correctly in this wiring scheme.

Normally, if the empeg were connected with analog audio to a regular
amplifier, but you used this wiring scheme of connecting the memory wire to
the accessory power, then there would be a pop sound when turning off the
ignition. But with the BlueGigaEmpeg, all audio is digital and goes through
the Bluetooth connection, so there will not be a pop sound from the empeg in
this case.

By the way, I'm using a set of "Add-a-circuit" fuse adapters to connect my
empeg to the car's power at the car's fuse panel. This let me avoid the hassle
of trying to tap into the factory stereo wiring. If you use this method, make
sure to research to find the correct size for your car. For example, mine are
the "low profile mini fuse" type. Some helpful links:
- https://en.wikipedia.org/wiki/Fuse_(automotive)
- https://www.google.com/search?q=Add-a-circuit+Fuse+Adapter


Connect external hardware connections
------------------------------------------------------------------------------
Do not connect the BlueGigaEmpeg module to the empeg Car's tuner module
connector until after completing the step in this document titled ["Modify
empeg Car interior for I2S digital audio connection"](#i2s).

If the I2S wiring modification has been completed, connect the tuner module
connector from the empeg Car player docking sled to the tuner connector on the
BlueGigaEmpeg module. This plug carries 12v power to the BlueGigaEmpeg module
and also carries I2S audio data.

Connect the RS-232 serial plug from the empeg Car player docking sled directly
to the serial port on the BlueGigaEmpeg module. This plug carries the
play/pause/next control commands from the BlueGigaEmpeg to the empeg Car
player software, and also carries the track metadata (title/artist/etc) from
the empeg Car player software to the BlueGigaEmpeg module.

####  USB Connector:

The USB "type B" connector on the outside of the BlueGigaEmpeg module
enclosure is left disconnected during normal operation. It is only used for
debugging, and for uploading the latest Arduino code to the BlueGigaEmpeg
module. However, if you are mounting your BlueGigaEmpeg module permanently in
a hard-to-reach location in your car, make sure to run a USB cable from this
port to a reachable location, so that you can connect a laptop to it later.
See the section of this document titled ["Debug Bluetooth Connection if
needed"](#debug- bluetooth-connection-if- needed) for more details on how to
use the USB connector for debugging.


Mounting
------------------------------------------------------------------------------
The BlueGigaEmpeg module can be permanently mounted to the car if needed. Four
mounting tabs are included on the casing for this purpose, though Velcro or
other methods can work too. It can also be simply stuffed in somewhere with
the rest of the attachment wiring, as long as it's not "dangling" and putting
stress on the attachment cables.

When mounting:

- Try to leave the recessed RESET/PAIR button reachable somehow. You won't
  need to use it very often, but when you do, it's super nice to be able to
  reach it. If you can't make the button reachable, then the USB cable will
  also allow you to pair, but the button is much more convenient.

- Make sure to leave the USB port accessible for firmware updates, debugging,
  and for pairing in situations where you can't reach the RESET/PAIR button.
  My best recommendation is to find a long USB type-A-to-type-B cable (It's a
  standard USB printer cable, you probably have a few extras of these lying
  around already), attach it to the BlueGigaEmpeg's USB port, and route it
  somewhere in your car that you can reach with a laptop when you need to. 


Usage
==============================================================================
- [Pairing                                                                         ](#pairing)
- [Normal operation                                                                ](#normal-operation)


Pairing
------------------------------------------------------------------------------
Turn on your car's ignition to apply power to the empeg. This should also
apply power to the connected BlueGigaEmpeg module via the tuner module
connector. Make sure the empeg is not in "sleep" mode, since power to the
tuner module connector is turned off in sleep mode.

The first time you use the BlueGigaEmpeg, you will need to pair it with your
Bluetooth car stereo. You might also wish to re-pair again after editing the
BlueGigaEmpeg firmware. The BlueGigaEmpeg has a recessed RESET/PAIR button
for this purpose.

####  Pairing Procedure:

- Turn the volume level on your car stereo down to a low level, so that you're
  not blasted when the pairing process is complete.

- If your car stereo has a feature that lists existing Bluetooth pairings, and
  if there is an existing entry for "empeg Car" from an earlier pairing (for
  example, if you need to pair a second time), make sure to delete that
  earlier entry from the car stereo, because the security key will become
  invalid when you press the RESET/PAIR button in the next step.

- Press the recessed RESET/PAIR button on the BlueGigaEmpeg module. You do not
  need to hold down the button, a single click is all it needs. The adjacent
  LED should light up blue for about 30 seconds or so.

  - If you can't reach the RESET/PAIR button, then it is also possible to
    initiate pairing from the USB debug connection, via a connected laptop, by
    enabling the "typeZtoPair" flag variable in the Arduino software. See
    ["Debug Bluetooth connection if needed"](#debug-bluetooth-connection-if-needed)
    for more information.

- On the car stereo, initiate its pairing mode while the blue LED on the
  BlueGigaEmpeg module is lit. If the stereo offers you a list of devices to
  pair with, then look for the device "empeg Car" and pair it.

- Your car stereo might put up a random number on the screen and prompt you to 
  confirm that number on the device. If it does, the BlueGigaEmpeg will
  automatically confirm that number for you.
  
- Your car stereo might prompt you to press a button to confirm or finalize
  the pairing process, or it might prompt you to choose the connection type.
  For instance, on my Honda, its touchscreen will prompt me to choose
  whether to pair it as a music device, a phone device, or both. If you get
  a prompt like this, choose the music device option.

- If the pairing doesn't work the first time, also try initiating pairing from
  the car stereo *after* the blue LED on the BlueGigaEmpeg module goes out.
  From your car stereo's touch screen interface, tell it to search for
  Bluetooth devices, and when the device "empeg Car" appears, select it.
 
- If the pairing still doesn't work, try sleeping the empeg (long press on top
  button), turning off the car stereo completely (turn off car ignition), turn
  on the car ignition again, wake the empeg, and try again from the top.

- When the pairing is completed, set the volume. With the car stereo volume
  still turned down to a low level, turn the volume on the empeg Car front
  panel all the way up to 0.0 db. Then gently increase the volume on the car
  stereo to the desired listening level. From now on, control the the overall
  volume level from the car stereo controls and leave the empeg set to 0.0 db.
  Note that there are features in the Hijack kernel for setting the volume
  level on startup, so if the empeg keeps booting into a low volume mode,
  check the Hijack kernel settings for "Volume Level on Boot" and set it to
  "Previous".

Car and empeg should be playing nicely together now, assuming everything else
is working correctly. Audio from the empeg comes out of your car stereo's
speakers, the car stereo's track change controls will change tracks on the
empeg, and the track titles will appear on the car stereo's screen.

See [Troubleshooting](#troubleshooting) if you encounter any problems.


Normal operation
------------------------------------------------------------------------------
After you have successfully paired with your car stereo once, you should not
need to pair it again each time you get in the car. It should automatically
reconnect each time you start your car.

Some car stereos will let you pair your phone and the empeg at the same time.
My Honda allows me to do this. When pairing, the touch screen lets me choose
whether the paired device is a phone device or a music device. I choose to
pair my phone as a phone, and the empeg Car as a music device. Then, each time
I start my car, they both pair up correctly. I can use the steering wheel
controls to initiate a speakerphone call, which automatically mutes and pauses
playback on the empeg, and resumes it after I hang up the call.

The empeg Car should automatically pause its playback when you switch to
another input on your car stereo, such as switching to the radio or CD player,
and should automatically resume playback when you switch back. 

The BlueGigaEmpeg implements a specific set of Bluetooth controls. Not all car
stereos support all of these controls. For instance, my Honda does not do
Fast Forward or Rewind with any Bluetooth device, so those features don't work
on my Honda.

####  Implemented:
 - Pause
 - Play
 - Previous Track
 - Next Track
 - Rewind
 - Fast Forward
 - Stop
   - Pressing Stop on the car stereo should pause the empeg.
 - Shuffle
   - Note: Shuffle is implemented as a toggle on/off command. If your stereo
     has a feature to display the current shuffling state (displaying whether
	 shuffle is currently turned on or turned off), it will not be correctly
	 indicated on your stereo's screen. The empeg car does not report its
	 shuffle state to the serial port, and so the Bluetooth interface cannot
	 get a reliable query of the current state of shuffle.
	 
####  Not implemented:
The items below are not implemented for Bluetooth, but you can still do them on
the empeg front panel.
 - Music Search
 - Repeat mode (repeat all tracks, repeat one track, etc.)
 - Volume up/down/mute (set empeg to 0.0db and use car stereo volume instead)

####  Sleep mode:
The empeg tuner connector only supplies power to the BlueGigaEmpeg module if
the empeg is awake. If you put the empeg into sleep mode via a long press on
the top button, it will go to sleep, but that also means the Arduino+Bluetooth
assembly stops receiving power. This is actually good: you want to be able to
turn the Bluetooth on and off and reboot it sometimes, and it's easy to do
that just by sleeping and waking the empeg from its front panel.

Most of the time, leave the player in "awake" mode. When you shut off the car
ignition, the player fully shuts down. When you start the car again, the empeg
starts up in "awake" mode and immediately supplies power to the BlueGigaEmpeg
module, which then autoconnects to the car stereo head unit.


Troubleshooting
------------------------------------------------------------------------------
- [Pairing issues                                                                  ](#pairing-issues)
- [Other issues                                                                    ](#other-issues)
- [Set Bluetooth PIN code if needed (most likely not needed)                       ](#set-bluetooth-pin-code-if-needed-most-likely-not-needed)
- [Test AVRCP behavior and set serial port crossover jumpers if needed             ](#test-avrcp-behavior-and-set-serial-port-crossover-jumpers-if-needed)
- [Debug Bluetooth connection if needed                                            ](#debug-bluetooth-connection-if-needed)


Pairing issues
------------------------------------------------------------------------------
When you press the recessed RESET/PAIR button on the BlueGigaEmpeg, it does
the following:

 - Erases all previous Bluetooth pairings from the Bluetooth chip.
 
 - Resets the Bluetooth chip to default settings.

 - Looks for Bluetooth devices to pair with for about 30 seconds, and pairs
   automatically with the first one that it sees.

 - The blue RESET/PAIR LED is lit during this pairing mode, and will turn
   off once pairing is complete, or once the 30 seconds has run out.

Though this is counterintuitive: Sometimes you don't press that RESET/PAIR
button at all. Sometimes, instead, you do the pairing when the RESET/PAIR LED
is not lit. There are some stereos which need you to initiate pairing solely
on the car stereo's touch screen. My Honda is one of these types of stereos.
In my case, for initial pairing with my car stereo the first time, I don't
press the RESET/PAIR button at all (except for the purpose of erasing previous
pairings). I just go to my car's touch screen, tell it to pair with the
Bluetooth device named "empeg Car", and it works. 

Some Bluetooth stereos still need you to press that RESET/PAIR button on the
BlueGigaEmpeg, which should light the LED for about 30 seconds, during which
time you should also put your stereo into pair mode.

If one method doesn't work for you, try the other method. For example this
sequence will work for many stereos and headsets:

 - Press the recessed RESET/PAIR button, and while the LED is still lit, then
   initiate pairing on your Bluetooth stereo via its pairing feature.

 - If that doesn't work, then wait for the RESET/PAIR LED on the
   BlueGigaEmpeg to go dark, and then, without touching the RESET/PAIR
   button on the BlueGigaEmpeg module, initiate pairing from your stereo via
   its pairing feature.

Sometimes Bluetooth devices can get into odd states which require a reboot to
fix. If you're having pairing troubles, reboot both devices: Reboot the car
stereo by turning the ignition off and on again, and reboot the BlueGigaEmpeg
by sleeping and waking the empeg with a long press on the top button of the
front panel.

When trying to pair, turn off any nearby Bluetooth devices which are in
"discoverable" mode or "pairing" mode. The BlueGigaEmpeg has no user
interface, so it has no way to pick and choose which device it's trying to
pair with. It will attempt to pair with the first one it sees, so if there's
more than one pairable device in the immediate neighborhood, it might not get
the one you think it's getting.

If necessary, experiment with different positioning of the BlueGigaEmpeg
module in relation to your car stereo. The Bluetooth chip is located near the
top center of the unit, near recessed RESET/PAIR button. It will work best if
it has line of sight to your car stereo and/or is physically close to it.

Paired devices share a set of security keys with each other. Pressing the
recessed RESET/PAIR button on the BlueGigaEmpeg module erases previous
pairings, thus erasing those keys from the WT32i chip. This is important if
your car stereo remembers an onscreen list of existing paired Bluetooth
devices and gives you the option to manage those devices: The empeg Car's
entry in that list becomes invalid when you press its RESET/PAIR button. So
you will need to delete the empeg Car from that list before trying to pair
it a second time.


Other issues
------------------------------------------------------------------------------
If there is good audio, but commands such as next/previous track and
play/pause don't work, and if track titles do not display on your car stereo
(assuming your stereo has this ability), then there may be a simple fix for
this. See the section titled ["Test AVRCP behavior and set serial port
crossover jumpers if needed"](#test-avrcp-behavior-and-set-serial-port-crossover-jumpers-if-needed).

Also check to make sure that the wires for the serial port connection (or any
other connection for that matter) have not pulled out of the back of the
docking connector on the empeg sled. This is a common problem. See the empeg
FAQ for more information on this problem.

If AVRCP commands and track titles fail to work intermittently, and it has
occurred after you have uploaded a new version of the BlueGigaEmpeg firmware
to the Arduino, then check to make sure that your Arduino IDE's compiler files
have the corrected values for the serial port buffers as described in the
[Updating firmware](#updating-firmware) section.

If you have trouble with the empeg waking up after you shut off the car
ignition, see the section here: ["Modify empeg's power connection to
car"](#power).

Check to see if the issue you're encountering is a known bug or a
previously-closed bug:

- Open bugs: https://github.com/tfabris/BlueGigaEmpeg/issues

- Closed bugs: https://github.com/tfabris/BlueGigaEmpeg/issues?q=is:issue+is:closed

If you have other problems not listed here, see this section: ["Debug
Bluetooth connection if needed"](#debug-bluetooth-connection-if-needed).


Set Bluetooth PIN code if needed (most likely not needed)
------------------------------------------------------------------------------
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
------------------------------------------------------------------------------
If you are lucky, the empeg will respond to commands from your car stereo's
controls for play, pause, next track, etc., and will display track titles on
its screen. This is done via the Bluetooth Audio Video Remote Control Profile
("AVRCP"), and the serial port connection to the empeg Car.

However, if the Bluetooth pairs successfully and streams music, but you are
unable to play/pause/next the empeg via the car stereo's controls, and the
track titles don't show up on the car stereo's screen, then try the following.

First check to make sure that your car stereo even has the features you're
trying to use. For example, try pairing your smartphone to the car stereo and
see if you get play/pause/next/previous commands, and track titles, with that.

If your smartphone works, and the empeg does not, double check that the
serial port is actually working on the empeg docking sled, and that none of
its wires have pulled out of the docking connector.

If the serial port is good but you still get no joy with AVRCP commands or
track titles, then make sure that you correctly upgraded your player to the
latest Hijack kernel as described [here](#hijack), and that you performed the
changes listed in ["Empeg Car configuration changes"](#empeg-car-configuration-changes).

Finally, you can try changing the crossover jumper block inside the
BlueGigaEmpeg module as described below.

The crossover jumpers exist because I have seen empeg sleds with different 
wiring on the RS-232 plug. Some are wired straight, some are wired crossover.
I added the jumpers so that you can set the BlueGigaEmpeg board as straight or
crossover to compensate for this. The BlueGigaEmpeg ships with its jumpers in
the crossover configuration by default, in order to compensate for the 
situation where the empeg sled serial plug is wired as straight through. I 
think/hope this is the most common configuration.

To change the jumpers, open the BlueGigaEmpeg enclosure with a 2.5mm hex tool
and lift out the board assembly. Look near the button and LED, and you'll see
the RS-232 Crossover jumper block, with silkscreened instructions on the board
to describe how to change the setting. Set the RS-232 Crossover jumper block
to the opposite setting.

Place the assembly back into the BlueGigaEmpeg enclosure so that the recessed
RESET/PAIR button and the LED fit into the holes on the enclosure. If the LED
got bent, straighten it carefully so the LED fits into the hole.

When reassembling the BlueGigaEmpeg enclosure, be careful not to overtighten
the screws, if you tighten them too tight, you'll strip the plastic they're
screwed into, and then they won't work any more.


Debug Bluetooth connection if needed
------------------------------------------------------------------------------
Everything hopefully will work perfectly, but there may be bugs in the Arduino
code because this was not tested with a wide range of Bluetooth gear. If you
need to debug the connection, here are some helpful tips.

####  Debug mode with Arduino

The BlueGigaEmpeg module is an Arduino command+control board, sandwiched to a
custom interface board, connected to a BlueGiga WT32i Bluetooth chip. The
firmware running on the Arduino allows for a terminal debug interface to the
Bluetooth chip. This is automatically invoked when you connect a USB cable
from your computer to the Arduino's USB connector, which is the USB-B
connector accessible on the outside of the BlueGigaEmpeg casing. Once
connected, a USB-serial port will appear as a device on your computer through
the UART chip built into the Arduino.

Installing device drivers for the Arduino's USB-serial UART will be needed. To
install the device drivers and get the Arduino Serial Monitor working, install
the Arduino IDE by following the instructions in the section ["Updating
firmware"](#updating-firmware). This will also ensure that you are updated to
the latest BlueGigaEmpeg firmware before trying to debug any problems.

Once the device drivers are working for the Arduino's USB-serial UART, then
any serial terminal program will be able to communicate with the Arduino and
will work with its debug port. When connected to the Arduino debug serial
port, you can use these kinds of serial terminal programs to access the
terminal debug interface:

 - The "Serial Monitor" feature built into the Arduino IDE program. To run
   it, launch the Arduino IDE, select the "Tools" menu, then "Serial Monitor".

 - Any third party serial terminal program, such as "Putty".

When connected to this serial terminal interface, everything you type is sent
to the Bluetooth chip as a command, and all of the chip's responses are shown
on your screen. You will also see all commands that the Arduino sends to the
Bluetooth chip automatically, and see all of the chip's responses to them.

####  Power up sequence (startup order) for Arduino debugging:

The USB cable from the computer will power the Arduino (and also the Bluetooth
chip because it is connected to the Arduino through the BlueGigaEmpeg
interface board), but when the BlueGigaEmpeg assembly is connected to the
empeg, power comes from the empeg tuner connector. Both power sources can be
connected at the same time during debugging. If using the USB cable for
debugging mode, the sequence order of connecting the cables and applying power
is important.

The BlueGigaEmpeg assembly, and all devices which are part of the assembly
including the Arduino, normally get their power from the 12v power coming off
the tuner connector on the empeg sled wiring harness. This means that if you
plug it in and power the empeg, then the BlueGigaEmpeg is already powered by
the time you try to attach your USB debug cable to the Arduino.

This causes a problem: If the Arduino is already externally powered when you
connect the USB cable, then the computer cannot find its USB UART and is
unable to connect to the debug port.

So to successfully debug via the Arduino USB debug cable, you must connect 
the USB cable *first*, before applying power to the empeg (or before waking 
the empeg from sleep). Meaning, if you are debugging in the car, you should 
connect the USB cable first, and then turn on the ignition. If you are 
debugging indoors, connect the USB cable first, then connect the AC adapter
to the empeg.

If you are doing debugging/troubleshooting indoors, it's possible to use the
empeg's 12v AC adapter, connected to the player's AC adapter input. The
BlueGigaEmpeg module works in both AC/Home mode and DC/Car mode. However, a
sled wiring harness is still needed in AC/Home mode, because the BlueGigaEmpeg
still needs the tuner connector on the harness in order to function.


####  Sending a bug report:

- Turn on the computer and connect the USB cable from the computer to the
  Arduino debug port (the USB-B connector on the outside of the BlueGigaEmpeg
  casing).

- Run the Arduino Serial Monitor or other serial terminal as described above.

- Set the Serial Monitor program to 115200 baud, with either "Newline" or
  "Both NL and CR" as line endings. If you are using a different serial
  terminal program, set it to 115200 8n1 and turn on local echo.

- Apply power to the empeg. You should see certain messages on the terminal
  such as "empeg player boot process has started".

- Reproduce the issue, so that the commands and responses show up on the
  terminal output while the issue is occurring.

- After the issue is reproduced, pause the empeg player so that additional
  timestamp reports no longer show up (screen stops scrolling constantly).

- Type the "set" and "list" commands into the serial monitor so that their
  results show up on the screen.

- Save the output of the terminal program into a text document. For some
  terminal programs there is an option to save the output directly, for others
  you will need to "select all" then copy and paste into a text editor.
  - If you happen to be using Hyperterminal on old versions of Windows, it has
    a known bug with copy/paste where it mangles the output, so don't use
	that, use its feature for saving its output directly instead. Other
	terminal programs should be OK though.

- Submit bug reports this way:
  - Create an account at http://www.GitHub.com if you don't already have one.
  - Search through existing bugs to see if your issue is related to them.
    - Open bugs: https://github.com/tfabris/BlueGigaEmpeg/issues
    - Closed bugs: https://github.com/tfabris/BlueGigaEmpeg/issues?q=is:issue+is:closed
  - If yours is a new issue, then go to https://github.com/tfabris/BlueGigaEmpeg/issues
    and press "New Issue".
  - In the bug report, describe:
    - Model of Bluetooth playback device.
    - Frequency of the occurrence of the issue.
    - Steps to reproduce the issue.
    - Expected behavior versus actual behavior.
  - Attach your debug log to the bug report.

- When you are done with the debug session, remove the USB cable from the
  Arduino debug port. This is normally left disconnected during normal
  operation of the BlueGigaEmpeg module.
  
####  Variables in the Arduino sketch:

There are some flag variables in the BlueGigaEmpeg.ino sketch file, defined at
the top of the code, which can be modified if you need them when debugging.
Change them as you see fit, and re-upload the sketch to the Arduino. See the
["Updating firmware"](#updating-firmware) section for information on uploading
the sketch to the Arduino.

Remember to change flag variables back to their default values when you are
done debugging, as some of them can slow down the responses on the
BlueGigaEmpeg and cause the Bluetooth communication to malfunction
intermittently.

The most common flags you might want to change are these. Search for these
flags in the BlueGigaEmpeg.ino sketch file. Refer to the code comments
accompanying these flags in the file to understand what they do.

      empegSendCommandDebug
      displayEmpegSerial
      displayTracksOnSerial
      typeZtoPair

The "typeZtoPair" flag is particularly useful when the BlueGigaEmpeg module is
permanently mounted in your car. It will allow you to initiate RESET/PAIR mode
on the BlueGigaEmpeg from a connected laptop, without having to physically
press the RESET/PAIR button. This can be helpful if you have mounted the
BlueGigaEmpeg module in a hard-to-reach location in your car. Enable the flag,
re-upload the sketch to the Arduino, after which, if you type Z in the serial
debug console, the BlueGigaEmpeg will go into RESET/PAIR mode, the same as if
you had pressed the button.

####  Useful debugging commands in the BlueGiga WT32i iWrap command language:

Typing any of these into the serial terminal should send these commands to the
Bluetooth chip directly. Many will echo a response. The Bluetooth chip will
respond with "SYNTAX ERROR" if there is a problem with any command.

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
linked in the ["Resources"](#resources) section of this document, for more
details on the incredibly rich and complex command language of this Bluetooth
chip.

A note about response ordering in the debug console:

Sometimes the responses from the Bluetooth module do not show up on the screen
immediately, because the main input loop for the Arduino code might not have
caught up and read all the responses from the serial port yet. So, for
example, a bad command might be issued several lines back, but the "SYNTAX
ERROR" response doesn't appear until later. So don't assume that any
command/response pair will always appear together on the debug console screen.


Updating firmware
==============================================================================
Instructions for updating the firmware on the BlueGigaEmpeg module.

- [Modify your Arduino compiler for larger buffer size                             ](#modify-your-arduino-compiler-for-larger-buffer-size)
- [Compile and upload the latest version of BlueGigaEmpeg.ino to the Arduino       ](#compile-and-upload-the-latest-version-of-bluegigaempegino-to-the-arduino)


Modify your Arduino compiler for larger buffer size
------------------------------------------------------------------------------
The firmware on the BlueGigaEmpeg module resides in an Arduino chip which is
part of the BlueGigaEmpeg assembly. To update to the latest BlueGigaEmpeg
firmware on the Arduino, you will be uploading the sketch to the Arduino. But
there is a modification that you must perform first: You must increase the
default size of the Arduino serial port buffers. This can only be done by
modifying the compiler's files directly. If this isn't done, then after you
upload new firmware to the Arduino, there will be intermittent errors in the
BlueGigaEmpeg operation, such as commands sometimes being missed and track
titles sometimes not working or being incomplete.

Download and install the Arduino IDE, linked from the
[Prerequisites](#prerequisites) section of this document. Make sure to
download and install the standalone version of the Arduino IDE; do not try to
use the Arduino web editor. Though the web editor is convenient, it won't work
for this project because it doesn't have the capability of changing the header
files to increase the size of the serial port buffers.

Once it is installed, you'll need to edit a file to increase the size of the
Arduino's serial port buffers. The file that you need to edit will be the same
on all operating systems, but the location of the file will be different
depending on which OS you're using. The approximate location will be something
like this, but the exact location will vary:

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

Leave the "SERIAL_xx_BUFFER_SIZE 16" lines alone. Only modify the ones that
were originally set to "64" and increase them as described above.

Save the file.

Note: If you reinstall or upgrade the Arduino IDE program, you will need to 
perform this edit again before uploading new BlueGigaEmpeg code.


Compile and upload the latest version of BlueGigaEmpeg.ino to the Arduino
------------------------------------------------------------------------------
Obtain the BlueGigaEmpeg.ino Arduino project file from GitHub, linked in the
[Prerequisites](#prerequisites) section of this document. In the GitHub
project web site for BlueGigaEmpeg, find the button titled "Clone or download"
and obtain the repository. Most people will select "Download Zip" to download
the zip file, though if you are a Git user, feel free to clone the repository
or select "Open in Desktop". If you are downloading the zip file, unzip the
entire project into a folder named "BlueGigaEmpeg" on your computer's hard
disk. This will likely require renaming the folder to "BlueGigaEmpeg" after
unzipping the file.

####  Important:

The folder in which the BlueGigaEmpeg.ino file resides must be named
"BlueGigaEmpeg" and not, for example, "BlueGigaEmpeg-Master", because the
Arduino IDE won't recognize it as an Arduino project file unless the parent
folder name is the same as the ".ino" file name. I don't know why this is a
requirement, but based on my experiences with the Arduino IDE, that seems to
be the case. If you're synchronizing with this repository via Git, it will
automatically be named correctly, but if you are downloading and unzipping
the zip file, you will likely have to rename the folder after unzipping it.

####  Connection order special instructions:

The Arduino is located inside the enclosure of the BlueGigaEmpeg module, and
you upload the code to it via the USB connector on the end of the
BlueGigaEmpeg casing. Uploading code to the Arduino may be done with the
BlueGigaEmpeg module disconnected from the empeg Car. No external power source
is required in this case, the Arduino gets its power from the computer via the
USB cable in this situation. When connecting the Arduino to the computer via
the USB cable, if that's the only connection, then no special connection order
is needed.

In some cases, though, you may also want to upload code to the Arduino with
the BlueGigaEmpeg module connected to the empeg Car. This is possible to do,
however, special connection instructions are needed if you do this:

- Do not connect the BlueGigaEmpeg module to the empeg Car until after
  completing the step in this document titled ["Modify empeg Car interior for
  I2S digital audio connection"](#i2s). Damage may occur if you connect it
  before making the modification.
  
- The computer will not "see" the USB connection to the Arduino if it receives
  power from the empeg first. So before connecting the USB cable between the
  computer and the Arduino, make sure that the empeg is not powered up, or that
  it is in sleep mode.

####  Upload the sketch:

Connect the USB cable from the computer to the Arduino USB connector. The
Arduino USB connector is the USB "type B" plug which is exposed on the end of
the BlueGigaEmpeg enclosure.

When first plugging the Arduino into the computer, it may need to install
device driver files for the USB/serial connection to the Arduino. In theory,
the installer for the Arduino IDE should have taken care of installing these
drivers for you. If not, search for help documents online about getting
drivers for the "Arduino Mega 2560" chip working.

Run the Arduino IDE. In its "Tools" menu, select the "Board" menu and choose
"Arduino/Genuino Mega or Mega 2560". Then use the "Tools" menu to select
"Port" and choose the correct serial port for the Arduino connected to the
USB cable.

Launch the Arduino Serial Monitor feature from the Arduino IDE. This is done
by selecting "Tools", then "Serial Monitor". Configure it to 115200 BPS, and
now you should see some serial port output from the Arduino. If you don't
already see this, try closing and re-opening the Serial Monitor.

If the Serial Monitor is working, select the "File" menu in the Arduino IDE,
select "Open" and locate the BlueGigaEmpeg.ino project that you obtained
earlier. Compile and upload it to the Arduino Mega board by pressing the
"Upload" button on the main window. The Upload button is the green circle
with the arrow inside it.

It will take several seconds to upload. Look at the Arduino IDE and make sure
that there is no red text or other error messages near the bottom of the
window which would indicate a problem compiling or uploading the sketch. If
it is good, there will be a very small piece of text on the IDE, on the green
bar between the code pane (top) and the console pane (bottom), saying "done
uploading".

After the latest version of the BlueGigaEmpeg.ino sketch is successfully
uploaded to the Arduino, look at the output in the Serial Monitor again. The
top section of the output should list the RX and TX buffer sizes near the
beginning, and indicate that they are good. An error message will appear
if the serial port buffer sizes are not good.

Close the Arduino IDE and disconnect the USB cable from the computer. The USB
cable is normally left disconnected during normal operation of the
BlueGigaEmpeg module. It is safe to leave a USB cable attached to it, for
example, if the BlueGigaEmpeg module is permanently mounted in a hard-to-reach
place, you should leave a USB cable attached that extends to a reachable
location. This will make future updates and debugging easier.


Technical Data
==============================================================================
All sections below are for developer reference, and should not be needed by
end-users of the BlueGigaEmpeg module unless performing repairs or other
maintenance.

- [Bluetooth Chip Firmware Upgrade                                                 ](#bluetooth-chip-firmware-upgrade)
- [Set jumpers and switches, and modify Bluetooth board                            ](#set-jumpers-and-switches-and-modify-bluetooth-board)
- [Hardware interface information and notes (internal board connections)           ](#hardware-interface-information-and-notes-internal-board-connections)
- [Resources                                                                       ](#resources)
- [Test, packing, and shipment checklist                                           ](#test-packing-and-shipment-checklist)

Bluetooth Chip Firmware Upgrade
------------------------------------------------------------------------------
Note: This is only for developer reference. This should only be needed if you
are getting a replacement board directly from BetzTechnik, or you are
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
   [Resources](#resources) section of this document.

 - Connect your Windows computer to the Bluetooth device via a USB micro cable
   connected to the "UART" port on the BetzTechnik WT32i breakout board. (If
   you are running a Windows VM, you may need to assign this device to the
   USB of the virtual machine. For example, if running Parallels on Mac, it
   will prompt you to do so when you plug it in while Parallels is running.)

 - The red LED on the BetzTechnik board will dimly pulse, indicating that the
   chip is asleep. Move the power switch on the BetzTechnik board into the
   "on/up" position in order to turn on the board and make the LED blink
   brightly and randomly.

 - Run Windows Device Manager. This can be found in the Windows control panel,
   by searching from the Start menu, or by running the program "DEVMGMT.MSC".

-  Look in Windows Device Manager to see if the USB connection has made a new
   serial port appear as a device under the heading "Ports (COM & LPT)".

 - If there is no new serial port there, look for a little yellow boo boo icon
   in the USB section instead. If there is a little yellow boo boo icon
   instead of a serial port, then you must install the necessary FTDI drivers
   for the USB-serial connection to the board. Links to these drivers are in
   the [Resources](#resources) section of this document.

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

 - Make sure the upgrade completes successfully (a message box should say so).

 - After the upgrade is successful, unplug the USB cable from the board and
   computer, and close the Serial DFU Tool software.

 - Make sure to cut JP4, and set the power switch to the down/off position, as
   described in the next step.


Set jumpers and switches, and modify Bluetooth board
------------------------------------------------------------------------------
Note: This is only for developer reference. This is only needed if you have
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

Use a continuity tester to make sure that the two halves of JP4 do not have
continuity.

On the BetzTechnik board, set the power switch "U5" to the "down" position
(looking at the board so that the board's large silkscreen name is readable)
which is the "off" position for this switch. 


Hardware interface information and notes (internal board connections)
------------------------------------------------------------------------------
The BlueGigaEmpeg interface board implements everything described below. The
information below is for developer reference only, and you should not need to
do any of the connections below yourself, unless you are developing your own
interface board.

Arduino must be the "Mega" model with three hardware serial ports built in. I
tried it with an "Uno" model using software serial ports, and it had problems
where characters sometimes got dropped, and it could not keep up with the data
rates that I wanted.

####  Arduino and RS-232 port, critical connections:

Arduino serial port 1 (Tx1/Rx1) connects to the empeg's RS-232 port, but it
must go through an RS-232 converter circuit based on a MAX232 chip. This is
necessary because you can't connect an actual RS-232 cable directly to an
Arduino, the voltage and signaling are different. So it must go through the
MAX232. The full schematic for this circuit is linked in the
[Resources](#resources) section of this document, and the pinouts are also
listed below.

The RX/TX crossover configurations for the MAX232 and the RS-232 plug on the
BlueGigaEmpeg can be confusing. The empeg Car sled serial port may or may not
be already wired into a crossover configuration. I have seen both cases. There
are jumpers on the BlueGigaEmpeg PCB to swap the RX/TX configuration to the
serial port as needed. Default shipping configuration is "Crossed" to
compensate for a straight connection coming out of the sled.

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

The empeg tuner connector is used to supply power to the Pololu step-down
transformer power supply, and from there, on to the rest of the assembly. Blue
wire on tuner connector connects to the voltage input pin on Pololu, and the
black wire on the tuner connector connects to the Pololu ground input pin.

Arduino "Vin" pin is connected to the 7.5v output rail from the Pololu
step-down transformer power supply circuit.

Grounding: Multiple Arduino GND pins connected to the output ground rail of
the Pololu power supply.

Arduino and Button: Arduino pin 52 digital I/O pin, connected to one of the
ground legs of button. This same line (or the other ground leg of the button)
also goes through a 10k pulldown resistor to ground. One of the + legs of the
button connects to +5v coming from the Arduino 5v pin. Follow examples on the
Internet of how to implement a simple temporary pushbutton on an Arduino:
https://www.arduino.cc/en/Tutorial/Button

Note: The BlueGigaEmpeg printed circuit board, Rev. 6, has the position of the
10k button resistor labeled on the wrong side of the PCB. It will work on
either side, but I discovered after manufacturing the Rev. 6 boards that there
is better clearance from the Arduino components if you move it to the other
side of the board. Rev. 7, not yet manufactured at the time of this writing,
has it corrected in the CAD file already.

Arduino and Pair mode indicator LED: Arduino pin 50 digital I/O pin, connected
to +LED through a resistor. Then, -LED connect to GND. The value of the
resistor is determined by an online LED resistor calculator and the LED spec
values. Example: If using a blue LED with a 3.2v forward voltage and 20ma
current, and the Arduino analog power supply from the analog pins will be 5
volts, then use a 100 ohm resistor for this value. An online LED resistor
calculator can be found here: http://led.linear1.org/1led.wiz

LED soldering height: To fit into the hole in the casing well, the top of the
diode assembly itself inside the LED lens (the metal part) should top out at
the same height as the top of the button plastic. Lens will protrude higher
than the button by vaguely 2mm or so.

LED anode: The positive/anode pin is the long lead. The positive pin is the
side with the smaller metal bit visible inside the lens. The positive/anode
pin goes into the hole on the PCB with the round pad. The negative/cathode pin
goes into the hole on the PCB with the square pad. The triangle/bar symbol on
the PCB silkscreen is this:

       positive  >|  negative


####  BlueGiga Bluetooth WT32i chip+board, critical connections:

BetzTechnik Bluetooth chip+board "JP4 FTDI UART Enable" jumper is cut after
applying firmware update.

BetzTechnik Bluetooth chip+board "Smd_2_pole_switch" or "U5" switch is set to 
the "off" position.

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
connector, as described in the section of this document titled ["Modify empeg
Car interior for I2S digital audio connection"](#i2s).

Each one of the three I2S lines will need to be reduced from 5v to 2.5v, to
prevent frying the WT32i chip (I fried two chips this way until we figured
this out). The I2S lines need to come in through a set of resistors arranged
in a 50% voltage divider configuration, two 1.5k resistors for each one of the
three lines. 1.5k is used instead of 10k to improve the S/N ratio on the I2S
lines at the expense of more current draw from the SA7705H DSP chip on the
empeg Car. This fixes GitHub issue #69. Example of one line (repeat a total of
three times, one for each of the I2S connections):

    EMPEG IISC-----VVVVV----+----VVVVV-----GND
                  1.5Kohm   |   1.5Kohm                    (3x) 
                            |
                            +--------------WT32i PCM_CLK

Note: The BlueGigaEmpeg printed circuit board, Rev. 6, still has the old 10k
value for the I2S voltage divider resistors silkscreened onto the board. The
correct value for the I2S voltage divider resistors is 1.5k. Also, these are
silkscreened onto the wrong side of the board. Theoretically they should work
on either side of the board, but if you put them on the opposite side, then
there's more clearance from the Arduino. Rev. 7, not yet manufactured at the
time of this writing, has them corrected in the CAD file already.


Resources
------------------------------------------------------------------------------
####  Purchase:
| Item                        | Url                                                          |
|:----------------------------|:-------------------------------------------------------------|
| Empeg Car player            | http://empegbbs.com/ubbthreads.php/forums/11/1/For_Sale      |
| Arduino MEGA 2560 R3 Board  | https://www.amazon.com/gp/product/B01H4ZLZLQ                 |
| BetzTechnik WT32i Breakout  | http://www.betztechnik.ca/store/p3/WT32i_breakout_board.html |
| BlueGigaEmpeg Interface     | tfabris@gmail.com                                            |

####  Download software:
| Item                        | Url                                                                                 |
|:----------------------------|:------------------------------------------------------------------------------------|
| Arduino IDE                 | https://www.arduino.cc/en/Main/Software                                             |
| BlueGigaEmpeg sketch        | https://github.com/tfabris/BlueGigaEmpeg                                            |
| Hijack Kernel for empeg     | http://empeg-hijack.sourceforge.net/                                                |
| Tony's Empeg Logo Editor    | http://empegbbs.com/ubbthreads.php/ubb/download/Number/7067/filename/logoedit17.zip |
| WT32i Firmware Upgrade      | https://www.silabs.com/documents/login/software/iWRAP-Firmware-Releases.zip         |
| FTDI USB driver             | http://www.ftdichip.com/FTDrivers.htm                                               |

####  Bluetooth information, schematics, and command references:
| Item                        | Url                                                                                         |
|:----------------------------|:--------------------------------------------------------------------------------------------|
| Bluetooth AVRCP specs       | https://www.bluetooth.org/docman/handlers/DownloadDoc.ashx?doc_id=292286                    |
| Command reference           | https://www.silabs.com/documents/login/reference-manuals/iWRAP6-API-RM.pdf                  |
| AVRCP command reference     | https://www.silabs.com/documents/login/application-notes/AN986.pdf                          |
| Dev Board User Guide        | https://www.silabs.com/documents/login/user-guides/UG215.pdf                                |
| Dev Board Data Sheet        | https://www.silabs.com/documents/login/data-sheets/WT32i-DataSheet.pdf                      |
| Dev Board Reference Design  | https://www.silabs.com/documents/login/reference-designs/Reference-Design-DKWT32i.pdf       |
| Dev Board Full Schematic    | https://www.silabs.com/documents/login/reference-designs/DKWT32i-v2.2.zip                   |
| BetzTechnik Schematic       | http://www.betztechnik.ca/uploads/2/5/6/7/25674051/wt32i.pdf                                |
| Pololu V.Reg #2853 Pinout   | https://a.pololu-files.com/picture/0J5850.1200.jpg                                          |
| Arduino Mega Pin Map        | https://www.arduino.cc/en/uploads/Hacking/Atmega168PinMap2.png                              |
| Arduino Mega Standalone     | https://www.arduino.cc/en/Main/Standalone                                                   |
| ClassOfDevice generators    | http://bluetooth-pentest.narod.ru/software/bluetooth_class_of_device-service_generator.html |
|                             | http://www.ampedrftech.com/cod.htm                                                          |
| MAX232 circuit for Arduino  | https://www.avrprogrammers.com/articles/max232-arduino                                      |
| TI MAX232E datasheet        | http://www.ti.com/lit/ds/symlink/max232e.pdf                                                |
| BlueGiga Forum              | https://www.silabs.com/community/wireless/bluetooth                                         |
| Empeg BBS thread            | http://empegbbs.com/ubbthreads.php/topics/370217                                            |
| Designing low noise PCBs    | http://www.ti.com/lit/an/szza009/szza009.pdf                                                |
| GitHub Markdown             | https://github.com/adam-p/markdown-here/wiki/Markdown-Cheatsheet                            |
|                             | https://guides.github.com/features/mastering-markdown/                                      |

####  Upgrade firmware on the WT32i:
| Item                        | Url                                                                         |
|:----------------------------|:----------------------------------------------------------------------------|
| Firmware Update Guide       | https://www.silabs.com/documents/login/user-guides/UG216.pdf                |
| Page containing firmware    | https://www.silabs.com/documents/login/data-sheets/WT32i-DataSheet.pdf      |
| Link to firmware zip file   | https://www.silabs.com/documents/login/software/iWRAP-Firmware-Releases.zip |
| Prolific PL2303 USB driver  | http://www.prolific.com.tw/US/ShowProduct.aspx?p_id=225&pcid=41             |
| FTDI USB driver             | http://www.ftdichip.com/FTDrivers.htm                                       |

####  Parts list used in BlueGigaEmpeg interface board:
| Item                        | Qty   | Url                                                                                                                |
|:----------------------------|:------|:-------------------------------------------------------------------------------------------------------------------|
| BlueGigaEmpeg PCB           |       | tfabris@gmail.com                                                                                                  |
| Pololu 7.5v V.Reg #2853     |       | https://www.pololu.com/product/2853                                                                                |
| TI MAX232E                  |       | https://www.digikey.com/product-detail/en/texas-instruments/MAX232EIN/296-27963-5-ND/1511027                       |
| RS-232 connector            |       | https://www.digikey.com/product-detail/en/assmann-wsw-components/A-DS-09-A-KG-T2S/AE10968-ND/1241804               |
| Molex tuner connector       |       | https://www.digikey.com/product-detail/en/molex-llc/0039295083/WM3923-ND/356037                                    |
| Molex connector screws      | Qty:2 | https://www.digikey.com/product-detail/en/b-f-fastener-supply/MPMS-003-0008-PH/H743-ND/274954                      |
| M3 hex bolts (for case)     | Qty:4 | https://www.amazon.com/gp/product/B00SN36C6M                                                                       |
| LED                         |       | https://www.digikey.com/product-detail/en/cree-inc/C503B-BCN-CV0Z0461/C503B-BCN-CV0Z0461-ND/1922945                |
| Reset button                |       | https://www.digikey.com/product-detail/en/e-switch/TL1105LF250Q/EG2506-ND/378972                                   |
| Jumpers 0.10 pitch          | Qty:2 | https://www.digikey.com/product-detail/en/te-connectivity-amp-connectors/2-382811-0/2-382811-0-ND/1864296          |
| Male conn. headers, 3x2     |       | https://www.digikey.com/product-detail/en/molex-llc/0010897060/WM9874-ND/3068084                                   |
| Male conn. headers, 18x2    |       | https://www.digikey.com/product-detail/en/3m/961236-6404-AR/3M9466-36-ND/2071927                                   |
| Male conn. headers, 22x1    | Qty:2 | https://www.digikey.com/product-detail/en/3m/961122-6404-AR/3M9457-22-ND/2071905                                   |
| Male conn. headers, 10x1    |       | https://www.digikey.com/product-detail/en/3m/961110-6404-AR/3M9457-10-ND/2071896                                   |
| Male conn. headers, 8x1     | Qty:2 | https://www.digikey.com/product-detail/en/3m/961108-6404-AR/3M9457-08-ND/7104637                                   |
| Fem. conn. headers  22x1    | Qty:2 | https://www.digikey.com/product-detail/en/3m/929850-01-22-RA/929850E-01-22-ND/1094203                              |
| 2.2uf Ceramic Capacitors    | Qty:4 | https://www.digikey.com/product-detail/en/murata-electronics-north-america/RCER71H225K2K1H03B/490-11889-ND/4277808 |
| 0.1uf Ceramic Capacitor     |       | https://www.amazon.com/gp/product/B071VVTC7Z/ref=oh_aui_detailpage_o04_s00                                         |
| 1.5k ohm resistors          | Qty:6 | https://www.amazon.com/gp/product/B077FMSR86/ref=oh_aui_detailpage_o00_s00                                         |
| 10k ohm resistors           | Qty:3 | https://www.amazon.com/gp/product/B0185FIOTA/ref=oh_aui_detailpage_o02_s00                                         |
| 100 ohm resistor            |       | https://www.digikey.com/product-detail/en/stackpole-electronics-inc/CF18JT100R/CF18JT100RCT-ND/2022718             |
| Jumper Wires 9.5 inches     | Qty:3 | (lying around)                                                                                                     |


Test, packing, and shipment checklist
------------------------------------------------------------------------------
Verify the following things before final packing and shipping:
- BetzTechnik board upgraded to version 6.2.0-1122.
- BetzTechnik board JP4 is cut.
- BetzTechnik board power switch is in the down/off position.
- Jumpers on BlueGigaEmpeg board are set to the "crossover" configuration.
- LED is straight.
- Arduino contains current version of Github code, check version at bootup.
- Arduino displays good serial buffers, check messages at bootup.
- WT32i reports version 6.2.0-1122 when booted.
- Powering up empeg displays a message on debug console (empeg messages work).
- Pairing button and LED works, pairing mode pairs with the playback device.
- Play/pause works on Bluetooth playback device (AVRCP+empeg commands work).
- Audio playback quality is good.
- Touch-check for no hot components after playing music for a few minutes.
- Place assembly into enclosure, make sure fit is perfect, LED is in the hole.
- Make sure pair button works with the enclosure closed up.
- Screw in the enclosure bolts slowly in stages, to prevent heat stripping.
- Can remove and reinsert enclosure bolts without stripping the plastic.
- Box includes baggie with USB cable, jumper wires, stickers, shrink tubing.
- Printout of one-pager instruction sheet is included in the box.
