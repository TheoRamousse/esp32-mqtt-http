#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "../lib/temperature.h"
#include "../lib/buffer.h"
#include "time.h"
#include "../lib/arduinoJson.h"
#include <EEPROM.h>
#include <PubSubClient.h>
#include <ArduinoBLE.h>

#define SERVICE_UUID "9e37b54e-52d1-493c-8969-3bf5c986987b"
#define CHARACTERISTIC_UUID "4a840088-60bb-4edb-8eeb-1dc4dd3592c9"

const char *ssid = "iPhone de Théo";
const char *password = "zzzzzzzz";

BLEService service("12345678-1234-5678-1234-56789abcdef0");                                 // UUID du service
BLECharacteristic dataCharacteristic("00000000-0000-3200-0670-056000000001", BLEWrite, 20); // UUID de la caractéristique
int TEMP_SLEEP_DURATION = 2;
int CONNECTION_FREQ = 10;
int PROTOCOLE = 1;

WiFiClient client;
String urlHttp = "http://172.20.10.3:3000";
const char *mqtt_server = "172.20.10.3";
String idForBroker = "monSuperDevinhnjnuhezhueriguruhuce123";

PubSubClient mqttClient(client);

int64_t tsStart;

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

  byte firstByte = EEPROM.read(0);
  Serial.println(firstByte);
  if (firstByte != 0xFF)
  {
    EEPROM.get(0, TEMP_SLEEP_DURATION);
    EEPROM.get(4, CONNECTION_FREQ);
    EEPROM.get(8, PROTOCOLE);
    EEPROM.end();
    return;
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
    Serial.println('.');
    delay(200);
  }

  if ((WiFi.status() == WL_CONNECTED))
  { // Check the current connection status
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
    }
    else
    {
      Serial.println("Error on HTTP request GET");

      String payloadGet = httpGet.getString();
      Serial.println(httpCodeGet);
      Serial.println(payloadGet);
    }

    httpGet.end();
    if (PROTOCOLE == 1)
    {

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
      if (!mqttClient.connected())
      {
        reconnect();
      }

      if (client)
      { // If a new client connects,
        mqttClient.loop();

        mqttClient.publish("esp", "esp32");
        String text = serialize();
        Serial.println(text);
        delay(1000);
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

  if (!BLE.begin())
  {
    Serial.println("Impossible de démarrer le BLE !");
    while (1)
      ;
  }

  // Configurez le service et les caractéristiques
  BLE.setLocalName("Serveur de données");
  BLE.setAdvertisedService(service);
  service.addCharacteristic(dataCharacteristic);
  BLE.addService(service);

  // Commencez à diffuser le service
  BLE.advertise();

  tsStart = getTime();
}

void loop()
{
  BLEDevice central = BLE.central();

  // Si une connexion est établie
  if (central)
  {
    Serial.print("Connecté à : ");
    Serial.println(central.address());

    if (dataCharacteristic.written())
    {
      String res = (char *)dataCharacteristic.value();
      delay(100);
      Serial.println(res);
      DynamicJsonDocument doc(256);

      DeserializationError error = deserializeJson(doc, res);

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
    }
  }

  int64_t diff = getTime() - tsStart;

  if (diff > CONNECTION_FREQ * 1000)
  {
    sendRequest();
    initBuffer();
    tsStart = getTime();
  }

  addValue(readTemp());
  delay(TEMP_SLEEP_DURATION * 1000);
}