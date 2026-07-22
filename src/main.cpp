/*
 * LilyGO T-Echo Weather & GPS Logger
 * Based on official LilyGO Display_lilygo example patterns.
 * Board: nrf52840_dk_adafruit (PCA10056) with 1:1 48-pin GPIO map.
 * Display: GxEPD library (not GxEPD2) with GxIO_SPI, matching official example.
 */

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <TinyGPS++.h>

#include <GxEPD.h>
#include <GxDEPG0150BN/GxDEPG0150BN.h>  // 1.54" b/w (official driver)
#include <Fonts/FreeMonoBold9pt7b.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>

#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>

#include "t_echo_pins.h"

// --- Display objects (matching official example pattern) ---
SPIClass        *dispPort  = nullptr;
GxIO_Class      *io        = nullptr;
GxEPD_Class     *display   = nullptr;

// --- Sensors ---
Adafruit_BME280 bme;
TinyGPSPlus gps;

// --- State ---
uint8_t currentPage = 0;
bool bmePresent = false;
unsigned long lastUpdate = 0;
const unsigned long UPDATE_INTERVAL_MS = 30000;

// --- Logging ---
using namespace Adafruit_LittleFS_Namespace;
const char* LOG_FILENAME = "/metrics.csv";
bool memoryFull = false;
unsigned long lastLogTime = 0;
const unsigned long LOG_INTERVAL_MS = 3600000; // 1 hour

void logMetrics() {
    if (memoryFull) return;

    File file(InternalFS);
    bool fileExists = file.open(LOG_FILENAME, FILE_O_READ);
    if (fileExists) {
        if (file.size() > 200000) { // Limit to ~200KB
            memoryFull = true;
            file.close();
            return;
        }
        file.close();
    }

    if (file.open(LOG_FILENAME, FILE_O_WRITE)) {
        if (!fileExists) {
            file.write("Date,Time,Lat,Lon,Speed(kmh),Alt(m),Sats,Temp(C),Hum(%),Press(hPa)\n");
        }

        // Date/Time
        char timeStr[32];
        if (gps.time.isValid() && gps.date.isValid() && gps.date.year() > 2000) {
            sprintf(timeStr, "%04d/%02d/%02d,%02d:%02d:%02d",
                    gps.date.year(), gps.date.month(), gps.date.day(),
                    gps.time.hour(), gps.time.minute(), gps.time.second());
        } else {
            sprintf(timeStr, "NO_DATE,NO_TIME");
        }

        // GPS
        char gpsStr[64];
        if (gps.location.isValid()) {
            sprintf(gpsStr, "%.5f,%.5f,%.1f,%.1f,%u",
                    gps.location.lat(), gps.location.lng(),
                    gps.speed.kmph(), gps.altitude.meters(), (unsigned int)gps.satellites.value());
        } else {
            sprintf(gpsStr, "NO_FIX,NO_FIX,N/A,N/A,0");
        }

        // BME
        char bmeStr[32];
        if (bmePresent) {
            sprintf(bmeStr, "%.1f,%.1f,%.1f",
                    bme.readTemperature(), bme.readHumidity(), bme.readPressure() / 100.0f);
        } else {
            sprintf(bmeStr, "N/A,N/A,N/A");
        }

        file.print(timeStr);
        file.print(",");
        file.print(gpsStr);
        file.print(",");
        file.print(bmeStr);
        file.print("\n");
        file.close();
        
        Serial.println("[LOG] Saved hourly metrics to internal flash.");
    } else {
        Serial.println("[LOG ERROR] Failed to open log file.");
    }
}

// --- Battery ---
float readBatteryVoltage() {
    analogReadResolution(12);
    uint32_t raw = analogRead(Adc_Pin);
    return (raw / 4096.0f) * 3.6f * 2.0f;
}

uint8_t batteryPercent(float v) {
    if (v >= 4.2f) return 100;
    if (v <= 3.3f) return 0;
    return (uint8_t)((v - 3.3f) / 0.9f * 100.0f);
}

