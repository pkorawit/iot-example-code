/*
 *  This sketch sends data via HTTP GET requests to data.sparkfun.com service.
 *
 *  You need to get streamId and privateKey at data.sparkfun.com and paste them
 *  below. Or just customize this script to talk to other HTTP servers.
 *
 */

#include <WiFi.h>
#include <Wire.h>

#define LM73_ADDR 0x4D
#define SEND_DELAY 3000

const int analogInPin = 36;
const char* ssid     = "Saitama_2.4G";
const char* password = "0817677402";

int slot_data_temp = 1;
int slot_data_ldr  = 2;

float temp = 0;
int ldr = 0;

float readTemperature();

void setup()
{
    Serial.begin(115200);
    Wire1.begin(4, 5);
    delay(10);

    // We start by connecting to a WiFi network

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void loop()
{

  temp = readTemperature();
  ldr  = analogRead(analogInPin);

  Serial.print("Temp  = ");  Serial.println(temp);
  Serial.print("LDR   = ");  Serial.println(ldr);

  delay(SEND_DELAY);
}

float readTemperature() {
  Wire1.beginTransmission(LM73_ADDR);
  Wire1.write(0x00);
  Wire1.endTransmission();

  uint8_t count = Wire1.requestFrom(LM73_ADDR, 2);
  float temp = 0.0;
  if (count == 2) {
    byte buff[2];
    buff[0] = Wire1.read();
    buff[1] = Wire1.read();
    temp += (int)(buff[0] << 1);
    if (buff[1] & 0b10000000) temp += 1.0;
    if (buff[1] & 0b01000000) temp += 0.5;
    if (buff[1] & 0b00100000) temp += 0.25;
    if (buff[0] & 0b10000000) temp *= -1.0;
  }
  return temp;
}

