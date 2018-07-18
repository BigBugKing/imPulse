
/* This program will test merge current and voltage sensors with the uSD card reader
 *  
 *  Author: Javier Betancor
 *  
 *   Circuit for the uSD card:
 * 
 *   SD card attached to SPI bus as follows:
 *   MOSI - pin 11
 *   MISO - pin 12
 *   CLK - pin 13
 *   CS - pin 4 
 */

//Libraries
#include <SPI.h>
#include <SD.h>

//Datalogger settings
const int chipSelect = 4; //CS for the datalogger
char filename[16];
int n = 0;

//Define Pins
int sensorCurrent = 6; //Analog Pin 6 for current sensor
int sensorVoltage = 7; //Analog Pin 7 for voltage sensor

float conv = 0.07398200758;//(5000/1024)*(1/66);
int iter = 100; //Number of iterations
float sum = 0; 



void setup() {
  pinMode(sensorCurrent,INPUT);
  Serial.begin(9600);

  //Wait until Serial is available
  while (!Serial) {
    ; 
  }

  //Serial.print("Initializing SD card...");
 
  if (!SD.begin(chipSelect)) {
    //Serial.println("Card failed, or not present");
    //No card inserted, so no datalogger, buzzer will beep here
    delay(1000);
    abort();
      
  }
  //Serial.println("Card initialized");
  snprintf(filename, sizeof(filename), "log%03d.txt", n); // Includes a three-digit sequence number in the file name
  while(SD.exists(filename)) {
    n++;
    snprintf(filename, sizeof(filename), "log%03d.txt", n);
  }

  //Filename contains a nonexisten file name
  File dataFile = SD.open(filename,FILE_READ);
  Serial.println(n);
  Serial.println(filename);
  dataFile.close();
  
}

void loop() {

  String dataString = ""; //String for assembling data to log
  
  //Current Sensor
  float amps = (analogRead(sensorCurrent)-511)*conv;
  dataString += String(amps,4);
  dataString += ",";

  //Voltage sensor
  readSensor(sensorVoltage);
  float volts = sum/3.45;//Spreadsheet trend line
  dataString += String(volts/1000,2);
  dataString += ",";
  
  //Serial.print("Voltage [V]: ");
  //Serial.print(volts/1000,2);
  //Serial.print(" ");
  //Serial.print("Current [mA]: ");
  //Serial.println(amps,3);

  File dataFile = SD.open(filename, FILE_WRITE); //IMPROVEMENT: NEW FILE EACH TIME!?

  // If the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    //Serial.println(dataString);
  }
  // if the file isn't open, pop up an error:
  else {
    //Serial.println("Couldn't open new file");
    //Couldn't save data, buzzer will beep here
    delay(1000);
    abort();
  }

}

void readSensor(int sensor) {
sum = 0;
for (int j = 0; j < iter; j++) {
sum += analogRead(sensor);
delay(7);
}
}
