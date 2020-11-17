// ****************************************************************************
//       Sketch: ChevyMusic
// Date Created: 11/10/2020
//
//     Comments: Builds off of example WavTrigAdvanced from Robersonic, to be used
//               with Robortsonic's WAV Trigger.
//  
// Programmers: Jamie Robertson, info@robertsonics.com ((initial example code)
//              Marc Elliott, marc.elliott@live.com (modifications)
//
// ****************************************************************************
//
// To use this sketch with an UNO, you'll need to:
//
// 1) Download and install the AltSoftSerial library.
// 2) Download and install the Metro library.
// 3) Connect 3 wires from the UNO to the WAV Trigger's serial connector:
//
//    Uno           WAV Trigger
//    ===           ===========
//    GND  <------> GND
//    Pin9 <------> RX
//    Pin8 <------> TX
//    Pin10<------> PLO (Low when playing, HI else)
//
//    Uno           SR501 IR Sensor
//    ===           ===============
//    GND  <------> GND
//    5V   <------> Vcc 
//    Pin7 <------> TrigOut
//
//    Pin6 <------> LED
//
//    If you want to power the WAV Trigger from the Uno, then close the 5V
//    solder jumper on the WAV Trigger and connect a 4th wire:
//
//    5V   <------> 5V
//
//    (If you are using an Arduino with extra hardware serial ports, such as
//    an Arduino Mega or Teensy, you don't need AltSoftSerial, and you should
//    edit the wavTrigger.h library file to select the desired serial port
//    according to the documentation contained in that file. And use the
//    appropriate TX/RX pin connections to your Arduino)
//
// 4) Download and install the demo wav files onto the WAV Trigger's microSD
//    card. You can find them here:
//
//    http://robertsonics.com/2015/04/25/arduino-serial-control-tutorial/
//
//    You can certainly use your own tracks instead, although the demo may
//    not make as much sense. If you do, make sure your tracks are at least
//    10 to 20 seconds long and have no silence at the start of the file.

#include <Metro.h>
#include <AltSoftSerial.h>    // Arduino build environment requires this
#include <wavTrigger.h>

//#define MUSIC_TRACK (1)
#define MAX_LEVEL (0)   // +1 dBm
//#define MAX_MUSIC_LEVEL (3)  // 0 dBm

enum {
  NOBODY_AROUND = 0, // play music only
  PERSON_PRESENT, 
};

wavTrigger wTrig;             // Our WAV Trigger object

Metro gLedMetro(500);         // LED blink interval timer

#define MIN_TRACK_NUM 1 
byte gLedState = 0;           // LED State
int  gSeqState = NOBODY_AROUND; // Main program sequencer state
int  gRateOffset = 0;         // WAV Trigger sample-rate offset
int  gNumTracks;              // Number of tracks on SD card
int  gRandTrack = MIN_TRACK_NUM;
int  gLastTrack = 0;
int  LED = 13;
int  PIR = 7;                 // Passive IR Sensor attached to PIN7
int  gPirState = 0;           // State of Passive IR sensor (0-no detect 1-detected human)

int WAV_PLAYING_PIN = 10;
int gPlayingState = 1;         // 0 - playing, 1= not playing



char gWTrigVersion[VERSION_STRING_LEN];    // WAV Trigger version string


