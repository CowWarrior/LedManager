# Bugs

Known bugs and their status. When fixing one, mark it **Fixed** with the commit hash in the same
commit as the fix.

## Open

### Default effect loses its parameters at boot
- **Symptom:** setting Beat with a custom color as default boots back into blue Beat. Same class of
  problem for every effect with parameters: Solid boots black (looks off), Pattern boots blank,
  Image boots blank, and brightness always resets to 64.
- **Cause:** only the effect *name* is persisted. `HandleSetEffect` stores
  `_config.effectDefault = _currentEffect`; color, image name and brightness are never written to
  `config.json`, and `setup()` calls `ActivateEffect(_config.effectDefault)` with no parameters.
  Beat's built-in fallback color is `CRGB(0, 0, 64)` (`src/fastledutils.cpp`, `DrawLEDBeatEffect`).
- **Where:** `src/main.cpp` — `HandleSetEffect`, `SerializeConfig`/`DeserializeConfig`, `setup()`.
- **Fix idea:** add `effect.color`, `effect.imgname`, `effect.brightness` to the config, save them on
  `setdefault=1`, pass them to `ActivateEffect` at boot; fall back to built-in defaults when absent.

### `festive` can't be restored as the default effect
- **Symptom:** setting Festive as default boots into no effect.
- **Cause:** typo — `ActivateEffect` sets `_currentEffect = "fastive"`, which is what gets saved; at
  boot `"fastive"` matches no branch and falls through to the no-op `else`.
- **Where:** `src/main.cpp`, `ActivateEffect`, `festive` branch.

### Config page sends WiFi settings in the wrong JSON shape (suspected — verify)
- **Symptom (expected):** saving WiFi settings from the AP config page stores `"null"` for SSID,
  password and hostname.
- **Cause:** `data-source/config.htm` sends flat JSON `{"ssid","pwd","hostname"}`, but
  `DeserializeConfig` reads nested `doc["wifi"]["ssid"]` etc. It also drops `effect.default`.
- **Where:** `data-source/config.htm` (~line 118), `src/main.cpp` `DeserializeConfig`.
- **Related:** `DeserializeConfig` ignores the `deserializeJson` error and always returns `true`, so
  malformed JSON also overwrites the config. Fixing `config.htm` requires a filesystem upload.

### Config file can be lost if the board resets mid-save
- **Cause:** `FSWriteFile` deletes the existing file, then creates and writes the new one. A reset or
  brownout in between leaves no `config.json` → boots into AP mode.
- **Where:** `src/fileutils.cpp`, `FSWriteFile`.
- **Fix idea:** write to a temp file, then rename over the original.
- **Related:** every FS helper calls `SPIFFS.begin(true)`, which **formats** the filesystem if a mount
  ever fails — wiping config, images and web pages.

### WiFi password printed in plain text on serial
- **Cause:** with `DEBUGMODE` on, `ReadConfig` and `DeserializeConfig` print the full config JSON,
  including `wifi.pwd`, at every boot.
- **Where:** `src/main.cpp`, `ReadConfig`, `DeserializeConfig`, `SaveConfig`, `HandleSetConfig`.

### LED geometry `#define`s in `main.cpp` have no effect on the LED code
- **Cause:** `LED_MATRIX_WIDTH`/`HEIGHT`/`INTERLACED`/`GPIO_PIN` are defined in `main.cpp`, but
  `fastledutils.cpp` is a separate translation unit and uses its own `#ifndef` defaults (16x16,
  not interlaced, pin 13). `main.cpp`'s `LED_NUM_LEDS` also references undefined `MATRIX_WIDTH`.
- **Also:** `DrawLEDImageEffect` is hardcoded to 16x16.
- **Fix idea:** move them to `build_flags` in `platformio.ini` so every file sees them.

## Fixed

### Pattern effect wrote past the LED buffer, corrupting the WiFi config — `734f9c9`
When 256 isn't a multiple of the pattern length, the draw loop wrote past `leds[]` into adjacent
globals (`_currentEffect`, then `_config.wifiSSID`/`wifiPassword`). The next save persisted the
damage → SSID `null`, garbage password, AP mode at boot. A 17-color custom pattern reproduced it.

### Empty pattern hung the board — `734f9c9`
A pattern with no colors made the draw loop step by 0 forever.

### "Set as default effect" checkbox always saved the effect as default — `0f325c2`
`$("#chkDefaultEffect").val()` is always `"on"`; now uses `.is(":checked")`. Every effect change was
rewriting `config.json`, which is how the pattern corruption above got persisted.

### Invalid SSID waited the full 60 s WiFi timeout — `014c435`
Empty, `"null"` or >32-char SSIDs now skip straight to AP mode.

### Current effect string mangled when received by client — probably `734f9c9` (unconfirmed)
Old Known Issue from `main.cpp`. `_currentEffect` sits right after the LED buffer in memory, so the
pattern overflow would have overwritten it. Reopen if it still happens.
