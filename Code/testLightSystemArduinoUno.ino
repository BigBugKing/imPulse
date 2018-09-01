/*
 * Reference: http://www.gammon.com.au/interrupts -- Amazing guy!! =) 
 */

#include "FastLED.h"

#define NUM_LEDS 50 //LEDs in your strip for both lights as they have the same number of LEDs
#define R_DATA_PIN 7//27 //Data pin for WS281B RGB LED type for rear light
#define F_DATA_PIN 6//26 //Data pin for WS281B RGB LED type for rear light
CRGB Rleds[NUM_LEDS]; //Define the array of LEDs
CRGB Fleds[NUM_LEDS]; //Define the array of LEDs

const byte Brake = 4; //Brake Pin
const byte LI = 2; //Left Indicator Pin
const byte RI = 3;//25; //Right Indicator Pin
volatile int countL = 1; //Count for LI
volatile int countR = 1; //Count for LI

//Add LDR (3)
const byte LDR1 = 1; //For ambient light
//const byte LDR2 = ;
//const byte LDR3 = ;



void setup (){
  
  FastLED.addLeds<WS2812B, R_DATA_PIN, GRB>(Rleds, NUM_LEDS); //Add rear LEDs
  FastLED.addLeds<WS2812B, F_DATA_PIN, GRB>(Fleds, NUM_LEDS); //Add front LEDs
  pinMode(LI, INPUT_PULLUP);
  pinMode(RI, INPUT_PULLUP);
  pinMode(Brake, INPUT_PULLUP);
  pinMode(LDR1, INPUT);
  //pinMode(LDR2, INPUT);
  //pinMode(LDR3, INPUT);
  attachInterrupt (digitalPinToInterrupt (LI), LIP, CHANGE); //LI interrupt
  attachInterrupt (digitalPinToInterrupt (RI), RIP, CHANGE); //RI interrupt

  //Startup strategy using only front lights with fade in/out and a funny face
  
}  // End of setup

void loop (){

  //Headlight condition
  int ambLight = analogRead(LDR1);
  if (ambLight <= 500){
    ambLight = map(ambLight,500,150,0,50); //Perhaps PID?
    headLightOn(ambLight);
    //brakeLight(20);//issues...CPU or way of programming?
    if (countL %2 == 0){
      indicatorOn(1,-1);
    }else if(countR %2 == 0){
      indicatorOn(0,1);
    }
    
  //Brake light condition
  }else if (ambLight <= 500 && digitalRead(Brake)==LOW){
    ambLight = map(ambLight,500,150,0,50);
    headLightOn(ambLight);
    //brakeLight(50);//issues...CPU or way of programming?
     if (countL %2 == 0){
      indicatorOn(1,-1);
    }else if(countR %2 == 0){
      indicatorOn(0,1);
    }
    
  }else if(digitalRead(Brake)==LOW){
    brakeLight(20);
    
  //Left indicator condition 
  }else if (countL %2 == 0){
    indicatorOn(0,-1);
    
  //Right indicator condition
  }else if(countR %2 == 0){
    indicatorOn(1,1);
  
  //Just keep lights off
  }else{
    lightsOff();
  }

} //End of loop

// Left Indicator Pressed Interrupt Service Routine (ISR)
void LIP (){
  if (digitalRead (LI) == LOW){
    countL++;
  }
}  // End of LIP

// Right Indicator Pressed ISR
void RIP (){
  if (digitalRead (RI) == LOW){
    countR++;
  }
}  // End of RIP


//indicatorOn
void indicatorOn(int z,int j) {
  FastLED.setBrightness(20);
  for (int i = 0; i<=5;i++){

    //Front Light
    Fleds[(5-z)-i*j].setRGB( 255, 30, 0);
    Fleds[(14+z)+i*j].setRGB( 255, 30, 0);
    Fleds[(25-z)-i*j].setRGB( 255, 30, 0);
    Fleds[(34+z)+i*j].setRGB( 255, 30, 0);
    Fleds[(45-z)-i*j].setRGB( 255, 30, 0);

    //Rear Light
    Rleds[(5-z)+i*j].setRGB( 255, 30, 0);
    Rleds[(14+z)-i*j].setRGB( 255, 30, 0);
    Rleds[(25-z)+i*j].setRGB( 255, 30, 0);
    Rleds[(34+z)-i*j].setRGB( 255, 30, 0);
    Rleds[(45-z)+i*j].setRGB( 255, 30, 0);
    FastLED.show();
    delay (70); 
  }
  delay(200);
  
  FastLED.setBrightness(20);
  for (int i = 0; i<=5;i++){

    //Front Light
    Fleds[(5-z)-i*j].setRGB( 0, 0, 0);
    Fleds[(14+z)+i*j].setRGB( 0, 0, 0);
    Fleds[(25-z)-i*j].setRGB( 0, 0, 0);
    Fleds[(34+z)+i*j].setRGB( 0, 0, 0);
    Fleds[(45-z)-i*j].setRGB( 0, 0, 0);

    //Rear Light
    Rleds[(5-z)+i*j].setRGB( 0, 0, 0);
    Rleds[(14+z)-i*j].setRGB( 0, 0, 0);
    Rleds[(25-z)+i*j].setRGB( 0, 0, 0);
    Rleds[(34+z)-i*j].setRGB( 0, 0, 0);
    Rleds[(45-z)+i*j].setRGB( 0, 0, 0);
   
    FastLED.show();
}
}//End of indicatorOn

//lightsOff
void lightsOff(){
  for (int i = 0; i<NUM_LEDS;i++){
  Rleds[i].setRGB( 0, 0, 0);
  Fleds[i].setRGB( 0, 0, 0);
  FastLED.show();
  }
} //End of lightsOff

//brakeLight
void brakeLight(int k){
  FastLED.setBrightness(k);
  for (int i = 0; i<NUM_LEDS;i++){
  Rleds[i].setRGB(255, 0, 0);
  FastLED.show();
  }
}//End of brakeLight

//headLightOn
void headLightOn(int k){
  FastLED.setBrightness(k);
  for (int i = 0; i<NUM_LEDS;i++){
  Fleds[i].setRGB(255, 255, 255);
  FastLED.show();
  }
}//End of HeadLightOn


