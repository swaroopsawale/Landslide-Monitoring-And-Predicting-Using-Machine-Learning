#include <WiFi.h>
#include <Wire.h>
#include "ThingSpeak.h"
#include "DHT.h"
#include "Adafruit_MPU6050.h"
#include "Adafruit_Sensor.h"

// WiFi credentials
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// ThingSpeak settings
unsigned long myChannelNumber = 2826177;
const char* myWriteAPIKey = "5SAXDEZE0YLZSY58";

// DHT Sensor
#define DHTPIN 4
#define DHTTYPE DHT22  // (You wrote DHT22 in your simulation)
DHT dht(DHTPIN, DHTTYPE);

// Ultrasonic Sensor
#define TRIG_PIN 5
#define ECHO_PIN 18

// Soil Moisture Sensor
#define SOIL_MOISTURE_PIN 34

// MPU6050
Adafruit_MPU6050 mpu;

WiFiClient client;

void setup() {
  Serial.begin(115200);

  dht.begin();
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  Wire.begin(21, 22); // ESP32 I2C pins

  if (!mpu.begin()) {
    Serial.println("❌ MPU6050 Not Found!");
    while (1) delay(10);
  }
  Serial.println("✅ MPU6050 Connected!");

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi connected!");
  } else {
    Serial.println("\n❌ WiFi connection failed.");
  }

  ThingSpeak.begin(client);
}

void loop() {
  // Read DHT22 values
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // Read Soil Moisture
  int soilMoisture = analogRead(SOIL_MOISTURE_PIN);

  // Measure Distance from Ultrasonic
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  float distance = duration * 0.034 / 2;

  // Read MPU6050 Acceleration
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float accelX = a.acceleration.x;
  float accelY = a.acceleration.y;
  float accelZ = a.acceleration.z;

  // Print data to Serial Monitor
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print("%, Temperature: ");
  Serial.print(temperature);
  Serial.println("°C");

  Serial.print("Soil Moisture: ");
  Serial.println(soilMoisture);

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  Serial.print("Acceleration (X,Y,Z): ");
  Serial.print(accelX); Serial.print(", ");
  Serial.print(accelY); Serial.print(", ");
  Serial.println(accelZ);

  // Upload to ThingSpeak
  ThingSpeak.setField(1, temperature);
  ThingSpeak.setField(2, humidity);
  ThingSpeak.setField(3, soilMoisture);
  ThingSpeak.setField(4, distance);
  ThingSpeak.setField(5, accelX);
  ThingSpeak.setField(6, accelY);
  ThingSpeak.setField(7, accelZ);

  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  
  if (x == 200) {
    Serial.println("✅ Data uploaded to ThingSpeak.");
  } else {
    Serial.print("❌ Upload failed, error code: ");
    Serial.println(x);
  }

  delay(20000); // wait 20 seconds to respect ThingSpeak API limits
}
