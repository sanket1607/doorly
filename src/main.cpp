#include <Arduino.h>
#include <HttpClient.h>
#include <WiFi.h>

// WiFi details
const char* ssid = "KIRAN RAGHAVENDRA"; 
const char* pass = "kiran789"; 

// Server details
const char kHostname[] = "18.189.16.153";
const char kPath[] = "/upload";  // Path for photo upload
const int kPort = 5000;

// Pin definitions
#define BUTTON_PIN 33  // GPIO for button
#define BUZZER_PIN 32  // GPIO for buzzer

bool buzzer_on = false;
bool photo_received = false;
String photoData = ""; // Holds received photo data

void connectWiFi() {
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

// Function to send photo to server
void sendPhoto(String photoData) {
    WiFiClient c;
    HttpClient http(c, kHostname, kPort);

    Serial.println("Sending photo to server...");
    int err = http.post(kPath, "application/json", photoData.c_str());
    if (err == 0) {
        int statusCode = http.responseStatusCode();
        if (statusCode == 200) {
            Serial.println("Photo sent successfully.");
        } else {
            Serial.println("Failed to send photo. Status code: " + String(statusCode));
        }
    } else {
        Serial.print("HTTP POST failed: ");
        Serial.println(err);
    }

    http.stop();
}

void setup() {
    Serial.begin(115200);  // UART communication for ESP32-CAM
    delay(2000);

    Serial.println("Starting Setup...");

    // Set up buzzer pin with PWM
    ledcSetup(0, 2000, 8);        // Channel 0, frequency 2 kHz, 8-bit resolution
    ledcAttachPin(BUZZER_PIN, 0); // Attach the buzzer to the channel

    pinMode(BUTTON_PIN, INPUT_PULLUP); // Configure button pin with internal pull-up resistor

    Serial.println("Connecting to WiFi...");
    connectWiFi();
}

void loop() {
    // Check if the button is pressed
    if (digitalRead(BUTTON_PIN) == LOW) {  // Button press detected
        Serial.println("Button pressed!");
        buzzer_on = true;

        // Send a signal to ESP32-CAM to capture photo
        Serial.println("Requesting photo from ESP32-CAM...");
        Serial.println("CAPTURE"); // Signal to ESP32-CAM to capture photo
        delay(1000);
    }

    if (Serial.available()) {  // Check if photo data is received from ESP32-CAM
        photoData = Serial.readString(); // Read photo data
        photo_received = true;
    }

    if (buzzer_on && photo_received) {
        // Activate the buzzer for 3 seconds
        ledcWriteTone(0, 1000); // Turn buzzer ON with a 1 kHz tone
        delay(3000);
        ledcWriteTone(0, 0);   // Turn buzzer OFF

        // Send photo data to the server
        sendPhoto(photoData);

        // Debounce delay
        delay(1000);
        buzzer_on = false;
        photo_received = false;
        photoData = "";
    }

    delay(10); // Small delay for stability
}