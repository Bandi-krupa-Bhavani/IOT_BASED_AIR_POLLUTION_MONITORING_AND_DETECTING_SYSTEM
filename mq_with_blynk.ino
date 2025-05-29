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

// Blynk Credentials
char auth[] = "4unYjiWnQ5K80E2Olh88hS_jYPPAj34i"; // Replace with your Blynk Token
char ssid[] = "realme 8i";        // Replace with your WiFi SSID
char pass[] = "raj@12345";    // Replace with your WiFi Password

// Define OLED Display size
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Sensor and Relay Pins
const int MQ135_A0 = 32;  // MQ-135 connected to GPIO32
const int MQ9_A0 = 34;    // MQ-9 connected to GPIO34
const int RELAY_PIN = 25; // Relay connected to GPIO25 (Engine control)

// Pollution Thresholds (Adjust based on real testing)
const float MQ135_THRESHOLD = 1.2;  // Threshold for air quality (Volts)
const float MQ9_THRESHOLD = 0.4;    // Threshold for CO levels (Volts)

// Define GPS Module Pins
#define RXD2 16  // GPS TX
#define TXD2 17  // GPS RX

HardwareSerial gpsSerial(1);
TinyGPSPlus gps;

void setup() {
    Serial.begin(115200);

    // Connect to WiFi and Blynk
    WiFi.begin(ssid, pass);
    Blynk.begin(auth, ssid, pass);
    
    // Initialize GPS
    gpsSerial.begin(9600, SERIAL_8N1, RXD2, TXD2);

    // Relay Setup
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW); // Ensure relay is ON initially (Engine running)

    // OLED Setup
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x64
        Serial.println(F("SSD1306 allocation failed"));
        for (;;);
    }
    delay(2000);
    display.clearDisplay();
}

void loop() {
    Blynk.run(); // Run Blynk process

    // Read MQ-135 Sensor
    float voltage_MQ135 = analogRead(MQ135_A0) * (3.3 / 4095.0);
    Blynk.virtualWrite(V1, voltage_MQ135); // Send MQ135 data to Blynk

    // Read MQ-9 Sensor
    float voltage_MQ9 = analogRead(MQ9_A0) * (3.3 / 4095.0);
    Blynk.virtualWrite(V2, voltage_MQ9);  // Send MQ9 data to Blynk

    // Display Sensor Values on Serial Monitor
    Serial.print("MQ-135 (Air Quality): ");
    Serial.print(voltage_MQ135);
    Serial.print(" V || ");

    Serial.print("MQ-9 (CO Level): ");
    Serial.print(voltage_MQ9);
    Serial.print(" V || ");

    // Check GPS Data
    while (gpsSerial.available()) {
        gps.encode(gpsSerial.read());
    }

    if (gps.location.isValid()) {
        float latitude = gps.location.lat();
        float longitude = gps.location.lng();

        Serial.print("Latitude: ");
        Serial.print(latitude, 4);
        Serial.print(" | Longitude: ");
        Serial.println(longitude, 4);

        Blynk.virtualWrite(V4, latitude);  // Send Latitude to Blynk
        Blynk.virtualWrite(V5, longitude); // Send Longitude to Blynk
    } else {
        Serial.println("Waiting for GPS...");
    }

    // Determine Pollution Status
    bool pollutionDetected = (voltage_MQ135 > MQ135_THRESHOLD || voltage_MQ9 > MQ9_THRESHOLD);
    int pollutionStatus = pollutionDetected ? 1 : 0;
    Blynk.virtualWrite(V3, pollutionStatus); // Send Pollution Status to Blynk Gauge

    // Update Relay & OLED Display based on Pollution Status
    if (pollutionDetected) {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        Serial.println("High Pollution Detected! Stopping Engine...");
        digitalWrite(RELAY_PIN, HIGH); // Turns OFF the relay (Engine OFF)

        display.setCursor(10, 15);
        display.print("HIGH POLLUTION!");
        display.setCursor(10, 35);
        display.print("ENGINE OFF!");

         Blynk.logEvent("High Pollution Alert", " High Pollution Detected! Engine Stopping.");
         
    } else {
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(WHITE);
        Serial.println("Air Quality Normal. Engine Running.");
        digitalWrite(RELAY_PIN, LOW); // Turns ON the relay (Engine ON)

        display.setCursor(20, 15);
        display.print("Engine");
        display.setCursor(20, 35);
        display.print("Running");
    }

    display.display(); // Refresh OLED screen
    delay(1000); // Wait 1 second before next reading
}
