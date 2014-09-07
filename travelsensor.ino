/** travelsensor.ino
  *
  * Learning to use the Arduino platform; displaying various sensor outputs
  * on a liquid crystal display (temperature, light level and pressure).
  *
  * Components:
  * - SparkFun RedBoard 
  * - Arduino and Breadboard Holder
  * - White Solderless Breadboard
  * - some wires
  * - 16x2 White on Black LCD (with headers)
  * - TMP36 Temperature Sensor
  * - GL5528 Photo-Cell
  * - 2 buttons
  * - Piezo Speaker - PC Mount 12mm 2.048kHz
  */

  /** timer.ino
  *
  * The timer code comes from the Metro library: https://github.com/thomasfredericks/Metro-Arduino-Wiring
  * but was rewritten and modified to use arrays in Sketch. That paticular code (timer_reset, timer_check) 
  * is licensed differently than the main travelsensor.ino (see MIT license below):
  */
  
// The MIT License (MIT)
//
// Copyright (c) 2013 thomasfredericks
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <LiquidCrystal.h>

// constants
#define 	M_E   2.7182818284590452354

#define MODE_CONFIG 0
#define MODE_DISPLAY 1

#define TIMER_DISPLAY 0
#define TIMER_CONFIG 1
#define TIMER_REDRAW 2

#define SYMBOL_DEGREE 0

// custom degree sign for display on LCD
byte degree[8] = {
B01110,
B10001,
B10001,
B01110,
B00000,
B00000,
B00000,
}; 

// hardware set-up
LiquidCrystal lcd(12,11,5,4,3,2);
const int button1Pin = 7;
const int button2Pin = 8;
const int piezoPin = 6;

// configuration
const boolean debug = true;

const int timer_count = 3;
const int displayInterval = 2000;
const int configInterval = 3500;
const int redrawInterval = 400;

const int buttonDelay = 350;

const int sensorCount = 2;
const String label [sensorCount] = {"Light", "Temperature"};
const String unit  [sensorCount] = {"Lux"  , "C"          };
const int    symbol[sensorCount] = {-1     , SYMBOL_DEGREE}; // if set to -1, don't print anything
const int    pin   [sensorCount] = {A1     , A0           };
const float  step  [sensorCount] = {1.0    , 0.1          };

// state variables
int currentSensor;
int mode;
float delta[sensorCount];

