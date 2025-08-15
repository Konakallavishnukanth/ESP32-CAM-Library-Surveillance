# ESP32-CAM Library Surveillance System

This project uses an **ESP32-CAM module** to create a simple library surveillance system.  
It allows **secure login**, **live video streaming**, and **snapshot capture** over Wi-Fi.

## Features
- **Secure Login** (username & password)
- **Live Video Stream** from ESP32-CAM
- **Capture Snapshot** and download as image
- **Wi-Fi Setup** using WiFiManager
- Simple HTML interface with background image

## How It Works
1. The ESP32-CAM connects to Wi-Fi using **WiFiManager**.
2. Users access the device's IP address through a web browser.
3. A **login page** is shown before accessing the camera feed.
4. After login, users can:
   - View **live video stream** (`/stream`)
   - Capture and download snapshots (`/capture`)
5. The camera uses `esp_camera` library for image capture and streaming.

## Hardware Required
- ESP32-CAM (AI Thinker model)
- FTDI USB-to-Serial programmer (for uploading code)
- Jumper wires
- Power supply (5V)

## How to Use
1. Upload the code to the ESP32-CAM using Arduino IDE.
2. Connect to the "ESP32-CAM-Setup" Wi-Fi network.
3. Enter your Wi-Fi credentials.
4. Open the ESP32-CAM’s IP address in your browser.
5. Login using:
   - **Username:** admin(As you set)
   - **Password:** password(As you set)

## Example Applications
- Library surveillance
- Classroom monitoring
- Remote observation

---

**Author:** Vishnu kanth konakalla  

