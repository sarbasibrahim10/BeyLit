# Standalone Uno SG90 diagnostic

Separate PlatformIO project; does not replace ESP32 firmware.

Power off before rewiring. Remove bracket load. Move servo signal from ESP32 GPIO18 to Uno D9; never connect Uno D9 to ESP32 GPIO18. Servo red goes to Uno 5V and brown/black to Uno GND, as currently supplied. USB/Uno power capacity remains unverified under load. Initial centering can jump. Disconnect ESP32 from this isolated test.

Build: `pio run -d diagnostics/uno-servo`
Upload after identifying the Uno port: `pio run -d diagnostics/uno-servo -t upload --upload-port COM_NUMBER`
Monitor at 115200. Send T to start a slow 1500 -> 1400 -> 1600 -> 1500 microsecond command sequence. Send X to stop pulses. Signal detaches at completion or after 12 seconds. No movement is enabled on startup.

Pass: unloaded servo visibly moves both ways and returns without resets or mechanical contact. Commands are not position feedback. If it moves here, investigate ESP32 signal/wiring; if it does not, supply, lead connections and servo condition remain candidates. Measure voltage during commands; an idle measurement alone is insufficient.

No Uno upload or physical servo test has yet been completed. Windows currently exposes only the ESP32 CP210x serial device and Bluetooth ports. An unknown USB device reports descriptor failure; its identity is unconfirmed. Try a known data-capable USB cable and another laptop port for the Uno, then enumerate ports again.
