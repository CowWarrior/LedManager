# Features

Planned features and ideas for future implementation. Bugs live in `BUGS.md`.

## Per-board configuration

### Problem
Board-specific settings are global `#define`s at the top of `src/main.cpp` (matrix width/height,
interlacing, GPIO pin, default effect…). The same firmware runs on very different boards:

| Board type | LEDs | Layout | Typical use |
|---|---|---|---|
| Exterior strips (around the house, e.g. FACADE) | 512 | 512x1 strip | patterns, solid, beat, rainbow |
| E-frames | 256 | 16x16 serpentine matrix | pixel-art images, showcase |
| Dev board (DEVBOARD) | — | — | development and testing |

Before every firmware update the constants must be edited by hand to match the target board, which
is easy to forget — and flashing the wrong geometry onto a board silently misbehaves.

On top of that, those `#define`s currently don't even reach the LED code (see `BUGS.md`, "LED
geometry `#define`s in `main.cpp` have no effect on the LED code"), and the image effect is
hardcoded to 16x16.

### Goals
- Pick the target board, not individual constants, when building/uploading.
- One place per board type that holds all of its settings, committed to the repo.
- Settings visible to every source file (fixes the `fastledutils.cpp` bug above).
- Effects adapt to the geometry (image/showcase only make sense on matrices).

### Options

**A. One PlatformIO environment per board type (compile time)**
Each board type gets an `[env:...]` in `platformio.ini` with its own `build_flags` (and upload port):

```ini
[env]                       ; shared settings
platform = espressif32
board = esp32dev
framework = arduino
lib_deps = fastled/FastLED @ ^3.4.0
           ArduinoJson

[env:eframe]
build_flags = -D LED_MATRIX_WIDTH=16 -D LED_MATRIX_HEIGHT=16 -D LED_MATRIX_INTERLACED=1

[env:strip]
build_flags = -D LED_MATRIX_WIDTH=512 -D LED_MATRIX_HEIGHT=1 -D LED_MATRIX_INTERLACED=0
```

Build/upload with `pio run -e eframe -t upload`, or pick the environment in the VS Code status bar.
- ✅ Small change, no runtime cost, FastLED's compile-time pin template keeps working.
- ✅ `build_flags` are seen by every file, so it fixes the geometry bug for free.
- ❌ Still one build per board type; choosing the wrong environment is still possible (but it's one
  choice instead of several constants).

**B. Geometry in `config.json` (runtime)**
One firmware for every board; width/height/interlacing/default effect stored in the board's config and
editable from the config page.
- ✅ A single firmware image for all boards; a board can be repurposed without reflashing.
- ❌ `leds[]` must be allocated at runtime; FastLED's data pin is a template parameter, so the pin
  needs a `switch` over supported pins (or stays compile-time).
- ❌ A wrong/corrupted config breaks the display — needs validation and safe fallbacks.

**C. Hybrid**
Compile-time environment per *hardware* (pin, max LEDs), runtime config for the rest (actual
width/height, default effect, features).

### Recommendation
Start with **A**: it removes the "remember to change constants" step, fixes the geometry bug and is a
small, low-risk change. Move to **C** later if repurposing boards without reflashing becomes useful.

### Related ideas
- **OTA updates (ArduinoOTA):** exterior strips are hard to reach with a USB cable; uploading over
  WiFi (`upload_protocol = espota`, `upload_port = <hostname>.local`) pairs naturally with one
  environment per board.
- **Board capabilities in `/api/info`:** report width/height/type so the web UI hides effects that
  don't fit (e.g. Image/Showcase on a 512x1 strip).
- **Image effect sized from the matrix** instead of hardcoded 16x16.

### Open questions
- Full list of board types, their GPIO pins and LED counts (are all strips 512? all frames 16x16?).
- Do different boards of the same type need different settings (hostname is already per-board in
  `config.json`)?
- Does each board type have a sensible default effect to bake in?

## Unique device identity (hostname per ESP32)

Related to per-board configuration, but separate: that one is about the board **type** (same for
every e-frame), this one is about each **individual** device (different for every ESP32).

### Context
Every board gets a DHCP reservation and a DNS name on the router, e.g.
`devboard.home.arpa → 192.168.0.122`. Duplicate or wrong hostnames make the router config confusing,
so each ESP32 (identified by its MAC) must have a unique hostname.

### Problem
- The hostname comes from `config.json`, falling back to `PIXELART` when not set — every
  unconfigured board shows up with the same name.
- The access point is always `PIXELART-AP`, so several boards in AP mode at once can't be told apart.
- **Trap:** pre-loading WiFi settings by putting `data/config.json` in the filesystem image (see
  `README.md`) copies the *same* hostname to every board flashed with that image.

### Ideas
- Default hostname derived from the MAC when none is configured, e.g. `PIXELART-2824` (last two MAC
  bytes); same suffix for the AP SSID (`PIXELART-2824-AP`).
- Show the MAC on the config page and in `/api/info` (already reported there) so it's easy to match a
  board to its router reservation.
- Keep WiFi credentials and hostname separate when pre-loading config: shared WiFi settings in the
  filesystem image, hostname set per board (or derived from the MAC).
- Optional mDNS (`<hostname>.local`) — not needed with the router's DNS, but useful for OTA uploads
  and for discovery by a control center (below).

## Control center (possibly a separate project)

One web UI to see and control every ESP32 at once (e.g. "all exterior strips → Festive").

### What the firmware would need (in this project)
- **CORS headers** on `/api/*` — a page served from elsewhere can't call the boards' APIs from the
  browser without `Access-Control-Allow-Origin`.
- **Discovery:** either a configured list of hostnames (fits the `*.home.arpa` naming) or mDNS
  service advertisement (e.g. `_ledmanager._tcp`).
- **Board capabilities and status in `/api/info`:** type, geometry, current effect, firmware version —
  so the control center can group boards and hide unsupported effects (see per-board configuration).
- Stable, documented API (currently parameters travel in the query string, even for PUT/POST).

### Hosting options
- A static page (HTML + JS) served from any web server, or from one of the ESP32s.
- A small server app (e.g. in Docker) that also handles grouping, scenes and schedules.
- An existing home-automation platform (e.g. Home Assistant) talking to the boards' REST API,
  instead of building a new UI.

### Open questions
- New repo or a folder in this one?
- Scenes/schedules (e.g. exterior strips on at sunset)? The boards already sync time via NTP.