// --- Drawing ---
void drawWeatherPage() {
    float batV = readBatteryVoltage();
    uint8_t batPct = batteryPercent(batV);
    float tempC = bmePresent ? bme.readTemperature() : 0;
    float hum   = bmePresent ? bme.readHumidity() : 0;
    float press = bmePresent ? bme.readPressure() / 100.0f : 0;
    float alt   = bmePresent ? bme.readAltitude(1013.25f) : 0;

    display->fillScreen(GxEPD_WHITE);
    display->setTextColor(GxEPD_BLACK);
    display->setFont(&FreeMonoBold9pt7b);

    // Header
    display->setCursor(2, 14);
    if (memoryFull) {
        display->print("MEM FULL");
    } else {
        display->print("T-ECHO");
    }
    display->setCursor(130, 14);
    display->print(batPct);
    display->print("%");
    display->drawFastHLine(0, 18, 200, GxEPD_BLACK);

    // Title
    display->setCursor(10, 36);
    display->print("WEATHER");

    // Temperature
    display->drawRect(5, 42, 190, 30, GxEPD_BLACK);
    display->setCursor(10, 62);
    display->print("T: ");
    if (bmePresent) {
        display->print(tempC, 1);
        display->print(" C");
    } else {
        display->print("N/A");
    }

    // Humidity
    display->drawRect(5, 76, 190, 30, GxEPD_BLACK);
    display->setCursor(10, 96);
    display->print("H: ");
    if (bmePresent) {
        display->print(hum, 1);
        display->print(" %");
    } else {
        display->print("N/A");
    }

    // Pressure
    display->drawRect(5, 110, 190, 30, GxEPD_BLACK);
    display->setCursor(10, 130);
    display->print("P: ");
    if (bmePresent) {
        display->print(press, 0);
        display->print(" hPa");
    } else {
        display->print("N/A");
    }

    // Altitude
    display->drawRect(5, 144, 190, 30, GxEPD_BLACK);
    display->setCursor(10, 164);
    display->print("A: ");
    if (bmePresent) {
        display->print(alt, 0);
        display->print(" m");
    } else {
        display->print("N/A");
    }

    // GPS status at bottom
    display->setCursor(10, 190);
    if (gps.location.isValid()) {
        display->print("GPS FIX ");
        display->print(gps.satellites.value());
        display->print(" sat");
    } else {
        display->print("GPS SEARCHING...");
    }

    display->update();
}

void drawGPSPage() {
    float batV = readBatteryVoltage();
    uint8_t batPct = batteryPercent(batV);

    display->fillScreen(GxEPD_WHITE);
    display->setTextColor(GxEPD_BLACK);
    display->setFont(&FreeMonoBold9pt7b);

    // Header
    display->setCursor(2, 14);
    if (memoryFull) {
        display->print("MEM FULL");
    } else {
        display->print("T-ECHO");
    }
    display->setCursor(130, 14);
    display->print(batPct);
    display->print("%");
    display->drawFastHLine(0, 18, 200, GxEPD_BLACK);

    // Title
    display->setCursor(10, 36);
    display->print("GPS NAV");

    // Lat
    display->drawRect(5, 42, 190, 30, GxEPD_BLACK);
    display->setCursor(10, 62);
    display->print("LAT:");
    if (gps.location.isValid()) {
        display->print(gps.location.lat(), 5);
    } else {
        display->print(" NO FIX");
    }

    // Lon
    display->drawRect(5, 76, 190, 30, GxEPD_BLACK);
    display->setCursor(10, 96);
    display->print("LON:");
    if (gps.location.isValid()) {
        display->print(gps.location.lng(), 5);
    } else {
        display->print(" NO FIX");
    }

    // Speed / Alt
    display->drawRect(5, 110, 190, 30, GxEPD_BLACK);
    display->setCursor(10, 130);
    display->print("SPD:");
    if (gps.speed.isValid()) {
        display->print(gps.speed.kmph(), 1);
        display->print("km/h");
    } else {
        display->print(" --");
    }

    // Time
    display->drawRect(5, 144, 190, 30, GxEPD_BLACK);
    display->setCursor(10, 164);
    display->print("UTC:");
    if (gps.time.isValid()) {
        if (gps.time.hour() < 10) display->print("0");
        display->print(gps.time.hour());
        display->print(":");
        if (gps.time.minute() < 10) display->print("0");
        display->print(gps.time.minute());
        display->print(":");
        if (gps.time.second() < 10) display->print("0");
        display->print(gps.time.second());
    } else {
        display->print(" --:--:--");
    }

    // Sats at bottom
    display->setCursor(10, 190);
    display->print("SATS: ");
    display->print(gps.satellites.value());

    display->update();
}

