# LilyGO T-Echo Weather & GPS Logger

A standalone Arduino-based firmware for the **LilyGO T-Echo (nRF52840)** that transforms it into a portable weather station and GPS logger utilizing the onboard E-Paper display.

## Features

- **Weather Station Page**: Displays live Temperature, Humidity, Pressure, and Altitude using the onboard BME280 sensor.
- **GPS Navigation Page**: Displays Latitude, Longitude, Altitude, Speed, UTC Time, and Satellite count using the onboard L76K GNSS module.
- **E-Paper UI**: A clean, low-power user interface running on the 1.54" SPI E-Paper display, automatically updating every 30 seconds.
- **Battery Status**: Displays accurate battery percentage based on the onboard ADC voltage divider.
- **Capacitive Touch**: Use the capacitive touch button on the side to toggle between the Weather and GPS pages.
- **Heartbeat LED**: Blinks the green LED every 2 seconds to indicate system health.

## Hardware Requirements

- [LilyGO T-Echo](https://lilygo.cc/products/t-echo-lilygo) (or T-Echo Plus)
- Includes: nRF52840 MCU, BME280 Sensor, L76K GNSS, 1.54" E-Paper display.

## Software Dependencies

This project is built using [PlatformIO](https://platformio.org/).

**Libraries Used:**
- `adafruit/Adafruit BME280 Library`
- `adafruit/Adafruit Unified Sensor`
- `mikalhart/TinyGPSPlus`
- `adafruit/Adafruit GFX Library`
- `GxEPD` (from LilyGO official repository)

## Installation & Flashing

### Using PlatformIO (Recommended)

1. Clone this repository.
2. Open the folder in VSCode with the PlatformIO extension installed.
3. The `platformio.ini` is already configured for the correct board (`nrf52840_dk_adafruit`) which ensures all 48 pins of the nRF52840 map correctly 1:1.
4. Build the project.
5. **Flash via UF2 Bootloader:**
   - Double-click the reset button on the T-Echo.
   - The device will mount as a USB flash drive named `TECHOBOOT`.
   - Convert your `.hex` to `.uf2` using `uf2conv.py`.
   - Copy the `firmware.uf2` file to the `TECHOBOOT` drive. The device will automatically reboot and start the logger.

## Pin Configurations (T-Echo Reference)

This project uses the exact hardware mappings defined in the official LilyGO specifications.

- **E-Paper Display**: CS (30), DC (28), RST (2), BUSY (3), MOSI (29), MISO (38), SCK (31), Power (43)
- **BME280 (I2C)**: SDA (26), SCL (27), Address: 0x77
- **GPS L76K (UART2)**: RX (41), TX (40), Wakeup (34), Reset (37), PPS (36)
- **Touch Button**: Pin 11 (Active HIGH)
- **LEDs**: Green (33), Red (35), Blue (14)

## Acknowledgements

- Based on the official [LilyGO T-Echo](https://github.com/Xinyuan-LilyGO/T-Echo) examples.
- Uses the robust [GxEPD](https://github.com/ZinggJM/GxEPD) library for display driving.
