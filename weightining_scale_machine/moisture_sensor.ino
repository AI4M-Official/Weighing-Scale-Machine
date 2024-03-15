#include <Adafruit_ADS1X15.h>

#include <Wire.h>
//#include <Adafruit_ADS1015.h>

#define ADS1115_ADDRESS 0x48u   // ADS1115 I2C address
#define MOISTURE_SENSOR_PIN 0u  // ADS1115 input pin for moisture sensor

#define SDA_PIN 4u              // Defined SDA pin
#define SCL_PIN 5u              // Defined SCL pin

Adafruit_ADS1115 ads;

void setup() {
  Serial.begin(115200u);
  Wire.begin(SDA_PIN, SCL_PIN); // Initialize I2C with defined SDA and SCL pins
  setupADS1115();
}

void setupADS1115() {
  ads.begin();
}

void loop() {
  readMoisture();
  delay(1000); // Adjust delay as needed
}

void readMoisture() {
  int16_t moistureValue = ads.readADC_SingleEnded(MOISTURE_SENSOR_PIN);
  float moisturePercentage = (moistureValue * 100.0f) / 32767.0f;
  Serial.print(F("Moisture: "));
  Serial.println(moisturePercentage);
}