void drawScreenSaver() {
    display->fillScreen(GxEPD_WHITE);
    display->setTextColor(GxEPD_BLACK);
    display->setFont(&FreeMonoBold9pt7b);

    // Terminal-style ASCII Logo
    const char* art[] = {
        "   _  _",
        " _| || |_",
        "|_  ..  _|",
        "|_      _|",
        "  |_||_|",
        "",
        " T-ECHO",
        " LOGGER",
        "",
        " (SLEEP)"
    };

    int y = 40;
    for (int i = 0; i < 10; i++) {
        display->setCursor(40, y);
        display->print(art[i]);
        y += 18;
    }
    
    display->update();
}

void updateDisplay() {
    if (currentPage == 0) drawWeatherPage();
    else if (currentPage == 1) drawGPSPage();
    else if (currentPage == 2) drawScreenSaver();
}

void setupDisplay() {
    // Create SPI port for display (matches official example exactly)
    dispPort = new SPIClass(
        /*SPIPORT*/ NRF_SPIM2,
        /*MISO*/    ePaper_Miso,
        /*SCLK*/    ePaper_Sclk,
        /*MOSI*/    ePaper_Mosi);

    io = new GxIO_Class(
        *dispPort,
        /*CS*/  ePaper_Cs,
        /*DC*/  ePaper_Dc,
        /*RST*/ ePaper_Rst);

    display = new GxEPD_Class(
        *io,
        /*RST*/  ePaper_Rst,
        /*BUSY*/ ePaper_Busy);

    dispPort->begin();
    display->init(/*115200*/);
    display->setRotation(2);
    display->fillScreen(GxEPD_WHITE);
    display->setTextColor(GxEPD_BLACK);
}

void setup() {
    Serial.begin(115200);
    // Remove delay(200); we can just proceed

    // Power enable (official pattern)
    pinMode(Power_Enable_Pin, OUTPUT);
    digitalWrite(Power_Enable_Pin, HIGH);
    
    // Also enable Power_Enable1_Pin just in case it powers sensors on some revisions
    pinMode(Power_Enable1_Pin, OUTPUT);
    digitalWrite(Power_Enable1_Pin, HIGH);

    // E-Paper backlight
    pinMode(ePaper_Backlight, OUTPUT);
    digitalWrite(ePaper_Backlight, LOW); // OFF backlight (saves power)

    // LEDs
    pinMode(GreenLed_Pin, OUTPUT);
    pinMode(RedLed_Pin, OUTPUT);
    pinMode(BlueLed_Pin, OUTPUT);
    digitalWrite(GreenLed_Pin, LOW); // ON (active low)
    digitalWrite(RedLed_Pin, HIGH);
    digitalWrite(BlueLed_Pin, HIGH);

    // Buttons
    pinMode(UserButton_Pin, INPUT_PULLUP);
    pinMode(Touch_Pin, INPUT_PULLUP);

    Serial.println("==================================");
    Serial.println("   LilyGO T-Echo Logger Booting   ");
    Serial.println("==================================");

    // I2C + BME280
    Wire.setPins(SDA_Pin, SCL_Pin);
    Wire.begin();
    // Try official 0x77 first, then fallback to 0x76
    bmePresent = bme.begin(0x77, &Wire) || bme.begin(0x76, &Wire);
    Serial.print("[INIT] BME280: ");
    Serial.println(bmePresent ? "FOUND" : "NOT FOUND");

    // GPS UART (use Serial2 matching official code)
    Serial2.setPins(Gps_Rx_Pin, Gps_Tx_Pin);
    Serial2.begin(9600);

    // Wake up GPS
    pinMode(Gps_Wakeup_Pin, OUTPUT);
    digitalWrite(Gps_Wakeup_Pin, HIGH);

    // Reset GPS
    pinMode(Gps_Reset_Pin, OUTPUT);
    digitalWrite(Gps_Reset_Pin, HIGH);
    delay(10);
    digitalWrite(Gps_Reset_Pin, LOW);
    delay(10);
    digitalWrite(Gps_Reset_Pin, HIGH);
    delay(200);

    // GPS Initialization commands (from official gps_probe)
    Serial2.write("$PCAS04,5*1C\r\n"); // Use GPS + GLONASS
    delay(250);
    Serial2.write("$PCAS03,1,0,0,0,1,0,0,0,0,0,,,0,0*02\r\n"); // RMC and GGA only
    delay(250);
    Serial2.write("$PCAS11,3*1E\r\n"); // Vehicle Mode
    delay(250);

    Serial.println("[INIT] GPS initialized");

    // Initialize File System
    InternalFS.begin();
    Serial.println("[INIT] InternalFS initialized");

    // Display init (using official pattern)
    setupDisplay();
    Serial.println("[INIT] Display initialized");

    // Draw initial UI
    updateDisplay();
    Serial.println("[INIT] Initial UI drawn");

    // Turn off green LED after boot
    digitalWrite(GreenLed_Pin, HIGH);
    Serial.println("[READY] Setup complete");
}

