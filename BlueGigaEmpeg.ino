// ----------------------------------------------------------------------------
// BlueGigaEmpeg
// by Tony Fabris
// ----------------------------------------------------------------------------
// A project to use a Silicon Labs BlueGiga WT32i bluetooth chip, combined with
// an Arduino Mega board with an added RS232 port, to act as an intermediary
// between the Empeg Car stereo and a modern car. The purpose is twofold:
//
// 1. Bluetooth A2DP connection allows high quality stereo audio from the
//    empeg car outputs to play on the modern car stereo speakers without
//    needing an AUX input on the modern car stereo (many cars no longer
//    have an AUX input available any more).
//
// 2. Allow Empeg Car to recieve commands such as "next", "previous", "pause" etc.
//    from the bluetooth connection, thus allowing the car's touchscreen and
//    steering wheel controls to be able to change tracks on the empeg.
//    This is accomplished by custom code running on the Arduino (this code).
//
// The audio connection works like this:
//  Empeg-Car ->
//  I2S digital outputs from Empeg Car ->
//  I2S digital inputs on bluetooth chip ->
//  Bluetooth chip paired with factory car stereo via Bluetooth A2DP ->
//  Sound from Empeg comes out the car speakers.
// 
// The data connection works like this:
//  Car stereo user presses touchscreen controls or steering wheel controls for |<<, >>|, pause, play, stop, etc. ->
//  Controls are sent to bluetooth chip from car stereo via Bluetooth AVRCP ->
//  Bluetooth chip, by default, echoes the AVRCP commands on its serial output ->
//  Arduino board receives these serial commands on its serial port ->
//  Arduino runs code which interprets these AVRCP commands and turns them into Empeg-Car-compatible commands ->
//  Arduino outputs these Empeg-Car serial commands to its attached RS232, via another serial port ->
//  Empeg car receives the serial commands and performs the action.
//  And also...
//  Empeg car outputs track metadata messages on its serial port ->
//  Arduino board receives track information from the empeg serial port ->
//  Arduino sends track information to the bluetooth chip via its serial port ->
//  Bluetooth chip sends track information to the car stereo ->
//  Track data such as Title and Artist appear on the car stereo's display.
//
// NOTE: At the time of this writing I only tested this on a very limited
// set of bluetooth gear. I have tested it on:
//    My Honda Accord 2017 factory stereo.
//    Kenwood bluetooth-equipped car stereo in my roommate's car.
//    Plantronics Voyager Edge bluetooth headset.
// There may be differences in bluetooth implementation on other audio gear,
// and so there might be bugs using this on your car stereo. Your mileage may
// vary! :-)
//
// Many thanks to the members of the empegBBS who helped with this so much. The
// initial chip suggestion was by BBS member Elperepat, and even though I ended
// up not using the particular chip that he suggested (I had trouble with that
// chip), it was his suggestion that started me experimenting with bluetooth.
// Big thanks to Shonky who had many helpful suggestions and who pointed out
// the WT32i chip that is the one that finally got used in this particular
// design. Also particular thanks to Stuart Euans of Eutronix who created the
// Empeg Display Extender board which was also critical to me being able to
// implement this at all. And massive thanks to Mark Lord who assisted me in
// countless ways with the design and implementation, taught me a ton of
// important things, found my bugs, fixed issues with the Arduino compiler,
// and pointed out all of my EE design flaws.
//
//
// ----------------------------------------------------------------------------
// Reference materials, resources, and purchase links:
// ----------------------------------------------------------------------------
// Purchase:
// Arduino MEGA 2560 R3 Board: https://www.amazon.com/gp/product/B01H4ZLZLQ       
// MAX232 circuit for Arduino: https://www.avrprogrammers.com/articles/max232-arduino
//                             http://justanotherlanguage.org/content/building-max232-circuit-serial-port-communication
// BetzTechnik WT32i Breakout: http://www.betztechnik.ca/store/p3/WT32i_breakout_board.html
// Pololu 5v 5a V.Reg #2851:   https://www.pololu.com/product/2851
// Molex tuner connector:      https://www.digikey.com/product-detail/en/molex-llc/0039295083/WM3923-ND/356037
// RS-232 connector:           https://www.digikey.com/product-detail/en/assmann-wsw-components/A-DS-09-A-KG-T2S/AE10968-ND/1241804
//
// Software, datasheets, and command references (may need to create an account at the Silicon Labs web site):
// Bluetooth AVRCP specs:      https://www.bluetooth.org/docman/handlers/DownloadDoc.ashx?doc_id=292286
// Command reference:          https://www.silabs.com/documents/login/reference-manuals/iWRAP6-API-RM.pdf
// AVRCP command reference:    https://www.silabs.com/documents/login/application-notes/AN986.pdf
// Dev Board User Guide:       https://www.silabs.com/documents/login/user-guides/UG215.pdf
// Dev Board Data Sheet:       https://www.silabs.com/documents/login/data-sheets/WT32i-DataSheet.pdf
// BetzTechnik Schematic:      http://www.betztechnik.ca/uploads/2/5/6/7/25674051/wt32i.pdf
// Pololu V.Reg #2851 Pinout:  https://a.pololu-files.com/picture/0J5850.1200.jpg
// Arduino Mega Pin Map:       https://www.arduino.cc/en/uploads/Hacking/Atmega168PinMap2.png
// Arduino Mega Standalone:    https://www.arduino.cc/en/Main/Standalone
// BlueGiga Forum:             https://www.silabs.com/community/wireless/bluetooth
//
// Upgrading firmware on the WT32i:
// Firmware Update Guide:      https://www.silabs.com/documents/login/user-guides/UG216.pdf
// Page containing firmware:   https://www.silabs.com/documents/login/data-sheets/WT32i-DataSheet.pdf
// Link to firmware zip file:  https://www.silabs.com/documents/login/software/iWRAP-Firmware-Releases.zip
//
// ClassOfDevice generators:    http://bluetooth-pentest.narod.ru/software/bluetooth_class_of_device-service_generator.html
//                              http://www.ampedrftech.com/cod.htm
//
// Windows drivers for the UART chip when connecting USB cable on dev board directly to PC:
// Prolific PL2303 USB driver: http://www.prolific.com.tw/US/ShowProduct.aspx?p_id=225&pcid=41
//
// Hijack Kernel for empeg:    http://empeg-hijack.sourceforge.net/
// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------
// Bluetooth Dev Board Firmware Upgrade
// ----------------------------------------------------------------------------
// Make sure the Bluetooth WT32i chip+board is updated to the latest firmware
// (links in "Reference Materials" above). You can run the Windows-based upgrader
// tool with a USB micro cable plugged into the "UART" port on the dev board.
// At the time of this writing, here were the steps I took:
//
//  - Make sure that jumper JP4 on the BetzTechnik board is connected. You will need
//    to cut this trace after you're done upgrading, and/or temporarily reconnect it
//    for future firmware updates to the WT32i chip.
//  - Make sure the Bluetooth breakout board is fully disconnected from the ardunio
//    and all of its related electronics modules which are part of this assembly.
//    Upgrade will not work if it is connected to Arduino serial port.
//  - Download and unzip the firmware zip file linked in "Reference Materials" above.
//  - Plug in the USB cable to the bluetooth board in the "UART" port.
//  - Look in Windows device manager to see if the USB cable has made a serial port.
//  - If it has a little yellow boo boo icon instead of a serial port, then...
//  - Install the Prolific PL2303 drivers for the USB-serial connection to the dev board.
//  - Now, plugging in the dev board into the PC with the USB cable makes a serial port
//    appear in the Windows Device Manager.
//  - Once that's sorted, run the "SerialDFU.exe" tool found in one of the unzipped folders.
//  - Make sure the "Get Device Type" button works and shows your device information.
//  - Have it upgrade your chip with the most recent WT32i file among the unzipped files.
//    The full path is:
//       iWRAP_Firmware_Releases
//         Firmware
//          DFU
//           SerialDFU     (SerialDFU.exe is the upgrader tool in this folder)
//            DFU_Images
//              WT32i      (ai-6.2.0-1122_aptxll.bc5.dfu is the firmware file in this folder)
//  - Note: When upgrading, there is a checkbox on the screen in the SerialDFU.exe
//    utility which says "Factory Restore All". Make sure to CHECK that checkbox when doing
//    the firmware upgrade.
//
//  ----------------------------------------------------------------------
//  IMPORTANT - Modifying the Arduino compiler for larger buffer size
//  ----------------------------------------------------------------------
//  For this code to work you must increase the size of the serial port
//  buffers in the Arduino compiler, otherwise some of the track
//  titles will not work. The symptom will be that you switch songs on
//  the empeg, and the track title on the car stereo screen does not
//  change to the new song title.
//  
//  To fix the issue, you must edit one of the header files in the
//  Arduino compiler libraries, and then you must compile and upload
//  your sketch from your local PC using the local Arduino compiler and
//  uploader (the online web editor will not work for this).
//  
//  The file that you need to edit will be the same on all operating
//  systems, but the location of the file will be different depending
//  on which OS you're using. The approximate location will be somewhere
//  around this location, but the install location will vary:
//  
//     (install location)/hardware/arduino/avr/cores/arduino/HardwareSerial.h
//  
//  On a Macintosh computer, the file is harder to find. It is located
//  in the following place on a Macintosh:
//  
//     /Applications/Arduino.app/Contents/Java/hardware/arduino/avr/cores/arduino/HardwareSerial.h
//  
//  Or, more specifically, in the Macintosh Finder, navigate to:
//  
//     Macintosh HD -> /Applications/Arduino
//  
//  Ctrl-click on the application file "Arduino" and select "Show Package
//  Contents". Then navigate to:
//  
//     Contents/Java/hardware/arduino/avr/cores/arduino/HardwareSerial.h
//  
//  Ctrl-click on the file "HardwareSerial.h" and select "Open With" and
//  choose your favorite quick text editor program to edit the file with it.
//  Locate the following code section:
//  
//            #if !defined(SERIAL_TX_BUFFER_SIZE)
//            #if ((RAMEND - RAMSTART) < 1023)
//            #define SERIAL_TX_BUFFER_SIZE 16
//            #else
//            #define SERIAL_TX_BUFFER_SIZE 64
//            #endif
//            #endif
//            #if !defined(SERIAL_RX_BUFFER_SIZE)
//            #if ((RAMEND - RAMSTART) < 1023)
//            #define SERIAL_RX_BUFFER_SIZE 16
//            #else
//            #define SERIAL_RX_BUFFER_SIZE 64
//            #endif
//            #endif
//  
//  First, check to make sure this section of code is getting compiled
//  at all. Insert the following line immediately before the line which
//  says "#define SERIAL_RX_BUFFER_SIZE 64"
//  
//      #error This line of code has been reached successfully
//  
//  Now, in your Arduino compiler program, attempt to verify/compile and
//  upload your sketch to your Arduino. The compilation step should fail
//  and it should clearly show the error message that you just inserted.
//  Now you know that the header file is the correct header file and that
//  your compiler is picking it up and using it.  
//  
//  Now remove the error line from the header file so that it will build.
//  
//  Now, instead, edit the lines "#define SERIAL_TX_BUFFER_SIZE 64" and
//  "#define SERIAL_RX_BUFFER_SIZE 64" and change them to 128 instead:
//  
//      #define SERIAL_TX_BUFFER_SIZE 128
//          ...    ...    ...
//      #define SERIAL_RX_BUFFER_SIZE 128
//  
//  Save the file, compile and upload to your Arduino, and you're done.
//  
//
// ----------------------------------------------------------------------------
// Hardware Connection information (internal board connections):
// ----------------------------------------------------------------------------
// Arduino must be the "Mega" model with three hardware serial ports built in.
// I tried it with an "Uno" model using software serial ports, and it had problems
// where characters sometimes got dropped, and it could not keep up with the data
// rates that I wanted.
// 
// Arduino serial port 1 goes to the empeg's RS-232 port, attached via a MAX232 chip
// circuit (see resources above for schematic). This is necessary because you can't
// connect an actual RS-232 cable directly to an Arduino, the voltage and signaling are
// different. So it must go through the MAX232 circuit.
// 
// Arduino serial port 2 (RX2/TXT) goes to the bluetooth chip's serial port. This is at
// TTL level so the Arduino and the bluetooth chip can connect almost directly with wires
// or traces instead of needing to go through a MAX232 circuit. However, the TX wire from
// the Arudino is running at 5v TTL, and the BlueGiga chip runs at 3v (and our assembly
// will actually have it nominally at 2.5v), you must run the Arduino TX2 output through
// a simple voltage divider to step 5v TTL from Arduino down to 2.5v for the BlueGiga chip.
// This is a simple circuit with just 2 resistors. More details below.
//
//
// Arduino and RS-232 port, critical connections:
// 
// You must roll your own RS-232 circuit, connecting the Arduino hardware serial port 1 TX1/RX1
// pins to a MAX232 as directed by the schematic linked above. Leave the option open to flip them into
// a crossover configuration just in case they don't work straight. I can never keep track of which
// way they're supposed to go, I just get it working correctly on my board and then don't touch it.
// Give it 5 volts from the 5v rail, according to the MAX232 schematic in the resources linked above.
// (There will also be capacitors in that schematic that must be correctly placed too.)
// 
// Empeg tuner connector is used to supply power to the arduino and the rest of the assembly.
// Blue wire on tuner connecter connects to the voltage input pin on the 12v-to-5v step-down transformer
// power supply, and the black wire on the tuner connector connects to the ground input pin on the
// step-down power supply.
// 
// Arduino "5v" pin is connected to the 5v output rail of the 12v-to-5v step-down transformer power
// supply circuit.
// 
// Grounding - Multiple arduino GND pins connected to the output ground rail of the 5v power supply. 
//
// Arduino and Pair mode indicator LED - Arduino pin 50 digital I/O pin, connected to +LED through
// resistor (current of resistor determined by calculator and LED values), then -LED connect to GND.
// Resistor current calculator: http://led.linear1.org/1led.wiz
// Example: If using a blue LED with a 3.2v forward voltage and 20ma current, and the Arduino
// analog power supply from the analog pins will be 5 volts, then use a 100 ohm resistor for this value.
//
// Arduino and Button - Arduino pin 52 digital I/O pin, connected to one of the ground legs of button.
// Other ground leg of button goes through 10k pulldown resistor to ground. One of the + legs of
// the button connects to +5v. Follow examples on the internet of how to implement
// a simply temporary pushbutton on an Arduino: https://www.arduino.cc/en/Tutorial/Button
//
// BlueGiga Bluetooth WT32i chip+board, critical connections:
//
// Bluetooth chip+board "VBus" or "5v" power pin connected to 5v rail from the step-down power supply.
//
// Bluetooth chip+Board "Gnd" pin connected to the ground rail. Additional GND pins are good too,
// to make sure solid ground is achieved. 
//
// Bluetooth chip+board "RX/TX" pins connected to the "TX2/RX2" pins (serial port 2) of Arduino
// Note: This is a crossover connection, i.e., RX from the Arduino is connected to TX on the dev board.
// On the line that runs from the Arduino TX2 pin to the RX pin on the Bluetooth chip, add a voltage
// divider circut by using two 10k resistors in the following configuration:
//
// Connect one of the 10K resistors to the TX2 pin from the Arduino, connect the other 10K resistor to
// GND, and connect the other leads of each resistor to each other. The RX-input of the WT32i then
// gets connected to that same point (where the two resistors are tied together).
//
// Self jumpers and switches on the Bluetooth chip+board, the BetzTechnik WT32i Breakout board V2:
//
//   IMPORTANT: Must cut the jumper at "J4 FTDI_+5v" but only do this AFTER successfully
//   updating the chip's firmware to the latest version. Cutting the jumper shuts off
//   the the onboard UART and prevents the UART and the Ardino pin-to-pin serial connection
//   from arguing with each other. This prevents errors on the serial port which cause the
//   chip to reboot randomly.
//
//   "Smd_2_pole_switch" set to the left or up position, to put it into always-on mode.
//   The red LED on the breakout board will blink randomly when power is applied.
//   Symptoms of this switch being set wrong will be: LED on the breakout board
//   blinks steady when power is applied instead of randomly, and there is no serial
//   port communication, and you need to press the reset button on the board to start up the module.
//
// 
// ----------------------------------------------------------------------------
// Hardware Connection information (external):
// ----------------------------------------------------------------------------
// Debugging console to Arduino (during software development): USB cable
// connected from computer to Arduino USB port.
// 
// Power to Arduino: For debugging mode, the USB cable from the computer can
// power the Arduino and the bluetooth board. When connected to the empeg,
// supply power to the 12v power input for the power supply (implemented
// as a 12v->5v DC-DC power converter as part of this assembly) via the
// Tuner connector.
// 
// Note: Do not try to use direct 12v from the car to power the Arduino into the
// Arduino board's power 5.5mm barrel plug connector, because the Arduino can't
// actually handle direct 12v power from the car despite what its specs say.
// Instead, use the 12v-to-5v step-down DC-DC transformer included as part
// of the final design, with 12v +/- connections supplied and connected to the
// tuner connector.
// 
// Power to BlueGiga board+chip: The BlueGiga breakpout board gets its power from
// the 5v rail that comes from the 12v-to-5v step-down power supply.
//
// RS-232 serial port connector connected to the serial port on the empeg Car player
// via its car docking sled.
//
// Audio: connects to I2S data coming from the empeg over modified wires on the
// tuner connector. More details about that below.
//
//
// ----------------------------------------------------------------------------
// Special note about empeg car power connection:
// ----------------------------------------------------------------------------
// I might recommend that, for this installation, you wire up the Empeg to your
// car differently than it would normally otherwise be wired up.
//
// This design is intended to be used in such a way so that the empeg is
// not the primary stereo in the system. The empeg becomes one of the bluetooth
// inputs into a modern car stereo. So the empeg sleep mode, the empeg "always
// on" power connection, and the way this bluetooth module gets its power from
// the empeg tuner connector, all combine to cause some interesting problem with
// power state transitions. At least they did in my car, your mileage may vary.
//
// I found that if I connected the empeg to my car via the regular method (i.e.
// the ignition wire to the orange wire on the sled, and constant 12v power to
// the yellow wire on the sled) then there were certain unusual states that it
// could get into, where I might sometimes shut off my car but the empeg would
// come back up out of sleep mode and play tracks silently to an unconnnected
// bluetooth module.
// 
// If this happens to you, then I recommend connecting it like this:
// - Car power 12v accessory power -> Connect to yellow "memory" wire on empeg.
// - Connect that same 12v accessory power to orange "ignition" wire on empeg.
// - Do not connect your car's constant 12v power to any part of the empeg.
// - Connect your car's illumination wire to the empeg's illum. sense wire.
// - Connect ground to ground of course.
// That should be all you need. No audio or amplifier connections because the
// bluetooth takes care of that now. No cellphone mute wire because the
// bluetooth takes care of that now. No amp remote wire because there is no
// amp. No FM tuner or aux connection, etc, etc... 
//
// In this situation, the empeg does not have a 12v constant power concept
// at all and will not go into sleep mode when you turn off the ignition.
// However it, and the bluetooth, will always power on and off at the same
// time with the car's headunit and nothing will get confused. You might lose
// the clock/date/time information on the empeg sometimes but your modern car
// likely has a perfectly functional clock of its own.
//
// Finally, you'll notice that the empeg sled connector only supplies power
// if the empeg is awake. If you put the empeg into Sleep mode via a long
// press on the top button, it will go to sleep but that also means the 
// Arduino+bluetooth assembly stops receiving power. This is actually good:
// you want to be able to turn the bluetooth on and off and reboot it
// sometimes, and it's easy to do that just by sleeping or waking the player
// by hand. In my case I leave the player awake most of the time, so that
// when I shut off the car ignition, the player fully shuts down, and then
// when I start my car again, the empeg starts up in Awake mode and
// immediately supplies power to the bluetooth assembly which then autoconnects
// to the car stereo head unit.
//
//
// ----------------------------------------------------------------------------
// Empeg Car Interior Modification for Digital I2S connection:
// ----------------------------------------------------------------------------
//
// You will be modifying the interior of your empeg Car so that there can be
// three wires coming out the docking connector which carry digital audio 
// data ("I2S" aka "IIS") and can connect to this external module. This is done
// by hijacking three of the wires originally used for the FM Tuner Module
// and its molex connector. This tuner module connector will instead plug
// into my electronic assembly which contains the bluetooth chip and the
// Arduino.
//
// IMPORTANT: Tuner module will no longer work after this modification.
// If you have any tuner modules, disconnect all of them from any and all
// sleds that you own. Damage may occur to your empeg and your tuner module
// if you dock your modified empeg to any tuner module after you make this
// modification.
//
// Disassemble your empeg player by carefully removing the fascia, lid and
// drive tray. Refer to the empeg FAQ for disassembly instructions. You must
// do the disassembly carefully so as not to damage the empeg. You should
// theoretically be able to do this without disconnecting the IDE cables
// but it might be easier if you do. You should not need to remove the
// display board.
//
// Locate 5 blank IIS pads on the front right side of the empeg motherboard. 
// Pads are outlined in a white silkscreen rectangle with the letters "IIS"
// next to the outline. White silkscreen rectangle will have one corner of
// it with a small diagonal "snip" to the corner which will indicate pad 
// number 1 of the set of 5 pads. This diagonal snip will be towards the back
// of the empeg, and the remaining pads continue towards the front of the
// empeg. Pads are not individually labeled with numbers, but understand that
// they are logically numbered 1 2 3 4 5 starting from the back and going
// towards the front of the empeg. The pinouts of these five pads are:
//     1 = IISC  aka  SCK     aka "serial clock"
//     2 = IISW  aka  WS      aka "word select"
//     3 = GND   aka  Ground  same as chassis ground of empeg
//     4 = IISD1 aka  SD      aka "serial data" (1 of 2)
//     5 = IISD2 aka  SD      aka "serial data" (2 of 2)
// Carefully solder some small jumper wires to pads 1,2, and 4, and you must
// keep track of which wires are soldered to which pads.
//
// When soldering, make sure the jumper wires and the solder are flat to the
// board instead of sticking upwards. The disk drive tray gets close to that
// location when mounted, so make sure they don't have a chance to contact
// the tray. Take a look at Stu's instructions for his digital sound board
// for the empeg, which utilizes these same three connections on the IIS pads
// for an example of how to solder these three wires correctly:
//        http://www.eutronix.com/media/dig-cxxxx_manual.pdf
// After soldering, cover the solder points with tape or some other insulator
// to prevent them from shorting out on the drive tray.
//
// Locate the two white connectors on the back part of the empeg motherboard 
// which connect two groups of small colored wires to the docking connector
// assembly. Locate the rightmost of the two connectors nearest the Ethernet
// port on the back of the empeg. Unplug this connector and using a tiny flat
// tool such as the tip of a jeweler's screwdriver, lift up the small white tabs
// and gently disengage and pull out the rightmost three pins nearest the
// Ethernet connector to free up those wires. On my unit, these wires were
// colored, going left to right, yellow+green, red, and brown, though your unit
// may differ. The brown wire is the one on the end closest to the Ethernet
// connector, the red wire is the second from the end closest to the Ethernet
// connector, and the yellow+green wire is the third closest to the Ethernet
// connector.
//
// You should now have three loose wires inside the empeg, coming out of the 
// docking connector assembly, with small pin connectors on the ends of them.
// Carefully solder and shrink-tube your three jumper wires to directly these pin
// connectors. After you have done this, your empeg's IIS pads will, when docked,
// now be carried out the back of the empeg, via the docking sled, into the tuner
// module molex connector. Solder your jumper wires to the empeg interior wires
// in this order:
//    IIS Pad 1 (IISC)  jumpered to Yellow+Green wire, originally third from end.
//    IIS Pad 2 (IISW)  jumpered to Brown wire, originally the end wire.
//    IIS Pad 4 (IISD1) jumpered to Red wire, originally second from end.
// 
// Make sure that your jumper wires are carefully tucked down around the left
// side of the empeg and out of the way of the disk drive tray assembly when you
// reinstall it. 
//
// Final wiring positions and colors:
//
// Empeg IIS pads   My Jumper wires*  Int. Empeg wires**   Int. wht Conn pos***   Sled Tuner Plug   BlueGigaEmpeg Tuner Plug    Bluetooth Chip   Usage on BlueGigaEmpeg Assembly
// 1 IISC            Yellow            Yellow+Green         Third from end         7 Purple          7  SCK                      30  PCM_CLK      Serial Clock
// 2 IISW            Orange            Brown wire           Right end              2 Grey            2  WS                       29  PCM_SYNC     Word Select
// 4 IISD1           Red               Red wire             Second from end        1 Pink            1  SDIN                     27  PCM_IN       Serial Data
//                                                                                 4 Black           4  GND                          GND          Universal Ground
//                                                                                 8 Blue            8  12vPower                                  Power to voltage regulator
//
// *(These are just the colors I used for my jumper wires, yours may differ.)
// **(These were the colors inside my empeg, yours may differ. These are the interior wires connecting the docking sled connector to the motherbopard.)
// ***(Original positions of these wires on the white connector near the Ethernet plug, now disconnected from that plug.)
//
// Finally, make sure that the variable "btAudioRoutingControlString", which is
// set elsewhere in this code, is configured to be the one which uses the I2S digital
// audio instead of the analog audio.
//
//
// ----------------------------------------------------------------------------
// Empeg Car Configuration information:
// ----------------------------------------------------------------------------
// I have tested this with empeg Car firmware version 2.00 Beta 13. I do not
// know if it will work the same on 3.0 Alpha version of empeg Car firmware.
//
// Using the Emplode or Jemplode software, edit the empeg Car's config.ini.
// 
// If you are using Emplode on Windows, you must first edit the Microsoft
// Windows registry before it will allow you to alter the config.ini. First,
// run REGEDIT.EXE in windows and locate the following key:
//    HKEY_CURRENT_USER
//      Software
//        SONICblue
//          emplode
//            2.0
//              Settings
// Inside the "Settings" key, create a new DWORD value named
//                allow_edit_config
// with a value of
//                1
// then restart the Emplode software. Emplode will now have a menu
// option which allows you to edit the config.ini on the player.
// Change the settings as follows (adding sections if necessary):
//
//  [Hijack]
//  suppress_notify=2
//
//  [Output]
//  notify=1
//
//  [Serial]
//  car_rate=115200
//
// Make sure to synchronize with the player after changing the config.ini settings.
//
// Install the latest Hijack Kernel onto the empeg car player if it is not already
// installed (link to Hijack in "Resources", above). Make sure it is Hijack version
// 522 or later.
//
// Open up the Hijack kernel settings on the player via a longpress on the knob.
// Change "Serial port assignment" to "Player uses serial port" and follow the
// instructions to reboot the player.
//
//
// ----------------------------------------------------------------------------
// Notes about debugging via the Arduino USB connection - Startup sequence
// ----------------------------------------------------------------------------
// Your mileage may vary, but on my system there is an interesting trick with
// debugging via the Arduino USB cable and the Arduino Serial Monitor.
// 
// When everything is connected together (empeg, arduino, bluetooth), the assembly
// and all devices which are part of the assembly will get their power off of the
// 12v power coming off the tuner connector on the empeg sled. This means that
// if you plug all this stuff in, it's already powered by the time you try to 
// attach your USB debug cable to Arduino. 
//
// On my computer system, if the arduino is already externally powered when I
// connect my debug cable, then my computer cannot find its USB UART and is
// unable to connect to the debug port. 
//
// So to successfully debug via the Arduino USB debug cable and use the Arduino
// Serial Monitor program, I must connect the USB cable *first*, before applying
// power to the empeg. Meaning if I am debugging in the car, I should connect
// the USB cable first and then turn on the ignition, that way the Serial
// Monitor program can find a port to connect to.
//
// When using the Arduino Serial Monitor program, set it to 115200 baud,
// with either Newline or "Both NL and CR" as line endings. If you are using a
// different serial terminal program, set it to 115200 8n1 and turn on local
// echo.

