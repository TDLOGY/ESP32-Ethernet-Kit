# CH390 Ethernet SPI Test on ESP32-S3

This project demonstrates how to initialize and use the CH390 SPI Ethernet controller with an ESP32-S3 using the ESP-IDF framework. The main application code is contained in `main.c`.

## Hardware Requirements

*   **Development Board:** ESP32-S3 (e.g., ESP32-S3-DevKitC)
*   **Ethernet Module:** CH390 SPI Ethernet Module
*   **Jumper wires**

## Pin Connections / Wiring

Connect the CH390 module to the ESP32-S3 according to the GPIO definitions in `main.c`:

| CH390 Pin | ESP32-S3 GPIO | Note |
| :--- | :--- | :--- |
| **MISO** | GPIO 11 | SPI Master In Slave Out |
| **MOSI** | GPIO 12 | SPI Master Out Slave In |
| **SCLK** | GPIO 13 | SPI Clock |
| **CS** | GPIO 14 | Chip Select |
| **INT** | GPIO 10 | Interrupt |
| **RST** | Not connected | Configured as `-1` in software |
| **VCC** | 3.3V | Ensure adequate power supply |
| **GND** | GND | Common ground |

## Software Prerequisites

Ensure you have the **ESP-IDF** (Espressif IoT Development Framework) installed and configured on your system.
*   Recommended version: ESP-IDF v5.0 or newer.

## Build and Flash Instructions

1.  **Initialize the ESP-IDF environment:**
    Open your terminal and source the export script (varies by OS):
    ```bash
    # On Linux/macOS
    . $HOME/esp/esp-idf/export.sh
    
    # On Windows (Command Prompt)
    %userprofile%\esp\esp-idf\export.bat
    ```

2.  **Set the target chip to ESP32-S3:**
    Navigate to your project directory containing `main.c` and run:
    ```bash
    idf.py set-target esp32s3
    ```

3.  **Configure the project (Optional):**
    If you need to tweak any specific ESP-IDF parameters (like flash size), open the configuration menu:
    ```bash
    idf.py menuconfig
    ```

4.  **Build, Flash, and Monitor:**
    Connect your ESP32-S3 to your computer via USB. Run the following command to compile the code, flash it to the board, and open the serial monitor (replace `/dev/ttyUSB0` with your actual serial port, e.g., `COM3` on Windows):
    ```bash
    idf.py -p /dev/ttyUSB0 build flash monitor
    ```

## Expected Output

Once flashed successfully, the ESP32-S3 will boot up, initialize the SPI bus, and configure the CH390 MAC/PHY. In the serial monitor, you should see logs similar to:

```text
I (xxx) CH390_TEST: Test hardware IC CH390...
I (xxx) CH390_TEST: Starting Ethernet driver... Waiting for IP allocation
I (xxx) CH390_TEST: Ethernet Driver Started
I (xxx) CH390_TEST: Ethernet Link Up!
I (xxx) CH390_TEST: Địa chỉ MAC: xx:xx:xx:xx:xx:xx
I (xxx) CH390_TEST: ~~~~~~~~~~~~~~~~~~~~~~~~~~~
I (xxx) CH390_TEST: Successfully got IP Address!
I (xxx) CH390_TEST: IP:      192.168.1.100
I (xxx) CH390_TEST: Netmask: 255.255.255.0
I (xxx) CH390_TEST: Gateway: 192.168.1.1
I (xxx) CH390_TEST: ~~~~~~~~~~~~~~~~~~~~~~~~~~~
```

*(Note: To exit the serial monitor, press `Ctrl` + `]`)*