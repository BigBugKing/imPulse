/* This program will be managing your bike its sensors, 
 *  energy generation, lighting system and Blynk!
 *  Author: Javier Betancor
 *  Project: imPulse
 *  Version: 0.0.2 - 30/10/18
 *  Libraries/packages needed: FireBeetle driver, VCP driver for MAC, arduino-esp32, 
 *    sudo easy_install pip, python -m pip install pyserial FastLED 3.2.1, 
 *    Blynk 0.5.4, ESP_BLE_Arduino 0.4.16
 */

/*---------- LIBRARIES ----------*/

/*----- Smart Lighting System -----*/

//#define FASTLED_INTERRUPT_RETRY_COUNT 0 //Just in case -- Ref: https://github.com/FastLED/FastLED/issues/306
#define FASTLED_ALLOW_INTERRUPTS 0 //Ref: https://github.com/FastLED/FastLED/issues/306
#include "FastLED.h"

/*----- Blynk -----*/

//#define BLYNK_PRINT Serial //Comment this out to disable prints and save space

#define BLYNK_USE_DIRECT_CONNECT

#include <BlynkSimpleEsp32_BLE.h>
#include <BLEDevice.h>
#include <BLEServer.h>


/*---------- PINS/VARIABLES DEFINITION ----------*/

/*----- Smart Lighting System -----*/

#define NUM_LEDS 50 //LEDs in your strip for both lights as they have the same number of LEDs
#define NUM_STRIPS 2 //No. of independent strips (Front Light + Rear Light = 2)
#define R_DATA_PIN 2// D9 (IO02) - Data pin for WS281B RGB LED type for rear light
#define F_DATA_PIN 13// D7 (IO13) - Data pin for WS281B RGB LED type for front light
CRGB Rleds[NUM_LEDS]; //Define the array of LEDs
CRGB Fleds[NUM_LEDS]; //Define the array of LEDs

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

const byte Brake = 25;//D2 (IO25) - Brake Pin
bool stateBrake = HIGH; //Brake Light boolbean state
bool stateBeam = HIGH; //Beam Light boolbean state
int ambLight = 0; //Initialize LDR varioable for analog readings
int LI = 5;// D8 (IO05) - Left Indicator Pin
int RI = 27;// D4 (IO27) - Right Indicator Pin
volatile int countL = 1; //Count for LI
volatile int countR = 1; //Count for RI

const byte topLDR = 39; // A1 (IO39) - Top LDR
//const byte rightLDR = 34 ; // A2 (IO34) - Rigth LDR
//const byte leftLDR = 36; // A0 (IO36) - Left LDR

/*----- DATA LOGGER -----*/
int sensorCurrent = 35; //A3 (IO35) for current sensor
int sensorVoltage = 15; //A4 (IO15) for voltage sensor
float conv = 0.0153;//(4141/4096)*(1/66); TUNE when PoweBank is connected. Low value because of the Diode (5-0.7) = 4.3V
int iter = 50; //Number of iterations
double sum = 0; //Number of total readings 

/*----- Blynk -----*/
WidgetLED ledBrake(V4);//Brake light in Blynk
WidgetLED ledRightInd(V5);//Right indicator in Blynk
WidgetLED ledLeftInd(V6);//Left indicator in Blynk
WidgetLED ledBeam(V7);//Beam light in Blynk
float linearSpeed = 0;
char auth[] = "d5c5166f65a44eea86f942b742dc9935"; //Blynk App auth. token


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

/*----- Blynk -----*/

BLYNK_WRITE(V1){ //Get linear speed form phone
  GpsParam gps(param);
  linearSpeed = gps.getSpeed();//Parameter coming from Phone
}

BLYNK_READ(V2){ //Send linear speed to Blynk
  Blynk.virtualWrite(V2, linearSpeed);
}

BLYNK_READ(V3){ //Send power generated to Blynk
  
  float amps = (trueVoltage(sensorCurrent)-2.076)*conv;//2.161V measured with Vmeter across OUT and GND pins of the ACS71230A
  float volts = readSensor(sensorVoltage)/11.872; //Spreadsheed trend line
  float powGen = abs(volts/10*amps); // P = V*I [W] NEED CHANGES
  Blynk.virtualWrite(V3, powGen);
}


