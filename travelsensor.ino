
#include <LiquidCrystal.h>

LiquidCrystal lcd(12,11,5,4,3,2);

const int statusPin = 13;

const int sensorCount = 2;
String labels[sensorCount] = {"Light level", "Temperature °C"};
int    pins  [sensorCount] = {1            , 0            };
float  factor[sensorCount] = {1            , 0.4882814    };
int    offset[sensorCount] = {0            , -50          };

int currentSensor;


void setup()
{
  
  // LCD init
  lcd.begin(16, 2);
  lcd.clear();

  // pin init
  for(int i = 0; i < sensorCount; i++) {
    pinMode(pins[i], INPUT);
  }
  pinMode(statusPin, OUTPUT);
  digitalWrite(statusPin, LOW);

  // output rotation init
  currentSensor = 0;
  printLabel(currentSensor);
  timer_reset();
}


float readSensor(int sensor) {
  int reading = analogRead(pins[sensor]);
  return (reading * factor[sensor]) + offset[sensor];
}

void printLabel(int sensor) {
  lcd.setCursor(0,0);
  lcd.print(labels[sensor]);
}

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

  delay(200);
  
}

