# ESP32-CAM Streaming Server Project

## Description
This project implements a video streaming server using the ESP32-CAM module. It provides real-time camera feed access through a web interface, making it suitable for surveillance, monitoring, or remote viewing applications.

## Features
- Real-time video streaming over WiFi
- Web interface for easy access
- Automatic WiFi connection
- Configurable camera settings
- Low-latency image transmission

## Hardware Requirements
- ESP32-CAM (AI-Thinker) module
- USB-UART programmer (FTDI, CP2102, or similar)
- 5V power supply
- Female-to-female jumper wires

## Software Requirements
- PlatformIO IDE
- ESP32 Arduino framework
- ESP32 board support package
- Required libraries:
  - WebServer
  - WiFi
  - ESP32Camera

## Installation
1. Clone the repository
```bash
git clone https://github.com/PaeSielawa/RCCar-ESP32Cam.git
```

2. Open project in PlatformIO
3. Configure WiFi credentials in `src/main.cpp`:
```cpp
const char* ssid = "YourWiFiNetwork";
const char* password = "YourWiFiPassword";
```

## Wiring
For programming, connect the USB-UART programmer to ESP32-CAM:
| ESP32-CAM | UART Programmer |
|-----------|----------------|
| GND       | GND           |
| 5V        | 5V            |
| U0R       | TX            |
| U0T       | RX            |
| GPIO0     | GND (only when flashing) |

## Project Configuration
Project settings in `platformio.ini`:
```ini
[env:esp32cam]
platform = espressif32
board = esp32cam
framework = arduino
monitor_speed = 115200
board_build.flash_mode = qio
board_build.partitions = huge_app.csv
board_build.f_cpu = 240000000L
```

## Usage
1. Upload the code to ESP32-CAM
2. Power cycle the device
3. Check serial monitor for IP address
4. Open web browser and navigate to: `http://<ESP32-IP-Address>`

## Camera Settings
Default camera configuration:
- Resolution: 800x600 (SVGA)
- Quality: 10 (range: 0-63)
- Frame Size: Optimized for streaming
- Frame Buffer Count: 2

## Troubleshooting
- **Camera initialization failed**: Check camera module connection
- **WiFi connection issues**: Verify credentials and signal strength
- **Streaming problems**: Ensure stable power supply
- **Upload fails**: Make sure GPIO0 is connected to GND during flashing

## Contributing
1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## Acknowledgments
- ESP32-CAM module by AI-Thinker
- Arduino ESP32 framework
- PlatformIO development platform
  
## License
This project was created by PS. All rights reserved.