// ----------------------------------------------------------------------------
// Notes about pairing and bluetooth PIN codes
// ----------------------------------------------------------------------------
// This device implements a RESET/PAIR button which does the following:
//
//  - Erases all previous pairings
//
//  - Looks for devices to pair with for 30 seconds and pairs with
//    the first one that it sees.
//
//  - The RESET/PAIR LED is lit during this pairing mode.
// 
// But, this is counterintuitive: Sometimes you don't press that RESET/PAIR
// button at all, and instead you pair when the RESET/PAIR LED is *not* lit.
//
// There are some stereos which need you to initiate pairing solely on the
// car stereo's touch screen. Mine is one of these types of stereos. In my
// case, for initial pairing with my car stereo the first time, I don't
// press the RESET/PAIR button at all. I just go to my car's touch screen,
// tell it to pair with the bluetooth device named "empeg Car", and it works.
// After that, it reconnects automatically each time I get in the car and
// start it up (though it takes a moment to connect).
//
// Some stereos and headsets *do* need you to press that RESET/PAIR button,
// which should light the LED for 30 seconds, and then you should put your
// headset or stereo into pair mode while the LED is still lit.
//
// If one method doesn't work for you, try the other method. For example
// this sequence will work for many stereos and headsets:
//
//  - Press the RESET/PAIR button, and while the LED is still lit,
//    then initiate pairing at your stereo or headset via its
//    pairing feature.
//
//  - If that doesn't work, then wait for the RESET/PAIR LED to go dark
//    and then, without touching the RESET/PAIR button, try to initiate
//    pairing with your stereo or headset via its pairing feature.
//
// Special note about bluetooth PIN codes:
// 
// The code defaults to Secure Simple Pairing in "Just Works" mode, which
// does not require a PIN code to successfully pair. It will fall back to
// using a PIN code if the Secure Simple Pairing doesn't work. This should
// be good for most devices. However the PIN code defaults to "0000", and
// though that should work with most devices, if your pin code happens to
// be different from "0000", then you should change the code in this sketch.
// Fine the line in the code below that defines the variable
// "btPinCodeString" - that line contains the "0000" default PIN code.
// Change it to your correct PIN code for your device. It can accept PIN
// codes up to 16 digits long.
//



// ----------------------------------------------------------------------------
// Program code starts here
// ----------------------------------------------------------------------------

// ------------------------------------
// Global variables and definitions
// ------------------------------------

// String to control the type of bluetooth authentication that you want to use.
// This is currently configured for "just works" pairing with no PIN code,
// with a fallback to the PIN code if the "just works" mode fails.
const String btAuthTypeString = "SET BT SSP 3 0";

// If the device falls back to using a PIN code, here is where to set the PIN.
// Change "0000" to your correct PIN code for your stereo. It can accept PIN
// codes up to 16 digits long.
const String btPinCodeString = "SET BT AUTH * 0000";

// Variable to control whether or not this program performs a conversion of
// High ASCII to UTF-8 in the code. For instance, on the empeg, you might have
// a track by "Blue Öyster Cult", with the "Ö" being represented by a High
// ASCII character (a single ASCII byte with a value greater than 127). In that
// situation, you might see the the car stereo's LCD toucscreen display say
// "Blue ?yster Cult". With this variable set to "true" it will instead convert
// that to UTF-8, which should be readable by your car stereo touch screen.
// If you encounter problems with some track titles which might already be
// encoded as UTF-8 on your empeg, then try setting this value to "false"
// and see if that fixes it.
//   Setting true:
//      - Character values between ASCII 128 and ASCII 255 in the track metadata
//        on the empeg will be converted to their UTF-8 equivalents before being
//        sent up the bluetooth chain to the car stereo.
//   Setting false:
//      - No UTF-8 conversion will be performed and the track metadata is sent
//        from the empeg to the car stereo without any changes.
boolean PerformUtf8Conversion = true;

// Debugging tool for the part of the code that sends commands to the Empeg.
// Normally, typing commands into the Arduino debug console will send those
// same characters to the bluetooth chip, so that you can try out commands
// on the bluetooth chip. The "EmpegSendCommandDebug" flag, below, expands
// on this when the flag is enabled (set to "true") in the following way:
// When in this mode, typing any of the empeg command characters (N P C W) into
// the Arduino debug console will trigger the code to send that command to the empeg.
// Since all of your typing gets sent to the bluetooth chip, as well, it's possible
// that typing one of these empeg commands by itself will cause "SYNTAX ERROR" to be
// echoed back by the bluetooth chip. That's OK, no worries, you probably didn't hurt
// it. This is only needed for software development, not needed for final build runtime.
//   Setting true:
//           - Typing N, P, C, or W into the Arduino main serial port debug console,
//             even if they are included as part of another command such as a command
//             sent to the bluetooth, will result in the N, P, C, or W command being
//             sent to the empeg (as well as being sent to the bluetooth chip).
//   Setting false:
//           - Commands typed in the Arduino main serial port debug console will go
//             solely to the bluetooth chip and will not be echoed to the empeg.
// This should be set to false most of the time, and only set to true during
// debugging sessions
boolean EmpegSendCommandDebug=false;

// Choose whether or not to display the Empeg Serial Port outputs (for instance
// the empeg boot up messages) on the serial debug console of the Arduino. Only
// needed for debugging, not needed for final build runtime.
//     Setting true:
//           - Characters received from the Empeg Car serial cable are displayed
//             on the Arduino main serial port debugging console terminal screen.
//             It is mixed in with all the other serial output from the bluetooth
//             chip all at the same time, so it might be messy if the bluetooth
//             chip and the empeg are saying things at the same time.
//     Setting false:
//           - No echoing of the Empeg Car serial cable output occurs on the debug
//             screen (though the empeg output is still interpreted and acted upon
//             by the software, it's just not displayed on the debug screen).
// This should be set to false most of the time, and only set to true during
// debugging sessions, since it slows down processing to put out too much
// data on the serial port when it is not needed.
boolean displayEmpegSerial=false;

// Choose whether or not to display the interpreted track metadata on the
// Arduino debugging serial/USB console port.
//      Setting true:
//           - Each time there is new track metadata collected, a block of
//             text displaying all of that metadata gets displayed on the
//             Arduino debugging console.
//      Setting false:
//           - Track metadata is processed normally but the text blocks
//             listing all the metadata in one group is not displayed on
//             the Arduino debug serial/USB console.
// This should be set to false most of the time, and only set to true during
// debugging sessions, since it slows down processing to put out too much
// data on the serial port when it is not needed.
boolean displayTracksOnSerial=false;

// Experimental - When we see the empeg player application start up, then send a
// pause command to the player. If the bluetooth initial connection speed is faster
// than the empeg bootup speed, then this will mean things start up in pause mode.
// However if the empeg finishes bootup and starts is player application before
// the bluetooth connection process is complete, then the act of connecting the
// the bluetooth will issue the commands to start playback, so things will start
// up playing. This is an experiment to fix some initial connection behavioral problems.
// Would prefer to not to have to use this at all, so try it with it turned off first.
boolean empegStartPause = false;

// Arduino serial port 2 is connected to Bluetooth chip
#define BlueGigaSerial Serial2 

// Arduino serial port 1 is connected to empeg's RS-232 port via MAX232 circuit.
#define EmpegSerial Serial1

// Experimental: String to tell the unit to automatically try reconnecting every few seconds
// if it ever becomes disconnected from its main pairing buddy. Trying to make it grab hold
// of the car stereo as soon as the car turns on, instead of connecting to my cellphone every time.
// NOTE: This includes a baked-in second additional command to store the configuration of the
// connection. The docs say that this STORECONFIG command is merely to store the reconnect
// functionality but it's not made clear if it stores all configuration information or just that one.
// ALSO NOTE: The docs say that the reconnect timer (the first numeric parameter) should be
// *longer* than 500ms.
const String autoReconnectString = "SET CONTROL RECONNECT 600 0 0 7 0 A2DP A2DP AVRCP\r\nSTORECONFIG";

// Debug version of reconnect command for testing and comparison.
//      const String autoReconnectString = " ";

// Matrix of special case fix strings and their output timings.
// When a bluetooth statement is received from the bluetooth module,
// this code will respond back to the bluetooth module with the
// specified text. These are "plain" response strings without any
// detailed parsing, they are just input->output pairs for answering
// certain pieces of text on the bluetooth module's serial port.
// There is other code elsewhere to handle input/output that requires
// more detailed handling and parsing. 
int scFixMatrixSize = 8;
String scFixMessageMatrix[8][2] =
{
    // Bluetooth in                        // Bluetooth out                         
  { "AVRCP PLAY PRESS",                   "A2DP STREAMING START"},

  // Attempting to fix some issues where there is silence on the line after
  // first connection with some devices. Attempt to make sure the streaming
  // is started when there are messages that indicate we have a connection
  // of some kind.
  { "CONNECT 2",                          "A2DP STREAMING START"},
  // { "PDU_REGISTER_NOTIFICATION",          "A2DP STREAMING START"}, // This one didn't work or help

  // We issue a "Streaming start" command (above) when we get "play press",
  // because that fixes a whole bunch of issues with various cases where the
  // audio stream isn't connected when music is playing. It works. However,
  // don't do the corresponding thing for "streaming stop", because it causes
  // a problem where, when you unpause a song, there is a slight delay while
  // streaming starts up again, so, if you unpause at the 00:00 mark of a
  // song, then the beginning of the song is cut off. I am leaving this line
  // in here, commented out, to remind myself not to ever try to do this.
  //        { "AVRCP PAUSE PRESS",                  "A2DP STREAMING STOP"},

  // Get Capbailities 2 is asking for manufacturer ID. According to the 
  // BlueGiga documentation the chip will fill in the response details
  // for manufacturer ID automatically if you give it a blank response.
  // (By "blank response" I mean, responding with "AVRCP RSP" with no
  // further parameters following after the "RSP")
  { "GET_CAPABILITIES 2",                 "AVRCP RSP"},

  // Get Capabilities 3 is asking for which status notifications my bluetooth
  // chip wants to receive. In this case track/status changes (codes 1 and 2,
  // those codes were obtained from the BlueGiga documentation).
  { "GET_CAPABILITIES 3",                 "AVRCP RSP 1 2"},

  // "LIST_APPLICATION_SETTING_ATTRIBUTES" is how the host stereo queries the
  // bluetooth module to find out if your device supports things like turning
  // on the EQ, turning shuffle on and off, and turning repeat on and off. This
  // is the generalized query to find out what attributes are supported by your
  // playback device at all, it is not asking for individual setting values.
  //
  // At the current time I want to respond with "none" to indicate that I have no
  // settings that I want to be changeable from the host stereo's user interface.
  // However, the documentation for the BlueGiga module isn't clear on how to
  // respond with "no" or none" so that the host stereo never tries to change our
  // settings for us. I've tried responding with "blank", i.e., "AVRCP RSP", and
  // also with a 0, i.e., "AVRCP RSP 0". Both methods seem to give approximately
  // the same results which is that on some host stereos it seems to work and
  // make the host stereo stop querying for settings changes, whereas on others
  // there are still additional queries I must respond to (see next entry below).
  //
  // At some point in the future I may expand this and allow for certain settings
  // to be changeable, at which point I will edit this response in more detail.
  // Since "settings" encompasses Shuffle and Repeat in the bluetooth spec, then
  // someday we might be able to do more detailed work with this response so that
  // we can interpret commands to change the Repeat mode and the Shuffle mode on
  // the empeg. 
  { "LIST_APPLICATION_SETTING_ATTRIBUTES",  "AVRCP RSP"},

  // "LIST_APPLICATION_SETTING_VALUES" is the way that the host stereo queries
  // your bluetooth module for specific indvidual setting values. Not generally
  // whether a particular settings group is supported at all, but rather, looking
  // specifically for the exact setting of this particular thing it's querying.
  // If I responded to the application setting attributes with a blank or 0
  // response, my personal opinion is that the host stereo shouldn't then turn
  // around and ask me for specific setting values, since I just told it "no".
  // But guess what the Kenwood stereo does? It asks me for setting values anyway.
  // Sigh. Respond to them with this entry below. Again, the BlueGiga documentation
  // does not make it clear how to respond with "no!". So I am also trying doing
  // this with either blank or 0, like above. I'm not sure which one works and I'm
  // still not certin how to respond to queries like this with a "no".
  { "LIST_APPLICATION_SETTING_VALUES",      "AVRCP RSP"},
 
  // Respond to "RING 1" with a streaming start command seems to help a lot of
  // initial device connection situations when host stereos first connect to
  // our bluetooth module. Some initiate streaming automatically, and some do
  // not, and it doesn't hurt to re-issue the command to initiate streaming,
  // to go ahead and re-start the stream each time. If the stream is already
  // established, no interruption occurs, it just keeps playing.
  { "RING 1",                              "A2DP STREAMING START"},

  // Experiment - Respond to certain AVRCP connection success messages with a command
  // that is supposed to force the bluetooth to repeatedly retry connections if it
  // ever becomes disconnected. Hopefully this will increase the chances that
  // the empeg connects to the car stereo when you start the car, instead of connecting
  // to your phone in your pocket. On my stereo, when this system works, it results
  // in the perfect combination of the phone pairing to the car as a phone only
  // (no music, just phone), and the empeg pairing as the music source, simultaneously.
  { "AUDIO ROUTE 0 A2DP LEFT RIGHT",          autoReconnectString},

  // Additional ones that may or may not be needed, not certain - I had a memory
  // problem and was trying to reduce the size of this matrix so these are
  // commented out as part of an austerity measure. TO DO: Check to see if they
  // are needed on other systems. The "AUDIO ROUTE" message above might be enough
  // and these would be just extra repeats. However I'm not certain that the 
  // "AUDIO ROUTE" command appears every time after a successful connect or not.
  // { "CONNECT 0 A2DP 19",                      autoReconnectString},
  // { "CONNECT 1 A2DP 19",                      autoReconnectString},
  // { "CONNECT 2 A2DP 19",                      autoReconnectString},
  // { "A2DP STREAMING START 0",                 autoReconnectString},
};

