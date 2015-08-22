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
void loop() {
  // [1] Update the current acc of each axis
  float ax, ay, az;

  accelemeter.getAcclemeter(&ax, &ay, &az);

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
