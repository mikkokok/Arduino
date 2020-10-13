#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

#define SEALEVELPRESSURE_HPA (1002.6)
const char* ssid = "";
const char* password = "";
const char* mqtt_server = "";
const char* mqtt_user = "";      
const char* mqtt_password = ""; 
const char* topic = "";    
const char* clientID = "";
const int sensorDelay = 300000;

WiFiClient espClient;
PubSubClient client(espClient);

Adafruit_BME280 bme; // I2C
 
void setup(void) {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW); 
  Serial.begin(9600);
    
  Serial.println("");
  Serial.print("Connecting to ");
  Serial.println(ssid);
 
  WiFi.begin(ssid, password);
  

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
    
  // Pins D1 and D2 in Wemos D1 mini
  Wire.begin(4, 5);
  if (!bme.begin(0x76)) {
    digitalWrite(LED_BUILTIN, LOW); 
     
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
    digitalWrite(LED_BUILTIN, HIGH);
  }
  client.setServer(mqtt_server, 1883);
  digitalWrite(LED_BUILTIN, HIGH);
} // setup
 
void loop(void) {  
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(clientID, mqtt_user, mqtt_password)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      Serial.print(client.state());
      delay(5000);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
  
  if (client.connected()) {
    Serial.println("Send message to MQTT queue");
    char temperatureResult[12];
    dtostrf(bme.readTemperature(), 4, 2, temperatureResult);
    char pressureResult[8];
    dtostrf(bme.readPressure(), 6, 2, pressureResult);
    char humidityResult[8];
    dtostrf(bme.readHumidity(), 4, 2, humidityResult);

    const size_t capacity = JSON_OBJECT_SIZE(20);
    DynamicJsonDocument doc(capacity);

    doc["SensorName"] = clientID;
    doc["Temperature"] = temperatureResult;
    doc["Pressure"] = pressureResult;
    doc["Humidity"] = humidityResult;

    char result[2150];
    serializeJson(doc, Serial);
    serializeJson(doc, result, 200);

    Serial.println("");
    if(client.publish(topic, result, false)) {
      Serial.println("Message sending succeeded");
      } else {
        Serial.println("Message sending failed");
      }
    }
   delay(sensorDelay);
} // loop
