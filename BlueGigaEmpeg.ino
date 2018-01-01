// ----------------------------------------------------------------------------
// BlueGigaEmpeg
// by Tony Fabris
// ----------------------------------------------------------------------------
// A project to use a Silicon Labs BlueGiga WT32i bluetooth chip, combined with
// an Arduino Mega board with an added RS232 port, to act as an intermediary
// between the empeg Car stereo and a modern car. 
//
// Please refer to the accompanying README.txt file for details about the
// electronics interface, and how to modify and configure your empeg Car to
// work with this code.

// ------------------------------------
// Global variables and definitions
// ------------------------------------

// String to control the type of bluetooth authentication that you want to use.
// 
// Uncomment this line for "just works" pairing with no PIN code,
// with a fallback to the PIN code if the "just works" mode fails.
// Use this one if you are having trouble with the default security
// scheme below.
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

// Variable to control whether or not to do the Reset Line startup code
// to support Mark Lord's module which needs it. Enable this if you have implemented
// the reset line in hardware (see README.txt for details of the implementation).
boolean performResetLinePhysical = true;

// Control whether or not the module uses digital I2S audio, or analog line-level
// audio inputs (for example the line level inputs on the BlueGiga dev board).
boolean digitalAudio = true;

// Debugging tool for the part of the code that sends commands to the empeg.
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

// Choose whether or not to display the empeg Serial Port outputs (for instance
// the empeg boot up messages) on the serial debug console of the Arduino. Only
// needed for debugging, not needed for final build runtime.
//     Setting true:
//           - Characters received from the empeg Car serial cable are displayed
//             on the Arduino main serial port debugging console terminal screen.
//             It is mixed in with all the other serial output from the bluetooth
//             chip all at the same time, so it might be messy if the bluetooth
//             chip and the empeg are saying things at the same time.
//     Setting false:
//           - No echoing of the empeg Car serial cable output occurs on the debug
//             screen (though the empeg output is still interpreted and acted upon
//             by the software, it's just not displayed on the debug screen).
// This should be set to false most of the time, and only set to true during
// debugging sessions, since it slows down processing to put out too much
// data on the serial port when it is not needed.
boolean displayEmpegSerial=false;

// Variable to control whether output from the empeg and bluetooth serial
// ports are logged to the debug console character-by-character or line-by-line.
// Here's the tradeoff: If you log character-by-character you have a clearer
// picture of exactly when various messages were received from each of those
// devices in relation to each other, but the debug console gets hard to read.
// For instance, if logging character-by-character, the output will look like:
//     Bluetooth sends "GOODBYE" 
//     Empeg sends "hello" a microsecond later
//     Debug console output looks like this:
//        GhOeOlDlBoYE
// However if you log line-by-line, it's more readable but you might get the
// wrong idea of who sent what first. It might look like this:
//     hello
//     GOODBYE
// Even though GOODBYE was the first one to start sending characters, he ended
// his transmission last so he gets logged last. This usually isn't a big deal.
// Usage:
//      Setting true:
//           - Output from the empeg and the bluetooth are logged a line
//             at a time, for additional readability, but order may be wrong.
//      Setting false:
//           - Output from the empeg and the bluetooth are logged a character
//             at a time, which is less readable but slightly better timing.
//
// Special note: This isn't fully implemented for every single possible line
// of output on the debug port. There are still a couple places in the code
// where it still logs character by character. If this becomes a problem 
// then I will address it at that time later. In the meantime, this handles
// most of the situations where we'd want to see it log line by line.
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
boolean displayTracksOnSerial=true;

// Strings to define which codecs to use for A2DP audio transmission.
// 
// Uncomment this string to use default SBC codec.
const String codecString="SET CONTROL CODEC SBC JOINT_STEREO 44100 0";
//
// Uncomment this string to use Apple AAC codec and fall back to
// the default SBC codec if the AAC codec is not available. This 
// didn't work correctly on my Honda stereo despite the AAC codec
// being supported. It put my stereo into a mode where no sound
// came out and the track titles kept getting re-queried once per
// second, causing odd behavior on the host stereo. You might be
// able to use this successfully, but I wasn't able to use it.
//   const String codecString="SET CONTROL CODEC AAC JOINT_STEREO 44100 0\r\n            SET CONTROL CODEC SBC JOINT_STEREO 44100 1";
//
// Uncomment this string to use APT-X codec. Requires special firmware download
// and the purchase of a special license for APT-X codec from Silicon Labs.
//   const String codecString="SET CONTROL CODEC APT-X JOINT_STEREO 44100 0\r\n            SET CONTROL CODEC SBC JOINT_STEREO 44100 1";

// Optional - When we see the empeg player application start up, then send a
// pause command to the player. If the bluetooth initial connection speed is faster
// than the empeg bootup speed, then this will mean things start up in pause mode.
// However if the empeg finishes bootup and starts its player application before
// the bluetooth connection process is complete, then the act of connecting the
// the bluetooth will issue the commands to start playback, so things will start
// up playing. The reason for this feature to exist is to reduce the amount of 
// time that the empeg is playing "silently" with no bluetooth device listeneing
// to what it is playing. For example, in a situation where you start the car
// and it takes 25 seconds for the car stereo to boot up and connect to the
// bluetooth, while the empeg only takes about 8 seconds to boot up, then you
// will have skipped about 17 seconds of the song you were listening to when
// you turned the car off. With this feature, theoretically, you will not lose
// much if any of that time and the empeg will resume playing more or less where
// you left off. (In theory, when it works.) The drawback is, if you are pairing
// with a stereo which pairs up quickly, connecting faster than the empeg completing
// its bootup procedure, then the empeg might start in pause mode and you have to
// unpause it by hand.
// TO DO: Implement Issue #42: Create a flag that detects when bluetooth is
// successfully paired and streaming, and then skip sending the pause command
// if the bluetooth is streaming at the moment the empeg finishes booting up. If we 
// can do that successfully then this won't need to be a flag, we can just implement
// it 100 percent of the time.
boolean empegStartPause = true;

// String to tell the unit to automatically try reconnecting every few seconds when and
// if it ever becomes disconnected from its main pairing buddy. Trying to make it grab hold
// of the car stereo as soon as the car turns on, instead of connecting to my cellphone every
// time. Tried two possible versions of this, only one of which produced decent results.
//
// Version 1: "SET CONTROL RECONNECT". Getting the timing parameter just right on this
// command seems to affect whether or not the host stereo asks me for incorrect PDU messages
// and other possible bug-like issues I encountered. The string includes a baked-in second
// additional STORECONFIG command to store the configuration of the connection. The docs
// say that this STORECONFIG command is merely to store the reconnect functionality but it's
// not made clear if it stores all configuration information or just that one. NOTE: The
// docs say that the reconnect timer (the first numeric parameter) should be *longer* than
// 500ms. However I have found it should even be longer than that, to prevent other timing
// and bug issues, seemingly caused by trying to reconnect too quickly. 
const String autoReconnectString = "SET CONTROL RECONNECT 4800 0 0 7 0 A2DP A2DP AVRCP\r\n            STORECONFIG";
//
// Note: To repro the "Bad PDU Registration bug", uncomment this line (a short reconnect repros the issue frequently):
//    const String autoReconnectString = "SET CONTROL RECONNECT 800 0 0 7 0 A2DP A2DP AVRCP\r\n            STORECONFIG";
//
// Version 2: "SET CONTROL AUTOCALL". This seems to work well, when it works. But some
// of its other issues are not livable. For example sometimes it does a good job of
// reconnecting after power on, but then other times it completely fails to even attempt
// any kind of reconnection whatsoever after power on. Without rhyme or reason. 
// Note: "19" is the secret code for "A2DP" (it is the "L2CAP PSM" code for A2DP).
// const String autoReconnectString = "SET CONTROL AUTOCALL 19 501 A2DP";

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

