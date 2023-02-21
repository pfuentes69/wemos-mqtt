#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const int buttonPin = D3;

const char* ssid     = "Papillon_EXT"; // "WIFI-WISeKey";
const char* password = "70445312"; // "75468471810334508893";

int status = WL_IDLE_STATUS;   // the Wifi radio's status

IPAddress broker(54, 194, 211, 75); // abxvtnyocolvw-ats.iot.eu-west-1.amazonaws.com
const char* AWSEndpoint = "abxvtnyocolvw-ats.iot.eu-west-1.amazonaws.com";

const char*  certificatePemCrtRSA = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDWTCCAkGgAwIBAgIUekWuZOKzzh4PrrPy6xkMSJRFjOIwDQYJKoZIhvcNAQEL" \
"BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g" \
"SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTIwMDUxNDEyNTY0" \
"OFoXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0" \
"ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKcPmO0WRtWZ3p7JvCOH" \
"qaDYZ7NHZBDeItpPE1cZZjdZYijLCOxjkN5wVOqEiB/4e9tUxgYMD7rdD5B390bt" \
"SDZ1YQmHdG/ShlRlaBoc4oef7K0SVPRyxVAZF4klzg5hyYunTkPDZj51pxc17RlS" \
"s91G7S8ZQlrM1ARM2u/33ZHd8teX01b+KAkG2y+hHYEYuRI46d8FVPkppNbMRQqy" \
"dBBh57EYEdvu7wK5rk5o0D24/C9pxvR7pz5quNwG80k4LdOcLUTd4Dz/oWPEGvHk" \
"CASq2DeXP4HD5gLfsAschzp+v/Lz7wztuWM3PRBGhnu0gowmxN55PyKlfxm3PRSw" \
"yI8CAwEAAaNgMF4wHwYDVR0jBBgwFoAUGTtRs9fv+J2gkVEDgsrigaQKtBIwHQYD" \
"VR0OBBYEFIgMtX/ctDcjhtxBiTi5/TMkAKQbMAwGA1UdEwEB/wQCMAAwDgYDVR0P" \
"AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQC/B9goXZjGYxMhXE/SdAScu57y" \
"lrK/t5xchB4BF3P7gGCmsai8Y5cAwIXgfV5lN1LKnAZEWHGGByoo7YKUOlbUC1CY" \
"rREzOgynlc4R4pTROyhT+lgjNrJGo4E8frxwRV+QKfN2Wf3VlCdgwmKfsfv2bnXU" \
"ARaFCZqQzt8ZLc5t5cNjjUmHN2e/tj7OoFgWcntHCLN7cSver7JvyVJv6uB3NG23" \
"NHvLUTrT/NAS8sKif3ZdiEF+EaorK7wjPEwftEQPF1Az4F/nt8wHZ1SWqVc6P6Fa" \
"UrNBNYFJ3GdUp4Eqdrq0zgsY6si+keLdPNEPtlILg4vnRYlqZGlhaCs36ASx\n" \
"-----END CERTIFICATE-----\n";