// Strings that will be handled by our more detailed call-and-response
// routine, "RespondToQueries", later in the code. These strings require
// more detailed processing than just "string in, string out", so this
// is merely a preliminary checklist that indicates whether a string needs
// a detailed response at all, to speed up string processing later. So this
// is merely a small list of possible strings that might need detailed
// responses if these strings are received. There is a piece of code later
// in the main string handling routine which will check to see if the string
// matches one of these list items, and if it does, it will pass it on to the
// function that will do the actual detailed responding. NOTE: if you add a
// new response string to this list, then also add the necessary detailed
// response prorgramming to the "RespondToQueries" function.
int rspMatrixSize = 5;
String rspMatrix[5] =
{
  "PLAYBACK_STATUS_CHANGED",
  "TRACK_CHANGED",
  "GET_PLAY_STATUS",
  "GET_ELEMENT_ATTRIBUTES",
  "PDU_REGISTER_NOTIFICATION",
};  

// Table of events that we will reject when we received a PDU_REGISTER_NOTIFICATION
// request for these particular events. The event number for each one is specific and
// can be found in section 6.3.3 of the BlueGiga iWrap AVRCP command reference documentation.
int rejMatrixSize = 11;
String rejMatrix[11][2] =
{
  {"TRACK_REACHED_END",                   "3"},
  {"TRACK_REACHED_START",                 "4"},
  {"PLAYBACK_POSITION_CHANGED",           "5"},
  {"BATT_STATUS_CHANGED",                 "6"},
  {"SYSTEM_STATUS_CHANGED",               "7"},
  {"PLAYER_APPLICATION_SETTING_CHANGED",  "8"},
  {"NOW_PLAYING_CHANGED",                 "9"},
  {"AVAILABLE_PLAYERS_CHANGED",           "10"},
  {"ADDRESSED_PLAYERS_CHANGED",           "11"},
  {"UIDS_CHANGED",                        "12"},
  {"VOLUME_CHANGED",                      "13"},
};

// Reserve variable space for the transaction status label used to hold a
// particular string in memory in the query/response code. The host stereo
// sends a query which contains a specific transaction label, which is a
// number that is usually just a single hexadecimal digit, and it uses this
// digit as an identification so that it can send several queries and await
// several responses and can sort out which response is from which query.
// So we must respond with that same transaction label digit in our response.
// Though I've only seen it be one digit, we are reserving two bytes here
// just in case it gets bigger.
int transactionLabelSize = 2;
static String transactionLabel = "";

// Specific transaction labels to use for specific types of messages.
// These are part of the fix to what I called the "Kenwood Bug" where the
// Kenwood stereo refused to query for any new track titles past the first
// track. These will also be using the memory reservation size of "2"
// defined above since they are copies of the same transaction label.
static String transactionLabelPlaybackStatusChanged = "";
static String transactionLabelTrackChanged = "";

// Reserve memory space for the AVRCP response string we will be sendin
// a response in the query/response handler code. Size of string can get
// big because it may contain long track titles as well as the preamble
// string before the track title, so save some extra space for this string.
// In fact, the string could potentially be this long if the stereo happens
// to query the bluetooth chip for all track metadata elements all at once
// and each element is the maxiumum length that each possible element can be:
// "AVRCP RSP 7 "                       = 12 characters, the response preamble and the number of elements.
// "4 "9999" 5 "9999" "                 = 18 characters, the track number and total number of tracks.
// "7 "4294967295" "                    = 15 characters, the current playback position in MS in Decimal.
// "1 "TrackTitle255" "                 = 260 characters, the track title up to 255 characters plus ID, quotes and spaces
// "2 "TrackArtist255" "                = 260 characters, the track artist up to 255 characters plus ID, quotes and spaces
// "3 "TrackAlbum255" "                 = 260 characters, the track album up to 255 characters plus ID, quotes and spaces
// "6 "TrackGenre255" "                 = 260 characters, the track genre up to 255 characters plus ID, quotes and spaces
// Grand total largest possible string  = 1080 characters. It will never get this big unless something is crazy. 
int queryResponseStringSize = 1080;
static String queryResponseString = "";

// Matrix of messages and responses which are needed when in special pairing mode.
// These also include some special casing for the bluetooth device address. The
// "{0}" in these strings are a special cased token indicating that we should insert
// the bluetooth device address into that location of the response string. The "{0}"
// is also defined in another variable below.
// NOTE: Update the matrix size and the array size both, if you are changing these.
int pmMatrixSize=3;
String pmMessageMatrix[3][2] =
{
  // Respond to messages such as INQUIRY_PARTIAL 0c:e0:e4:6c:75:68 240404
  { "INQUIRY_PARTIAL ",          "PAIR {0}"},
  
  // Respond to messages such as "PAIR 0c:e0:e4:6c:75:68 OK"
  // Respond during the pair process with a connection attempt.
  // Response should be "CALL", the address to connect to, then
  // a special code indicating the target type and profile type.
  // "19" is a special code for a certain kind of target which is
  // a particular secret-code hard-to find value called an
  // "L2CAP psm" and the special secret L2CAP psm for A2DP is "19".
  { " OK",                       "CALL {0} 19 A2DP"},  
  
  // Respond to messages such as "CONNECT 0 A2DP 19"
  // Respond during the pair process with a connection attempt.
  // Response should be "CALL", the address to connect to, then
  // a special code indicating the target type and profile type.
  // "17" is a special code for a certain kind of target which is
  // a particular secret-code hard-to find value called an
  // "L2CAP psm" and the special secret L2CAP psm for AVRCP is "17".
  { "CONNECT 0 A2DP 19",        "CALL {0} 17 AVRCP"},
};

// String to be used in token substitutions above (change both the matrix above
// and also the string below if you change the tokenization flag string).
const String tokenSubstitutionString = "{0}";

// Length of time to be in pairing mode before giving up.
// Make sure to change both variables below. The first variable
// is in milliseconds and the second variable is in seconds. Note:
// Can't be bigger than 32768 milliseconds (32 seconds).
int pairTimeMilliseconds = 30000;

// String to send to the bluetooth chip when we begin pairing mode
// by pressing the reset/pair button on this electronic assembly.
// This is just a command to scan the air for other bluetooth devices
// for xx seconds (number should match milliseconds above). Then the
// program code below will listen for responses to this device inquiry
// and will respond appropriately in the appropriate pairing section
// of the code. The reset/pair LED will be lit up blue during this time.
// NOTE: This pairing feature has only been useful to me for pairing
// up with devices which do NOT have a user interface. For example,
// I have to use this pairing feature with a bluetooth headset. But
// on my car stereo, (which has a detailed touchscreen user interface
// for pairing) I can't actually pair with the device when I'm in
// this pairing mode. I have to wait until this is done (or don't
// trigger it at all to begin with) and THEN I will be able to pair
// with the device from my car stereo's front panel.
const String pairBeginString = "INQUIRY 30";

// String that I am using to detect that the pairing process has completed
// and to stop responding to pairing-related messages coming on the serial
// port from the bluetooth chip. And to turn off the blue pair/reset LED.
const String pairDetectionString = "AUDIO ROUTE ";

// Global string which will be used for storing special values from
// parsed responses (most likely bluetooth device addresses from the
// pmMessageMatrix above) which will then be saved in this global
// variable and then re-used elsewhere, for example, receiving the
// inquiry bluetooth address and then sending the Pair command back
// with the bluetooth address in the response. The address will be
// saved in this string.
static String pairAddressString = "";
int pairAddressStringMaxLength = 25;

// The translation table of AVRCP messages to Empeg serial commands.
// This is the commands that we must send to the empeg serial port
// if certain messages come in from the bluetooth module.
// NOTE: Update the matrix size and the array size both, if you are changing these.
int empegCommandMatrixSize = 12;
String empegCommandMessageMatrix[12][2] =
{
  // Bluetooth Module reports     // Send command to empeg
  { "AVRCP PLAY PRESS",           "C"},
  { "AVRCP PAUSE PRESS",          "W"},
  { "AVRCP STOP PRESS",           "W"},
  { "A2DP STREAMING START",       "C"},
  { "NO CARRIER ",                "W"},
  { "A2DP STREAMING STOP",        "W"},
  { "AVRCP FORWARD PRESS",        "N"},
  { "AVRCP BACKWARD PRESS",       "P"},  
  { "AVRCP FAST_FORWARD PRESS",   "F"},  
  { "AVRCP FAST_FORWARD RELEASE", "A"},  
  { "AVRCP REWIND PRESS",         "B"},  
  { "AVRCP REWIND RELEASE",       "A"},
};

// Global variables to hold the track metadata information for the currently-playing
// track on the empeg Car, so that they can be in the responses to the queries from
// the host stereo to the bluetooth chip. The numbers in the variable names (for
// example the "01" in "trackTitelString01" is a reminder to myself of which element
// attribute code in the bluetooths spec reprsents which metadata. For instance
// attribute code "1" is "Track Title", atrribute code "2" is "Artist", etc.
// Note that these are all strings because the docs and specification say that they
// must be sent up the stream as strings specifically. Depending on which response
// that you're responding to, sometimes some of these strings must be quote-enclosed.
// Documentation says that the maximum length for each one is 255 bytes. I'll use
// two string length limiters to save memory on the arduino: A long one for the track
// metadata strings, and a short one for things like track number and playback position.
int metadataMaxLength = 255;
int metadataSmallLength = 10;
static String trackTitleString01 = "Unknown track on empeg Car";
static String trackArtistString02 = "Unknown artist on empeg Car";
static String trackAlbumString03 = "(Album N/A)";
static String trackNumberString04 = "00";
static String trackTotalNumberString05 = "99999";
static String trackGenreString06 = "Unknown genre on empeg Car";
static String trackPlaybackPositionString07 = "00";
static String trackPlaybackLengthString="999999";
long trackPlaybackPositionMs = -1L; // Initialize to flag value meaning "unknown"

// String to control the audio routing on the chip. 
// Use this string if you are using the Line In jacks on the bluetooth device for audio (analog):
// const String btAudioRoutingControlString = "SET CONTROL AUDIO INTERNAL INTERNAL EVENT KEEPALIVE";    
// Use this string if you are using the I2S aka IIS connection on the bluetooth device
// (this is a special digital connection which requires modifying inside of empeg Car player):
const String btAudioRoutingControlString = "SET CONTROL AUDIO INTERNAL I2S_SLAVE EVENT KEEPALIVE 16";

// Strings to set the gain level of the Empeg. See bluetooth chip docs for gain level table.
//
// Uncomment this line if your player will be used in AC/Home mode, (1v outputs): 
// const String empegGainSettingString = "SET CONTROL GAIN E 0";
//
// Uncomment this line if your player will be used in DC/Car mode, such as in a sled (4v outputs):
const String empegGainSettingString = "SET CONTROL GAIN 9 0";

// Strings to hold an incoming line of serial characters from bluetooth module or empeg.
// Will be reset each time an entire message from the bluetooth module or empeg is processed.
static String btInputString = "";
int btInputStringMaxLength = 100;
static String empegInputString = "";
int empegInputStringMaxLength = 300;

// Strings to hold some outgoing commands, reserve early to save memory.
static String commandToSend = "";
int commandToSendMaxLength = 85;
static String sendOutputString = "";
int sendOutputStringMaxLength = 85;

// Global variables used in main "Loop" function to detect when an entire complete
// message string has been received from the bluetooth module or empeg. Will be set to
// true when a line termination character is received indicating that the string is complete.
boolean btStringComplete = false;
boolean empegStringComplete = false;

// Variable to keep track of what the current Empeg Playing state is reported to be.
// This is used when responding to the bluetooth module's queries for playback state.
// The empeg is always either playing or paused, so this is either true or false.
boolean empegIsPlaying = false; 

// Variable to keep track of whether or not we think the empeg has completed its bootup
// procedure and the player is running, or if it's some unknown state such as it's still
// in the process of booting up. This can help us decide better whether or not it's
// in a playing or paused state and whether it is ready to accept serial port commands.
boolean empegPlayerIsRunning = true;

// Variable to keep track of whether or not we have seen a timestamp message from the
// empeg serial port yet so far. The very first timestamp we receive from the empeg should
// be recorded, but not acted upon. There is a routine to detect whether or not the player
// is playing or paused depending on whether the timestamp is changing or frozen. The problem
// is that the timestamp, the very first time we receive one after bootup, will almost always
// be considered to be different from the one extant in memory (which at bootup is 00:00:00)
// so there will at every bootup be a "mistake" where it thinks the empeg is playing when
// it isn't necessarily playing. So keep track of when we got the FIRST timestamp since bootup
// so that we don't "do stuff" the very first time, we just record the timestamp.
boolean empegFirstTimestamp = true;

// Variables for the state of the "Reset/Pair" button that I am implementing on this assembly.
// The button, when pressed, will clear out the module's bluetooth pairings, do a factory
// reset, and then do a pairing process where it attempts to pair with the first device
// that it sees. Note: For pairing with a car stereo with a touchscreen, this button
// is not be needed and in fact might interfere with the process. For some car stereos,
// the correct pairing process will be to NOT touch this button and instead do everything
// from the car stereo's touchscreen interface. Use this button for resetting the module
// and for pairing with things that don't have user interfaces, such as bluetooth headsets.
const int pairButtonPin = 52;
const unsigned long pairButtonDebounceTimeMs = 50;
unsigned long pairButtonLastPressedMs = 0;
int pairButtonState;
int lastPairButtonState = LOW;

// Variable for pin number of the reset/pair indicator blue LED.
const int pairLedPin = 50;

// Variable to globally keep track of whether we have recently initiated reset/pairing mode.
bool pairingMode = false;


// ------------------------------------
// Program code functions
// ------------------------------------

// ----------------------------------------------------------------------------
// Setup
//
// "Setup" is a default Arduino built-in function, which always runs
// once at the program startup, which happens in the following situations:
// - When power is initially applied to the Arduino.
// - Right after uploading the program to the Arduino.
// - Connecting to the Arduino's USB debug console.
// ----------------------------------------------------------------------------
void setup()
{
  // Set up the built in blinker LED on the Arduino board to active so that
  // I can use it to indicate behaviors of things during development and
  // debugging so I can check to see that my program is working in cases
  // without a connected computer. 
  pinMode(LED_BUILTIN, OUTPUT);

  // Make sure the built in LED is turned off at startup.
  digitalWrite(LED_BUILTIN, LOW);

  // Set up the Reset/pair blue LED indicator to "output" state.
  pinMode(pairLedPin, OUTPUT);

  // Make sure the reset/pair blue LED indicator is off at startup.
  digitalWrite(pairLedPin, LOW);
  
  // Set up the pair button to be in the "read" state.
  pinMode(pairButtonPin, INPUT);

  // Reserve bytes for the input strings, which are the strings we check to
  // see if there is a valid processable bluetooth AVRCP command incoming
  // from the bluetooth chip arriving from its serial port, or a processable
  // empeg status or metadata string arriving from the empeg serial port.
  btInputString.reserve(btInputStringMaxLength);
  empegInputString.reserve(empegInputStringMaxLength);

  // Reserve bytes for the some other strings to save memory.
  commandToSend.reserve(commandToSendMaxLength);
  sendOutputString.reserve(sendOutputStringMaxLength);
  pairAddressString.reserve(pairAddressStringMaxLength);
  transactionLabel.reserve(transactionLabelSize);
  transactionLabelPlaybackStatusChanged.reserve(transactionLabelSize);
  transactionLabelTrackChanged.reserve(transactionLabelSize);
  queryResponseString.reserve(queryResponseStringSize);

  // Reserve bytes for all the track metadata strings to save memeory.
  trackTitleString01.reserve(metadataMaxLength);
  trackArtistString02.reserve(metadataMaxLength);
  trackAlbumString03.reserve(metadataMaxLength);
  trackNumberString04.reserve(metadataSmallLength);
  trackTotalNumberString05.reserve(metadataSmallLength);
  trackGenreString06.reserve(metadataMaxLength);
  trackPlaybackPositionString07.reserve(metadataSmallLength);
  trackPlaybackLengthString.reserve(metadataSmallLength);
  
  // Open serial communications on the built Arduino debug console serial port
  // which is pins 0 and 1, this is the same serial output that you see when
  // using the Arduino serial monitor and/or a terminal program connected
  // to the USB interface that is used for Arduino monitoring and debugging.
  Serial.begin(115200);
  Log(F("Built in Arduino Serial has been started."));
  
  // Set the data rate for the Arduino Mega hardware serial port connected
  // to the bluetooth module's serial port. This is the one that is directly
  // connected because it is UART-to-UART, pin-to-pin, it doesn't have to go
  // through a MAX232 chip.
  BlueGigaSerial.begin(115200);
  Log(F("BlueGiga Serial has been started."));
  
  // Set the data rate for the serial port connected to the Empeg Car's
  // serial port, via the RS-232 or MAX232 circuit connected to the Arduino.
  // The empeg Car defaults to 115200 BPS in home/AC mode, and has configurable
  // BPS when sled-docked in car/DC mode. Configuration details in the hookup
  // information comments near the top of this file, above. Note that you will
  // experience problems controlling the empeg if it is set to anything other
  // than 115200 in car mode, so make sure you have done that step.
  EmpegSerial.begin(115200);
  Log(F("Empeg Serial has been started."));

  // Report the chip type
  reportChipType();

  // Verify the serial buffer size
  verifySerialBuffer();

  // Debugging
  // Log("Size of scFixMessageMatrix is:        " + String(sizeof(scFixMessageMatrix)));
  // Log("Size of rspMatrix is:                 " + String(sizeof(rspMatrix)));
  // Log("Size of rejMatrix is:                 " + String(sizeof(rejMatrix)));
  // Log("Size of pmMessageMatrix is:           " + String(sizeof(pmMessageMatrix)));
  // Log("Size of empegCommandMessageMatrix is: " + String(sizeof(empegCommandMessageMatrix)));

  // Debugging only - deliberately reboot the bluetooth module to simulate what
  // happens on first power up, to test if our freewheel timer at startup is
  // the correct amount of time.
  // QuickResetBluetooth(1);

  // Turn on the built in Arduino LED to indicate the setup activity has begun.
  digitalWrite(LED_BUILTIN, HIGH);
  
  // Freewheel and wait for the bluetooth device to boot up and be ready for
  // commands (bluetooth device receives power at the same time Arduino does).
  // EXPERIMENT - Try zero freewheeling at startp of bluetooth chip. Testing
  // seems to indicate that the chip accepts commands, even settings commands,
  // just fine immediately, even before it displays its own bootup message text.
  //        DisplayAndProcessCommands(1000, false);
  
  // Configre the Bluetooth device at startup, call my routine which sets all
  // data to the desired overall system defaults. This does not erase any
  // pairing information, that is left untouched.
  SetGlobalChipDefaults();

  // EXPERIMENT: If you encounter a repeat of the issue where there's a dropout followed
  // by the track metadata failing to update all of a sudden, then see if this fixes the
  // issue by fully resetting the bluetooth chip each time that the arduino inexplicably
  // resets itself. See GitHub issue for details:
  //      https://github.com/tfabris/BlueGigaEmpeg/issues/2
  // I don't want to enable this unless I really really have to, because this causes
  // bad user experience when initally connecting to the car stereo at car startup.
  //      QuickResetBluetooth(0);      

  // Turn off the built in Arduino LED to indicate the setup activity is complete
  digitalWrite(LED_BUILTIN, LOW);
}

// ----------------------------------------------------------------------------
// Loop
//
// "Loop" function is a built in Ardunio function, it is the main runtime
// program loop in Arduino. This function repeatedly loops while the Arduino
// is powered up. It will loop as quickly as we allow it to loop. Any code
// that executes as a sub-function of this code will be the thing that slows
// it down. For example, if you stay in a subroutine for too long then this
// loop will not execute while that subroutine is running. Hopefully all the
// subroutines I have written are all very short and quick to execute. We want
// this loop running fast so that it can get and process all characters from
// all serial ports.
// ----------------------------------------------------------------------------
void loop()
{
  // The MainInputOutput returns a CHAR variable. I'm not using it here
  // but I use it elsewhere so it's just a placeholder here.
  static char mainLoopChar = 0;

  // Read and handle all main input/ouptut functions on all serial ports, 
  // one character at a time. This runs as quickly as we let it run, since it
  // simply happens each time we go through the main Arduino "loop" function.
  mainLoopChar = MainInputOutput();

  // Read and handle all digital pushbutton I/O's such as the reset/pair button.
  MainPushbuttonHandler();
}

