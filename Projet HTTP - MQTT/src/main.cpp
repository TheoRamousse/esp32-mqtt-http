#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "../lib/temperature.h"
#include "../lib/buffer.h"
#include "time.h"
#include "../lib/arduinoJson.h"
#include <EEPROM.h>
#include <PubSubClient.h>
#include <bitset>

const char *ssid = "iPhone de Théo";
const char *password = "zzzzzzzz";

int TEMP_SLEEP_DURATION = 2;
int CONNECTION_FREQ = 10;
int PROTOCOLE = 2;
int IS_BINARY = 1;

WiFiClient client;
String urlHttp = "http://172.20.10.3:3000";
const char *mqtt_server = "172.20.10.3";
String idForBroker = "monSuperDevice123";

PubSubClient mqttClient(client);

int64_t tsStart;

int64_t getTime()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (tv.tv_sec * 1000LL + (tv.tv_usec / 1000LL));
}

std::string stringToBinary(std::string const &str)
{
  std::string binary = "";
  for (char const &c : str)
  {
    binary += std::bitset<8>(c).to_string() + ' ';
  }
  return binary;
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

void callback(char *topic, byte *payload, unsigned int length)
{
  DynamicJsonDocument doc(256);
  DeserializationError error = deserializeJson(doc, payload);

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
      // Subscribe
      mqttClient.setCallback(callback);
      mqttClient.subscribe("esp");
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
        if (IS_BINARY == 1)
          text = stringToBinary(text.c_str()).c_str();
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

        mqttClient.publish("esp", "esp32 " + IS_BINARY);
        String text = serialize();
        if (IS_BINARY == 1)
          text = stringToBinary(text.c_str()).c_str();
        mqttClient.publish("esp32", text.c_str());
      }
    }
  }
}

void setup()
{
  Serial.begin(115200);
  delay(1000);

  mqttClient.setServer(mqtt_server, 1883);
  loadConfigFromEEPROM();
  initBuffer();

  tsStart = getTime();
}

void loop()
{

  int64_t diff = getTime() - tsStart;
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