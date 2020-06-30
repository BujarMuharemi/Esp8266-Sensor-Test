
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

#define PIN 12
#define NUMPIXELS 16 

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
#define DELAYVAL 500 // Time (in milliseconds) to pause between pixels

// Update these with values suitable for your network.

const char* ssid = ".........";
const char* password = "............";
const char* mqtt_server = "34.107.25.251";
String clientId = "Led-Streifen";
int portNum = 1999;

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
int rgb[3]={70,70,70};


void updateLedColor(){
    //farbe anhand der bekommen werde verandern
    for(int i=0; i<NUMPIXELS; i++) { 
      pixels.setPixelColor(i, pixels.Color(rgb[0],rgb[1],rgb[2]));
      pixels.show();
    }
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

//wird aufgerufen wenn eine neue nachricht auf den gesubten topics kommt
void callback(char* topic, byte* payload, unsigned int length) {
  //Serial.print("Message arrived [");
  //Serial.print(topic);
  //Serial.print("] ");
  
  
//zum parsen der Farben
  if (strcmp(topic,"led1/r")==0){    
    String value="";
    for (int i = 0; i < length; i++) {      
      value+=(char)payload[i];
    }        
    Serial.println(value.toInt());
    rgb[0]=value.toInt();
    updateLedColor();
    
  }
  else if(strcmp(topic,"led1/g")==0){
    String value="";
    for (int i = 0; i < length; i++) {      
      value+=(char)payload[i];
    }        
    Serial.println(value.toInt());
    rgb[1]=value.toInt(); 
    updateLedColor();   
  }  
  
  else if(strcmp(topic,"led1/b")==0){    
    String value="";
    for (int i = 0; i < length; i++) {      
      value+=(char)payload[i];
    }     
    Serial.println(value.toInt());
    rgb[2]=value.toInt();
    updateLedColor();
  }
  
  //Die Ledstreifen an/aus schalten
  else if (strcmp(topic,"led1/state")== 0) { 
    if((char)payload[0] == '1'){ 
       for(int i=0; i<NUMPIXELS; i++) { // For each pixel...
        // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255       
        pixels.setPixelColor(i, pixels.Color(rgb[0],rgb[1],rgb[2]));
        pixels.show();   // Send the updated pixel colors to the hardware. 
       }   
    }else{
      pixels.clear();
      pixels.show();
    }   
  }
        
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    //String clientId = "ESP8266Client-";
    //clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("led1/state");
      client.subscribe("led1/r");
      client.subscribe("led1/g");
      client.subscribe("led1/b");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, portNum);
  client.setCallback(callback);
  pixels.begin();
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf (msg, 50, "hello world #%ld", value);
    //Serial.print("Publish message: ");
    //Serial.println(msg);    
  }
}
