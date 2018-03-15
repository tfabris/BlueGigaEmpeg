
// --------------------------------------------------------------------------
// BlueGigaEmpeg
// by Tony Fabris
// --------------------------------------------------------------------------

// A project to use a Silicon Labs BlueGiga WT32i Bluetooth chip, combined
// with an Arduino Mega board with an added RS232 port, to act as an
// intermediary between the empeg Car stereo and a modern car.
//
// Please refer to the accompanying README file for details about the
// electronics interface, and how to modify and configure your empeg Car to
// work with this code.


// ------------------------------------
// Global variables and definitions
// ------------------------------------

// String to control the type of Bluetooth authentication that you want to
// use.
//
// Uncomment this line for "just works" pairing with no PIN code, with a
// fallback to the PIN code if the "just works" mode fails. Use this one if
// you are having trouble with the default security scheme below.
//
//        const String btAuthTypeString = "SET BT SSP 3 0";
//
// Default security scheme for "yes/no prompt" security type, required for
// Mark Lord's car stereo. The BlueGigaEmpeg module (this code) will respond
// with a "yes" automatically. You will see your stereo prompt with a
// confirmation number on its touch screen and then it will magically get
// answered automatically and the prompt will go away and you'll be paired.
const String btAuthTypeString = "SET BT SSP 1 0";

// If the device falls back to using a PIN code, here is where to set the PIN.
// Change "0000" to your correct PIN code for your stereo. It can accept PIN
// codes up to 16 digits long.
const String btPinCodeString = "SET BT AUTH * 0000";

// Control whether or not the module uses digital I2S audio, or analog line-
// level audio inputs (for example the line level inputs on the BlueGiga dev
// board).
boolean digitalAudio = true;

// Choose whether or not to display the empeg Serial Port outputs (for
// instance the empeg boot up messages) on the serial debug console of the
// Arduino. Only needed for debugging, not needed for final build runtime.
//   Setting true:
//   - Characters received from the empeg Car serial cable are displayed
//     on the Arduino main serial port debugging console terminal screen.
//     It is mixed in with all the other serial output from the Bluetooth
//     chip all at the same time, so it might be messy if the Bluetooth
//     chip and the empeg are saying things at the same time.
//   Setting false:
//   - No echoing of the empeg Car serial cable output occurs on the debug
//     screen (though the empeg output is still interpreted and acted upon
//     by the software, it's just not displayed on the debug screen).
// This should be set to false most of the time, and only set to true during
// debugging sessions, since it slows down processing to put out too much data
// on the serial port when it is not needed.
boolean displayEmpegSerial=false;

// Variable to control whether output from the empeg and Bluetooth serial
// ports are logged to the debug console character-by-character or line-by-
// line. Here's the trade-off: If you log character-by-character you have a
// clearer picture of exactly when various messages were received from each of
// those devices in relation to each other, but the debug console gets hard to
// read. For instance, if logging character-by-character, the output will look
// like:
//     Bluetooth sends "GOODBYE" 
//     Empeg sends "hello" a microsecond later
//     Debug console output looks like this:
//        GhOeOlDlBoYE
// However if you log line-by-line, it's more readable but you might get the
// wrong idea of who sent what first. It might look like this:
//     hello
//     GOODBYE
// Even though GOODBYE was the first one to start sending characters, he ended
// his transmission last so he gets logged last. This usually isn't a big
// deal. Usage:
//    Setting true:
//      - Output from the empeg and the Bluetooth are logged a line
//        at a time, for additional readability, but order may be wrong.
//    Setting false:
//      - Output from the empeg and the Bluetooth are logged a character
//        at a time, which is less readable but slightly better timing.
// Special note: This isn't fully implemented for every single possible line
// of output on the debug port. There are still a couple places in the code
// where it still logs character by character. If this becomes a problem then
// I will address it at that time later. In the meantime, this handles most of
// the situations where we'd want to see it log line by line.
boolean logLineByLine=true;

// Choose whether or not to display millisecond deltas with each log line.
//      Setting true:
//           - Each logged line of output prints a delta, in milliseconds,
//             which is the difference from the last logged line of output,
//             for profiling.
//           - Logged output is not accompanied by a milliseconds number.
// Note: Recommend turning on this feature only when "logLineByLine" is true.
boolean outputMillis=true;

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


// Configure the mode that the BlueGigaEmpeg uses to reconnect to the host
// stereo. Chooses the main reconnection string that commands the Bluetooth
// module how to behave at power-on, in terms of how it attempts to reconnect
// to its previously-connected pairing buddy (if any).
//      Setting 0:
//           - Disable reconnection. A blank string is sent to the Bluetooth
//             chip instead of sending a command to automatically reconnect.
//             This prevents the BlueGigaEmpeg from automatically connecting
//             again after having been previously paired. It will be up to the
//             host stereo to initiate automatic reconnection in this case.
//             Note: If you want to use this non-default setting, you must do
//             the following: You must upload the modified code to the
//             BlueGigaEmpeg, then with the BlueGigaEmpeg running, you must
//             reset and re-pair the BlueGigaEmpeg unit by pressing the
//             RESET/PAIR button.
//      Setting 1:
//           - Default setting. The Bluetooth chip is commanded to attempt
//             automatic reconnection to the previously-paired host stereo at
//             any time that it is disconnected, including at power-on. It
//             will continuously attempt reconnection in all cases at all
//             times when it is not connected, if there is any previous
//             pairing buddy in its memory.
//      Setting 2:
//           - Alternate setting. Similar to Setting 1 above, but uses a
//             slightly different syntax for the reconnect command. This is an
//             attempt to work around a bug where AVRCP commands didn't always
//             work on certain devices. See GitHub issue #71 "Onkyo AVRCP
//             commands do not work." for more details. Enable this alternate
//             mode only if it fixes that specific bug with your stereo, since
//             it may cause adverse affects when used with other stereos.
//             Note: If you want to use this non-default setting, you must do
//             the following: You must upload the modified code to the
//             BlueGigaEmpeg, then with the BlueGigaEmpeg running, you must
//             reset and re-pair the BlueGigaEmpeg unit by pressing the
//             RESET/PAIR button.
//
// This setting should be left at the default value of "1" in most cases.
const int autoReconnectMode = 1;



// Auto Reconnect - This next section is a big deal, it was the largest source
// of bugs and issues on the WT32i chip over the long run, due to unclear
// documentation. 
// 
// String to tell the unit to automatically try reconnecting every few seconds
// when and if it ever becomes disconnected from its main pairing buddy.
// Trying to make it grab hold of the car stereo as soon as the car turns on,
// instead of connecting to my cellphone every time. Tried multiple possible
// versions of this, only one of which produced decent results. Even that one
// was very difficult to get working correctly.
//
// Version 1: "SET CONTROL RECONNECT". (Only one that worked decently.)
//
// Descriptions of parameters which come after the "RECONNECT" parameter:
//
//   First parameter is retry interval in milliseconds. The docs say that the
//   reconnect timer should be longer than 500ms.
//
//   Second and third parameters are max attempts and total timeout, which are
//   set to zero to indicate infinite retry.
//
//   Fourth parameter is a 16 bit bitmask. See section 6.89 in the
//   "iWRAP 6.1.1 USER GUIDE AND API REFERENCE" document for details. It must
//   be typed in as a hexadecimal number. The bitmask values are:
//
//      Bit 0 (value 1): Automatically store connection order after each
//      established connection. If this bit is set to 0 then user must
//      manually store device order before power off device by command:
//      STORECONFIG
//
//      Bit 1 (value 2): Start again reconnection after correctly
//      disconnected device.
//
//      Bit 2 (value 4): Automatically make defragmentation and reset
//      module when there is no free memory for storing connection history
//      order.
//
//      Bit 3 (value 8): Automatically make reset module when link loss
//      connection appear.
//
//      Bit 4 (value 16): Reconnect to all devices. When currently BT
//      connection is disconnected by the user and Bit 1 is set then last
//      connected device will be taken into account during reconnection
//      procedure as a last device in queue. When this bit is 0 and Bit 1
//      is set then last connected device will be out of reconnection list
//      until next reset or successful connection appear.
//
//      Bit 5 (value 32): If during reconnection link key from remote device
//      is missing then it will be removed automatically from paired device
//      list.
//
//      Remaining bits of the fourth parameter must be zero.
//
//   Fifth and sixth parameters are custom target and custom profile. It should
//   be "19 A2DP" to set a custom A2DP profile, and "0 NONE" or perhaps "0 NONE
//   A2DP AVRCP" if you don't want to use a custom profile at all and you're
//   trying to specify the A2DP and AVRCP profiles directly without a custom
//   profile. "19" is the secret code, the "L2CAP PSM" code, for "A2DP", and 17
//   is the secret code for "AVRCP".
//
//   Seventh parameter and on (if used), are the list of profiles and the order
//   in which profiles are tried to connect. For instance if your list was HFP
//   A2DP AVRCP then it would try to connect to those profiles in that order
//   (ignoring whatever was in the custom profile right before it). If your list
//   is CUSTOM A2DP AVRCP then it would try the custom profile first, then A2DP
//   then AVRCP. It's important to remember that you must put CUSTOM in the list
//   to use the custom profile. You may omit the list entirely (ending the
//   command at the custom profile) and in that case the custom profile is
//   assumed to be used since it is the only specified profile. This is very
//   confusing and hard to glean from the docs. This caused many bugs as I tried
//   different combinations. Here are the bad ones I tried due to
//   misunderstanding the bad docs, and each of their bad results:
//
//    const String autoReconnectString = "SET CONTROL RECONNECT 800 0 0 7 0 A2DP A2DP AVRCP\r\n            STORECONFIG";
//    const String autoReconnectString = "SET CONTROL RECONNECT 4800 0 0 7 0 A2DP A2DP AVRCP\r\n            STORECONFIG";
//    - Incorrect parameters. "0 A2DP" is non-sequitur, should be "0 NONE", but
//      that doesn't matter since it was ignored. Instead it tried to connect
//      A2DP AVRCP in that order.
//    - This caused issue #60, bad PDU registrations from the Honda stereo.
//    - Reconnect interval 800 is too short, exacerbating other problems. The
//      shorter the reconnect interval, the more often issue #60 reproduced.
//    - STORECONFIG isn't needed. The "7" bitmask means we don't need it.
//
//   const String autoReconnectString = "SET CONTROL RECONNECT 2888 0 0 1f 0 NONE A2DP AVRCP";
//   const String autoReconnectString = "SET CONTROL RECONNECT 888 0 0 1f 0 NONE AVRCP A2DP";
//    - Parameters are syntactically correct.
//    - Updated bitmask in fourth parameter to better behavior (1F). However
//      it contains a mistake: The bit for "Automatically reset module when
//      link loss connection" was set, which caused reboots sometimes.
//    - This caused frequent reboots of the unit as it tried to connect to,
//      the Honda, making things worse.
//    - This fixed issue #71 on the Onkyo (no AVRCP on Onkyo) but caused
//      Honda reboots during reconnection and let the iPhone in the door
//      sometimes.
//    - Speedier reconnect speed attempts (888) simply exacerbated reboots.
//
//   const String autoReconnectString = "SET CONTROL RECONNECT 2888 0 0 1f 19 A2DP AVRCP";
//   const String autoReconnectString = "SET CONTROL RECONNECT 2888 0 0 1f 17 AVRCP A2DP";
//    - Incorrect parameters. The lack of the word "CUSTOM" in the seventh
//      parameter position means that custom profile never is used. So only
//      one of the two ever got used. Either it didn't fix Onkyo issue #71,
//      or it didn't play any sound.
//    - 1f in fourth parameter still has the "reset module on link loss" bug.
// 
//   const String autoReconnectString = "SET CONTROL RECONNECT 2888 0 0 1f 19 A2DP CUSTOM AVRCP";
//   const String autoReconnectString = "SET CONTROL RECONNECT 2888 0 0 1f 17 AVRCP CUSTOM A2DP";
//    - Parameters are syntactically correct.
//    - Even though custom profile is used in addition to the other profile,
//      the first one re-induced issue #60 Bad PDU registrations, and the
//      second one induced a large number of IWRAP reboots due to the "1f".
//   
//   const String autoReconnectString = "SET CONTROL RECONNECT 2888 0 0 1f 19 A2DP";
//    - Parameters are syntactically correct. A single A2DP profile is used.
//    - Works perfectly on Honda and several other devices.
//    - Causes issue #71, no AVRCP on Onkyo because the Onkyo does not
//      automatically initiate an AVRCP connection after we established an
//      A2DP connection. All other devices do, but not the Onkyo.
//    
//   const String autoReconnectString = "";
//    - Blank string works to turn off reconnect. Set it to this blank string
//      if you want to turn off the BlueGiga iWrap6 reconnect feature and
//      depend upon your stereo headunit to reconnect. This is now a
//      configurable option at the top of the code, see "autoReconnectMode" at
//      the top of the code for details.
//
//   const String autoReconnectString = "SET CONTROL RECONNECT 2888 0 0 17 0 NONE A2DP AVRCP";
//    - Parameters are syntactically correct.
//    - 17 instead of 1f means no weird IWRAP reboots.
//    - No custom profile used.
//    - A2DP and AVRCP are tried in that order.
//    - Fixes issue #71 on Onkyo.
//    - Re-induces issue #60 all over again just like the earlier
//      syntactically-incorrect one.
// 
//   const String autoReconnectString = "SET CONTROL RECONNECT 2888 0 0 17 0 NONE AVRCP A2DP";
//    - Parameters are syntactically correct.
//    - 17 instead of 1f means I expect no weird IWRAP reboots, but got them
//      anyway.
//    - No custom profile used.
//    - Backwards order of AVRCP and A2DP connection, trying AVRCP first in
//      an attempt to avoid issue #60.
//    - Successfully avoids issue #60 but there are still random IWRAP
//      reboots, so this is a no-go.
//    - Also causes a particularly bad problem where the head unit constantly
//      queries for new track info in a continuous loop. Very bad.
//
//   const String autoReconnectString = "SET CONTROL RECONNECT 2888 0 0 17 19 A2DP";
//    - Parameters are syntactically correct.
//    - 17 instead of 1f means no weird IWRAP reboots.  
//    - "19 A2DP" without specifying "CUSTOM" is the same as if I had used
//      "19 A2DP CUSTOM", i.e., it ends up defaulting to "CUSTOM" anyway.
//    - Works perfectly on Honda and several other devices.
//    - Causes issue #71, no AVRCP on Onkyo because the Onkyo does not
//      automatically initiate an AVRCP connection after we established an
//      A2DP connection. All other devices do, but not the Onkyo.
//
// Currently used, default, auto reconnect string:
//    - Parameters are syntactically correct.
//    - 17 instead of 1f means no weird IWRAP reboots.  
//    - Attempt to see if the root of many problems was the custom profile
//      entirely. In this case, do not use any custom profile, instead
//      specify A2DP only, and specify it directly without a custom profile.
//    - Conceptually "0 NONE A2DP" should be the same as "19 A2DP" or
//      "19 A2DP CUSTOM", but this command has been super weird in other ways,
//      so work with this for a while and see if it fixes other problems.
//    - Works perfectly on Honda, still need to test on other devices.
//    - To Do: Test to see if it magically fixes Issue #71 on Onkyo. I am not
//      optimistic, but it's worth a try.
const String autoReconnectString = "SET CONTROL RECONNECT 2888 0 0 17 0 NONE A2DP";

// Version 1.5: Alternate version of autoReconnectString to fix issue #71.
//
// This is an attempted work around to issue #71 "No AVRCP on Onkyo". This is
// not a solution since this cannot work universally to all devices, it causes
// problems on other devices (induces issue #60 "Bad PDU Registrations" on my
// Honda). However a user can enable this with the flag variable
// "autoReconnectMode" at the top of the code if they encounter a problem.
//    - Parameters are syntactically correct.
//    - 17 instead of 1f means no weird IWRAP reboots.
//    - No custom profile used.
//    - A2DP and AVRCP are tried in that order.
//    - Fixes issue #71 on Onkyo.
//    - Re-induces issue #60 all over again just like the earlier
//      syntactically-incorrect one.
const String alternateAutoReconnectString = "SET CONTROL RECONNECT 2888 0 0 17 0 NONE A2DP AVRCP";


// Version 2: "SET CONTROL AUTOCALL".
//
// Abandoned version of reconnect feature. This seems to work well, when it
// works. But some of its other issues are not livable. For example sometimes
// it does a good job of reconnecting after power on, but then other times it
// completely fails to even attempt any kind of reconnection whatsoever after
// power on. Without rhyme or reason.  Note: "19" is the secret code for
// "A2DP" (it is the "L2CAP PSM" code for A2DP).
//     const String autoReconnectString = "SET CONTROL AUTOCALL 19 501 A2DP";
//
// Version 3: Monkey reconnect.
//
// Abandoned version of reconnect feature. A reconnect by my monkey code
// instead of the built-in reconnect. This is my own personal reconnect
// feature which reconnects just by looping in my code. This was a failed
// attempt to fix issue #60. Keeping the code here, disabled, for posterity,
// but do not enable this feature, it causes more problems than it solves.
const unsigned long monkeyReconnectInterval = 3600L;
boolean monkeyReconnectEnabled = false;

// Strings to define which codecs to use for A2DP audio transmission. To
// remove a codec from the capability list, use its command with no
// parameters. For example:
//       SET CONTROL CODEC AAC
// Will remove the AAC codec from the capability list. This is NOT in the
// documentation, so I had to figure that out from trial and error.
// 
// Uncomment this string to use default SBC codec.
const String codecString="SET CONTROL CODEC SBC JOINT_STEREO 44100 0\r\n            SET CONTROL CODEC APT-X_LL\r\n            SET CONTROL CODEC APT-X\r\n            SET CONTROL CODEC AAC";
//
// Uncomment this string to use Apple AAC codec and fall back to the default
// SBC codec if the AAC codec is not available. I believe this requires the
// purchase and installation of a special license. This is probably not worth
// it because the chip's command documentation says that it is unsupported on
// most devices, and you'll only see it functioning on Apple-made devices.
// In particular, it doesn't work on my target system, my Honda car stereo,
// so I won't be attempting to support this.
//   const String codecString="SET CONTROL CODEC AAC JOINT_STEREO 44100 0\r\n            SET CONTROL CODEC SBC JOINT_STEREO 44100 1\r\n            SET CONTROL CODEC APT-X_LL\r\n            SET CONTROL CODEC APT-X";
//
// Uncomment this string to use APT-X Low Latency codec. Requires purchase of
// and installation of a special license for APT-X codec from Silicon Labs.
// The license installation process in onerous and requires the purchase of
// a special chip programming tool called a CSR USB-SPI programmer. Even if
// you have the tool, the steps to accomplish the job are very tricky and the
// Silicon Labs tech support does not give you all the important details that
// you need to know to accomplish the job. I was able to do it once, with a
// lot of trial and error, but honestly I don't think it's worth it to do it.
//   const String codecString="SET CONTROL CODEC APT-X_LL JOINT_STEREO 44100 0\r\n            SET CONTROL CODEC APT-X JOINT_STEREO 44100 1\r\n            SET CONTROL CODEC SBC JOINT_STEREO 44100 2\r\n            SET CONTROL CODEC AAC";
//
// How to tell which codec is being used: When initial connection to your host
// stereo occurs, a message will appear on the serial port like one of the
// ones below, which tells you the details of what codec is used:
//     A2DP CODEC SBC JOINT_STEREO 44100 BITPOOL 2-53
//     A2DP CODEC SBC JOINT_STEREO 48000 BITPOOL 2-53
//     A2DP CODEC APT-X_LL STEREO 44100

// Variable to control whether or not this program performs a conversion of
// High ASCII to UTF-8 in the code. For instance, on the empeg, you might have
// a track by "Blue Öyster Cult", with the "Ö" being represented by a High
// ASCII character (a single ASCII byte with a value greater than 127). In
// that situation, you might see the the car stereo's LCD touchscreen display
// say "Blue ?yster Cult". With this variable set to "true" it will instead
// convert that to UTF-8, which should be readable by your car stereo touch
// screen. If you encounter problems with some track titles which might
// already be encoded as UTF-8 on your empeg, then try setting this value to
// "false"
// and see if that fixes it.
//   Setting true:
//      - Character values between ASCII 128 and ASCII 255 in the track metadata
//        on the empeg will be converted to their UTF-8 equivalents before being
//        sent up the Bluetooth chain to the car stereo.
//   Setting false:
//      - No UTF-8 conversion will be performed and the track metadata is sent
//        from the empeg to the car stereo without any changes.
boolean PerformUtf8Conversion = true;

// Arduino serial port 2 is connected to Bluetooth chip.
#define BlueGigaSerial Serial2 

// Arduino serial port 1 is connected to empeg's RS-232 port via MAX232.
// circuit.
#define EmpegSerial Serial1

