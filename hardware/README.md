# Parts and wiring
Recovered from the NEO v2 source and original complete build guide.

| Part | Quantity | Connection |
|---|---:|---|
| ESP32-S3 DevKit | 1 | USB power/programming |
| SSD1306 128×64 I2C OLED | 1 | VCC → 3V3; GND → GND; SDA → GPIO8; SCL → GPIO9 |
| TTP223 touch module | 1 | VCC → 3V3; GND → GND; OUT → GPIO4 |
| WS2812 pixel | 1 | VCC → regulated 5V; GND → GND; DIN → GPIO5 through 330 Ω |
| Small passive piezo buzzer | 1 | Signal → GPIO6; other lead → GND |
| USB supply/cable and wiring | As needed | All grounds common |

Use the touch module's momentary, active-HIGH mode. Do not feed a 5V touch output into the ESP32. A 5V WS2812 may need a 74AHCT125 or equivalent data level shifter; retain GPIO5 as its input. Use a transistor driver for a buzzer requiring more current than a GPIO can supply. Keep the original 0x3C OLED address unless the actual module uses a different address.