// ----------------------------------------------------------------------------
// Report Chip Type
//
// This is a function to log the type of Arduino Mega chip that
// this program is running on. This is oversimplified and does not
// show all chip types, just a subset of them.
// ----------------------------------------------------------------------------
void reportChipType()
{
  // Check to see if this is an AT Mega type of chip
  if ( SIGNATURE_0 == 0x1E || SIGNATURE_0 == 0x00)
  {
    // Chip is one of the AT Mega type of chip, so
    // check to see which one it is and report it 
    if ( SIGNATURE_1 == 0x95 && SIGNATURE_2 == 0x0F)
    {
      Log(F("Arduino chip type: AT Mega 328p - Warning: This chip may have low memory."));
    }
    else if ( SIGNATURE_1 == 0x98 && SIGNATURE_2 == 0x01)
    {
      Log(F("Arduino chip type: AT Mega 2560 - Good."));
    }
    else
    {
      Log(F("Arduino chip type: Some unknown type of ATMel chip - Warning: This chip may not meet recommended spec."));
    }
  }
  else
  {
      Log(F("Arduino chip type: Some type of Non-ATMel chip - Warning: This chip may not meet recommended spec."));
  }
}


// ----------------------------------------------------------------------------
// Verify Serial Buffer
//
// Report a problem if the serial buffer is not large enough. The Arduino
// header libraries must be edited in order for this program to work, and this
// code prechecks to make sure that this program was compiled with correctly
// modified header libraries to make sure there is enough buffer space.
// ----------------------------------------------------------------------------
void verifySerialBuffer()
{
  // Create variable to use in this function.
  static String myTxBufferSizeString;
  static int myTxBufferSizeInt;
  static String myRxBufferSizeString;
  static int myRxBufferSizeInt;

  // Retrieve variable which was defind in compiler header file.
  myRxBufferSizeString = (String)SERIAL_RX_BUFFER_SIZE;
  myTxBufferSizeString = (String)SERIAL_TX_BUFFER_SIZE;
  myRxBufferSizeInt = myRxBufferSizeString.toInt();
  myTxBufferSizeInt = myTxBufferSizeString.toInt();
  
  // Verify serial buffer size
  if ((myRxBufferSizeInt < 128) || (myTxBufferSizeInt < 128))
  {
    Log(F("------------------------------------------------------------------------------"));
    Log(F("         Compilation problem has occurred with this Arduino sketch.           "));
    Log(F(" "));
    Log(F("  This program requires a larger serial buffer size than standard size.       "));
    Log(F("  The Arduino compiler's header files must be edited to make this possible.   "));
    Log(F("  Please edit the following file, recompile and re-upload this sketch:        "));
    Log(F(" "));
    Log(F("     (install location)/hardware/arduino/avr/cores/arduino/HardwareSerial.h   "));
    Log(F(" "));
    Log(F("  Increase the SERIAL_RX_BUFFER_SIZE and SERIAL_TX_BUFFER_SIZE values to 128, "));
    Log(F("  recompile, and reupload your sketch to the Arduino module.                  "));
    Log(F("  Please refer to the code comments at the top of this sketch for details.    "));
    Log(F(" "));
    Log(F("------------------------------------------------------------------------------"));
  }
  else
  {
      Log("Serial receive buffer size: " + (String)myRxBufferSizeInt + " - Good.");
      Log("Serial xmit buffer size:    " + (String)myTxBufferSizeInt + " - Good.");
  }
}



// ----------------------------------------------------------------------------
// MainPushbuttonHandler
//
// Workhorse for handling digital pushbuttons implemented in this assembly,
// for example this routine implements the reset/pair button I made.
// Includes debouncing logic so that you don't try to run the same command
// multiple times from a single button press.
// ----------------------------------------------------------------------------
void MainPushbuttonHandler()
{
  // Record the timestamp of the moment in time that we started this function.
  // This timestamp counts up from the moment the ardunio is powered up.
  // The unsigned long can be up to 4,294,967,295, which in milliseconds is
  // over 1100 hours. So if this value wraps around, I don't know what will
  // happen, but I hope you're not running your car for 1100 hours without
  // turning it off once in a while.
  static unsigned long currentMillis = 0;
  currentMillis = millis();

  // Read the state of the reset/pair button at the time we started this function.
  static int pairButtonCurrentReading = LOW;
  pairButtonCurrentReading = digitalRead(pairButtonPin);

  // Check to see if the state of the button changed since our last reading of it.
  if (pairButtonCurrentReading != lastPairButtonState)
  {
    // If the state was different, then reset the debouncing timer
    pairButtonLastPressedMs = currentMillis;
  }

  // Check to see how long it has been since the last time we got a state change.
  // Only consider the button state to have changed if our time range is outside
  // the time range of our debounce timer.
  if ((currentMillis - pairButtonLastPressedMs) > pairButtonDebounceTimeMs)
  {
    // In here, these actions only take place if the time range is outside of
    // the range of our debounce timer.

    // If the button state has changed, AND we are outside of our time range,
    // then we can take action.
    if (pairButtonCurrentReading != pairButtonState)
    {
      pairButtonState = pairButtonCurrentReading;

      // Only take action if the new button state is HIGH
      if (pairButtonState == HIGH)
      {
        // Okay, we can execute the function that we are supposed to execute
        // if the reset/pair button has been pressed. This function will reset
        // the pairing data, put the device back to defaults, and then go into
        // pairing mode for a short period of time.
        PairBluetooth();
      }
    }
  }

  // Save the current button reading into the last button state,
  // for use in the next run through this loop.
  lastPairButtonState = pairButtonCurrentReading;
}

// ----------------------------------------------------------------------------
// SetGlobalChipDefaults
//
// Initializes the settings we want for this particular bluetooth chipset.
// ----------------------------------------------------------------------------
void SetGlobalChipDefaults()
{
  // For convenience, every time we reset or start up our chip, we also send empeg a pause command,
  // setting the empeg to a paused state. Then, if bluetooth reconnects after the reset, then the
  // unpause will happen automatically later, when it receives the stream start command from the
  // car stereo.
  // EXPERIMENT - Not doing this any more because I think it might confuse the pairing process
  // because we also send a "streaming start" command when we pause/play.
  //    SendEmpegCommand('W');   

  // Log to the debug console that we are starting this procedure.
  Log(F("Setting Defaults."));
  
  // Configure the system to not accidentally go into data mode in certain situations.
  // Parameters are:
  // "-"  = Disable ASCII character escape sequence that would put it into command mode.
  // "00" = Bitmask for which digital I/O pins are used for DTR signal (00=none)
  // "0"  = DTR disabled.
  SendBlueGigaCommand(F("SET CONTROL ESCAPE - 00 0"));

  // Configure serial port local echo settings. "5" is send only errors and events to screen,
  // otherwise silent. This prevents some of the "echo back" commands coming back from
  // our sent commands accidentally triggering other functions in this program. 
  SendBlueGigaCommand(F("SET CONTROL ECHO 5"));

  // Configure the system so that it does not send battery warnings nor attempt to shut down
  // when encountering low voltage situtations, since this device will only be running when the
  // car ignition is on, and will be getting the voltage (indirectly) from the car power.
  SendBlueGigaCommand(F("SET CONTROL BATTERY 0 0 0 0"));

  // Change some configuration bits on the player. See section 6.75 of the iWrap command
  // reference documentation for details on this topic. The format of the command is:
  //    SET CONTROL CONFIG 0000 0000 0000 1100
  // The default setting is the one shown above. Each group of digits is a hexadecimal number which
  // represents a set of configuration bits. The default setting of "1100" in the "config block"
  // sets bit positions 8 and 12 (aka 0b0001000100000000) which are flags for "Enables SCO links"
  // and "Randomly replace one of the existing pairings".
  // I am experimentally adding this bit:
  //     Bit 14 aka 0b0100000000000000 - "UART will be optimized for low latency"
  // This is an experimental attempt to improve an issue where there is a visible difference between
  // the empeg visuals on the empeg VFD display and the audio coming out of the host stereo's speakers.
  SendBlueGigaCommand(F("SET CONTROL CONFIG 0000 0000 0000 5100"));

  // Configure the WT32i development board to NOT use the additional external TI AIC32I external codec chip.
  // Unused if you're not using the BlueGiga Development board which contains the external codec.
  //  SendBlueGigaCommand(F("SET CONTROL EXTCODEC PRE"));

  // Set bluetooth Class of Device - Docs say this is needed.
  // CoD generator form:   http://bluetooth-pentest.narod.ru/software/bluetooth_class_of_device-service_generator.html
  // Alternate generator:  http://www.ampedrftech.com/cod.htm
  //  0x200420 = 
  //  - Major service class: Audio
  //  - Major device class:  AudioVideo
  //  - Minor device class:  CarAudio
  // 0x280428 =
  //  - Major service class: Audio and Capturing
  //  - Major device class:  AudioVideo
  //  - Minor device class:  HiFi Audio
  // 0x200428 =
  //  - Major service class: Audio
  //  - Major device class:  AudioVideo
  //  - Minor device class:  HiFi Audio and Car Audio
  // 
  // The docs say: "In the case of A2DP Source, the specification mandates the use
  // of the Capturing Service bit (0x080000). The Audio Service (0x200000) and
  // Audio/Video Major Device Class (0x000400) should be used, but they are not
  // mandatory. A common example Minor Device Class is Hi-Fi Audio Device
  // (0x000028). The combined CoD is then 0x280428."
  // 
  // NOTE: in the docs for the BlueGiga module, it shows examples where,
  // even though this is a hexadecimal number, it is not preceded by "0x".
  // So for example, "SET BT CLASS 280428" is the correct command whereas
  // "SET BT CLASS 0x280428" is not, at least according to the docs, and
  // it even says "SYNTAX ERROR" if you precede it with the 0x.
  SendBlueGigaCommand(F("SET BT CLASS 280428"));  

  // Turn off "Serial Port Profile" on the bluetooth chip, we don't need it for A2DP.
  SendBlueGigaCommand(F("SET PROFILE SPP"));

  // Configure chip to the "A2DP Source" profile, which sends music out to the paired
  // car stereo instead of the other way around (the other way is called "A2DP Sink").
  SendBlueGigaCommand(F("SET PROFILE A2DP SOURCE"));

  // Set chip to recieve AVRCP commands with specific categories - See bluetooth chip's
  // PDF documetation for descriptions of the AVRCP categories, section 3.2 of
  // the A2DP/AVRCP profile document. 
  // - Bit Value 0001 - Category 1: Player/Recorder - must support PLAY and PAUSE
  // - Bit Value 0002 - Category 2: Monitor/Amplifier - must support VOLUP and VOLDN (volume up and down)
  // - Bit Value 0004 - Category 3: Tuner - must support CHUP and CHDN (channel up and down)
  // - Bit Value 0008 - Category 4: Menu - must support MSELECT, MUP, MDN, MLEFT, MRIGHT (menu select and directions)
  // Example: value of "3" would be Player/recorder plus Monitor/Amplifier.
  SendBlueGigaCommand(F("SET PROFILE AVRCP TARGET 3"));

  // Our project identifies itself nicely on car stereo's LCD screen when pairing,
  // with a nice happy friendly name that we like very much. Apologies if I don't
  // accurately capitalize "empeg" (little e) in every place in my code. It's 
  // correct here to have the lower case e, and this is the customer-facing string,
  // so this is the place it has to be correct the most.
  SendBlueGigaCommand(F("SET BT NAME empeg Car"));

  // Set bluetooth pairing security authorization types (defined at top of program).
  SendBlueGigaCommand(btAuthTypeString);
  SendBlueGigaCommand(btPinCodeString);

  // Audio routing on the chip. Control string for audio routing is configured at top of program.
  //     SendBlueGigaCommand(F("SET CONTROL AUDIO INTERNAL INTERNAL EVENT KEEPALIVE"));    
  SendBlueGigaCommand(btAudioRoutingControlString);    
  
  // Turn off the mic preamp to prevent distortion on line level inputs if they are being used.
  SendBlueGigaCommand(F("SET CONTROL PREAMP 0 0"));    
  
  // Turn off the mic bias voltage (well, set it to as low as possible).
  SendBlueGigaCommand(F("SET CONTROL MICBIAS 0 0"));    
  
  // Set line level input gain on the bluetooth chip, value is set in global constant at top of
  // program. This is for situations where the line level inputs might be used.
  SendBlueGigaCommand(empegGainSettingString);    

  // Docs say that a "RESET" is needed in order for the profile changes above to take effect.
  // The docs do not make it clear what the difference is between the commands "RESET" and
  // "BOOT 0", which, to my eye, look like they do the same thing, but I can't be sure.
  // Note that "RESET" is just a reboot, whereas "SET RESET" is a factory-reset of the chip.
  // EXPERIMENT - Try doing tge RESET only in cases where the user has pressed the RESET-PAIR
  // button on my assembly. That means don't do it here, but only do it outside this routine
  // in places where it is correctly applicable. The purpose of this experiment is so that the
  // head unit device doesn't repeatedly disconnect and reconnect from the bluetooth more times
  // than is absolutely necessary when the unit powers up. I have a headset which is quick
  // enough at reconnecting that it can reconnect immediately after a power cycle, only to 
  // have it disconnect and reconnect again when this line gets hit.
  //     QuickResetBluetooth(0);      

  // Freewheel for a moment after reset and swallow the lines of the reset messages.
  // EXPERIMENT - Do not bother to swallow responses from the reset messages. This might
  // not be needed any more after all my other code fixes and now might be getting in the way.
  //      DisplayAndSwallowResponses(6, 300);
  
  Log(F("Done Setting Defaults."));

  // Debugging only, unit testing the element query code.
  //  RespondToQueries("GET_ELEMENT_ATTRIBUTES 07 04 05 07 01 02 03 06"); // All elements
  //  RespondToQueries("GET_ELEMENT_ATTRIBUTES 7 00004 005 07 1 02 3 06"); // All elements but with different numbers of digits
  //  RespondToQueries("GET_ELEMENT_ATTRIBUTES 07 04 05 07 01abc 02 03 06?"); // Non-numeric characters after some of the elements
  //  RespondToQueries("GET_ELEMENT_ATTRIBUTES 07 04 05 07 01 02 03"); // Negative case, missing one of the element codes
  //  RespondToQueries("GET_ELEMENT_ATTRIBUTES 01 03"); // Single element
}


// ----------------------------------------------------------------------------
// PairBluetooth
//
// This is the function that gets executed when you press the Reset/Pair button.
// Resets the pairing table on the bluetooth chip, erasing all previous paired
// devices, so that the device can freshly pair anew with a new device.
// Also runs a program that tries to pair with discovered devices for a short
// amount of time. Note: For connection to a car stereo, you might not need to
// issue this command, pairing might work fine without touching this function.
// I have found it is necessary with things like bluetooth headsets, but not
// always necessary for pairing with car stereos. For car stereos, you handle
// all pairing steps from the stereo's touch screen.
// 
// This function doubles as a way to reset the device to factory defaults
// and restart it, in cases where that is needed.
// ----------------------------------------------------------------------------
void PairBluetooth()
{
  // Don't execute this if we are currently already in pairing mode.
  // Protect against possible re-entrant code.
  if (pairingMode)
  {
    return;
  }

  // Set flag indicating we recently started pairing mode.
  pairingMode = true;

  // Turn on the Reset/pair indicator blue LED
  digitalWrite(pairLedPin, HIGH);

  // Set the Pair Address string to a blank to indicate that we
  // need to get a new address to pair with.
  pairAddressString = "";

  // Log what we are about to do, to the Arduino debugging console.
  Log (F(" "));
  Log (F("--------------------------------------"));
  Log (F("           RESETTING DEVICE           "));
  Log (F("--------------------------------------"));
  Log (F(" "));

  // Erase all previous bluetooth pairings (not included in the factory default reset below).
  SendBlueGigaCommand(F("SET BT PAIR *"));

  // Erase all previous auto-reconnect settings (not sure if included in the factory default reset).
  SendBlueGigaCommand(F("SET CONTROL RECONNECT *"));

  // Reset device to factory defaults. Note that "SET RESET" (factory defaults) is different
  // from "RESET" (which is just a reboot). Note that this does not erase pairings (that is
  // accomplished by the commands above instead).
  QuickResetBluetooth(2); 

  // Freewheel for a moment after reset to factory defaults, and swallow the lines of the
  // reset messages. This is needed because the global chip defaults below won't "take" if
  // you don't give some time after the reset message is issued.
  DisplayAndSwallowResponses(8, 600);

  // Set up the device for use with the empeg car.
  SetGlobalChipDefaults();

  // EXPERIMENT - Move the "RESET" statement (which originally occured inside the
  // SetGlobalChipDefaults statement) out here so that we only do it in cases where
  // the user presses the RESET-PAIR button on my assembly, and not at bootup.
  QuickResetBluetooth(0);    

  // Freewheel for a moment after setting global chip defaults. Need a time window before
  // the chip will accept the INQUIRY command below.
  DisplayAndSwallowResponses(9, 700);

  // Log Partial Completeness.
  Log (F(" "));
  Log (F("--------------------------------------"));
  Log (F("   Done resetting. Beginning pair.    "));
  Log (F("--------------------------------------"));
  Log (F(" "));  

  // Notify users watching the debug console about the pairing quirk.
  Log (F(" "));
  Log (F("--------------------------------------"));
  Log (F("  Note: For some devices, attempting  "));
  Log (F("  to pair right now will not work.    "));
  Log (F("  Sometimes, for example, with some   "));
  Log (F("  touchscreen devices, you must wait  "));
  Log (F("  until the LED is off and the pair   "));
  Log (F("  routine here is done, and then      "));
  Log (F("  initiate the pairing process solely "));
  Log (F("  from the touchscreen.               "));
  Log (F("                                      "));
  Log (F("  Other devices may need to be put in "));
  Log (F("  pairing mode right now. Depends on  "));
  Log (F("  the device.                         "));
  Log (F("--------------------------------------"));
  Log (F(" "));  

  // Clear out our input string before starting the pairing process
  // just in case we pressed the pair button in the middle of a string input
  btInputString = "";
  
  // Initiate the process of pairing, which is merely a command to query for
  // available bluetooth devices in the air nearby. Then we will respond to
  // any responses that we see on the serial port for a number of seconds,
  // handled in the loop below.
  SendBlueGigaCommand(pairBeginString);

  // Process pairing commands for x milliseconds before quitting. The main
  // loop for processing serial input/output will automatically detect the
  // necessary responses from bluetooth devices in the air nearby and answer
  // appropriately at the appropriate times. At regular intervals, check to
  // see if the pairing process was completed and bail out of the loop if it
  // was. Note, the hardcoded "30" here is arbitrary, it's the number of times
  // during this loop that we check for loop completion. For instance, if
  // pairTimeMilliseconds is 30, then this will check for loop completion
  // once per second. If pairTimeMilliseconds is 15, then this will check for
  // loop completion once every half second, etc.
  for (int i=0; i<=30; i++)
  {
    // Process commands for a portion of the time as we wait for pairing
    // to complete. Pairing response codes, and pairing completion detection,
    // is a piece of special case code elsewhere in this program code, in
    // the string handling/processing code which is executed automatically.
    // So pairing completion will occur automatically if the host device is put
    // into pairing mode during this loop and the device is found.
    DisplayAndProcessCommands(pairTimeMilliseconds / 30, false);

    // Check if the pairing process is detected as having been completed
    // by special case code elsewhere in the code flow. The flag to detect
    // whether it has been completed is the pairingMode variable, which the
    // special case code will set to "false" when it detects it has completed
    // the pairing process and has done all the pairing steps.
    if (pairingMode == false)
    {
      // if the pairing process got completed for us, break out of the
      // waiting loop to continue with this routine early.
      break;
    }
  }
  
  // We are done with pairing mode if either we have run out of loop time
  // or if the code above detects that pairing mode got completed.
  pairingMode = false;

  // Turn off the pairing indicator blue LED
  digitalWrite(pairLedPin, LOW);

  // Log that we are done.
  Log (F(" "));
  Log (F("--------------------------------------"));
  Log (F("        Pairing process ended.        "));
  Log (F("--------------------------------------"));
  Log (F(" "));      
}

