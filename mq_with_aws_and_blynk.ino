#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL3hRbHBK3a"
#define BLYNK_TEMPLATE_NAME "Bike Pollution Monitor"

#define BLYNK_DEVICE_NAME "Bike Pollution Monitor"
#define BLYNK_FIRMWARE_VERSION "1.0.0"

#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HardwareSerial.h>
#include <TinyGPS++.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

// OLED Display Settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Sensor & Relay Pins
const int MQ135_A0 = 32;
const int MQ9_A0 = 34;
const int RELAY_PIN = 25;

// Pollution Thresholds
const float MQ135_THRESHOLD = 1.5;
const float MQ9_THRESHOLD = 0.8;

// GPS Module Pins
#define RXD2 16
#define TXD2 17
HardwareSerial gpsSerial(1);
TinyGPSPlus gps;

// Wi-Fi Credentials
char auth[] = "4unYjiWnQ5K80E2Olh88hS_jYPPAj34i"; // Replace with your Blynk Token
const char* ssid = "OnePlus 7";
const char* password = "1234@567";  

// AWS IoT Core Settings
const char* mqtt_server = "a1s2te1nmhg9mc-ats.iot.eu-north-1.amazonaws.com";
const char* mqtt_topic = "bike/pollution";
WiFiClientSecure net;
PubSubClient client(net);

// SSL Certificates
const char* aws_cert_ca = "-----BEGIN CERTIFICATE-----\n"
"MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n"
"ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n"
"b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n"
"MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n"
"b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n"
"ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n"
"9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n"
"IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n"
"VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n"
"93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n"
"jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n"
"AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\n"
"A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\n"
"U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n"
"N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n"
"o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n"
"5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n"
"rqXRfboQnoZsG4q5WTP468SQvvG5\n"
"-----END CERTIFICATE-----\n";

const char* aws_cert_client = "-----BEGIN CERTIFICATE-----\n"
"MIIDWjCCAkKgAwIBAgIVANv1dvfN6DPsAKMfbTM9FZqnBXREMA0GCSqGSIb3DQEB\n"
"CwUAME0xSzBJBgNVBAsMQkFtYXpvbiBXZWIgU2VydmljZXMgTz1BbWF6b24uY29t\n"
"IEluYy4gTD1TZWF0dGxlIFNUPVdhc2hpbmd0b24gQz1VUzAeFw0yNTAzMTgwMjEw\n"
"MTNaFw00OTEyMzEyMzU5NTlaMB4xHDAaBgNVBAMME0FXUyBJb1QgQ2VydGlmaWNh\n"
"dGUwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDDfgrsDH2pMD95DGUi\n"
"8LUOLZIGeDUU1lab2Rv2BKGT/4CHzHOxLYAdlJEQL/UeF8YHoB0eJ4uTRREU1n+B\n"
"8twMNPamlKhN/tfLtE4Qt7TFsTS04dWuDuLUmf4+Y+kVoThNvnRYB2L1Uh4roO1S\n"
"lwlW2gailgVvirK6451bPxwaYOMPj8PXPVd94FAKGM4OQ58vaDDGkoGq5CQYwV78\n"
"66s5qXlCloQfjd9vLjAK3htbAawB7YTaW8EX9lDgzfbKlg4++vmEP6cpGQQPhaiY\n"
"BHgzimvX2oBsMsYQyCnnG/U9vYLt5w5S46NGY/voyUqUQ4WnXR8h9fCXkCMsp7Ep\n"
"GQnrAgMBAAGjYDBeMB8GA1UdIwQYMBaAFN87ZudGCtWhyYnSpG3r3w8A4SBYMB0G\n"
"A1UdDgQWBBTnGX+7cqnlATByPw7NDi0uLolItzAMBgNVHRMBAf8EAjAAMA4GA1Ud\n"
"DwEB/wQEAwIHgDANBgkqhkiG9w0BAQsFAAOCAQEAgwFCbQH4xwVh9p9Zer+HEY8Z\n"
"OGglApRq5/6IbLHJ/+FZKOmUteIr/HgZmetu2ltoHT9SW7F3sEsIz6mYYGONcT+H\n"
"Z31ELrl3I+zxbMMny60haS0KbiNkixn61n40wEeMyJxrSxUwpJ2o8ZjkKEI9K2G8\n"
"GvHoGILka4gL2HV4Va/HB4fGA1YvY8gIlAutHQ+KI9IjOZAuj33YcgOEzqWPu4Vk\n"
"yLjWer8a6syTgasTwfdD+AuacPJxrxYdFo4tkEOcJsg7YaHVrYCtuQECGkz36Me5\n"
"ApffjTr2iAIX7I7H2dpH4kugpspOIRp725eRiLUGeZZ5vI/w/jMHgnXVNAwHBg==\n"
"-----END CERTIFICATE-----\n";