const char*  privatePemKeyRSA = \
"-----BEGIN RSA PRIVATE KEY-----\n" \
"MIIEogIBAAKCAQEApw+Y7RZG1Znensm8I4epoNhns0dkEN4i2k8TVxlmN1liKMsI" \
"7GOQ3nBU6oSIH/h721TGBgwPut0PkHf3Ru1INnVhCYd0b9KGVGVoGhzih5/srRJU" \
"9HLFUBkXiSXODmHJi6dOQ8NmPnWnFzXtGVKz3UbtLxlCWszUBEza7/fdkd3y15fT" \
"Vv4oCQbbL6EdgRi5Ejjp3wVU+Smk1sxFCrJ0EGHnsRgR2+7vArmuTmjQPbj8L2nG" \
"9HunPmq43AbzSTgt05wtRN3gPP+hY8Qa8eQIBKrYN5c/gcPmAt+wCxyHOn6/8vPv" \
"DO25Yzc9EEaGe7SCjCbE3nk/IqV/Gbc9FLDIjwIDAQABAoIBAEKLYvXlbre8v8Fu" \
"SAO7ESVhrgTqhgB2C1n8L479LgsUDpaDMX2/tz/zbM+xlOtvNh7KqMpV2ZosXfvE" \
"3XmiIKaYoNuD2iyEpj9N2Wa1ZMJzQHo8GBz67n+WTxqxNV/jMb3wGavCVKLCiJkl" \
"QNlaaQzWKLofDKBQgI9p8beuetKT+lkgarEWaz6GbWrPOdcdvwXZTaRzXE015S8p" \
"02lxVl2rvfkik2BuFhsPDB19SJaKPYR8cmGpHZDwX64eiVVpialMb/d0hGtAn3Zz" \
"BTx7aQ9o1LMdFlC7Liyfw95rg3BPxUWT17zUmDiTNfOJM4D0DudAfU7LDZINyhtW" \
"5Vev7WkCgYEA0RxDQL/mepi2D+kFOFh+GicVf0CDh7cLJPydQweexsrvdkpd74HT" \
"mkmYDvOnCUbn+bz/RG/GNab9d1IQPuR6M4rVgR7z4fDUKlL/IaVoEj7lY7wlp1V7" \
"H1afZ4+nV6PC397Z9J2OEJ4mN7Z6Wr98QDiRYFXsAxv+nxNOtmkP8vUCgYEAzIWI" \
"r7h0CsmvmDNzUa8xcbRYkCu9rto0/eUyP4kihp0gWJUEt8E1UqcObeoYatVUKWu8" \
"9vDSNEBBxwpb1c4Q0pvMH062339XB1G0I5lrid9yKDeapnaBVNSdjua6VVD6U9do" \
"WWAfCJkFYpqO+kJ5ybFtq7Jfg0T/lWVa9PgCQvMCgYA/ySY+nwrYBLMsgUEFYgD9" \
"S0TEb1Jv2Ib+vkveQXnOW+LVq3Oh9nEslBxdGzetncJvLJaVMp88iHayqgaomJsq" \
"E8RywZVVK1gcnPqUMddgEW15kc/OjkWjVpIDTg+WrS5piZnkgxbtvMAdqH0EJ3ro" \
"QBkgULVQcX6m2YXeIIgr7QKBgCLHvqPrYUiIXeUrMrw8Z9MnUTxLQ/mdQA/BT1dA" \
"se9kfyCxTtkU8UV6BVkpyzc3yhU1LjBsacLa/pSjrVRhs7itJ/xW/YBqfllPSqwX" \
"JhOPPTGbqyAN3RaZBaZMlHl3yOpDIoq4bu6eXy0Sjaf/cAidtMHTFq0TKce1Mc+g" \
"8XmDAoGAJf0qfKnJvd60ItaYx3PJ8JvOvwThC0LlUo0m6HNn4aO9At6Go2VsrSLp" \
"0dfrrb+VASoPhZocSr6QkCVGKs9Fw77kzqA6ydJML+Hn1mMK3/6mzds7z5K009fw" \
"B9uBjYTrYCmysZdRgeeS0UXxLK7E2SD+NDD/8mY/NLYWY3iO7cU=\n" \
"-----END RSA PRIVATE KEY-----\n";

const char*  certificatePemCrtEC = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIBojCCAUigAwIBAgIUUGdB/sKBxsa+eBGEyV67t1a8LvMwCgYIKoZIzj0EAwIw" \
"OzEVMBMGA1UEAwwMREVWSU9UU1VCQ0ExMRUwEwYDVQQKDAxXSVNFS0VZIERFTU8x" \
"CzAJBgNVBAYTAkNIMB4XDTIwMDUxMzE0MzcyNloXDTIyMDUxMzE0MzcyNlowPjEa" \
"MBgGA1UEAwwRY2VydC10ZXN0cGYtZGV2MDExEzARBgNVBAoMCldJU2VLZXkgU0Ex" \
"CzAJBgNVBAYTAkNIMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAENwAWEAeLzFUN" \
"QH3yPIFiCPBq6jNhWiqsVWmmTF5b7hVZR6R8x5kqOeR+XEDfUNLNBCOV9szqf9uY" \
"pkuSG90mOqMnMCUwEwYDVR0lBAwwCgYIKwYBBQUHAwIwDgYDVR0PAQH/BAQDAgWg" \
"MAoGCCqGSM49BAMCA0gAMEUCIAqAJEDL+F5zMaIis6JDA/suAn+CPoHTgYWCvK5H" \
"U0wiAiEAiELhEW3LPSfwBENy0TB43uOCkgerzcCKi10bjNsEofs=\n" \
"-----END CERTIFICATE-----\n";

