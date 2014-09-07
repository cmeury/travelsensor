/** travelsensor.ino
  *
  * Learning to use the Arduino platform; displaying various sensor outputs
  * on a liquid crystal display (temperature, light level and pressure).
  *
  * Components:
  * - SparkFun RedBoard 
  * - Arduino and Breadboard Holder
  * - White Solderless Breadboard
  * - some wires, a LED and a 330 Ohm transistor
  * - 16x2 White on Black LCD (with headers)
  * - TMP36 Temp Sensor
  * - GL5528 Photo-Cell
  *
  */

#include <LiquidCrystal.h>

LiquidCrystal lcd(12,11,5,4,3,2);

const int statusPin = 13;

const int sensorCount = 2;
const String labels[sensorCount] = {"Light", "Temperature"};
const String unit  [sensorCount] = {"Lux"        , "C"};
const int    pins  [sensorCount] = {1            , 0            };
const float  factor[sensorCount] = {1            , 0.4882814    };
const int    offset[sensorCount] = {0            , -50          };

// array of function pointers, assigned in setup()
float (*conv[sensorCount]) (int);

/**
  * Conversion of the Lux reading from the photo-cell (GL55228) is done
  * according to the marvellous research done here:
  * http://pi.gate.ac.uk/posts/2014/02/25/airpisensors/
  */
float conv_lux(int reading) {
  return reading;
}

/**
  * Conversion of the temperature read-out is done according
  * to the SparkFun Inventor's Kit 3.1 guide-book.
  */
float conv_temp(int reading) {
  float voltage = map(reading,0,1023,0,5000) / 1000.0;
  return (voltage - 0.5) * 100.0;
}

// state variables
int currentSensor;

void setup()
{
  // Set-up serial connection for debugging
  Serial.begin(9600);
  
  // Assign conversion functions to function pointer array
  conv[0] = conv_lux;
  conv[1] = conv_temp;

  // Initialize LCD and clear the display
  lcd.begin(16, 2);
  lcd.clear();

  // Set the modes for the sensor and status pins
  for(int i = 0; i < sensorCount; i++) {
    pinMode(pins[i], INPUT);
  }
  pinMode(statusPin, OUTPUT);
  
  // Turn off the status LED
  digitalWrite(statusPin, LOW);

  // Set the first sensor to read to 0
  currentSensor = 0;
  
  // Print the first label, otherwise it will only be printed after
  // the first 'interval' milliseconds have passed.
  printLabel(currentSensor);
  
  // Start the timer
  timer_reset();
}

/**
  * Do an analog of the given sensor number.
  * Uses pins[] array to read out the proper pin and the conv[] array
  * to convert the reading with the appropriate conversion function 
  * to the final display value.
  */
float readSensor(int sensor) {
  int reading = analogRead(pins[sensor]);
  return conv[sensor](reading);
}

/**
  * Print the label and unit of the current sensor in the top row of the LCD.
  */
void printLabel(int sensor) {
  lcd.setCursor(0,0);
  lcd.print(labels[sensor]);
  lcd.print(" (");
  lcd.print(unit[sensor]);
  lcd.print(")");
}

/**
  * MAIN LOOP
  */
void loop()
{
  if(timer_check()) {
    currentSensor++;
    if(currentSensor >= sensorCount) {
      currentSensor = 0;
    }
    lcd.clear();
    printLabel(currentSensor);
  }
  
  float val = readSensor(currentSensor);
  lcd.setCursor(0,1);
  lcd.print("                ");
  lcd.setCursor(0,1);
  lcd.print(val);

  // wait a bit, otherwise the floating point values changes too fast
  delay(200);
  
}

