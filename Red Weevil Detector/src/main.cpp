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

volatile bool blinkLED = true;
volatile bool buttonPressed = false;
volatile unsigned long lastDebounceTime = 0;
// Interrupt triggers immediately when the button is pressed
void IRAM_ATTR handleButton()
{
  if ((millis() - lastDebounceTime) > 250)
  { // 250ms debounce
    blinkLED = !blinkLED;
    buttonPressed = true;
    lastDebounceTime = millis();
  }
}

void battery_check(); // Function to check battery level and control LEDs
void record_signal(); // Function to record analog signals for ML

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Setup complete");

  pinMode(RED_LED_PIN, OUTPUT);   // Red LED
  pinMode(GREEN_LED_PIN, OUTPUT); // Green LED

  pinMode(BUTTON_PIN, INPUT_PULLUP); // Button (pullup to prevent floating)
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

  // Battery Check, started.

  // Begin analog recording once battery check is dismissed by the button
  battery_check();
  record_signal();

}

// Demo function for testing each module.

void battery_check()
{
  // Read analog value
  int analogValue = analogRead(ANALOG_PIN);

  // Read battery value (6V mapped to 3.3V via voltage divider)
  // ESP32 ADC is 12-bit, so 3.3V = 4095
  int batADC = analogRead(BATTERY_PIN);
  int batPercent = map(batADC, 0, 4095, 0, 100);
  batPercent = constrain(batPercent, 0, 100);

  // Display on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Battery:");
  lcd.setCursor(0, 1);
  lcd.print(batPercent);
  lcd.print("%");

  // Print to Serial
  Serial.print("Analog Value: ");
  Serial.print(analogValue);
  Serial.print(" | Battery: ");
  Serial.print(batPercent);
  Serial.println("%");

  // If battery level drops below 4V, Turn RED LED ON, and Blink it continuously. Otherwise, Turn GREEN LED ON and blink it continuously until button is pressed.
  while (blinkLED)
  {
    if (batPercent < 66) // 4V is approximately 66% of 6V
    {
      digitalWrite(GREEN_LED_PIN, LOW);
      digitalWrite(RED_LED_PIN, HIGH);
      delay(500); // LED ON duration
      digitalWrite(RED_LED_PIN, LOW);
      delay(500); // LED OFF duration
    }
    else
    {
      digitalWrite(RED_LED_PIN, LOW);
      digitalWrite(GREEN_LED_PIN, HIGH);
      delay(500); // LED ON duration
      digitalWrite(GREEN_LED_PIN, LOW);
      delay(500); // LED OFF duration
    }
  }

  // Turn off both LEDs when the button is pressed and the while loop ends
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, LOW);
}

void record_signal()
{

  // Clear the LCD and display recording message, turn off both LEDs
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, LOW);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Recording...");

  unsigned long startTime = millis();
  unsigned long lastCountdownTime = startTime;
  int countdown = 10;

  lcd.setCursor(0, 1);
  lcd.print("Time left: 10s");

  // Output formatting so you can easily copy-paste or read the data via Python
  Serial.println("---START_RECORDING---");
  Serial.println("timestamp_ms,analog_value");

  while (millis() - startTime < 10000)
  {
    unsigned long currentTime = millis();

    int val = analogRead(ANALOG_PIN);

    // Print data continuously to Serial Monitor
    Serial.print(currentTime - startTime);
    Serial.print(",");
    Serial.println(val);

    // Update countdown every 1 second
    if (currentTime - lastCountdownTime >= 1000)
    {
      countdown--;
      lcd.setCursor(11, 1);
      if (countdown < 10)
        lcd.print(" "); // Add space to clear remaining '0' from '10'
      lcd.print(countdown);
      lcd.print("s ");
      lastCountdownTime += 1000;
    }

    // ~100Hz sampling rate loop delay. Change if ML model requires a different rate.
    delay(10);
  }

  Serial.println("---END_RECORDING---");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Done!");

  buttonPressed = false; // Reset button state for potential future use

  // Turn on the GREEN LED to indicate successful recording completion
  digitalWrite(RED_LED_PIN, HIGH);
}

void loop()
{
  // Should run the record_signal whenever the button is pressed.
  if (buttonPressed)
  {
    record_signal();
  }
}
