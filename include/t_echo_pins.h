#ifndef T_ECHO_PINS_H
#define T_ECHO_PINS_H

// ==========================================
// LilyGO T-Echo (nRF52840) Pin Definitions
// Source: Official LilyGO T-Echo repository
//   examples/Factory/utilities.h
// Board: nrf52840_dk_adafruit (PCA10056)
//   which has 1:1 pin mapping (Arduino pin N = nRF GPIO N)
//   _PINNUM(port, pin) = (port * 32 + pin)
// ==========================================

#ifndef _PINNUM
#define _PINNUM(port, pin)  ((port) * 32 + (pin))
#endif

// --- E-Paper Display (1.54" SPI 200x200, GDEH0154D67, SSD1681) ---
#define ePaper_Miso         _PINNUM(1, 6)   // P1.06 = 38
#define ePaper_Mosi         _PINNUM(0, 29)  // P0.29 = 29
#define ePaper_Sclk         _PINNUM(0, 31)  // P0.31 = 31
#define ePaper_Cs           _PINNUM(0, 30)  // P0.30 = 30
#define ePaper_Dc           _PINNUM(0, 28)  // P0.28 = 28
#define ePaper_Rst          _PINNUM(0, 2)   // P0.02 = 2
#define ePaper_Busy         _PINNUM(0, 3)   // P0.03 = 3
#define ePaper_Backlight    _PINNUM(1, 11)  // P1.11 = 43

// --- LoRa SX1262 ---
#define LoRa_Miso           _PINNUM(0, 23)  // P0.23 = 23
#define LoRa_Mosi           _PINNUM(0, 22)  // P0.22 = 22
#define LoRa_Sclk           _PINNUM(0, 19)  // P0.19 = 19
#define LoRa_Cs             _PINNUM(0, 24)  // P0.24 = 24
#define LoRa_Rst            _PINNUM(0, 25)  // P0.25 = 25
#define LoRa_Busy           _PINNUM(0, 17)  // P0.17 = 17
#define LoRa_Dio1           _PINNUM(0, 20)  // P0.20 = 20

// --- GPS L76K ---
#define Gps_Rx_Pin          _PINNUM(1, 9)   // P1.09 = 41
#define Gps_Tx_Pin          _PINNUM(1, 8)   // P1.08 = 40
#define Gps_Wakeup_Pin      _PINNUM(1, 2)   // P1.02 = 34
#define Gps_Reset_Pin       _PINNUM(1, 5)   // P1.05 = 37
#define Gps_pps_Pin         _PINNUM(1, 4)   // P1.04 = 36

// --- I2C Bus (shared: BME280, PCF8563 RTC) ---
#define SDA_Pin             _PINNUM(0, 26)  // P0.26 = 26
#define SCL_Pin             _PINNUM(0, 27)  // P0.27 = 27

// --- BME280 Sensor ---
#define BME280_ADDR         0x77  // Official: 0x77 (not 0x76!)

// --- Power ---
#define Power_Enable_Pin    _PINNUM(0, 12)  // P0.12 = 12
#define Power_Enable1_Pin   _PINNUM(0, 13)  // P0.13 = 13

// --- LEDs ---
#define GreenLed_Pin        _PINNUM(1, 1)   // P1.01 = 33
#define RedLed_Pin          _PINNUM(1, 3)   // P1.03 = 35
#define BlueLed_Pin         _PINNUM(0, 14)  // P0.14 = 14

// --- Buttons ---
#define Touch_Pin           _PINNUM(0, 11)  // P0.11 = 11 (HIGH level active)
#define UserButton_Pin      _PINNUM(1, 10)  // P1.10 = 42 (LOW level active)

// --- Battery ---
#define Adc_Pin             _PINNUM(0, 4)   // P0.04 = 4

// --- RTC PCF8563 ---
#define RTC_Int_Pin         _PINNUM(0, 16)  // P0.16 = 16

#endif // T_ECHO_PINS_H