// Arduino serial port 2 is connected to Bluetooth chip
#define BlueGigaSerial Serial2 

// Arduino serial port 1 is connected to empeg's RS-232 port via MAX232 circuit.
#define EmpegSerial Serial1

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

  // Experiment: Attempt to fix issue #45 "Initial boot and connect problem on Honda".
  // See if it's possible to make things work correctly without needing to issue a
  // "Streaming Start" command when someone presses "play". There should already have
  // been such a command issued when the "connect" happened so theoretically we shouldn't
  // have needed this at all. So try not doing this and see if it fixes other problems
  // where the head unit got confused by things.                 
  // { "AVRCP PLAY PRESS",                   "A2DP STREAMING START"},

  // Attempting to fix some issues where there is silence on the line after
  // first connection with some devices. Attempt to make sure the streaming
  // is started when there are messages that indicate we have a connection
  // of some kind. 
  { "CONNECT 2",                          "A2DP STREAMING START"},

  // This one didn't work or help or fix anything. Don't use this one.
  // { "PDU_REGISTER_NOTIFICATION",          "A2DP STREAMING START"}, 

  // We issue a "Streaming start" command (above) in certain situations,
  // because that fixes a whole bunch of issues with various cases where the
  // audio stream isn't connected when music is playing. It works. However,
  // don't do the corresponding thing for "streaming stop", because it causes
  // a problem where, when you unpause a song, there is a slight delay while
  // streaming starts up again, so, if you unpause at the 00:00 mark of a
  // song, then the beginning of the song is cut off. I am leaving this line
  // in here, commented out, to remind myself not to ever try to do this.
  //        { "AVRCP PAUSE PRESS",                  "A2DP STREAMING STOP"},

  // Respond to PIN code prompt as if we were a user saying
  // OK to that PIN code. This was needed on Mark Lord's car stereo.
  // It is expected to be needed when the pairing security scheme is
  // set for "SET BT SSP 1 0" at the top of this program. NOTE: This uses
  // the pair address substitution string in the same way that the
  // Pairing Mode strings use it. See the pmMessageMatrix for details.
  { "SSP CONFIRM ",                                 "SSP CONFIRM {0} OK"},

  // Get Capbailities 2 is asking for manufacturer ID. According to the 
  // BlueGiga documentation the chip will fill in the response details
  // for manufacturer ID automatically if you give it a blank response.
  // (By "blank response" I mean, responding with "AVRCP RSP" with no
  // further parameters following after the "RSP"). This is in the docs
  // in the file "AN986.PDF" titled "AN986: BLUETOOTH® A2DP AND AVRCP
  // PROFILES". The details are found in section 6.4.1 which states:
  //
  //       GET_CAPABILITIES 2   The Controller is querying for supported
  //                            Company IDs
  //
  //                AVRCP RSP  [id0 id1 …]
  //                           List of IrDA Company IDs. Usually, the list
  //                           can be left empty, as iWRAP will automatically
  //                           fill in the Bluetooth SIG
  //
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

  // Respond to this particular AVRCP connection success message with a command
  // that is supposed to force the bluetooth to repeatedly retry connections if it
  // ever becomes disconnected. Hopefully this will increase the chances that
  // the empeg connects to the car stereo when you start the car, instead of connecting
  // to your phone in your pocket. On my stereo, when this system works, it results
  // in the perfect combination of the phone pairing to the car as a phone only
  // (no music, just phone), and the empeg pairing as the music source, simultaneously.
  { "AUDIO ROUTE 0 A2DP LEFT RIGHT",          autoReconnectString},
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

// Variabe to control whether or not we reconnect the bluetooth module whenever
// we see a bad PDU_REGISTER_NOTIFICATION message. Ideally we want to reject any
// attempts to register any notifications that we won't be able to respond to.
// The host stereo should never ask us for anything other than messages 1 and 2
// (TRACK_CHANGED and PLAYBACK_STATUS_CHANGED) but my Honda sometimes asks for
// messages 3 and above (in the table below) when it shouldn't be. In those 
// cases, a disco/reco of the bluetooth module fixes the problem. This flag
// controls whether or not that disco/reco occurs. It's here to protect against
// a possible infinite reboot loop which might occur on someone else's stereo.
// Though if this trigger is hit at all, it means they've got bigger problems
// where the host stereo will hang anyway because it isn't getting responses
// to its registration requests. But that's a different issue than a reboot loop.
boolean reconnectIfBadRegistrationReceived=true;

// Table of events that we will cause us to reconnect when we received a PDU_REGISTER_NOTIFICATION
// request for these particular events. The event number for each one is specific and
// can be found in section 6.3.3 of the BlueGiga iWrap AVRCP command reference documentation.
// This is part of an attempt to work around a bug where we get queried for these
// particular registrations when we were not supposed to be queried for them at all.
int rejMatrixSize = 11;
String rejMatrix[11] =
{
 "TRACK_REACHED_END",                   
 "TRACK_REACHED_START",                 
 "PLAYBACK_POSITION_CHANGED",           
 "BATT_STATUS_CHANGED",                 
 "SYSTEM_STATUS_CHANGED",               
 "PLAYER_APPLICATION_SETTING_CHANGED",  
 "NOW_PLAYING_CHANGED",                 
 "AVAILABLE_PLAYERS_CHANGED",           
 "ADDRESSED_PLAYERS_CHANGED",           
 "UIDS_CHANGED",                        
 "VOLUME_CHANGED",                      
};

// Reserve variable space for the transaction status labels used to hold a
// particular string in memory in the query/response code. The host stereo
// sends a query which contains a specific transaction label, which is a
// number that is usually just a single hexadecimal digit, and it uses this
// digit as an identification so that it can send several queries and await
// several responses and can sort out which response is from which query.
// So we must respond with that same transaction label digit in our response.
// Must have specific transaction labels to use for specific types of messages.
// These are part of the fix to what I called the "Kenwood Bug" where the
// Kenwood stereo refused to query for any new track titles past the first
// track. These will also be using the memory reservation size of "2"
// defined above since they are copies of the same transaction label.
int transactionLabelSize = 2;
static String transactionLabelPlaybackStatusChanged = "";
static String transactionLabelTrackChanged = "";

// Matrix of messages and responses which are needed when in special pairing mode.
// These also include some special casing for the bluetooth device address. The
// "{0}" in these strings are a special cased token indicating that we should insert
// the bluetooth device address into that location of the response string. The "{0}"
// is also defined in another variable below.
// NOTE: Update the matrix size and the array size both, if you are changing these.
int pmMatrixSize=4;
String pmMessageMatrix[4][2] =
{
  // Respond to messages such as INQUIRY_PARTIAL 0a:ea:ea:6a:7a:6a 240404
  { "INQUIRY_PARTIAL ",          "PAIR {0}"},

  // Respond to messages such as "PAIR 0a:ea:ea:6a:7a:6a OK"
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
  //
  // NOTE BUGFIX: Don't do an AVRCP call while the host is in the middle
  // of pairing its second A2DP channel. In other words, don't do an
  // AVRCP call on the first ("0") connect from the host stereo, only 
  // do it on the second ("1") one. So comment out this line:
  //       { "CONNECT 0 A2DP 19",        "CALL {0} 17 AVRCP"},
  // But subsequent ones are OK to do, and in fact are required for 
  // AVRCP to work on the paired system.
  { "CONNECT 1 A2DP 19",        "CALL {0} 17 AVRCP"},
  { "CONNECT 2 A2DP 19",        "CALL {0} 17 AVRCP"},
};

