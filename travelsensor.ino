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

#define 	M_E   2.7182818284590452354

LiquidCrystal lcd(12,11,5,4,3,2);

const boolean debug = true;

const int button1Pin = 7;
const int button2Pin = 8;
const int piezoPin = 6;

const int sensorCount = 2;
const String label[sensorCount] = {"Light", "Temperature"};
const String unit [sensorCount] = {"Lux"  , "C"          };
const int    pin  [sensorCount] = {1      , 0            };

// array of function pointers, assigned in setup()
float (*conv[sensorCount]) (int);

/**
  * Conversion of the Lux reading from the photo-cell (GL55228) is done
  * according to the marvellous research done here:
  * http://pi.gate.ac.uk/posts/2014/02/25/airpisensors/
  */
float conv_lux(int reading) {
  
  // calculate the current resistance of the photo-cell
  float pullUp = 10000; // 10 kOhm pull-down resistor
  float vin = 5000;     // mV
  float vout = map(reading,0,1023,0,5000);  // in mV
  float cellRes = pullUp * ((vin/vout)-1);
  
  // convert to Lux
  float alpha = ( log(cellRes/1000) - 4.125 ) / -0.6704;
  float lux = pow(M_E, alpha); // e^alpha
  
  return lux;
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



  // Set the modes for the sensor, button and piezo pins
  for(int i = 0; i < sensorCount; i++) {
    pinMode(pin[i], INPUT);
  }
  pinMode(button1Pin, INPUT);
  pinMode(button2Pin, INPUT);
  pinMode(piezoPin, OUTPUT);

  // Set the first sensor to read to 0
  currentSensor = 0;
  
  // Initialize LCD and clear the display
  lcd.begin(16, 2);
  lcd.clear();

  // Display splash screen
  splashScreen();
  
  // Print the first label, otherwise it will only be printed after
  // the first 'interval' milliseconds have passed.
  lcd.clear();
  printLabel(currentSensor);
  
  // Start the timer
  timer_reset();

}

void splashScreen() {
  lcd.setCursor(0,0);
  lcd.print("travelsensor");
  lcd.setCursor(0,1);
  lcd.print(" - sep 14");
  tone(piezoPin, 500, 100);
  delay(110);
  tone(piezoPin, 750, 100);
  delay(110);
  tone(piezoPin, 1000, 100);
  delay(110);
  delay(1000);
}
/**
  * Do an analog of the given sensor number.
  * Uses pins[] array to read out the proper pin and the conv[] array
  * to convert the reading with the appropriate conversion function 
  * to the final display value.
  */
float readSensor(int sensor) {
  int reading = analogRead(pin[sensor]);
  float converted = conv[sensor](reading);
  
  // debug
  if(debug) {
    Serial.print("Sensor[");
    Serial.print(label[sensor]);
    Serial.print("]: Analog Reading: ");
    Serial.print(reading);
    Serial.print(", Converted: ");
    Serial.print(converted);
    Serial.print(" ");
    Serial.println(unit[sensor]);
  }
  
  return converted;
}

/**
  * Print the label and unit of the current sensor in the top row of the LCD.
  */
void printLabel(int sensor) {
  lcd.setCursor(0,0);
  lcd.print(label[sensor]);
  lcd.print(" (");
  lcd.print(unit[sensor]);
  lcd.print(")");
}

/**
  * MAIN LOOP
  */
void loop()
{
  if(debug) {
    Serial.print("Button1: ");
    Serial.println(digitalRead(button1Pin));
    Serial.print("Button2: ");
    Serial.println(digitalRead(button2Pin));
  }

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