// Special Case Fix Matrix (scFixMatrix).
// Matrix of special case fix strings and their Bluetooth responses. When a
// Bluetooth statement is received from the Bluetooth module, this code will
// respond back to the Bluetooth module with the specified text. These are
// "plain" response strings without any detailed parsing, they are just
// input->output pairs for answering certain pieces of text on the Bluetooth
// module's serial port. There is other code elsewhere to handle input/output
// that requires more detailed handling and parsing.
// NOTE: Update the matrix size and the array size both, if you are changing
// these.
int scFixMatrixSize = 12;
String scFixMessageMatrix[12][2] =
{
    // Bluetooth in                        // Bluetooth out        

  // Respond to PIN code prompt as if we were a user saying OK to that PIN
  // code. This was needed on Mark Lord's car stereo. It is expected to be
  // needed when the pairing security scheme is set for "SET BT SSP 1 0" at
  // the top of this program. NOTE: This uses the pair address substitution
  // string in the same way that the Pairing Mode strings use it. See the
  // pmMessageMatrix for details.
  { "SSP CONFIRM ",                                 "SSP CONFIRM {0} OK"},

  // We issue a "Streaming start" command in certain situations. However,
  // don't do the corresponding thing for "streaming stop", because it causes
  // a problem where, when you unpause a song, there is a slight delay while
  // streaming starts up again, so, if you unpause at the 00:00 mark of a
  // song, then the beginning of the song is cut off. I am leaving this line
  // in here, commented out, to remind myself not to ever try to do this.
  //        { "AVRCP PAUSE PRESS",                  "A2DP STREAMING STOP"},

  // Get Capabilities 2 is asking for manufacturer ID. According to the
  // BlueGiga documentation the chip will fill in the response details for
  // manufacturer ID automatically if you give it a blank response. (By "blank
  // response" I mean, responding with "AVRCP RSP" with no further parameters
  // following after the "RSP"). This is in the docs in the file "AN986.PDF"
  // titled "AN986: BLUETOOTH® A2DP AND AVRCP PROFILES". The details are found
  // in section 6.4.1 which states:
  //
  //       GET_CAPABILITIES 2   The Controller is querying for supported
  //                            Company IDs
  //
  //                AVRCP RSP  [id0 id1 …]
  //                           List of IrDA Company IDs. Usually, the list
  //                           can be left empty, as iWRAP will automatically
  //                           fill in the Bluetooth SIG
  //
  // Despite this, I'm attempting to respond with the correct ID of Silicon
  // Labs which is 02FF or decimal 767. This neither hurts nor helps it seems.
  { "GET_CAPABILITIES 2",                 "AVRCP RSP 02FF"},

  // Get Capabilities 3 is asking for which status notifications my Bluetooth
  // chip wants to receive. In this case track/status changes (codes 1 and 2,
  // those codes were obtained from the BlueGiga documentation).
  { "GET_CAPABILITIES 3",                 "AVRCP RSP 1 2"},

  // "LIST_APPLICATION_SETTING_ATTRIBUTES" is how the host stereo queries the
  // Bluetooth module to find out if your device supports things like turning
  // on the EQ, turning shuffle on and off, and turning repeat on and off. This
  // is the generalized query to find out what attributes are supported by your
  // playback device at all, it is not asking for individual setting values.
  //
  // Responding with "3" meaning "Shuffle" so that my empeg player can respond
  // to commands from the car stereo head unit to turn shuffle on and off.
  { "LIST_APPLICATION_SETTING_ATTRIBUTES",  "AVRCP RSP 3"},

  // "LIST_APPLICATION_SETTING_VALUES" is the way that the host stereo queries
  // your Bluetooth module for the range of supported individual setting values
  // for a given settings group. Not generally whether a particular settings
  // group is supported at all, but rather, looking for the list of possible
  // setting values of of this particular thing it's querying. In this case,
  // the host stereo is not expected to ask for anything but the shuffle value,
  // since in the original query above, we have responded only with saying that
  // shuffle ("3") is available. So we respond with "1 2" indicating that the
  // setting for "shuffle" can have two possible values: 1 for "off" and 2 for
  // "all tracks". Note: Cannot add "Repeat" in here because the empeg car serial
  // command set does not contain a command for "Repeat".
  { "LIST_APPLICATION_SETTING_VALUES",      "AVRCP RSP 1 2"},

  // "GET_APPLICATION_SETTING_VALUE" is the way that the host stereo queries your
  // Bluetooth module for the current state of the setting. For instance, it is
  // asking whether Shuffle is currently on or off. The host stereo is not expected
  // to ask for anything but the shuffle value, since in the original query above,
  // we have responded only with saying that shuffle ("3") is available. So we
  // blindly respond with "1" indicating that shuffle is off. This is tricky, since
  // the empeg does not tell us the shuffle state on its serial port (it merely
  // toggles its shuffle state when it receives a "%" command but says nothing in
  // response) then we have no way of responding back to the head unit accurately
  // as to whether or not shuffle is on or off. We merely can listen for the query
  // and try to respond with "something" to prevent the host headunit from hanging
  // at this point. It turns out that this blind response works well on the one
  // host headunit that I've got to test it against (Vixy and Fishy's Kenwood).
  // Pressing the  shuffle button on their stereo works fine and toggles shuffle.
  // Note: Cannot add "Repeat" in here because the empeg car serial command set
  // does not contain a command for "Repeat".
  { "GET_APPLICATION_SETTING_VALUE",      "AVRCP RSP 1"},

  // Fix issue #45/60 "Initial boot and connect problem on Honda" by carefully
  // selecting in which situations we issue a "streaming start" command. Note:
  // do not issue it on the "RING 0" message, only do it for higher numbers.
  // Otherwise you're trying to start streaming when its still in the process
  // of setting up the second A2DP channel and/or the AVRCP channel, and that
  // sometimes confuses the head unit. Also note: You might think you don't
  // need these at all (the host and client should negotiate this by
  // themselves) but it doesn't work to remove these. If you don't do this
  // then in some cases your empeg plays silently to a headunit that makes
  // no sound. So these are required.
  { "RING 1",                              "A2DP STREAMING START"},
  { "RING 2",                              "A2DP STREAMING START"},
  { "RING 3",                              "A2DP STREAMING START"},

  // While attempting to fix issue #71 (No Onkyo AVRCP), while also trying to
  // prevent re-inducing issue #60 (Host sends bad PDU registrations forcing)
  // me to reboot), I re-encountered issue #67 (silence after cold boot) and
  // I think it's because I needed to speed up the reconnect interval to make
  // it work. I attempted to put STREAMING START commands back in here at
  // connect time. This did not fully fix issue #67 but significantly reduced
  // its frequency. I later got another repeat of issue #67 which seems to
  // indicate that there might be some other spot later on, even later than
  // these, where I might be able to issue another STREAMING START which might
  // fix issue #67 for good, not sure.
  { "CONNECT 1 A2DP 19",                   "A2DP STREAMING START"},
  { "CONNECT 2 A2DP 19",                   "A2DP STREAMING START"},
  { "CONNECT 3 A2DP 19",                   "A2DP STREAMING START"},
};

// Variable to control whether or not we reconnect the Bluetooth module
// whenever we see a bad PDU_REGISTER_NOTIFICATION message. Ideally we want to
// reject any attempts to register any notifications that we won't be able to
// respond to. The host stereo should never ask us for anything other than
// messages 1 and 2 (TRACK_CHANGED and PLAYBACK_STATUS_CHANGED) but my Honda
// sometimes asks for messages 3 and above when it shouldn't be. In those
// cases, a disco/reco of the Bluetooth module fixes the problem. This flag
// controls whether or not that disco/reco occurs. It's here to protect
// against a possible infinite reboot loop which might occur on someone else's
// stereo. Though if this trigger is hit at all, it means they've got bigger
// problems where the host stereo will hang anyway because it isn't getting
// responses to its registration requests. But that's a different issue than a
// reboot loop.
boolean reconnectIfBadRegistrationReceived=true;

// Variable to control whether or not we reboot the Bluetooth module 
// whenever we see a bad connection at the wrong sample rate. This works
// around issue #70, the case where sometimes the stereo would connect to
// the BlueGigaEmpeg with a bad sample rate of 48000, causing crackling
// high pitched playback audio. This runs a risk of an infinite reboot
// loop, so it's being controlled here with a boolean flag that we can 
// disable if necessary.
boolean workAroundChipmunkProblem=true;

// Reserve variable space for the transaction status labels used to hold a
// particular string in memory in the query/response code. The host stereo
// sends a query which contains a specific transaction label, which is a
// number that is usually just a single hexadecimal digit, and it uses this
// digit as an identification so that it can send several queries and await
// several responses and can sort out which response is from which query. So
// we must respond with that same transaction label digit in our response.
// Must have specific transaction labels to use for specific types of
// messages. These will be using the memory reservation size of "2" since they
// are copies of a single-character transaction label string.
int transactionLabelSize = 2;
static String transactionLabelPlaybackStatusChanged = "";
static String transactionLabelTrackChanged = "";

// Reserve variable space for which Bluetooth channel the AVRCP connection is
// currently on, for example, the LIST command might look like:
//
// list
// LIST 3
// LIST 0 CONNECTED A2DP 672 0 0 374 0 0 ...
// LIST 1 CONNECTED A2DP 672 0 0 374 0 0 ...
// LIST 2 CONNECTED AVRCP 672 0 0 1 0 0  ...
//
// The numbers after the word LIST are the channel that each of those things
// is set to run upon. Mostly I don't care or need to know those but there is
// one situation where it is useful: If I am bouncing just the AVRCP channel
// to fix a bug, I need to know which one it's on.
static String avrcpChannel = "";

// Pair Mode Matrix (pmMatrix).
// Matrix of messages and responses which are needed when in special pairing
// mode. These also include some special casing for the Bluetooth device
// address. The "{0}" in these strings are a special cased token indicating
// that we should insert the Bluetooth device address into that location of
// the response string. The "{0}" is also defined in another variable below.
// NOTE: Update the matrix size and the array size both, if you are changing
// these.
int pmMatrixSize=5;
String pmMessageMatrix[5][2] =
{
  // Respond to messages such as INQUIRY_PARTIAL 0a:ea:ea:6a:7a:6a 240404
  { "INQUIRY_PARTIAL ",          "PAIR {0}"},

  // Respond to messages such as "PAIR 0a:ea:ea:6a:7a:6a OK". Respond during
  // the pair process with a connection attempt. Response should be "CALL",
  // the address to connect to, then a special code indicating the target type
  // and profile type. "19" is a special code for a certain kind of target
  // which is a particular secret-code hard-to find value called an "L2CAP
  // psm" and the special secret L2CAP psm for A2DP is "19".
  { " OK",                       "CALL {0} 19 A2DP"},  
  
  // Respond to messages such as "CONNECT 0 A2DP 19" Respond during the pair
  // process with a connection attempt. Response should be "CALL", the address
  // to connect to, then a special code indicating the target type and profile
  // type. "17" is a special code for a certain kind of target which is a
  // particular secret-code hard-to find value called an "L2CAP psm" and the
  // special secret L2CAP psm for AVRCP is "17".
  //
  // NOTE BUGFIX: Don't do an AVRCP call while the host is in the middle of
  // pairing its second A2DP channel. In other words, don't do an AVRCP call
  // on the first ("0") connect from the host stereo, only do it on the second
  // ("1") one and later. So comment out this line:
  //       { "CONNECT 0 A2DP 19",        "CALL {0} 17 AVRCP"},
  // But subsequent ones are OK to do, and in fact are required for AVRCP to
  // work on the paired system.
  { "CONNECT 1 A2DP 19",        "CALL {0} 17 AVRCP"},
  { "CONNECT 2 A2DP 19",        "CALL {0} 17 AVRCP"},
  { "CONNECT 3 A2DP 19",        "CALL {0} 17 AVRCP"},
};

// String to be used in token substitutions above (change both the matrix
// above and also the string below if you change the tokenization flag
// string).
const String tokenSubstitutionString = "{0}";

// Get Bluetooth Address Matrix (gbaMatrix).
// Gets Bluetooth address of pairing buddy. Set of strings which are the
// trigger phrases to be used for identifying strings from the Bluetooth module
// that can be used for getting the address of our main pairing buddy at any
// generic time as opposed to just during the pairing process. This allows us
// to figure out who our pairing buddy is even when they are the ones
// initiating the connection as opposed to us initiating it during the pairing
// process. This complexity is needed because ForceQuickReconnect needs to
// know who our current pairing buddy is all the time as opposed to just when
// we recently pressed the pair button.
int gbaMatrixSize=2;
String gbaMessageMatrix[2] =
{
  "SET BT PAIR ",
  "SSP CONFIRM ",
};

// String that I am using to detect that the pairing process has completed and
// to stop responding to pairing-related messages coming on the serial port
// from the Bluetooth chip. And to turn off the blue pair/reset LED.
const String pairDetectionString = "AUDIO ROUTE ";

// Global string which will be used for storing special values from parsed
// responses (most likely Bluetooth device addresses from the pmMessageMatrix
// above) which will then be saved in this global variable and then re-used
// elsewhere, for example, receiving the inquiry Bluetooth address and then
// sending the Pair command back with the Bluetooth address in the response.
// The address will be saved in this string.
static String pairAddressString = "";
int pairAddressStringMaxLength = 25;

// Empeg Command Matrix.
// The translation table of AVRCP messages to empeg serial commands. This is
// the commands that we must send to the empeg serial port if certain messages
// come in from the Bluetooth module.
// NOTE: Update the matrix size and the array size both, if you are changing
// these.
int empegCommandMatrixSize = 16;
String empegCommandMessageMatrix[16][2] =
{
  // Bluetooth Module reports     // Send command to empeg
  { "AVRCP PLAY PRESS",           "C"},
  { "AVRCP PAUSE PRESS",          "W"},
  { "AVRCP STOP PRESS",           "W"},
  { "WRAP THOR AI",               "W"},  // Part of bugfix for issue #26 - Pause automatically any time the chip resets itself unexpectedly.
  { "A2DP STREAMING START",       "C"},  // Tried removing this to fix issue #26 but it made things worse instead of better. BT headset AVRCP stopped working.
  { "NO CARRIER 0",               "W"},  // Bugfix: Only trigger a pause on carrier loss of A2DP first channel (0). Fixes issue #22.
  { "A2DP STREAMING STOP",        "W"},
  { "AVRCP FORWARD PRESS",        "N"},
  { "AVRCP BACKWARD PRESS",       "P"},  
  { "AVRCP FAST_FORWARD PRESS",   "F"},  
  { "AVRCP FAST_FORWARD RELEASE", "A"},  
  { "AVRCP REWIND PRESS",         "B"},  
  { "AVRCP REWIND RELEASE",       "A"},
  { "Debug Value Vol Up",         "+"},  // In this list only for console debugging of Vol up
  { "Debug Value Vol Dn",         "-"},  // In this list only for console debugging of Vol dn

  // Special case here for "Shuffle". Shuffle is is considered a "setting" in
  // the Bluetooth specification, meaning that the head unit queries for what
  // settings are supported and then we respond with what settings our device
  // supports, and what the available values are. The problem is that with
  // shuffle, the empeg does not report back to us the current state of the
  // shuffle, so we cannot report accurately back to the headunit whether
  // shuffle is on or shuffle is off. So all we can do is blindly toggle it
  // every time we get a message that is telling us to toggle shuffle on or
  // off. This works well on the one stereo I've got to test it on so far,
  // which is Vixy & Fishy's Kenwood. NOTE: Cannot add "repeat" since the
  // empeg serial port command set does not contain a repeat command.
  { "SET_APPLICATION_SETTING_VALUE", "%"},
};

// Issue #42: Global variable to keep track of whether or not we are connected
// and streaming to the Bluetooth. Certain behaviors should only be done when
// connected.
bool connected = false;

// Global variables to hold the track metadata information for the currently-
// playing track on the empeg Car, so that they can be in the responses to the
// queries from the host stereo to the Bluetooth chip. The numbers in the
// variable names (for example the "01" in "trackTitelString01" is a reminder
// to myself of which element attribute code in the Bluetooth spec represents
// which metadata. For instance attribute code "1" is "Track Title", attribute
// code "2" is "Artist", etc. Note that these are all strings because the docs
// and specification say that they must be sent up the stream as strings
// specifically. Depending on which response that you're responding to,
// sometimes some of these strings must be quote-enclosed. Documentation says
// that the maximum length for each one is 255 bytes. I'll use two string
// length limiters to save memory on the Arduino: A long one for the track
// metadata strings, and a short one for things like track number and playback
// position. Thing is... This is taking up too much memory and the input
// strings need to be shorter. Current status: Shortened metadata max length
// as an austerity measure.
int metadataMaxLength = 150;
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

// Global variables to hold the track metadata information which was the
// last set of metadata reported by the HandleEmpegStateChange routine. This set
// of variables is used for the purpose of limiting too much oversending of data
// up the Bluetooth. If all the data was the same as the last runthrough of the
// HandleEmpegStateChange function, then it won't bother to send the
// notification.
static String priorTitleString01 = "Unknown track on empeg Car";
static String priorArtistString02 = "Unknown artist on empeg Car";
static String priorAlbumString03 = "(Album N/A)";
static String priorNumberString04 = "00";
static String priorTotalNumberString05 = "99999";
static String priorGenreString06 = "Unknown genre on empeg Car";
static String priorPlaybackPositionString07 = "00";

// Strings to control the audio routing on the chip. Choose which string to
// use by setting the boolean variable "digitalAudio" in the code above. This
// string is used if you are using the Line In jacks on the Bluetooth device
// for audio (analog), aka "digitalAudio = false".
const String analogAudioRoutingControlString = "SET CONTROL AUDIO INTERNAL INTERNAL EVENT KEEPALIVE";  

// This string is used if you are using the I2S aka IIS connection on the
// Bluetooth device (this is a special digital connection which requires
// modifying inside of empeg Car player) aka "digitalAudio = true".
// The "16" means "16 bits in master mode". It might not be needed (docs
// are not clear on this point).
const String digitalAudioRoutingControlString = "SET CONTROL AUDIO INTERNAL I2S_SLAVE EVENT KEEPALIVE 16";

// Strings to set the gain level of the empeg, but only when BlueGigaEmpeg is
// configured for analog audio. Normally it's not used; BlueGigaEmpeg runs on
// digital audio so this gain setting usually does nothing. See Bluetooth chip
// docs for gain level table.
//
// Uncomment this line if your player will be used in AC/Home mode, (1v outputs):
//     const String empegGainSettingString = "SET CONTROL GAIN E 0";
//
// Uncomment this line if your player will be used in DC/Car mode, such as in
// a sled (4v outputs):
const String empegGainSettingString = "SET CONTROL GAIN 9 0";

// Strings to hold an incoming line of serial characters from Bluetooth
// module, empeg, or user debug console. Will be reset each time an entire
// message from any of those sources are processed.
static String btInputString = "";
int btInputStringMaxLength = 275;
static String empegInputString;
int empegInputStringMaxLength = 275;
static String userInputString;
int userInputStringMaxLength = 150;

// Some other string limits, how many string input characters will we allow
// from the serial port before it is too excessive and we should start a new
// string again?
int btSwallowInputStringMaxLength = 150;

// Global variables used in main "Loop" function to detect when an entire
// complete message string has been received from the Bluetooth module, empeg,
// or from the user via the Arduino debug serial port. Will be set to true
// when a line termination character is received indicating that the string is
// complete.
boolean btStringComplete = false;
boolean empegStringComplete = false;
boolean userStringComplete = false;

// Variable to keep track of what the current empeg Playing state is reported
// to be. This is used when responding to the Bluetooth module's queries for
// playback state. The empeg is always either playing or paused, so this is
// either true or false.
boolean empegIsPlaying = false; 

// Variable to keep track of what the last playing state we sent up to the
// Bluetooth was, so that if it hasn't changed then we don't need to waste the
// serial bandwidth to send it information that it already knows. "0" is the
// original startup state so that it knows whether it's sent something at all
// to begin with. At runtime after having sent one of the states up the
// Bluetooth at least once, then this value will be either "1" or "2".
static String priorIsPlaying = "0";

// Variable to keep track of whether or not we think the empeg has completed
// its bootup procedure and the player is running, or if it's some unknown
// state such as it's still in the process of booting up. This can help us
// decide better whether or not it's in a state that is ready to accept
// serial port commands.
boolean empegPlayerIsRunning = true;

// Variable to keep track of whether or not we have seen a timestamp message
// from the empeg serial port yet so far. The very first timestamp we receive
// from the empeg after a boot, a W or C command, or a track change command,
// should be recorded, but not acted upon. There is a routine to detect
// whether or not the player is playing or paused depending on whether the
// timestamp is changing or frozen. The problem is that the timestamp, the
// very first time we receive one after bootup or W or C or whatnot, will
// almost always be considered to be different from the one extant in memory
// (which at bootup or track change is 00:00:00) so there will at every bootup
// be a "mistake" where it thinks the empeg is playing when it isn't
// necessarily playing. So keep track of when we got the FIRST timestamp since
// bootup so that we don't "do stuff" the very first time, we just record the
// timestamp.
boolean empegFirstTimestamp = true;

// Additional variable to keep track of whether or not we have seen the first
// timestamp since first bootup. This is an additional level bugfix for issue
// #26 that is required after the fix to issue #57 which complicated things a
// little bit.
boolean empegFirstTimestampSinceBootup = true;

// Variables for the state of the "Reset/Pair" button that I am implementing
// on this assembly. The button, when pressed, will clear out the module's
// Bluetooth pairings, do a factory reset, and then do a pairing process where
// it attempts to pair with the first device that it sees. Note: For pairing
// with a car stereo with a touchscreen, this button is not be needed and in
// fact might interfere with the process. For some car stereos, the correct
// pairing process will be to NOT touch this button and instead do everything
// from the car stereo's touchscreen interface. Use this button for resetting
// the module and for pairing with things that don't have user interfaces,
// such as Bluetooth headsets.
const int pairButtonPin = 52;
const unsigned long pairButtonDebounceTimeMs = 50;
unsigned long pairButtonLastPressedMs = 0;
int pairButtonState;
int lastPairButtonState = LOW;

// Variable for Arduino pinout number of the reset/pair indicator blue LED.
const int pairLedPin = 50;

// Variable to globally keep track of whether we have recently initiated
// reset/pairing mode.
bool pairingMode = false;

// Variable to globally keep track of whether we have recently initiated a
// reset or a bounce of the AVRCP channel and should therefore not be trying
// to reset yet again in the same breath. Protect against reentrant code.
bool forceQuickReconnectMode = false;
bool bounceAvrcpMode = false;

// Variable to globally keep track of whether we are in the middle of a fast
// forward or rewind operation that was initiated from the Bluetooth. This is
// part of the fix to issue #32, "Fast forward can run away from you and get
// stuck."
bool blueToothFastForward = false;

// Variable to keep track of the timestamp of the prior output line. Used to
// calculate deltas between output lines for profiling.
unsigned long priorOutputLineMillis = 0;

// Include the version number file.
#include "Version.h"


// ------------------------------------
// Program code functions
// ------------------------------------


