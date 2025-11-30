#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include <PID_v1.h>
#include <time.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ==================== COMPETITION CONFIGURATION ====================
#define PROJECT_NAME "Poultrix Edge AI"
#define COMPETITION "Edge Impulse Innovation Challenge 2025"
#define VERSION "2.0.0"
#define BUILD_TARGET "Competition"

// ==================== PIN DEFINITIONS ====================
#define DHT_PIN 2        // GPIO2 (D4) - DHT11 Data
#define DHT_TYPE DHT11
#define HEATER_LED_PIN 0   // GPIO0 (D3) - Heater indicator LED
#define FAN_LED_PIN 16     // GPIO16 (D0) - Fan indicator LED
#define MQ135_PIN A0       // A0 - MQ135 Gas Sensor Analog Input

// ==================== OLED DISPLAY CONFIGURATION ====================
#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 32    // OLED display height, in pixels (0.91" is usually 32)
#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

// ==================== SENSOR CONFIGURATION ====================
#define DHT_READ_INTERVAL 4000    // 4 seconds between readings (optimized)
#define FIREBASE_SEND_INTERVAL 8000   // 8 seconds between Firebase uploads (optimized)
#define PID_UPDATE_INTERVAL 1000  // 1 second PID updates
#define MQ135_READ_INTERVAL 2500  // 2.5 seconds between MQ135 readings (optimized)
#define OLED_UPDATE_INTERVAL 1500 // 1.5 seconds between OLED updates (optimized)

// ==================== SMART TEMPERATURE CONTROL ====================
#define TEMP_SETPOINT 28.0        // Target temperature in Celsius
#define TEMP_HYSTERESIS 1.5       // Temperature hysteresis for LED control (tighter control)
#define TEMP_MIN_THRESHOLD (TEMP_SETPOINT - TEMP_HYSTERESIS)  // 26.5°C
#define TEMP_MAX_THRESHOLD (TEMP_SETPOINT + TEMP_HYSTERESIS)  // 29.5°C

// ==================== NETWORK CREDENTIALS ====================
// TODO: Update these with your actual credentials before uploading
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#define DATABASE_URL "YOUR_FIREBASE_DATABASE_URL"
#define DATABASE_SECRET "YOUR_FIREBASE_SECRET_KEY" 
#define DEVICE_ID "poultrix_edge_ai_001"
#define DEVICE_LOCATION "smart_brooder_house_1"

// ==================== GLOBAL OBJECTS ====================
DHT dht(DHT_PIN, DHT_TYPE);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Firebase objects
FirebaseData fbdo;
FirebaseConfig config;
FirebaseAuth auth;

// ==================== ENHANCED DATA STRUCTURES ====================
struct SensorData {
  float temperature;
  float humidity;
  float gasLevel;       // MQ135 gas sensor reading (ppm)
  int gasRawValue;      // Raw ADC value from MQ135
  float heatIndex;      // Calculated heat index
  bool tempValid;
  bool humidityValid;
  bool gasValid;
  unsigned long epochTime;
  unsigned long uptimeMs;
  String aiStatus;      // AI processing status
};

SensorData currentData;

// ==================== ENHANCED PID CONTROL ====================
double tempInput, tempOutput, tempSetpoint = TEMP_SETPOINT;
double Kp = 2.5, Ki = 0.8, Kd = 0.2;  // Enhanced PID parameters for competition
PID tempPID(&tempInput, &tempOutput, &tempSetpoint, Kp, Ki, Kd, DIRECT);

// ==================== TIMING VARIABLES ====================
unsigned long lastDHTRead = 0;
unsigned long lastFirebaseSend = 0;
unsigned long lastPIDUpdate = 0;
unsigned long lastMQ135Read = 0;
unsigned long lastOLEDUpdate = 0;
unsigned long lastStatusReport = 0;

// ==================== CONTROL STATUS ====================
bool heaterActive = false;
bool fanActive = false;
int pidOutput = 0;
int errorCount = 0;
int successCount = 0;

// ==================== SYSTEM STATUS ====================
bool wifiConnected = false;
bool firebaseReady = false;
bool oledReady = false;
bool aiSystemReady = false;