const char* aws_cert_private = "-----BEGIN RSA PRIVATE KEY-----\n"
"MIIEogIBAAKCAQEAw34K7Ax9qTA/eQxlIvC1Di2SBng1FNZWm9kb9gShk/+Ah8xz\n"
"sS2AHZSREC/1HhfGB6AdHieLk0URFNZ/gfLcDDT2ppSoTf7Xy7ROELe0xbE0tOHV\n"
"rg7i1Jn+PmPpFaE4Tb50WAdi9VIeK6DtUpcJVtoGopYFb4qyuuOdWz8cGmDjD4/D\n"
"1z1XfeBQChjODkOfL2gwxpKBquQkGMFe/OurOal5QpaEH43fby4wCt4bWwGsAe2E\n"
"2lvBF/ZQ4M32ypYOPvr5hD+nKRkED4WomAR4M4pr19qAbDLGEMgp5xv1Pb2C7ecO\n"
"UuOjRmP76MlKlEOFp10fIfXwl5AjLKexKRkJ6wIDAQABAoIBAAsUH4ZVBuIJq+8u\n"
"NqUOQxR0nKvlMAzoQKxAV4dPp+B9QElx2rSVwYbrnnTuYtO7wSs5daz5zL0h0t/9\n"
"b+hrXPEMJ1Y1YnxZDt2qcCoeXw49BqGhkrx0d4JY38GlY9Yn9srhGjLK6F7CspcC\n"
"XbCqgor7fBjGq7QwhgR1VbuQDgWoknfkFaaNrIZJvyN1yZ6DWTKBLG/mxI6qxFQr\n"
"7A5qZT/IYpZgkev/1a+gzFem7K2Iv/2CLegq076CBxfnbd3hVPqPglmeHqHrVe82\n"
"lUKbXetTj5mmfJgpUCK5fJN1DqdbGZ/vzRiyr8oVxwXS/IoziVpLc8YRNelhFvj4\n"
"VmFFRbECgYEA+AlgyzyHVGJEgyPtgfIa3BBTtv5bjX6Dj/4agzk64lihiaHhVZGQ\n"
"/IFQb5FNzZhomfFmUYJNfCLd73OWzHPiyhRhozamJH+t1oPGeo4zO7u2tVC1dG44\n"
"Yf6UNP2ykPlZtTp8NaEGAYvcupoFFDJP6h0VReGOvYs/atg0tbDIHpUCgYEAycTN\n"
"H9l7fI/w5cSbKaAFfabX5EVxguTgTZg41UMKAEkADX15eCGC56dAC7avTHzhwPyq\n"
"Qag83v0+mCkUBxhObwwZV+E2PH1xzhx18f0XXniNV5lV+BiXQF0V0HPYW9RUS8Sw\n"
"ZJXCBty/Jgoc95Nv2OXmYRLXsSV5XRCIiGL45n8CgYAmTQxtVcNv/FyoA3xBfKk5\n"
"IixLLavlhYu7SHm/ma7GFPmczyqElB3XYACTudRR0x8T5nruSzi72oZ621aF678x\n"
"hUQud9nf+PdXELItapfuuWXVSck7WWnLeAxawnI4gMihe46CmYLb6kUDa5UtMTe1\n"
"5xfJ2rK4AjCgxxIcnFfKoQKBgG1MA0XGfMMcAgWlO9wPiAnp0DcievvJ2cuOsvKr\n"
"TwaahMHJ/3vYw12TVvlBa7JGT9BHzj6k0NyGWFr6nZO+UfaLmkrlf4jC91wMRMPi\n"
"Mr+FFCOot04enAhMpF1383LDESq+EENccG9hMtZ7Ufe20TawSty45UhboWONajhf\n"
"TK8nAoGAJwxp7OnMzGNBe9offI97VM1+xSJ6S9lPAAmDLSFaARm8tpzfawvkbDdX\n"
"mvalNFVJVRYLOK0eJ9KvLKZr3IAwUPgA4ernUATUKWLO+YzwTGzqlYrtk0SIgTRF\n"
"0k9olAC8mko1gVkiJ6M2PtKogF+hHC7PHYWweSS4pg8JEIyT2QY=\n"
"-----END RSA PRIVATE KEY-----\n";

