#include <Wire.h>
#include "MMA7660.h"
#include <rgb_lcd.h>
//=================================================
// Define
//=================================================
//=================================================
// Midi notes - Offset for easy value calculations
#define NOTE_C      0
#define NOTE_D      2
#define NOTE_E      4
#define NOTE_F      5
#define NOTE_G      7
#define NOTE_A      9
#define NOTE_B      11
#define NOTE_BASE  24  // Octave 0, C
//=================================================
// X axis = loudness mapping
#define LOUDNESS_TOTAL  3
const float kfLoudnessAclTrigger[ LOUDNESS_TOTAL ] = { 0.9, 0.15, -0.5 };
const int kiMidiVelocity[ LOUDNESS_TOTAL ] = { 0x00, 0x45, 0x65 };
//=================================================
// Y axis = pitch
#define PITCH_TOTAL  7
const float kfPitchAclTrigger[ PITCH_TOTAL ] 
  = { 0.75, 0.50, 0.25, 0, -0.25, -0.50, -0.75 };
const int kiMidiPitch[ PITCH_TOTAL ] 
  = { NOTE_C, NOTE_D, NOTE_E, NOTE_F, NOTE_G, NOTE_A, NOTE_B };
const int iOctave = 3;
//=================================================
#define PIN_DIGITAL_BUZZER 2
//=================================================
// Global Variables
//=================================================
MMA7660 accelemeter;
rgb_lcd lcd;

int iCurrentNote = NOTE_BASE;
int iCurrentVolume = 0;
//=================================================
void setup() {
  // put your setup code here, to run once:
  // Will use serial for Midi output, Set MIDI baud rate:
  Serial.begin(115200);
  accelemeter.init();
  // Start the buzzer
  pinMode(PIN_DIGITAL_BUZZER, OUTPUT);
  
    //Setup LCD display
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);    
  lcd.setRGB(0, 255, 0);

  return;
}
//=================================================
//void playNote(int scale, int note, int duration) {
//  //int tones[] = { 1915, 1700, 1519, 1432, 1275, 1136, 1014, 956 };
//  int tones[7 * 7] = { PRERIOD_NOTE_C1, PRERIOD_NOTE_D1, PRERIOD_NOTE_E1, PRERIOD_NOTE_F1, PRERIOD_NOTE_G1, PRERIOD_NOTE_A1, PRERIOD_NOTE_B1,
//                       PRERIOD_NOTE_C2, PRERIOD_NOTE_D2, PRERIOD_NOTE_E2, PRERIOD_NOTE_F2, PRERIOD_NOTE_G2, PRERIOD_NOTE_A2, PRERIOD_NOTE_B2,
//                       PRERIOD_NOTE_C3, PRERIOD_NOTE_D3, PRERIOD_NOTE_E3, PRERIOD_NOTE_F3, PRERIOD_NOTE_G3, PRERIOD_NOTE_A3, PRERIOD_NOTE_B3,
//                       PRERIOD_NOTE_C4, PRERIOD_NOTE_D4, PRERIOD_NOTE_E4, PRERIOD_NOTE_F4, PRERIOD_NOTE_G4, PRERIOD_NOTE_A4, PRERIOD_NOTE_B4,
//                       PRERIOD_NOTE_C5, PRERIOD_NOTE_D5, PRERIOD_NOTE_E5, PRERIOD_NOTE_F5, PRERIOD_NOTE_G5, PRERIOD_NOTE_A5, PRERIOD_NOTE_B5,
//                       PRERIOD_NOTE_C6, PRERIOD_NOTE_D6, PRERIOD_NOTE_E6, PRERIOD_NOTE_F6, PRERIOD_NOTE_G6, PRERIOD_NOTE_A6, PRERIOD_NOTE_B6,
//                       PRERIOD_NOTE_C7, PRERIOD_NOTE_D7, PRERIOD_NOTE_E7, PRERIOD_NOTE_F7, PRERIOD_NOTE_G7, PRERIOD_NOTE_A7, PRERIOD_NOTE_B7
//                     };
//
//  // play the tone corresponding to the note name
//  tone(PIN_DIGITAL_BUZZER, tones[scale * 7 + note - 1], duration);
//  noTone(8);
//
//  return;
//}
//=================================================
void loop() {
  //int NOTES[] = { 1, 1, 2, 3, 3, 2, 1, 2, 3, 1 } ;
  // put your main code here, to run repeatedly:
  // [1] Update the current acc of each axis
  float ax, ay, az;
//  Serial.println("accleration of X/Y/Z: ");
//  Serial.print(ax);
//  Serial.println(" g");
//  Serial.print(ay);
//  Serial.println(" g");
//  Serial.print(az);
//  Serial.println(" g");
//  Serial.println("*************");

  accelemeter.getAcclemeter(&ax, &ay, &az);

//  for (int i = 0; i < (sizeof(NOTES) / sizeof(int)); i++) {
//    if (ax < -1) ax = -1;
//    if (ax > 1) ax = 1;
//
//    playNote(abs(ax) * 6, NOTES[i], 100 + abs(ay) * 100);
//    delay(50);
//  }

  int iLoud = MapLoudness( ax );
  int iNote = MapPitch( ay );
  
  // If note has changed from current note, turn it off before playing the 
  // new note
  if( iNote != iCurrentNote || iLoud != iCurrentVolume )
  {
    noteOff(0x90, iOctave, iCurrentNote);
    iCurrentNote = iNote;
    iCurrentVolume = iLoud;
    noteOn(0x90, iOctave, iCurrentNote, iCurrentVolume);
  }
  
  // Debug output
  lcd.setCursor(0, 0); 
    lcd.print(iCurrentVolume);
    lcd.print(" - ");
    lcd.print(iCurrentNote);
    lcd.print("          "); // For cleaning up stray output
  lcd.setCursor(0, 1);
    lcd.print(ax);
    lcd.print(", ");
    lcd.print(ay);
    lcd.print(", ");
    lcd.print(az);
    lcd.print("          "); // For cleaning up stray output

  //delay(200);
  //noteOff(0x90, iOctave, iNote);
  delay(300);
  return;
}