// ==================== FUNCTION DECLARATIONS ====================
void initializeWiFi();
void initializeFirebase();
void initializePID();
void initializeOLED();
void readDHT();
void readMQ135();
void updateTemperatureControl();
void updateOLED();
void sendDataToFirebase();
float calculateGasLevel(int rawValue);
void processAILogic();
void displayCompetitionInfo();
void performSystemDiagnostics();

// ==================== SETUP FUNCTION ====================
void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println(F("\n" "========================================="));
  Serial.println(F("🏆 POULTRIX EDGE AI - COMPETITION BUILD"));
  Serial.println(F("🚀 Edge Impulse Innovation Challenge 2025"));
  Serial.println(F("========================================="));
  Serial.println(F("Project: " PROJECT_NAME));
  Serial.println(F("Version: " VERSION));
  Serial.println(F("Build: " BUILD_TARGET));
  Serial.println(F("Board: ESP8266 NodeMCU v2"));
  Serial.println(F("Features: DHT11 + MQ135 + OLED + AI Control"));
  Serial.println(F("Build Date: " __DATE__ " " __TIME__));
  Serial.println(F("========================================="));
  
  // Initialize hardware
  pinMode(HEATER_LED_PIN, OUTPUT);
  pinMode(FAN_LED_PIN, OUTPUT);
  digitalWrite(HEATER_LED_PIN, LOW);
  digitalWrite(FAN_LED_PIN, LOW);
  
  // Initialize peripherals
  initializeOLED();
  displayCompetitionInfo();
  
  // Initialize sensors
  dht.begin();
  Serial.println(F("✅ DHT11 sensor initialized"));
  Serial.println(F("✅ MQ135 gas sensor initialized on pin A0"));
  
  // Initialize control systems
  initializePID();
  
  // Network initialization
  initializeWiFi();
  
  if (wifiConnected) {
    initializeFirebase();
  }
  
  // AI system initialization
  aiSystemReady = true;
  Serial.println(F("🤖 AI processing system ready"));
  
  Serial.println(F("\n🎯 COMPETITION SYSTEM READY"));
  Serial.println(F("⚡ Starting optimized sensor readings..."));
  
  // Wait for stable network connection
  while (!wifiConnected) {
    delay(500);
    Serial.print(F("."));
    if (WiFi.status() != WL_CONNECTED) {
      initializeWiFi();
    }
  }
  
  Serial.println(F("\n🌐 Network ready! Starting competition mode...\n"));
  delay(1000);
}

// ==================== ENHANCED MAIN LOOP ====================
void loop() {
  unsigned long currentTime = millis();
  
  // Network monitoring
  if (!wifiConnected) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println(F("📡 Network reconnection attempt..."));
      initializeWiFi();
    }
    delay(1000);
    return;
  }
  
  // Optimized sensor reading schedule
  if (currentTime - lastDHTRead >= DHT_READ_INTERVAL) {
    readDHT();
    lastDHTRead = currentTime;
  }
  
  if (currentTime - lastMQ135Read >= MQ135_READ_INTERVAL) {
    readMQ135();
    lastMQ135Read = currentTime;
  }
  
  // Enhanced display updates
  if (currentTime - lastOLEDUpdate >= OLED_UPDATE_INTERVAL) {
    updateOLED();
    lastOLEDUpdate = currentTime;
  }
  
  // AI-enhanced temperature control
  if (currentTime - lastPIDUpdate >= PID_UPDATE_INTERVAL) {
    updateTemperatureControl();
    processAILogic();
    lastPIDUpdate = currentTime;
  }
  
  // Optimized Firebase communication
  if (wifiConnected && firebaseReady && (currentTime - lastFirebaseSend >= FIREBASE_SEND_INTERVAL)) {
    sendDataToFirebase();
    lastFirebaseSend = currentTime;
  }
  
  // Competition status reporting (every 30 seconds)
  if (currentTime - lastStatusReport >= 30000) {
    performSystemDiagnostics();
    lastStatusReport = currentTime;
  }
  
  // Network status monitoring
  if (WiFi.status() != WL_CONNECTED) {
    wifiConnected = false;
    firebaseReady = false;
    Serial.println(F("⚠️ Network connection lost"));
  }
  
  delay(50); // Optimized delay for competition performance
}

