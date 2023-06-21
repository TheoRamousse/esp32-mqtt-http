#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "../lib/temperature.h"
#include "../lib/buffer.h"
#include "time.h"
#include "../lib/arduinoJson.h"
#include <EEPROM.h>
#include <PubSubClient.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

#define SERVICE_UUID "9e37b54e-52d1-493c-8969-3bf5c986987b"
#define CHARACTERISTIC_UUID "4a840088-60bb-4edb-8eeb-1dc4dd3592c9"

const char *ssid = "iPhone de Théo";
const char *password = "zzzzzzzz";

BLEServer *pServer;
BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;

int TEMP_SLEEP_DURATION = 2;
int CONNECTION_FREQ = 10;
int PROTOCOLE = 2;

WiFiClient client;
String urlHttp = "http://172.20.10.3:3001";
const char *mqtt_server = "172.20.10.3";
String idForBroker = "monSuperDevinhuhezhueriguruhuce123";

PubSubClient mqttClient(client);

int64_t tsStart;

class MyServerCallbacks : public BLEServerCallbacks
{
  void onConnect(BLEServer *pServer)
  {
    deviceConnected = true;
  }

  void onDisconnect(BLEServer *pServer)
  {
    deviceConnected = false;
  }
};

void processBLEData(String data)
{

  Serial.println("Données BLE reçues : " + data);

  pCharacteristic->setValue("Données reçues avec succès !");
  pCharacteristic->notify();
}

class MyCallbacks : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    std::string value = pCharacteristic->getValue();

    if (value.length() > 0)
    {
      // Appeler la fonction pour traiter les données
      processBLEData(value.c_str());
    }
  }
};

int64_t getTime()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (tv.tv_sec * 1000LL + (tv.tv_usec / 1000LL));
}

String asString(float *buffer)
{
  String result = "";

  for (int cpt = 0; cpt < myIndex(); cpt++)
  {
    if (buffer[cpt] == -100000.69)
      break;

    if (cpt == 0)
      result = result + buffer[cpt];
    else
      result = result + ", " + buffer[cpt];
  }

  return result;
}

void saveConfigToEEPROM()
{
  EEPROM.begin(512); // Début de l'utilisation de l'EEPROM

  // Sauvegarde des données de configuration
  EEPROM.put(0, TEMP_SLEEP_DURATION);
  EEPROM.put(4, CONNECTION_FREQ);
  EEPROM.put(8, PROTOCOLE);

  EEPROM.commit(); // Enregistrement des modifications
  EEPROM.end();    // Fin de l'utilisation de l'EEPROM
}

void loadConfigFromEEPROM()
{
  EEPROM.begin(512); // Début de l'utilisation de l'EEPROM

  int test;
  EEPROM.get(0, test);

  if (test != -1)
  {
    // Chargement des données de configuration
    EEPROM.get(0, TEMP_SLEEP_DURATION);
    EEPROM.get(4, CONNECTION_FREQ);
    EEPROM.get(8, PROTOCOLE);
  }

  EEPROM.end(); // Fin de l'utilisation de l'EEPROM
}

String serialize()
{
  String result = "";
  StaticJsonDocument<256> doc;

  JsonObject config = doc.createNestedObject("config");
  config["tempFreq"] = TEMP_SLEEP_DURATION;
  config["connectionConfig"] = PROTOCOLE;
  config["connectionFreq"] = CONNECTION_FREQ;

  JsonArray temperatures = doc.createNestedArray("temperatures");

  for (int cpt = 0; cpt < myIndex(); cpt++)
  {
    temperatures.add(getBuffer()[cpt]);
  }

  doc.add(temperatures);

  serializeJson(doc, result);

  return result;
}