// ----------------------------------------------------------------------------
// MainInputOutput
// 
// Main workhorse to handle the main loop character input output and logging
// to serial port. Detetcs if the character strings are any of the special commands
// that we are waiting for. Must call this repeatedly (for instance in main loop)
// for it to be useful, since it only processes one character at a time.
// This function will receive serial port data, log it to the debug port, interpret
// it, and if certain AVRCP commands are received, then respond to them and/or
// send the corresponding serial commands to the empeg-car if appropriate.
//
// Returns:
//    (char) The character it processed that it received from the bluetooth
//    serial port, if any. If no character was received it will return a zero
//    value character. Note that if a zero value character was received it will
//    also return that. So don't use zero as the test to see whether or not it
//    processed a character.
// ----------------------------------------------------------------------------
char MainInputOutput()
{
  // Initialize the characters that will be retrieved in this loop.

  // From the bluetooth module.
  char inChar = 0; 

  // From the debug serial port (so you can type commands to the bluetooth)
  char userChar = 0;

  // From the empeg car serial port
  char empegChar = 0;


  // Check to see if any characters are available on the empeg serial port
  // from the empeg car, display them on the debugging console, and process
  // them into individual lines for later parsing for track status and
  // track metadata information.
  if (EmpegSerial)
  {
    if (EmpegSerial.available())
    {
      // Read one character (byte) from the empeg serial port.
      empegChar = EmpegSerial.read();

      if (displayEmpegSerial)
      {
        // Log to the debugging console what we saw on the port.
        LogChar(empegChar);
      }

      // Add the character that we read to our input line string for use later.
      empegInputString += empegChar;   

      // Check to see if it's a linefeed or if the length of the
      // string is too large to hold, either way we consider the string
      // to be done, baked, and ready to do something with.
      if ((empegChar == '\n') || (empegInputString.length() >= (empegInputStringMaxLength - 1)))
      {
        // String is ready to be processed if one of the above things was true.
        empegStringComplete = true;
      }         
    }
  }

  // Check to see if we have received a completed string from the empeg
  // and therefore need to start processing the contents of that string.
  if (empegStringComplete)
  {
    empegStringComplete = false;
        
    // Call the function to process the string and do the needful.
    HandleEmpegString(empegInputString);
    
    // Reset the string after processing it, ready for the next line.
    empegInputString = "";
  }  
  
  // Check to see if any characters are available on the bluetooth serial port
  // from the bluetooth module, display them on the debugging console,
  // and process them into individual lines for later parsing and command/response.
  if(BlueGigaSerial)
  {
    if (BlueGigaSerial.available())
    {
      // Read one character (byte) from the bluetooth serial port.
      inChar = BlueGigaSerial.read();
      
      // Log to the Arduino debugging console what we saw on the bluetooth serial port.
      LogChar(inChar); 
      
      // Add the character that we read to our input line string for use later.
      btInputString += inChar;
      
      // Check to see if it's a carriage return or if the length of the
      // string is too large to hold, either way we consider the string
      // to be done, baked, and ready to do something with.
      if ((inChar == '\n') || (btInputString.length() >= (btInputStringMaxLength - 1)))
      {
        // String is ready to be processed if one of the above things was true.
        btStringComplete = true;
      }
    }
  }

  // Check to see if we have received a completed string from the bluetooth
  // and therefore need to start processing the contents of that string.
  if (btStringComplete)
  {
    btStringComplete = false;
        
    // Call the function to process the string and do the needful.
    HandleString(btInputString);
    
    // Reset the string after processing it, ready for the next line.
    btInputString = "";
  }
    
  // If I type something on the debug console (the Arduino main USB serial
  // debug port) assume that this is intended as a command to send to
  // the bluetooth module, and forward it there.
  if (Serial)
  {
    if (Serial.available())
    {
      // Read the character from the serial port.
      userChar = Serial.read();
  
      // Write that character to the bluetooth Board.
      BlueGigaSerial.write(userChar);
  
      // Local echo the character back to the debug console.
      LogChar(userChar);
      
      // Perform special debugging trick in which we test the empeg Serial port
      // commands if we are in the special empeg debugging mode. To use this
      // feature, enable EmpegSendCommandDebug flag at the top of the code. Then,
      // send one of the empeg command keys (N P C W - Case inensitive) to the
      // Arduino serial console to trigger the effect.
      if (EmpegSendCommandDebug)
      {
        // Iterate through our matrix of incoming avrcp commands looking for a match.
        for (int i=0; i<empegCommandMatrixSize; i++)
        {
          // Make the string comparison to find out if there is a match.
          // Match to the second column in the table ("[1]") to match the
          // empeg single-key command rather than the AVRCP string.
          if ((empegCommandMessageMatrix[i][1]).equalsIgnoreCase((String)userChar)) 
          {
            // Because this stunt happens before the CR or CRLF or whatever
            // is processed at the end of the command, also send the necessary
            // line terminator to the screen so it doesn't print on the same
            // line as the next thing logged to the serial port. 
            LogChar('\n');

            // Directly trigger the empeg command-sending code with the
            // AVRCP message (column "[0]") already in the variable, to
            // force the action to trigger right away.
            HandleString(empegCommandMessageMatrix[i][0]);
          }
        }      
      }
    }
  }
   
  // Return from this function, return the bluetooth character we processed in this loop.
  return inChar;
}


// ----------------------------------------------------------------------------
// DisplayAndProcessCommands
//
// Runs the main input loop for a specified amount of time (including the fact
// that it will process commands if the main input loop receives them). This
// function is mostly useful for waiting for certain commands to be processed
// and to respond with a response of some kind.
//
// Parameters
//    (int) idleTimeMs  The amount of time in approximate milliseconds to wait.
//    (bool) waitForLineEnding   Stop waiting if a CR or LF was received.
// ----------------------------------------------------------------------------
void DisplayAndProcessCommands(int idleTimeMs, bool waitForLineEnding)
{
  static char displayChar = 0;
  
  for (int i=0; i<idleTimeMs; i++)
  {
    displayChar = MainInputOutput();
    if (waitForLineEnding)
    {
      if (displayChar == '\n')
      {
        return;
      }
    }
    delay(1);
  }
}

// ----------------------------------------------------------------------------
// HandleString
// 
// Function to process the string data received from the bluetooth chip.
// When a complete string is received, call this function to look up and
// see if the contents of the sting match something in our table of
// commands (defined at the top of the program), If there is a match,
// send the corresponding empeg command to the empeg serial port to command
// the empeg to do the same behavior that the bluetooth chip had just been
// commanded to do.
//
// This also handles other cases where perhaps we need to respond to
// some command that was issued to the bluetooth chip from the host stereo,
// and now we have to respond to that command directly on the bluetooth.
// ----------------------------------------------------------------------------
void HandleString(String theString)
{
  // Initialize the variables we will be using in this function.
  commandToSend = "";
  static char empegCommandToSend = 0;

  // Bugfix - If we get a "NO CARRIER" message from the bluetooth chip then
  // we need to set our globally stored transaction labels to blanks. This is
  // because, upon disconnection from a bluetooth device, the transaction labels
  // become invalid and now we should not be using them any more. They should be
  // reset so that upon the next reconnection that we can receive new transaction
  // labels. This will hopefully fix an issue where if you turn off the head device
  // (the stereo, the bluetooth headset, whatever), and then turn it back on again
  // without rebooting the BlueGiga and the Arduino, then it will not start
  // sending bogus transaction labels to the head device causing it to glitch
  // or disconnect.
  if (theString.indexOf(F("NO CARRIER")) > (-1))
  {
    ClearGlobalVariables();
  }

  // Handle query/response strings for things like the track metadata
  // and the playback status information. Make sure our program responds
  // to these queries in a timely fashion if it receives them.
  // This is a time-sensitive routine that needs to respond immediately
  // as soon as we get a query string so I have put it at the top of
  // the string handling routing. However there is a catch-22 where
  // one of the responses might need to be the track metadata from the
  // empeg. Since getting the track metadata from the empeg might be
  // slow, we need to be careful here and not spend "slow time" trying
  // to get the empeg track metadata every single time. 
  //
  // Initially, look inside the quick-and-dirty matrix of possible
  // queries to respond to...
  for (int i=0; i<rspMatrixSize; i++)
  {  
     // Make the string comparison to find out if there is a match.
      if (theString.indexOf(rspMatrix[i]) > (-1))
      {
        // A match has been found, process the query/repsonse command.
        // The function will do all detailed handling of these strings.
        RespondToQueries(theString);

        // Bugfix: Only respond to queries once based on the contents
        // of the quick-and-dirty response matrix. There are some entries
        // in the matrix which may generate a double response if we don't
        // break here. For instance, the host stereo might query for the
        // string like this:
        //  AVRCP 1 PDU_REGISTER_NOTIFICATION 4 PLAYBACK_STATUS_CHANGED 0
        // This might generate two hits in our matrix, once for
        // PLAYBACK_STATUS_CHANGED and once for PDU_REGISTER_NOTIFICATION.
        // So place a break statement here to stop going through the loop
        // once we have found a hit in the table.
        break;
      }
  }

  // If we are inside the pairing process and we have detected that
  // the pairing process is complete, then set the flag tp indicate
  // that the pairing process is complete. The pairing process will
  // end once the flag has been set to false. The actual handling
  // of the inputs and responses to the pairing mode strings
  // is handled a little farther down in the code.
  if (pairingMode)
  {
    if (theString.indexOf(pairDetectionString) > (-1))
    {
      Log(F("Detected: Pairing Mode is Complete."));
      pairingMode = false;
    }
  }
  
  // Iterate through our matrix of AVRCP commands looking for a match,
  // to find commands that may be translatable into empeg commands.
  // This right here is the main heart, the main reason this program exists.
  for (int i=0; i < empegCommandMatrixSize; i++)
  {
    // Make the string comparison to find out if there is a match.
    // Look for the AVRCP command (column "[0]" in the table).
    if (theString.indexOf(empegCommandMessageMatrix[i][0]) > (-1))
    {
      // A match has been found, send the command to the empeg.
      // The corresponding empeg command is stored in column "[1]"
      // in the command matrix table.
      empegCommandToSend = empegCommandMessageMatrix[i][1].charAt(0);      

      // Send the command.
      SendEmpegCommand(empegCommandToSend);
    }
  }
  
  // Handle special case fix strings which are responses to certain
  // strings we receive on the bluetooth chip that we must respond
  // back to on the bluetooth chip. First iterate through our matrix
  // of special case commands looking for a match.
  for (int i=0; i<scFixMatrixSize; i++)
  {
    // Make the string comparison to find out if there is a match.
    // Look for the special command (column "[0]" in the table).
    if (theString.indexOf(scFixMessageMatrix[i][0]) > (-1))
    {
      // A match has been found, process the speical case.
      
      // Turn on the Arduino LED to indicate something has happened.
      digitalWrite(LED_BUILTIN, HIGH);
   
      // Prepare the command we are going to send.
      commandToSend = "";
      commandToSend += scFixMessageMatrix[i][1];

      // Send the special case fix command.
      SendBlueGigaCommand(commandToSend);
           
      // Turn off the Arduino LED.
      digitalWrite(LED_BUILTIN, LOW);
    }
  }

  // Handle pairing mode strings - these are the strings that are only
  // active when we are in Reset/Pairing mode and will not be processed
  // at any other time.
  if (pairingMode)
  {
    // First iterate through our matrix of pairing commands looking for a match.
    for (int i=0; i<pmMatrixSize; i++)
    {
      // Make the string comparison to find out if there is a match.
      // Look for the special command (column "[0]" in the table).
      if (theString.indexOf(pmMessageMatrix[i][0]) > (-1))
      {
        // A match has been found, process the pairing command.
        
        // Turn on the Arduino LED to indicate something has happened.
        digitalWrite(LED_BUILTIN, HIGH);

        // Process the pullout of the string substitution bit if any.
        // This is a super special case for one particular string.
        // It retrieves the bluetooth device address of the host stereo
        // that we are pairing to, and places it into a global variable
        // for later use in the response string.
        GrabPairAddressString(theString);
        
        // Prepare command we are going to send.
        commandToSend = "";
        commandToSend += pmMessageMatrix[i][1];

        // Substitute the bluetooth host address string in the response command,
        // if we have it and if the command contains the token for the replacement.
        if (pairAddressString != "")
        {
          if (commandToSend.indexOf(tokenSubstitutionString) > (-1))
          {
            commandToSend.replace(tokenSubstitutionString, pairAddressString);
          }
        }

        // Send the final assembled pairing response string to the bluetooth chip.
        SendBlueGigaCommand(commandToSend);
        
        // Turn off the Arduino LED.
        digitalWrite(LED_BUILTIN, LOW);
      }
    }
  }  
}


// ---------------------------------------------------------------
// GrabPairAddressString
//
// Grabs the bluetooth address from a string that looks like this:
// "INQUIRY_PARTIAL 0c:e0:e4:6c:75:68 240404" and puts it into the
// global variable "pairAddressString". Only does the first one
// it finds since the runstart of the program or since the user
// has pressed the "reset/pair" button on the assembly. Only useful
// during the inside of the pairing procedure since it doesn't save
// the value in permanent storage for any reason. We leave it up
// to the bluetooth chip to store the pair addresses, and the 
// bluetooth chip automatically reconnects to paired devices when
// it is powered back on, so our code only needs to know this
// pairing address for the length of time that is needed to
// complete the initial pairing process.
// ---------------------------------------------------------------
void GrabPairAddressString(String stringToParse)
{
  // Set variables we will be using the parsing code below.
  static int firstSpace = 0;
  static int lastSpace = 0;
  firstSpace = 0;
  lastSpace = 0;

  // If we already have a pair address string, don't retrieve another one,
  // we only want to do this for the first address in the list that we find.
  if (pairAddressString != "")
  {
    return;
  }
  
  // If we're not in the middle of the pairing procedure, don't bother since
  // we can't make use of it otherwise.
  if (!pairingMode)
  {
    return;
  }
  
  // Get our bluetooth address out of the string if it's the one exact
  // special case string that we expect to see at this particular moment 
  if (stringToParse.indexOf(F("INQUIRY_PARTIAL ")) > (-1))
  {
    // Obtain the location of the first space
    firstSpace = stringToParse.indexOf(F(" "));

    // Obtain the location of the last space
    lastSpace = stringToParse.lastIndexOf(F(" "));

    // Obtain the substring between the two spaces
    pairAddressString = stringToParse.substring(firstSpace, lastSpace);

    // Trim the string of any possible whitespace
    pairAddressString.trim();

    // Log what we got.
    Log (F("------------------------------------"));
    Log (F("    Pairing with device address:    "));
    Log (pairAddressString);
    Log (F("------------------------------------"));
  }
}

// ----------------------------------------------------------------------------
// Log
//
// Log a string of output to the Arduino USB debug serial port.
// ----------------------------------------------------------------------------
void Log(String logMessage)
{
  // Make sure the main serial port is available before outputting.
  if (Serial)
  {
    // Print the resulting string.
    Serial.print(logMessage);

    // Print a CRLF at the end of the log message to alleviate us from
    // the need of adding one by hand at the end of every logged message
    // throughout the code.
    Serial.print(F("\r\n"));
  }       
}

// ----------------------------------------------------------------------------
// LogChar
//
// Output a single character to the Arduino USB debug serial port.
// ----------------------------------------------------------------------------
void LogChar(char logChar)
{
  // Make sure the main serial port is available before outputting.
  if (Serial)
  {
    // Write the actual character
    Serial.write(logChar);
  }       
}

// ----------------------------------------------------------------------------
// SendBlueGigaCommand
//
// Send a command to the bluetooth chip. 
// Automatically append the required line ending character.
// Some chips require a carriage return, some chips require a linefeed or CRLF.
// ----------------------------------------------------------------------------
void SendBlueGigaCommand(String commandString)
{
  // Make sure serial port is available to send to
  if(BlueGigaSerial)
  {
    // Log the command we are sending.
    Log("--> " + commandString);
  }
  else
  {
    // Log the command we cannot be sending.
    Log("--> CANNOT SEND COMMAND - NO SERIAL PORT: " + commandString);  
    return;  
  }

  // Add the line ending to the command string.
  sendOutputString = "";
  sendOutputString = commandString + "\r\n";

  // Send the string.
  BlueGigaSerial.print(sendOutputString); 

  // Debugging Only:
  // "Immediate Response Display" feature.
  //
  // A single line of code below does a special thing:
  // Show repsonses from the chip on the debug console
  // immediately after sending a command instead of waiting
  // for the next normal run of the main interpreter loop.
  //
  // This works and it is helpful to see things like the 
  // chip immediately responding with "SYNTAX ERROR" for
  // the exact command line that we sent, as opposed to
  // seeing it appear in the console much later. For example,
  // in normal operation, at the first startup of this Arduino
  // code, there are several chip-setup commands which get sent
  // to the bluetooth chip at startup. They are all sent quickly
  // in sequence one after the other, and normally, the code
  // doesn't get around to displaying the chip's responses to
  // the commands until after the entire group of commands is
  // sent. So, for example, you might see this in the console:
  // 
      //Setting Defaults.
      //SET CONTROL ECHO 5
      //SET PROFILE A2DP SOURCE
      //SET PROFILE AVRCP TARGET 3
      //SET PROFILE SPP
      //SET BT CLASS 280428
      //SET BT NAME empeg Car
      //SET BT SSP 3 0
      //SET BT AUTH * 0000
      //SET CONTROL AUDIO INTERNAL INTERNAL EVENT KEEPALIVE
      //SET CONTROL PREAMP 0 0
      //SET CONTROL MICBIAS 0 0
      //SET CONTROL GAIN A 0
      //RESET
      //WRAP THOR AI (6.2.0 build 1122)
      //Copyright (c) 2003-2017 Silicon Labs Inc.
      //READY.
      //Done Setting Defaults.
      //SYNTAX ERROR
  //
  // That "SYNTAX ERROR" at the end there is the problem.
  // It was in response to one of those SET commands above,
  // but the problem is that now you don't know which one
  // it was in response to.
  // 
  // By enabling the line of code below, it gives the code
  // a chance to immediately display the response from the
  // bluetooth chip so you can tell which command got the
  // syntax error.
  //
  // The risk here is a possibility for reentrant code that
  // might get out of control. So don't leave the line
  // enabled in final production, just enable it to check
  // something and then disable it again when you're done.
  // 
  // The other issue with enabling this command is that
  // it slows down our output response in cases where we
  // must have the response much quicker.
  //
  // Here is the line of code:
  //     DisplayAndProcessCommands(335, false);
  //
  // Re-enable the line above if you are debugging an immediate
  // send/response issue where you have issued a bunch of
  // commands to the bluetooth chip in sequence and you're
  // having trouble figuring out which of the commands
  // is the one that produced the "SYNTAX ERROR" message.
  // Make sure to disable it again when you're done debugging.
}