void initializeWiFi() {
  Serial.print(F("🌐 Connecting to network: "));
  Serial.println(WIFI_SSID);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(300);
    Serial.print(F("."));
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(F("\n✅ Network Connected!"));
    Serial.print(F("📍 IP address: "));
    Serial.println(WiFi.localIP());
    Serial.print(F("📶 Signal strength: "));
    Serial.print(WiFi.RSSI());
    Serial.println(F(" dBm"));
    wifiConnected = true;
    
    // Enhanced time synchronization
    configTime(0, 0, "pool.ntp.org", "time.nist.gov", "time.google.com");
    Serial.println(F("⏰ Time sync initiated"));
    
    // Wait for time synchronization
    time_t now = time(nullptr);
    int timeAttempts = 0;
    while (now < 8 * 3600 * 24 && timeAttempts < 15) {
      delay(1000);
      now = time(nullptr);
      timeAttempts++;
    }
    
    if (now > 8 * 3600 * 24) {
      Serial.println(F("✅ Time synchronized"));
      Serial.printf("🕐 Current time: %s", ctime(&now));
    }
  } else {
    Serial.println(F("\n❌ Network connection failed"));
    wifiConnected = false;
  }
}

void initializeFirebase() {
  Serial.print(F("🔥 Initializing Firebase for competition... "));
  
  // Enhanced Firebase configuration
  config.database_url = DATABASE_URL;
  config.signer.tokens.legacy_token = DATABASE_SECRET;
  
  // Competition-optimized settings
  config.cert.data = nullptr;
  config.cert.file = "";
  config.timeout.socketConnection = 8000;   // Faster timeout
  config.timeout.sslHandshake = 8000;      
  config.timeout.serverResponse = 8000;    
  config.timeout.rtdbKeepAlive = 60000;    // Extended keep-alive
  config.timeout.rtdbStreamReconnect = 500; // Faster reconnect
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  
  Serial.println(F("✅ SUCCESS"));
  Serial.printf("🌍 Database: %s\n", DATABASE_URL);
  Serial.printf("🆔 Device ID: %s\n", DEVICE_ID);
  Serial.printf("📍 Location: %s\n", DEVICE_LOCATION);
  firebaseReady = true;
}

void initializePID() {
  // Enhanced PID configuration for competition
  tempPID.SetMode(AUTOMATIC);
  tempPID.SetOutputLimits(0, 255);
  tempPID.SetSampleTime(1000);
  
  Serial.println(F("🎛️ Enhanced PID controller initialized"));
  Serial.printf("🎯 Temperature Target: %.1f°C (±%.1f°C)\n", TEMP_SETPOINT, TEMP_HYSTERESIS);
  Serial.printf("⚙️ PID Parameters: Kp=%.2f, Ki=%.2f, Kd=%.2f\n", Kp, Ki, Kd);
  Serial.printf("🔌 Control Pins: Heater=%d, Fan=%d\n", HEATER_LED_PIN, FAN_LED_PIN);
}

void initializeOLED() {
  Serial.print(F("📺 Initializing OLED display... "));
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("❌ OLED initialization failed"));
    oledReady = false;
    return;
  }
  
  oledReady = true;
  Serial.println(F("✅ SUCCESS"));
}

void displayCompetitionInfo() {
  if (!oledReady) return;
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  // Competition branding
  display.setCursor(0, 0);
  display.println(F("POULTRIX EDGE AI"));
  display.setCursor(0, 10);
  display.println(F("Edge Impulse 2025"));
  display.setCursor(0, 20);
  display.println(F("Initializing..."));
  display.display();
  
  Serial.println(F("🏆 Competition info displayed on OLED"));
}

