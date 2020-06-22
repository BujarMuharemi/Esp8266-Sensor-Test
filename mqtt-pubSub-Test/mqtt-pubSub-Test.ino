/*
  Basic ESP8266 MQTT example
  This sketch demonstrates the capabilities of the pubsub library in combination
  with the ESP8266 board/library.

  It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off

  It will reconnect to the server if the connection is lost using a blocking
  reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
  achieve the same result without blocking the main loop.

  To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"

*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>

Adafruit_BMP280 bmp; // use I2C interface
Adafruit_Sensor *bmp_temp = bmp.getTemperatureSensor();
Adafruit_Sensor *bmp_pressure = bmp.getPressureSensor();

#include "DHT.h"
#define DHTPIN 13     // D7@WEMOS
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
DHT dht(DHTPIN, DHTTYPE);



// Update these with values suitable for your network.
const char* ssid = "<___WIFI___>";
const char* password = "BujarResul2017@";

const char* mqtt_server = "34.107.25.251";
String clientId = "clientId-0SE60tkswF";
int portNum = 1999;

int sampleTime = 10000;

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;



void setup_bmp280() {
  Serial.println(F("BMP280 Sensor event test"));

  if (!bmp.begin()) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
    while (1) delay(10);
  }

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

  //bmp_temp->printSensorDetails();
}

float get_bmp280_data(String type) {
  sensors_event_t temp_event, pressure_event;
  bmp_temp->getEvent(&temp_event);
  bmp_pressure->getEvent(&pressure_event);

  if (type.equals("temperature")) {
    return temp_event.temperature;
  } else if (type.equals("pressure")) {
    return pressure_event.pressure;
  } else {
    //returns absolute zero, as "error"
    return -273.15;
  }

}

float get_dht22_data(String type) {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return -273.15;
  }
  float hit = dht.computeHeatIndex(t, h);

  if (type.equals("temperature")) {
    return t;
  } else if (type.equals("humidity")) {
    return h;
  } else if (type.equals("heatindex")) {
    return hit;
  } else {
    //returns absolute zero, as "error"
    return -273.15;
  }

}

void publish_float(float value, const char* topic) {  
  Serial.println("Published: "+String(value)+" @Topic: "+topic);
  dtostrf(value, 6, 2, msg);
  client.publish(topic, msg);
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    //clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("connected", "hello world");
      client.subscribe("bmp280/temperature");
      client.subscribe("bmp280/humidity");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//--------------MAIN SETUP
void setup() {
  //pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  setup_bmp280();
  dht.begin();
  client.setServer(mqtt_server, portNum);
  //client.setCallback(callback); not needed as we just send data
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > sampleTime) {

    lastMsg = now;
    //++value;
    //snprintf (msg, 50, "hello world #%ld", value);
    //Serial.print("Publish message: ");
    /*
      float temperature = dht.getTemperature();
      dtostrf(temperature, 6, 2, msg);
      Serial.println(msg);
      client.publish("temp", msg);

      float humidity = dht.getHumidity();
      dtostrf(humidity, 6, 2, msg);
      Serial.println(msg);
      client.publish("humidity", msg);
    */
    
    publish_float(get_bmp280_data("temperature"),"bmp280/temperature");        
    publish_float(get_bmp280_data("pressure"),"bmp280/pressure");

    publish_float(get_dht22_data("temperature"),"dht22/temperature");
    publish_float(get_dht22_data("humidity"),"dht22/humidity");
    publish_float(get_dht22_data("heatindex"),"dht22/heatindex");

  }
}