//  plays a MIDI note.  Doesn't check to see that
//  cmd is greater than 127, or that data values are  less than 127:
//  velocity: loudness of note (0x45 => medium, 0x65 => medium high)
//  note: Use the NOTE_XXX constant for the CDEFGAB Notation
//  octave: -2~8
void noteOn(int cmd, int octave, int note, int velocity) {
  int pitch = NOTE_BASE + note + octave * 12;
  Serial.write(cmd);
  Serial.write(pitch);
  Serial.write(velocity);
}

void noteOff(int cmd, int octave, int note) {
  int pitch = NOTE_BASE + note + octave * 12;
  int velocity = 0;
  Serial.write(cmd);
  Serial.write(pitch);
  Serial.write(velocity);
}

bool isBetween( float fVal, float fA, float fB )
{
  bool bIsBetween = false;
  // A < Val < B
  if( fVal > fA && fVal < fB )
  {
    bIsBetween = true;
  }
  // B < Val < A
  else if( fVal < fA && fVal > fB )
  {
    bIsBetween = true;
  }
  return bIsBetween;
}

int MapLoudness( float fAclX )
{
  int iMidiVal = kiMidiVelocity[0];
  bool bFound = false;
  for( int i = 0; i < LOUDNESS_TOTAL && !bFound; i++ )
 {
   if( fAclX > kfLoudnessAclTrigger[i] )
   {
     bFound = true;
     iMidiVal = kiMidiVelocity[i];
   }
 }
 
 if( !bFound )
 {
   iMidiVal = kiMidiVelocity[LOUDNESS_TOTAL - 1];
 }
 
  return iMidiVal; 
}

int MapPitch( float fAclY )
{
  int iMidiVal = kiMidiPitch[0];
  bool bFound = false;
  for( int i = 0; i < PITCH_TOTAL && !bFound; i++ )
 {
   if( fAclY > kfPitchAclTrigger[i] )
   {
     bFound = true;
     iMidiVal = kiMidiPitch[i];
   }
 }
 
 if( !bFound )
 {
   iMidiVal = kiMidiPitch[PITCH_TOTAL - 1];
 }
 return iMidiVal;
}
