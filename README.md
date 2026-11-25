# LedManager

ESP32 firmware that drives a 16x16 WS2812B LED matrix and serves a web UI / HTTP API for choosing
effects and managing pixel-art images. Built with PlatformIO (Arduino framework, `esp32dev`).

## Hardware setup (Windows)

### USB driver

This ESP32 board (ESP32-D0WD-V3) uses a **Silicon Labs CP2102** USB-to-UART chip
(hardware ID `VID_10C4&PID_EA60`). Windows needs the CP210x VCP driver before a COM port appears:

**Download:** https://www.silabs.com/software-and-tools/usb-to-uart-bridge-vcp-drivers?tab=downloads

1. Download the **CP210x Universal Windows Driver** and unzip it.
2. Right-click `silabser.inf` → **Install**.
3. Unplug and replug the board. It should appear in Device Manager under **Ports (COM & LPT)** as
   *Silicon Labs CP210x USB to UART Bridge (COMx)*.

> **CH341SER will not work for this board.** That driver is for WCH CH340/CH341 chips
> (`VID_1A86`), which some other ESP32 boards use. With the wrong (or no) driver, the board shows in
> Device Manager as *CP2102 USB to UART Bridge Controller* with error **Code 28** and no COM port.

### COM port

- The board currently enumerates as **COM7**. If the number changes (e.g. a different USB socket),
  update `upload_port` and `monitor_port` in `platformio.ini`.
- The port speed shown in Device Manager (9600) doesn't matter — tools set their own. The serial
  monitor uses **115200** baud.
- Only one program can use the COM port at a time. Close any serial monitor before uploading.

### Checking the connection

Ask the chip to identify itself (works regardless of what firmware is flashed; resets the board):

```
~/.platformio/packages/tool-esptool/esptool.exe --port COM7 chip-id
```

## Build and flash

```
pio run                  # build firmware
pio run -t upload        # flash firmware
pio run -t buildfs       # build the filesystem image from data/
pio run -t uploadfs      # flash the filesystem image
pio device monitor       # serial monitor (115200)
```

Web files are edited in `data-source/` and minified into `data/www/` automatically on build. After
changing them, rebuild **and upload the filesystem image** — flashing the firmware alone doesn't
update the web pages.