void setup (){
  
  /*----- Smart Lighting System -----*/
  
  FastLED.addLeds<WS2812B, R_DATA_PIN, GRB>(Rleds, NUM_LEDS); //Add rear LEDs to stripe 0
  FastLED.addLeds<WS2812B, F_DATA_PIN, GRB>(Fleds, NUM_LEDS); //Add front LEDs to stripe 1
  pinMode(LI, INPUT_PULLUP);
  pinMode(RI, INPUT_PULLUP);
  pinMode(Brake, INPUT_PULLUP);
  pinMode(topLDR, INPUT);
  //pinMode(rightLDR, INPUT);
  //pinMode(leftLDR, INPUT);
  attachInterrupt (digitalPinToInterrupt (LI), LIP, CHANGE); //LI interrupt
  attachInterrupt (digitalPinToInterrupt (RI), RIP, CHANGE); //RI interrupt
  //Serial.begin(9600);

  /*----- DATA LOGGER -----*/
  pinMode(sensorCurrent,INPUT);
  pinMode(sensorVoltage,INPUT);

  /*----- Blynk -----*/
  Blynk.setDeviceName("imPulse");
  Blynk.begin(auth);
  
 
  //Startup protocol? -- FUTURE RELEASE
  
}  // End of setup

void loop (){
  
  /*----- Smart Lighting System -----*/

  //Brake light condition does not depend on LDR values
  if (digitalRead(Brake) == LOW){
    
    brakeLight(50);
    stateBrake = LOW;
    
  } else if (digitalRead(Brake)== HIGH && stateBrake == LOW){
    
    brakeOff();
    stateBrake = HIGH;
    
  }

  ambLight = analogRead(topLDR);

  //Beam Light condition -- Fix flickering when is dark and street lights are approached
  if (ambLight <= 60){
    
    headLightOn(50);//PID, etc...
    brakeLight(20);
    stateBeam = LOW;
    
  } else if (ambLight > 60 && stateBeam == LOW){
    
    lightsOff();
    stateBeam = HIGH;
    ledBrake.off();
    ledBeam.off();
    
  }
  //Indicators' condition
  if(countR%2 == 0){
   
    indicatorOn(1,1);
      
  }else if(countL%2 == 0){
    
    indicatorOn(0,-1);
  }
  
  /*----- Blynk -----*/

  Blynk.run();

} //End of loop

/*----------  CUSTOM FUNCTIONS ----------*/

/*----- Smart Lighting System -----*/

//indicatorOn
void indicatorOn(int z,int j) {
  FastLED.setBrightness(50);
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

  if(z == 1 && j == 1){ledRightInd.on();}else{ledLeftInd.on();}
  
  //FastLED.setBrightness(50);
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
    if(z == 1 && j == 1){ledRightInd.off();}else{ledLeftInd.off();}
}//End of indicatorOn

//lightsOff
void lightsOff(){
  fill_solid(Fleds, NUM_LEDS, CRGB::Black);
  fill_solid(Rleds, NUM_LEDS, CRGB::Black);
  FastLED.show();
  ledBrake.off();
  ledBeam.off();
} //End of lightsOff

//brakeLight
void brakeLight(int y){
  fill_solid(Rleds, NUM_LEDS, CRGB::Red);
  FastLED[0].showLeds(y);
  ledBrake.on();
}//End of brakeLight

//brakeOff
void brakeOff(){
  fill_solid(Rleds, NUM_LEDS, CRGB::Black);
  FastLED.show();
  ledBrake.off();
} //End of brakeOff

//headLightOn
void headLightOn(int k){
  fill_solid(Fleds, NUM_LEDS, CRGB::White);
  FastLED[1].showLeds(k);
  ledBeam.on();
}//End of HeadLightOn


/*----- DATA LOGGER -----*/

double readSensor(byte sensor) {
  sum = 0;
  for (int j = 0; j < iter; j++) {
    sum += analogRead(sensor);
    delay(7);
  }
  return sum;
}

double trueVoltage(byte pin){
  sum = 0;
  for (int j = 0; j < iter; j++) {
    sum += analogRead(pin);
    delay(7);
  }
  sum = sum/iter;
  if(sum < 1 || sum > 4095) return 0;
  return -6E-15*pow(sum,4) + 3E-11*pow(sum,3) - 4E-8*pow(sum,2) + 0.0008*sum + 0.1367;//Check trendline for ESP32
  }
