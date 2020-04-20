#include "DHTesp.h"

#ifdef ESP32
#pragma message(THIS EXAMPLE IS FOR ESP8266 ONLY!)
#error Select ESP8266 board.
#endif

DHTesp dht;

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("Status\tHumidity (%)\tTemperature (C)\t(F)\tHeatIndex (C)\t(F)");
  String thisBoard= ARDUINO_BOARD;
  Serial.println(thisBoard);

  // Autodetect is not working reliable, don't use the following line
  // dht.setup(17);
  // use this instead: 
  dht.setup(4, DHTesp::DHT11); // Connect DHT sensor to GPIO D2 !

  // Connect D0 to RST to wake up
  pinMode(D0, WAKEUP_PULLUP);
}

void loop()
{
  //delay(dht.getMinimumSamplingPeriod());
  delay(2000);
  float humidity = dht.getHumidity();
  float temperature = dht.getTemperature();

  Serial.print(dht.getStatusString());
  Serial.print("\t");
  Serial.print(humidity, 1);
  Serial.print("\t\t");
  Serial.print(temperature, 1);
  Serial.print("\t\t");  
  Serial.print("\t\t");
  Serial.println(dht.computeHeatIndex(temperature, humidity, false), 1);
  //ESP.deepSleep(5 * 1000000);
}