void reconnect()
{
  // Loop until we're reconnected
  while (!mqttClient.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttClient.connect(idForBroker.c_str()))
    {
      Serial.println("connected");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void sendRequest()
{
  WiFi.mode(WIFI_AP);
  WiFi.begin(ssid, password);
  delay(1000);

  Serial.println("\nConnecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(200);
  }

  if ((WiFi.status() == WL_CONNECTED))
  { // Check the current connection status
    if (PROTOCOLE == 1)
    {
      HTTPClient httpGet;
      httpGet.begin(client, urlHttp);
      httpGet.addHeader("Content-Type", "application/json");
      int httpCodeGet = httpGet.GET();

      if (httpCodeGet > 0)
      { // Check for the returning code
        String payloadGet = httpGet.getString();
        DynamicJsonDocument doc(256);
        Serial.println(payloadGet);

        DeserializationError error = deserializeJson(doc, payloadGet);

        // Test if parsing succeeds.
        if (error)
        {
          Serial.print(F("deserializeJson() failed: "));
          Serial.println(error.f_str());
          return;
        }

        TEMP_SLEEP_DURATION = doc["tempFreq"];
        CONNECTION_FREQ = doc["connectionFreq"];
        PROTOCOLE = doc["connectionConfig"];

        saveConfigToEEPROM();

        HTTPClient http;
        http.begin(client, urlHttp + "/api/esp32/esp32test");
        http.addHeader("Content-Type", "application/json");
        String text = serialize();
        int httpCode = http.PUT(text);

        if (httpCode > 0)
        { // Check for the returning code

          String payload = http.getString();
          Serial.println(httpCode);
          Serial.println(payload);
        }

        else
        {
          Serial.println("Error on HTTP request PUT");

          String payload = http.getString();
          Serial.println(httpCode);
          Serial.println(payload);
        }

        http.end(); // Free the resources
      }
      else
      {
        Serial.println("Error on HTTP request GET");

        String payloadGet = httpGet.getString();
        Serial.println(httpCodeGet);
        Serial.println(payloadGet);
      }

      httpGet.end();
    }
    else
    {
      if (!mqttClient.connected())
      {
        reconnect();
      }

      if (client)
      { // If a new client connects,
        mqttClient.loop();

        mqttClient.publish("esp", "esp32");
        String text = serialize();
        mqttClient.publish("esp32", text.c_str());
      }

      mqttClient.disconnect();
    }
  }

  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
}

void setup()
{
  Serial.begin(115200);
  delay(1000);

  mqttClient.setServer(mqtt_server, 1883);
  loadConfigFromEEPROM();
  initBuffer();

  BLEDevice::init("ESP des Théos"); // Nom du périphérique BLE
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(BLEUUID((uint16_t)0x180F)); // Service générique de batterie

  pCharacteristic = pService->createCharacteristic(
      BLEUUID((uint16_t)0x2A19), // Caractéristique de niveau de batterie
      BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_WRITE |
          BLECharacteristic::PROPERTY_NOTIFY |
          BLECharacteristic::PROPERTY_INDICATE);

  pCharacteristic->setCallbacks(new MyCallbacks());

  pCharacteristic->addDescriptor(new BLE2902());

  pService->start();
  pServer->getAdvertising()->start();

  tsStart = getTime();
}

void loop()
{

  int64_t diff = getTime() - tsStart;

  if (!deviceConnected && oldDeviceConnected)
  {
    delay(500);                  // Attendre une déconnexion propre
    pServer->startAdvertising(); // Recommencer l'annonce
    Serial.println("Attente de la connexion BLE...");
    oldDeviceConnected = deviceConnected;
  }

  if (deviceConnected && !oldDeviceConnected)
  {
    oldDeviceConnected = deviceConnected;
  }

  if (diff > CONNECTION_FREQ * 1000)
  {
    sendRequest();
    initBuffer();
    tsStart = getTime();
  }

  addValue(readTemp());
  esp_sleep_enable_timer_wakeup(TEMP_SLEEP_DURATION * 100000);
  delay(500);
  esp_light_sleep_start();
}