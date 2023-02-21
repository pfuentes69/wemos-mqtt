#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <time.h>

// WiFi Credentials
const char* ssid = "Papillon";
const char* password = "70445312";

const char* mqtt_server = "raspberrypi2.local";

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

// Network objects
WiFiClientSecure wifiClient;

// Functions
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
  Serial.print(asctime(&timeinfo));
}

// Verify TLS Connection
void verifyTLS() {
  // Use WiFiClientSecure class to create TLS connection
  Serial.print("connecting to ");
  Serial.println(mqtt_server);
  if (!wifiClient.connect(mqtt_server, 8883)) {
    Serial.println("connection failed");
    return;
  }

  if (wifiClient.verify(BROKER_FINGERPRINT, mqtt_server)) {
    Serial.println("certificate matches");
  } else {
    Serial.println("certificate doesn't match");
  }
}

void setup() {
  Serial.begin(115200);

  setClock();

  wifiClient.setTrustAnchors(&caCert);         /* Load CA cert into trust store */
//  wifiClient.allowSelfSignedCerts();               /* Enable self-signed cert support */
  wifiClient.setFingerprint(BROKER_FINGERPRINT);  /* Load SHA1 mqtt cert fingerprint for connection validation */
  wifiClient.setClientECCert(&deviceCert, &deviceKey, BR_KEYTYPE_KEYX | BR_KEYTYPE_SIGN, BR_TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256);
//  wifiClient.setCertificate(&deviceCert);
//  wifiClient.setPrivateKey(&deviceKey);


  verifyTLS();

}

void loop() {
  // put your main code here, to run repeatedly:
}