// String to be used in token substitutions above (change both the matrix above
// and also the string below if you change the tokenization flag string).
const String tokenSubstitutionString = "{0}";

// "Get Bluetooth Address" strings (GBA).
// Set of strings which are the trigger phrases to be used for identifying
// strings from the bluetooth module that can be used for getting the address
// of our main pairing buddy at any generic time as opposed to just during
// the pairing process. This allows us to figure out who our pairing buddy
// is even when they are the ones initiating the connection as opposed to
// us inititaiting it during the pairing process. This complexity is needed
// because ForceQuickReconnect needs to know who our current pairing buddy
// is all the time as opposed to just when we recently pressed the pair button.
int gbaMatrixSize=2;
String gbaMessageMatrix[2] =
{
  "SET BT PAIR ",
  "SSP CONFIRM ",
};

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

// The translation table of AVRCP messages to empeg serial commands.
// This is the commands that we must send to the empeg serial port
// if certain messages come in from the bluetooth module.
// NOTE: Update the matrix size and the array size both, if you are changing these.
int empegCommandMatrixSize = 13;
String empegCommandMessageMatrix[13][2] =
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
// Thing is... This is taking up too much memory and the input strings need to be shorter.
// Current status: Shortened metadata max length as an austerity measure.
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

// Strings to control the audio routing on the chip. Choose which string to
// use by setting the boolean variable "digitalAudio" in the code above.
// This string is used if you are using the Line In jacks on the bluetooth
// device for audio (analog), aka "digitalAudio = false".
const String analogAudioRoutingControlString = "SET CONTROL AUDIO INTERNAL INTERNAL EVENT KEEPALIVE";  

// This string is used if you are using the I2S aka IIS connection on the
// bluetooth device (this is a special digital connection which requires
// modifying inside of empeg Car player) aka "digitalAudio = true".
const String digitalAudioRoutingControlString = "SET CONTROL AUDIO INTERNAL I2S_SLAVE EVENT KEEPALIVE 16";

// Strings to set the gain level of the empeg. See bluetooth chip docs for gain level table.
// Uncomment this line if your player will be used in AC/Home mode, (1v outputs): 
// const String empegGainSettingString = "SET CONTROL GAIN E 0";
// Uncomment this line if your player will be used in DC/Car mode, such as in a sled (4v outputs):
const String empegGainSettingString = "SET CONTROL GAIN 9 0";

// Strings to hold an incoming line of serial characters from bluetooth module or empeg.
// Will be reset each time an entire message from the bluetooth module or empeg is processed.
static String btInputString = "";
int btInputStringMaxLength = 275;
static String empegInputString;
int empegInputStringMaxLength = 275;

// Some other string limits, how many string input characters will we allow from the serial
// port before it is too excessive and we should start a new string again?
int btSwallowInputStringMaxLength = 150;

// Global variables used in main "Loop" function to detect when an entire complete
// message string has been received from the bluetooth module or empeg. Will be set to
// true when a line termination character is received indicating that the string is complete.
boolean btStringComplete = false;
boolean empegStringComplete = false;

// Variable to keep track of what the current empeg Playing state is reported to be.
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

// Variable for the pin number of the Arduino pin that is connected to the bluetooth chip's
// reset pin.
const int resetLinePin = 51;

// Variable to globally keep track of whether we have recently initiated reset/pairing mode.
bool pairingMode = false;

// Variable to globally keep track of whether we have recently initiated a reset and should
// therefore not be trying to reset yet again in the same breath. Protect against reentrant code.
bool forceQuickReconnectMode = false;

// Variable to globally keep track of whether we are in the middle of a
// fast forward or rewind operation that was initiated from the bluetooth.
// This is part of the fix to issue #32, "Fast forward can run away from
// you and get stuck."
bool blueToothFastForward = false;

// Variable to keep track of the timestamp of the prior output line.
// Used to calculate deltas between output lines for profiling.
unsigned long priorOutputLineMillis = 0;