void loop() {
    // Heartbeat: blink green LED every 2 seconds
    static unsigned long lastBlink = 0;
    if (millis() - lastBlink > 2000) {
        lastBlink = millis();
        digitalWrite(GreenLed_Pin, !digitalRead(GreenLed_Pin));
    }

    // Serial Commands (DUMP / CLEAR)
    if (Serial.available() > 0) {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();
        cmd.toUpperCase();
        if (cmd == "DUMP") {
            File file(InternalFS);
            if (file.open(LOG_FILENAME, FILE_O_READ)) {
                Serial.println("=== LOG DUMP START ===");
                while (file.available()) {
                    Serial.write(file.read());
                }
                Serial.println("\n=== LOG DUMP END ===");
                file.close();
            } else {
                Serial.println("No log file found.");
            }
        } else if (cmd == "CLEAR") {
            InternalFS.remove(LOG_FILENAME);
            memoryFull = false;
            Serial.println("Log file cleared.");
            updateDisplay(); // Remove MEM FULL message
        }
    }

    // GPS
    while (Serial2.available() > 0) {
        gps.encode(Serial2.read());
    }

    // User button (physical button on the side) - LOW active
    if (digitalRead(UserButton_Pin) == LOW) {
        delay(50); // debounce
        if (digitalRead(UserButton_Pin) == LOW) {
            Serial.println("[INPUT] User button pressed");
            currentPage = (currentPage + 1) % 3;
            updateDisplay();
            
            // Wait for release
            while (digitalRead(UserButton_Pin) == LOW) {
                // Keep processing GPS while waiting so we don't lose data
                while (Serial2.available() > 0) {
                    gps.encode(Serial2.read());
                }
                delay(10);
            }
        }
    }

    // Periodic refresh (skip if in screen saver mode to prevent screen flashing)
    if (currentPage != 2) {
        if (millis() - lastUpdate >= UPDATE_INTERVAL_MS) {
            lastUpdate = millis();
            updateDisplay();
        }
    }

    // Periodic Logging
    if (millis() - lastLogTime >= LOG_INTERVAL_MS || lastLogTime == 0) {
        // Wait until GPS has a fix or 5 minutes have passed to avoid recording blank time forever on boot
        if (gps.time.isValid() || millis() > 300000 || lastLogTime != 0) {
            lastLogTime = millis();
            logMetrics();
        }
    }
}