// ---------------------------------------------------------------------------
// Setup
//
// "Setup" is a default Arduino built-in function, which always runs
// once at the program startup, which happens in the following situations:
// - When power is initially applied to the Arduino.
// - Right after uploading the program to the Arduino.
// - Connecting to the Arduino's USB debug console.
// ---------------------------------------------------------------------------
void setup()
{
  // Initialize our variable which keeps track of the time deltas between
  // output lines for profiling, so I can see how many milliseconds have
  // elapsed between each piece of output. Start it at the timestamp when the
  // program starts.
  priorOutputLineMillis = millis();

  // Set up the Reset/pair blue LED indicator to "output" state.
  pinMode(pairLedPin, OUTPUT);

  // Make sure the reset/pair blue LED indicator is off at startup.
  digitalWrite(pairLedPin, LOW);
  
  // Set up the pair button to be in the "read" state.
  pinMode(pairButtonPin, INPUT);

  // Reserve bytes for the input strings, which are the strings we check to
  // see, for example, if there is a valid processable Bluetooth AVRCP command
  // incoming from the Bluetooth chip arriving from its serial port.
  btInputString.reserve(btInputStringMaxLength);
  empegInputString.reserve(empegInputStringMaxLength);

  // Reserve bytes for the some other strings to save memory.
  pairAddressString.reserve(pairAddressStringMaxLength);
  transactionLabelPlaybackStatusChanged.reserve(transactionLabelSize);
  transactionLabelTrackChanged.reserve(transactionLabelSize);
  avrcpChannel.reserve(transactionLabelSize);
  priorIsPlaying.reserve(2);

  // Reserve bytes for all the track metadata strings to save memeory.
  trackTitleString01.reserve(metadataMaxLength);
  trackArtistString02.reserve(metadataMaxLength);
  trackAlbumString03.reserve(metadataMaxLength);
  trackNumberString04.reserve(metadataSmallLength);
  trackTotalNumberString05.reserve(metadataSmallLength);
  trackGenreString06.reserve(metadataMaxLength);
  trackPlaybackPositionString07.reserve(metadataSmallLength);
  trackPlaybackLengthString.reserve(metadataSmallLength);

  priorTitleString01.reserve(metadataMaxLength);
  priorArtistString02.reserve(metadataMaxLength);
  priorAlbumString03.reserve(metadataMaxLength);
  priorNumberString04.reserve(metadataSmallLength);
  priorTotalNumberString05.reserve(metadataSmallLength);
  priorGenreString06.reserve(metadataMaxLength);
  priorPlaybackPositionString07.reserve(metadataSmallLength);

  // Configure the auto reconnect mode of this device based upon the user
  // setting at the top of the code. This is to allow for alternate versions
  // of the automatic reconnection behavior of the BlueGigaEmpeg. This is
  // one part of a set of workarounds to mitigate issue #71 (no AVRCP on
  // Onkyo) and issue #60 (Bad PDU Registrations) at the same time, since it
  // seems that I can't fix both of them easily at the same time using the
  // built in reconnect feature of the BlueGiga chip.
  switch (autoReconnectMode)
  {
    case 0:
      // If the user has configured autoReconnectMode to 0, then set
      // autoReconnectString to a blank string to indicate that no automatic
      // reconnects are to be attempted.
      autoReconnectString = "";
      break;

    case 1:
      // If the user has configured autoReconnectMode to 1, that is the
      // default setting. In this case, autoReconnectString is already set to
      // the correct default setting and there is nothing needed to do in this
      // situation. Exit out of the switch statement without changing the
      // contents of the string autoReconnectString.
      break;

    case 2:
      // If the user has configured autoReconnectMode to 2, then it is because
      // they want to use an alternate version of the reconnect string to fix
      // a specific issue on their specific stereo. Change autoReconnectString
      // to the alternate string so that the alternate string is used in all
      // cases instead of the default string.
      autoReconnectString = alternateAutoReconnectString;
      break;

    default:
      // If the user has mis-configured the autoReconnectMode variable, then
      // do nothing since autoReconnectString defaults to the correct default
      // setting.
      break;
  }
  
  // Open serial communications on the built Arduino debug console serial port
  // which is pins 0 and 1, this is the same serial output that you see when
  // using the Arduino serial monitor and/or a terminal program connected to
  // the USB interface that is used for Arduino monitoring and debugging.
  Serial.begin(115200);
  Log(F("Built in Arduino Serial has been started."));

  // Set the data rate for the Arduino Mega hardware serial port connected to
  // the Bluetooth module's serial port. This is the one that is directly
  // connected because it is UART-to-UART, pin-to-pin, it doesn't have to go
  // through a MAX232 chip.
  BlueGigaSerial.begin(115200);
  Log(F("BlueGiga Serial has been started."));
  
  // Set the data rate for the serial port connected to the empeg Car's serial
  // port, via the RS-232 and MAX232 circuit connected to the Arduino. The
  // empeg Car defaults to 115200 BPS in home/AC mode, and has configurable
  // BPS when sled-docked in car/DC mode. Refer to the accompanying README
  // file for more details about the RS-232 connection to the empeg. Note that
  // you will experience problems controlling the empeg if it is set to
  // anything other than 115200 in car mode, so make sure you have done that
  // step.
  EmpegSerial.begin(115200);
  Log(F("Empeg Serial has been started."));

  // Additional fix for issue #26 - Always send a pause to the empeg
  // immediately at powerup of the module so that all reset conditions
  // including power bounce have the empeg paused no matter what.
  SendEmpegCommand('W');

  // Log the version number from the version number file.
  Log("BlueGigaEmpeg Version " + String(majorVersion) + "." + String(minorVersion) + "." + String(buildNumber));

  // Report the chip type
  reportChipType();

  // Verify the serial buffer size
  verifySerialBuffer();

  // Configure the Bluetooth device at startup, call my routine which sets all
  // data to the desired overall system defaults. This does not erase any
  // pairing information, that is left untouched.
  SetGlobalChipDefaults();

  // Experimental fix for issue #37 and #28 at the same time. Decide whether
  // the chip should be in general discovery mode (GIAC), or in limited
  // discovery mode (LIAC) at boot up. If we already have a current pairing
  // buddy, then put it into Limited mode. If we don't have a pairing buddy,
  // then put it into General mode. The idea here is that if I want to pair
  // with my empeg when it's in the trunk of my Honda, I don't want to be
  // forced to walk back there and press the pair button if I don't have to.
  // To allow pairing from the Honda front panel without a buttonpress, the
  // device has to be in GIAC mode. But I also don't want the device to be too
  // promiscuous at other times. So to resolve the balance in this catch-22,
  // I'm saying that I only go into GIAC mode at bootup when we don't already
  // have a pairing buddy lined up, or, when we're actively trying to pair.
  //
  // First step is to query the Bluetooth module to find out if we have a
  // pairing buddy at this time. Issue the command to list existing pairings.
  SendBlueGigaCommand(F("SET BT PAIR"));

  // After listing the existing pairings, idle for a moment to allow the main
  // string processing loop to receive the response from the pair list. The
  // response can take as much as 500ms or more to come back, so idle for it
  // for up to <some arbitrarily long time period> and then be done idling as
  // soon as you get any response at all (the "true" in this statement stops
  // idling once you get a line ending character, and the pair query response
  // is a single line which should be the first line you receive after the
  // query).
  DisplayAndProcessCommands(3000, true);

  // At this point the main string processor routine will, if it found a
  // pairing buddy in the response to the SET BT PAIR command, have filled out
  // our variable of our current pairing buddy, and we can now check it to see
  // if it has a value in it. A blank value means no pairing buddy, a nonblank
  // value means that we found a previously programmed pairing buddy.
  if (pairAddressString == "")
  {
    // No previous pairing buddy was found, so place the Bluetooth chip into a
    // mode where it is more easily discoverable. This is the "General Inquiry
    // Access Code (GIAC)" value. We will put the device in this mode only
    // when we haven't got a pairing buddy yet, or when we are are actively in
    // pairing mode.
    Log(F("There is no existing pairing buddy. Placing chip into general discovery mode."));
    SendBlueGigaCommand(F("SET BT LAP 9e8b33"));
  }
  else
  {
    // A previous pairing buddy was found in the Bluetooth chip's memory, so
    // place the Bluetooth chip into a mode where it is not easily
    // discoverable. This is the "Limited Dedicated Inquiry Access Code
    // (LIAC)" value. We will leave the device in this mode most of the time
    // so that it is not shown in the pairing screens of most devices unless
    // you are in pairing mode.
    Log(F("There is an existing pairing buddy. Placing chip into limited discovery mode."));
    SendBlueGigaCommand(F("SET BT LAP 9e8b00"));    
  }

  // Reset the Bluetooth chip every time the Arduino chip is started up and
  // after we have set all of its defaults (which will still be saved after
  // the reset; this is a soft reset). Resetting it here prevents bugs where
  // the Bluetooth state is desynchronized from the Arduino state.
  QuickResetBluetooth(0);    
}


// ---------------------------------------------------------------------------
// Loop
//
// "Loop" function is a built in Arduino function, it is the main runtime
// program loop in Arduino. This function repeatedly loops while the Arduino
// is powered up. It will loop as quickly as we allow it to loop. Any code
// that executes as a sub-function of this code will be the thing that slows
// it down. For example, if you stay in a subroutine for too long then this
// loop will not execute while that subroutine is running. Hopefully all the
// subroutines I have written are all very short and quick to execute. We want
// this loop running fast so that it can get and process all characters from
// all serial ports.
// ---------------------------------------------------------------------------
void loop()
{
  // The MainInputOutput returns a CHAR variable. I'm not using it here but I
  // use it elsewhere so it's just a placeholder here.
  static char mainLoopChar = 0;

  // Read and handle all main input/output functions on all serial ports, one
  // character at a time. This runs as quickly as we let it run, since it
  // simply happens each time we go through the main Arduino "loop" function.
  mainLoopChar = MainInputOutput();

  // Read and handle all digital pushbutton I/O's such as the reset/pair
  // button.
  MainPushbuttonHandler();
}


// ---------------------------------------------------------------------------
// Report Chip Type
//
// This is a function to log the type of Arduino Mega chip that this program
// is running on. This is oversimplified and does not show all chip types,
// just a subset of them.
// ---------------------------------------------------------------------------
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


// ---------------------------------------------------------------------------
// Verify Serial Buffer
//
// Report a problem if the serial buffer is not large enough. The Arduino
// header libraries must be edited in order for this program to work, and this
// code pre-checks to make sure that this program was compiled with correctly
// modified header libraries to make sure there is enough buffer space.
// ---------------------------------------------------------------------------
void verifySerialBuffer()
{
  // Create variable to use in this function.
  String myTxBufferSizeString;
  int myTxBufferSizeInt;
  String myRxBufferSizeString;
  int myRxBufferSizeInt;

  // Retrieve variable which was defined in compiler header file.
  myRxBufferSizeString = (String)SERIAL_RX_BUFFER_SIZE;
  myTxBufferSizeString = (String)SERIAL_TX_BUFFER_SIZE;
  myRxBufferSizeInt = myRxBufferSizeString.toInt();
  myTxBufferSizeInt = myTxBufferSizeString.toInt();
  
  // Verify serial buffer size
  if ((myRxBufferSizeInt < 256) || (myTxBufferSizeInt < 128))
  {
    Log("Serial receive buffer size: " + (String)myRxBufferSizeInt);
    Log("Serial xmit buffer size:    " + (String)myTxBufferSizeInt);
    Log(F("------------------------------------------------------------------------------"));
    Log(F("         Compilation problem has occurred with this Arduino sketch.           "));
    Log(F(" "));
    Log(F("  This program requires a larger serial buffer size than standard size.       "));
    Log(F("  The Arduino compiler's header files must be edited to make this possible.   "));
    Log(F("  Please edit the following file, recompile and re-upload this sketch:        "));
    Log(F(" "));
    Log(F("     (install location)/hardware/arduino/avr/cores/arduino/HardwareSerial.h   "));
    Log(F(" "));
    Log(F("  Increase SERIAL_RX_BUFFER_SIZE to 256 and SERIAL_TX_BUFFER_SIZE to 128,     "));
    Log(F("  recompile, and reupload your sketch to the Arduino module.                  "));
    Log(F("  Please refer to the README file accompanying this sketch for details.       "));
    Log(F(" "));
    Log(F("------------------------------------------------------------------------------"));
  }
  else
  {
      Log("Serial receive buffer size: " + (String)myRxBufferSizeInt + " - Good.");
      Log("Serial xmit buffer size:    " + (String)myTxBufferSizeInt + " - Good.");
  }
}


// ---------------------------------------------------------------------------
// MainPushbuttonHandler
//
// Workhorse for handling digital pushbuttons implemented in this assembly,
// for example this routine implements the reset/pair button I made. Includes
// debouncing logic so that you don't try to run the same command multiple
// times from a single button press.
// ---------------------------------------------------------------------------
void MainPushbuttonHandler()
{
  // Record the timestamp of the moment in time that we started this function.
  // This timestamp counts up from the moment the Arduino is powered up. The
  // unsigned long can be up to 4,294,967,295, which in milliseconds is over
  // 1100 hours. So if this value wraps around, I don't know what will happen,
  // but I hope you're not running your car for 1100 hours without turning it
  // off once in a while.
  static unsigned long currentMillis = 0;
  currentMillis = millis();

  // Read the state of the reset/pair button at the time we started this
  // function.
  static int pairButtonCurrentReading = LOW;
  pairButtonCurrentReading = digitalRead(pairButtonPin);

  // Check to see if the state of the button changed since our last reading of
  // it.
  if (pairButtonCurrentReading != lastPairButtonState)
  {
    // If the state was different, then reset the debouncing timer
    pairButtonLastPressedMs = currentMillis;
  }

  // Check to see how long it has been since the last time we got a state
  // change. Only consider the button state to have changed if our time range
  // is outside the time range of our debounce timer.
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

  // Save the current button reading into the last button state, for use in
  // the next run through this loop.
  lastPairButtonState = pairButtonCurrentReading;
}


// ---------------------------------------------------------------------------
// SetGlobalChipDefaults
//
// Initializes the settings we want for this particular Bluetooth chipset.
// ---------------------------------------------------------------------------
void SetGlobalChipDefaults()
{
  // For convenience, every time we reset or start up our chip, we also send
  // empeg a pause command, setting the empeg to a paused state. Then, if
  // Bluetooth reconnects after the reset, then the unpause will happen
  // automatically later, when it receives the stream start command from the
  // car stereo.
  Log(F("Pausing empeg for convenience while setting global chip defaults."));
  SendEmpegCommand('W');   

  // Log to the debug console that we are starting this procedure.
  Log(F("Setting Defaults."));
  
  // Configure the system to not accidentally go into data mode in certain
  // situations. Parameters are:
  // "-"  = Disable ASCII character escape sequence that would put it into command mode.
  // "00" = Bitmask for which digital I/O pins are used for DTR signal (00=none)
  // "0"  = DTR disabled.
  SendBlueGigaCommand(F("SET CONTROL ESCAPE - 00 0"));

  // Configure serial port local echo settings. "5" is send only errors and
  // events to screen, otherwise silent. This prevents some of the "echo back"
  // commands coming back from our sent commands accidentally triggering other
  // functions in this program.
  SendBlueGigaCommand(F("SET CONTROL ECHO 5"));

  // Configure the system so that it does not send battery warnings nor
  // attempt to shut down when encountering low voltage situations, since
  // this device will only be running when the car ignition is on, and will be
  // getting the voltage (indirectly) from the car power.
  SendBlueGigaCommand(F("SET CONTROL BATTERY 0 0 0 0"));
  
  // Change some configuration bits on the player. See section 6.75 of the
  // iWrap command reference documentation for details on this topic. The
  // format of the command is:  
  //    SET CONTROL CONFIG 0000 0000 0000 1100
  // The default setting is the one shown above. Each group of digits is a
  // hexadecimal number which represents a set of configuration bits. The
  // default setting of "1100" in the "config block" sets bit positions 8 and
  // 12 (aka 0b0001000100000000) which are flags for "Enables SCO links" and
  // "Randomly replace one of the existing pairings".
  // I am also adding this bit:
  //     Bit 14 aka 0b0100000000000000 - "UART will be optimized for low latency"
  // This is an attempt to improve an issue where there is a visible
  // difference between the empeg visuals on the empeg VFD display and the
  // audio coming out of the host stereo's speakers. This does not solve the
  // problem but it doesn't seem to hurt. For true low latency we'd need to
  // buy the APT-X LL codec (licensing fee).
  SendBlueGigaCommand(F("SET CONTROL CONFIG 0000 0000 0000 5100"));

  // Configure the A2DP codec that will be used for the audio connection. This
  // string is defined at the top of this program and there are multiple
  // options for which codec can be used. First, send the command to clear
  // existing codecs if there are multiple ones, then send the command to set
  // up the codecs defined at the top of the code.
  SendBlueGigaCommand(F("SET CONTROL CODEC"));
  SendBlueGigaCommand(codecString);

  // Configure the WT32i development board to NOT use the additional external
  // TI AIC32I external codec chip. Unused if you're not using the BlueGiga
  // Development board which contains the external codec.
  //  SendBlueGigaCommand(F("SET CONTROL EXTCODEC PRE"));

  // Set Bluetooth Class of Device - Docs say this is needed.
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
  // 0x240438 =
  //  - Major service class: Rendering and Audio
  //  - Major device class:  AudioVideo
  // 0x240100 =
  //  - Major service class: Rendering and Audio
  //  - Major device class:  Computer
  // 0x240200 = 
  //  - Major service class: Rendering and Audio
  //  - Major device class:  Phone
  // 0x24020C =
  //  - Major service class: Rendering and Audio
  //  - Major device class:  Phone
  //  - Minor device class:  Smartphone
  // 0x60020C = 
  //  - Major service class: Telephony and Audio
  //  - Major device class:  Phone
  //  - Minor device class:  Smartphone  
  // 
  // The docs say: "In the case of A2DP Source, the specification mandates the
  // use of the Capturing Service bit (0x080000). The Audio Service (0x200000)
  // and Audio/Video Major Device Class (0x000400) should be used, but they
  // are not mandatory. A common example Minor Device Class is Hi-Fi Audio
  // Device (0x000028). The combined CoD is then 0x280428."
  // 
  // NOTE: in the docs for the BlueGiga module, it shows examples where, even
  // though this is a hexadecimal number, it is not preceded by "0x". So for
  // example, "SET BT CLASS 280428" is the correct command whereas "SET BT
  // CLASS 0x280428" is not, at least according to the docs, and it even says
  // "SYNTAX ERROR" if you precede it with the 0x.
  SendBlueGigaCommand(F("SET BT CLASS 280428"));  

  // Turn off "Serial Port Profile" on the Bluetooth chip, we don't need it
  // for A2DP.
  SendBlueGigaCommand(F("SET PROFILE SPP"));

  // Configure chip to the "A2DP Source" profile, which sends music out to the
  // paired car stereo instead of the other way around (the other way is
  // called "A2DP Sink").
  SendBlueGigaCommand(F("SET PROFILE A2DP SOURCE"));

  // Set chip to receive AVRCP commands with specific categories - See
  // Bluetooth chip's PDF documentation for descriptions of the AVRCP
  // categories, section 3.2 of the A2DP/AVRCP profile document.
  // - Bit Value 0001 - Category 1: Player/Recorder - must support PLAY and PAUSE
  // - Bit Value 0002 - Category 2: Monitor/Amplifier - must support VOLUP and VOLDN (volume up and down)
  // - Bit Value 0004 - Category 3: Tuner - must support CHUP and CHDN (channel up and down)
  // - Bit Value 0008 - Category 4: Menu - must support MSELECT, MUP, MDN, MLEFT, MRIGHT (menu select and directions)
  // Example: value of "3" would be Player/recorder plus Monitor/Amplifier.
  SendBlueGigaCommand(F("SET PROFILE AVRCP TARGET 1"));

  // Our project identifies itself nicely on car stereo's LCD screen when
  // pairing, with a nice happy friendly name that we like very much.
  // Apologies if I don't accurately capitalize "empeg" (little e) in every
  // place in my code. It's  correct here to have the lower case e, and this
  // is the customer-facing string, so this is the place it has to be correct
  // the most.
  SendBlueGigaCommand(F("SET BT NAME empeg Car"));

  // Set Bluetooth pairing security authorization types (defined at top of
  // program).
  SendBlueGigaCommand(btAuthTypeString);
  SendBlueGigaCommand(btPinCodeString);

  // Audio routing on the chip. Control string for audio routing is configured
  // at top of program.
  if (digitalAudio)
  {
    SendBlueGigaCommand(digitalAudioRoutingControlString);    
  }
  else
  {
    SendBlueGigaCommand(analogAudioRoutingControlString);  
  }

  // Turn off the mic preamp to prevent distortion on line level inputs if
  // they are being used.
  SendBlueGigaCommand(F("SET CONTROL PREAMP 0 0"));    
  
  // Turn off the mic bias voltage (well, set it to as low as possible).
  SendBlueGigaCommand(F("SET CONTROL MICBIAS 0 0"));    
  
  // Set line level input gain on the Bluetooth chip, value is set in global
  // constant at top of program. This is for situations where the line level
  // inputs might be used.
  SendBlueGigaCommand(empegGainSettingString);    

  Log(F("Done Setting Defaults."));
}


// ---------------------------------------------------------------------------
// PairBluetooth
//
// This is the function that gets executed when you press the Reset/Pair
// button. Resets the pairing table on the Bluetooth chip, erasing all
// previous paired devices, so that the device can freshly pair anew with a
// new device. Also runs a program that tries to pair with discovered devices
// for a short amount of time. Note: For connection to a car stereo, you might
// not need to issue this command, pairing might work fine without touching
// this function. I have found it is necessary with things like Bluetooth
// headsets, but not always necessary for pairing with car stereos. For car
// stereos, sometimes you handle all pairing steps from the stereo's touch
// screen.
//
// This function doubles as a way to reset the device to factory defaults and
// restart it, in cases where that is needed.
// ---------------------------------------------------------------------------
void PairBluetooth()
{
  // Don't execute this if we are currently already in pairing mode. Protect
  // against possible re-entrant code.
  if (pairingMode)
  {
    return;
  }

  // Set flag indicating we recently started pairing mode, and turn on the
  // Reset/pair indicator blue LED so that it stays on  bright for a long time
  // during the pairing process.
  pairingMode = true;
  digitalWrite(pairLedPin, HIGH);

  // Log what we are about to do, to the Arduino debugging console.
  Log (F(" "));
  Log (F("--------------------------------------"));
  Log (F("           RESETTING DEVICE           "));
  Log (F("--------------------------------------"));
  Log (F(" "));

  // Close current Bluetooth pairings so that we can erase them. Turns out
  // that the command to erase Bluetooth pairings (used below) will get a
  // SYNTAX ERROR if you issue the command while it's connected.
  SendBlueGigaCommand(F("CLOSE 0"));
  DisplayAndSwallowResponses(1, 100);
  SendBlueGigaCommand(F("CLOSE 1"));
  DisplayAndSwallowResponses(1, 100);

  // Clear the global variable of our pairing buddy to occur as close as
  // possible to the erasing of the pairings from the Bluetooth chip. Also do
  // the avrcpChannel at the same time since it's part of all that.
  pairAddressString = ""; 
  avrcpChannel = "";

  // Erase all previous Bluetooth pairings (not included in the factory
  // default reset below). Note: the Star is required to erase pairings,
  // otherwise the command would just list the pairings.
  SendBlueGigaCommand(F("SET BT PAIR *")); 
  DisplayAndSwallowResponses(1, 100);

  // Erase all previous auto-reconnect settings (not included in the factory
  // default reset and must be disabled before attempting to pair).
  SendBlueGigaCommand(F("SET CONTROL AUTOCALL"));     // Blank is required to disable 
  DisplayAndSwallowResponses(1, 100);
  SendBlueGigaCommand(F("SET CONTROL RECONNECT *"));  // Star is required to disable
  DisplayAndSwallowResponses(1, 100);
  SendBlueGigaCommand(F("STORECONFIG"));              // Make sure SET CONTROL RECONNECT is stored
  DisplayAndSwallowResponses(1, 100);

  // Make certain that the above changes are saved and "take" - RESET seems to
  // be the sure way.
  QuickResetBluetooth(0);

  // See what our settings are after this. Debugging an issue where the stuff
  // above didn't clear out our current pairing buddy. It should have.
  SendBlueGigaCommand(F("SET BT PAIR"));
  DisplayAndSwallowResponses(2, 300);

  // Reset device to factory defaults using "SET RESET" command (parameter 2).
  // Note that "SET RESET" (factory defaults) is different from "RESET" (which
  // is just a reboot). This does not erase pairings (that is accomplished by
  // the commands above instead).
  QuickResetBluetooth(2); // "2" means factory reset of all settings

  // Set up the device for use with the empeg car.
  SetGlobalChipDefaults();

  // Experimental attempt to fix issue #63. If you have more than one device
  // paired, then the auto reconnect feature is funky and does not always
  // successfully beat the iPhone to the punch when you start the car. We want
  // the empeg to win that race every time. My suspicion is that if you pair
  // with something like a Bluetooth headset (reset/pair buttonpress needed)
  // and then subsequently pair with the Honda (no press of reset/pair button
  // needed) then you now have two paired items in the pairing buddy table.
  // Then when it comes time to do auto reconnect, it has to round robin
  // through the list of pairing buddies, meaning sometimes it loses the
  // reconnection race to the iPhone. Attempt to fix the issue by limiting the
  // number of paired devices to 1 so that the automatic reconnect, when it
  // gets enabled, is always only ever trying to reconnect to a single stereo
  // instead of potentially multiple stereos, and thus always does it
  // quicker. Note: Must do this after the factory reset or else it will go
  // away immediately.
  SendBlueGigaCommand(F("SET BT PAIRCOUNT 1")); 
  DisplayAndSwallowResponses(1, 100);

  // The reset statement (which was originally in the SetGlobalChipDefaults
  // statement) is no longer part of SetGlobalChipDefaults, so we must do it
  // here as part of the pre-pairing procedure. IMPORTANT NOTE: I discovered
  // that this is a super critical part of the process. It's not enough just
  // to do a SET RESET and set the chip defaults right before pairing, it's
  // actually necessary to fully reboot the unit right before pairing, or else
  // the pairing process will not work correctly. So make sure this gets done
  // before the pairing process.
  QuickResetBluetooth(0); // "0" means just a reboot

  // Freewheel for a moment after setting global chip defaults. Need a time
  // window before the chip will accept the INQUIRY command below.
  DisplayAndSwallowResponses(9, 700);

  // Log Partial Completeness.
  Log (F(" "));
  Log (F("--------------------------------------"));
  Log (F("   Done resetting. Beginning pair.    "));
  Log (F("--------------------------------------"));
  Log (F(" "));  

  // Clear out our input string before starting the pairing process just in
  // case we pressed the pair button in the middle of a string input
  btInputString = "";
  
  // Place the Bluetooth chip into a mode where it is discoverable. This is
  // the "General/Unlimited Inquiry Access Code (GIAC)" value. This allows
  // other devices to see us more easily while we are in pairing mode.
  Log(F("Placing chip into general discovery mode for the pairing process."));
  SendBlueGigaCommand(F("SET BT LAP 9e8b33"));

  // Attempt pairing x number of seconds before quitting. The main loop for
  // processing serial input/output will automatically detect the necessary
  // responses from Bluetooth devices in the air nearby and answer
  // appropriately at the appropriate times. At regular intervals, check to
  // see if the pairing process was completed and bail out of the loop.
  for (int i=0; i<=30; i++)
  {
    // At even intervals throughout the loop, begin an inquiry. Do this
    // at evenly spaced intervals instead of continuously, so that a
    // another device that is trying to initiate pairing can see it. 
    // This is an experiment to fix GitHub issue #37.
    if ( (i==0) || (i==10) || (i==20) )
    {
      Log(F("Starting inquiry."));
      SendBlueGigaCommand(F("INQUIRY 5"));
    }

    // GitHub issue #37 experiment. At even intervals through the loop,
    // cancel the inquiry. It should theoretically already have been
    // self-canceled, the inquiry command should self-stop after the
    // correct number of seconds already. This is experimental and
    // "just in case".
    if ( (i==5) || (i==15) || (i==25) )
    {
      Log(F("Stopping inquiry."));
      SendBlueGigaCommand(F("IC"));
    }

    // Process commands in our main loop for 1 second. If a device is
    // discovered then it will be automatically paired by the main
    // string processing handler.
    DisplayAndProcessCommands(1000, false);

    // Check if the pairing process has been completed by the main string
    // processing handler. If so, the main string processing handler will
    // have set this to "false", so break out if we detect this.
    if (pairingMode == false) { break; }
  }
  
  // GitHub issue #75. When pairing has finished, cancel the inquiry mode.
  // This is particularly useful in cases where pairing succeeded while it
  // was still inside an inquiry. If the "break" statement above is hit
  // because the pairing was successful, then the inquiry mode can be
  // terminated early at this point. If it hits this line after all inquiries
  // are finished making attempts and no pairing was done during that period,
  // then this is "insurance" to make sure to turn off inquiry mode when we
  // have decided to give up the search.
  Log(F("Stopping inquiry."));
  SendBlueGigaCommand(F("IC"));
  
  // We are done with pairing mode if either we have run out of loop time or
  // if the code above detects that pairing mode got completed. Set the global
  // flag indicating that pairing mode has stopped, and also turn off the
  // pairing indicator blue LED, so that it is mostly off after the pairing
  // process is done.
  pairingMode = false;
  digitalWrite(pairLedPin, LOW);

  // Attempting to fix issue #28 and issue #37 at the same time. Check to see
  // if pairing was successful and if we got a pairing buddy. A blank value
  // means no pairing buddy was found during the pairing procedure above, a
  // nonblank value means that we found a pairing buddy during pairing above.
  if (pairAddressString == "")
  {
    // No previous pairing buddy was found, so place the Bluetooth chip into a
    // mode where it is more easily discoverable. This is the "General Inquiry
    // Access Code (GIAC)" value. We will put the device in this mode only
    // when we haven't got a pairing buddy yet, or when we are are actively in
    // pairing mode.
    Log(F("No pairing buddy was found. Placing chip into general discovery mode."));
    SendBlueGigaCommand(F("SET BT LAP 9e8b33"));
  }
  else
  {
    // A previous pairing buddy was found in the Bluetooth chip's memory, so
    // place the Bluetooth chip into a mode where it is not easily
    // discoverable. This is the "Limited Dedicated Inquiry Access Code
    // (LIAC)" value. We will leave the device in this mode most of the time
    // so that it is not shown in the pairing screens of most devices unless
    // you are in pairing mode.
    Log(F("A pairing buddy was found. Placing chip into limited discovery mode."));
    SendBlueGigaCommand(F("SET BT LAP 9e8b00"));    
  }  

  // Log that we are done.
  Log (F(" "));
  Log (F("--------------------------------------"));
  Log (F("        Pairing process ended.        "));
  Log (F("--------------------------------------"));
  Log (F(" "));      
}


