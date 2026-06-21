# EspScopeGen 🚀

A high-performance, web-based **Oscilloscope & Signal Generator Dashboard** running locally on the **ESP32 WROVER-E**.

This project turns your ESP32 into a portable lab instrument. It hosts a modern, responsive web application directly on the chip, allowing you to visualize signals, simulate waveforms, control hardware DAC outputs, and monitor system telemetry in real time.

---

## 🌟 Key Features

* **Real-time Web Oscilloscope**: 
  * Captures ADC readings from GPIO34.
  * Supports custom trigger modes (Auto/Normal), trigger levels, and rising/falling edge selection.
  * Dynamic timebase adjustments.
* **Internal Waveform Simulator**: 
  * Generates virtual waveforms including **Sine**, **Square**, **Triangle**, **Sawtooth**, **ECG (Electrocardiogram)**, and **Noise**.
  * Customizable frequency and amplitude.
* **Hardware DAC Waveform Generator**:
  * Utilizes ESP32's hardware DAC (`dac_cosine` driver) to output a true analog cosine wave on GPIO25.
  * Adjustable frequency and attenuation (amplitude).
* **System Telemetry & Controls**:
  * Real-time monitoring of SRAM and PSRAM (total & free).
  * System specs (Chip model, core count, CPU frequency, and uptime).
  * Remote control to toggle onboard LEDs (GPIO2 and GPIO4).
* **Zero-Filesystem Overhead**:
  * Web interface is compiled directly into the C++ binary using a Python script, removing the need for SPIFFS/LittleFS filesystem setup.

---

## 🛠️ Project Structure

```text
├── wifi_test/
│   ├── wifi_test.ino       # Main Arduino/C++ firmware
│   ├── page_fixed.html     # Web UI source code (HTML/CSS/JS)
│   ├── page.h              # Hex-encoded Web UI compiled for C++
│   ├── gen_hex.py          # Python utility to compile HTML to page.h
│   └── upload.bat          # Upload helper script
├── .gitignore              # Ignores local bin files and notes
└── README.md               # Project documentation (This file)
```

---

## 🚀 Getting Started

### 1. Prerequisites
* **Arduino IDE** (or Arduino CLI) with the ESP32 board package installed.
* **Python 3** (if you want to modify the Web UI).

### 2. Modifying the Web UI (Optional)
If you make changes to `wifi_test/page_fixed.html`, you must recompile it to `page.h` before uploading:
```bash
python wifi_test/gen_hex.py
```

### 3. Compilation & Upload
You can compile and upload the project using Arduino IDE, or via the Arduino CLI with the following command:
```powershell
arduino-cli compile --upload --port COM5 --fqbn esp32:esp32:esp32wrover wifi_test/wifi_test.ino
```
*(Make sure to adjust the COM port and FQBN according to your setup).*

### 4. Running the Dashboard
1. Power on the ESP32.
2. Connect your computer/phone to the Access Point named `ESP32_WROVER_DASHBOARD` (No password required by default).
3. Open your browser and navigate to `http://192.168.4.1`.
4. Enjoy your portable Web Oscilloscope & Signal Generator!

---

## 🤝 Contributing
Contributions, issues, and feature requests are welcome! Feel free to check the issues page or submit a pull request to make **EspScopeGen** even better.

---

## 📄 License
This project is open-source and available under the [MIT License](LICENSE).