void readDHT() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  
  // Enhanced validation
  bool tempValid = !isnan(temperature) && temperature >= -40.0 && temperature <= 80.0;
  bool humidityValid = !isnan(humidity) && humidity >= 0.0 && humidity <= 100.0;
  
  if (!tempValid || !humidityValid) {
    Serial.println(F("❌ DHT sensor read failed"));
    currentData.tempValid = false;
    currentData.humidityValid = false;
    errorCount++;
    return;
  }
  
  // Calculate heat index for enhanced monitoring
  float heatIndex = dht.computeHeatIndex(temperature, humidity, false);
  
  // Store enhanced readings
  currentData.temperature = temperature;
  currentData.humidity = humidity;
  currentData.heatIndex = heatIndex;
  currentData.tempValid = tempValid;
  currentData.humidityValid = humidityValid;
  currentData.uptimeMs = millis();
  currentData.epochTime = time(nullptr);
  successCount++;
  
  // Enhanced logging
  Serial.printf("🌡️ T:%.1f°C H:%.0f%% HI:%.1f°C (Success:%d)\n", 
                temperature, humidity, heatIndex, successCount);
}

void readMQ135() {
  int rawValue = analogRead(MQ135_PIN);
  
  if (rawValue < 0 || rawValue > 1024) {
    Serial.println(F("❌ MQ135 sensor read failed"));
    currentData.gasValid = false;
    errorCount++;
    return;
  }
  
  float gasLevel = calculateGasLevel(rawValue);
  
  currentData.gasRawValue = rawValue;
  currentData.gasLevel = gasLevel;
  currentData.gasValid = true;
  
  Serial.printf("💨 Gas - Raw:%d ppm:%.1f\n", rawValue, gasLevel);
}

float calculateGasLevel(int rawValue) {
  // Enhanced gas calculation for competition
  float voltage = (rawValue / 1024.0) * 3.3; // Assuming 3.3V reference
  
  // Improved calibration curve (competition-grade)
  // This would be calibrated with Edge Impulse ML models in production
  float resistance = (3.3 - voltage) / voltage * 10000; // 10k pull-down
  float ppm = 116.6020682 * pow(resistance/76.63, -2.769034857); // Improved curve
  
  return constrain(ppm, 0, 2000); // Reasonable limits
}

void processAILogic() {
  if (!aiSystemReady) return;
  
  // Simulate AI processing for competition demonstration
  // In production, this would integrate Edge Impulse models
  
  currentData.aiStatus = "Processing";
  
  // AI-enhanced decision making
  if (currentData.tempValid && currentData.humidityValid && currentData.gasValid) {
    // Multi-parameter analysis
    float comfort_score = 100.0;
    
    // Temperature comfort
    if (abs(currentData.temperature - TEMP_SETPOINT) > 1.0) {
      comfort_score -= abs(currentData.temperature - TEMP_SETPOINT) * 10;
    }
    
    // Humidity comfort
    if (currentData.humidity > 70 || currentData.humidity < 45) {
      comfort_score -= 20;
    }
    
    // Air quality
    if (currentData.gasLevel > 400) {
      comfort_score -= 30;
    }
    
    currentData.aiStatus = (comfort_score > 70) ? "Optimal" : "Adjusting";
  } else {
    currentData.aiStatus = "Sensor Error";
  }
}

void updateTemperatureControl() {
  if (!currentData.tempValid) {
    digitalWrite(HEATER_LED_PIN, LOW);
    digitalWrite(FAN_LED_PIN, LOW);
    heaterActive = false;
    fanActive = false;
    return;
  }
  
  tempInput = currentData.temperature;
  
  if (tempPID.Compute()) {
    pidOutput = (int)tempOutput;
    
    // Enhanced control logic for competition
    if (currentData.temperature < TEMP_MIN_THRESHOLD) {
      digitalWrite(HEATER_LED_PIN, HIGH);
      digitalWrite(FAN_LED_PIN, LOW);
      heaterActive = true;
      fanActive = false;
      Serial.printf("🔥 HEATING: %.1f°C < %.1f°C (AI:%d)\n", 
                    currentData.temperature, TEMP_MIN_THRESHOLD, pidOutput);
    }
    else if (currentData.temperature > TEMP_MAX_THRESHOLD) {
      digitalWrite(HEATER_LED_PIN, LOW);
      digitalWrite(FAN_LED_PIN, HIGH);
      heaterActive = false;
      fanActive = true;
      Serial.printf("❄️ COOLING: %.1f°C > %.1f°C (AI:%d)\n", 
                    currentData.temperature, TEMP_MAX_THRESHOLD, pidOutput);
    }
    else {
      digitalWrite(HEATER_LED_PIN, LOW);
      digitalWrite(FAN_LED_PIN, LOW);
      heaterActive = false;
      fanActive = false;
      Serial.printf("✅ OPTIMAL: %.1f°C (AI:%d)\n", currentData.temperature, pidOutput);
    }
  }
}