// ---------------------------------------------------------------------------
// MainInputOutput
// 
// Main workhorse to handle the main loop character input output and logging
// to serial port. Detects if the character strings are any of the special
// commands that we are waiting for. Must call this repeatedly (for instance
// in the main loop) for it to be useful, since it only processes one
// character at a time. This function will receive serial port data, log it to
// the debug port, interpret it, and if certain AVRCP commands are received,
// then respond to them and/or send the corresponding serial commands to the
// empeg-car if appropriate.
//
// Returns:
//    (char) The character it processed that it received from the Bluetooth
//    serial port, if any. If no character was received it will return a zero
//    value character. Note that if a zero value character was received it
//    will also return that. So don't use zero as the test to see whether or
//    not it processed a character.
// ---------------------------------------------------------------------------
char MainInputOutput()
{
  // Initialize the characters that will be retrieved in this loop.

  // From the Bluetooth module.
  char inChar = 0; 

  // From the debug serial port (so you can type commands to the Bluetooth)
  char userChar = 0;

  // From the empeg car serial port
  char empegChar = 0;

  // Check to see if any characters are available on the empeg serial port
  // from the empeg car, display them on the debugging console, and process
  // them into individual lines for later parsing for track status and track
  // metadata information.
  if (EmpegSerial)
  {
    if (EmpegSerial.available())
    {
      // Read one character (byte) from the empeg serial port.
      empegChar = EmpegSerial.read();

      if (displayEmpegSerial)
      {
        // Log to the debugging console what we saw on the port but only if we
        // are in character-by-character mode
        if (!logLineByLine)
        {
          LogChar(empegChar);
        }
      }

      // Add the character that we read to our input line string for use later.
      empegInputString += empegChar;   

      // Check to see if it's a linefeed or if the length of the string is too
      // large to hold, either way we consider the string to be done, baked,
      // and ready to do something with.
      if ((empegChar == '\n') || (empegInputString.length() >= (empegInputStringMaxLength - 2)))
      {
        // String is ready to be processed if one of the above things was true.
        empegStringComplete = true;

        // Trim extra CR LF and whitespace at the end of the string if any.
        empegInputString.trim();        

        // Log the string if we are in line by line logging mode.
        if (displayEmpegSerial)
        {
          if (logLineByLine)
          {
            Log(empegInputString);
          }
        }
      }         
    }
  }

  // Check to see if we have received a completed string from the empeg and
  // therefore need to start processing the contents of that string.
  if (empegStringComplete)
  {
    empegStringComplete = false;

    // Call the function to process the string and do the needful.
    HandleEmpegString(empegInputString);
    
    // Reset the string after processing it, ready for the next line.
    empegInputString = "";
  }  
  
  // Check to see if any characters are available on the Bluetooth serial port
  // from the Bluetooth module, display them on the debugging console, and
  // process them into individual lines for later parsing and
  // command/response.
  if(BlueGigaSerial)
  {
    if (BlueGigaSerial.available())
    {
      // Begin blinking the LED to indicate activity.
      BlinkBlue(true);

      // Read one character (byte) from the Bluetooth serial port.
      inChar = BlueGigaSerial.read();
      
      // Log to the Arduino debugging console what we saw on the Bluetooth
      // serial port but only if we are in character-by-character mode
      if (!logLineByLine)
      {
        LogChar(inChar); 
      }
      
      // Add the character that we read to our input line string for use
      // later.
      btInputString += inChar;
      
      // Check to see if it's a carriage return or if the length of the string
      // is too large to hold, either way we consider the string to be done,
      // baked, and ready to do something with.
      if ((inChar == '\n') || (btInputString.length() >= (btInputStringMaxLength - 2)))
      {
        // String is ready to be processed if one of the above things was
        // true.
        btStringComplete = true;

        // Trim extra CR LF and whitespace at the end of the string if any.
        btInputString.trim();

        // Log the string if we are in line by line logging mode.
        if (logLineByLine)
        {
          Log(btInputString);
        }
      }

      // Finish blinking the LED.
      BlinkBlue(false);
    }
  }

  // Check to see if we have received a completed string from the Bluetooth
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
  // debug port) the things that I type could either be empeg single-character
  // serial port commands intended for the empeg, or they could be iWrap
  // commands intended for the Bluetooth chip.  Check to see if any characters
  // are available from the user on the Arduino serial port from the Arduino
  // debug cable, and process them into individual lines for later parsing and
  // command/response.
  if(Serial)
  {
    if (Serial.available())
    {
      // Begin blinking the LED to indicate activity.
      BlinkBlue(true);

      // Read one character (byte) from the Bluetooth serial port.
      userChar = Serial.read();
      
      // Add the character that we read to our input line string for use
      // later.
      userInputString += userChar;
      
      // Check to see if it's a carriage return or if the length of the string
      // is too large to hold, either way we consider the string to be done,
      // baked, and ready to do something with.
      if ((userChar == '\n') || (userInputString.length() >= (userInputStringMaxLength - 2)))
      {
        // String is ready to be processed if one of the above things was
        // true.
        userStringComplete = true;

        // Trim extra CR LF and whitespace at the end of the string if any.
        userInputString.trim();

        // Log the string.
        Log(userInputString);
      }

      // Finish blinking the LED.
      BlinkBlue(false);
    }
  }

  // Check to see if we have received a completed string from the user
  // and therefore need to start processing the contents of that string.
  if (userStringComplete)
  {
    userStringComplete = false;

    // Call the function to process the string and do the needful.
    HandleUserString(userInputString);
    
    // Reset the string after processing it, ready for the next line.
    userInputString = "";
  }

  // Monkey Reconnect is a failed attempt to work around issue #60, problems
  // with the existing reconnect features found in the iWrap6 language. The
  // fix was to try my own reconnects instead of using either of the existing
  // automatic reconnect features in the iWrap6 language. Since both of them
  // gave me troubles on my Honda stereo, I attempted to control my own
  // reconnects by doing it myself, in my main input loop, to see if I could
  // solve any of the troubles I was having. In the end, this caused more
  // problems than it solved, and the fix to issue #60 turned out to be
  // something completely different. It was actually caused by bad parameters
  // to one of the built-in auto reconnect command which was not clearly
  // documented. I am keeping the code here for posterity, since I might want
  // to leverage this same set of functionaly for other features at a later
  // date. But I am leaving it disabled for now, with the variable
  // monkeyReconnectEnabled set to false at the top of the code.
  if (monkeyReconnectEnabled)
  {
    // First, check to see if it's been x seconds of elapsed time since bootup
    // (where x is the monkey reconnect interval specified at the top of this
    // code) by using the modulo arithmetic operator. For instance if you
    // divide the current timestamp milliseconds by 5000 then the remainder
    // value (the result of the modulo % operation) will come out as zero only
    // once every five seconds. This is a simple way to do a one-line repeat
    // timer without needing to store and flip flags and such.
    if((millis() % monkeyReconnectInterval) == 0)
    {
      // If we are not yet connected and we are not already in pairing mode,
      // then try to connect to our existing pairing buddy if we have one.
      if (!pairingMode && !connected)
      {
        // If we don't have a pairing buddy yet, then we need to try to find
        // out who that pairing buddy is/was. To do this, we issue a command
        // to the Bluetooth chip to report who its pairing buddies are. The
        // command which asks for a list of pairing buddies is "SET BT PAIR".
        // Then, there is code elsewhere, in the string handler routines,
        // which automatically detects the resulting response of the list of
        // pairing buddies and parses out the correct pairAddressString from
        // that list and places it in the variable for us. So we won't be able
        // to make a connection attempt on this loop, but on a later loop, if
        // there was a pairing buddy in the list, then the pairAddressString
        // will be populated on our next loop through and then it will try to
        // pair with that address. If there is no current pairing buddy, then
        // the string will remain blank and we'll just keep asking over and
        // over again in a loop.
        if (pairAddressString == "")
        {
          // Check to see if the Bluetooth chip has a pairing buddy already.
          Log(F("Trying to find out if we have a pairing buddy yet."));
          SendBlueGigaCommand(F("SET BT PAIR"));

          // Prevent this code from firing twice in the same millisecond if
          // the loop happens to execute extra fast. Normally I would cringe
          // at a blind sleep in the code, but this can only occur if there
          // hasn't been a connection yet, and so the BlueGigaEmpeg isn't
          // doing much critical yet. And also, it's only for 1 millisecond
          // every few seconds, so it has less chance of causing problems.
          delay(1);
        }
        else
        {
          // We have a pairing buddy address, so try to connect to that buddy.
          Log(F("Trying to connect now to our current pairing buddy."));
          SendBlueGigaCommand("CALL " + pairAddressString + " 19 A2DP");

          // Attempt to fix issue #28 and issue #37 at the same time. If we are
          // setting ourselves to auto reconnect to someone, then we must have
          // successfully paired and connected with someone. So now place the
          // Bluetooth chip into a mode where it is not easily discoverable. This
          // is the "Limited Dedicated Inquiry Access Code (LIAC)" value. We will
          // leave the device in this mode most of the time so that it is not
          // shown in the pairing screens of most devices unless you are in
          // pairing mode.
          Log(F("Since there is an existing pairing buddy, placing chip into limited discovery mode."));
          SendBlueGigaCommand(F("SET BT LAP 9e8b00"));  

          // Prevent this code from firing twice in the same millisecond if
          // the loop happens to execute extra fast. Same caveats as above.
          delay(1);
        }
      }
    }
  }
   
  // Return from this function, return the Bluetooth character we processed in
  // this loop.
  return inChar;
}


// ---------------------------------------------------------------------------
// DisplayAndProcessCommands
//
// Repeatedly runs the serial data processing routine (MainInputOutput) for a
// specified amount of time, so that you can continue to process data outside
// the top level main program loop. 
// 
// An example of using this would be: if you are inside a subroutine and you
// need to wait for a certain amount of time before returning control back to
// the main program loop, but you can't afford to hang the serial I/O and the
// prevent the main program loop for that long.
// 
// Another common use for this would be: If you issue a command to the
// Bluetooth and you want to wait for the response, and then process the data
// retrieved in the response (assuming that the data is automatically
// retrieved by the MainInputOutput routine).
//
// Parameters
//    (ulong) idleTimeMs  Amount of time in approximate milliseconds to wait.
//    (bool)  waitForLineEnding   Stop waiting if a CR or LF was received.
// ---------------------------------------------------------------------------
void DisplayAndProcessCommands(unsigned long idleTimeMs, bool waitForLineEnding)
{
  // Character that will be returned from the main input/output function,
  // which processes one character per loop through the function and returns
  // that character.
  static char receivedChar = 0;

  // Record the timestamp of the moment in time that we started this function
  // so that we can tell how long we have been waiting here in this function.
  unsigned long startingDnpMillis = 0;
  startingDnpMillis = millis();

  // Loop the main input output function one character at a time. If string
  // processing needs to occur during the wait, it will do that then return
  // back to this loop normally.
  do
  {
    // Perform the main input output loop as long as we're still waiting.
    receivedChar = MainInputOutput();

    // Check to see if we are supposed to bail out as soon as we get a line
    // ending.
    if (waitForLineEnding)
    {
      if (receivedChar == '\n')
      {
        return;
      }
    }
  }
  while ((millis() - startingDnpMillis) <= idleTimeMs);
  // Continue looping: Current milliseconds on the clock minus the starting
  // milliseconds gives us the total time we've been in this loop. Check to
  // see if our time is up.
}


// ---------------------------------------------------------------------------
// HandleUserString
// 
// Function to process the string data received from the Arduino debug cable
// which has been typed by the user in a debugging session. When a complete
// string is received, call this function to perform processing and actions on
// the string.
// ---------------------------------------------------------------------------
void HandleUserString(String &theString)
{
  // Check to see if the string was one of our supported Empeg single-
  // character command strings. If so, send it to the empeg or do other needed
  // processing on it. If not, send it to the Bluetooth chip, because
  // everything typed on the console which isn't an empeg command must by
  // elimination be a Bluetooth command. Do this by checking for each of the
  // possible supported empeg command matrix characters first, processing
  // them, and doing a  "return" statement, if one of them is hit. If none of
  // the "return" statements are hit, then whatever is left over must be a
  // Bluetooth command.

  // User can type Z by itself on the debug console to start pairing. If the
  // string is a Z and only a Z, then call the pairing routine, the same as if
  // the user had pressed the RESET/PAIR button on the BlueGigaEmpeg casing.
  if (String("z").equalsIgnoreCase(theString))
  {
    // Initiate pairing sub-mode.
    PairBluetooth();

    // Done handling string, return from this routine so we don't double-
    // process commands.
    return;
  }

  // Iterate through our empeg command matrix. This is a matrix of the single-
  // character empeg serial port commands, and also their corresponding AVRCP
  // Bluetooth chip commands. If we get a match just throw the AVRCP command
  // into our regular AVRCP processing loop which will handle everything for
  // us, including sending the corresponding correct command to the empeg
  // serial port.
  for (int i=0; i<empegCommandMatrixSize; i++)
  {
    // Make the string comparison to find out if there is a match. Match to
    // the second column in the table ("[1]") to match the empeg single-key
    // command rather than the AVRCP string. If the user typed a single
    // character (and only a single character) which matches the corresponding
    // empeg serial port command, then process it.
    if ((empegCommandMessageMatrix[i][1]).equalsIgnoreCase(theString))
    {
      // Directly trigger the empeg command-sending code with the AVRCP
      // message (column "[0]") already in the variable, to force the action
      // to trigger right away. This should also do the necessary part of
      // sending the empeg single-character key command to the empeg serial
      // port.
      HandleString(empegCommandMessageMatrix[i][0]);

      // Done handling string, return from this routine so we don't double-
      // process commands.
      return;
    }
  } 

  // If we haven't had a match yet (if none of the "return" statements above
  // have been hit), then the string must be a completely different non-empeg
  // command which was intended for the Bluetooth serial port. Send it along
  // to the Bluetooth serial port now.
  SendBlueGigaCommand(theString);
}


