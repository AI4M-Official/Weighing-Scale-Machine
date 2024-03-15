#include "HX711.h"
#include <Wire.h>

#include <BluetoothSerial.h>

#define HX711_DOUT_PIN 21   // Connect DOUT pin of HX711 to pin 21 on ESP32
#define HX711_CLK_PIN  22   // Connect CLK pin of HX711 to pin 22 on ESP32

BluetoothSerial SerialBT;

HX711 scale;

float weight;
float calibration_factor = 2471.24;  // Adjust this value for calibration



void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32_BT_Scale");  // Set the Bluetooth name for your ESP32

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

  // Send the weight information over Bluetooth
  SerialBT.print("Weight: ");
  SerialBT.print(weight);
  SerialBT.println(" g");

  
  delay(100);
}