/** *** ******* *** */
/** *** SENSORS *** */
/** *** ******* *** */

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
  if(false) {
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


/** *** ** *** */
/** *** UI *** */
/** *** ** *** */

void splashScreen() {
  if(debug) {
    Serial.println("*** travelsensor booting up...");
  }
  lcd.setCursor(0,0);
  lcd.print("travelsensor");
  lcd.setCursor(0,1);
  lcd.print(" - sep 14");
  tone(piezoPin, 500, 100);
  delay(110);
  tone(piezoPin, 750, 100);
  delay(110);
  tone(piezoPin, 1000, 100);
  delay(210);
  tone(piezoPin, 500, 100);
  delay(110);
  tone(piezoPin, 750, 100);
  delay(110);
  tone(piezoPin, 1000, 300);
  delay(310);
  delay(1000);
}

void configBeep() {
  tone(piezoPin, 500, 200);
  delay(210);
  tone(piezoPin, 1500, 400);
  delay(410);
  tone(piezoPin, 1500, 100);
  delay(110);
}

void displayBeep() {
  tone(piezoPin, 1100, 200);
  delay(230);
  tone(piezoPin, 1100, 200);
}

/**
  * Print the label and unit of the current sensor in the top row of the LCD.
  */
void printLabel(int sensor) {
  lcd.setCursor(0,0);
  lcd.print(label[sensor]);
  lcd.print(" (");

  if(symbol[sensor] != -1) {
    lcd.write(byte(symbol[sensor]));
  }
  lcd.print(unit[sensor]);
  lcd.print(")");
}

/**
  * Print the given value in the second row, clearing the first 8 columns first.
  */
void printValue(float value) {
  lcd.setCursor(0,1);
  lcd.print("        ");
  lcd.setCursor(0,1);
  lcd.print(value);
}

/**
  * Print the given value in the second row, 8th column, clearing the last 8 columns first.
  */
void printOffset(float value) {
  lcd.setCursor(8,1);
  lcd.print("        ");
  lcd.setCursor(8,1);
  lcd.print(value);
}

/**
  * Is button1 currently being pressed?
  */
boolean button1Pressed() {
  return digitalRead(button1Pin) == LOW;
}

/**
  * Is button2 currently being pressed?
  */
boolean button2Pressed() {
  return digitalRead(button2Pin) == LOW;
}


/** *** ***** *** */
/** *** TIMER *** */
/** *** ***** *** */

unsigned long* last;
int* intervals;

/**
  * Set-up the given amount of timers.
  */
void timer_init(int count) {
  last = new unsigned long[count];
  intervals = new int[count];
}

/**
  * Have we reached or passed the interval for the given timer?
  */
boolean timer_check(int timer) {
  unsigned long now = millis();
  if(intervals[timer] == 0) {
    last[timer] = now;
    return true;
  }
  if( (now-last[timer]) >= intervals[timer]) {
    last[timer] += intervals[timer];
    return true;
  }
  return false; 
}

/**
  * Reset or start the given timer using this interval.
  */
void timer_reset(int timer, int interval) {
  last[timer] = millis();
  intervals[timer] = interval;
}


/** *** ***** *** */
/** *** SETUP *** */
/** *** ***** *** */

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

  // Set the first sensor to read to 0, setting display mode
  currentSensor = 0;
  mode = MODE_DISPLAY;
  
  // Set the delta values for all sensors to 0
  for(int i = 0; i < sensorCount; i++) {
    delta[i] = 0;
  }
  
  // Initialize LCD and clear the display
  lcd.begin(16, 2);
  lcd.clear();
  lcd.createChar(SYMBOL_DEGREE, degree);
  
  // Display splash screen
  splashScreen();
  
  // Print the first label, otherwise it will only be printed after
  // the first 'interval' milliseconds have passed.
  lcd.clear();
  printLabel(currentSensor);
  
  // Set-up two timers
  timer_init(timer_count);
  
  // Start the display and redraw timer  
  timer_reset(TIMER_DISPLAY, displayInterval);
  timer_reset(TIMER_REDRAW, redrawInterval);
}


/** *** **** *** */
/** *** LOOP *** */
/** *** **** *** */

void loop()
{

  // activating config mode
  if( (button1Pressed() || button2Pressed() ) && mode == MODE_DISPLAY) {
    if(debug) {
      Serial.println("*** Entering CONFIG mode ***");
    }
    mode = MODE_CONFIG;
    timer_reset(TIMER_CONFIG, configInterval);
    configBeep();
    delay(buttonDelay);
    return;
  }
  
  if(mode == MODE_CONFIG) {
    if(timer_check(TIMER_CONFIG)) {
      // exiting config mode
      if(debug) {
        Serial.println("*** Back to DISPLAY mode ***");
      }
      mode = MODE_DISPLAY;
      displayBeep();
      timer_reset(TIMER_DISPLAY, displayInterval);
    } else {
      // we are in config mode, process the button presses
      if(button1Pressed() || button2Pressed()) {
        float sign;
        if(button1Pressed()) {
          sign = -1.0;
        } else if(button2Pressed()) {
          sign = 1.0;
        }
        delta[currentSensor] = delta[currentSensor] + (step[currentSensor] * sign);
        printOffset(delta[currentSensor]);
        lcd.setCursor(15,1);
        lcd.print("C");
        delay(buttonDelay);
        timer_reset(TIMER_CONFIG, configInterval);
      }
    }
    return;
  }
  
  if(mode == MODE_DISPLAY) {
    if(timer_check(TIMER_REDRAW)) {
      if(timer_check(TIMER_DISPLAY)) {
        currentSensor++;
        if(currentSensor >= sensorCount) {
          currentSensor = 0;
        }
        lcd.clear();
        printLabel(currentSensor);
      }
      
      float readout = readSensor(currentSensor);
      readout += delta[currentSensor];
      printValue(readout);

      // wait a bit, otherwise the floating point values changes too fast
      delay(50);
    
      return;
    }
  }
}