// ---------------------------------------------------------------------------
// HandleString
// 
// Function to process the string data received from the Bluetooth chip.
// When a complete string is received, call this function to look up and
// see if the contents of the sting match something in our table of
// commands (defined at the top of the program), If there is a match,
// send the corresponding empeg command to the empeg serial port to command
// the empeg to do the same behavior that the Bluetooth chip had just been
// commanded to do.
//
// This also handles other cases where perhaps we need to respond to some
// command that was issued to the Bluetooth chip from the host stereo, and now
// we have to respond to that command directly on the Bluetooth.
// ---------------------------------------------------------------------------
void HandleString(String &theString)
{
  // Initialize the variables we will be using in this function.
  String commandToSend;
  commandToSend = "";
  static char empegCommandToSend = 0;

  // Bugfix - If we get a "NO CARRIER" message from the Bluetooth chip, then
  // we need to set our globally stored transaction labels to blanks. This is
  // because, upon disconnection from a Bluetooth device, the transaction
  // labels become invalid and now we should not be using them any more. They
  // should be reset so that upon the next reconnection that we can receive
  // new transaction labels. This will hopefully fix an issue where if you
  // turn off the head device (the stereo, the Bluetooth headset, whatever),
  // and then turn it back on again without rebooting the BlueGiga and the
  // Arduino, then it will not start sending bogus transaction labels to the
  // head device causing it to glitch or disconnect.
  //
  // UPDATE: Reverting this supposed bugfix. This caused problems with the
  // Bluetooth headset where it would sometimes say NO CARRIER 3 for the a2DP
  // connection and then wouldn't get it play/pause notifications. By
  // disabling this it could get its play/pause notifications despite the NO
  // CARRIER signal. See Github issue #22 for more details on this issue.
  //    if (theString.indexOf(F("NO CARRIER")) > (-1))
  //    {
  //      ClearGlobalVariables();
  //    }

  // Bugfix - If we see the boot message from the BT chip, reset our globals.
  // I have seen situations where the Bluetooth chip has reset itself and we
  // see its initial boot message on the debug console again. If we see it we
  // need to clear out our variables because that means the Bluetooth chip
  // just got reset and now our global variables aren't valid any more.
  if (theString.indexOf(F("WRAP THOR AI")) > (-1))
  {
    ClearGlobalVariables();
  }  

  // If the string indicates that we are connected to the Bluetooth, then set
  // the corresponding global variable that indicates we are connected to the
  // Bluetooth.
  if (theString.indexOf(F("A2DP STREAMING START"))      > (-1)) {connected = true;}
  if (theString.indexOf(F("CONNECT 1 A2DP"))            > (-1)) {connected = true;}
  if (theString.indexOf(F("CONNECT 2 A2DP"))            > (-1)) {connected = true;}
  if (theString.indexOf(F("AUDIO ROUTE"))               > (-1)) {connected = true;}
  if (theString.indexOf(F("GET_ELEMENT_ATTRIBUTES"))    > (-1)) {connected = true;}
  if (theString.indexOf(F("GET_PLAY_STATUS"))           > (-1)) {connected = true;}
  if (theString.indexOf(F("PLAYBACK_STATUS_CHANGED"))   > (-1)) {connected = true;}
  if (theString.indexOf(F("PDU_REGISTER_NOTIFICATION")) > (-1)) {connected = true;}

  // If the string indicates that we are disconnected from the Bluetooth, then
  // set the corresponding global variable that indicates we are disconnected
  // from the Bluetooth.
  if (theString.indexOf(F("A2DP STREAMING STOP"))  > (-1)) {connected = false;}
  if (theString.indexOf(F("WRAP THOR AI"))         > (-1)) {connected = false;}
  if (theString.indexOf(F("NO CARRIER 0 ERROR"))   > (-1)) {connected = false;}
  if (theString.indexOf(F("NO CARRIER 1 ERROR"))   > (-1)) {connected = false;}

  // Attempt to work around GitHub issue #70, crackling high pitched playback,
  // by detecting the case where the problem occurs, and rebooting the WT32i
  // if it happens. I can't figure out how to prevent it altogether so we
  // will work around it. This is controlled by a global flag at the top
  // of the code, "workAroundChipmunkProblem".
  if (workAroundChipmunkProblem)
  {
    // Detect the problem, which is a string saying that its codec is working
    // at 48khz instead of the required 44.1khz. The bad string looks like
    // this, but not all pieces of the string might match exactly, so look
    // for some of the sections but not all of them:
    //      A2DP CODEC SBC JOINT_STEREO 48000 BITPOOL 2-53
    // Example of another string that might appear if a different codec is
    // being used:
    //      A2DP CODEC APT-X_LL STEREO 44100
    // So your check below needs to accomodate all possible combinations while
    // still triggering on the bad state (48khz sampling rate).
    if ( (theString.indexOf(F("A2DP CODEC ")) > (-1)) && (theString.indexOf(F(" 48000 ")) > (-1)) )
    {
      // If the problem is detected, then restart the Bluetooth.
      Log(F("Stereo connected with bad audio sampling rate of 48000. Restarting Bluetooth module."));
      QuickResetBluetooth(0);

      // This workaround for issue #70 caused a whole other problem, which
      // was that some AVRCP stuff didn't work correctly either after the
      // reboot. Experimentally trying a double reboot here to see if it
      // fixes both problems.
      QuickResetBluetooth(0);
    }
  }

  // Handle "get Bluetooth address" strings - these are strings that are
  // intended to tell me who my current pairing buddy is. This must come early
  // in the process so that if the GBA strings are found in the list, they
  // obtain the necessary address prior to any responses which might use that
  // address lower down.
  for (int i=0; i<gbaMatrixSize; i++)
  {
    // Make the string comparison to find out if there is a match.
    if (theString.indexOf(gbaMessageMatrix[i]) > (-1))
    {
      // Process the pullout of the address of our pairing buddy
      // out of the string that is expected to contain our 
      // pairing buddy's address in it.
      GrabPairAddressString(theString, gbaMessageMatrix[i]);
    }
  }

  // Handle "grab channel number" strings - These are strings that represent
  // which channel a given Bluetooth connection is using on the Bluetooth
  // chip. The things after the word LIST are the channel numbers. 
  // list
  // 0001758 LIST 3
  // 0000047 LIST 0 CONNECTED A2DP 672 0 0 374 0 0 ...
  // 0000039 LIST 1 CONNECTED A2DP 672 0 0 374 0 0 ...
  // 0000038 LIST 2 CONNECTED AVRCP 672 0 0 1 0 0  ...
  if (theString.indexOf(F("LIST ")) > (-1) && theString.indexOf(F("CONNECTED ")) > (-1) )
  {
    // Process the pullout of the address of our pairing buddy
    // out of the string that is expected to contain our 
    // pairing buddy's address in it.
    GrabChannelNumber(theString);
  }

  // Handle query/response strings for things like the track metadata and the
  // playback status information. Make sure our program responds to these
  // queries in a timely fashion if it receives them. This is a time-sensitive
  // routine that needs to respond immediately as soon as we get a query
  // string, so I have put it near the top of the string handling routine.
  // However, there is a catch-22, where one of the responses might need to be
  // the track metadata from the empeg. Since getting the track metadata from
  // the empeg might be slow, we need to be careful here and not spend "slow
  // time" trying to get the empeg track metadata every single time. This is
  // OK because the track metadata is gathered in a separate routine and
  // stored in global variables that this routine can then respond with when
  // needed.
  RespondToQueries(theString);

  // If we are inside the pairing process, and we have detected that the
  // pairing process is complete, then set the flag to indicate that the
  // pairing process is complete. The pairing process will end once the flag
  // has been set to false. The actual handling of the inputs and responses to
  // the pairing mode strings is handled a little farther down in the code.
  if (pairingMode)
  {
    if (theString.indexOf(pairDetectionString) > (-1))
    {
      Log(F("Detected: Pairing Mode is Complete."));
      pairingMode = false;
    }
  }
  
  // Iterate through our matrix of AVRCP commands looking for a match, to find
  // commands that may be translatable into empeg commands. This right here is
  // the main heart, the main reason this program exists.
  for (int i=0; i < empegCommandMatrixSize; i++)
  {
    // Make the string comparison to find out if there is a match. Look for
    // the AVRCP command (column "[0]" in the table).
    if (theString.indexOf(empegCommandMessageMatrix[i][0]) > (-1))
    {
      // A match has been found, send the command to the empeg. The
      // corresponding empeg command is stored in column "[1]" in the command
      // matrix table.
      empegCommandToSend = empegCommandMessageMatrix[i][1].charAt(0);      

      // Send the command.
      SendEmpegCommand(empegCommandToSend);

      // Special case code, part of fixing issue #32 "Fast forward can run
      // away from you and get stuck." Find out if the message we just got
      // from the Bluetooth was the beginning start initiation of a fast
      // forward or a rewind command, and if it was, set a flag to indicate
      // that we are now in the middle of doing a Bluetooth-initiated FF/REW
      // where we want protection from runaway situations. The flag being set
      // here will prevent us from canceling FF/REW when the user is doing the
      // FF/REW from the front panel in situations where it can't run away.
      // The only time that runaway protection is needed is if the Bluetooth
      // initiated it, not if the empeg front panel initiated it.
      if ( (theString.indexOf(F("AVRCP FAST_FORWARD PRESS")) > (-1)) || (theString.indexOf(F("AVRCP REWIND PRESS")) > (-1)) )
      {
        // If we have detected that Bluetooth initiated a FF or a REW, then
        // set the flag saying so, so that our protection code can trigger if
        // needed.
        blueToothFastForward = true;
      }

      // Also we must be able to cancel out of the FF/REW runaway protection
      // when user has released the FF or REW button.
      if ( (theString.indexOf(F("AVRCP FAST_FORWARD RELEASE")) > (-1)) || (theString.indexOf(F("AVRCP REWIND RELEASE")) > (-1)) )
      {
        // If we have detected the Bluetooth successfully sent a message which
        // cancels the FF or REW operation, then also cancel our runaway
        // protection code trigger.
        blueToothFastForward = false;
      }
    }
  }

  // Handle special case fix strings which are responses to certain strings we
  // receive on the Bluetooth chip that we must respond back to on the
  // Bluetooth chip. First iterate through our matrix of special case commands
  // looking for a match.
  for (int i=0; i<scFixMatrixSize; i++)
  {
    // Make the string comparison to find out if there is a match. Look for
    // the special command (column "[0]" in the table).
    if (theString.indexOf(scFixMessageMatrix[i][0]) > (-1))
    {
      // A match has been found, process the special case.
      // Prepare the command we are going to send.
      commandToSend = "";
      commandToSend += scFixMessageMatrix[i][1];

      // Substitute the Bluetooth pairing buddy string in the response
      // command, if we have it and if the command contains the token for the
      // replacement.
      if (pairAddressString != "")
      {
        if (commandToSend.indexOf(tokenSubstitutionString) > (-1))
        {
          commandToSend.replace(tokenSubstitutionString, pairAddressString);
        }
      }      

      // Send the special case fix command.
      SendBlueGigaCommand(commandToSend);
    }
  }

  // Respond to a particular AVRCP connection success message (in this case 
  // triggering off of the AUDIO ROUTE success message) with a command which
  // will tell the Bluetooth to repeatedly retry reconnections if it ever
  // becomes disconnected. This allows the empeg to reconnect to the car stereo
  // when you start the car, instead of your stereo always connecting to your
  // phone in your pocket and playing music from the phone instead. On my Honda
  // stereo, this results in the perfect combination of the phone pairing to the
  // car as a phone only (no music, just phone), and the empeg pairing as the
  // music source, simultaneously.
  //
  // This string/response pair was originally part of scFixMessageMatrix. It has
  // been removed from there and placed in here, as a "by hand" check, because
  // the reconnection string code is now only done if we're not doing the other
  // reconnect method (the monkey reconnect). So it needs a special if
  // statement here. Even though monkey reconnect is permanently disabled, I'm
  // leaving the code down here instead of in the scFixMessageMatrix because
  // it helps keep Arduino memory consumption down a bit.
  if (!monkeyReconnectEnabled)
  { 
    // Attempt to fix bug #63 where the reconnect didn't seem to occur in all
    // case. Remove the check for the "0" part of this string so that it does
    // not matter which Link ID is the one that is connected in order to
    // trigger the reconnect enabler. Original check was for the full string
    // "AUDIO ROUTE 0 A2DP LEFT RIGHT" but now checking for the two halves of
    // it on either side of the zero independently.
    if ( (theString.indexOf(F("AUDIO ROUTE ")) > (-1)) && (theString.indexOf(F(" A2DP LEFT RIGHT")) > (-1)) )
    {
      // Use the autoReconnectString defined at the top of this program.
      SendBlueGigaCommand(autoReconnectString);

      // Attempt to fix issue #28 and issue #37 at the same time. If we are
      // setting ourselves to auto reconnect to someone, then we must have
      // successfully paired and connected with someone. So now place the
      // Bluetooth chip into a mode where it is not easily discoverable. This
      // is the "Limited Dedicated Inquiry Access Code (LIAC)" value. We will
      // leave the device in this mode most of the time so that it is not
      // shown in the pairing screens of most devices unless you are in
      // pairing mode.
      Log(F("Since there is an existing pairing buddy, placing chip into limited discovery mode."));
      SendBlueGigaCommand(F("SET BT LAP 9e8b00"));        
    }
  }
    
  // Handle pairing mode strings - these are the strings that are only active
  // when we are in Reset/Pairing mode and will not be processed at any other
  // time.
  if (pairingMode)
  {
    // First iterate through our matrix of pairing commands looking for a
    // match.
    for (int i=0; i<pmMatrixSize; i++)
    {
      // Make the string comparison to find out if there is a match. Look for
      // the special command (column "[0]" in the table).
      if (theString.indexOf(pmMessageMatrix[i][0]) > (-1))
      {
        // A match has been found, process the pairing command. Process the
        // pullout of the address of our pairing buddy. This is a super
        // special case for one particular string. It retrieves the Bluetooth
        // device address of the host stereo that we are pairing to, and
        // places it into a global variable for later use in the response
        // string.
        GrabPairAddressString(theString, F("INQUIRY_PARTIAL "));
        
        // Prepare command we are going to send.
        commandToSend = "";
        commandToSend += pmMessageMatrix[i][1];

        // Substitute the Bluetooth pairing buddy string in the response
        // command, if we have it and if the command contains the token for
        // the replacement.
        if (pairAddressString != "")
        {
          if (commandToSend.indexOf(tokenSubstitutionString) > (-1))
          {
            commandToSend.replace(tokenSubstitutionString, pairAddressString);
          }
        }

        // Send the final assembled pairing response string to the Bluetooth
        // chip.
        SendBlueGigaCommand(commandToSend);
      }
    }
  }  
}


// ---------------------------------------------------------------------------
// GrabPairAddressString
//
// Grabs the Bluetooth address from a string that looks like this:
//      "INQUIRY_PARTIAL 0a:ea:ea:6a:7a:6a 240404"
// or perhaps a string that looks like this:
//      "SET BT PAIR 0a:ea:ea:6a:7a:6a ad11da9ed235df50e86eeef1e3ba8b"
// and puts it into the global variable "pairAddressString". Only does the
// first one it finds since the start of the program, or since the user has
// pressed the "reset/pair" button on the assembly. Does not save the variable
// in permanent storage, so if you need this variable after bootup you have to
// re-retrieve it with special code. We leave it up to the Bluetooth chip to
// store the pair addresses, and the Bluetooth chip automatically reconnects
// to paired devices when it is powered back on, so our code only needs to
// remember this pairing address during runtime of the program.
// 
// Parameters:
//     stringToParse: The string that will be parsed to extract the pairing
//                    address such as: 
//                           INQUIRY_PARTIAL 0a:ea:ea:6a:7a:6a 240404
//     triggerString: The portion of the string which is the trigger string
//                    which falls just before the Bluetooth address. In the
//                    string above the trigger string would be
//                    "INQUIRY_PARTIAL " including the trailing space.
// ---------------------------------------------------------------------------
void GrabPairAddressString(String stringToParse, String triggerString)
{
  // Set variables we will be using the parsing code below.
  static int firstSpace = 0;
  static int lastSpace = 0;
  firstSpace = 0;
  lastSpace = 0;

  // If we already have a pair address string, don't retrieve another one, we
  // only want to do this for the first one that we find in a run.
  if (pairAddressString != "")
  {
    return;
  }

  // Get our Bluetooth address out of the string if it's the one exact special
  // case string that we expect to see at this particular moment
  if ( stringToParse.indexOf(triggerString) > (-1)  )
  {
    // Obtain the location of the first space character which is at the end of
    // the trigger string. Start searching shortly before the end of the
    // trigger string since the trigger string I'm passing in here contains
    // the space too. For example if our trigger string is "INQUIRY_PARTIAL "
    // which is 16 characters including the space, start searching at 15.
    //
    // BUGFIX - don't assume that our trigger string is at the start. For
    // example, when the thing we're trying to fix is the bad PDU
    // registrations, then the strings coming in can get mangled by things
    // like overflowed RS-232 input buffers. In any case, I once got a bad
    // string from the Bluetooth that looked like this:
    //    AVRCP 0 PDU_REGISTER_NOTIFICATION 0 TRACK_REACHED_END 0SET BT PAIR 4e:fe:7e:5e:1e:2e 10e0f6999e06f4
    // The data I really wanted was the address out of that string, but
    // initially this code had assumed that the SET BT PAIR would be at the
    // start of the string. Fixing it to look for it in the proper place now.
    firstSpace = stringToParse.indexOf(F(" "), stringToParse.indexOf(triggerString) + triggerString.length() - 2);

    // Obtain the location of the second space, start looking for it a couple
    // characters after the first space.
    lastSpace = stringToParse.indexOf(F(" "), firstSpace + 2);

    // Obtain the substring between the two spaces
    pairAddressString = stringToParse.substring(firstSpace, lastSpace);

    // Trim the string of any possible whitespace
    pairAddressString.trim();

    // Find out if the thing we got was an address. It should contain some
    // colon characters at the very least. Throw it away if it's not a  proper
    // Bluetooth address (we will merely check for a colon).
    if (pairAddressString.indexOf(F(":")) < 1) // First colon should be at least at char position 2, but never at 0
    {
      // Clear it out, it wasn't what we wanted, set it to nothing.
      pairAddressString = "";
    }
    else
    {
      // Log what we got.
      Log (F("------------------------------------"));
      Log (F("   Pairing buddy device address:    "));
      Log (pairAddressString);
      Log (F("------------------------------------"));
    }
  }
}


// ---------------------------------------------------------------------------
// GrabChannelNumber
//
// Grabs the Bluetooth channel number from a string that looks like this:
// "LIST 2 CONNECTED AVRCP 672 0 0 235 ..."  and puts it into one of my global
// variables for keeping track of channel numbers such as "avrcpChannel".
// Parameters:
//     stringToParse: The string that will be parsed to extract the pairing
//                    address such as: 
//                           LIST 2 CONNECTED AVRCP 672 0 0 235 ...
//
// Note: At the moment this only grabs the AVRCP channel number, it can be
// expanded later to include other channel numbers if needed.
// ---------------------------------------------------------------------------
void GrabChannelNumber(String stringToParse)
{
  // Set variables we will be using the parsing code below.
  static int firstSpace = 0;
  static int lastSpace = 0;
  firstSpace = 0;
  lastSpace = 0;

  // Quick bailout if it's not the AVRCP line we're looking for. At the
  // current time I'm only doing the AVRCP channel but I can expand this later
  // to also get the first and second A2DP channels if I end up needing them.
  // Note that the routine which calls this already looked for "LIST" and
  // "CONNECTED" so I don't have to check for them here.
  if (stringToParse.indexOf(F("AVRCP")) < 0) {return;}

  // Set the value we're trying to grab to blank to start.
  avrcpChannel = "";

  // Get our channel out of the string if it's the one exact special case
  // string that we expect to see at this particular moment.
  if (stringToParse.indexOf(F("LIST ")) > (-1))
  {
    // Obtain the location of the first space character which is at the end of
    // the trigger string. Start searching shortly before the end of the
    // trigger string since the trigger string I'm passing in here contains
    // the space too. For example if our trigger string is "LIST "
    // which is 5 characters including the space, start searching at 4.
    firstSpace = stringToParse.indexOf(F(" "), 4);

    // Obtain the location of the second space, start looking for it a couple
    // characters after the first space.
    lastSpace = stringToParse.indexOf(F(" "), firstSpace + 2);

    // Obtain the substring between the two spaces
    avrcpChannel = stringToParse.substring(firstSpace, lastSpace);  

    // Trim the string of any possible whitespace
    avrcpChannel.trim();

    // Find out if the thing we got was a channel number. It should contain a
    // low single digit number at the very least. Throw it away if it's not.
    if ( (avrcpChannel != "0") && (avrcpChannel != "1") && (avrcpChannel != "2") && (avrcpChannel != "3") && (avrcpChannel != "4") ) 
    {
      // Clear it out, it wasn't what we wanted, set it to nothing.
      avrcpChannel = "";
    }
    else
    {
      // Log what we got.
      Log (F("------------------------------------"));
      Log (F("         AVRCP Channel is:"));
      Log (avrcpChannel);
      Log (F("------------------------------------"));
    }
  }
}


// ---------------------------------------------------------------------------
// Log
//
// Log a string of output to the Arduino USB debug serial port. Had to make it
// into two overloaded functions (a form of static polymorphism) calling a
// base function, so that one version of it could accept a string by reference
// as its parameter, and the other version could accept the F() macro (the 
// Flash String Helper) as its parameter. If it weren't for the fact that I
// needed to save a bit of string-handling memory by accepting it as a
// reference, then I wouldn't need the fancy overloaded version, and could
// have done this with a single function.
// ---------------------------------------------------------------------------
void Log(const String &logMessage)
{
  BaseLog(logMessage);
}

void Log(__FlashStringHelper* logMessage)
{
  String tempLogMessage = logMessage;
  BaseLog(tempLogMessage);
}

void BaseLog(const String &logMessage)
{
  // Variable used in time calculations.
  static unsigned long currentOutputLineMillis = 0;

  char timestring[10];
  int x;

  // Make sure the main serial port is available before outputting.
  if (Serial)
  {
    // Calculate the delta between the last time that we logged an output line
    // and now, so that we can profile our output.
    if (outputMillis)
    {
      // Set the value of the current output timing before printing it, so that
      // the act of printing it doesn't cloud our profiling results.
      currentOutputLineMillis = millis();

      x = sprintf(timestring, "%07d",  currentOutputLineMillis - priorOutputLineMillis);

      // Print things out including the time delta.
      Serial.print(String(timestring) + " " + logMessage);

      // Set the prior one to be the current one, ready for the next
      // calculation.
      priorOutputLineMillis = currentOutputLineMillis;
    }
    else
    {
      // Print the resulting string without time information.
      Serial.print(logMessage);
    }

    // Print a CRLF at the end of the log message to alleviate us from the
    // need of adding one by hand at the end of every logged message
    // throughout the code.
    Serial.print(F("\r\n"));
  }       
}


// ---------------------------------------------------------------------------
// LogChar
//
// Output a single character to the Arduino USB debug serial port.
// ---------------------------------------------------------------------------
void LogChar(char logChar)
{
  // Make sure the main serial port is available before outputting.
  if (Serial)
  {
    // Write the actual character
    Serial.write(logChar);
  }       
}


// ---------------------------------------------------------------------------
// SendBlueGigaCommand
//
// Send a command to the Bluetooth chip. Automatically append the required
// line ending character which is appropriate for this chip. 
// ---------------------------------------------------------------------------
void SendBlueGigaCommand(String commandString)
{
  // Make sure serial port is available to send to
  if(BlueGigaSerial)
  {
    // Log the command we are sending.
    Log("--^ " + commandString);
  }
  else
  {
    // Log the command we cannot be sending.
    Log("--^ CANNOT SEND COMMAND - NO SERIAL PORT: " + commandString);  
    return;  
  }

  // Add the line ending to the command string.
  String sendOutputString;
  sendOutputString = "";
  sendOutputString = commandString + "\r\n";

  // Send the string.
  BlueGigaSerial.print(sendOutputString); 

  // Debugging Only:
  // "Immediate Response Display" feature.
  //
  // A single line of code below does a special thing: Show responses from the
  // chip on the debug console immediately after sending a command instead of
  // waiting for the next normal run of the main interpreter loop.
  //
  // This works and it is helpful to see things like the chip immediately
  // responding with "SYNTAX ERROR" for the exact command line that we sent,
  // as opposed to seeing it appear in the console much later. For example, in
  // normal operation, at the first startup of this Arduino code, there are
  // several chip-setup commands which get sent to the Bluetooth chip at
  // startup. They are all sent quickly in sequence one after the other, and
  // normally, the code doesn't get around to displaying the chip's responses
  // to the commands until after the entire group of commands is sent. So, for
  // example, you might see this in the console:
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
  // That "SYNTAX ERROR" at the end there is the problem. It was in response
  // to one of those SET commands above, but the problem is that now you don't
  // know which one it was in response to. By enabling the line of code below,
  // it gives the code a chance to immediately display the response from the
  // Bluetooth chip so you can tell which command got the syntax error.
  //
  // The risk here is a possibility for reentrant code that might get out of
  // control. So don't leave the line enabled in final production, just enable
  // it to check something and then disable it again when you're done. The
  // other issue with enabling this command is that it slows down our output
  // response in cases where we must have the response much quicker.
  //
  // Here is the line of code:
  //   DisplayAndProcessCommands(50, true);
  //
  // Re-enable the line above if you are debugging an immediate send/response
  // issue where you have issued a bunch of commands to the Bluetooth chip in
  // sequence and you're having trouble figuring out which of the commands is
  // the one that produced the "SYNTAX ERROR" message. Make sure to disable it
  // again when you're done debugging.
}