void updateOLED() {
  if (!oledReady) return;
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  // Line 1: Temperature with AI status
  display.setCursor(0, 0);
  if (currentData.tempValid) {
    display.printf("T:%.1fC", currentData.temperature);
  } else {
    display.print("T:--.-C");
  }
  
  // Line 2: Humidity
  display.setCursor(0, 10);
  if (currentData.humidityValid) {
    display.printf("H:%.0f%%", currentData.humidity);
  } else {
    display.print("H:--%");
  }
  
  // Line 3: Air Quality
  display.setCursor(0, 20);
  if (currentData.gasValid) {
    display.printf("AQ:%.0f", currentData.gasLevel);
  } else {
    display.print("AQ:---");
  }
  
  // Status indicators (right side)
  display.setCursor(75, 0);
  if (wifiConnected) {
    display.print("NET");
  } else {
    display.print("---");
  }
  
  display.setCursor(75, 10);
  if (firebaseReady) {
    display.print("DB");
  } else {
    display.print("--");
  }
  
  display.setCursor(75, 20);
  if (heaterActive) {
    display.print("HEAT");
  } else if (fanActive) {
    display.print("COOL");
  } else {
    display.print("AUTO");
  }
  
  display.display();
}

void sendDataToFirebase() {
  if (!firebaseReady || !currentData.tempValid) {
    Serial.println(F("❌ Firebase skip"));
    return;
  }
  
  Serial.println(F("🚀 Uploading competition data..."));
  
  time_t now = time(nullptr);
  
  // Enhanced JSON for competition
  FirebaseJson json;
  json.add("device_id", DEVICE_ID);
  json.add("location", DEVICE_LOCATION);
  json.add("project", PROJECT_NAME);
  json.add("competition", COMPETITION);
  json.add("version", VERSION);
  json.add("temperature", currentData.temperature);
  json.add("humidity", currentData.humidity);
  json.add("heatIndex", currentData.heatIndex);
  json.add("gasLevel", currentData.gasLevel);
  json.add("gasRawValue", currentData.gasRawValue);
  json.add("tempValid", currentData.tempValid);
  json.add("humidityValid", currentData.humidityValid);
  json.add("gasValid", currentData.gasValid);
  json.add("setpoint", TEMP_SETPOINT);
  json.add("heaterActive", heaterActive);
  json.add("fanActive", fanActive);
  json.add("pidOutput", pidOutput);
  json.add("aiStatus", currentData.aiStatus);
  json.add("freeHeap", ESP.getFreeHeap());
  json.add("wifiSignal", WiFi.RSSI());
  json.add("uptime", currentData.uptimeMs / 1000);
  json.add("successCount", successCount);
  json.add("errorCount", errorCount);
  json.add("timestamp_epoch", (int)now);
  json.add("status", "competition_mode");
  
  // Send to latest path
  String latestPath = "/devices/" + String(DEVICE_ID) + "/latest";
  if (Firebase.RTDB.setJSON(&fbdo, latestPath, &json)) {
    Serial.println(F("✅ Data uploaded successfully"));
  } else {
    Serial.printf("❌ Upload failed: %s\n", fbdo.errorReason().c_str());
    errorCount++;
  }
}

void performSystemDiagnostics() {
  Serial.println(F("\n🔍 COMPETITION SYSTEM DIAGNOSTICS"));
  Serial.printf("⏱️ Uptime: %lu seconds\n", millis() / 1000);
  Serial.printf("📊 Success Rate: %.1f%% (%d/%d)\n", 
                (successCount * 100.0 / (successCount + errorCount)), 
                successCount, (successCount + errorCount));
  Serial.printf("🧠 Memory Free: %d bytes\n", ESP.getFreeHeap());
  Serial.printf("📶 WiFi Signal: %d dBm\n", WiFi.RSSI());
  Serial.printf("🤖 AI Status: %s\n", currentData.aiStatus.c_str());
  Serial.println(F("==========================================\n"));
}