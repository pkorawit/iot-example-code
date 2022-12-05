#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"


class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

/* Assign a unique ID to this sensor at the same time */
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

void displaySensorDetails(void) {
  sensor_t sensor;
  accel.getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.print("Sensor: ");
  Serial.println(sensor.name);
  Serial.print("Driver Ver: ");
  Serial.println(sensor.version);
  Serial.print("Unique ID: ");
  Serial.println(sensor.sensor_id);
  Serial.print("Max Value: ");
  Serial.print(sensor.max_value);
  Serial.println(" m/s^2");
  Serial.print("Min Value: ");
  Serial.print(sensor.min_value);
  Serial.println(" m/s^2");
  Serial.print("Resolution: ");
  Serial.print(sensor.resolution);
  Serial.println(" m/s^2");
  Serial.println("------------------------------------");
  Serial.println("");
  delay(500);
}

void displayDataRate(void) {
  Serial.print("Data Rate: ");

  switch (accel.getDataRate()) {
    case ADXL345_DATARATE_3200_HZ:
      Serial.print("3200 ");
      break;
    case ADXL345_DATARATE_1600_HZ:
      Serial.print("1600 ");
      break;
    case ADXL345_DATARATE_800_HZ:
      Serial.print("800 ");
      break;
    case ADXL345_DATARATE_400_HZ:
      Serial.print("400 ");
      break;
    case ADXL345_DATARATE_200_HZ:
      Serial.print("200 ");
      break;
    case ADXL345_DATARATE_100_HZ:
      Serial.print("100 ");
      break;
    case ADXL345_DATARATE_50_HZ:
      Serial.print("50 ");
      break;
    case ADXL345_DATARATE_25_HZ:
      Serial.print("25 ");
      break;
    case ADXL345_DATARATE_12_5_HZ:
      Serial.print("12.5 ");
      break;
    case ADXL345_DATARATE_6_25HZ:
      Serial.print("6.25 ");
      break;
    case ADXL345_DATARATE_3_13_HZ:
      Serial.print("3.13 ");
      break;
    case ADXL345_DATARATE_1_56_HZ:
      Serial.print("1.56 ");
      break;
    case ADXL345_DATARATE_0_78_HZ:
      Serial.print("0.78 ");
      break;
    case ADXL345_DATARATE_0_39_HZ:
      Serial.print("0.39 ");
      break;
    case ADXL345_DATARATE_0_20_HZ:
      Serial.print("0.20 ");
      break;
    case ADXL345_DATARATE_0_10_HZ:
      Serial.print("0.10 ");
      break;
    default:
      Serial.print("???? ");
      break;
  }
  Serial.println(" Hz");
}

void displayRange(void) {
  Serial.print("Range: +/- ");

  switch (accel.getRange()) {
    case ADXL345_RANGE_16_G:
      Serial.print("16 ");
      break;
    case ADXL345_RANGE_8_G:
      Serial.print("8 ");
      break;
    case ADXL345_RANGE_4_G:
      Serial.print("4 ");
      break;
    case ADXL345_RANGE_2_G:
      Serial.print("2 ");
      break;
    default:
      Serial.print("?? ");
      break;
  }
  Serial.println(" g");
}

void setup(void) {
  Serial.begin(115200);
  Serial.println("Accelerometer Test");
  Serial.println("");

  /* Initialise the sensor */
  if (!accel.begin()) {
    /* There was a problem detecting the ADXL345 ... check your connections */
    Serial.println("Ooops, no ADXL345 detected ... Check your wiring!");
    while (1)
      ;
  }

  /* Set the range to whatever is appropriate for your project */
  accel.setRange(ADXL345_RANGE_16_G);
  // displaySetRange(ADXL345_RANGE_8_G);
  // displaySetRange(ADXL345_RANGE_4_G);
  // displaySetRange(ADXL345_RANGE_2_G);

  /* Display some basic information on this sensor */
  displaySensorDetails();

  /* Display additional settings (outside the scope of sensor_t) */
  displayDataRate();
  displayRange();
  Serial.println("");

  Wire1.begin(4, 5);
  delay(10);
  // Create the BLE Device
  BLEDevice::init("M5Atom");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService* pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_INDICATE);

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  pCharacteristic->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");
}

void loop(void) {
  char txString[8];
  char txXString[8];
  char txYString[8];
  char txZString[8];
  // notify changed value
  if (deviceConnected) {
    /* Get a new sensor event */
    sensors_event_t event;
    accel.getEvent(&event);

    /* Display the results (acceleration is measured in m/s^2) */
    Serial.print("X: ");
    Serial.print(event.acceleration.x);
    Serial.print(" ");
    Serial.print("Y: ");
    Serial.print(event.acceleration.y);
    Serial.print(" ");
    Serial.print("Z: ");
    Serial.print(event.acceleration.z);
    Serial.print(" ");
    Serial.println("m/s^2 ");

    dtostrf(event.acceleration.x, 1, 2, txXString);
    dtostrf(event.acceleration.y, 1, 2, txYString);
    dtostrf(event.acceleration.z, 1, 2, txZString);
    sprintf(txString, "%0.3f,%0.3f,%0.3f", event.acceleration.x, event.acceleration.y,event.acceleration.z);
    pCharacteristic->setValue(txString);
    pCharacteristic->notify();
    delay(10);  // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
  }
  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);                   // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising();  // restart advertising
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
  }

  delay(50);
}