// ---------------------------------------------------------------------------
// RespondToQueries
//
// Command the Bluetooth chipset to respond to certain queries made by the
// host stereo, such as queries for playback status and track titles.
// ---------------------------------------------------------------------------
void RespondToQueries(String &queryString)
{
  // Quick check to make sure that the string is one of the strings that we
  // can respond to in this function. Return if not, to save time.
  boolean canRespondToThisQuery = false;
  if (queryString.indexOf(F("PLAYBACK_STATUS_CHANGED"))   > (-1)) {canRespondToThisQuery = true;}
  if (queryString.indexOf(F("TRACK_CHANGED"))             > (-1)) {canRespondToThisQuery = true;}
  if (queryString.indexOf(F("GET_PLAY_STATUS"))           > (-1)) {canRespondToThisQuery = true;}
  if (queryString.indexOf(F("GET_ELEMENT_ATTRIBUTES"))    > (-1)) {canRespondToThisQuery = true;}
  if (queryString.indexOf(F("PDU_REGISTER_NOTIFICATION")) > (-1)) {canRespondToThisQuery = true;}
  if (!canRespondToThisQuery) {return;}

  // Obtain the transaction label from these queries. Some of the queries
  // we're responding to here have a transaction label, and our response must
  // contain the same transaction label in the response. Query comes in from
  // the host stereo like this:  
  //     AVRCP 0 PDU_REGISTER_NOTIFICATION 4 PLAYBACK_STATUS_CHANGED 0
  // It means:
  // - On the AVRCP channel which is currently paired in slot/channel 0
  // - The host stereo is registering a notification with us.
  // - This message carries with it a transaction label of "4" (it might be a
  //   different character than 4)
  // - Register for a notification message for playback status changing (it
  //   would like us to please send it playback status notifications)
  // - That last 0 is an unused parameter in the register_notification request
  //   according to the BlueGiga iWrap documentation.
  //
  // For these queries, you should then respond with a response that contains
  // the transaction label. Example:
  //     AVRCP NFY INTERIM 4 1 0
  // Where 4 is the transaction label, 1 is the event ID code for "Playback
  // status changed", and 0 is the code for the state of playback (in this
  // example 0 means stopped).
  // Note that the transaction label might be a hex value, I have seen it be
  // in the range of 0-9 a-f, but I haven't seen it go above f in my tests.
  //
  // Later, then if you want to send the notification to the host stereo that
  // something has changed, for example if you want to tell the host stereo
  // that the playback status changed, then you would send a command that was
  // something like this (this is handled elsewhere in the code):
  //      AVRCP NFY CHANGED 4 1 1
  // Where:
  //      AVRCP NFY   - The notification command to send to the head unit.
  //      CHANGED     - Indicate to the head unit that there was a change of
  //                    some kind.
  //      4           - The transaction label from the headunit's original
  //                    notification registration request
  //      1           - The notification message we will send contains event
  //                    ID 1, which is "PLAYBACK_STATUS_CHANGED", from a
  //                    specific table in section 6.3.3 of the AVRCP command
  //                    reference for the BlueGiga iWrap command language.
  //      1           - The sole parameter value for the
  //                    PLAYBACK_STATUS_CHANGED, with "1" indicating
  //                    "playing" and other values indicating other things.
  //

  // Set up variables.
  String transactionLabel;
  static int transactionLabelStringPosition = 0;
  static int elementsStartSelectPosition = 0;
  static int elementsEndSelectPosition = 0;
  static int numberOfElements = 0;
  static int currentElementCode = 0;

  // Variable for the AVRCP response string we will be sending up as  a
  // response in the query/response handler code. Size of string can get big
  // because it may contain long track titles as well as the preamble. In
  // fact, the string could potentially be this long if the stereo happens to
  // query the Bluetooth chip for all track metadata elements all at once and
  // each element is the maximum length that each possible element can be:
  // "AVRCP RSP 7 "                       = 12 characters, the response preamble and the number of elements.
  // "4 "9999" 5 "9999" "                 = 18 characters, the track number and total number of tracks.
  // "7 "4294967295" "                    = 15 characters, the current playback position in MS in Decimal.
  // "1 "TrackTitle255" "                 = 260 characters, the track title up to 255 characters plus ID, quotes and spaces
  // "2 "TrackArtist255" "                = 260 characters, the track artist up to 255 characters plus ID, quotes and spaces
  // "3 "TrackAlbum255" "                 = 260 characters, the track album up to 255 characters plus ID, quotes and spaces
  // "6 "TrackGenre255" "                 = 260 characters, the track genre up to 255 characters plus ID, quotes and spaces
  // Grand total largest possible string  = 1080 characters. It will never get this big unless something is crazy. 
  String queryResponseString;

  // Obtain the transaction label if any.
  // Note the space at the end of "REGISTER_NOTIFICATION ".
  // Total length of the string "REGISTER_NOTIFICATION " is 22
  transactionLabel = "";
  transactionLabelStringPosition = queryString.indexOf(F("REGISTER_NOTIFICATION "));
  if ((transactionLabelStringPosition) > (-1)) 
  {
    // Get the next few characters in the section where I expect to see the
    // transaction label.
    transactionLabel = queryString.substring(transactionLabelStringPosition + 21, transactionLabelStringPosition + 24);
    transactionLabel.trim();

    // Special case where we save off some possible global variables
    // which are the specific transaction labels for two specific
    // types of query/response strings we might be dealing with.
    if (queryString.indexOf(F("PLAYBACK_STATUS_CHANGED")) > (-1))
    {
      // If we got a "REGISTER_NOTIFICATION" for "PLAYBACK_STATUS_CHANGED"
      // then save off the global variable for the transaction label  from
      // that particular notification registration for that transaction.
      transactionLabelPlaybackStatusChanged = transactionLabel;
    }
    if (queryString.indexOf(F("TRACK_CHANGED")) > (-1))
    {
      // If we got a "REGISTER_NOTIFICATION" for "TRACK_CHANGED" then save off
      // the global variable for the transaction label from that particular
      // notification registration for that transaction.
      transactionLabelTrackChanged = transactionLabel;
    }

    // Special case to work around bug which hung the headunit stereo
    // intermittently. Sometimes the stereo would attempt to do
    // PDU_REGISTER_NOTIFICATION for certain types of events which we are not
    // going to be using. Example request comes in from the host stereo like
    // this:    
    //   AVRCP 0 PDU_REGISTER_NOTIFICATION 4 TRACK_REACHED_END 0
    // We should not have gotten a registration for that because I had already
    // earlier responded to "GET_CAPABILITIES 3" with a response indicating I
    // only wanted to register messages 1 and 2 (TRACK_CHANGED and
    // PLAYBACK_STATUS_CHANGED) so why is it sending me registrations for
    // these other types of codes? It must be a bug, the only question is
    // whose bug. (Turns out it was mine for using the wrong parameters to
    // the auto reconnect command.)
    //
    // Initially I tried to respond to each of these, but there is a problem
    // where they are not all correctly documented in the BlueGiga
    // documentation, and so many attempts to respond to some of these result
    // in SYNTAX ERROR. An example of a response might look like:
    //      AVRCP NFY INTERIM 4 3 0
    // Where 4 is the transaction label, 3 is the event ID code for "Track
    // reached end", and 0 is the parameter. Though that one works, there are
    // others which do not work and get SYNTAX ERROR. In particular, I am
    // unable to respond to the messages NOW_PLAYING_CHANGED and
    // AVAILABLE_PLAYERS_CHANGED.
    //
    // I have tried many different syntaxes and parameters and all result in
    // SYNTAX ERROR. Examples:
    //
    //    AVRCP 0 PDU_REGISTER_NOTIFICATION e NOW_PLAYING_CHANGED 0
    //    --^ AVRCP NFY INTERIM e 9 0
    //    SYNTAX ERROR
    //
    //    AVRCP 0 PDU_REGISTER_NOTIFICATION d AVAILABLE_PLAYERS_CHANGED 0
    //    --^ AVRCP NFY INTERIM d 10
    //    SYNTAX ERROR
    //
    // Silicon Labs tech support ticket is here:
    //       https://siliconlabs.force.com/5001M000017Jb5W
    //
    // For now: Don't respond: Simply reset the chip when it hits one of
    // these things. This works around the problem on my Honda stereo
    // because it stops asking for these messages to be registered on the
    // SECOND connection after startup (it only asks on the first connection
    // after the startup). It shouldn't ever be asking for these messages at
    // all because my response to "GET_CAPABILITIES 3" should have told it
    // that I only want registrations for messages 1 and 2.
    //
    // WARNING: This crazy messed up workaround has the potential to put
    // this whole thing into an infinite reboot loop, but it's the best I've
    // got at the moment. Hopefully it will only reboot the one time and
    // then work correctly the second time. If you get an infinite reboot
    // loop, set the variable reconnectIfBadRegistrationReceived at the top
    // of the program to "false", though that means you have bigger problems
    // because your host stereo is not paying attention to you when you told
    // it only register for messages 1 and 2. Your host stereo will likely
    // hang and not get track titles or be able to send steering wheel
    // control commands to the player.
    if (reconnectIfBadRegistrationReceived)
    {
      if (queryString.indexOf(F("TRACK_REACHED_END"))                  > (-1)) { Log(F("WARNING: Bad registration message received.")); ForceQuickReconnect(); return;}
      if (queryString.indexOf(F("TRACK_REACHED_START"))                > (-1)) { Log(F("WARNING: Bad registration message received.")); ForceQuickReconnect(); return;}
      if (queryString.indexOf(F("PLAYBACK_POSITION_CHANGED"))          > (-1)) { Log(F("WARNING: Bad registration message received.")); ForceQuickReconnect(); return;}
      if (queryString.indexOf(F("BATT_STATUS_CHANGED"))                > (-1)) { Log(F("WARNING: Bad registration message received.")); ForceQuickReconnect(); return;}
      if (queryString.indexOf(F("SYSTEM_STATUS_CHANGED"))              > (-1)) { Log(F("WARNING: Bad registration message received.")); ForceQuickReconnect(); return;}
      if (queryString.indexOf(F("PLAYER_APPLICATION_SETTING_CHANGED")) > (-1)) { Log(F("WARNING: Bad registration message received.")); ForceQuickReconnect(); return;}
      if (queryString.indexOf(F("NOW_PLAYING_CHANGED"))                > (-1)) { Log(F("WARNING: Bad registration message received.")); ForceQuickReconnect(); return;}
      if (queryString.indexOf(F("AVAILABLE_PLAYERS_CHANGED"))          > (-1)) { Log(F("WARNING: Bad registration message received.")); ForceQuickReconnect(); return;}
      if (queryString.indexOf(F("ADDRESSED_PLAYERS_CHANGED"))          > (-1)) { Log(F("WARNING: Bad registration message received.")); ForceQuickReconnect(); return;}
      if (queryString.indexOf(F("UIDS_CHANGED"))                       > (-1)) { Log(F("WARNING: Bad registration message received.")); ForceQuickReconnect(); return;}
      if (queryString.indexOf(F("VOLUME_CHANGED"))                     > (-1)) { Log(F("WARNING: Bad registration message received.")); ForceQuickReconnect(); return;}
    }
  }

  // Process PLAYBACK_STATUS_CHANGED if that is the query we received. Example
  // response might look like: "AVRCP NFY INTERIM 4 1 1"
  if (queryString.indexOf(F("PLAYBACK_STATUS_CHANGED")) > (-1))
  {
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
    // At the moment we only have playing or paused available as states on the
    // empeg, I don't know about "stopped", the empeg might not have a state
    // for that at all. Not yet implementing the "fast forwarding" and
    // "rewinding" response states, as the FF/REW functions seem to work fine
    // without it in the one instance I tested it (on a Kenwood car stereo).
    // If we find a need to implement those other states we can work on it
    // later.
    if (empegIsPlaying)
    {
      queryResponseString += F(" 1");
    }
    else
    {
      queryResponseString += F(" 2");
    }

    // Response string is assembled, send it:
    SendBlueGigaCommand(queryResponseString);
    return;
  }

  // Process TRACK_CHANGED if that is the query we received. Example response
  // might look like: "AVRCP NFY INTERIM 4 2 1"
  if (queryString.indexOf(F("TRACK_CHANGED")) > (-1))
  {
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

    // Response string is assembled, send it:
    SendBlueGigaCommand(queryResponseString);
    return;
  }

  // Respond to GET_PLAY_STATUS request. 
  // Example response might look like: "AVRCP RSP 4294967295 4294967295 1"
  if (queryString.indexOf(F("GET_PLAY_STATUS")) > (-1))
  {
    queryResponseString = "";

    // Answer with an "AVRCP RSP" command.
    queryResponseString += F("AVRCP RSP ");
    
    // First two parameters are the length and position (in that order) of the
    // track in milliseconds. You can respond with 0xffffffff (which I think
    // is 4294967295) in those spots to indicate song length reporting is not
    // supported. IMPORTANT: When responding to GET_PLAY_STATUS you do not put
    // your timestamp surrounded by quote characters, whereas when responding
    // to GET_ELEMENT_ATTRIBUTES (done elsewhere), you do need to surround
    // your timestamps with quotes.    
    queryResponseString += trackPlaybackLengthString;
    queryResponseString += " ";
    queryResponseString += trackPlaybackPositionString07;

     // And the play status which is one of:
    //        0 - Stopped
    //        1 - Playing
    //        2 - Paused
    //        3 - Fast forwarding
    //        4 - Rewinding
    // At the moment we only have playing or paused available as states on the
    // empeg, I don't know about "stopped", the empeg might not have a state
    // for that at all. Not yet implementing the "fast forwarding" and
    // "rewinding" response states, as the FF/REW functions seem to work fine
    // without it in the one instance I tested it (on a Kenwood car stereo).
    // If we find a need to implement those other states we can work on it
    // later.
    if (empegIsPlaying)
    {
      queryResponseString += F(" 1");
    }
    else
    {
      queryResponseString += F(" 2");
    }

    // Response string is assembled, send it:
    SendBlueGigaCommand(queryResponseString);
    return;
  }

  // Respond to GET_ELEMENT_ATTRIBUTES request. 
  // Example query might look like: "GET_ELEMENT_ATTRIBUTES 01 03" 
  // Example response might look like: "AVRCP RSP 1 3 \"Album on empeg Car\""
  if (queryString.indexOf(F("GET_ELEMENT_ATTRIBUTES ")) > (-1))
  {
    // Zero out our response string to start
    queryResponseString = "";

    // Need to parse out the numbers after GET_ELEMENT_ATTRIBUTES which
    // indicate number of elements and which element. For example:
    // "GET_ELEMENT_ATTRIBUTES 01 03"
    // means, get one element, of element id "3" which is album title.
    //
    // Special processing: When the host stereo queries for track titles and
    // other metadata, it can do it in one of two ways:
    //    1. Separate queries for each individual piece of metadata, i.e., one
    //       query for Title, another query for Artist, another query for
    //       Genre, etc.
    //    2. A single query for multiple combinations of the metadata, such as
    //       querying for all of them, or a subset of them, all at once with a
    //       single query statement.
    // There is support in the Bluetooth command set for both of those
    // methods, the code below will respond successfully to both of them.
    //
    // Note: The attribute parameters need to be surrounded by quote
    // characters. The code below is where I bake them in. Theoretically, by
    // the time the metadata strings reach this point, I have already stripped
    // quote characters (if any) in the track metadata, so that when I bake
    // the quotes into the responses below, then there is not a parsing
    // problem on the host end.

    // Obtain the first numeric group from the string to determine how many
    // elements we have to return in our response. For example in
    // "GET_ELEMENT_ATTRIBUTES 02 04 05" we are looking for the "02" which
    // would indicate that we need to return two elements.
    //
    // First find the start position, which is the end of the
    // "GET_ELEMENT_ATTRIBUTES " including the space, which is 23 characters:
    elementsStartSelectPosition = queryString.indexOf(F("GET_ELEMENT_ATTRIBUTES ")) + 23;

    // Then find the end position which is the next space.
    elementsEndSelectPosition =  queryString.indexOf(F(" "), elementsStartSelectPosition);

    // If it's not found, we have a parsing error so bail out.
    if (elementsEndSelectPosition <= 23) {return;}

    // Turn it into an integer variable:
    numberOfElements = queryString.substring(elementsStartSelectPosition, elementsEndSelectPosition).toInt();

    // If the number is an unexpected size, we have a parsing error so bail
    // out. We only know how to parse elements numbered 1 through 7 so 7 is
    // the max.
    if (numberOfElements < 1) {return;}
    if (numberOfElements > 7) {return;}

    // Start building the response string.
    queryResponseString += F("AVRCP RSP ");

    // Add the number of elements to the response string
    queryResponseString += String(numberOfElements);

    // Loop through all elements and query for each element ID and build a
    // completed response string out of them.
    for (int i=1; i<=numberOfElements; i++)
    {
      // Zero out the element code so that each time through the loop we know
      // that we will be getting a proper element code since the value must be
      // in the range of 1 through 7 if we parsed it correctly.
      currentElementCode = 0;

      // Start by finding the prior spot in the string where we had finished
      // our selection and interpretation of the number. If this is the first
      // time through the loop, this will be the string position immediately
      // following the space after the number of elements. If this is a
      // subsequent time through the loop, then this will be the string
      // position immediately following the space after the prior element code
      // number.
      elementsStartSelectPosition = elementsEndSelectPosition + 1;

      // Now find the next trailing space if there is one. 
      elementsEndSelectPosition =  queryString.indexOf(F(" "), elementsStartSelectPosition);

      // Check to see if we even found a space at all.
      if (elementsEndSelectPosition <= elementsStartSelectPosition)
      {
        // Parse the current element code out to the end of the string because
        // we didn't find a space
        currentElementCode = queryString.substring(elementsStartSelectPosition).toInt();
      }
      else
      {
        // Parse the current element code out to the position of the space we
        // found because we found a space
        currentElementCode = queryString.substring(elementsStartSelectPosition, elementsEndSelectPosition).toInt();
      }

      // Add the element code to the response string with spaces around it.
      queryResponseString += " " + String(currentElementCode) + " ";

      // Add the text of the element itself to the response string, based on
      // the current codenumber for which elements were queried for.
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

    // Response string is assembled, send it:
    SendBlueGigaCommand(queryResponseString);
    return;
  }

    // This turns out to be a very useful debugging message, don't delete it.
    // It helped me discover an important bug at one point. If this message
    // appears it means I did something wrong, or perhaps there is a new
    // Bluetooth message I don't know how to handle.
    Log(F("Dropping out of the bottom of the RespondToQueries function without responding."));
}


// ---------------------------------------------------------------------------
// BounceAvrcpChannel
//
// Quickly disconnect and reconnect only the AVRCP channel of a connection.
// This is an attempt to work around issue #60 to get rid of the bad PDU
// registration messages on my Honda stereo.
//
// UPDATE: This did not fix issue #60. The result was that, despite the AVRCP
// connection successfully disconnecting and reconnecting and doing everything
// I designed this routine to do, it didn't fix it. What happened after the
// reconnection was that all AVRCP communication just halted at that point.
// Subsequent disco/recos didn't improve it. So this is an abandoned fix.
// Keeping the code around for posterity because it has some good insights.
// ---------------------------------------------------------------------------
void BounceAvrcpChannel()
{
  // Check to see if we're already trying to bounce AVRCP. If we are then
  // don't try to do it twice (protect against reentrant code).
  if (bounceAvrcpMode)
  {
    Log(F("Already in the process of trying to bounce the AVRCP channel. Not doing it again."));
    return;
  }

  // Set the flag indicating we are about to try bouncing the AVRCP channel.
  bounceAvrcpMode = true;

  // Log what we are about to do.
  Log(F("Preparing to quick-bounce the AVRCP connection to fix an issue with AVRCP messages."));

  // Clear out our variable for the AVRCP channel so we can grab it fresh and
  // be sure it's fresh once we've grabbed it.
  avrcpChannel = "";

  // Send the command to the Bluetooth that we'd like to see a list of
  // channels so that other parts of my code will then automatically grab the
  // channel number from the resulting output. Then freewheel a bit to allow
  // the system to find that AVRCP channel number from the list. Freewheel
  // can be kind of long in case there are other bad PDU registrations in
  // the serial queue, let those spool out.
  SendBlueGigaCommand(F("LIST"));
  DisplayAndProcessCommands(600, false);

  // Make sure we have a pairing buddy address in memory. Send the command to
  // go find who our pairing buddy is. Other code in my main interpreter loop
  // will automatically extract that value for us. Make  sure to freewheel
  // long enough to allow the main interpreter loop to grab the value.
  SendBlueGigaCommand(F("SET BT PAIR"));
  DisplayAndProcessCommands(600, false);

  // If we have both of the values, then send the commands to bounce the AVRCP
  // channel. Otherwise. I can't do it, so fall back to a full reconnect.
  if ((avrcpChannel != "") && (pairAddressString != ""))
  {
    Log(F("Quick-bouncing the AVRCP connection now."));

    // Close only the AVRCP channel while leaving the A2DP channels open so
    // that the user does not notice a dropout in the audio while we do this.
    SendBlueGigaCommand("CLOSE " + avrcpChannel);

    // The CLOSE command will be immediately followed by a NO CARRIER message
    // which, if we don't swallow the response, will cause the player to 
    // become paused by other automatic code elsewhere. So swallow the pause
    // message to fix that.
    DisplayAndSwallowResponses(2, 300);

    // Freewheel a bit to make sure the channel is really closed.
    DisplayAndProcessCommands(600, false);

    // Reopen another AVRCP channel, hopefully this one will be better than
    // the last one.
    SendBlueGigaCommand("CALL " + pairAddressString + " 17 AVRCP");
  }
  else
  {
    // If the above didn't work, then fall back to our old brute force method
    // of fixing the issue: A full reconnect. This will blip the audio.
    Log(F("Unable to quick-bounce the AVRCP connection as requested, forcing reconnect instead."));
    ForceQuickReconnect();
  }

  // Clear the flag that protected against reentrant code.
  bounceAvrcpMode = false;
}


// ---------------------------------------------------------------------------
// ForceQuickReconnect
//
// Quickly disconnect and reconnect to the currently-connected host stereo to
// work around an issue. The issue is that my car stereo starts asking
// questions that I don't know how to answer, and when this happens, I can
// always solve the problem by disconnecting and reconnecting. The questions
// that it is asking is that it's doing PDU_REGISTER_NOTIFICATION for events
// which are not well documented in the BlueGiga iWrap command language docs.
// Any response I attempt to give for these event registrations results in a
// SYNTAX ERROR from iWrap. I have found that these only occur on the first
// connection after powerup, and that if I disconnect and reconnect then after
// that I stop being asked for these registrations.
//
// Note 1: This routine assumes that the Bluetooth chip is connected to the
// host stereo at the time that this function is called.
//
// Note 2: This workaround has the potential to put the unit into an infinite
// reconnect loop if the host stereo doesn't stop asking the unanswerable
// questions after the reconnect. My stereo doesn't do that, but others might.
// There is a variable at the top of this code to turn the infinite reconnect
// loop off, but if you run into this issue then you've got other problems.
// The variable to disable it is called reconnectIfBadRegistrationReceived.
// ---------------------------------------------------------------------------
void ForceQuickReconnect()
{
  // Check to see if we're already trying to force a reconnect. If we are then
  // don't try to do it twice (protect against reentrant code).
  if (forceQuickReconnectMode)
  {
    Log(F("Already in the process of trying to force a quick reconnect. Not doing it again."));
    return;
  }

  // Set the flag indicating we are about to try a forced reconnect.
  forceQuickReconnectMode = true;

  // Make sure Bluetooth chip configured and ready to immediately reconnect as
  // soon as it disconnects. But only if we are not doing the "manual" monkey
  // reconnect feature.
  if (!monkeyReconnectEnabled)
  {
    SendBlueGigaCommand(autoReconnectString);

    // Attempt to fix issue #28 and issue #37 at the same time. If we are
    // setting ourselves to auto reconnect to someone, then we must have
    // successfully paired and connected with someone. So now place the
    // Bluetooth chip into a mode where it is not easily discoverable. This
    // is the "Limited Dedicated Inquiry Access Code (LIAC)" value. We will
    // leave the device in this mode most of the time so that it is not
    // shown in the pairing screens of most devices unless you are in
    // pairing mode.
    Log(F("ForceQuickReconnect mode. Assuming that there is a pairing buddy since we are connected. Placing chip into limited discovery mode."));
    SendBlueGigaCommand(F("SET BT LAP 9e8b00"));  
  }
  
  // UPDATE: After a lot of work for doing a truly quick reconnect version,
  // and for all the bugs it exposed and that I subsequently had to fix, and
  // finally when I got the thing working with expected behavior... ... ... It
  // didn't fix the issue. The quick reconnect does not properly solve the
  // problem. What happens is that when the stereo reconnects, though it is a
  // successful quick reconnect, it comes up in a mode where all of the AVRCP
  // commands have stopped working altogether and there are no more track
  // title updates or steering wheel control commands. So we have to just go
  // back to our original plan of forcing a full reset of the Bluetooth module
  // in this case.
  //
  Log(F("ForceQuickReconnect: Resetting/rebooting Bluetooth module."));
  QuickResetBluetooth(0);

  // Indented below is all of the code from the speedy version of the
  // reconnect. Keeping it around for posterity and because it offers some
  // insights.

                // // First, let us obtain the address of the current pairing buddy. This
                // // point in the code assumes we HAVE a current pairing buddy. Send
                // // the command to the Bluetooth to have it report back our current
                // // pairing buddy's address.
                // SendBlueGigaCommand(F("SET BT PAIR"));

                // // Allow unit to freewheel a bit to give it a chance to retrieve
                // // the pairing buddy response from the Bluetooth chip.
                // // It must be a pretty long freewheel because, when the bad PDU
                // // issue occurs, it's usually in the middle of a whole bunch of
                // // other stuff happening and many messages are going back and
                // // forth. So we must freewheel a lot here so that we have a 
                // // fighting chance of getting the stuff we need to get out of
                // // this. Also make sure to use the "false" parameter so that
                // // if there are multiple messages coming through quickly at the
                // // same time, it doesn't accidentally quit looking for the
                // // address just because a different message came through an
                // // instant before the pairing buddy's address came through.
                // DisplayAndProcessCommands(1000, false); 

                // // At this point we should have the pairing buddy's address in
                // // the global variable... we hope. Check to see if we do.
                // if (pairAddressString != "")
                // {
                //   Log(F("ForceQuickReconnect: speedy reset."));
                //   Log(F("--------------------"));
                //   Log(F("Pair Address String:"));
                //   Log(pairAddressString);
                //   Log(F("--------------------"));

                //   // If we have a pairing buddy known, then issue the commands to
                //   // force a quick reconnect which is quicker than fully resetting
                //   // the Bluetooth chip.
                //   SendBlueGigaCommand(F("CLOSE 0"));

                //   // Must have a short freewheel between the close and call commands
                //   // in order for it to work efficiently. This is counterintuitive
                //   // but the reconnect can fail or work too slowly if you issue the
                //   // call command instantaneously after you issue the close command.
                //   // I have found that on some devices, something like 100ms is too
                //   // short and the device ends up disconnecting completely. So I am
                //   // trying different higher values to prevent that problem.
                //   // 
                //   // Note, second parameter must be false for this particular case to
                //   // work so that there is an actual delay and that it doesn't quit
                //   // the processing as soon as it gets a single message.
                //   DisplayAndProcessCommands(500, false);   

                //   // Now reconnect using the pair address of our pairing buddy
                //   // that we retrieved above.
                //   SendBlueGigaCommand("CALL " + pairAddressString + " 19 A2DP");
                // }
                // else
                // {
                //   // If we don't have a pairing buddy (or if the routine to obtain
                //   // our pairing buddy's address failed) then we have no choice but
                //   // an actual reset of our Bluetooth chip. Disconnect the Bluetooth
                //   // and count on the reconnect configuration to do its job and
                //   // reconnect automatically. This is not the greatest because the
                //   // reconnect after the reset is longer than I'd like it to be.
                //   // Too much of a hanging silence. So hope the above code works
                //   // instead.
                //   Log(F("ForceQuickReconnect: HARD RESET."));
                //   QuickResetBluetooth(0);
                // }

  // Done with our routine, set flag to indicate we are done.
  forceQuickReconnectMode = false;

  // Debug logging.
  Log(F("End: ForceQuickReconnect routine."));
}


// ---------------------------------------------------------------------------
// DisplayAndSwallowResponses
//
// For either a set amount of time, and/or a set amount of response messages,
// whichever occurs first, display all responses received from the Bluetooth
// chip onto the Arduino debug port and then do nothing about them: do not try
// to process the strings or execute any commands for that period of time.
//
// Parameters:
// - The number of possible response messages to swallow.
// - The number of milliseconds to wait for each of those possible responses.
// 
// Note:
// The max total wait time will be the first parameter times the second
// parameter, if no messages are received. If any message is received, then
// the wait shortens by that portion of the wait loop.
// ---------------------------------------------------------------------------
void DisplayAndSwallowResponses(int numResponsesToSwallow, unsigned long waitTimePerResponseMs)
{ 
  // Character to be read from the serial port and then discarded
  static char swallowChar = 0;

  // Variable to be used for measuring how long each loop was waited for in
  // milliseconds
  unsigned long startingDnsMillis = 0;

  // Variable that will gather the string we'll be logging but ignoring.
  String btSwallowInputString;

  // Clean out our line by line display string at the moment we enter this
  // function.
  btSwallowInputString = "";

  // Wait for response characters.
  for (int m=0; m < numResponsesToSwallow; m++) // swallow up to this many individually-lined messages.
  {
    // Obtain the current time on the clock where we are beginning a wait to
    // swallow responses.
    startingDnsMillis = millis();

    // Loop and retrieve characters as long as the clock hasn't run out
    do
    {
      // Clear out the character that we will be retrieving.
      swallowChar = 0;

      // Check to see if a character is available on the serial port and then
      // swallow it.
      if (BlueGigaSerial)
      {
        if (BlueGigaSerial.available())
        {
          // Blink the LED to indicate Bluetooth serial port activity.
          BlinkBlue(true);

          // Read the data from the Bluetooth serial port.
          swallowChar = BlueGigaSerial.read();

          // Log the data to the serial port if not in line-by-line mode.
          if(!logLineByLine)
          {
            LogChar(swallowChar);
          }

          // Add our character to our longer line-by-line logging string.
          btSwallowInputString += swallowChar;

          // Finish blinking the LED when we are done reading the character.
          // NOTE: Doing this early here, to make sure to unblink the LED
          // before we have a chance to break out of the timed loop. This is
          // to prevent cases where this routine might leave the LED "stuck
          // on" for longer that anticipated.
          BlinkBlue(false);

          // Check to see if it's a linefeed or if the length of the string is
          // too large to hold, either way we consider the string
          if (swallowChar == '\n' || (btSwallowInputString.length() >= (btSwallowInputStringMaxLength - 2)))
          {
            if (logLineByLine)
            {
              // Trim line endings off the logged string
              btSwallowInputString.trim();

              // Log with a label that this string is being ignored.
              Log("  IGNORED: " + btSwallowInputString);
            }

            // Clean out our string for the next round of logging the next string.
            btSwallowInputString = "";

            // If the character was a line ending, then break out of just the
            // inner timed loop and move on to the outer loop (the next
            // individually-lined message to wait for).
            break;
          }
        }
      }
    }
    while ((millis() - startingDnsMillis) <= waitTimePerResponseMs);
    // Each loop waits for a certain amount of time. Current milliseconds on
    // the clock, minus the timestamp that we started each loop, gives us how
    // long we've been in the loop so far. Check to see if it's still less
    // than our overall wait time, and if not, finish the loop.
  }
}


// ---------------------------------------------------------------------------
// SetTrackMetaData
//
// Set the metadata for the currently playing track in memory so that it can
// be responded to as soon as the host queries for this information.
//
// Parameters:
//
// stringToParse: The string to parse from the empeg, such as "Red Barchetta"
//                or "Rush" or "Progressive Rock" or "0".
//
// empegMessageCode: The single character of the message code from the
//                   empeg which preceded the string to parse such as "G" or
//                   "T" or whatever.
//
// Track data coming from the empeg will be like this by the time it reaches
// this routine (the single letter is the empegMessageCode and the string
// following the letter is the stringToParse):
//
//  N  <tracknumber, actually playlist position, starts at zero>
//  Z  <album name, added by Mark Lord in recent hijack versions>
//  D  <track duration in milliseconds, added in recent hijack versions>
//  T  <track title>
//  A  <artist>
//  G  <genre>  
// ---------------------------------------------------------------------------
void SetTrackMetaData(char empegMessageCode, String &stringToParse)  // Pass by reference to work around memory bug #25.
{
  // Pre-strip doublequote characters out of the track data because our
  // messages to the Bluetooth chip need doublequote characters as delimiters
  // when we send metadata up to the host stereo. Silicon Labs support says
  // there is no more elegant way to do this: Replacing doublequote with two
  // single quotes is the best way so far. It actually looks really good on my
  // car stereo touchscreen display, so I am sticking with this for now.
  stringToParse.replace(F("\""), F("''"));

  // Pre-strip single quote characters too, I believe I ran into a problem
  // with this track title by The Police:
  //     Hungry for You (J'aurais Toujours Faim de Toi)
  // UPDATE: This turns out to not actually be required at all. The issue I
  // had with that particular song title was a serial buffer problem and
  // unrelated to the single quote. Do not do this.
  //         stringToParse.replace(F("'"), F("`"));

  // Make sure string is trimmed of white space and line breaks UPDATE: Not
  // trimming since there was an earlier step which does this already before
  // even calling this routine.
  // stringToParse.trim();

  // Now set the desired metadata based on the values we passed in. This
  // routine only gets one line at a time so we only need to process one of
  // them each time and then return.

  // Track Number. Actually this isn't really the track number, it's the
  // playlist position in the current running order, but hey, we work with
  // what we've got.
  if (empegMessageCode == 'N')
  {
    // Track number from the serial port is a zero-based index so the first
    // track in a playlist is reported as "0" on the serial port. This
    // disagrees with the empeg display screen which shows "1" for the first
    // track in a playlist. so increase this number by 1 each time we parse
    // it.
    trackNumberString04 = String(stringToParse.toInt()+1);  // Set value increased by 1.
    if (displayTracksOnSerial)
    {
      LogChar(' ');
      LogChar(' ');
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

  // Track Duration in milliseconds, but presented to us as a string. Added by
  // Mark Lord in a recent version of Hijack. We don't need to do math on
  // this, we just need to turn it around and send it right back up the
  // Bluetooth as a string as-is.
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
      LogChar(' ');
      LogChar(' ');
      Log(F("---------------------------------------------------"));
    }
    return;
  }
}


// ---------------------------------------------------------------------------
// ReplaceHighAsciiWithUtf8
// 
// Fix track metadata strings which contain High ASCII characters and replace
// them with UTF-8 characters.
// 
// My empeg's track/artist/album fields contained High ASCII characters such
// as the "Ö" in "Blue Öyster Cult". These appear as a "unrecognized symbol"
// on my car stereo's LCD touchscreen UI. This function translates those
// symbols into UTF-8 characters, which are then accepted and displayed
// correctly on the car stereo's LCD touchscreen interface.
//
// NOTE: If your empeg's track/artist/album fields are already in UTF-8, then
// disable this function by changing this global variable at the top of this
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
// ---------------------------------------------------------------------------
String ReplaceHighAsciiWithUtf8(String &stringToMakeUtf8Char)
{
  // Check to see if we are even supposed to perform this conversion at all.
  // Look at the global variable, and if it's false, return the same string as
  // input, with no conversion.
  if (PerformUtf8Conversion == false)
  {
    return stringToMakeUtf8Char;
  }

  // Set a static variable to keep track of the character we are pulling out
  // of the string to analyze. Do this as a "byte" instead of a "char" to make
  // our calculations look more straightforward, because a "byte" is unsigned
  // 0-255 while a "char" is signed -127 to +127.
  static byte oneStringChar = 0;

  // Create string that we will be using to return from this function. Make
  // sure it starts out empty.
  String utf8ReturnString = "";

  // Loop through each character in the string and convert it to UTF-8, adding
  // it back into the return string as we go.
  for (int loopVar=0; loopVar < stringToMakeUtf8Char.length(); loopVar++)
  {
    // Get the character found at that position of the input string.
    oneStringChar = stringToMakeUtf8Char.charAt(loopVar);

    // Low ASCII values, i.e. 0-127, are the same in UTF-8 and ASCII.
    if ((oneStringChar >= 0) && (oneStringChar <= 127))
    {
      // If the character is not high ASCII, then just add the character
      // straight back into the return string with no changes.
      utf8ReturnString += char(oneStringChar);
    }
    
    // High ASCII values 128 through 191 are the same as ASCII but preceded by
    // a hex C2 byte.
    if ((oneStringChar >= 128) && (oneStringChar <= 191))
    {
      // Insert a hex C2 byte before inserting the same character as the
      // original input string in this position.
      utf8ReturnString += char(0xC2);
      utf8ReturnString += char(oneStringChar);
    }
    
    // High ASCII values 192 through 255 are preceded by a hex C3 byte and the
    // character value is different by a fixed amount.
    if ((oneStringChar >= 192) && (oneStringChar <= 255))
    {
      // Insert a hex C3 byte first.
      utf8ReturnString += char(0xC3);
      
      // Modify the original character to be 64 (0x40) less than it was before
      // (0xC0 becomes 0x80 for example).
      utf8ReturnString += char(oneStringChar - 64);
    }
  }

  // Return our return string
  return utf8ReturnString;
}


// ---------------------------------------------------------------------------
// SendEmpegCommand
//
// Send a command to the empeg serial port to make the empeg do a desired
// behavior such as N\n for next track, P\n for previous track, W\n for pause,
// etc.
//
// Parameters:
// - The command char to send, without the linefeed. Linefeed will be added.
// ---------------------------------------------------------------------------
void SendEmpegCommand(char empegCommandToSend)
{
  // Partial fix for issue #57 - Flickering play/pause icons because of double
  // issuance of play/pause state reporting to head unit due to timestamp
  // changing after getting S0 or S1 from the player. When we issue a W or C
  // or whatnot command to the player, it will respond with its play state
  // being paused or playing by saying S0 or S1 on the serial port. But then
  // shortly thereafter another timestamp appears on the serial port so it
  // then triggers my code which detects that play state has gone pausey or
  // unpausey based on the timestamp. We need to mute that code for Just One
  // More Timestamp when we issue a W or a C or a whatnot to the player so
  // that the first timestamp coming out of the player after a W or a C or a
  // whatnot doesn't cause the play/pause icon on the car stereo touchscreen
  // to flicker back and forth between play and pause.
  empegFirstTimestamp=true;

  // Set the empeg's state variable based on the command we will send. Ideally
  // we would always let the empeg tell us what state it's in and never change
  // this state ourselves. However there is a special case which needs this.
  // It is the case when the player state quickly switches from play to pause
  // or vice versa and the host stereo queries for the play status. Sometimes
  // the host stereo queries for the status only once and it does it too
  // quickly before the empeg has a chance to tell us what its state is. So
  // the response to the host stereo's playback status query can be the wrong
  // value. so when we send a pause or a play to the empeg we quickly set is
  // state here so that we can respond correctly to the query when it comes.
  if (empegCommandToSend == 'W')
  {
    // Only change the empeg player state if the empeg player is listening.
    if (empegPlayerIsRunning)
    {
      // Set the empeg state variable to "paused"
      empegIsPlaying = false;
    }
  }
  if (empegCommandToSend == 'C')
  {
    // Only change the empeg player state if the empeg player is listening.
    if (empegPlayerIsRunning)
    {
      // Set the empeg state variable to "playing"
      empegIsPlaying = true;
    }
  }

  // Only bother to send commands if we believe the empeg has finished booting
  // up and we think we will be successful at sending the command. If not, log
  // the failure.
  if (!empegPlayerIsRunning)
  {
    Log("Unable to send command to empeg, the empeg player application is not yet running: " + (String)empegCommandToSend);
  }
  else
  {
    // Log what we are sending.
    Log("--^ Sending to empeg: " + (String)empegCommandToSend);

    // Write out the corresponding empeg command to the empeg's
    // serial port followed by a linefeed character.
    if (EmpegSerial)
    {
      EmpegSerial.print(empegCommandToSend);
      EmpegSerial.write('\n');
    }

    // Handle and log whatever state changes we did AFTER we sent the command
    // (or not) to the empeg player.
    //     HandleEmpegStateChange(1);
    // Bugfix: Actually don't this merely because we told the player to change
    // tracks. Instead only trigger a state change notification when the
    // player responds with new/changed information about its state, i.e.,
    // handle the empeg state change elsewhere. Merely telling the empeg state
    // to change by sending it a command doesn't mean that it actually changed
    // yet. Notifying the host stereo that a change occurred at this moment is
    // actually too early, and it will query for and receive false data at
    // this too-early moment, and will waste precious time sending and
    // receiving all the wrong track metadata, slowing down the receipt of the
    // correct metadata a few moments later. Commenting out the call to
    // HandleEmpegStatChange here fixes GitHub issue #38, "Track number
    // changes before the other metadata.".
  }
}


// ---------------------------------------------------------------------------
// HandleEmpegString
// 
// Function to process the string data received from the empeg car serial
// port. Locate strings which look like notifications of track data and turn
// those into variables for later sending up the Bluetooth.
// ---------------------------------------------------------------------------
void HandleEmpegString(String &theString)
{
  // Set up variables for this function.
  String empegDetailString = "";
  String empegTimecodeString = "";
  static char empegMessageCode = ' ';
  static int empegMessageCodeStartPos = 0;
  static int empegDetailStringStartPos = 0;
  static int empegHours = 0;
  static int empegMinutes = 0;
  static int empegSeconds = 0;
  static long empegMs = 0;
  static bool foundEmpegMessageCode = false;

  // Lists of characters we will be searching for as our empeg message code
  // strings. The track metadata codes look like this, and they always appear
  // in a group with the Genre at the end as the last one in the group.
  //
  // UPDATE: With later Hijack versions and a "suppress_notify=2" in its
  // config.ini, these have been significantly shortened to remove the
  // "serial_notify_thread.cpp", the number and the colon, so the strings
  // start at the @@ signs.
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

  // In addition there will also be a few other messages from the serial port
  // we care about. We will keep these as part of a larger list for general
  // pre-check processing to make sure we found any codes at all in the list.
  //
  // serial_notify_thread.cpp: 170:@@ S1    <single digit code for track playback status - 0=paused 1=playing>
  // serial_notify_thread.cpp: 180:@@ #4830  0:00:00   <timestamp of current playback position preceded by FID number>
  // serial_notify_thread.cpp: 180:@@ #4830  0:00:01
  // serial_notify_thread.cpp: ???:@@ R<number of tracks in current running order, added by Mark Lord in recent hijack versions>
  //                                     Note that R will appear intermittently, separate from all of the above  
  static char empegMessageCodeList[9]  = { 'N', 'Z', 'D', 'T', 'A', 'G', 'S', '#', 'R', };
  static int empegMessageCodeListLen = 9;

  // Start this routine by setting our static variables to blank state for
  // later retrieval.
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

  // If the string indicates that the empeg player application is just booting
  // up, then turn off our ability to send things to it so that we don't
  // confuse our playing state too much. Trigger on a few different strings
  // found in bootup.
  if (theString.startsWith(F("empeg-car bootstrap")))
  {
    Log(F("--------------------------------------"));
    Log(F("empeg player boot process has started."));
    Log(F("--------------------------------------"));
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

  // If the string is our string which indicates the empeg player has
  // started...
  if (theString.startsWith(F("Prolux 4 empeg car ")))
  {
    // If the empeg player application is running, set our global variable to
    // indicate that it is running so that we do the correct behavior and
    // detect the correct states
    if (!empegPlayerIsRunning)
    {
      Log(F("-------------------------------------"));
      Log(F("empeg player application has started."));
      Log(F("-------------------------------------"));
      empegPlayerIsRunning = true;
    }

    // Do not allow empeg to play silently if there is no Bluetooth connected.
    // If we get an empeg startup message, indicating that its boot sequence
    // is done and the player app has started, then send a pause command to
    // the player. This helps prevent too much lost time in songs at the
    // startup of your car. Without this feature, in situations where the
    // Bluetooth was not connected/paired/streaming yet, then the empeg would
    // sit there silently playing all your tunes to nobody.
    //
    // Currently using the Prolux Visuals message as the way of finding the
    // startup of the empeg player because the "Starting player" message is a
    // little iffy and not always detected by this routine. Not sure why but
    // it didn't always work when I tried it.
    //
    // Also here: Fix issue #42, base certain behaviors off of whether or not
    // the Bluetooth seems to be connected.
    if (connected)
    {
      Log(F("Detected empeg bootup sequence completion, but we are already connected to Bluetooth. Not pausing player."));
    }
    else
    {
      Log(F("Detected empeg bootup sequence completion, sending pause command to player."));
      SendEmpegCommand('W');
    }
    
  }
  
  // Look for the string position of our trigger phrase in the text of the
  // string we received from the empeg. Example of string we might see would
  // be:
  //     "  serial_notify_thread.cpp: 170:@@ S0"
  empegMessageCodeStartPos = theString.indexOf(F("@@ "));

  // Check to see if the trigger phrase was found.
  if (!(empegMessageCodeStartPos > -1))
  {
    // If the string from the empeg doesn't contain our trigger phrase, then
    // ignore it and return from our subroutine.
    return;
  }
  else
  {
    // Any receipt of the trigger phrase message means that the player
    // application has started (maybe we missed the first player startup
    // message or something) so in this case we also want to set our global
    // variable to indicate that the player application is running so that we
    // do the correct behavior and detect the correct states.
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
    // This will also work if we are using a recent version of Mark Lord's
    // Hijack which deliberately shortens these strings down before we get
    // them, so that the string above might have started as "@@ S0" too. This
    // should work for both formats.

    // At this point it possibly still contains a carriage return or line feed
    // at the end, so perform a trim on it (in-place trim of string) to clean
    // whitespace and CR/LFs.
    empegDetailString.trim();

    // Now obtain the message code as a single character following the space
    // at the end of the trigger phrase. It is the first character of what we
    // have remaining. For instance, in: "  serial_notify_thread.cpp: 170:@@ S0"
    // the message code is "S"
    empegMessageCode = empegDetailString.charAt(0);
    if (empegMessageCode == ' ')
    {
      // If no string was retrieved then there is something wrong and we do
      // not have a coded string. Perhaps it was a mangled piece of data with
      // our trigger phrase at the end of the string. Bail out of the routine.
      return;
    }

    // Precheck to make sure that we really got one of the message codes that
    // we were hoping for. It should match one of the strings in this list if
    // it is.
    for (int m=0; m < empegMessageCodeListLen; m++)
    {
      // Check to see if the code is in our list of acceptable codes
      if (empegMessageCode == empegMessageCodeList[m])
      {
        // Code was found, flag it, log it, and break out of the FOR loop
        // completely because we don't need to check any more.
        foundEmpegMessageCode = true;
        break;
      }
    }

    // If we didn't find a code we were looking for, then we can bail out of
    // this routine right now.
    if (!foundEmpegMessageCode)
    {
      return;
    }

    // And then now that we're done getting a good message code, let's shorten
    // the detail string even more by stripping out the message code and
    // leaving just the parameter data after the message code. The "substring"
    // function has a zero-based index, so setting the start parameter of 1
    // means starting at the second character. And leaving the ending
    // parameter blank means go to the end of the string. So in this case, if
    // the detail string got here as, say, "ZCelery Stalks at Midnight" then
    // the resulting string would now be "Celery Stalks at Midnight".
    empegDetailString = empegDetailString.substring(1); 
  }

  // If we received any track metadata from the empeg, specifically, if it's
  // one of the ones that comes all in a group at once, such as the track
  // title and artist and such, set that data.
  for (int m=0; m < empegMessageCodeTrackDataListLen; m++)
  {
    if (empegMessageCode == empegMessageCodeTrackDataList[m])
    {
      // Found one. Call the routine to parse and set the track metadata that
      // all comes in one group all at once, such as track title and artist
      // and such.
      SetTrackMetaData(empegMessageCode, empegDetailString);

      // NOTE: The next line is super important. Every time the empeg changes
      // its track metadata, we need to send a command to the Bluetooth to
      // make it re-query us for the track information. In this way, if the
      // track changes on the empeg due to a state change then it will ask us
      // for the new track data. This will need to happen each time the empeg
      // changes track number, metadata, playback length, etc (though not
      // playback position). This will need to happen both when the Bluetooth
      // initiates the change (ie the user presses NEXT on his steering wheel)
      // and also when the empeg moves on to the next track naturally itself
      // (i.e. you're just letting the empeg run and it goes to the next track
      // and now new track data has appeared on its serial port output).
      // However, we should call out to the this function only when the empeg
      // itself has told us that the data has changed, not merely because we
      // sent an "n" or "p" command to the empeg. There is a serial data
      // transmit/receive cost and a processing cost associated with the
      // (fairly large) set of queries that it makes when it gets this
      // message, so you should only do it when absolutely necessary.
      //
      // Trigger this only when we get the line containing a "Genre" change
      // ("G") since that is the last of the track information sets to appear
      // on the serial port in a block of messages. This prevents us from
      // trying to make the large set of queries occur for every single line
      // of metadata.
      if (empegMessageCode == 'G')
      {
        HandleEmpegStateChange(2);
      }

      // Bugfix for issue #32 "Fast forward can run away from you and get
      // stuck". If we are in the middle of a fast forward or rewind operation
      // that was initiated from the Bluetooth, prevent a "runaway fast
      // forward" situation by canceling the operation and resetting it here
      // and now. Only do this if the fast forward or rewind operation was
      // initiated by the Bluetooth, not if it was initiated from the player's
      // front panel. This is important, so that we prevent runaway Bluetooth
      // situations by stopping at track boundaries but still allow the user
      // to ff/rew across track boundaries from the player front panel. First,
      // check to see if we were in the middle of a Bluetooth-initiated FF or
      // REW operation and thus need to be doing protection at all.
      if (blueToothFastForward)
      {
        // We are now in a situation where a new track boundary has been
        // crossed (because we got new track data just now) and now we know we
        // were also in the middle of a FF/REW operation initiated by the
        // Bluetooth, so begin implementing the protection code now. Start by
        // turning off the flag variable since we're handling the situation
        // now.
        blueToothFastForward = false;

        // Send a command to the empeg Car player to stop/cancel the fast
        // forward or rewind operation that we're in the middle of, since we
        // have just hit a track boundary and we need to stop it before we
        // cross too many track boundaries and the operation runs away from us
        // and never stops due to  dropped serial port data.
        SendEmpegCommand('A');
      }

      // If it was one of these pieces of data (like a track title or such)
      // then we are done and can return from this routine to save time.
      return;
    }
  }

  // If we received the information from the empeg serial port indicating that
  // it is now playing, then do the needful
  if ((empegMessageCode == 'S') && (empegDetailString == "1"))
  {
    empegIsPlaying = true;
    HandleEmpegStateChange(1);
    return;
  }
  
  // If we received the information from the empeg serial port indicating that
  // it is now paused, then do the needful
  if ((empegMessageCode == 'S') && (empegDetailString == "0"))
  {
    empegIsPlaying = false;
    HandleEmpegStateChange(1);
    return;
  }

  // Total tracks in current playlist is not output on the empeg serial by
  // default but with a new version of Mark Lord's Hijack then we can now
  // parse this information sometimes. Handle that here because it will arrive
  // at intermittent times due to the fact that Hijack only gets this data
  // after the disk drive spins down.
  if (empegMessageCode == 'R')
  {
    // Check to see if this string differs from what we already had in the
    // global variable, and perform different steps depending on  whether this
    // is new information (thus requiring a whole trip into the state change
    // routine) or if it's just a repeat of what we already know, thus not
    // needing the extra processing.
    if (trackTotalNumberString05 == empegDetailString  )
    {
      // Log (F("Playlist length was not new, not updating."));
    }
    else
    {
      // Assign the global playlist length variable to the value Mark so
      // kindly gave us.
      trackTotalNumberString05 = empegDetailString;  

      // Tell the head unit that we have a new playlist length to report to
      // it.
      HandleEmpegStateChange(2);
    }
  }

  // If we received information from the empeg serial port that our playback
  // position has changed, then do the needful. The playback position is
  // tricky because the line includes the FID number that we have to strip out
  // and also it's in ascii HH:MM:SS format whereas the Bluetooth chip wants
  // it in milliseconds, so we'll have to do some work here.  
  // Example of full string: "  serial_notify_thread.cpp: 180:@@ #f4f0  0:00:12"
  // By the time it reaches this code it will be:               "#f4f0  0:00:12"
  if (empegMessageCode == '#')
  {
    // Preset the hours minutes and seconds to invalid values as a sanity
    // check (will check that these are at least zero or higher at the end of
    // the parsing routine).
    empegHours = -1;
    empegMinutes = -1;
    empegSeconds = -1;

    // Split at the first space it finds. In my example above, the first space
    // is actually two spaces, but I think there is a chance that the hour
    // field might be space-padded, so plan on it being one space (or not) and
    // then whatever it finds after that. Note: The +1 is so that it gets the
    // string starting AFTER the first space, not INCLUDING the first space.
    empegTimecodeString = empegDetailString.substring(empegDetailString.indexOf(F(" ")) +1 );
    
    // Debug: Log what we are parsing, our string should look like " 0:00:12"
    // at this point, including the possibility of one leading space if the
    // hour is space-padded (not sure).
    // Log ("Parsing timecode: " + empegTimecodeString);   

    // Sanity check: Make sure there is at least one colon, otherwise bail out.
    if (!(empegTimecodeString.indexOf(F(":")) > -1))
    {
      Log(F("Error decoding timestamp string. First colon not found."));
      return;
    }    

    // Split into hour, minute, and second.
    // First get the integer of the value of the digits before the first colon
    // (the hours).
    empegHours = empegTimecodeString.substring(0, empegTimecodeString.indexOf(F(":"))).toInt();

    // Now shorten the string and get everything after the first colon.
    // String should now look like "00:12" - reduced down to minutes and seconds
    empegTimecodeString = empegTimecodeString.substring(empegTimecodeString.indexOf(F(":")) +1);

    // Now get the minutes (integer of the value of the digits before the
    // remaining colon).
    empegMinutes = empegTimecodeString.substring(0, empegTimecodeString.indexOf(F(":"))).toInt();

    // Sanity check: Make sure that last remaining colon is left there,
    // otherwise bail out.
    if (!(empegTimecodeString.indexOf(F(":")) > -1))
    {
      Log(F("Error decoding timestamp string. Second colon not found."));
      return;
    }    
    
    // Now shorten the string and get everything after the remaining colon.
    // String should now look like "12" - it's now just the seconds left.
    empegTimecodeString = empegTimecodeString.substring(empegTimecodeString.indexOf(F(":")) +1);

    // Get the decimal value of the remaining section, the seconds.
    empegSeconds = empegTimecodeString.toInt();

    // Sanity check that there are good values inside all of these. These
    // values were PRE SET TO NEGATIVE so that we could do this check right
    // here. If they reach this point negative, then something went wrong. So
    // check for negative values.
    if ((empegHours < 0) || (empegMinutes < 0) || (empegSeconds < 0))
    {
      Log(F("Parsing error when trying to parse timestamp from empeg serial output."));
      Log(F("Either the hours, seconds, or minutes was less than zero."));
      return;
    }

    // Calculate the timestamp in milliseconds.
    // Note: must append an "L" to the constant values so that they calculate
    // as longs in the final calculation instead of ints.
    empegMs = (empegHours * 1000L * 60L * 60L) + (empegMinutes * 1000L * 60L) + (empegSeconds * 1000L);

    // Fix Github issue #26 "empegStartPause feature does not work as
    // expected." The problem is that the player starts playing the song
    // after bootup no matter what. It does this even though I already sent
    // a "W" to the player after the player application started. Even though
    // that first "W" worked, there is a second unpausing which occurs a
    // little bit later. The player pauses when the first "W" is sent, and
    // then it starts playing the track just a few moments later. This seems
    // to happen at about the same time as the first timestamp appears on
    // the serial port. Fix the issue by sending yet another pause statement
    // to the empeg immediately after it sees that first timestamp after
    // bootup.
    if (empegFirstTimestampSinceBootup)
    {
      // Issue #42, base certain behaviors off of whether or not the Bluetooth
      // seems to be connected.
      if (connected)
      {
        Log(F("First timestamp after bootup detected, but we are already connected to Bluetooth. Not pausing player."));
      }
      else
      {
        Log(F("Pausing player on first timestamp after boot up."));
        SendEmpegCommand('W');
      }

      // When we are done processing this first bootup timestamp flag, then
      // clear the flag so it works normally thereafter.
      empegFirstTimestampSinceBootup = false;
    }

    // The next section of code must only be done if it was NOT the first
    // timestamp message we ever received from the empeg since either boot up
    // or since sending a command to pause, unpause, or change tracks. Because
    // the first timestamp we receive will almost always be different from
    // the timestamp in global memory, the very first one will frequently be
    // flagged as having made a pause/play-state change even if there really
    // wasn't one. So instead only do this in cases where it wasn't the first
    // timestamp. If we hit this routine and it WAS the first timestamp, then
    // flip the flag to false for the rest of the program run.
    if (empegFirstTimestamp)
    {
      // Debug log that we have entered this "first time" code.
      //    Log(F("First timestamp received from empeg Car - Not performing play-pause state change code just yet."));

      // This was the first timestamp we've seen since program bootup or since
      // issuing a W or C or whatnot, so flip the flag so that we take action
      // on the second timestamp and all subsequent ones.
      empegFirstTimestamp = false;
    }
    else
    {
      // This was NOT the first timestamp we've received from the empeg, so we
      // can take some action on the timestamp.
      //
      // Check empegMs against our global variable trackPlaybackPositionMs, so
      // that we can determine if the playback counter position has changed
      // since the last time that we received a message about it. If it is
      // different, we perform a special case fix: update the empegIsPlaying
      // flag to indicate that the empeg is playing because the track times
      // are changing. This is to work around a bug where sometimes the empeg
      // doesn't report that it has started playing again after unpausing from
      // its front panel. The bug causes the play/pause indicator on the car
      // stereo's touchscreen to display the wrong playback state, indicating
      // that the player is paused when it really isn't paused. So we will
      // work around this by updating the state variable if we see the
      // timestamps changing. There is a drawback to this, which is, if you
      // are performing FF and REW from the front panel of the player while
      // the playback is paused (this is one of the empeg's existing
      // capabilities), then the car stereo head unit will think that playback
      // state is "playing" during the time that you are FF/REWing on the
      // empeg front panel. This is OK, this is a better thing to have than
      // the incorrectly-displayed playback state during normal playback.
      // Though there is the possibility to respond with a particular playback
      // state to the headunit of "fast forwarding" or "rewinding", but this
      // is not implemented in my code yet. So for now, we have two possible
      // states, playing and paused, and if we detect that the empeg has
      // changing track timestamps then we will update that state.
      if (empegMs != trackPlaybackPositionMs)
      {
        // The empeg is considered to be playing at this point because the
        // current track playback position is different from the last one that
        // we read from the serial port. Now, check to see if the state of the
        // empeg playback has changed from what was previously expected, and
        // if so, display a message as well as flipping the playback state
        // variable. Must flip the variable first then display the message
        // that the state had changed.
        if (empegIsPlaying == false)
        {
          empegIsPlaying = true;
          HandleEmpegStateChange(1);
        }
        // If the playback state wasn't wrong to begin with, no action is
        // needed.
      }
      else
      {
        // The empeg is considered to be paused at this point because the
        // current track playback position is unchanged from the last time. Do
        // the opposite variable-flip as above.
        if (empegIsPlaying == true)
        {
          empegIsPlaying = false;
          HandleEmpegStateChange(1);
        }
        // If the playback state wasn't wrong to begin with, no action is
        // needed.
      }
    }

    // Now that we are done checking whether or not the timestamp has changed,
    // now we can update the global "holder" of the playback time to reflect
    // what we read from the serial port message. Now it is stored globally
    // and ready  to be checked against the next playback position value that
    // we receive from the empeg serial port.
    trackPlaybackPositionMs = empegMs;

    // Convert milliseconds to a string and assign it to our global playback
    // position string as a string value that can be sent to the Bluetooth
    // chip.
    trackPlaybackPositionString07 = String(empegMs);
  }
}


// ---------------------------------------------------------------------------
// HandleEmpegStateChange
//
// When the empeg's playing state changes (playing or paused), there are a few
// things that need to be done, some commands than need to be sent to the
// Bluetooth chip to let it know that the state has changed. For instance this
// function will send the AVRCP NFY CHANGED message to the head unit. This
// routine handles doing whatever is necessary when the empeg state changes.
//
// Parameter:   Supply 1 for "playback status changed" (i.e., play/pause).
//              Supply 2 for "track changed" (i.e., title/artist/etc.)
//
// NOTE: Though it is tempting to try, do not send either A2DP STREAMING STOP
// or A2DP STREAMING START in the HandleEmpegStateChange function at all. It
// causes problems during the pairing and connection processes if we do it at
// the wrong time due to the empeg state changing in the middle of other
// things.
//
// NOTE: Though it is tempting to try, do not send an AVRCP PLAY or AVRCP
// PAUSE message during HandleEmpegStateChange. It does not help to indicate
// to the host stereo that we have started playing by sending a play command.
// Since we are in A2DP Source Mode, the host stereo doesn't respond to PLAY
// or PAUSE commands. I tried to do it to fix issue #22 where the Bluetooth
// headset would get the wrong idea and it would have a desynchronization with
// the play/pause state, but it did not make it better. The real source of the
// desynchronization was a completely different thing. So don't do that here
// because it makes additional SYNTAX ERROR messages appear at times and it's
// not helpful and it's the wrong place to do it.
// ---------------------------------------------------------------------------
void HandleEmpegStateChange(int typeOfStateChange)
{
  // Partial fix for issue #57 - Flickering play/pause icons because of double
  // issuance of play/pause state reporting to head unit due to timestamp
  // changing. Immediately after notifying the head unit that there was a
  // state change (either due to track change or due to play/pause state
  // change) then there is a chance that the next timestamp message from the
  // empeg will be misinterpreted so don't do timestamp processing immediately
  // after having told the headunit that there was a state change.
  empegFirstTimestamp=true;

  if (typeOfStateChange == 1)
  {
    // Send a notification to the head unit that the "playback status
    // changed". Command details for this command are:
    //      AVRCP NFY {INTERIM | CHANGED} {transaction_label} {event_ID} [value]
    // Where:
    //      AVRCP NFY   - The notification command to send to the head unit.
    //      CHANGED     - Indicate to the head unit that there was a change of
    //                    some kind.
    //      x           - Send this message with the specific transaction label
    //                    from the corresponding previous "REGISTER_NOTIFICATION".
    //      1           - The notification message we will send contains event ID
    //                    1, which is "PLAYBACK_STATUS_CHANGED".
    //      1           - The sole parameter value for the PLAYBACK_STATUS_CHANGED,
    //                    with a "1" for "playing" and "2" for "paused"
    if (transactionLabelPlaybackStatusChanged != "")
    {
      // A transaction label for this type of transaction exists, so we can
      // send the notification message.
      if (empegIsPlaying)
      {
        // Only send the message if the playback state really changed. Don't
        // waste the bandwidth if it hasn't changed since the last time we
        // sent a status change message.
        if (priorIsPlaying != F("1"))
        {
          // If the empeg is playing, send the notification that we are
          // playing.
          SendBlueGigaCommand("AVRCP NFY CHANGED " + transactionLabelPlaybackStatusChanged + " 1 1");

          // Update our variable that indicates what the last runthrough of
          // sending PLAYBACK_STATUS_CHANGED up the Bluetooth did.
          priorIsPlaying = F("1");

          // Report to our Arduino console debug log what the current state of
          // playback on the empeg is.
          ReportEmpegPlayingState();                 
        }
        else
        {
          // Debugging message
          Log(F("Not sending PLAYBACK_STATUS_CHANGED notification with the PLAYING status because there is no state change, we already tried to send this data."));
        }
      }
      else
      { 
        // Only send the message if the playback state really changed. Don't
        // waste the bandwidth if it hasn't changed since the last time we
        // sent a status change message.
        if (priorIsPlaying != F("2"))
        {
          // If the empeg is paused, send the notification that we are paused.
          SendBlueGigaCommand("AVRCP NFY CHANGED " + transactionLabelPlaybackStatusChanged + " 1 2");
    
          // Update our variable that indicates what the last runthrough of
          // sending PLAYBACK_STATUS_CHANGED up the Bluetooth did.
          priorIsPlaying = F("2");

          // Report to our Arduino console debug log what the current state of
          // playback on the empeg is.
          ReportEmpegPlayingState();          
        }
        else
        {
          // Debugging message
          Log(F("Not sending PLAYBACK_STATUS_CHANGED notification with the PAUSED status because there is no state change, we already tried to send this data."));
        }
      }
    }
  }

  if (typeOfStateChange == 2)
  {
    // Send a notification to the host stereo headunit that the track itself
    // has changed, so that it will re-query us for the track metadata.
    // Command details for this command are:
    //      AVRCP NFY {INTERIM | CHANGED} {transaction_label} {event_ID} [value]
    // Where:
    //      AVRCP NFY   - The notification command to send to the head unit.
    //      CHANGED     - Indicate to the head unit that there was a change.
    //      x           - Specific transaction label.
    //      2           - The notification message we will send contains event
    //                    ID 2, which is "TRACK_CHANGED".
    //      1           - The sole parameter value for the TRACK_CHANGED, with
    //                    a "1" indicating that there is indeed a track selected.
    if (transactionLabelTrackChanged != "")
    {
      // A transaction label for this type of notification exists, we can send
      // the message to the host stereo.

      // But first check to see if we have already sent this exact same track
      // metadata up the Bluetooth, and if so, don't bother sending it up
      // again since that wastes bandwidth and CPU time.
      boolean dataIsTheSameThisTime = true;
      if ( priorTitleString01            !=  trackTitleString01             ) { dataIsTheSameThisTime = false;}
      if ( priorArtistString02           !=  trackArtistString02            ) { dataIsTheSameThisTime = false;}
      if ( priorAlbumString03            !=  trackAlbumString03             ) { dataIsTheSameThisTime = false;} 
      if ( priorNumberString04           !=  trackNumberString04            ) { dataIsTheSameThisTime = false;} 
      if ( priorTotalNumberString05      !=  trackTotalNumberString05       ) { dataIsTheSameThisTime = false;} 
      if ( priorGenreString06            !=  trackGenreString06             ) { dataIsTheSameThisTime = false;} 
      if ( priorPlaybackPositionString07 !=  trackPlaybackPositionString07  ) { dataIsTheSameThisTime = false;} 

      // Check to see if any of the data has changed this time round.
      if (!dataIsTheSameThisTime)
      {
        // If some/any of the data has changed, send it up the Bluetooth. We
        // don't get to control which pieces of data go up the Bluetooth. The
        // Bluetooth host head unit queries for the data it wants after we
        // notify that the track data changed.
        SendBlueGigaCommand("AVRCP NFY CHANGED " + transactionLabelTrackChanged + " 2 1"); 

        // Update our variables that indicate what the last runthrough of
        // sending TRACK_CHANGED up the Bluetooth did.
        priorTitleString01            =  trackTitleString01            ;
        priorArtistString02           =  trackArtistString02           ;
        priorAlbumString03            =  trackAlbumString03            ;
        priorNumberString04           =  trackNumberString04           ;
        priorTotalNumberString05      =  trackTotalNumberString05      ;
        priorGenreString06            =  trackGenreString06            ;
        priorPlaybackPositionString07 =  trackPlaybackPositionString07 ;         
      }
      else
      {
        // Debugging message
        Log(F("Not sending TRACK_CHANGED notification because there is no new track data, we already tried to send this data."));
      }
    }
  }
}


// ---------------------------------------------------------------------------
// ReportEmpegPlayingState
//
// Log to the console session the current believed state of the empeg whether
// it is playing or paused. Used only for logging, there is  a different place
// in the code that responds to Bluetooth queries asking for the playback
// state.
// ---------------------------------------------------------------------------
void ReportEmpegPlayingState()
{
  if (empegIsPlaying)
  {
    Log(F("Empeg state..................................PLAYING  >"));
  }
  else
  {
    Log(F("Empeg state..................................PAUSED   ||"));
  }
}


// ---------------------------------------------------------------------------
// BlinkBlue
//
// Blink the reset/pair LED to indicate some kind of activity on the BT
// module. If we are in normal runtime mode, blink the LED on briefly. If we
// are in Reset/Pair mode, where the LED is expected to already be on, then
// blink it off briefly instead.
//
// Note: You must call this routine once to start the blink and once to stop
// the blink, with the parameter changed each time. Call this routine at the
// start of your indicated activity with "true" and at the end of your
// indicated activity with "false". Use this only for very brief indicators of
// activity so that the end user doesn't confuse these blinks for the main
// reset/pair mode indication.
// 
// Parameter: true to start the blink, false to stop the blink.
//
// In actual usage, the "blink off" state is pretty much undetectable. In
// other words, when we are in reset/pair mode, the activity that occurs
// during that mode does in fact make the LED blink off briefly, the problem
// is that you don't really see it because it's so brief.
// ---------------------------------------------------------------------------
void BlinkBlue(boolean startBlink)
{
  // Decide if we are starting the blink or stopping the blink.
  if (startBlink)
  {
    // Blink on the reset/pair LED briefly during times that we are processing
    // Bluetooth data from the Bluetooth chip. However if we are in pairing
    // mode, it's opposite; blink it off briefly instead, because the LED is
    // already "on" and yet we also want to indicate activity during pairing
    // mode.
    if(pairingMode)
    {
      digitalWrite(pairLedPin, LOW);
    }
    else
    {
      digitalWrite(pairLedPin, HIGH);
    }
  }
  else
  {
    // Blink off the reset/pair LED at the end of the period where we were
    // processing Bluetooth data from the Bluetooth chip. However if we are in
    // pairing mode, it's opposite; blink it back on instead, because the LED
    // was originally already "on" and yet we also want to indicate activity
    // during pairing mode.
    if(pairingMode)
    {
      digitalWrite(pairLedPin, HIGH);
    }
    else
    {
      digitalWrite(pairLedPin, LOW);
    }
  }
}


// ---------------------------------------------------------------------------
// QuickResetBluetooth
//
// Quickly reset the Bluetooth module and also clean out any global variables
// which we should not be trying to save any more thanks to the module getting
// reset. This is an attempt to avoid a bug where desynchronization between
// the Bluetooth initialization state and the Arduino initialization state
// make things go a little wonky in the software. When issuing Bluetooth
// commands to reset the player, do not simply call "RESET" or "BOOT 0" by
// itself. Should call this routine instead.
// 
// Parameter:
// resetType       0 = Use the "RESET" command
//                 1 = Use the "BOOT 0" command
//                 2 = Use the "SET RESET" command (factory defaults)
//
// Note that "SET RESET" (factory defaults) is different from "RESET" (which
// is just a reboot). Note that this does not erase Bluetooth pairings. Also,
// I don't know if "RESET" is any different from "BOOT 0" for my purposes, but
// it's being included here just in case.
// ---------------------------------------------------------------------------
void QuickResetBluetooth(int resetType)
{
  // Convenience feature to help with issue #26. Pause the player if we do
  // any kind of reset on the Bluetooth module because all of the reset types
  // below will disconnect the Bluetooth module. Music should always be paused
  // any time that the Bluetooth module is disconnected to prevent the tracks
  // from spooling out silently to no audience.
  SendEmpegCommand('W');

  // During initial attempts to fix issue #70, though those initial fix
  // attempts did not work, there was an important thing that I discovered
  // which I would like to try here anyway. Before resetting the module, let
  // the module idle a bit before actually performing the reset. I found that
  // sometimes certain commands which configured the module wouldn't "take" if
  // the reset command came too quickly after issuing the command to the
  // module. As if the module didn't have time to process "saving" the
  // configuration change unless I gave it a bit of time to think about it.
  // Attempt to fix this by giving it a bit of time before every reset
  // attempt. Because I'm assuming a chip reset is about to occur here (i.e.,
  // "all bets are off"), don't bother to even try to process any commands
  // while we're waiting. Just blow everything off while we're waiting.
  DisplayAndSwallowResponses(4,500);

  // Perform different types of resets depending on the parameter passed in.
  switch (resetType)
  {
    case 0:
      Log(F("Performing soft reset of Bluetooth module."));
      SendBlueGigaCommand(F("RESET"));
      ClearGlobalVariables();
      DisplayAndSwallowResponses(4, 500);
      connected = false;
      break;

    case 1:
      SendBlueGigaCommand(F("BOOT 0"));
      ClearGlobalVariables();
      DisplayAndSwallowResponses(4, 500);
      connected = false;
      break;

    case 2:
      SendBlueGigaCommand(F("SET RESET"));
      ClearGlobalVariables();
      DisplayAndSwallowResponses(6, 800);
      break;

    default:
      return;
  }
}


// ---------------------------------------------------------------------------
// ClearGlobalVariables
//
// Clear out any global variables which we should not be remembering or
// re-using if the Bluetooth chip happened to have gotten reset, either
// accidentally or deliberately.
// ---------------------------------------------------------------------------
void ClearGlobalVariables()
{
  Log(F("Clearing global variables."));
  transactionLabelPlaybackStatusChanged = "";
  transactionLabelTrackChanged = "";

  // Woops- bugfix. Don't clear this one out at all ever. Pairing mode,
  // regular mode, any mode... Basically any time I clear this out I get some
  // kind of bug. This isn't the right place to clear this var.
  // if (!pairingMode)
  // {
  //   pairAddressString = "";
  // }

  priorTitleString01            =  "" ;
  priorArtistString02           =  "" ;
  priorAlbumString03            =  "" ;
  priorNumberString04           =  "" ;
  priorTotalNumberString05      =  "" ;
  priorGenreString06            =  "" ;
  priorPlaybackPositionString07 =  "" ; 
  priorIsPlaying                =  "0";
}

