#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <PubSubClient.h>
#include "DHTesp.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_SSD1306.h"

#define OLED_RESET 0  // GPIO0
Adafruit_SSD1306 display(OLED_RESET);

DHTesp dht;
long dhtInterval;
long pubInterval = 15000;

const int buttonPin = D3;

const char* ssid     = "Papillon_EXT"; // "WIFI-WISeKey";
const char* password = "70445312"; // "75468471810334508893";

int status = WL_IDLE_STATUS;   // the Wifi radio's status

IPAddress brokerIP(192, 168, 2, 103); // abxvtnyocolvw-ats.iot.eu-west-1.amazonaws.com
const char* brokerSNI = "raspberrypi2.local";


const char* devID = "d:dev01";
// const char* devUS = "dev01";
// const char* devPW = "dev01";

const char* devTopic = "PapillonIoT/TestSensor/command";

// The following two variables are used for the example outTopic messages.
unsigned int counter = 0;
unsigned long lastPublished = 0;

// Multi AP Wifi utility class
ESP8266WiFiMulti wifiMulti;
// Initialize the Ethernet client object
WiFiClient WIFIclient;
//PubSubClient client(AWSEndpoint, 8883, callback, espClient);
PubSubClient client(WIFIclient);

unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

bool errorStatus = false;

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection.......");
    // Attempt to connect, just a name to identify the client
    if (client.connect(devID)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("PapillonIoT/TestSensor/status","{\"status\":\"Connected!\"}");
      // ... and resubscribe
      client.subscribe(devTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//print any message received for subscribed topic
void callback(char* topic, byte* payload, unsigned int length) {
  String sTopic = topic;
  String sCommand = sTopic.substring(sTopic.lastIndexOf("/") + 1);

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  String sPayload = "";
  for (unsigned int i=0; i < length; i++) {
    sPayload += (char)payload[i];
  }
  Serial.println(sPayload);

  Serial.println("Command: " + sCommand);
}

// Set time via NTP, as required for x.509 validation
void setClock() {
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.println(asctime(&timeinfo));
}

void setup() {
  // initialize serial for debugging
  Serial.begin(115200);

  wifiMulti.addAP("Papillon", "70445312");
  wifiMulti.addAP("Papillon_EXT", "70445312");
  wifiMulti.addAP("InsecureWifi", "");

	Serial.println("Connecting Wifi...");
  while(wifiMulti.run() != WL_CONNECTED) {
      Serial.print(".");
      delay(1000);
  }
  Serial.println();
  if(wifiMulti.run() != WL_CONNECTED) {
    Serial.println("WiFi not connected!");
    delay(1000);
  } else {
      Serial.println("WiFi connected");
      Serial.print("AP : ");
      Serial.println(WiFi.SSID());
      Serial.print("IP address : ");
      Serial.println(WiFi.localIP());
  }

  //connect to MQTT server
  client.setServer(brokerSNI, 1883);
  client.setCallback(callback);

  // Configure button port
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);

  //
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 64x48)
  dht.setup(D4, DHTesp::DHT11); // Connect DHT sensor to GPIO 17
  dhtInterval = dht.getMinimumSamplingPeriod();
}

float humidity, newHumidity, temperature, newTemperature;
unsigned long previousMillis = 0;        // will store last time LED was updated
unsigned long lastPublish = 0;
String payload;

void loop() {
  // put your main code here, to run repeatedly:
  if (!client.connected()) {
    reconnect();
  } else {
    // Check button
    int reading = digitalRead(buttonPin);
    if (reading == LOW) {
      Serial.println("PUSH!");
      display.setCursor(0,20);
      if (errorStatus) {
        payload = "{\"command\":\"error\",\"value\":\"0\"}";
        display.print("ERROR OFF");
      } else {
        payload = "{\"command\":\"error\",\"value\":\"1\"}";
        display.print("ERROR ON");
      }
      errorStatus = !errorStatus;
      client.publish("PapillonIoT/command", (char*) payload.c_str());
      display.display();
      delay(100);
      display.setCursor(0,20);
      display.print("         ");
      display.display();
    }
    // Update DHT and display
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= dhtInterval) {
      // save the last time the DHT11 was read
      previousMillis = currentMillis;
      newHumidity = dht.getHumidity();
      newTemperature = dht.getTemperature() * 0.9;
      if (newHumidity > 0) {
        if ((newTemperature != temperature) || (newHumidity != humidity)) {
          humidity = newHumidity;
          temperature = newTemperature;
          display.clearDisplay();
          display.setTextSize(1);
          display.setTextColor(WHITE);
          display.setCursor(0,0);
          display.print("T = ");
          display.print(temperature,0);
          display.println(" C");
          display.setCursor(0,10);
          display.print("H = ");
          display.print(humidity,0);
          display.println(" %");
          display.display();
        }
        if (currentMillis - lastPublish >= pubInterval) {
          // save the last the publish was done
          lastPublish = currentMillis;
          payload = "{\"temperature\":\"" + String(newTemperature) + "\", \"humidity\":\"" + String(newHumidity) + "\"}";
          display.setCursor(0,20);
          client.publish("PapillonIoT/TestSensor/data", (char*) payload.c_str());
          display.print("PUBLISH!");
          display.display();
          delay(100);
          display.setCursor(0,20);
          display.print("        ");
          display.display();
        }
      }
    }
  }
  client.loop();
}