// ****************************************************************************
void setup() {
  
  // Serial monitor
  Serial.begin(57600);
 
  // Initialize the LED pin
  pinMode(LED,OUTPUT);
  digitalWrite(LED,gLedState);

  pinMode(WAV_PLAYING_PIN, INPUT);

  pinMode(PIR, INPUT);
  gPirState = digitalRead(PIR);
  //Serial.print("Pir:");
  //Serial.println(gPirState);

  // If the Arduino is powering the WAV Trigger, we should wait for the WAV
  //  Trigger to finish reset before trying to send commands.
  delay(1000);

  // WAV Trigger startup at 57600
  wTrig.start();
  delay(10);
  
  // Send a stop-all command and reset the sample-rate offset, in case we have
  //  reset while the WAV Trigger was already playing.
  wTrig.stopAllTracks();
  wTrig.samplerateOffset(0);
  
  // Enable track reporting from the WAV Trigger
  wTrig.setReporting(true);
  
  // Allow time for the WAV Trigger to respond with the version string and
  //  number of tracks.
  delay(100); 
  
  // If bi-directional communication is wired up, then we should by now be able
  //  to fetch the version string and number of tracks on the SD card.
  if (wTrig.getVersion(gWTrigVersion, VERSION_STRING_LEN)) {
      Serial.print(gWTrigVersion);
      Serial.print("\n");
      gNumTracks = wTrig.getNumTracks();
      Serial.print("Number of tracks = ");
      Serial.print(gNumTracks);
      Serial.print("\n");
  }
  else
      Serial.print("WAV Trigger response not available");


  wTrig.samplerateOffset(0);            // Reset our sample rate offset
  wTrig.masterGain(2);                  // Reset the master gain to 0dB
              
  //wTrig.trackGain(MUSIC_TRACK, -40);              // Preset Track gain to -40dB
  //wTrig.trackPlayPoly(MUSIC_TRACK);               // Start Track
  //wTrig.trackFade(MUSIC_TRACK, MAX_MUSIC_LEVEL, 2000, 0);       // Fade Track up to 0dB over 2 secs  
  //wTrig.trackLoop(MUSIC_TRACK, 1);                // Enable Track looping
  gRandTrack = MIN_TRACK_NUM;
  gLastTrack = gRandTrack;
}


// ****************************************************************************
// This program uses a Metro timer to create a sequencer that steps through
//  states at 6 second intervals - you can change this rate above. Each state
//  Each state will demonstrate a WAV Trigger serial control feature.
//
//  In this advanced example, some states wait for specific audio tracks to
//  stop playing before advancing to the next state.

void loop() {
  
  int i;

  bool triggered = false;
  // Call update on the WAV Trigger to keep the track playing status current.
  wTrig.update();
  // Get latest state of Passive Infrared Sensor
  gPirState = digitalRead(PIR);
  Serial.print("Pir:");
  Serial.println(gPirState);
  
  if (gPirState) {
     gSeqState = PERSON_PRESENT;
     triggered = true;
  } 
  else
  { 
      gSeqState = NOBODY_AROUND;
  } 

  // We will limit the song rate to at most once per 60 seconds, but only after 
  // previous track has finished

  gPlayingState = digitalRead(WAV_PLAYING_PIN);
  Serial.print("Playing:");
  Serial.println(gPlayingState);

  if ((triggered == true ) &&
      (gPlayingState == 1)) {
     // If we have motion, AND 
     // are not currently playing a track, play a new track
     triggered = false;
     // stop the last track
     wTrig.trackFade(gLastTrack, -70, 1000, true); 
     //delay(1000);
     if (gSeqState == PERSON_PRESENT) {
         // Time to play next track
         // Do not play the same track over again
         //gRandTrack = gLastTrack + 1;
         //if (gRandTrack > gNumTracks) gRandTrack = MIN_TRACK_NUM;
         while (gRandTrack == gLastTrack) {
            gRandTrack = random(1, gNumTracks);
         }

         Serial.print("gRandTrack =");
         Serial.println(gRandTrack);

         gLastTrack = gRandTrack;
         wTrig.trackGain(gRandTrack, -40);             // Preset Track 1 gain to -40dB
         wTrig.trackPlayPoly(gRandTrack);               // Start Track 1
         wTrig.trackFade(gRandTrack, MAX_LEVEL, 100, false);   // Fade Track 1 up to 0db over 2 secs
         wTrig.update();                      
     } else {
          // nobody around, do nothing
     }
           
  } // if (gSeqState.check() == 1)
 
  if (gSeqState != NOBODY_AROUND) 
     gLedState = 1;
  else 
     gLedState = 0;
  digitalWrite(LED, gLedState);


  // Delay 200 msecs
  delay(200);
}
