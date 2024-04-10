#include <esp_system.h>
#include <BLEDevice.h>

#include <BLEUtils.h>

#include <BLEServer.h> //Library to use BLE as server

#include <BLE2902.h> 

#include <Wire.h>
#include <Adafruit_ADS1X15.h>

#include <HX711.h>


#define HX711_DOUT_PIN 21u      // HX711 data out pin
#define HX711_CLK_PIN 22u       // HX711 clock pin



#define OLED_SDA_PIN 4u         // OLED display SDA pin
#define OLED_SCL_PIN 5u         // OLED display SCL pin

#define ADS1115_ADDRESS 0x48u   // ADS1115 I2C address
#define MOISTURE_SENSOR_PIN 0u  // ADS1115 input pin for moisture sensor


float Weight = 0.1;
float moisture = 6.1;


// Structure for HX711
struct HX711Sensor {
  HX711 scale;
  float_t weight;
  float_t calibration_factor;
};


// Structure for ADS1115
struct ADS1115Sensor {
  Adafruit_ADS1115 ads;
};


HX711Sensor hx711Sensor;      // HX711 object for weight measurement
ADS1115Sensor adsSensor;      // ADS1115 object for moisture measurement

bool _BLEClientConnected = false;


#define SERVICE_UUID "91bad492-b950-4226-aa2b-4ede9fa42f59"


BLECharacteristic FrootWeightCharacteristic("cba1d466-344c-4be3-ab3f-189f80dd7518", BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor FrootWeightDescriptor(BLEUUID((uint16_t)0x2902));

BLECharacteristic FrootMoistureCharacterisitc("ca73b3ba-39f6-4ab3-91ae-186dc9577d99", BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor FrootMoistureDescriptor(BLEUUID((uint16_t)0x2903));

#define ALPHA 0.05 // Adjust the filter coefficient as needed

float MoistureRawData[32];
uint8_t MoistureIndex = 0; 
uint8_t GattUpdateCounter = 0;

float low_pass_filter(float data, float prev_filtered_data) {
    // Apply the low-pass filter equation
    return ALPHA * data + (1 - ALPHA) * prev_filtered_data;
}


class MyServerCallbacks : public BLEServerCallbacks {

    void onConnect(BLEServer* pServer) {

      _BLEClientConnected = true;
      Serial.print("Client Connected ");

    };


    void onDisconnect(BLEServer* pServer) {

      _BLEClientConnected = false;
      Serial.print("Client DisConnected ");
      esp_restart();
    }

};


/*
 * Function:  setupADS1115
 * --------------------
 * Initializes the ADS1115 analog-to-digital converter.
 * This function is called from the setup() function.
 */
void setupADS1115() {
  adsSensor.ads.begin();  // Initialize ADS1115
}

/*
 * Function:  setupHX711
 * --------------------
 * Initializes the HX711 load cell amplifier.
 * This function is called from the setup() function.
 */
void setupHX711() {
  hx711Sensor.scale.begin(HX711_DOUT_PIN, HX711_CLK_PIN);  // Initialize HX711
  hx711Sensor.scale.set_scale();        // Set scale for HX711
  hx711Sensor.scale.tare();             // Tare HX711
  hx711Sensor.calibration_factor = 2471.24f;  // Set calibration factor
}

float median_filter(float data[], int size) {
    // Sorting the data (using bubble sort for simplicity)
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - i - 1; j++) {
            if (data[j] > data[j + 1]) {
                // Swap elements if out of order
                float temp = data[j];
                data[j] = data[j + 1];
                data[j + 1] = temp;
            }
        }
    }
    // Median is the middle value (or average of two middle values if window size is even)
    if (size % 2 == 0) {
        return (data[size / 2] + data[size / 2 - 1]) / 2.0;
    } else {
        return data[size / 2];
    }
}


/*
 * Function:  readMoisture
 * --------------------
 * Reads the moisture level from the ADS1115 analog-to-digital converter.
 * Prints the moisture level to serial.
 */
float readMoisture() {
  
 
  MoistureRawData[MoistureIndex] = (float)adsSensor.ads.readADC_SingleEnded(MOISTURE_SENSOR_PIN);  // Read moisture value from ADS1115
   static float PrevMoistureData = MoistureRawData[0];
  MoistureIndex ++ ;

  if(MoistureIndex >=32)
  {
    MoistureIndex = 0;
    float MedianData = median_filter(MoistureRawData, 32);
    PrevMoistureData = MedianData; //low_pass_filter(MedianData,PrevMoistureData);
  }
  
  return PrevMoistureData;
}


void InitBLE() {

  BLEDevice::init("Froot Server");

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();

  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *bmeService = pServer->createService(SERVICE_UUID);

  // Create BLE Characteristics and Create a BLE Descriptor
  // Weight
  bmeService->addCharacteristic(&FrootWeightCharacteristic);
  FrootWeightDescriptor.setValue("Froot Grain Weight");
  FrootWeightCharacteristic.addDescriptor(&FrootWeightDescriptor);
  

  // Moisture
  bmeService->addCharacteristic(&FrootMoistureCharacterisitc);
  FrootMoistureDescriptor.setValue("Froot Grain Moisture");
  FrootMoistureCharacterisitc.addDescriptor(new BLE2902());
  
  // Start the service
  bmeService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");

}


void setup() {

  Serial.begin(115200);

  Serial.println("Froot");

  Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);  // Initialize I2C communication for OLED

  setupHX711();                   // Setup HX711 for weight measurement
  setupADS1115();                 // Setup ADS1115 for moisture measurement


  InitBLE();

}

void loop() {



  GattUpdateCounter++;
  moisture = readMoisture();
  if(GattUpdateCounter >= 40)
  {
    GattUpdateCounter = 0;
    if (_BLEClientConnected)
    {

      hx711Sensor.scale.set_scale(hx711Sensor.calibration_factor);
      hx711Sensor.weight = hx711Sensor.scale.get_units(5);          // Get weight measurement from HX711
      if (hx711Sensor.weight < 0.0f) {                    // Check if weight is negative
        hx711Sensor.weight = 0.0f;                        // Set weight to zero if negative
      }
      Weight = hx711Sensor.weight;
      
      static char Weightingram[6];
      dtostrf(Weight, 6, 2, Weightingram);
      //Set temperature Characteristic value and notify connected client
      FrootWeightCharacteristic.setValue(Weightingram);
      FrootWeightCharacteristic.notify();
      Serial.print("Weight: ");
      Serial.print(Weight);
      Serial.print(" grams");

    
      static char humidityTemp[8];
      dtostrf(moisture, 8, 2, humidityTemp);
      //Set humidity Characteristic value and notify connected client
      FrootMoistureCharacterisitc.setValue(humidityTemp);
      FrootMoistureCharacterisitc.notify();   
      Serial.print(" - Moisture: ");
      Serial.print(moisture);
      //Serial.println(" %");
      Serial.println("");
    }
  }
  
  delay(50);

}