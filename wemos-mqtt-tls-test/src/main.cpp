#include <Arduino.h>
#include <ESP8266WiFi.h> // #include <WiFi.h> // For ESP32
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "DHTesp.h"
#include <Adafruit_GFX.h>
#include "Adafruit_SSD1306.h"

// 
// CONSTANTS
//

// TLS Config
// Test CA
static const char CA_CERT[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----
)EOF";
X509List caCert(CA_CERT);  // Only with BearSSL (ESP8266)

// Broker Certificate
// NOT USED!!!
//static const char BROKER_CERT[] PROGMEM = R"EOF(
//-----BEGIN CERTIFICATE-----
//MIICHTCCAcSgAwIBAgIJANDeS8aKP/jfMAoGCCqGSM49BAMCMDUxCzAJBgNVBAYT
//AkNIMRAwDgYDVQQKDAdOYXZpdGVyMRQwEgYDVQQDDAtQYXBpbGxvbiBDQTAeFw0y
//MDA1MjQxNjEyNDhaFw0zMDA1MjIxNjEyNDhaMFExCzAJBgNVBAYTAkNIMQswCQYD
//VQQIDAJWRDEPMA0GA1UEBwwGVGFubmF5MRAwDgYDVQQKDAdOYXZpdGVyMRIwEAYD
//VQQDDAlsb2NhbGhvc3QwWTATBgcqhkjOPQIBBggqhkjOPQMBBwNCAAQ8Sx/OUcSj
//u3+Onl7DYkfnKylDHrkAK0kmDUFNNgd3kcUYyV/t3FTHTSvZl9OarZR+NX/2Ofbv
//icGl+HNhUZWro4GgMIGdMAkGA1UdEwQCMAAwDgYDVR0PAQH/BAQDAgWgMB0GA1Ud
//JQQWMBQGCCsGAQUFBwMBBggrBgEFBQcDAjBhBgNVHREEWjBYgglsb2NhbGhvc3SC
//EXJhc3BiZXJyeXBpLmxvY2FsghJyYXNwYmVycnlwaTIubG9jYWyCEnJhc3BiZXJy
//eXBpMy5sb2NhbIcEfwAAAYcEwKgCZYcEwKgCZzAKBggqhkjOPQQDAgNHADBEAiBI
//udJepUsbBGfaZjFe3NFgx0I5LVi6bCVBR4PSTl5rwQIgDG3De12quZZHI0sue0yu
//je9hzBSokGAcC5gmxDfJALg=
//-----END CERTIFICATE-----
//)EOF";
//X509List brokerCert(BROKER_CERT);  // Only with BearSSL (ESP8266)

// OPTIONAL: Extract fingerprint of the broker certificate, for connection validation - NOT USED
//const char* BROKER_FINGERPRINT = "60:5F:7D:AE:73:0E:BC:5A:E2:E7:BC:13:A7:49:34:CB:27:09:CD:C0";


// Device Client Certificate
static const char DEVICE_CERT[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDWjCCAkKgAwIBAgIVAOYloe5cvDoZ1RzCJDfqq0tRHrlCMA0GCSqGSIb3DQEB
CwUAME0xSzBJBgNVBAsMQkFtYXpvbiBXZWIgU2VydmljZXMgTz1BbWF6b24uY29t
IEluYy4gTD1TZWF0dGxlIFNUPVdhc2hpbmd0b24gQz1VUzAeFw0yMzAyMTkwODI2
MzVaFw00OTEyMzEyMzU5NTlaMB4xHDAaBgNVBAMME0FXUyBJb1QgQ2VydGlmaWNh
dGUwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDGh02tCF7Eci95e/q0
2XF3r+vGAxZNKxKnRD6XnpGpTVkQEQrDsbm6EEaFFzaOW6gG0gOmZiWSSUdqzC9T
JVfaSWZ6pYCVS/NTWP5l14OrgkG5zOObxc1lwZTPeJlHJjrwphkpdYMJpub/kDyF
uMer7EnVXSYHSpd3ZOXkRpqU2eNXfjVESDH3Szvw814phXTrTUbkj0k8GPtjNrTo
tkDTBoeft2UVYcaeKnF6/O7vaLqjxNxeUYpE55lA34uObA8XXl29menq2z4qY9qB
hMBcsHc7BVPxIy8PrRbaaAOuhv1pySs6Hy9AO4OQczV3/I6u/B6oq4zC3S8gFkzf
s4tRAgMBAAGjYDBeMB8GA1UdIwQYMBaAFKJULs07R5ECXdrFrJ0DTdkstW1pMB0G
A1UdDgQWBBTn/dAZBOBYVJrG9NmIlv8Mje2EnjAMBgNVHRMBAf8EAjAAMA4GA1Ud
DwEB/wQEAwIHgDANBgkqhkiG9w0BAQsFAAOCAQEAOucR7r1jtshgGBOcK2OsGATm
wq0YtlOZRyM2pxrXi+0IUM3aME3ezTj/lokZqOB8y85nI7r5+c30+jgP7O4zJibe
QKiuJNSHgnPcXHZZ56ogIrfvsFDFuF5yOr/IMp4ps48SNjMHWhHdLo289lkmyq2v
OybG5AUhRZCAKLGn2KvJcoD96RE8C75lDmx5nHgiY6g/K+u+QKDO8cUxeZElXcjf
P8pW0a2u2XIH82SLAOQ8jABTwfVYGKJaKFjUufTJLtC2k/bmR+U7yEll6OkPY6Ia
4QxRLQNp58grx+M0d46IGURk4QfnK8hBZI5HJs5xwFwPiuYXyqtg4I13DkDFBw==
-----END CERTIFICATE-----
)EOF";
X509List deviceCert(DEVICE_CERT);  // Only with BearSSL (ESP8266)

