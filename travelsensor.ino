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
  * - SFE BMP180 Pressure Breakout Board
  * - 2 buttons
  * - Piezo Speaker - PC Mount 12mm 2.048kHz
  */

#include <LiquidCrystal.h>
#include <SFE_BMP180.h>
#include <Wire.h>

// constants
#define 	M_E   2.7182818284590452354

#define MODE_CONFIG 0
#define MODE_DISPLAY 1

#define TIMER_DISPLAY 0
#define TIMER_CONFIG 1
#define TIMER_REDRAW 2

#define SYMBOL_DEGREE 0

#define BASEL_SEA_LEVEL_PRESSURE 1013.0 // from http://www.meteoschweiz.admin.ch/web/en/weather/current_weather.par0016.sub0036.html?region=5&station=BAS

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
const int displayInterval = 2500;
const int configInterval = 3500;
const int redrawInterval = 500;

const int buttonDelay = 350;

// pressure I2C driver object
SFE_BMP180 pressure;

const int sensorCount = 4;
const String label [sensorCount] = {"Light", "Temperature", "Pressure", "Altitude"};
const String unit  [sensorCount] = {"Lux"  , "C"          , "mb"    , "m"         };
const int    symbol[sensorCount] = {-1     , SYMBOL_DEGREE, -1      , -1          }; // if set to -1, don't print anything
const int    pin   [sensorCount] = {A1     , A0           , -1      , -1          };
const float  step  [sensorCount] = {1.0    , 0.1          , 10.0    , 10.0        };

// state variables
int cur_sens;
int mode;
float delta[sensorCount];

/** *** ******* *** */
/** *** SENSORS *** */
/** *** ******* *** */

// array of function pointers for reading and converting sensor data, assigned in setup()
double (*conv[sensorCount]) (double);
double (*read[sensorCount]) (int);

/**
  * Conversion of the Lux reading from the photo-cell (GL55228) is done
  * according to the marvellous research done here:
  * http://pi.gate.ac.uk/posts/2014/02/25/airpisensors/
  */
double conv_lux(double reading) {
  
  // calculate the current resistance of the photo-cell
  double pullUp = 10000; // 10 kOhm pull-down resistor
  double vin = 5000;     // mV
  double vout = map(reading,0,1023,0,5000);  // in mV
  double cellRes = pullUp * ((vin/vout)-1);
  
  // convert to Lux
  double alpha = ( log(cellRes/1000) - 4.125 ) / -0.6704;
  double lux = pow(M_E, alpha); // e^alpha
  
  return lux;
}

/**
  * Conversion of the temperature read-out is done according
  * to the SparkFun Inventor's Kit 3.1 guide-book.
  */
double conv_temp(double reading) {
  double voltage = map(reading,0,1023,0,5000) / 1000.0;
  return (voltage - 0.5) * 100.0;
}

/**
  * Return the given pressure as is.
  */
double conv_pressure(double reading) {
  return reading;
}

/**
  * Convert pressure to altitude using the a constant base-line.
  */
double conv_altitude(double reading) {
  return pressure.altitude(reading,BASEL_SEA_LEVEL_PRESSURE);
}

/**
  * Do an analog of the given sensor number.
  * Uses pins[] array to read out the proper pin.
  */
double read_analog(int sensor) {
  double reading = analogRead(pin[sensor]);
  return reading;
}

/**
  * Read-out pressure data. Taken from example code in SparkFun's BMP180 library.
  */
double read_pressure(int sensor) {
  char status = pressure.startTemperature();
  double T,P;
  if (status != 0)
  {
    delay(status); // wait for the measurement to complete
    status = pressure.getTemperature(T);
    if (status != 0)
    {
      status = pressure.startPressure(3);
      if (status != 0)
      {
        delay(status);
        status = pressure.getPressure(P,T);
        if (status != 0)
        {
          return P;
        }
      }
    }
  }
  return 1.0;
}


/** *** ** *** */
/** *** UI *** */
/** *** ** *** */

/**
  * Display splash screen.
  */
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

/**
  * Play beep when entering config mode.
  */
void configBeep() {
  tone(piezoPin, 500, 200);
  delay(210);
  tone(piezoPin, 1500, 400);
  delay(410);
  tone(piezoPin, 1500, 100);
  delay(110);
}

/**
  * Play beep when entering display mode.
  */
void displayBeep() {
  tone(piezoPin, 1100, 200);
  delay(230);
  tone(piezoPin, 1100, 200);
}
/**
  * Audio feedback for button presses.
  */
void buttonBeep() {
  tone(piezoPin, 400, 80);
  delay(85);
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
  clearOffset();
  lcd.setCursor(8,1);
  lcd.print(value);
}

/**
  * Clear the offset area of the LCD.
  */
void clearOffset() {
  lcd.setCursor(8,1);
  lcd.print("        ");
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
  if(debug) {
    // Set-up serial connection for debugging
    Serial.begin(9600);
  }
  
  // Assign read-out and conversion functions to function pointer array
  read[0] = read_analog;
  conv[0] = conv_lux;

  read[1] = read_analog;
  conv[1] = conv_temp;

  read[2] = read_pressure;
  conv[2] = conv_pressure;

  read[3] = read_pressure;
  conv[3] = conv_altitude;
  

  // Set the modes for the sensor, button and piezo pins
  for(int i = 0; i < sensorCount; i++) {
    pinMode(pin[i], INPUT);
  }
  pinMode(button1Pin, INPUT);
  pinMode(button2Pin, INPUT);
  pinMode(piezoPin, OUTPUT);

  // Set the first sensor to read to 0, setting display mode
  cur_sens = 0;
  mode = MODE_DISPLAY;
  
  // Set the delta values for all sensors to 0
  for(int i = 0; i < sensorCount; i++) {
    delta[i] = 0;
  }
  
  // Initialize pressure sensor
  pressure.begin();
  
  // Initialize LCD and clear the display
  lcd.begin(16, 2);
  lcd.clear();
  lcd.createChar(SYMBOL_DEGREE, degree);
  
  // Display splash screen
  splashScreen();
  
  // Print the first label, otherwise it will only be printed after
  // the first 'interval' milliseconds have passed.
  lcd.clear();
  printLabel(cur_sens);
  
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
      clearOffset();
      timer_reset(TIMER_DISPLAY, displayInterval);
    } else {
      // we are in config mode, process the button presses
      if(button1Pressed() || button2Pressed()) {
        
        buttonBeep();
        
        float sign;
        if(button1Pressed()) {
          sign = -1.0;
        } else if(button2Pressed()) {
          sign = 1.0;
        }
        
        delta[cur_sens] = delta[cur_sens] + (step[cur_sens] * sign);
        printOffset(delta[cur_sens]);
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
        cur_sens++;
        if(cur_sens >= sensorCount) {
          cur_sens = 0;
        }
        lcd.clear();
        printLabel(cur_sens);
      }
      
      double readout = read[cur_sens](cur_sens);
      double converted = conv[cur_sens](readout);
      converted += delta[cur_sens];
      printValue(converted);

      // wait a bit, otherwise the floating point values changes too fast
      delay(50);
    
      return;
    }
  }
}
