#include <Wire.h>
#include "HX711.h"
#include <BluetoothSerial.h>
#include <Adafruit_ADS1X15.h>

// HX711 configuration
#define HX711_DOUT_PIN 21   // Connect DOUT pin of HX711 to pin 21 on ESP32
#define HX711_CLK_PIN  22   // Connect CLK pin of HX711 to pin 22 on ESP32

HX711 scale;
float weight;
float calibration_factor = 2471.24;  // Adjust this value for calibration

// ADS1115 configuration
#define ADS1115_ADDRESS 0x48u   // ADS1115 I2C address
#define MOISTURE_SENSOR_PIN 0u  // ADS1115 input pin for moisture sensor

// I2C configuration
#define SDA_PIN 4u              // Defined SDA pin
#define SCL_PIN 5u              // Defined SCL pin

BluetoothSerial SerialBT;
Adafruit_ADS1115 ads;

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32_BT_Scale");

  // HX711 setup
  scale.begin(HX711_DOUT_PIN, HX711_CLK_PIN);
  scale.set_scale();
  scale.tare();  // Reset the scale to 0

  // ADS1115 setup
  Wire.begin(SDA_PIN, SCL_PIN);
  ads.begin();
}

void loop() {
  // Reading from HX711
  scale.set_scale(calibration_factor);
  weight = scale.get_units(5);
  
  // Ensure the weight is non-negative
  if (weight < 0) {
    weight = 0.00;
  }

  // Print weight information
  Serial.print("Weight (HX711): ");
  Serial.print(weight);
  Serial.println(" g");

  // Reading from ADS1115 (moisture sensor)
  int16_t moistureValue = ads.readADC_SingleEnded(MOISTURE_SENSOR_PIN);
  float moisturePercentage = (moistureValue * 100.0f) / 32767.0f;

  // Print moisture information
  delay(100);
  Serial.print("Moisture (ADS1115): ");
  Serial.print(moisturePercentage);
  Serial.println("%");

  // Send weight and moisture information over Bluetooth
  SerialBT.print("Weight (HX711): ");
  SerialBT.print(weight);
  SerialBT.println(" g");

  SerialBT.print("Moisture (ADS1115): ");
  SerialBT.print(moisturePercentage);
  SerialBT.println("%");

  delay(100);
}
