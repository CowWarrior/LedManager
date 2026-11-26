# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

ESP32 (Arduino framework, PlatformIO) firmware that drives a 16x16 WS2812B LED matrix via FastLED and
exposes a web UI + HTTP API for picking effects and managing pixel-art images. Inspired by
NightDriverStrip.

## Commands

PlatformIO's `pio` may not be on PATH; it lives at `~/.platformio/penv/Scripts/pio.exe`. Single env: `esp32dev`.

```
pio run                       # build firmware
pio run -t upload             # flash firmware (COM7, set in platformio.ini)
pio run -t buildfs            # build SPIFFS image from data/
pio run -t uploadfs           # flash SPIFFS image — needed after ANY change under data-source/ or data/
pio device monitor            # serial log, 115200 baud
```

There are no unit tests (`test/` holds only the PlatformIO README) and no linter.

## Hardware setup (Windows)

The ESP32 board (ESP32-D0WD-V3) uses a **Silicon Labs CP2102** USB-to-UART chip
(`VID_10C4&PID_EA60`), which needs the CP210x VCP driver:
https://www.silabs.com/software-and-tools/usb-to-uart-bridge-vcp-drivers?tab=downloads
(CP210x Universal Windows Driver → unzip → right-click `silabser.inf` → Install → replug board).

- The **CH341SER** driver does *not* work for this board — it's for WCH CH340/CH341 chips
  (`VID_1A86`), used by other ESP32 boards. Without the right driver the device shows in Device
  Manager with Code 28 and no COM port appears.
- The board currently enumerates as **COM7**; if the number changes (e.g. different USB socket),
  update `upload_port`/`monitor_port` in `platformio.ini`.
- Port speed in Device Manager (9600) is irrelevant; tools set their own (monitor uses 115200).
- Quick connectivity check without depending on the flashed firmware:
  `~/.platformio/packages/tool-esptool/esptool.exe --port COM7 chip-id`
- Only one program can hold the COM port; close any serial monitor before uploading.

## Build pipeline side effects

Two `pre:` extra_scripts run on **every** build (see `platformio.ini`):

- `pre_buildscript_versioning.py` increments the counter in `versioning` and regenerates
  `include/version.h` (`VERSION`, `VERSION_SHORT`, `BUILD_NUMBER`). Major/minor are hardcoded in the
  script (`V_MAJOR`, `V_MINOR`). Never hand-edit `version.h`. Because of this, `versioning` and
  `version.h` show as modified after any build.
- `auto_minify_files.py` minifies `.htm`/`.js` from `data-source/` into `data/www/` (htmlmin/jsmin,
  auto-installed into PlatformIO's Python), skipping files whose mtime hasn't advanced past what's
  recorded in `.file_timestamps`.

**Edit web files in `data-source/`, never in `data/www/`** — the latter is generated output that gets
overwritten. Non-minified assets (e.g. `favicon.ico`) live only in `data/www/`. Minification alone
does not reach the device: the SPIFFS image must be rebuilt and uploaded separately.

## Architecture

- `src/main.cpp` — everything app-level: config load/save, WiFi/AP decision, all HTTP route
  registration and handlers, effect dispatch (`ActivateEffect`), and showcase mode (cycles through
  `/images/*.dat` every 20 s). `loop()` is just: handle client requests → showcase tick → `DrawLEDFrame()`.
- `MiniServ` — thin wrapper over the Arduino `WebServer` (exposed publicly as `WServer`, which
  `main.cpp` registers routes on directly). Handles WiFi STA connect/auto-reconnect, AP fallback,
  query-string parsing (URL-decoded), and serving SPIFFS files.
- `fastledutils` — effect engine. A string effect name (`Solid`, `Beat`, `Rainbow`, `Pattern`,
  `Image`, `Default`) plus a string parameter set via `SetLEDCurrentEffect()`; `DrawLEDFrame()` is
  non-blocking and frame-rate-gated by `millis()`. Static effects (Pattern, Image) draw once, using
  `ledFrameIndex == 0` as the "needs redraw" flag.
- `fileutils` — SPIFFS read/write/delete/exists (each call does `SPIFFS.begin(true)`).
- `NtpHelper` — NTP time sync (server `time.nrc.ca`), only when STA WiFi connects.
- `arduinoutils` — serial print wrappers, onboard-LED blink, hex-string→int.

### Boot flow
Read `/config.json` from SPIFFS → if present, connect to WiFi → if WiFi fails or no config, start
open AP `PIXELART-AP`, and in AP mode `/` serves the config page instead of the main page. Config
is posted back via `/api/config?configdata=<json>`, then the device is rebooted via `/api/reboot`.
`data/config.json` is gitignored (contains WiFi credentials).

### HTTP API
All under `/api/`: `effect` (GET/PUT/POST), `images`, `image` (GET/PUT/DELETE), `storage`, `config`
(GET/PUT/POST), `reboot`, `info`. Parameters travel in the **query string** (even for PUT/POST),
including full image data and config JSON. Effect names in the API are lowercase (`solid`, `beat`,
`rainbow`, `northpole`, `quebec`, `festive`, `pattern`, `image`, `showcase`, `off`, `default`);
`setdefault=1` persists the effect as the boot default.

### Data formats
- Colors are 6-char uppercase hex strings (`RRGGBB`). A **pattern** is a concatenation of these,
  repeated along the whole strip.
- An **image** (`/images/<name>.dat`) is 256 concatenated `RRGGBB` values (1536 chars), row-major
  for a 16x16 grid. Wiring is serpentine: even rows are reversed when mapped to the strip.

### Bugs
Known bugs are tracked in `BUGS.md` (not GitHub Issues). Check it before working on a bug, and in
the fixing commit move the entry to **Fixed** with the commit hash.

### Gotchas
- The `LED_*` `#define`s at the top of `main.cpp` do **not** reach `fastledutils.cpp` (separate
  translation unit); it uses its own `#ifndef` defaults. To change matrix size/pin, change them in
  `fastledutils.cpp` or as `build_flags`. `DrawLEDImageEffect` is also hardcoded to 16x16.
- Debug serial output is gated per-module by `DEBUGMODE`, `MINISERV_DEBUGMODE`, and
  `FASTLEDUTILS_DEBUGMODE`.
- SPIFFS paths: `SPIFFS.open("/images")` must not have a trailing slash.
