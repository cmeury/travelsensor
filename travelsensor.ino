
#include <LiquidCrystal.h>

LiquidCrystal lcd(12,11,5,4,3,2);

const int ledPin = 13;
const int temperaturePin = 0;
const int photoPin = 1;

boolean buttonPressed;

float currentTemp;
int currentLight;
String labels[] = {"Light level", "Temperature"}

void setup()
{
  lcd.begin(16, 2);
  lcd.clear();

  pinMode(ledPin, OUTPUT);
  pinMode(temperaturePin, INPUT);
  pinMode(photoPin, INPUT);
  
  digitalWrite(ledPin, LOW);
  
  buttonPressed = false;
}

float degreesC(int pin) {
    int tmpReading = analogRead(temperaturePin);
    float tmpVoltage =  tmpReading * 0.004882814;
    return (tmpVoltage - 0.5) * 100.0;
}
void loop()
{
  
  if(buttonPressed == true) {
    // noop
  } else {
    int lightLevel = analogRead(photoPin);
    float temp = degreesC(temperaturePin);
  
    
    lcd.setCursor(0,0);
    lcd.print("Light level:    ");
    lcd.setCursor(0,0);
    lcd.print("Light level: ");
    lcd.print(lightLevel);
  
    lcd.setCursor(0,1);
    lcd.print("Current Tmp:    ");
    lcd.setCursor(0,1);
    lcd.print("Current Tmp:");
    lcd.print(temp);

    delay(100);
  }
}