// Include the version number file.
#include "Version.h"

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
  // Initialize our variable which keeps track of the time deltas between
  // output lines for profiling, so I can see how many milliseconds have
  // elapsed between each piece of output. Start it at the timestamp when
  // the program starts.
  priorOutputLineMillis = millis();

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

  // Set up the reset line pin (if it is implemented) to be in a clean "read"
  // state most of the time and only be set to write mode when we're using it.
  // Do this globally once at startup regardless of whether or not we are
  // calling ResetBluetoothPin() immediately afterward, because the position
  // of that call might move or change in the future. So do this on its own.
  if (performResetLinePhysical)
  {
    pinMode(resetLinePin, INPUT);
  }

  // Reserve bytes for the input strings, which are the strings we check to
  // see, for example, if there is a valid processable bluetooth AVRCP command
  // incoming from the bluetooth chip arriving from its serial port.
  btInputString.reserve(btInputStringMaxLength);
  empegInputString.reserve(empegInputStringMaxLength);

  // Reserve bytes for the some other strings to save memory.
  pairAddressString.reserve(pairAddressStringMaxLength);
  transactionLabelPlaybackStatusChanged.reserve(transactionLabelSize);
  transactionLabelTrackChanged.reserve(transactionLabelSize);

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
  
  // Set the data rate for the serial port connected to the empeg Car's
  // serial port, via the RS-232 and MAX232 circuit connected to the Arduino.
  // The empeg Car defaults to 115200 BPS in home/AC mode, and has configurable
  // BPS when sled-docked in car/DC mode. Refer to the accompanying README.txt
  // for more details about the RS-232 connection to the empeg. Note that you will
  // experience problems controlling the empeg if it is set to anything other
  // than 115200 in car mode, so make sure you have done that step.
  EmpegSerial.begin(115200);
  Log(F("Empeg Serial has been started."));

  // Additional fix for issue #26 - Always send a pause to the empeg immediately
  // at powerup of the module so that all reset conditions including power bounce
  // have the empeg paused no matter what.
  SendEmpegCommand('W');

  // Log the version number from the version number file.
  Log("BlueGigaEmpeg Version " + String(majorVersion) + "." + String(minorVersion) + "." + String(buildNumber));

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

  // Turn on the built in Arduino LED to indicate the setup activity has begun.
  digitalWrite(LED_BUILTIN, HIGH);

  // Perform the hardware reset with the RESET line on the bluetooth chip
  // to bring the chip out of "Sleep" mode on power up (if implemented).
  ResetBluetoothPin();

  // Configre the Bluetooth device at startup, call my routine which sets all
  // data to the desired overall system defaults. This does not erase any
  // pairing information, that is left untouched.
  SetGlobalChipDefaults();

  // Reset the bluetooth chip every time the Arduino chip is started up and 
  // after we have set all of its defaults (which will still be saved after
  // the reset; this is a soft reset). Resetting it here prevents bugs where
  // the bluetooth state is desynchronized from the Arduino state.
  QuickResetBluetooth(0);      

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
  String myTxBufferSizeString;
  int myTxBufferSizeInt;
  String myRxBufferSizeString;
  int myRxBufferSizeInt;

  // Retrieve variable which was defind in compiler header file.
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
    Log(F("  Please refer to the README.txt file accompanying this sketch for details.   "));
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
  Log(F("Pausing empeg for convenience while setting global chip defaults."));
  SendEmpegCommand('W');   

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
  // I am also adding this bit:
  //     Bit 14 aka 0b0100000000000000 - "UART will be optimized for low latency"
  // This is an attempt to improve an issue where there is a visible difference between
  // the empeg visuals on the empeg VFD display and the audio coming out of the host stereo's speakers.
  // This does not solve the problem but it doesn't seem to hurt. For true low latency we'd need to 
  // buy the APT-X codec (licensing fee) and use special firmware for the Bluetooth chipset.
  SendBlueGigaCommand(F("SET CONTROL CONFIG 0000 0000 0000 5100"));

  // Configure the A2DP codec that will be used for the audio connection. This string is defined
  // at the top of this program and there are multiple options for which codec can be used.
  SendBlueGigaCommand(codecString);

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
  if (digitalAudio)
  {
    SendBlueGigaCommand(digitalAudioRoutingControlString);    
  }
  else
  {
    SendBlueGigaCommand(analogAudioRoutingControlString);    
  }
  // Turn off the mic preamp to prevent distortion on line level inputs if they are being used.
  SendBlueGigaCommand(F("SET CONTROL PREAMP 0 0"));    
  
  // Turn off the mic bias voltage (well, set it to as low as possible).
  SendBlueGigaCommand(F("SET CONTROL MICBIAS 0 0"));    
  
  // Set line level input gain on the bluetooth chip, value is set in global constant at top of
  // program. This is for situations where the line level inputs might be used.
  SendBlueGigaCommand(empegGainSettingString);    

  Log(F("Done Setting Defaults."));
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
  // Debug code only.
  // Unit test code the force quick reconnect via a
  // button press. Comment this out for normal runtime.
  // ForceQuickReconnect(); return;

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
  SendBlueGigaCommand(F("SET BT PAIR *"));  // Star is required to erase pairings

  // Erase all previous auto-reconnect settings (not included in the factory default
  // reset and must be disabled before attempting to pair).
  SendBlueGigaCommand(F("SET CONTROL AUTOCALL"));     // Blank is required to disable 
  SendBlueGigaCommand(F("SET CONTROL RECONNECT *"));  // Star is required to disable
  SendBlueGigaCommand(F("STORECONFIG"));              // Make sure SET CONTROL RECONNECT is stored

  // Make certain that the above changes are saved and "take" - RESET seems to be the sure way
  QuickResetBluetooth(0);

  // Reset device to factory defaults using "SET RESET" command (parameter 2).
  // Note that "SET RESET" (factory defaults) is different from "RESET" (which
  // is just a reboot). This does not erase pairings (that is accomplished by
  // the commands above instead).
  QuickResetBluetooth(2); // "2" means factory reset of all settings

  // Set up the device for use with the empeg car.
  SetGlobalChipDefaults();

  // The reset statement (which originally occured inside the SetGlobalChipDefaults
  // statement) is no longer part of SetGlobalChipDefaults, so we must do it here
  // as part of the pre-pairing procedure. IMPORTANT NOTE: I discovered that this
  // is a super critical part of the process. It's not enough just to do a SET RESET
  // and set the chip defaults right before pairing, it's actually necessary to
  // fully reboot the unit right before pairing, or else the pairing process will
  // not work correctly. So make sure this gets done before the pairing process.
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
        // Log to the debugging console what we saw on the port
        // but only if we are in character-by-character mode
        if (!logLineByLine)
        {
          LogChar(empegChar);
        }
      }

      // Add the character that we read to our input line string for use later.
      empegInputString += empegChar;   

      // Check to see if it's a linefeed or if the length of the
      // string is too large to hold, either way we consider the string
      // to be done, baked, and ready to do something with.
      if ((empegChar == '\n') || (empegInputString.length() >= (empegInputStringMaxLength - 2)))
      {
        // String is ready to be processed if one of the above things was true.
        empegStringComplete = true;

        // Trim extra CR LF and whitespace at the end of the string if any
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
      
      // Log to the Arduino debugging console what we saw on the bluetooth serial port
      // but only if we are in character-by-character mode
      if (!logLineByLine)
      {
        LogChar(inChar); 
      }
      
      // Add the character that we read to our input line string for use later.
      btInputString += inChar;
      
      // Check to see if it's a carriage return or if the length of the
      // string is too large to hold, either way we consider the string
      // to be done, baked, and ready to do something with.
      if ((inChar == '\n') || (btInputString.length() >= (btInputStringMaxLength - 2)))
      {
        // String is ready to be processed if one of the above things was true.
        btStringComplete = true;

        // Trim extra CR LF and whitespace at the end of the string if any
        btInputString.trim();

        // Log the string if we are in line by line logging mode.
        if (logLineByLine)
        {
          Log(btInputString);
        }
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
        // // Special Case Debug code only. Disable for final release version.
        // // Unit test code for force quick reconnect via a typed console command
        // if (String("z").equalsIgnoreCase((String)userChar))
        // {
        //   ForceQuickReconnect();
        // }

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
//    (ulong) idleTimeMs  The amount of time in approximate milliseconds to wait.
//    (bool)  waitForLineEnding   Stop waiting if a CR or LF was received.
// ----------------------------------------------------------------------------
void DisplayAndProcessCommands(unsigned long idleTimeMs, bool waitForLineEnding)
{
  // Character that will be returned from the main input/output function, which
  // processes one character per loop through the function and returns that character.
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

    // Check to see if we are supposed to bail out as soon as we get a line ending.
    if (waitForLineEnding)
    {
      if (receivedChar == '\n')
      {
        return;
      }
    }
  }
  while ((millis() - startingDnpMillis) <= idleTimeMs);
  // Continue looping: Current milliseconds on the clock minus the starting milliseconds
  // gives us the total time we've been in this loop. Check to see if our time is up.
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
void HandleString(String &theString)
{
  // Initialize the variables we will be using in this function.
  String commandToSend;
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
  //
  // UPDATE: Reverting this supposed bugfix. This caused problems with the bluetooth
  // headset where it would sometimes say NO CARRIER 3 for the a2DP connection and
  // then wouldn't get it play/pause notifications. By disabling this it could get
  // its play/pause notifications despite the NO CARRIER signal. See Github issue #22
  // for more details on this issue.
  //    if (theString.indexOf(F("NO CARRIER")) > (-1))
  //    {
  //      ClearGlobalVariables();
  //    }

  // Same bugfix but with the boot message from the chip as well. I have seen
  // situations where the bluetooth chip has reset itself and we see its
  // initial boot message on the debug console again. If we see it we need
  // to clear out our variables because that means the bluetooth chip
  // just got reset and now our global variables aren't valid any more.
  if (theString.indexOf(F("WRAP THOR AI")) > (-1))
  {
    ClearGlobalVariables();
  }  

  // Handle "get bluetooth address" strings - these are strings that are
  // intended to tell me who my current pairing buddy is. This must come early
  // in the process so that if the GBA strings are found in the list, they 
  // obtain the necessary address prior to any responses which might use that
  // address lower down. 
  for (int i=0; i<gbaMatrixSize; i++)
  {
    // Make the string comparison to find out if there is a match.
    if (theString.indexOf(gbaMessageMatrix[i]) > (-1))
    {
      // Debug logging
      // Log(F(" === Getting ready to grab pairing buddy address. === "));

      // Turn on the Arduino LED to indicate something has happened.
      digitalWrite(LED_BUILTIN, HIGH);
   
      // Process the pullout of the address of our pairing buddy
      // out of the string that is expected to contain our 
      // pairing buddy's address in it.
      GrabPairAddressString(theString, gbaMessageMatrix[i]);
           
      // Turn off the Arduino LED.
      digitalWrite(LED_BUILTIN, LOW);
    }
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
  // the pairing process is complete, then set the flag to indicate
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

      // Special case code, part of fixing issue #32 "Fast forward can run away from you and get stuck."
      // Find out if the message we just got from the bluetooth was the beginning start inititation
      // of a fast forward or a rewind command, and if it was, set a flag to indicate that we are
      // now in the middle of doing a bluetooth-initiated FF/REW where we want protection from
      // runaway situations. The flag being set here will prevent us from canceling FF/REW when the
      // user is doing the FF/REW from the front panel in situations where it can't run away. The only
      // time that runaway protection is needed is if the bluetooth initiated it, not if the empeg
      // front panel inititated it.
      if ( (theString.indexOf(F("AVRCP FAST_FORWARD PRESS")) > (-1)) || (theString.indexOf(F("AVRCP REWIND PRESS")) > (-1)) )
      {
        // If we have detected that bluetooth initiated a FF or a REW, then set the flag
        // saying so, so that our protection code can trigger if needed.
        blueToothFastForward = true;
      }

      // Also we must be able to cancel out of the FF/REW runaway protection
      // when user has released the FF or REW button
      if ( (theString.indexOf(F("AVRCP FAST_FORWARD RELEASE")) > (-1)) || (theString.indexOf(F("AVRCP REWIND RELEASE")) > (-1)) )
      {
        // If we have detected the bluetooth successfully sent a message which cancels
        // the FF or REW operation, then also cancel our runaway protection code trigger.
        blueToothFastForward = false;
      }
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
      // A match has been found, process the special case.
      
      // Turn on the Arduino LED to indicate something has happened.
      digitalWrite(LED_BUILTIN, HIGH);
   
      // Prepare the command we are going to send.
      commandToSend = "";
      commandToSend += scFixMessageMatrix[i][1];

      // Substitute the bluetooth pairing buddy string in the response command,
      // if we have it and if the command contains the token for the replacement.
      if (pairAddressString != "")
      {
        if (commandToSend.indexOf(tokenSubstitutionString) > (-1))
        {
          commandToSend.replace(tokenSubstitutionString, pairAddressString);
        }
      }      

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

        // Process the pullout of the address of our pairing buddy.
        // This is a super special case for one particular string.
        // It retrieves the bluetooth device address of the host stereo
        // that we are pairing to, and places it into a global variable
        // for later use in the response string.
        GrabPairAddressString(theString, F("INQUIRY_PARTIAL "));
        
        // Prepare command we are going to send.
        commandToSend = "";
        commandToSend += pmMessageMatrix[i][1];

        // Substitute the bluetooth pairing buddy string in the response command,
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
// "INQUIRY_PARTIAL 0a:ea:ea:6a:7a:6a 240404" or perhaps a string
// that looks like this:
// SET BT PAIR 0a:ea:ea:6a:7a:6a ad11da9ed235df50e86eeef1e3ba8b
// and puts it into the global variable "pairAddressString". Only
// does the first one it finds since the runstart of the program
// or since the user has pressed the "reset/pair" button on the
// assembly. Does not save the variable in permanent storage so
// if you need this variable after bootup you have to re-retrieve
// it with special code. We leave it up to the bluetooth chip to
// store the pair addresses, and the bluetooth chip automatically
// reconnects to paired devices when it is powered back on, so
// our code only needs to remember this pairing address during
// runtime of the program.
// 
// Parameters:
//     stringToParse: The string that will be parsed to extract
//                    the pairing address such as 
//                    INQUIRY_PARTIAL 0a:ea:ea:6a:7a:6a 240404
//     triggerString: The portion of the string which is the
//                    trigger string which falls just before the
//                    bluetooth address. In the string above the
//                    trigger string would be "INQUIRY_PARTIAL ".
// ---------------------------------------------------------------
void GrabPairAddressString(String stringToParse, String triggerString)
{
  // Set variables we will be using the parsing code below.
  static int firstSpace = 0;
  static int lastSpace = 0;
  firstSpace = 0;
  lastSpace = 0;

  // If we already have a pair address string, don't retrieve another one,
  // we only want to do this for the first one that we find in a run.
  if (pairAddressString != "")
  {
    return;
  }

  // Debug logging
  // Log(F("=== Trying to grab pair buddy's address. ==="));

  // Get our bluetooth address out of the string if it's the one exact
  // special case string that we expect to see at this particular moment 
  if ( stringToParse.indexOf(triggerString) > (-1)  )
  {
    // Obtain the location of the first space character which is at the end of the trigger string.
    // Start searching shortly before the end of the trigger string since the trigger string I'm
    // passing in here contains the space too. For example if our trigger string is
    // "INQUIRY_PARTIAL " which is 16 characters including the space, start searching at 15.
    //
    // BUGFIX - don't assume that our trigger string is at the start. When the thing we're
    // trying to fix is the bad PDU registrations then the strings coming in can get mangled
    // because I think my stereo head unit has a bug or perhaps it overflows the string buffer
    // or something. In any case I got a bad string from the bluetooth that looked like this:
    //    AVRCP 0 PDU_REGISTER_NOTIFICATION 0 TRACK_REACHED_END 0SET BT PAIR 4e:fe:7e:5e:1e:2e 10e0f6999e06f4
    // The data I really wanted was the address out of that string but initially this code 
    // had assumed that the SET BT PAIR would be at the start of the string. Fixing it to
    // look for it in the proper place now.
    firstSpace = stringToParse.indexOf(F(" "), stringToParse.indexOf(triggerString) + triggerString.length() - 2);

    // Obtain the location of the second space, start looking for it a couple characters after the first space.
    lastSpace = stringToParse.indexOf(F(" "), firstSpace + 2);

    // Obtain the substring between the two spaces
    pairAddressString = stringToParse.substring(firstSpace, lastSpace);

    // Trim the string of any possible whitespace
    pairAddressString.trim();

    // Find out if the thing we got was an address. It should contain some
    // colon characters at the very least. Throw it away if it's not a 
    // proper bluetooth address (we will merely check for a colon).
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

// ----------------------------------------------------------------------------
// Log
//
// Log a string of output to the Arduino USB debug serial port. Had to make it
// into two functions calling a base function so that one version of it could
// accept a string by reference as its parameter and the other version could
// accept the F() macro as its parameter. If it weren't for the fact that I
// wanted to save a bit of string-handling memory by accepting it as a
// reference then I wouldn't need the fancy two-functions version below.
// Not sure if jumping through these hoops is helping me much, memory-wise.
// ----------------------------------------------------------------------------
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
    // Calculate the delta between the last time that we logged an output
    // line and now so that we can profile our output.
    if (outputMillis)
    {
      // Set the value of the current output timing before printing it so that
      // the act of printing it doesn't cloud our profiling results.
      currentOutputLineMillis = millis();

      x = sprintf(timestring, "%07d",  currentOutputLineMillis - priorOutputLineMillis);

      // Print things out including the time delta.
      Serial.print(String(timestring) + " " + logMessage);

      // Set the prior one to be the current one, ready for the next calcualtion.
      priorOutputLineMillis = currentOutputLineMillis;
    }
    else
    {
      // Print the resulting string without time information.
      Serial.print(logMessage);
    }

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
  //   DisplayAndProcessCommands(50, true);
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
void RespondToQueries(String &queryString)
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
  String transactionLabel;
  static int transactionLabelStringPosition = 0;
  static int elementsStartSelectPosition = 0;
  static int elementsEndSelectPosition = 0;
  static int numberOfElements = 0;
  static int currentElementCode = 0;

  // Variable for the AVRCP response string we will be sending up as 
  // a response in the query/response handler code. Size of string can get
  // big because it may contain long track titles as well as the preamble.
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
  String queryResponseString;

  // Obtain the transaction label if any.
  // Note the space at the end of "REGISTER_NOTIFICATION "
  // Total length of the string "REGISTER_NOTIFICATION " is 22
  transactionLabel = "";
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

    // Special case to work around bug which hung the headunit stereo intermittently.
    // Sometimes the stereo would attempt to do PDU_REGISTER_NOTIFICATION for certain
    // types of events which we are not going to be using. The table of these events
    // and their ID codes is defined in global variables at the top of this program.
    // Example request comes in from the host stereo like this:
    //   AVRCP 0 PDU_REGISTER_NOTIFICATION 4 TRACK_REACHED_END 0
    // We should not have gotten a registration for that because I had already
    // earlier responded to "GET_CAPABILITIES 3" with a response indicating I only
    // wanted to register messages 1 and 2 (TRACK_CHANGED and PLAYBACK_STATUS_CHANGED)
    // so why is it sending me registrations for these other types of codes? It must
    // be a bug, the only question is whose bug.
    // 
    // Initially I tried to respond to each of these, but there is a problem where
    // they are not all correctly documented in the BlueGiga documentation, and so
    // many attempts to respond to some of these result in SYNTAX ERROR. An example
    // of a response might look like:
    //      AVRCP NFY INTERIM 4 3 0
    // Where 4 is the transaction label, 3 is the event ID code for "Track
    // reached end", and 0 is the parameter. Though that one works, there are
    // others which do not work and get SYNTAX ERROR. In particular, I am unable to
    // respond to the messages NOW_PLAYING_CHANGED and AVAILABLE_PLAYERS_CHANGED.
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
    // Working on the theory that this may be occurring in situations where my code
    // is not getting the opportunity to correctly respond to the
    // GET_CAPABILITIES 3 message correctly, though I have seen this repro when
    // I had in fact just responded to it. However there is a chance that the
    // chip might have gotten reset after the response, thus the problem. Silicon
    // Labs tech support ticket is here:
    //       https://siliconlabs.force.com/5001M000017Jb5W
    //
    if (reconnectIfBadRegistrationReceived)
    {
      for (int l=0; l<rejMatrixSize; l++)
      {  
        // Make the string comparison to find out if there is a match.
        if (queryString.indexOf(rejMatrix[l]) > (-1))
        {
          // For now: Don't respond: Simply reset the chip when it hits one
          // of these things. This works around the problem on my Honda stereo
          // because it stops asking for these messages to be registered on
          // the SECOND connection after startup (it only asks on the first
          // connection after the startup). It shouldn't ever be asking for
          // these messages at all because my response to "GET_CAPABILITIES 3"
          // should have told it that I only want registrations for messages
          // 1 and 2.
          //
          // WARNING: This crazy messed up workaround has the potential
          // to put this whole thing into an infinite reboot loop, but
          // it's the best I've got at the moment. Hopefully it will only
          // reboot the one time and then work correctly the second time.
          // If you get an infinite reboot loop, set the variable 
          // reconnectIfBadRegistrationReceived at the top of the program
          // to "false", though that means you have bigger problems because
          // your host stereo is not paying attention to you when you told
          // it only register for messages 1 and 2. Your host stereo will
          // likely hang and not get track titles or be able to send
          // steering wheel control commands to the player.
          //
          Log(F("Unexpected registration message received. Something went wrong. Forcing bluetooth reconnect."));
          ForceQuickReconnect();
          return;
        }
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
// after that I stop being asked for these registrations. 
//
// Note 1: This routine assumes that the blueooth chip is connected to the
// host stereo at the time that this function is called.
//
// Note 2: This workaround has the potential to put the unit into an infinite
// reconnect loop if the host stereo doesn't stop asking the unanswerable
// questions after the reconnect. My stereo doesn't do that, but others might.
// There is a variable at the top of this code to turn the infinite reconnect
// loop off, but if you run into this issue then you've got other problems.
// The variable to disable it is called reconnectIfBadRegistrationReceived.
// ----------------------------------------------------------------------------
void ForceQuickReconnect()
{
  // Check to see if we're already trying to force a reconnect. If we are
  // then don't try to do it twice (protect against reentrant code).
  if (forceQuickReconnectMode)
  {
    Log(F("Already in the process of trying to force a quick reconnect. Not doing it again."));
    return;
  }

  // Set the flag indicating we are about to try a forced reconnect.
  forceQuickReconnectMode = true;

  // Make sure bluetooth chip configured and ready to
  // immediately reconnect as soon as it disconnects.
  SendBlueGigaCommand(autoReconnectString);

  // UPDATE: After a lot of work for doing a truly quick reconnect version,
  // and for all the bugs it exposed and that I subsequently had to fix,
  // and finally when I got the thing working with expected behavior...
  // ... ... ... It didn't fix the issue. The quick reconnect does not
  // properly solve the problem. What happens is that when the stereo
  // reconnects, though it is a successful quick reconnect, it comes up
  // in a mode where all of the AVRCP commands have stopped working
  // altogether and there are no more track title updates or steering
  // wheel control commands. So we have to just go back to our original
  // plan of forcing a full reset of the bluetooth module in this case.
  //
  Log(F("ForceQuickReconnect: Resetting/rebooting bluetooth module."));
  QuickResetBluetooth(0);

  // Indented below is all of the code from the speedy version of the reconnect.
  // Keeping it around for posterity and because it offers some insights.

                // // First, let us obtain the address of the current pairing buddy. This
                // // point in the code assumes we HAVE a current pairing buddy. Send
                // // the command to the bluetooth to have it report back our current
                // // pairing buddy's address.
                // SendBlueGigaCommand(F("SET BT PAIR"));

                // // Allow unit to freewheel a bit to give it a chance to retreive
                // // the pairing buddy response from the bluetooth chip.
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
                //   // the bluetooth chip.
                //   SendBlueGigaCommand(F("CLOSE 0"));

                //   // Must have a short freehweel between the close and call commands
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
                //   // an actual reset of our bluetooth chip. Disconnect the bluetooth
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
void DisplayAndSwallowResponses(int numResponsesToSwallow, unsigned long waitTimePerResponseMs)
{ 
  // Character to be read from the serial port and then discarded
  static char swallowChar = 0;

  // Variable to be used for measuring how long each loop was waited for in milliseconds
  unsigned long startingDnsMillis = 0;

  // Variable that will gather the string we'll be logging but ignoring.
  String btSwallowInputString;

  // Clean out our line by line display string at the moment we enter this function.
  btSwallowInputString = "";

  // Wait for response characters.
  for (int m=0; m < numResponsesToSwallow; m++) // swallow up to this many individually-lined messages.
  {
    // Obtain the current time on the clock where we are beginning a wait to swallow responses.  
    startingDnsMillis = millis();

    // Loop and retreive characters as long as the clock hasn't run out
    do
    {
      // Clear out the character that we will be retreiving.
      swallowChar = 0;

      // Check to see if a character is available on the serial port and then swallow it.
      if (BlueGigaSerial)
      {
        if (BlueGigaSerial.available())
        {
          swallowChar = BlueGigaSerial.read();

          if(!logLineByLine)
          {
            LogChar(swallowChar);
          }

          // Add our character to our longer line-by-line logging string.
          btSwallowInputString += swallowChar;

          // Check to see if it's a linefeed or if the length of the
          // string is too large to hold, either way we consider the string
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

            // If the character was a line ending, then break out of just the inner timed loop
            // and move on to the outer loop (the next individually- lined message to wait for).
            break;
          }
        }
      }
    }
    while ((millis() - startingDnsMillis) <= waitTimePerResponseMs);
    // Each loop waits for a certain amount of time. Current milliseconds on the clock, minus
    // the timestamp that we started each loop, gives us how long we've been in the loop so far.
    // Check to see if it's still less than our overall wait time, and if not, finish the loop.
  }
}


// ----------------------------------------------------------------------------
// SetTrackMetaData
//
// Set the metadata for the currently playing track in memory so that it can
// be responded to as soon as the host queries for this information.
// 
// Parameters:
// stringToParse: The string to parse from the empeg, such as "TRed Barchetta"
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
void SetTrackMetaData(char empegMessageCode, String &stringToParse)     // TEMPORARY: Partial Bugfix for Issue #25: Pass this by reference to work around memory bug.
{
  // Debug logging for issue #25 - String became blank only after passing into this function:
  // Log(F("stringToParse:"));
  // Log(stringToParse); 
  // Log(F(":stringToParse"));

  // Pre-strip doublequote characters out of the track data because
  // our messages to the bluetooth chip need doublequote characters
  // as delimiters when we send metadata up to the host stereo.
  // Silicon Labs support says there is no more elegant way to do
  // this: Repacing doublequote with two single quotes is the best
  // way so far. It actually looks really good on my car stereo
  // touchscreen display so I am sticking with this for now.
  stringToParse.replace(F("\""), F("''"));

  // Pre-strip single quote characters too, I believe I ran into a
  // problem with this track title by The Police:
  //     Hungry for You (J'aurais Toujours Faim de Toi)
  // UPDATE: This turns out to not actually be required at all. The
  // issue I had with that particular song title was a serial buffer
  // problem and unrelated to the single quote. Do not do this.
  //         stringToParse.replace(F("'"), F("`"));

  // Make sure string is trimmed of white space and line breaks
  // UPDATE: Not trimming since there was an earlier step which
  // does this already before even calling this routine.
  // stringToParse.trim();

  // Now set the desired metadata based on the values we passed in.
  // This routine only gets one line at a time so we only need to
  // process one of them each time and then return.

  // Track Number. Actually this isn't really the track
  // number, it's the playlist position in the current
  // running order, but hey, we work with what we've got.
  if (empegMessageCode == 'N')
  {
    // Track number from the serial port is a zero-based index
    // so the first track in a playlist is reported as "0" on
    // the serial port. This disagrees with the empeg display
    // screen which shows "1" for the first track in a playlist.
    // so increase this number by 1 each time we parse it.
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
      LogChar(' ');
      LogChar(' ');
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
String ReplaceHighAsciiWithUtf8(String &stringToMakeUtf8Char)
{
  // Check to see if we are even supposed to perform this conversion at all.
  // Look at the global variable, and if it's false, return the same string
  // as input, with no conversion.
  if (PerformUtf8Conversion == false)
  {
    return stringToMakeUtf8Char;
  }

  // Debug/profiling logging.
  // Log(F("--UTF-8 Conversion Start."));

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

  // Debug/profiling logging.
  // Log(F("--UTF-8 Conversion End."));

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
    // EXPERIMENTAL: Try to make output clearer by doing the handle
    // state change AFTER having send the command (or not), below.
    //    HandleEmpegStateChange();
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
    // EXPERIMENTAL: Try to make output clearer by doing the handle
    // state change AFTER having send the command (or not), below.
    //     HandleEmpegStateChange();
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
    Log("--^ Sending to empeg: " + (String)empegCommandToSend);

    // Write out the corresponding empeg command to the empeg's
    // serial port followed by a linefeed character.
    if (EmpegSerial)
    {
      EmpegSerial.print(empegCommandToSend);
      EmpegSerial.write('\n');
    }

    // EXPERIMENTAL: Handle and log whatever state changes we did AFTER we
    // sent the command (or not) to the empeg player.
    // EXPERIMENT STEP TWO: Try not doing this at all just because we told
    // the player to change tracks. Instead let's see what happens if we only
    // trigger this when the player responds with new/changed information
    // about its state (ie do this elswhere). This fixes GitHub issue #38,
    // "Track number changes before the other metadata.". Time will tell if
    // this has other repercussions that are unwanted. Initial testing looks good.
    //     HandleEmpegStateChange();
    
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

  // Lists of characters we will be searching for as our
  // empeg message code strings. The track metadata codes
  // look like this, and they always appear in a group with
  // the Genre at the end as the last one in the group.
  //
  // UPDATE: With later Hijack versions and a "suppress_notify=2" in its config.ini,
  // these have been significantly shortened to remove the "serial_notify_thread.cpp",
  // the number and the colon, so the strings start at the @@ signs.
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

    // Behavior controlled by "empegStartPause" flag: If we get an empeg startup message,
    // indicating that its boot sequence is done and the player app has started, then send a
    // pause command to the player as an attempt to help prevent too much lost time in songs
    // at the startup of your car. 
    // 
    // Notes on this procedure: 
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
      SetTrackMetaData(empegMessageCode, empegDetailString);

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
      // Trigger this only when we get the line containing a "Genre" change ("G")
      // since that is the last of the track information sets to appear on the
      // serial port in a block of messages.
      if (empegMessageCode == 'G')
      {
        HandleEmpegStateChange();
      }

      // Bugfix for issue #32 "Fast forward can run away from you and get stuck".
      // If we are in the middle of a fast forward or rewind operation that was
      // inititated from the bluetooth, prevent a "runaway fast forward" situation
      // by canceling the operation and resetting it here and now. Only do this if
      // the fast forward or rewind operation was initiated by the bluetooth, not
      // if it was initiated from the player's front panel. This is important, so
      // that we prevent runaway bluetooth situations by stopping at track boundaries
      // but still allow the user to ff/rew across track boundaries from the player
      // front panel. First, check to see if we were in the middle of a bluetooth-
      // initiated FF or REW operation and thus need to be doing protection at all.
      if (blueToothFastForward)
      {
        // We are now in a situation where a new track boundary has been crossed
        // (because we got new track data just now) and now we know we were also
        // in the middle of a FF/REW operation initiate by the bluetooth, so begin
        // implementing the protection code now. Start by turning off the flag
        // variable since we're handling the situation now.
        blueToothFastForward = false;

        // Send a command to the empeg Car player to stop/cancel the fast forward
        // or rewind operation that we're in the middle of, since we have just hit
        // a track boundary and we need to stop it before we cross too many track
        // boundaries and the operation runs away from us and never stops due to 
        // dropped serial port data.
        SendEmpegCommand('A');
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
  // sometimes. Handle that here because it will arrive at intermittent times due
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
    // whatever it finds after that. Note: The +1 is so that it gets the string
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
      // Log that we have entered this "first time" code.
      Log(F("First timestamp received from empeg Car - Not performing play-pause state change code just yet."));

      // Special case experiment to attempt to fix Github issue #26 "empegStartPause
      // feature does not work as expected." The problem is that the player starts
      // playing the song after bootup no matter what. It does this even though I
      // already sent a "W" to the player after the player application started. Even
      // though that first "W" worked, there is a second unpausing which occurs a
      // little bit later. The player pauses when the first "W" is sent, and then it
      // starts playing the track just a few moments later. This seems to happen at
      // about the same time as the first timestamp appears on the serial port.
      // Attempt to fix the issue by sending yet another pause statement to the 
      // empeg immediately after it sees that first timestamp after bootup. This is
      // experimental.
      if (empegStartPause)
      {
        Log(F("empegStartPause feature activated. Pausing player on first timestamp after boot up."));
        SendEmpegCommand('W');
      }

      // This was the first timestamp we've seen since program bootup, so flip the
      // flag so that we take action on the second timestamp and all subsequent ones.
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
    // UPDATE: Do not send either STREAMING STOP or STREAMING START
    // in the HandleEmpegStateChange code at all. This workaround is
    // no longer needed and also it causes problems during the reset
    // and pairing processes if we do it at the wrong time due to the
    // empeg state changing in the middle of other things.
    //     SendBlueGigaCommand(F("A2DP STREAMING START"));
    
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
      // Bugfix: If we haven't received a "REGISTER_NOTIFICATION" message to begin
      // with at startup yet, then don't attempt to send an arbitrary notification with an
      // arbitrary transaction label. I suspect that this arbitrary-transaction-labeled
      // message migh be going into a queue on the BlueGiga module and then being sent up
      // to the device later and confusing some devices. In particular I am wondering if it
      // is confusing a bluetooth headset I'm testing with. The solution here is to comment
      // out this line and send nothing at all in these cases.
      //    SendBlueGigaCommand(F("AVRCP NFY CHANGED 1 1 1"));
    }
    else
    {
      SendBlueGigaCommand("AVRCP NFY CHANGED " + transactionLabelPlaybackStatusChanged + " 1 1");
    }

    // Indicate to the host stereo that we have started playing by sending a play command.
    // This attempts to fix issue #22 where the bluetooth headset would get the wrong idea
    // and it would have a desynchronization with the play/pause state. This does not make
    // things either worse or better. Trying not doing this because it makes additional
    // SYNTAX ERROR messages appear at times. Verdict: this has no effect on issue #22.
    //     SendBlueGigaCommand(F("AVRCP PLAY"));
  }
  else
  {
    // Note: Do not send "A2DP STREAMING STOP" here or else you will
    // get a "cutoff" start of the song on the next unpause. Leave
    // the stream active even when playback is stopped to prevent
    // cutoffs. In fact, to fix a bug, we actually want to send a
    // streaming start here instead.
    // UPDATE: Do not send either STREAMING STOP or STREAMING START
    // in the HandleEmpegStateChange code at all. This workaround is
    // no longer needed and also it causes problems during the reset
    // and pairing processes if we do it at the wrong time due to the
    // empeg state changing in the middle of other things.
    //     SendBlueGigaCommand(F("A2DP STREAMING START"));

    
    // Send a notification to the head unit that the playback status
    // has changed (fix Kenwood bug). Command details are above,
    // with the last "2" in this command indicating "paused".
    if (transactionLabelPlaybackStatusChanged == "")
    {
      // Bugfix: If we haven't received a "REGISTER_NOTIFICATION" message to begin
      // with at startup yet, then don't attempt to send an arbitrary notification with an
      // arbitrary transaction label. I suspect that this arbitrary-transaction-labeled
      // message migh be going into a queue on the BlueGiga module and then being sent up
      // to the device later and confusing some devices. In particular I am wondering if it
      // is confusing a bluetooth headset I'm testing with. The solution here is to comment
      // out this line and send nothing at all in these cases.
      //     SendBlueGigaCommand(F("AVRCP NFY CHANGED 1 1 2"));
    }
    else
    {
      SendBlueGigaCommand("AVRCP NFY CHANGED " + transactionLabelPlaybackStatusChanged + " 1 2");
    }

    // Indicate to the host stereo that we have stopped playing by sending a pause command.
    // This attempts to fix issue #22 where the bluetooth headset would not start playing
    // when it was supposed to and there was a desynchronization in play state between
    // headset and bluetooth module. This does not make things either worse or better.
    // Trying not doing this because it makes additional SYNTAX ERROR messages appear at
    // times. Verdict: this has no effect on issue #22.
    //    SendBlueGigaCommand("AVRCP PAUSE");
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
      // BUGFIX: If we haven't received a "REGISTER_NOTIFICATION" message to begin
      // with at startup yet, then don't attempt to send an arbitrary notification with an
      // arbitrary transaction label. I suspect that this arbitrary-transaction-labeled
      // message migh be going into a queue on the BlueGiga module and then being sent up
      // to the device later and confusing some devices. In particular I am wondering if it
      // is confusing a bluetooth headset I'm testing with. The solution here is to comment
      // out this line and send nothing at all in these cases.
      //     SendBlueGigaCommand(F("AVRCP NFY CHANGED 2 2 1")); 
  }
  else
  {
    SendBlueGigaCommand("AVRCP NFY CHANGED " + transactionLabelTrackChanged + " 2 1"); 
  }
  
  // Report to our arduino console debug log what the current state of playback on the empeg is.
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
    Log(F("Empeg state..................................PLAYING  >"));
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
// resetType       0 = Use the "RESET" command and also physically reset BT module
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
  // Convenience feature to help with issue #26. Pause the player if we do
  // any kind of reset on the bluetooth module because all of the reset types
  // below will disconnect the bluetooth module. Music should always be paused
  // any time that the bluetooth module is disconnected to prevent the tracks
  // from spooling out silently to no audience.
  SendEmpegCommand('W');

  // Perform different types of resets depending on the parameter passed in.
  switch (resetType)
  {
    case 0:
      Log(F("Performing soft reset of bluetooth module."));
      SendBlueGigaCommand(F("RESET"));
      ClearGlobalVariables();
      DisplayAndSwallowResponses(4, 500);

      // Additional hardware reset of bluetooth after sofware reset.
      // this allows the unit to power up fully on any voltage instead
      // of being in sleep mode at board startup. 
      // EXPERIMENT: Trying to not do this for every reset, instead
      // only do it once at bootup. This is Mark Lord's suggestion to
      // not to this every time. Hopefully this will work reliably.
      //   ResetBluetoothPin(); 

      break;

    case 1:
      SendBlueGigaCommand(F("BOOT 0"));
      ClearGlobalVariables();
      DisplayAndSwallowResponses(4, 500);
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

// ----------------------------------------------------------------------------
// ResetBluetoothPin
//
// Physically reset the bluetooth module via its RST pin. This uses a
// connection between one of the Arduino's GPIO pins and the reset pin
// on the bluetooth module itself.
// ----------------------------------------------------------------------------
void ResetBluetoothPin()
{
  // Only perform this routine if we have implemented the physical reset line.
  if (performResetLinePhysical)
  {
    // Perform the steps to physically fire the reset line
    Log(F("Physically resetting bluetooth module with RST line - Begin."));  
    pinMode(resetLinePin, OUTPUT);
    digitalWrite(resetLinePin, HIGH);
    DisplayAndProcessCommands(100, false);
    digitalWrite(resetLinePin, LOW);
    DisplayAndProcessCommands(100, false);
    pinMode(resetLinePin, INPUT);
    Log(F("Physically resetting bluetooth module with RST line - Complete."));  

    // Pause for the bootup messages to appear after the reset.
    DisplayAndSwallowResponses(3, 300);
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
  Log(F("Clearing global variables."));
  transactionLabelPlaybackStatusChanged = "";
  transactionLabelTrackChanged = "";

  // Woops- bugfix. Don't clear this one out at all ever. Pairing mode,
  // regular mode, any mode... Basically any time I clear this out I 
  // get some kind of bug. This isn't the right place to clear this var.
  // if (!pairingMode)
  // {
  //   pairAddressString = "";
  // }
}