const char*  privatePemKeyEC = \
"-----BEGIN EC PRIVATE KEY-----\n" \
"MHcCAQEEIIZ4et98OATXVGc3euUoESuG8tm/aUp1x9L6ap2wxYQJoAoGCCqGSM49" \
"AwEHoUQDQgAENwAWEAeLzFUNQH3yPIFiCPBq6jNhWiqsVWmmTF5b7hVZR6R8x5kq" \
"OeR+XEDfUNLNBCOV9szqf9uYpkuSG90mOg==\n" \
"-----END EC PRIVATE KEY-----\n";

/**
 * Add your ssl certificate and ssl key to the data folder. (You can remove the example files.)
 * After adding the files, upload them to your ESP8266 with the following terminal command:
 *
 * platformio run --target uploadfs
 */
const char* devID = "testpf-awsca-dev01";
// const char* devUS = "dev02";
// const char* devPW = "dev02";

const char* devTopic = "PapillonIoT/TestSensor/+";

const char* certFile = "/cert.pem";
const char* keyFile = "/private.key";

// The following two variables are used for the example outTopic messages.
unsigned int counter = 0;
unsigned long lastPublished = 0;

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


void createTLSFiles() {
  Serial.println(__FUNCTION__);
  // create certificate file
  File fc = SPIFFS.open(certFile, "w");
  if (!fc)
    Serial.println("File '/cert.pem' open failed.");
  else {
    fc.print(certificatePemCrtEC);
    fc.close();
  }

  // create key file
  File fk = SPIFFS.open(keyFile, "w");
  if (!fk)
    Serial.println("File '/private.key' open failed.");
  else {
    fk.print(privatePemKeyEC);
    fk.close();
  }
}

void setup() {
  // initialize serial for debugging
  Serial.begin(115200);

  // initialize WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  //connect to MQTT server
  client.setServer(AWSEndpoint, 8883);
  client.setCallback(callback);
// Mount file system.
  if (!SPIFFS.begin()) 
    Serial.println("Failed to mount file system");
  else
    createTLSFiles();

  // Allows for 'insecure' TLS connections. Of course this is totally insecure,
  // but seems the only way to connect to IoT. So be cautious with your data.
  WIFIclient.setInsecure();
  // Read the SSL certificate from the spiffs filesystem and load it.
  File cert = SPIFFS.open(certFile, "r");
  if (!cert) Serial.println("Failed to open certificate file: " + String(certFile));
  if(WIFIclient.loadCertificate(cert)) Serial.println("Certificate loaded!");
  // Read the SSL key from the spiffs filesystem and load it.
  File key = SPIFFS.open(keyFile, "r");
  if (!key) Serial.println("Failed to open private key file: " + String(keyFile));
  if(WIFIclient.loadPrivateKey(key)) Serial.println("Private key loaded!");

  // Configure button port
  pinMode(buttonPin,  INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (!client.connected()) {
    reconnect();
  } else {
    int reading = digitalRead(buttonPin);
    if (reading == LOW) {
      Serial.println("PUSH!");

      String payload = "{\"button\":";
        payload += 1;
        payload += "}";

      client.publish("PapillonIoT/TestSensor/data", (char*) payload.c_str());

      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
      digitalWrite(LED_BUILTIN, HIGH);
    }
  }
  client.loop();
  delay(100);
}