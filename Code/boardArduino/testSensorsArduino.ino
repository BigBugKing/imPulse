
/* This program will test current and voltage sensors as a part of the datalogger system
 *  
 *  Author: Javier Betancor
 *  
 *  Reference: http://www.skillbank.co.uk/arduino/calibrate.htm
 */

//Define Pins
int sensorCurrent = 6; //Analog Pin 6 for current sensor
int sensorVoltage = 7; //Analog Pin 7 for voltage sensor

float conv = 0.07398200758;//(5000/1024)*(1/66);
int iter = 100; //Number of iterations
float sum = 0; 

void setup() {
  pinMode(sensorCurrent,INPUT);
  Serial.begin(9600);

}

void loop() {
  
  float amps = (analogRead(sensorCurrent)-511)*conv;

  readSensor(sensorVoltage);
  float volts = sum/3.45; //Spreadsheed trend line
  
  Serial.print("Voltage [V]: ");
  Serial.print(volts/1000,2);
  //Serial.println(analogRead(sensorVoltage));
  //Serial.print(" ");
  Serial.print(" Current [A]: ");
  Serial.println(amps,4);
  //Serial.println(conv,8);
  //Serial.println(analogRead(sensorCurrent));
 
}

void readSensor(int sensor) {
sum = 0;
for (int j = 0; j < iter; j++) {
sum += analogRead(sensor);
delay(7);
}
}
