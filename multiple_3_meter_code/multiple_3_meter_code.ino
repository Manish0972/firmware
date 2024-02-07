#include <Wire.h>
#include <FS.h>
#include <SoftwareSerial.h>
#include <ModbusMaster.h>
#include <WiFiManager.h>

#define MAX485_DE 23
#define MAX485_RE_NEG 22
#define GREEN_LED_PIN 32

#define RED_LED_PIN 12

SoftwareSerial modbus(17, 16);  // RX, TX

ModbusMaster node;
String DeviceUID = "SL03202362";

int slaveIDs[] = {1, 2, 3};
int maxMeter = 3;

int tab[] = {3, 5};
const char *registerNames[] = {"1", "2"};

const int numRegisters = sizeof(tab) / sizeof(tab[0]);
float values[numRegisters];

int maxRetryCount = 5; // Maximum number of retries before restarting
int retryCount = 0;

union u_tag
{
  uint16_t bdata[2];
  float flotvalue;
} uniflot;

void preTransmission()
{
  digitalWrite(MAX485_RE_NEG, HIGH);
  digitalWrite(MAX485_DE, HIGH);
}

void postTransmission()
{
  digitalWrite(MAX485_RE_NEG, LOW);
  digitalWrite(MAX485_DE, LOW);
}

void setup()
{
  Serial.begin(9600);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(MAX485_RE_NEG, OUTPUT);
  pinMode(MAX485_DE, OUTPUT);
  digitalWrite(MAX485_RE_NEG, LOW);
  digitalWrite(MAX485_DE, LOW);

  modbus.begin(9600);
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);
}

void loop()
{
  // Iterate through each meter and read and send data
  for (int i = 0; i < maxMeter; i++)
  {
    int currentSlaveID = slaveIDs[i];
    node.begin(currentSlaveID, modbus); // Set the current slave ID
    readAndSendData(currentSlaveID, DeviceUID);
    delay(10000); // Delay between readings for each meter
  }

  delay(100); // Adjust delay based on your needs
}

void readAndSendData(int currentSlaveID, const String &DeviceUID)
{
    String combinedID = DeviceUID + "-" + String(currentSlaveID);
  Serial.print("deviceUID : ");
  Serial.println(combinedID);

  for (int i = 0; i < numRegisters; ++i)
  {
    uint16_t result = node.readHoldingRegisters(tab[i] - 1, 2);

    if (result == node.ku8MBSuccess)
    {
      uint16_t highWord = node.getResponseBuffer(0x00);
      uint16_t lowWord = node.getResponseBuffer(0x01);

      float floatValue;
      memcpy(&floatValue, &highWord, sizeof(uint16_t));
      memcpy(((char *)&floatValue) + sizeof(uint16_t), &lowWord, sizeof(uint16_t));

      // Serial.print("Meter ");
      // Serial.print(currentSlaveID);
      // Serial.print(", ");
      Serial.print(registerNames[i]);
      Serial.print(" : ");
      Serial.println(floatValue);
    }
    else
    {
      digitalWrite(GREEN_LED_PIN, HIGH);
    }

    delay(40);
  }
}
