#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Pin Definitions
#define RED_LED_PIN 25
#define GREEN_LED_PIN 26
#define BUTTON_PIN 22
#define BATTERY_PIN 32
#define I2C_SCL_PIN 18
#define I2C_SDA_PIN 19
#define ANALOG_PIN 34

// Initialize LCD parameters (I2C address is usually 0x27 or 0x3F)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Function Prototypes: Demo functions for testing.

volatile bool blinkRed = true;
volatile unsigned long lastDebounceTime = 0;
// Interrupt triggers immediately when the button is pressed
void IRAM_ATTR handleButton()
{
  if ((millis() - lastDebounceTime) > 250)
  { // 250ms debounce
    blinkRed = !blinkRed;
    lastDebounceTime = millis();
  }
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Setup complete");

  pinMode(RED_LED_PIN, OUTPUT);   // Red LED
  pinMode(GREEN_LED_PIN, OUTPUT); // Green LED

  pinMode(BUTTON_PIN, INPUT); // Button
  attachInterrupt(BUTTON_PIN, handleButton, FALLING);
  pinMode(BATTERY_PIN, INPUT); // battery monitoring

  // Configure I2C with specified pins and initialize the LCD
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Ready...");

  pinMode(ANALOG_PIN, INPUT); // Analog Reading (via Audio Jack)

  // ! Warning ;
  // Don't Change : Preventing from the physical GND bridge.
  pinMode(23, INPUT);
}

// Demo function for testing each module.

// RED, GREEN blinking withb button control.

void loop()
{
  // Read analog value
  int analogValue = analogRead(ANALOG_PIN);

  // Read battery value (6V mapped to 3.3V via voltage divider)
  // ESP32 ADC is 12-bit, so 3.3V = 4095
  int batADC = analogRead(BATTERY_PIN);
  int batPercent = map(batADC, 0, 4095, 0, 100);
  batPercent = constrain(batPercent, 0, 100);

  // Format for Teleplot extension
  Serial.print(">Analog:");
  Serial.println(analogValue);
  Serial.print(">Battery:");
  Serial.println(batPercent);

  // Update the LCD
  lcd.setCursor(0, 0);
  lcd.print("Analog: ");
  lcd.print(analogValue);
  lcd.print("    "); // Pad with spaces to clear lingering digits

  lcd.setCursor(0, 1);
  lcd.print("Battery: ");
  lcd.print(batPercent);
  lcd.print("%   "); // Pad with spaces

  if (blinkRed)
  {
    digitalWrite(GREEN_LED_PIN, LOW);                // Ensure green is off
    digitalWrite(RED_LED_PIN, (millis() / 500) % 2); // Toggles HIGH/LOW every 500ms
  }
  else
  {
    digitalWrite(RED_LED_PIN, LOW); // Ensure red is off
    digitalWrite(GREEN_LED_PIN, (millis() / 500) % 2);
  }

  delay(50); // Delay for readable serial plotting and reduce I2C spamming
}
