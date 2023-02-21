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

// TLS Config
// Test CA
static const char CA_CERT[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIBWzCCAQACCQCYlqYv5TXAvzAKBggqhkjOPQQDAjA1MQswCQYDVQQGEwJDSDEQ
MA4GA1UECgwHTmF2aXRlcjEUMBIGA1UEAwwLUGFwaWxsb24gQ0EwHhcNMjAwNTI0
MTMxNzQ5WhcNMzAwNTIyMTMxNzQ5WjA1MQswCQYDVQQGEwJDSDEQMA4GA1UECgwH
TmF2aXRlcjEUMBIGA1UEAwwLUGFwaWxsb24gQ0EwWTATBgcqhkjOPQIBBggqhkjO
PQMBBwNCAATxCFUBwwxTB/eJgKsqvU8qMSavDyJ7dKggfdmEXJkV6qLsC0k0724n
PTZfd08Xl1hEcxGO+TCD+5RUBUxXzczVMAoGCCqGSM49BAMCA0kAMEYCIQC0vLpo
eeJzOAst6+0PA3N+6HORYESRuV8LMEsZUH1w7wIhAIVwro9I4wtNJLQoKNMyjNyh
w1Q1UdtpY8Pj2pPKThNu
-----END CERTIFICATE-----
)EOF";
X509List caCert(CA_CERT);  

// Broker Certificate
static const char BROKER_CERT[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIICHTCCAcSgAwIBAgIJANDeS8aKP/jfMAoGCCqGSM49BAMCMDUxCzAJBgNVBAYT
AkNIMRAwDgYDVQQKDAdOYXZpdGVyMRQwEgYDVQQDDAtQYXBpbGxvbiBDQTAeFw0y
MDA1MjQxNjEyNDhaFw0zMDA1MjIxNjEyNDhaMFExCzAJBgNVBAYTAkNIMQswCQYD
VQQIDAJWRDEPMA0GA1UEBwwGVGFubmF5MRAwDgYDVQQKDAdOYXZpdGVyMRIwEAYD
VQQDDAlsb2NhbGhvc3QwWTATBgcqhkjOPQIBBggqhkjOPQMBBwNCAAQ8Sx/OUcSj
u3+Onl7DYkfnKylDHrkAK0kmDUFNNgd3kcUYyV/t3FTHTSvZl9OarZR+NX/2Ofbv
icGl+HNhUZWro4GgMIGdMAkGA1UdEwQCMAAwDgYDVR0PAQH/BAQDAgWgMB0GA1Ud
JQQWMBQGCCsGAQUFBwMBBggrBgEFBQcDAjBhBgNVHREEWjBYgglsb2NhbGhvc3SC
EXJhc3BiZXJyeXBpLmxvY2FsghJyYXNwYmVycnlwaTIubG9jYWyCEnJhc3BiZXJy
eXBpMy5sb2NhbIcEfwAAAYcEwKgCZYcEwKgCZzAKBggqhkjOPQQDAgNHADBEAiBI
udJepUsbBGfaZjFe3NFgx0I5LVi6bCVBR4PSTl5rwQIgDG3De12quZZHI0sue0yu
je9hzBSokGAcC5gmxDfJALg=
-----END CERTIFICATE-----
)EOF";
X509List brokerCert(BROKER_CERT);

// Device Client Certificate
static const char DEVICE_CERT[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIBbzCCARSgAwIBAgIJANDeS8aKP/jgMAoGCCqGSM49BAMCMDUxCzAJBgNVBAYT
AkNIMRAwDgYDVQQKDAdOYXZpdGVyMRQwEgYDVQQDDAtQYXBpbGxvbiBDQTAeFw0y
MDA1MjUxOTQ3MDlaFw0zMDA1MjMxOTQ3MDlaMBAxDjAMBgNVBAMMBWRldjAxMFkw
EwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEFl4+xP3WSUnjl70JASPB+jervZxHA2cr
d3k+LM4Qs+jRLCYmrNBSo2V7hLhWAKNC2KlFO/6m0fo6SiqLMxtGV6MyMDAwCQYD
VR0TBAIwADAOBgNVHQ8BAf8EBAMCBaAwEwYDVR0lBAwwCgYIKwYBBQUHAwIwCgYI
KoZIzj0EAwIDSQAwRgIhAOmocpO4Gb5zaKTGisX8cU0n0/1Yk1ru6wAnqV4Qtsdk
AiEAmp/u2QCkoc1sfdDvDyNC9tWLqtyH4ad2/hJUpK6d36s=
-----END CERTIFICATE-----
)EOF";
X509List deviceCert(DEVICE_CERT);

// Device Private Key
static const char DEVICE_PRIVATE[] PROGMEM = R"KEY(
-----BEGIN EC PRIVATE KEY-----
MHcCAQEEIAuWeG9Avcsjo+ARACf8kiFrQZy/EacZQpWfKadthHTqoAoGCCqGSM49
AwEHoUQDQgAEFl4+xP3WSUnjl70JASPB+jervZxHA2crd3k+LM4Qs+jRLCYmrNBS
o2V7hLhWAKNC2KlFO/6m0fo6SiqLMxtGVw==
-----END EC PRIVATE KEY-----
)KEY";
PrivateKey deviceKey(DEVICE_PRIVATE);

const char* BROKER_FINGERPRINT = "60:5F:7D:AE:73:0E:BC:5A:E2:E7:BC:13:A7:49:34:CB:27:09:CD:C0";

const char* devID = "dev01";
// const char* devUS = "dev01";
// const char* devPW = "dev01";

const char* devTopic = "PapillonIoT/TestSensor/command";

// The following two variables are used for the example outTopic messages.
unsigned int counter = 0;
unsigned long lastPublished = 0;

// Multi AP Wifi utility class
ESP8266WiFiMulti wifiMulti;
// Initialize the Ethernet client object
WiFiClientSecure WIFIclient;
//PubSubClient client(AWSEndpoint, 8883, callback, espClient);
PubSubClient client(WIFIclient);

unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

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

  // Set time
  setClock();

  //connect to MQTT server
  client.setServer(brokerSNI, 8883);
  client.setCallback(callback);

  //WIFIclient.setInsecure();

  WIFIclient.setTrustAnchors(&caCert);         /* Load CA cert into trust store */
//  WIFIclient.allowSelfSignedCerts();               /* Enable self-signed cert support */
  WIFIclient.setFingerprint(BROKER_FINGERPRINT);  /* Load SHA1 mqtt cert fingerprint for connection validation */
  WIFIclient.setClientECCert(&deviceCert, &deviceKey, BR_KEYTYPE_KEYX | BR_KEYTYPE_SIGN, BR_TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256);
//  wifiClient.setCertificate(&deviceCert);
//  wifiClient.setPrivateKey(&deviceKey);

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
      payload = "{\"action\": \"PUSH!\"}";
      display.setCursor(0,20);
      client.publish("PapillonIoT/TestSensor/data", (char*) payload.c_str());
      display.print("PUSH!");
      display.display();
      delay(100);
      display.setCursor(0,20);
      display.print("        ");
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
          payload = "{\"temperature\":" + String(newTemperature) + ", \"humidity\":" + String(newHumidity) + "}";
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