/*---------- LIBRARIES ----------*/

/*----- Smart Lighting System -----*/

//#define FASTLED_INTERRUPT_RETRY_COUNT 0 //Just in case -- Ref: https://github.com/FastLED/FastLED/issues/306
#define FASTLED_ALLOW_INTERRUPTS 0 //Ref: https://github.com/FastLED/FastLED/issues/306
#include "FastLED.h"

/*---------- PINS/VARIABLES DEFINITION ----------*/

/*----- Smart Lighting System -----*/

#define NUM_LEDS 50 //LEDs in your strip for both lights as they have the same number of LEDs
#define R_DATA_PIN 2//Data pin for WS281B RGB LED type for rear light
#define F_DATA_PIN 13//Data pin for WS281B RGB LED type for front light
CRGB Rleds[NUM_LEDS]; //Define the array of LEDs
CRGB Fleds[NUM_LEDS]; //Define the array of LEDs

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

const byte Brake = 25;//Brake Pin
bool stateBrake = HIGH; //Brake Light boolbean state
bool stateBeam = HIGH; //Beam Light boolbean state
int ambLight = 0; //Initialize LDR varioable for analog readings
int LI = D8;//Left Indicator Pin
int RI = D4;//Right Indicator Pin
volatile int countL = 1; //Count for LI
volatile int countR = 1; //Count for LI

const byte topLDR = 39; ///Top LDR
//const byte rightLDR = 34 ; //Rigth LDR
//const byte leftLDR = 36; //Left LDR

/*----- DATA LOGGER -----*/

int sum = 0; //Sumatory of analogue measurements
int iter = 10; //No. of iterations for measurements

/*---------- OTHER FUNCTIONS ----------*/

/*----- Smart Lighting System -----*/

// Left Indicator Pressed Interrupt Service Routine (ISR) for ESP32
// Ref: https://techtutorialsx.com/2017/09/30/esp32-arduino-external-interrupts/
void IRAM_ATTR LIP (){ 
  portENTER_CRITICAL_ISR(&mux);
  if (digitalRead (LI) == LOW){
    countL++;
  }
  portEXIT_CRITICAL_ISR(&mux);
}  // End of LIP

// Right Indicator Pressed ISR
void IRAM_ATTR RIP (){
  portENTER_CRITICAL_ISR(&mux);
  if (digitalRead (RI) == LOW){
    countR++;
  }
  portEXIT_CRITICAL_ISR(&mux);
}  // End of RIP


void setup (){
  
  /*----- Smart Lighting System -----*/
  
  FastLED.addLeds<WS2812B, R_DATA_PIN, GRB>(Rleds, NUM_LEDS); //Add rear LEDs
  FastLED.addLeds<WS2812B, F_DATA_PIN, GRB>(Fleds, NUM_LEDS); //Add front LEDs
  pinMode(LI, INPUT_PULLUP);
  pinMode(RI, INPUT_PULLUP);
  pinMode(Brake, INPUT_PULLUP);
  pinMode(topLDR, INPUT);
  //pinMode(rightLDR, INPUT);
  //pinMode(leftLDR, INPUT);
  attachInterrupt (digitalPinToInterrupt (LI), LIP, CHANGE); //LI interrupt
  attachInterrupt (digitalPinToInterrupt (RI), RIP, CHANGE); //RI interrupt
  Serial.begin(9600);
 

  //Startup strategy? -- FUTURE RELEASE
  
}  // End of setup

void loop (){
  //Brake light condition
  if(digitalRead(Brake)==LOW){
      brakeLight(20);
      FastLED.show();
      stateBrake = LOW;
  }else if (digitalRead(Brake)==HIGH && stateBrake == LOW){
    lightsOff();
    stateBrake = HIGH;
  }
      
  //Headlight condition
  ambLight = analogRead(topLDR);

  if (ambLight <= 60 && stateBeam == HIGH){
    
    //ambLight = (-1)*ambLight;
    //ambLight=map(ambLight,-60,-10,0,50); //Perhaps PID? -- FUTURE RELEASE
    headLightOn(50);// ONE FON INSTEAD OF TWO??
    brakeLight(20);
    FastLED.show();
    stateBeam = LOW;
    
  }else if (digitalRead(Brake)==LOW && stateBeam == LOW){
    
    brakeLight(50);
    FastLED.show();
    
  }else if (ambLight > 60 && stateBeam == LOW){
    
      lightsOff();
      stateBeam = HIGH;
      
  } 

  //Indicators' condition  
  if(countR %2 == 0){
    
      indicatorOn(1,1);
      
  }else if(countL %2 == 0){
    
        indicatorOn(0,-1);
  }

} //End of loop

/*----------  CUSTOM FUNCTIONS ----------*/

/*----- Smart Lighting System -----*/

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
  }
  FastLED.show();
} //End of lightsOff

//brakeLight
void brakeLight(int y){
  FastLED.setBrightness(y);
  for (int i = 0; i<NUM_LEDS;i++){
  Rleds[i].setRGB(255, 0, 0);
  }
  //FastLED.show();
}//End of brakeLight

//headLightOn
void headLightOn(int k){
  FastLED.setBrightness(k);
  for (int i = 0; i<NUM_LEDS;i++){
  Fleds[i].setRGB(255, 255, 255);
  }
  //FastLED.show();
}//End of HeadLightOn

