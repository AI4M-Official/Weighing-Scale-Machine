#include "HX711.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define HX711_DOUT_PIN 21   // Connect DOUT pin of HX711 to pin 21 on ESP32
#define HX711_CLK_PIN  22   // Connect CLK pin of HX711 to pin 22 on ESP32

#define OLED_SDA_PIN 4     // Connect SDA pin of OLED to pin 4 on ESP32
#define OLED_SCL_PIN 5     // Connect SCL pin of OLED to pin 5 on ESP32

HX711 scale;

float weight;
float calibration_factor = 2471.24;  // Adjust this value for calibration

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); // Use default Wire for ESP32

void setup() {
  Serial.begin(115200);
  Serial.println("HX711 calibration sketch");
  Serial.println("Remove all weight from scale");
  Serial.println("After readings begin, place a known weight on the scale");
  Serial.println("Press + or a to increase the calibration factor");
  Serial.println("Press - or z to decrease the calibration factor");

  // Set up the HX711 with DOUT and CLK pins
  scale.begin(HX711_DOUT_PIN, HX711_CLK_PIN);

  scale.set_scale();
  scale.tare();  // Reset the scale to 0
  long zero_factor = scale.read_average();  // Get a baseline reading
  Serial.print("Zero factor: ");
  Serial.println(zero_factor);

  // Set up the OLED display with SDA and SCL pins
  Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C, true, true);
  delay(2000);
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
}

void loop() {
  scale.set_scale(calibration_factor);
  Serial.print("Reading: ");
  weight = scale.get_units(5);

  // Ensure the weight is non-negative
  if (weight < 0) {
    weight = 0.00;
  }

  Serial.print("Grams: ");
  Serial.print(weight);
  Serial.print(" g");
  Serial.print(" Calibration Factor: ");
  Serial.print(calibration_factor);
  Serial.println();

  display.clearDisplay();
  display.setTextSize(2); // Reduced font size
  display.setCursor(0, 0);
  display.print("EyePhy AG ");

  // Calculate the width of the weight text
  int16_t weightTextWidth = display.getCursorX();
  display.setTextSize(2); // Reduced font size
  display.print(weight);

  // Adjust the cursor position for the 'g' based on the width of the weight text
  display.setCursor(weightTextWidth, 25);
  display.setTextSize(2); // Reduced font size
  display.print("grams");

  display.display();
  delay(100);
}
