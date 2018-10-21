
/* This program will test current and voltage sensors as a part of the datalogger system
 *  
 *  Author: Javier Betancor
 *  
 *  Reference: http://www.skillbank.co.uk/arduino/calibrate.htm
 *             https://github.com/espressif/esp-idf/blob/master/docs/en/api-reference/peripherals/adc.rst
 *             
 */

//Define Pins
int sensorCurrent = 35; //Analog Pin 3 (IO35) for current sensor
int sensorVoltage = 15; //Analog Pin 4 (IO15) for voltage sensor



float conv = 0.015988;//(4322/4096)*(1/66); TUNE when PoweBank is connected. Low value because of the Diode (5-0.7) = 4.3V
int iter = 100; //Number of iterations
double sum = 0; 

void setup() {
  
  analogSetWidth(12);
  adcAttachPin(sensorCurrent);
  pinMode(sensorCurrent,INPUT);
  pinMode(sensorVoltage,INPUT);
  Serial.begin(9600);

}

void loop() {
  
  float amps = (trueVoltage(sensorCurrent)-2.161)*conv;//2.161V measured with Vmeter across OUT and GND pins of the ACS71230A

  float volts = readSensor(sensorVoltage)/11.872; //Spreadsheed trend line
  
  Serial.print("Voltage [V]: ");
  Serial.print(volts/1000,2);
  //Serial.println(sum);
  Serial.print(" ");
  Serial.print(" Current [A]: ");
  Serial.println(amps,4);
  //Serial.println(conv,8);
  //Serial.println(trueVoltage(sensorCurrent));
 
}

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
  return -6E-15*pow(sum,4) + 3E-11*pow(sum,3) - 4E-8*pow(sum,2) + 0.0008*sum + 0.1367;

} 
