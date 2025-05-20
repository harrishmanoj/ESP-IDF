# 🚦 Vehicle Indicator Control System (ESP32 + ESP-IDF)

This project implements a modular and real-time vehicle indicator system using the **ESP32 microcontroller**, written entirely in **C** with the **ESP-IDF framework**. The system manages **left**, **right**, and **hazard light** operations using push buttons and LEDs, simulating a real-world automotive blinker control.

---

## 🧠 Features

- 🔄 Toggle **left** and **right indicators** via physical buttons
- ⚠️ Activate **hazard lights** when both buttons are pressed
- 💡 LED blinking at 300ms intervals using **PWM**
- 🧩 Modular architecture separating hardware drivers and logic
- ⏱️ **FreeRTOS-based 100ms scheduler** for real-time task handling
- 📟 **UART logging** of button events and indicator state changes

---

## 📦 Project Structure

indicator-project/
├── main/
│ ├── indicator.c # Core logic & FreeRTOS tasks
│ ├── gpio_driver.c # GPIO button setup and debounce
│ ├── pwm_driver.c # LED PWM control
│ ├── uart_driver.c # UART initialization & logging
│ └── CMakeLists.txt
├── components/ # (Optional) External reusable modules
├── CMakeLists.txt
├── sdkconfig
└── README.md

## 🔧 Requirements

- ESP32 Dev Board
- ESP-IDF (v4.x or newer)
- Two push buttons (for left and right signals)
- Two LEDs (left and right indicators)
- USB-UART connection for serial logs

---

## 🚀 Getting Started

### 1. **Clone the Repository**
```bash
https://github.com/harrishmanoj/ESP-IDF/edit/main/VehicleIndicator.git
cd indicator-project



```
├── CMakeLists.txt
├── main
│   ├── CMakeLists.txt
│   └── main.c
└── README.md                  This is the file you are currently reading
```
Additionally, the sample project contains Makefile and component.mk files, used for the legacy Make based build system. 
They are not used or needed when building with CMake and idf.py.

BLE control was not implemented as it was only mentioned in the submission instructions and not part of the core task.
