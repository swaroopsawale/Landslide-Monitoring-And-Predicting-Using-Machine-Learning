#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <WebServer.h>

// Replace with your network credentials
const char* ssid = "Galaxy M31";
const char* password = "parth963";

// ThingSpeak API Key
String apiKey = "CLOL666HE17CVVGC";
const char* server = "http://api.thingspeak.com/update";

// GPIO pin mapping
#define SOIL_MOISTURE_PIN 34
#define VIBRATION_PIN 27
#define DHT_PIN 14
#define BUZZER_PIN 33

#define DHTTYPE DHT11
DHT dht(DHT_PIN, DHTTYPE);
Adafruit_MPU6050 mpu;

// Web server instance on port 80
WebServer serverWeb(80);

// Previous values
float prevResultant = 0.0;
int prevSoilMoisture = 0;
const float threshold = 3.0;  // Acceleration change threshold

// Handle buzzer on/off from web
void handleBuzzerOn() {
  digitalWrite(BUZZER_PIN, HIGH);
  serverWeb.send(200, "text/plain", "Buzzer turned ON");
}
void handleBuzzerOff() {
  digitalWrite(BUZZER_PIN, LOW);
  serverWeb.send(200, "text/plain", "Buzzer turned OFF");
}

void setup() {
  Serial.begin(115200);
  delay(100);

  pinMode(VIBRATION_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  dht.begin();

  if (!mpu.begin()) {
    Serial.println("MPU6050 not found!");
    while (1) delay(10);
  }

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
  Serial.print("ESP32 IP address: ");
  Serial.println(WiFi.localIP());

  // Web server routes
  serverWeb.on("/buzzer/on", handleBuzzerOn);
  serverWeb.on("/buzzer/off", handleBuzzerOff);
  serverWeb.begin();
  Serial.println("Web server started.");
}

void loop() {
  serverWeb.handleClient();

  int soilMoisture = analogRead(SOIL_MOISTURE_PIN);
  int vibrationRaw = digitalRead(VIBRATION_PIN);
  int vibrationValue = (vibrationRaw == LOW) ? 1 : 0;

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("DHT read error");
    temperature = 0;
    humidity = 0;
  }

  sensors_event_t a, g, temp_event;
  mpu.getEvent(&a, &g, &temp_event);

  float accX = a.acceleration.x;
  float accY = a.acceleration.y;
  float accZ = a.acceleration.z;

  float resultant = sqrt(accX * accX + accY * accY + accZ * accZ);
  float diff = abs(resultant - prevResultant);

  Serial.println("===================================");
  Serial.print("Soil Moisture: "); Serial.println(soilMoisture);
  Serial.print("Vibration: "); Serial.println(vibrationValue == 1 ? "Detected" : "None");
  Serial.print("Temperature: "); Serial.print(temperature); Serial.println(" °C");
  Serial.print("Humidity: "); Serial.print(humidity); Serial.println(" %");
  Serial.print("Acceleration X: "); Serial.println(accX);
  Serial.print("Acceleration Y: "); Serial.println(accY);
  Serial.print("Acceleration Z: "); Serial.println(accZ);
  Serial.print("Resultant Acceleration: "); Serial.println(resultant);
  Serial.print("Change from Previous: "); Serial.println(diff);
  Serial.println("===================================");

  // Buzzer logic: Continuous if soil moisture ≤ 1600
  if (soilMoisture <= 1600) {
    digitalWrite(BUZZER_PIN, HIGH);
    Serial.println("🔴 Landslide Danger! Continuous Alarm.");
  }
  // Intermittent buzzer if soil moisture decreasing
  else if (soilMoisture < prevSoilMoisture) {
    Serial.println("⚠️ Moisture Dropping - Intermittent Alarm");
    digitalWrite(BUZZER_PIN, HIGH);
    delay(300);
    digitalWrite(BUZZER_PIN, LOW);
    delay(300);
  }
  else {
    digitalWrite(BUZZER_PIN, LOW);
  }

  prevResultant = resultant;
  prevSoilMoisture = soilMoisture;

  // Send data to ThingSpeak
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(server) + "?api_key=" + apiKey +
                 "&field1=" + String(soilMoisture) +
                 "&field2=" + String(vibrationValue) +  // Modified to send 1/0
                 "&field3=" + String(temperature) +
                 "&field4=" + String(humidity) +
                 "&field5=0" +
                 "&field6=" + String(accX, 2) +
                 "&field7=" + String(accY, 2) +
                 "&field8=" + String(accZ, 2);

    http.begin(url);
    int httpCode = http.GET();
    if (httpCode > 0) {
      Serial.println("✅ Data sent to ThingSpeak.");
    } else {
      Serial.print("❌ Error sending data. HTTP code: ");
      Serial.println(httpCode);
    }
    http.end();
  } else {
    Serial.println("❌ WiFi not connected.");
  }

  delay(20000);  // 20 seconds (>=15s for ThingSpeak)
}