// Wi-Fi & MQTT Connection Setup
void connectAWS() {
    WiFi.begin(ssid, password);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }
    Serial.println("\nWi-Fi Connected!");

    net.setCACert(aws_cert_ca);
    net.setCertificate(aws_cert_client);
    net.setPrivateKey(aws_cert_private);
    client.setServer(mqtt_server, 8883);

    while (!client.connected()) {
        Serial.print("Connecting to AWS IoT...");
        if (client.connect("ESP32_Bike")) {
            Serial.println("Connected!");
        } else {
            Serial.print("Failed. Retry in 5 sec...");
            delay(5000);
        }
    }
}

// Send Data to AWS IoT
void publishData(String device_id,float mq135, float mq9, float lat, float lon, bool pollutionStatus) {
    char payload[300];
    snprintf(payload, sizeof(payload),
        "{\"Device_ID\": \"%s\", \"MQ135\": %.2f, \"MQ9\": %.2f, \"Latitude\": \"%.4f°\", \"Longitude\": \"%.4f°\", \"Message\": %s}",
        device_id.c_str(),mq135, mq9, lat, lon, pollutionStatus ? "\"High Pollution Detected! Stopping Engine...\"": "\"Air Quality Normal. Engine Running.\"");

    Serial.println("Publishing: " + String(payload));
    client.publish(mqtt_topic, payload);
}

void setup() {
    Serial.begin(115200);
    gpsSerial.begin(9600, SERIAL_8N1, RXD2, TXD2);
    Blynk.begin(auth, ssid, password);

    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);  // Engine ON

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("SSD1306 allocation failed");
        while (1);
    }
    display.clearDisplay();

    WiFi.mode(WIFI_STA);
    connectAWS();
}

void loop() {
    if (!client.connected()) {
        connectAWS();
    }
    client.loop();

    Blynk.run(); // Run Blynk process
    
    String device_id="Esp32";
    float voltage_MQ135 = analogRead(MQ135_A0) * (3.3 / 4095.0);
    Blynk.virtualWrite(V1, voltage_MQ135); // Send MQ135 data to Blynk
    
    float voltage_MQ9 = analogRead(MQ9_A0) * (3.3 / 4095.0);
    Blynk.virtualWrite(V2, voltage_MQ9);  // Send MQ9 data to Blynk
    float latitude=17.99836;
    float longitude=79.56193;

    Serial.print("MQ-135: "); Serial.print(voltage_MQ135);
    Serial.print(" V || MQ-9: "); Serial.print(voltage_MQ9);
    Serial.print(" V");
    
    while (gpsSerial.available()) {
        gps.encode(gpsSerial.read());
    }

    //float latitude = gps.location.isValid() ? gps.location.lat() : 0.0;
    //float longitude = gps.location.isValid() ? gps.location.lng() : 0.0;

    Serial.print(" || Latitude: "); Serial.print(latitude); Serial.print("°"); 
    Serial.print(" | Longitude: "); Serial.print(longitude); Serial.println("°");

    Blynk.virtualWrite(V4, latitude);  // Send Latitude to Blynk
    Blynk.virtualWrite(V5, longitude); // Send Longitude to Blynk

    bool pollutionDetected = (voltage_MQ135 > MQ135_THRESHOLD || voltage_MQ9 > MQ9_THRESHOLD);
    int pollutionStatus = pollutionDetected ? 1 : 0;
    Blynk.virtualWrite(V3, pollutionStatus); // Send Pollution Status to Blynk Gauge

    if (pollutionDetected) {
        digitalWrite(RELAY_PIN, HIGH);  // Engine OFF
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(10, 15);
        display.print("HIGH POLLUTION!");
        display.setCursor(10, 35);
        display.print("ENGINE OFF!");

        Blynk.logEvent("High Pollution Alert", " High Pollution Detected! Engine Stopping.");
        
    } else {
        digitalWrite(RELAY_PIN, LOW);  // Engine ON
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(WHITE);
        display.setCursor(20, 15);
        display.print("Engine");
        display.setCursor(20, 35);
        display.print("Running");
    }
    
    display.display();
    publishData(device_id,voltage_MQ135, voltage_MQ9, latitude, longitude, pollutionDetected);

    client.loop();
    delay(5000);  // 5-second interval
}