// Device Private Key
static const char DEVICE_PRIVATE[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
MIIEogIBAAKCAQEAxodNrQhexHIveXv6tNlxd6/rxgMWTSsSp0Q+l56RqU1ZEBEK
w7G5uhBGhRc2jluoBtIDpmYlkklHaswvUyVX2klmeqWAlUvzU1j+ZdeDq4JBuczj
m8XNZcGUz3iZRyY68KYZKXWDCabm/5A8hbjHq+xJ1V0mB0qXd2Tl5EaalNnjV341
REgx90s78PNeKYV0601G5I9JPBj7Yza06LZA0waHn7dlFWHGnipxevzu72i6o8Tc
XlGKROeZQN+LjmwPF15dvZnp6ts+KmPagYTAXLB3OwVT8SMvD60W2mgDrob9ackr
Oh8vQDuDkHM1d/yOrvweqKuMwt0vIBZM37OLUQIDAQABAoIBAERmkciwPZaJkPnW
UYbF9GY8YouSvLhRwteGl9o9dApU7/6tIHxUW51TxjhMyys5QSc4bLGEOCWywbx2
JUgMcbxv1bMkGq1VwbxRyTTlkCQjeOm/QhyFhkMs80yYTJdLALuxPrIeHsvQiX47
/95Cz+jYPIga0+NnO9CTHvILsW7fF3v3TwzrtKP35nt66+v2+/E8EOOwALBax9wl
jpKztfgqrpzbpeVuLJi/pl6l8tIaRva2cAreyYe+fVRmXaOoJ45PI1KvxvL3uw1l
5Vdp+KInpfkCGZ+l5Xm+8oHDaXuh+fqMzKFzJ/7eMRMTQO8MvLQZ+fWVtm9Xezcd
mqxfDFECgYEA9/FEACrFJtNHAuNKdRLCzOrJVtjSvO63fAZ8CKnU3oL9NeYajWen
iNhLZdOpw7kqsfP1fSfDh57Q8Pk/UMPx8npFGOG34CjOY9WiKgUmEhghkVJ7R8OI
PhoZp0qrhfKmBCJeYT4UEUBJZrkt4pLXU7zN/cfk37dKsNQhzxmlxvcCgYEAzPrx
ejXTBeJAyFITq74/9YCg8ADXqbrzv5b82lFyrWnVRXxdXebpsxrx4C6wzLPCIEHp
U399ltLNocMSWiZtGvQA/QEI4miXAtDStbtHJMP89fRriulEt+nHuKhLTnHfuyTI
oI37Uaz53j5khkAZK4R+a3AtE2NNcR2kvvbgRfcCgYBqJIcP6z9rV5dWzSba56Ut
RHXdEAVnjGpQVhZTPyE6AKmVoWRMlMtL8gyVEv/iIir884tsgNFcBck83/cTBE9X
lrBt7nNhKxbv9kzt4DZkGKCGLeFI35Q7wcCptXWoajBYJL5ysri0iejiaPQTwvkb
iABHXIPGohj/ilDO8BPgEQKBgH+t1/hdyoIhumSM63/Q05oicxzMCYnJz8aJUBrV
HO9zusq7epThHrTXlc4khzAa0FKze5jtEtd9m2SlU6sfwFpHns35dQVCTL4OQRG+
A8Sc59Z0VwCB6LH+Rs941N+aAthZQzbu+ZqYelO63XbaOlNnBi/6AbnP4w8QfewN
bwllAoGADTAdtA2tE4/KllPdHwSu1ODY/xY520WYB6ZH+ULYP3c28FJdOxoskFLs
v5H1U6wD4vABrtnRTRwOfrgLR41vZS9UwhrljKXdTANupqjXa8k0hnGVghacCOXB
I8q0+Mplces3YoWazw63CWe4PXq+jauJVRVKGtJpqDN/OjtAicQ=
-----END RSA PRIVATE KEY-----
)KEY";
PrivateKey deviceKey(DEVICE_PRIVATE);  // Only with BearSSL (ESP8266)

const char* devID = "aws-device-001";
// const char* devUS = "dev01";
// const char* devPW = "dev01";