// ----------------------------------------------------------------------------
// RespondToQueries
//
// Command the bluetooth chipset to respond to certain queries made by the
// host stereo, such as queries for playback status and track titles.
//
// NOTE: If you add new code below to handle a new query/response string,
// then make sure to update the "rspMatrix" at the top of the code, and
// vice-versa.
// ----------------------------------------------------------------------------
void RespondToQueries(String queryString)
{
  // Obtain the transaction label from these queries. Some of the queries we're responding to here
  // have a transaction label, and our response must contain the same transaction label in the response.
  // Query comes in from the host stereo like this:
  //     AVRCP 0 PDU_REGISTER_NOTIFICATION 4 PLAYBACK_STATUS_CHANGED 0
  // It means:
  // - On the AVRCP channel which is currently paired in slot/channel 0
  // - The host stereo is registering a notification with us.
  // - This message carries with it a transaction label of "4" (it might be a different character than 4)
  // - Register for a notification message for playback status changing (it would like us to please send it playback status notifications)
  // - That last 0 is an unused parameter in the register_notification request according to the BlueGiga iWrap documentation.
  //
  // For these queries, you should then respond with a response that contains the transaction label.
  // Example:
  //     AVRCP NFY INTERIM 4 1 0
  // Where 4 is the transaction label, 1 is the event ID code for "Playback status changed", and 0 is the code
  // for the state of playback (in this example 0 means stopped).
  // 
  // Note that the transaction label might be a hex value, I have seen it be in the range of 0-9 a-f,
  // but I haven't seen it go above f in my tests. 
  //
  // Later, then if you want to send the notification to the host stereo that something has changed,
  // for example if you want to tell the host stereo that the playback status changed, then you would
  // send a command that was something like this (this is handled elsewhere in the code):
  //      AVRCP NFY CHANGED 4 1 1
  // Where:
  //      AVRCP NFY   - The notification command to send to the head unit.
  //      CHANGED     - Indicate to the head unit that there was a change of some kind.
  //      4           - The transaction label from the headunit's original notification registration request
  //      1           - The notification message we will send contains event ID 1, which is "PLAYBACK_STATUS_CHANGED", from a specific table in section 6.3.3 of the AVRCP command reference for the BlueGiga iWrap command language.
  //      1           - The sole parameter value for the PLAYBACK_STATUS_CHANGED, with "1" indicating "playing" and other values indicating other things.
  //

  // Debugging: 
  // Log(F("Entering RespondToQueries function."));


  // Set up variables.
  transactionLabel = "";
  static int transactionLabelStringPosition = 0;
  static int elementsStartSelectPosition = 0;
  static int elementsEndSelectPosition = 0;
  static int numberOfElements = 0;
  static int currentElementCode = 0;

  // Obtain the transaction label if any.
  // Note the space at the end of "REGISTER_NOTIFICATION "
  // Total length of the string "REGISTER_NOTIFICATION " is 22
  transactionLabelStringPosition = queryString.indexOf(F("REGISTER_NOTIFICATION "));
  if ((transactionLabelStringPosition) > (-1)) 
  {
    // Get the next few characters in the section where I expect to see the transaction label
    transactionLabel = queryString.substring(transactionLabelStringPosition + 21, transactionLabelStringPosition + 24);
    transactionLabel.trim();

    // Debugging: 
    // Log("Transaction label string found, label is: " + transactionLabel);

    // Special case where we save off some possible global variables
    // which are the specific transaction labels for two specific
    // types of query/response strings we might be deailing with.
    // This is part of the fix for the "Kenwood Bug". See comments
    // in the function "HandleEmpegStateChange" for details on how
    // these variables are used and how they fix the "Kenwood Bug".
    if (queryString.indexOf(F("PLAYBACK_STATUS_CHANGED")) > (-1))
    {
      // If we got a "REGISTER_NOTIFICATION" for "PLAYBACK_STATUS_CHANGED"
      // then save off the global variable for the transaction label 
      // from that particular notification registration for that transaction.
      transactionLabelPlaybackStatusChanged = transactionLabel;
    }
    if (queryString.indexOf(F("TRACK_CHANGED")) > (-1))
    {
      // If we got a "REGISTER_NOTIFICATION" for "TRACK_CHANGED"
      // then save off the global variable for the transaction label 
      // from that particular notification registration for that transaction.
      transactionLabelTrackChanged = transactionLabel;
    }

    // EXPERIMENTAL:
    // Special case to work around bug which hung the headunit stereo intermittently.
    // Reject certain attempts to do PDU_REGISTER_NOTIFICATION for certain types of
    // events which we are not going to be using. The table of these events and their
    // ID codes is defined in global variables at the top of this program.
    // Example request comes in from the host stereo like this:
    //   AVRCP 0 PDU_REGISTER_NOTIFICATION 4 TRACK_REACHED_END 0
    // We need to respond to each of these but there is a problem where they are not
    // all correctly documented in the BlueGiga documentation, and so many attempts
    // to respond to some of these result in SYNTAX ERROR. An example of a response
    // might look like:
    //      AVRCP NFY INTERIM 4 3 0
    // Where 4 is the transaction label, 3 is the event ID code for "Track
    // reached end", and 0 is the parameter. Though that one works, there are
    // others which do not work and get SYNTAX ERROR. In particular, I am currently
    // having trouble responding to the messages
    //
    // NOW_PLAYING_CHANGED    and    AVAILABLE_PLAYERS_CHANGED
    //
    // I have tried many different syntaxes and parameters and all result in
    // SYNTAX ERROR. Examples:
    //
    //    AVRCP 0 PDU_REGISTER_NOTIFICATION e NOW_PLAYING_CHANGED 0
    //    --> AVRCP NFY INTERIM e 9 0
    //    SYNTAX ERROR
    //
    //    AVRCP 0 PDU_REGISTER_NOTIFICATION d AVAILABLE_PLAYERS_CHANGED 0
    //    --> AVRCP NFY INTERIM d 10
    //    SYNTAX ERROR
    //
    // TO DO: Await response from Silicon Labs tech suppor on this one and fix up
    // this routine to respond correctly. My support ticket with Silicon Labs is:
    //
    // https://siliconlabs.force.com/5001M000017Jb5W
    //
    for (int l=0; l<rejMatrixSize; l++)
    {  
      // Make the string comparison to find out if there is a match.
      if (queryString.indexOf(rejMatrix[l][0]) > (-1))
      {
        // debugging
        // Log(F("Rejection found."));
        
        // Experiment: Screw this and simply reset the chip when it hits one
        // of these things. This works around the problem on my Honda stereo
        // because it stops asking for these messages to be registered on
        // the SECOND connection after startup (it only asks on the first
        // connection after the startup) and so this is the only thing
        // I can think of until Silicon Labs answers my support request.
        // This basically imitates what I have to do by hand at startup.
        // WARNING: This crazy messed up workaround has the potential
        // to put this whole thing into an infinite reboot loop, but
        // it's the best I've got until Silicon labs answers me.
        ForceQuickReconnect();
        return;

        // Below is the code which will respond to the queries that we
        // want to reject. If there is a line above to "return" then the
        // code below will not be hit. The code below does not work fully
        // at this time, it responds correctly to some registrations but
        // not to all of them, thanks to incomplete BlueGiga docs.

        // A match has been found, respond to the query
        // via experimental undocumented syntax.
        queryResponseString = "";

        queryResponseString += F("AVRCP NFY INTERIM ");

        // First parameter is the transaction label
        queryResponseString += transactionLabel;

        // Add a space after the transaction label to precede
        // the next parameter which will come from a string table.
        queryResponseString += F(" ");

        // Second parameter is the event ID from the table of events,
        // for example event ID 3 means the event named "TRACK_REACHED_END",
        // full details in section 6.3.3 of the BlueGiga iWrap AVRCP 
        // command reference documentation from Silicon Labs.
        queryResponseString += rejMatrix[l][1];

        // Final parameter(s) differ depending on the registration we're responding
        // to. Attempt to respond correctly to each registration request.
        // For some on the rejection list, there will be no additional parameters
        // other than what was already built up above and just let it fall through.
        // For the ones I'm unsure about, below are the work in progress responses,
        // not all of which work at the current time.
        if(rejMatrix[l][0] == F("BATT_STATUS_CHANGED"))
        {
          queryResponseString += F(" 0");   // 0 = Battery Status Normal
        }  

        if(rejMatrix[l][0] == F("SYSTEM_STATUS_CHANGED"))
        {
          queryResponseString += F(" 0");   // 0 = System Status Power On
        }  

        if(rejMatrix[l][0] == F("PLAYER_APPLICATION_SETTING_CHANGED"))
        {
          queryResponseString += F(" 1");   // 1 = There will be a single following key/value pair
          queryResponseString += F(" 1");   // 1 = parameter key is 1=Equalizer
          queryResponseString += F(" 1");   // 1 = Equalizer value is 1=Off - Hopefully should not affect our head unit.
        }  

        if(rejMatrix[l][0] == F("NOW_PLAYING_CHANGED"))
        {
          // try no parameters here. Spec says no parameters should be here. However that does not work.
          // try 
          queryResponseString += F(" 1 1 1");  // experiments
        }  

        if(rejMatrix[l][0] == F("AVAILABLE_PLAYERS_CHANGED"))  
        {
          // "The interim and final responses to the notify shall contain no parameters."
          // However that does not work. Try...
          queryResponseString += F(" 1 1 1");  // experiments
        }  

        if(rejMatrix[l][0] == F("ADDRESSED_PLAYERS_CHANGED"))  
        {
          queryResponseString += F(" 1");   // 1 = Unique media player id
          queryResponseString += F(" 0");   // 0 = "Database unaware players shall always return UIDcounter=0"
        } 

        if(rejMatrix[l][0] == F("UIDS_CHANGED"))  
        {
          queryResponseString += F(" 0");   // 0 = "Database unaware players shall always return UIDcounter=0"
        } 

        if(rejMatrix[l][0] == F("VOLUME_CHANGED"))  
        {
          queryResponseString += F(" 0");   // 0 = Absolute volume was set to zero. Hopefully should not affect our head unit.
        } 

        // Debugging
        // Log("-=: " + queryResponseString);

        // Response string is assembled, send it:
        SendBlueGigaCommand(queryResponseString);

        // Return out of this routine because we don't want to accidentally
        // respond to something below when we intended to reject it wholesale.
        return;
      }
    }
  }

  // Process PLAYBACK_STATUS_CHANGED if that is the query we received.
  // Example response might look like: "AVRCP NFY INTERIM 4 1 1"
  if (queryString.indexOf(F("PLAYBACK_STATUS_CHANGED")) > (-1))
  {
    // Debugging
    // Log(F("Respondable string found."));

    queryResponseString = "";

    // Answer with an "AVRCP NFY INTERIM" command.
    queryResponseString += F("AVRCP NFY INTERIM ");

    // First parameter is the transaction label
    queryResponseString += transactionLabel;

    // Second parameter is the event ID "1" meaning "PLAYBACK STATUS CHANGED"
    queryResponseString += F(" 1");
    
    // Next parameter is the play status which is one of:
    //        0 - Stopped
    //        1 - Playing
    //        2 - Paused
    //        3 - Fast forwarding
    //        4 - Rewinding
    // At the moment we only have playing or paused available as states on the empeg,
    // I don't know about "stopped", the empeg might not have a state for that at all.
    // Not yet implementing the "fast forwarding" and "rewinding" response states, as
    // the FF/REW functions seem to work fine without it in the one instance I tested
    // it (on a Kenwood car stereo). If we find a need to implement those other states
    // we can work on it later.
    if (empegIsPlaying)
    {
      queryResponseString += F(" 1");
    }
    else
    {
      queryResponseString += F(" 2");
    }

    // Debugging
    // Log("-=: " + queryResponseString);

    // Response string is assembled, send it:
    SendBlueGigaCommand(queryResponseString);
    return;
  }

  // Process TRACK_CHANGED if that is the query we received.
  // Example response might look like: "AVRCP NFY INTERIM 4 2 1"
  if (queryString.indexOf(F("TRACK_CHANGED")) > (-1))
  {
    // Debugging
    // Log(F("Respondable string found."));

    queryResponseString = "";

    // Answer with an "AVRCP NFY INTERIM" command.
    queryResponseString += F("AVRCP NFY INTERIM ");
    
    // First parameter is the transaction label
    queryResponseString += transactionLabel;

    // Second parameter is the event ID "2" meaning "TRACK_CHANGED"
    queryResponseString += F(" 2");
    
    // Final parameter is one of the following:
    //    0 – no track is currently selected
    //    1 – a track is currently selected
    // At the moment hard code this to a track is currently selected...
    queryResponseString += F(" 1");

    // Debugging
    // Log("-=: " + queryResponseString);

    // Response string is assembled, send it:
    SendBlueGigaCommand(queryResponseString);
    return;
  }

  // Respond to GET_PLAY_STATUS request. 
  // Example response might look like: "AVRCP RSP 4294967295 4294967295 1"
  if (queryString.indexOf(F("GET_PLAY_STATUS")) > (-1))
  {
    // Debugging
    // Log(F("Respondable string found."));

    queryResponseString = "";

    // Answer with an "AVRCP RSP" command.
    queryResponseString += F("AVRCP RSP ");
    
    // First two parameters are the length and position (in that order) of the track
    // in milliseconds. You can respond with 0xffffffff (which I think is 4294967295)
    // in those spots to indicate song length reporting is not supported.
    // IMPORTANT: When responding to GET_PLAY_STATUS you do not put your timestamp
    // surrounded by quote characters, whereas when responding to GET_ELEMENT_ATTRIBUTES
    // (done elsewhere), you do need to surround your timestamps with quotes. 
    queryResponseString += trackPlaybackLengthString;
    queryResponseString += " ";
    queryResponseString += trackPlaybackPositionString07;

     // And the play status which is one of:
    //        0 - Stopped
    //        1 - Playing
    //        2 - Paused
    //        3 - Fast forwarding
    //        4 - Rewinding
    // At the moment we only have playing or paused available as states on the empeg,
    // I don't know about "stopped", the empeg might not have a state for that at all.
    // Not yet implementing the "fast forwarding" and "rewinding" response states, as
    // the FF/REW functions seem to work fine without it in the one instance I tested
    // it (on a Kenwood car stereo). If we find a need to implement those other states
    // we can work on it later.
    if (empegIsPlaying)
    {
      queryResponseString += F(" 1");
    }
    else
    {
      queryResponseString += F(" 2");
    }

    // Debugging
    // Log("-=: " + queryResponseString);

    // Response string is assembled, send it:
    SendBlueGigaCommand(queryResponseString);
    return;
  }

  // Respond to GET_ELEMENT_ATTRIBUTES request. 
  // Example query might look like: "GET_ELEMENT_ATTRIBUTES 01 03" 
  // Example response might look like: "AVRCP RSP 1 3 \"Album on empeg Car\""
  if (queryString.indexOf(F("GET_ELEMENT_ATTRIBUTES ")) > (-1))
  {
    // Debugging
    // Log(F("Respondable string found."));

    // Zero out our response string to start
    queryResponseString = "";

    // Need to parse out the numbers after GET_ELEMENT_ATTRIBUTES which
    // indicate number of elements and which element. For example:
    // "GET_ELEMENT_ATTRIBUTES 01 03"
    // means, get one element, of element id "3" which is album title.
    //
    // Special processing: When the host stereo queries for track titles and other metadata,
    // it can do it in one of two ways:
    //    1. Separate queries for each individual piece of metadata, i.e., one query for
    //       Title, another query for Artist, another query for Genre, etc.
    //    2. A single query for multiple combinations of the metadata, such as querying
    //       for all of them, or a subset of them, all at once with a single query statement.
    // There is support in the bluetooth command set for both of those methods, the code below will
    // respond successfully to both of them.
    //
    // Note: The attribute parameters need to be surrounded by quote characters.
    // The code below is where I bake them in. Theoretically, by the time the metadata
    // strings reach this point, I have already stripped quote characters (if any) in the
    // track metadata, so that when I bake the quotes into the responses below, then
    // there is not a parsing problem on the host end. 

    // Obtain the first numeric group from the string to determine how
    // many elements we have to return in our response. For example in 
    // "GET_ELEMENT_ATTRIBUTES 02 04 05" we are looking for the "02"
    // which would indicate that we need to return two elements.
    //
    // First find the start position, which is the end of the
    // "GET_ELEMENT_ATTRIBUTES " including the space, which is 23 characters:
    elementsStartSelectPosition = queryString.indexOf(F("GET_ELEMENT_ATTRIBUTES ")) + 23;

    // Then find the end position which is the next space.
    elementsEndSelectPosition =  queryString.indexOf(F(" "), elementsStartSelectPosition);

    // If it's not found, we have a parsing error so bail out.
    if (elementsEndSelectPosition <= 23) {return;}

    // Debugging: Log what we found.
    // Log("String of the number of elements: *" + queryString.substring(elementsStartSelectPosition, elementsEndSelectPosition) + "*");

    // Turn it into an integer variable:
    numberOfElements = queryString.substring(elementsStartSelectPosition, elementsEndSelectPosition).toInt();

    // If the number is an unexpected size, we have a parsing error so bail out.
    // We only know how to parse elements numbered 1 through 7 so 7 is the max.
    if (numberOfElements < 1) {return;}
    if (numberOfElements > 7) {return;}

    // Start building the response string.
    queryResponseString += F("AVRCP RSP ");

    // Add the number of elements to the response string
    queryResponseString += String(numberOfElements);

    // Loop through all elements and query for each element ID
    // and build a completed response string out of them.
    for (int i=1; i<=numberOfElements; i++)
    {
      // Zero out the element code so that each time through the loop
      // we know that we will be getting a proper element code since
      // the value must be in the range of 1 through 7 if we parsed
      // it correctly.
      currentElementCode = 0;

      // Start by finding the prior spot in the string where we had
      // finished our selection and interpretation of the number. If this
      // is the first time through the loop, this will be the string
      // position immediately folloing the space after the number of
      // elements. If this is a subsequent time through the loop,
      // then this will be the string position immediately following
      // the space after the prior element code number.
      elementsStartSelectPosition = elementsEndSelectPosition + 1;

      // Now find the next trailing space if there is one. 
      elementsEndSelectPosition =  queryString.indexOf(F(" "), elementsStartSelectPosition);

      // Check to see if we even found a space at all.
      if (elementsEndSelectPosition <= elementsStartSelectPosition)
      {
        // Debugging output.
        // Log("Element code string was not followed by a space: *" + queryString.substring(elementsStartSelectPosition) + "*");

        // Parse the current element code out to the end of the string because we didn't find a space
        currentElementCode = queryString.substring(elementsStartSelectPosition).toInt();
      }
      else
      {
        // Debugging output.
        // Log("Element code string was followed by a space: *" + queryString.substring(elementsStartSelectPosition, elementsEndSelectPosition) + "*");

        // Parse the current element code out to the position of the space we found because we found a space
        currentElementCode = queryString.substring(elementsStartSelectPosition, elementsEndSelectPosition).toInt();
      }

      // Add the element code to the response string with spaces around it.
      queryResponseString += " " + String(currentElementCode) + " ";

      // Add the text of the element itself to the response string,
      // based on the current codenumber for which elements were
      // queried for.
      switch (currentElementCode)
      {
        case 1:
          queryResponseString += ("\"" + trackTitleString01 + "\"");
        break;

        case 2:
          queryResponseString += ("\"" + trackArtistString02 + "\"");
        break;

        case 3:
          queryResponseString += ("\"" + trackAlbumString03 + "\"");
        break;

        case 4:
          queryResponseString += ("\"" + trackNumberString04 + "\"");
        break;

        case 5:
          queryResponseString += ("\"" + trackTotalNumberString05 + "\"");
        break;

        case 6:
          queryResponseString += ("\"" + trackGenreString06 + "\"");
        break;

        case 7:
          queryResponseString += ("\"" + trackPlaybackPositionString07 + "\"");
        break;

        default:
          Log(F("There was a parsing error related to parsing one of the metadata elements."));
          Log(F("Element number was not in the expected range of 1-7."));
          Log(F("String that had been parsed was this string:"));
          Log(queryString);
          return;
        break;
      }
    }

    // Debugging
    // Log("-=: " + queryResponseString);
    
    // Response string is assembled, send it:
    SendBlueGigaCommand(queryResponseString);
    return;
  }

    // Debugging: 
    Log(F("Dropping out of the bottom of the RespondToQueries function without responding."));
}

// ----------------------------------------------------------------------------
// ForceQuickReconnect
//
// Quickly disconnect and reconnect to the currently-connected host stereo
// to work around an issue. The issue is that my car stereo starts asking
// questions that I don't know how to answer, and when this happens, I can
// always solve the problem by disconnecting and reconnecting. The questions
// that it is asking is that it's doing PDU_REGISTER_NOTIFICATION for events
// which are not well documented in the BlueGiga iWrap command language docs.
// Any response I attempt to give for these event registrations results in 
// a SYNTAX ERROR from iWrap. I have found that these only occur on the first
// connection after powerup, and that if I disconnect and reconnect then
// after that I stop being asked for these registrations. This is a dirty
// workaround until I can get an answer back from Silicon Labs Tech Support.
//
// Note 1: This routine assumes that the blueooth chip is connected to the
// host stereo at the time that this function is called.
//
// Note 2: This workaround has the potential to put the unit into an infinite
// reconnect loop if the host stereo doesn't stop asking the unanswerable
// questions after the reconnect. My stereo doesn't do that, but others might.
// ----------------------------------------------------------------------------
void ForceQuickReconnect()
{
  // TO DO: Add a section here which only allows this to occur once per
  // powerup session by checking a global flag variable and toggling it here.
  // This would prevent an infinte reboot loop but would still not solve
  // the larger problem.

  // TO DO: If a proper answer is not forthcoming from Silicon Labs tech supprt
  // then make this into an optional workaround driven by a variable at the
  // top of the program.

  // Make sure bluetooth chip configured and ready to
  // immediately reconnect as soon as it disconnects.
  SendBlueGigaCommand(autoReconnectString);

  // Disconnect the bluetooth and count on the reconnect
  // configuration to do its job and reconnect automatically.
  // This is not the greatest because the reconnect after the boot
  // is still longer than I'd like it to be.
  QuickResetBluetooth(1);

  // TO DO: If a proper answer is not forthcoming from Silicon Labs tech support
  // then this needs to be made more robust and quicker. This would be done by
  // recording the paired device's address (storing it based on the PAIR command
  // for situations when this device inititated pairing, or the RING command,
  // for situations when the host stereo initiated pairing). 
}

