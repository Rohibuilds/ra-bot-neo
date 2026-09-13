# Build RA BOT NEO
1. Assemble the [parts and wiring](../hardware/README.md) with USB power disconnected.
2. Install Arduino IDE and Espressif **esp32 core 3.3.0**. Select **ESP32S3 Dev Module**, the actual USB serial port and the flash/PSRAM settings matching the module label.
3. Install the three versions listed in [DEPENDENCIES.md](../DEPENDENCIES.md).
4. Open `firmware/RA_BOT_NEO_ESP32S3_v2/RA_BOT_NEO_ESP32S3_v2.ino`. Verify, then Upload. This project needs no Wi-Fi credentials.
5. Open Serial Monitor at **115200 baud**. The OLED should show the RA TECH boot animation, then expressive eyes.
6. Test every gesture below, then leave it untouched for 15 seconds and touch to wake.

| Gesture | Response |
|---|---|
| 1 quick tap | Happy hello |
| 2 quick taps | Excited bounce |
| 3 quick taps | Confused/funny |
| 4 quick taps | Side-eye / glitch |
| 5 or more | Crazy reaction |
| Hold 0.6–2 seconds | Squishy pet |
| Hold 2–5 seconds | Cuddle / UWU |
| Hold over 5 seconds | Heart eyes |
| No touch for 15 seconds | Sleep |

Multi-taps must fall within the original 420 ms decision window. Very short electrical pulses under 25 ms are filtered. The original expression sequences use blocking animations, so rapid taps during an already-playing reaction may not be captured; finish one reaction before starting another.

## Troubleshooting
- **OLED blank:** verify 3V3/GND, GPIO8/9, address 0x3C and an SSD1306 rather than SH1106 display.
- **Always touching:** check TTP223 mode, mounting and active-HIGH output. Keep the sensor away from wiring and power noise.
- **Buzzer clicks:** use a passive piezo for variable-pitch tones.
- **LED unstable / board resets:** verify common ground, data direction, level shifting and supply decoupling.
- **Upload fails:** close Serial Monitor, select the real port, and hold BOOT while starting upload if the board requires it.