const char* devTopic = "aws-device-001/data";

// Replace the next variables with your SSID/Password combination
const char* ssid     = "Papillon"; // "WK-Guest"; 
const char* password = "70445312"; // "Trust4All!";

// Add your MQTT Broker IP address, example:
//const char* mqtt_server = "192.168.1.144";
const char* mqtt_server = "aofwbgp3py5sf-ats.iot.eu-west-1.amazonaws.com";
//IPAddress brokerIP(192, 168, 2, 103); // abxvtnyocolvw-ats.iot.eu-west-1.amazonaws.com
const char* brokerSNI = "aofwbgp3py5sf-ats.iot.eu-west-1.amazonaws.com";

// const int status = WL_IDLE_STATUS;   // the Wifi radio's status

const int buttonPin = D3;

//
// GLOBAL VARIABLES
//

// The following two variables are used for the example outTopic messages.
unsigned int counter = 0;
unsigned long lastPublished = 0;

// WiFiClient espClient;
WiFiClientSecure WIFIclient;

// PubSubClient client(espClient);
PubSubClient client(WIFIclient);

long lastMsg = 0;
char msg[50];
int value = 0;

String payload;

DHTesp dht;

#define OLED_RESET 0  // GPIO0
Adafruit_SSD1306 display(OLED_RESET);

//
// WIFI SETUP
//
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

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
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


//
// SUBSCRIBE CALLBACK
// Print any message received for subscribed topic
//
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

//
// RECONNECT TO BROKER
//
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(devID)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(devTopic,"{\"status\":\"Connected!\"}");
      // ... and resubscribe
//      client.subscribe(devTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//
// BOARD SETUP
//
void setup() {
  Serial.begin(115200);
  dht.setup(D4, DHTesp::DHT11); // Connect DHT sensor to GPIO 17
  pinMode(buttonPin,  INPUT_PULLUP);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 64x48)
  display.setTextSize(1);
  display.setTextColor(WHITE);

  // Config WiFi
  display.clearDisplay();
  display.setCursor(0,0);
  display.print("Init WiFi");
  display.display();
  setup_wifi();

  // Set time
  display.clearDisplay();
  display.setCursor(0,0);
  display.print("Set time");
  display.display();
  setClock();

  // Init MQTT
  display.clearDisplay();
  display.setCursor(0,0);
  display.print("Init MQTT");
  display.display();

// This doesn't work with ESP32, seems related to BearSSL
  // Load CA cert into trust store
  WIFIclient.setTrustAnchors(&caCert);
// Alternative using BearSSL (ESP8266)
  //  WIFIclient.setCACert(CA_CERT);


//  Enable self-signed cert support
  //  WIFIclient.allowSelfSignedCerts();

// Optional: Skip CA Validation
  //  WIFIclient.setInsecure();

//  WIFIclient.setCertificate(DEVICE_CERT); // ESP32
//  WIFIclient.setPrivateKey(DEVICE_PRIVATE); //ESP32

// Alternatives using BearSSL, to be checked why is different with ESP32
  WIFIclient.setClientRSACert(&deviceCert, &deviceKey);

// Alternative for EC crypto, to be tested again
  //  WIFIclient.setClientECCert(&deviceCert, &deviceKey, BR_KEYTYPE_KEYX | BR_KEYTYPE_SIGN, BR_TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256);

// Optional: Broker certificate validation
  //  WIFIclient.setFingerprint(BROKER_FINGERPRINT);

  //connect to MQTT server
  client.setServer(brokerSNI, 8883);
// Alternative for non-TLS connections
  //  client.setServer(mqtt_server, 1883);

// Configure SUB callback
  client.setCallback(callback);

}

// 
// MAIN PROCESS LOOP
//
void loop() {
  float humidity;
  float temperature;
  static bool buttonPressed = false;
  //  delay(dht.getMinimumSamplingPeriod());

  if (!client.connected()) {
    reconnect();
  } else {
    client.loop();
    // Only publish in 5 sec. intervals
    long now = millis();
    if (now - lastMsg > 5000) {

      humidity = dht.getHumidity();
      temperature = dht.getTemperature() * 0.9;
      lastMsg = now;
      
      // Publish message
      payload = "{\"temperature\":" + String(temperature) + ", \"humidity\":" + String(humidity) + "}";
      client.publish(devTopic, (char*) payload.c_str());
      Serial.println("Env. data publish");

      // Update display
      display.clearDisplay();
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

    if (digitalRead(buttonPin) == LOW) {
      if (!buttonPressed) {
        buttonPressed = true;
        payload = "{\"button\": \"true\" }";
        client.publish(devTopic, (char*) payload.c_str());
        Serial.println("Button down");
      }
    } else {
      if (buttonPressed) {
        buttonPressed = false;
        payload = "{\"button\": \"false\" }";
        client.publish(devTopic, (char*) payload.c_str());
        Serial.println("Button up");
      }    
    }
  }
}