// ----------------------------------------------------------------------------
// DisplayAndSwallowResponses
//
// For either a set amount of time, and/or a set amount of response messages,
// whichever occurs first, display all responses received from the bluetooth 
// chip onto the Arduino debug port and then do nothing about them: do not
// try to process the strings or execute any commands for that period of
// time.
//
// Parameters:
// - The number of possible response messages to swallow.
// - The number of milliseconds to wait for each of those possible responses.
// 
// Note:
// The max total wait time will be the first parameter times the second
// parameter, if no messsages are received. If any message is received,
// then the wait shortens by that portion of the wait loop.
// ----------------------------------------------------------------------------
void DisplayAndSwallowResponses(int numResponsesToSwallow, int waitTimePerResponseMs)
{  
  static char swallowChar = 0;
  
  // Wait for response characters.
  for (int m=0; m < numResponsesToSwallow; m++) // swallow up to this many messages...
  {
    for (int t=0; t < waitTimePerResponseMs; t++) // ...for up to this many milliseconds per possible message
    {
      swallowChar = 0;
      if (BlueGigaSerial)
      {
        if (BlueGigaSerial.available())
        {
          swallowChar = BlueGigaSerial.read();
          LogChar(swallowChar);            
          if (swallowChar == '\n')
          {
            break;
          }
        }
        else
        {
          delay(1);
        }
      }
    }
  }
}


// ----------------------------------------------------------------------------
// SetTrackMetaData
//
// Set the metadata for the currently playing track in memory so that it can
// be responded to as soon as the host queries for this information.
// 
// Parameters:
// stringToParse: The string to parse from the Empeg, such as "TRed Barchetta"
// or "ARush" or "GProgressive Rock".
// empegMessageCode: The single character of the message code from the
// empeg which preceded the string to pars such as "G" or "T" or whatever.
//
// Track data coming from the empeg will look something like this by the
// time it reaches this routine (the single letter is the empegMessageCode and
// the string following the letter is the stringToParse):
//
//  N  <tracknumber, actually playlist position, starts at zero>
//  Z  <album name, added by Mark Lord in recent hijack versions>
//  D  <track duration in milliseconds, added by Mark Lord in recent hijack versions>
//  T  <track title>
//  A  <artist>
//  G  <genre>  
// ----------------------------------------------------------------------------
void SetTrackMetaData(String stringToParse, char empegMessageCode)
{
  // Pre-strip doublequote characters out of the track data because
  // our messages to the bluetooth chip need doublequote characters
  // as delimiters when we send metadata up to the host stereo.
  // This fixes that particular problem, but not optimally.
  // TO DO: Figure out how to PROPERLY escape single and double
  // quotes in track metadata in the BlueGiga iWrap command language.
  stringToParse.replace(F("\""), F("''"));

  // Pre-strip single quote characters too, I believe I ran into a
  // problem with this track title by The Police:
  //     Hungry for You (J'aurais Toujours Faim de Toi)
  // This fixes that particular problem, but not optimally.
  // TO DO: Figure out how to PROPERLY escape single and double
  // quotes in track metadata in the BlueGiga iWrap command language.
  // UPDATE: This turns out to not actually be required at all. The
  // issue I had with that particular song title was a serial buffer
  // problem and unrelated to the single quote.
  //         stringToParse.replace(F("'"), F("`"));

  // Make sure string is trimmed of white space and line breaks
  stringToParse.trim();

  // Debug Logging
  // LogChar(' ');
  // LogChar(' ');
  // Log(stringToParse);

  // Now set the desired metadata based on the values we passed in.
  // This routine only gets one line at a time so we only need to
  // process one of them each time and then return.

  // Track Number. Actually this isn't really the track
  // number, it's the playlist position in the current
  // running order, but hey, we work with what we've got.
  // TO DO: Find out how to get the actual tracknumber
  // from the empeg's track as opposed to the playlist
  // position and optionally display that instead. The
  // actual track number is one of the fields in the
  // empeg track metadata.
  if (empegMessageCode == 'N')
  {
    trackNumberString04 = stringToParse;  // Set value
    if (displayTracksOnSerial)
    {
      Log(F("---------------------------------------------------"));
      LogChar(' ');
      LogChar(' ');
      Log(trackNumberString04);
    }
    return;
  }

  // Track Album Name. Added by Mark Lord in a recent version of Hijack.
  if (empegMessageCode == 'Z')
  {
    trackAlbumString03 = ReplaceHighAsciiWithUtf8(stringToParse);
    if (displayTracksOnSerial)
    {
      LogChar(' ');
      LogChar(' ');
      Log(trackAlbumString03);
    }
    return;
  }

  // Track Duration in milliseconds, but presented to us as a string.
  // Added by Mark Lord in a recent version of Hijack.
  // We don't need to do math on this, we just need to turn it around
  // and send it right back up the bluetooth as a string as-is.
  if (empegMessageCode == 'D')
  {
    trackPlaybackLengthString = stringToParse;
    if (displayTracksOnSerial)
    {
      LogChar(' ');
      LogChar(' ');
      Log(trackPlaybackLengthString);
    }
    return;
  }

  // Track Title.
  if (empegMessageCode == 'T')
  {
    trackTitleString01 = ReplaceHighAsciiWithUtf8(stringToParse);
    if (displayTracksOnSerial)
    {
      LogChar(' ');
      LogChar(' ');
      Log(trackTitleString01);
    }
    return;
  }

  // Artist.
  if (empegMessageCode == 'A')
  {
    trackArtistString02 = ReplaceHighAsciiWithUtf8(stringToParse);
    if (displayTracksOnSerial)
    {
      LogChar(' ');
      LogChar(' ');
      Log(trackArtistString02);
    }
    return;
  }
    
  // Genre.
  if (empegMessageCode == 'G')
  {
    trackGenreString06 = ReplaceHighAsciiWithUtf8(stringToParse);
    if (displayTracksOnSerial)
    {
      LogChar(' ');
      LogChar(' ');
      Log(trackGenreString06);
      Log(F("---------------------------------------------------"));
    }
    return;
  }

  // Sanity check, this code should never hit.
  //  Log(F("WARNING: DROPPED OUT OF BOTTOM OF SetTrackMetaData ROUTINE UNEXPECTEDLY."));
}

// ----------------------------------------------------------------------------
// ReplaceHighAsciiWithUtf8
// 
// Fix track metadata strings which contain High ASCII characters and replace
// them with UTF-8 characters.
// 
// My empeg's track/artist/album fields contained High Ascii characters such as
// the "Ö" in "Blue Öyster Cult". These appear as a "unrecognized symbol" on my
// car stereo's LCD touchscreen UI. This function translates those symbols into
// UTF-8 characters, which are then accepted and displayed correctly on the car
// stereo's LCD touchscreen interface.
//
// NOTE: If your empeg's track/artist/album fields are already in UTF-8,
// then disable this function by changing this global variable at the top of this
// program to "false":
//
//    boolean PerformUtf8Conversion = false;
//
// See the following web sites for information about the translation between
// UTF-8 and High ASCII:
// 
//      https://playground.arduino.cc/Main/Utf8ascii
//      http://kellykjones.tripod.com/webtools/ascii_utf8_table.html
//
// Parameter:  The string to convert (the input string).
// Returns:    The converted string. May be longer than the input string.
// ----------------------------------------------------------------------------
String ReplaceHighAsciiWithUtf8(String stringToMakeUtf8Char)
{
  // Check to see if we are even supposed to perform this conversion at all.
  // Look at the global variable, and if it's false, return the same string
  // as input, with no conversion.
  if (PerformUtf8Conversion == false)
  {
    return stringToMakeUtf8Char;
  }

  // Set a static variable to keep track of the character we
  // are pulling out of the string to analyze. Do this as a
  // "byte" instead of a "char" to make our calculations look
  // more straightforward, because a "byte" is unsigned 0-255
  // while a "char" is signed -127 to +127.
  static byte oneStringChar = 0;

  // Create string that we will be using to return from this function.
  // Make sure it starts out empty.
  String utf8ReturnString = "";

  // Loop through each character in the string and convert it
  // to UTF-8, adding it back into the return string as we go.
  for (int loopVar=0; loopVar < stringToMakeUtf8Char.length(); loopVar++)
  {
    // Get the character found at that position of the input string.
    oneStringChar = stringToMakeUtf8Char.charAt(loopVar);

    // Debugging output, disable in final release version.
    // Prints out the decimal values of each character in 
    // the converted string so I can see them.
    //     Serial.print(oneStringChar, DEC);
    //     Serial.print(F("\r\n"));

    // Low ASCII values, i.e. 0-127, are the same in UTF-8 and ASCII.
    if ((oneStringChar >= 0) && (oneStringChar <= 127))
    {
      // If the character is not high ASCII, then
      // just add the character straight back into the
      // return string with no changes.
      utf8ReturnString += char(oneStringChar);
    }
    
    // High ASCII values 128 through 191 are the same as ASCII but preceded by a hex C2 byte.
    if ((oneStringChar >= 128) && (oneStringChar <= 191))
    {
      // Debugging output, disable in final release version
      //   Log(F("--------------------- UTF-8 0xC2 conversion. -----------------------"));

      // Insert a hex C2 byte before inserting the same character as the original input string in this position.
      utf8ReturnString += char(0xC2);
      utf8ReturnString += char(oneStringChar);
    }
    
    // High ASCII values 192 through 255 are preceded by a hex C3 byte and the character value is different by a fixed amount.
    if ((oneStringChar >= 192) && (oneStringChar <= 255))
    {
      // Debugging output, disable in final release version
      //  Log(F("--------------------- UTF-8 0xC3 conversion. -----------------------"));

      // Insert a hex C3 byte first.
      utf8ReturnString += char(0xC3);
      
      // Modify the original character to be 64 (0x40) less than it was before (0xC0 becomes 0x80 for example). 
      utf8ReturnString += char(oneStringChar - 64);
    }
  }

  // Return our return string
  return utf8ReturnString;
}

// ----------------------------------------------------------------------------
// SendEmpegCommand
//
// Send a command to the empeg serial port to make the empeg do a desired behavior
// such as N\n for next track, P\n for previous track, W\n for pause, etc.
//
// Parameters:
// - The command string to send, without the linefeed. Linefeed will be added.
// ----------------------------------------------------------------------------
void SendEmpegCommand(char empegCommandToSend)
{
  // Turn on the Arduino LED to indicate something has happened.
  digitalWrite(LED_BUILTIN, HIGH);

  // Set the empeg's state variable based on the command we will send.
  // Ideally we would always let the empeg tell us what state it's in
  // and never change this state ourselves. However there is a special
  // case which needs this. It is the case when the player state quickly
  // switches from play to pause or vice versa and the host stereo
  // queries for the play status. Sometimes the host stereo queries
  // for the status only once and it does it too quickly before the
  // empeg has a chance to tell us what its state is. So the response
  // to the host stereo's playback status query can be the wrong value.
  // so when we send a pause or a play to the empeg we quickly set
  // is state here so that we can respond correctly to the query
  // when it comes.
  if (empegCommandToSend == 'W')
  {
    // Only change the empeg player state if the empeg player is listening.
    if (empegPlayerIsRunning)
    {
      // Set the empeg state variable to "paused"
      empegIsPlaying = false;
    }
    // Send a certain group of messages when our play state changes.
    HandleEmpegStateChange();
  }
  if (empegCommandToSend == 'C')
  {
    // Only change the empeg player state if the empeg player is listening.
    if (empegPlayerIsRunning)
    {
      // Set the empeg state variable to "playing"
      empegIsPlaying = true;
    }

    // Send a certain group of messages when our play state changes.
    HandleEmpegStateChange();
  }

  // Only bother to send commads if we believe the empeg has finished booting
  // up and we think we will be successful at sending the command. If not,
  // log the failure.
  if (!empegPlayerIsRunning)
  {
    Log("Unable to send command to empeg, the empeg player application is not yet running: " + (String)empegCommandToSend);
  }
  else
  {
    // Log what we are sending.
    // Log("Sending to empeg: " + (String)empegCommandToSend);

    // Write out the corresponding Empeg command to the Empeg's
    // serial port followed by a linefeed character.
    if (EmpegSerial)
    {
      EmpegSerial.print(empegCommandToSend);
      EmpegSerial.write('\n');
    }
    
    // Turn off the Arduino LED.
    digitalWrite(LED_BUILTIN, LOW);
  }
}


// ----------------------------------------------------------------------------
// HandleEmpegString
// 
// Function to process the string data received from the empeg car serial port.
// Locate strings which look like notifications of track data and turn those
// into variables for later sending up the bluetooth.
// ----------------------------------------------------------------------------
void HandleEmpegString(String theString)
{
  // To save memory, preset static variables for a certain set of string
  // data to be extracted from the string found on the empeg serial port.
  static String empegDetailString = "";
  static String empegTimecodeString = "";
  static char empegMessageCode = ' ';
  static int empegMessageCodeStartPos = 0;
  static int empegDetailStringStartPos = 0;
  static int empegHours = 0;
  static int empegMinutes = 0;
  static int empegSeconds = 0;
  static long empegMs = 0;
  static bool foundEmpegMessageCode = false;

  // Lists of characters we will be searching for as our
  // empeg message code strings. The track metadata codes
  // look like this, and they always appear in a group with
  // the Genre at the end as the last one in the group.
  //
  // serial_notify_thread.cpp: 116:@@ N<tracknumber, actually playlist position, starts at zero>
  // serial_notify_thread.cpp: 117:@@ F<fid> (we don't care about this one and will not process it)
  // serial_notify_thread.cpp: 191:@@ Z<album name, added by Mark Lord in recent hijack versions>
  // serial_notify_thread.cpp: 192:@@ D<track duration in milliseconds, added by Mark Lord in recent hijack versions>
  // serial_notify_thread.cpp: 118:@@ T<track title>
  // serial_notify_thread.cpp: 119:@@ A<artist>
  // serial_notify_thread.cpp: 120:@@ G<genre>  
  //
  static char empegMessageCodeTrackDataList[6]  = { 'N', 'Z', 'D', 'T', 'A', 'G',};
  static int empegMessageCodeTrackDataListLen = 6;

  // In addition there will also be a few other messages
  // from the serial port we care about. We will keep these
  // as part of a larger list for general pre-check processing
  // to make sure we found any codes at all in the list.
  //
  // serial_notify_thread.cpp: 170:@@ S1    <single digit code for track playback status - 0=paused 1=playing>
  // serial_notify_thread.cpp: 180:@@ #4830  0:00:00   <timestamp of current playback position preceded by FID number>
  // serial_notify_thread.cpp: 180:@@ #4830  0:00:01
  // serial_notify_thread.cpp: ???:@@ R<number of tracks in current running order, added by Mark Lord in recent hijack versions>
  //                                     Note that R will appear intermittently, separate from all of the above  
  static char empegMessageCodeList[9]  = { 'N', 'Z', 'D', 'T', 'A', 'G', 'S', '#', 'R', };
  static int empegMessageCodeListLen = 9;

  // Start this routine by setting our static variables to blank state for later retrieval.
  foundEmpegMessageCode = false;
  empegDetailString = "";
  empegTimecodeString = "";
  empegMessageCode = ' ';
  empegMessageCodeStartPos = -1;
  empegDetailStringStartPos = -1;
  empegHours = -1;
  empegMinutes = -1;
  empegSeconds = -1;
  empegMs = 0;

  // If the string indicates that the empeg player application is just booting up,
  // then turn off our ability to send things to it so that we don't confuse our
  // playing state too much. Trigger on a few different strings found in bootup.
  if (theString.startsWith(F("empeg-car bootstrap")))
  {
    empegPlayerIsRunning = false;
    empegIsPlaying = false;
  }
  if (theString.startsWith(F("Uncompressing Linux")))
  {
    empegPlayerIsRunning = false;
    empegIsPlaying = false;
  }
  if (theString.startsWith(F("ide_data_test")))
  {
    empegPlayerIsRunning = false;
    empegIsPlaying = false;
  }


  // If the string is our string which indicates the empeg player has started...
  if (theString.startsWith(F("Prolux 4 empeg car ")))
  {
    // If the empeg player application is running, set our global variable to indicate that
    // it is running so that we do the correct behavior and detect the correct states 
    if (!empegPlayerIsRunning)
    {
      Log(F("-------------------------------------"));
      Log(F("empeg player application has started."));
      Log(F("-------------------------------------"));
      empegPlayerIsRunning = true;
    }

    // Experimental - Quick special case - If we get a player startup message from the empeg,
    // indicating that its boot sequence is done and the player app has started, then send a
    // pause command to the player as an attempt to help the start/stop/start behavior of the
    // player at initial car+empeg+stereo+bluetooth bootup. 
    // 
    // Notes on this experiment: 
    // - Might cause player to come up and be in "pause" mode if the bluetooth pairup speed
    //   beats the empeg bootup speed. In my car, the bluetooth pairup is always slower than
    //   the empeg bootup, but for some other devices like bluetooth headsets this might win.
    // - Currently using the Prolux Visuals message as the way of finding the startup of the
    //   player because the "Starting player" message is a little iffy and not always detected
    //   by this routine. Not sure why but it didn't always work when I tried it.
    //
    if (empegStartPause)
    {
        Log(F("Detected empeg bootup sequence completion, sending pause command to player."));
        SendEmpegCommand('W');
    }
  }
  
  // Look for the string position of our trigger phrase in
  // the text of the string we received from the empeg.
  // Example of string we might see would be
  //     "  serial_notify_thread.cpp: 170:@@ S0"
  empegMessageCodeStartPos = theString.indexOf(F("@@ "));

  // Check to see if the trigger phrase was found.
  if (!(empegMessageCodeStartPos > -1))
  {
    // If the string from the empeg doesn't contain our trigger
    // phrase, then ignore it and return from our subroutine.    
    return;
  }
  else
  {
    // Any receipt of the trigger phrase message means that the player application
    // has started (maybe we missed the first player startup message or something) so in this
    // case we also want to set our global variable to indicate that the player application
    // is running so that we do the correct behavior and detect the correct states.
    if (!empegPlayerIsRunning)
    {
      Log(F("-------------------------------------"));
      Log(F("empeg player application has started."));
      Log(F("-------------------------------------"));
      empegPlayerIsRunning = true;
    }

    // Parse out the details of the string after the trigger phrase.
    empegMessageCodeStartPos += String(F("@@ ")).length();
    empegDetailString = theString.substring(empegMessageCodeStartPos);

    // empegDetailString now contains everything after the trigger phrase.
    // Example: if theString started as:    "  serial_notify_thread.cpp: 170:@@ S0"
    // then empegDetailString is now:       "S0"
    // This will also work if we are using a recent version of Mark Lord's Hijack
    // which deliberately shortens these strings down before we get them, so that the
    // string above might have started as "@@ S0" too. This should work for both formats.

    // At this point it possibly still contains a carriage return or line feed at the end,
    // so perform a trim on it (in-place trim of string) to clean whitespace and CR/LFs.
    empegDetailString.trim();

    // Now obtain the message code as a single character following the space
    // at the end of the trigger phrase. It is the first character of what we
    // have remaining. For instance, in: "  serial_notify_thread.cpp: 170:@@ S0"
    // the message code is "S"
    empegMessageCode = empegDetailString.charAt(0);
    if (empegMessageCode == ' ')
    {
      // If no string was retrieved then there is something
      // wrong and we do not have a coded string. Perhaps it
      // was a mangled piece of data with our trigger phrase
      // at the end of the string. Bail out of the routine.
      //   Log("Message code flag NOT found, empty string.");
      return;
    }

    // If we found a character, then log it and check it
    // to see if it's what we want, if it's in the list.
    //  Log("Message code flag being tested for acceptance: " + empegMessageCode);

    // Precheck to make sure that we really got one of the
    // message codes that we were hoping for. It should
    // match one of the strings in this list if it is.
    for (int m=0; m < empegMessageCodeListLen; m++)
    {
      // Check to see if the code is in our list of acceptable codes
      if (empegMessageCode == empegMessageCodeList[m])
      {
        // Code was found, flag it, log it, and break out of the
        // FOR loop completely because we don't need to check any more.
        //   Log("Valid empeg message code flag found: " + empegMessageCode);
        foundEmpegMessageCode = true;
        break;
      }
    }

    // If we didn't find a code we were looking for, then
    // we can bail out of this routine right now.
    if (!foundEmpegMessageCode)
    {
      //  Log(F("Valid empeg message code flag was not found. Exiting routine."));
      return;
    }

    // And then now that we're done getting a good message code, let's shorten the detail
    // string even more by stripping out the message code and leaving just the paramter
    // data after the message code. The "substring" function has a zero-based index, so
    // setting the start parameter of 1 means starting at the second character. And leaving
    // the ending parameter blank means go to the end of the string. So in this case, if
    // the detail string got here as, say, "ZCelery Stalks at Midnight" then the resulting
    // string would now be "Celery Stalks at Midnight".
    empegDetailString = empegDetailString.substring(1); 

    // Debugging only: Log what we parsed
    //   Log("empegDetailString: " + empegDetailString); 
  }

  // If we received any track metadata from the empeg, specifically,
  // if it's one of the ones that comes all in a group at once, such
  // as the track title and artist and such, set that data.
  for (int m=0; m < empegMessageCodeTrackDataListLen; m++)
  {
    if (empegMessageCode == empegMessageCodeTrackDataList[m])
    {
      // Found one. Call the routine to parse and set the track
      // metadata that all comes in one group all at once, such
      // as track title and artist and such.
      SetTrackMetaData(empegDetailString, empegMessageCode);

      // NOTE: The next line is super important. Every time the empeg gets
      // a state change, we need to send a command to the bluetooth to make it
      // re-query us for the track information. In this way, if the track changes
      // on the empeg due to a state change then it will ask us for the new
      // track data. This will need to happen each time the empeg changes track
      // number, metadata, playback length, etc (though not playback position).
      // This will need to happen both when the bluetooth initiates the change
      // (ie the user presses NEXT on his steering wheel) and also when the empeg
      // moves on to the next track naturally itself (i.e. you're just letting
      // the empeg run and it goes to the next track and now new track data has
      // appered on its serial port output). This should happen only when the
      // empeg itself has said that the data has changed, in other words, only
      // when the track metadata is different from whatever it was prior. There
      // is a serial data transmit/receive cost and a processing cost associated
      // with the (fairly large) set of queries that it makes when it gets this
      // message, so you should only do it when the track actually changes.
      // 
      // Currently trying to set this only when we get a "Genre" change ("G")
      // since that is the last of the track information sets to appear on the
      // serial port in a block of messages. So if we set it at the end then
      // we only do this once, rather than for every line of data we recieve.
      // You may be wondering: What happens if the Genre of a track isn't
      // set at all, or if it's set to a null string. Does the emepg serial
      // port still report the Genre? Does it still send a message to the
      // serial port with code "G", even if the Genre is just a blank string?
      // Well, I tested this and it does indeed send a line with the "G" trigger
      // and just blank characters after the G. So we will send a null string
      // and the car stereo will get a null string and just put blank or
      // unknown in that spot on its screen. At least that's the way mine worked.
      if (empegMessageCode == 'G')
      {
         HandleEmpegStateChange();
      }

      // If it was one of these pieces of data (like a track title or such)
      // then we are done and can return from this routine to save time.
      return;
    }
  }

  // If we received the information from the emepg serial port indicating that it is now playing, then do the needful
  if ((empegMessageCode == 'S') && (empegDetailString == "1"))
  {
    empegIsPlaying = true;
    HandleEmpegStateChange();
    return;
  }
  
  // If we received the information from the emepg serial port indicating that it is now paused, then do the needful
  if ((empegMessageCode == 'S') && (empegDetailString == "0"))
  {
    empegIsPlaying = false;
    HandleEmpegStateChange();
    return;
  }

  // Total tracks in current playlist is not output on the empeg serial by default but
  // with a new version of Mark Lord's Hijack then we can now parse this information
  // sometimes. Handle that here because it will arrive at intermittent times due.
  // to the fact that Hijack only gets this data after the disk drive spins down.
  if (empegMessageCode == 'R')
  {
    // Debug logging
    //  Log(F("Playlist length code R has been received from empeg serial port."));

    // Check to see if this string differs from what we already had in
    // the global variable, and perform different steps depending on 
    // whether this is new information (thus requiring a whole trip
    // into the state change routine) or if it's just a repeat of
    // what we already know, thus not needing the extra processing.
    if (trackTotalNumberString05 == empegDetailString  )
    {
      // Log (F("Playlist length was not new, not updating."));
    }
    else
    {
      // Log("New Playlist length: " + empegDetailString);

      // Assign the global playlist length variable to the value Mark so kindly gave us.
      trackTotalNumberString05 = empegDetailString;  

      // Tell the head unit that we have a new playlist length to report to it.
      // TO DO: Test to see if this works and if it's "Too much" for the
      // system to have this happening too often.
      HandleEmpegStateChange();
    }
  }

  // If we received information from the empeg serial port that our playback position has changed, then do the needful.
  // The playback position is tricky because the line includes the FID number that we have to strip out and also it's
  // in ascii HH:MM:SS format whereas the bluetooth chip wants it in milliseconds, so we'll have to do some work here.
  // Example of full string: "  serial_notify_thread.cpp: 180:@@ #f4f0  0:00:12"
  // By the time it reaches this code it will be:               "#f4f0  0:00:12"
  if (empegMessageCode == '#')
  {
    // Preset the hours minutes and seconds to invalid values as a sanity check
    // (will check that these are at least zero or higher at the end of the parsing routine).
    empegHours = -1;
    empegMinutes = -1;
    empegSeconds = -1;

    // Split at the first space it finds. In my example above, the first space
    // is actually two spaces, but I think there is a chance that the hour field
    // might be space-padded, so plan on it being one space (or not) and then
    // whatever it finds after that. Note: The +1 is so that it get the string
    // starting AFTER the first space, not INCLUDING the first space.
    empegTimecodeString = empegDetailString.substring(empegDetailString.indexOf(F(" ")) +1 );
    
    // Debug: Log what we are parsing, our string should look like " 0:00:12" at this point,
    // including the possibility of one leading space if the hour is space-padded (not sure).
    // Log ("Parsing timecode: " + empegTimecodeString);   

    // Sanity check: Make sure there is at least one colon, otherwise bail out.
    if (!(empegTimecodeString.indexOf(F(":")) > -1))
    {
      Log(F("Error decoding timestamp string. First colon not found."));
      return;
    }    

    // Split into hour, minute, and second.
    // First get the integer of the value of the digits before the first colon (the hours)
    empegHours = empegTimecodeString.substring(0, empegTimecodeString.indexOf(F(":"))).toInt();

    // Now shorten the string and get everything after the first colon.
    // String should now look like "00:12" - reduced down to minutes and seconds
    empegTimecodeString = empegTimecodeString.substring(empegTimecodeString.indexOf(F(":")) +1);

    // Now get the minutes (integer of the value of the digits before the remaining colon)
    empegMinutes = empegTimecodeString.substring(0, empegTimecodeString.indexOf(F(":"))).toInt();

    // Sanity check: Make sure that last remaining colon is left there, otherwise bail out.
    if (!(empegTimecodeString.indexOf(F(":")) > -1))
    {
      Log(F("Error decoding timestamp string. Second colon not found."));
      return;
    }    
    
    // Now shorten the string and get everything after the remaining colon.
    // String should now look like "12" - it's now just the seconds left.
    empegTimecodeString = empegTimecodeString.substring(empegTimecodeString.indexOf(F(":")) +1);

    // Get the decmial value of the remaining section, the seconds
    empegSeconds = empegTimecodeString.toInt();

    // Sanity check that there are good values inside all of these.
    // These values were PRE SET TO NEGATIVE so that we could do
    // this check right here. If they reach this point negative, then
    // something went wrong. So check for negative values.
    if ((empegHours < 0) || (empegMinutes < 0) || (empegSeconds < 0))
    {
      Log(F("Parsing error when trying to parse timestamp from empeg serial output."));
      Log(F("Either the hours, seconds, or minutes was less than zero."));
      return;
    }

    // Calculate the timestamp in milliseconds.
    // Note: must append an "L" to the constant values so that they
    // calculate as longs in the final calcuation instead of ints.
    empegMs = (empegHours * 1000L * 60L * 60L) + (empegMinutes * 1000L * 60L) + (empegSeconds * 1000L);


    // The next section of code must only be done if it was NOT the first timestamp
    // message we ever received from the empeg since boot up. Because the first 
    // timestamp we receive will almost always be different from the timestamp in
    // global memory, the very first one will frequently be flagged as having made
    // a pause/play-state change even if there really wasn't one. So instead only
    // do this in cases where it wasn't the first timestamp. If we hit this routine
    // and it WAS the first timestamp, then flip the flag to false for the rest of
    // the program run.
    if (empegFirstTimestamp)
    {
      // This was the first timestamp we've seen since program bootup, so flip the
      // flag so that we take action on the second timestamp and all subsequent ones.
      Log(F("First timestamp received from empeg Car - Not performing play-pause state change code just yet."));
      empegFirstTimestamp = false;
    }
    else
    {
      // This was NOT the first timestamp we've received from the empeg, so we can
      // take some action on the timestamp.
      //
      // Check empegMs against our global variable trackPlaybackPositionMs, so that
      // we can determine if the playback counter position has changed since the last
      // time that we received a message about it. If it is different, we perform a
      // special case fix: update the empegIsPlaying flag to indicate that the empeg
      // is playing because the track times are changing. This is to work around a
      // bug where sometimes the empeg doesn't report that it has started playing
      // again after unpausing from its front panel. The bug causes the play/pause
      // indicator on the car stereo's touchscreen to display the wrong playback
      // state, indicating that the player is paused when it really isn't paused.
      // So we will work around this by updating the state variable if we see
      // the timestamps changing. There is a drawback to this, which is, if you
      // are performing FF and REW from the front panel of the player while the
      // playback is paused (this is one of the empeg's existing capabilities),
      // then the car stereo head unit will think that playback state is "playing"
      // during the time that you are FF/REWing on the empeg front panel. This is
      // OK, this is a better thing to have than the incorrectly-displayed playback
      // state during normal playback. Though there is the possibility to respond
      // with a particular playback state to the headunit of "fast forwarding" or
      // "rewinding", but this is not implented in my code yet. So for now, we have
      // two possible states, playing and paused, and if we detect that the empeg
      // has changing track timestamps then we will update that state.
      if (empegMs != trackPlaybackPositionMs)
      {
        // The empeg is considered to be playing at this point because
        // the current track playback position is different from the last
        // one that we read from the serial port. Now, check to see if the
        // state of the empeg playback has changed from what was previously
        // expected, and if so, display a message as well as flipping the
        // playback state variable. Must flip the variable first then display
        // the message that the state had changed.
        if (empegIsPlaying == false)
        {
          empegIsPlaying = true;
          HandleEmpegStateChange();
        }
        // If the playback state wasn't wrong to begin with, no action is needed.
      }
      else
      {
        // The empeg is considered to be paused at this point because
        // the current track playback position is unchanged from the last
        // time. Do the opposite variable-flip as above.
        if (empegIsPlaying == true)
        {
          empegIsPlaying = false;
          HandleEmpegStateChange();
        }
        // If the playback state wasn't wrong to begin with, no action is needed.
      }
    }

    // Now that we are done checking whether or not the timestamp has changed,
    // now we can update the global "holder" of the playback time to reflect what
    // we read from the serial port message. Now it is stored globally and ready 
    // to be checked against the next playback position value that we receive from
    // the empeg serial port.
    trackPlaybackPositionMs = empegMs;

    // Convert milliseconds to a string and assign it to our global playback
    // position string as a string value that can be sent to the bluetooth chip.
    trackPlaybackPositionString07 = String(empegMs);

    // Debug output of the playback milliseconds.
    // Log("Hours: " + String(empegHours));
    // Log("Minutes: " + String(empegMinutes));
    // Log("Seconds: " + String(empegSeconds));
    // Log("Playback Milliseconds: " + trackPlaybackPositionString07);
  }
}

// ----------------------------------------------------------------------------
// HandleEmpegStateChange
//
// When the empeg's playing state changes (playing or paused), there are
// several things that need to be done, some commands than need to be sent
// to the bluetooth chip to let it know about some things. Also there are
// some things I'd like to do when the track information changes from one
// track to the next. I think that I can do all of them here all at once.
// ----------------------------------------------------------------------------
void HandleEmpegStateChange()
{
  if (empegIsPlaying)
  {
    // Special case: Make sure streaming is started so that there is
    // no "cutoff" of the start of the song if we press the play button
    // on the stereo's touch screen. Must do this before we send the
    // command to the empeg so that it happens first.
    SendBlueGigaCommand(F("A2DP STREAMING START"));
    
    // Final fix for the "Kenwood Bug". Description of the Kenwood Bug:
    // The AVRCP NFY CHANGED commands work on factory car stereo in
    // Honda Accord 2017 model no matter what the transaction label is.
    // This led me to believe that the transaction label was only important
    // when performing an immediate response to a state query (i.e., when
    // sending an "AVRCP NFY INTERIM" response), and that if I was the one
    // who was indicating the state change (i.e., when I was sending an
    // "AVRCP NFY CHANGED" statement to tell the host that something had
    // changed), that the transaction label was arbitrary and that I was
    // the person choosing the transaction label at that time. This was
    // fine for the Honda stereo: When those commands are issued to the
    // bluetooth chip, then the car stereo head unit immediately reacts
    // by re-querying for more new track metadata from the bluetooth chip.
    // 
    // However, when using those commands on a Kenwood bluetooth-equipped
    // car stereo, then nothing happens when the messages are sent up
    // with arbitrary transaction labels. The Kenwood stops sending new
    // queries for track information and just "sits there" blindly
    // playing the audio stream without getting any new track data.
    // 
    // It turns out that the transaction labels aren't arbitrary if
    // you received a "REGISTER_NOTIFICATION" message for the
    // "PLAYBACK_STATUS_CHANGED" or the "TRACK_CHANGED" messages.
    // Yes, you need to immediately respond with an "INTERIM"
    // notification, but then if you send a "CHANGED" notification
    // yourself later, then the transaction label still has to match.
    // The Kenwood stopped querying for new track data because the
    // transaction labels didn't match when I sent it "CHANGED"
    // notifications. The fix is to match the transaction labels
    // by storing them in a global variable.
    // 
    // Here's the first of the fixes: Send a notification to the head unit that the 
    // "playback status changed". Command details for this command are:
    //      AVRCP NFY {INTERIM | CHANGED} {transaction_label} {event_ID} [value]
    // Where:
    //      AVRCP NFY   - The notification command to send to the head unit.
    //      CHANGED     - Indicate to the head unit that there was a change of some kind.
    //      1           - Send this message with the specific transaction label from the
    //                    corresponding previous "REGISTER_NOTIFICATION" if we have it, or
    //                    if we don't have it yet, then fall back to using an arbitrary number.
    //      1           - The notification message we will send contains event ID 1, which is "PLAYBACK_STATUS_CHANGED".
    //      1           - The sole parameter value for the PLAYBACK_STATUS_CHANGED, with a "1" indicating "playing"
    if (transactionLabelPlaybackStatusChanged == "")
    {
      // Experimental - If we haven't received a "REGISTER_NOTIFICATION" message to begin
      // with at startup yet, then don't attempt to send an arbitrary notification with an
      // arbitrary transaction label. I suspect that this arbitrary-transaction-labeled
      // message migh be going into a queue on the BlueGiga module and then being sent up
      // to the device later and confusing some devices. In particular I am wondering if it
      // is confusing a bluetooth headset I'm testing with. The experiment here is to comment
      // out this line and send nothing at all in these cases.
      //    SendBlueGigaCommand(F("AVRCP NFY CHANGED 1 1 1"));
    }
    else
    {
      SendBlueGigaCommand("AVRCP NFY CHANGED " + transactionLabelPlaybackStatusChanged + " 1 1");
    }

    // Also indicate to the host stereo that we have started playing by sending a play command.
    // Indicate to the host stereo that we have stopped playing by sending a pause command.
    // EXPERIMENT - Try not sending pause and play commands to the host stereo. It might not
    // be even listening to them at all and in fact this might confuse it.
    //     SendBlueGigaCommand(F("AVRCP PLAY"));
  }
  else
  {
    // Note: Do not send "A2DP STREAMING STOP" here or else you will
    // get a "cutoff" start of the song on the next unpause. Leave
    // the stream active even when playback is stopped to prevent
    // cutoffs. In fact, to fix a bug, we actually want to send a
    // streaming start here instead.
    SendBlueGigaCommand(F("A2DP STREAMING START"));

    
    // Send a notification to the head unit that the playback status
    // has changed (fix Kenwood bug). Command details are above,
    // with the last "2" in this command indicating "paused".
    if (transactionLabelPlaybackStatusChanged == "")
    {
      // Experimental - If we haven't received a "REGISTER_NOTIFICATION" message to begin
      // with at startup yet, then don't attempt to send an arbitrary notification with an
      // arbitrary transaction label. I suspect that this arbitrary-transaction-labeled
      // message migh be going into a queue on the BlueGiga module and then being sent up
      // to the device later and confusing some devices. In particular I am wondering if it
      // is confusing a bluetooth headset I'm testing with. The experiment here is to comment
      // out this line and send nothing at all in these cases.
      //     SendBlueGigaCommand(F("AVRCP NFY CHANGED 1 1 2"));
    }
    else
    {
      SendBlueGigaCommand("AVRCP NFY CHANGED " + transactionLabelPlaybackStatusChanged + " 1 2");
    }

    // Indicate to the host stereo that we have stopped playing by sending a pause command.
    // EXPERIMENT - Try not sending pause and play commands to the host stereo. It might not
    // be even listening to them at all and in fact this might confuse it.
    //      SendBlueGigaCommand("AVRCP PAUSE");
  }

  // In all cases, send a notification to the host stereo headunit that the track
  // itself has changed, so that it will re-query us for the track metadata.
  // Command details for this command are:
  //      AVRCP NFY {INTERIM | CHANGED} {transaction_label} {event_ID} [value]
  // Where:
  //      AVRCP NFY   - The notification command to send to the head unit.
  //      CHANGED     - Indicate to the head unit that there was a change of some kind.
  //      2           - Send this message with the specific transaction label from the
  //                    corresponding previous "REGISTER_NOTIFICATION" if we have it, or
  //                    if we don't have it yet, then fall back to using an arbitrary number.
  //      2           - The notification message we will send contains event ID 2, which is "TRACK_CHANGED".
  //      1           - The sole parameter value for the TRACK_CHANGED, with a "1" indicating that there is indeed a track selected.
  if (transactionLabelTrackChanged == "")
  {
      // Experimental - If we haven't received a "REGISTER_NOTIFICATION" message to begin
      // with at startup yet, then don't attempt to send an arbitrary notification with an
      // arbitrary transaction label. I suspect that this arbitrary-transaction-labeled
      // message migh be going into a queue on the BlueGiga module and then being sent up
      // to the device later and confusing some devices. In particular I am wondering if it
      // is confusing a bluetooth headset I'm testing with. The experiment here is to comment
      // out this line and send nothing at all in these cases.
      //     SendBlueGigaCommand(F("AVRCP NFY CHANGED 2 2 1")); 
  }
  else
  {
    SendBlueGigaCommand("AVRCP NFY CHANGED " + transactionLabelTrackChanged + " 2 1"); 
  }
  
  // Report to our arduino console debug log what the current state of playback on the Empeg is.
  ReportEmpegPlayingState();
}

// ----------------------------------------------------------------------------
// ReportEmpegPlayingState
//
// Log to the console session the current believed state of the empeg
// whether it is playing or paused. Used only for logging, there is 
// a different place in the code that responds to bluetooth queries
// asking for the playback state.
// ----------------------------------------------------------------------------
void ReportEmpegPlayingState()
{
  if (empegIsPlaying)
  {
    Log(F("Empeg state...................................PLAYING  >"));
  }
  else
  {
    Log(F("Empeg state..................................PAUSED   ||"));
  }
}

// ----------------------------------------------------------------------------
// QuickResetBluetooth
//
// Quickly reset the bluetooth module and also clean out any global variables
// which we should not be trying to save any more thanks to the module getting
// reset. This is an attempt to avoid a bug where desynchronization between 
// the bluetooth initialization state and the Arduino initialization state
// make things go a little wonky in the software. When issuing bluetooth
// commands to reset the player, do not simply call "RESET" or "BOOT 0" by
// itself. Should call this routine instead.
// 
// Parameter:
// resetType       0 = Use the "RESET" command
//                 1 = Use the "BOOT 0" command
//                 2 = Use the "SET RESET" command (factory defaults)
//
// Note that "SET RESET" (factory defaults) is different from "RESET" (which
// is just a reboot). Note that this does not erase bluetooth pairings.
// Also, I don't know if "RESET" is any different from "BOOT 0" for my
// purposes, but it's being included here just in case.
// ----------------------------------------------------------------------------
void QuickResetBluetooth(int resetType)
{
  switch (resetType)
  {
    case 0:
      SendBlueGigaCommand(F("RESET"));
      ClearGlobalVariables();
      break;
    case 1:
      SendBlueGigaCommand(F("BOOT 0"));
      ClearGlobalVariables();
      break;
    case 2:
      SendBlueGigaCommand(F("SET RESET"));
      ClearGlobalVariables();
      break;
    default:
      return;
  }
}

// ----------------------------------------------------------------------------
// ClearGlobalVariables
//
// Clear out any global variables which we should not be remembering or 
// re-using if the bluetooth chip happened to have gotten reset, either
// accidentally or deliberately.
// ----------------------------------------------------------------------------
void ClearGlobalVariables()
{
  transactionLabelPlaybackStatusChanged = "";
  transactionLabelTrackChanged = "";
  pairAddressString = "";
}

