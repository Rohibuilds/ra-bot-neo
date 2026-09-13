# RA BOT NEO
**Rohi | RA TECH** · Robotics, electronics and embedded systems

An expressive ESP32-S3 desk companion with touch gestures, OLED eyes, sounds, a status LED and automatic sleep.

**[Open the complete code](firmware/RA_BOT_NEO_ESP32S3_v2/RA_BOT_NEO_ESP32S3_v2.ino) · [Wiring and parts](hardware/README.md) · [How to build](docs/README.md)**

## What is included
Single through five-plus tap reactions; three petting levels; blinking and looking around; sleep after 15 seconds and touch wake.

This repository restores the previously delivered project source and repairs identified software issues. It is a hardware prototype; source restoration does not constitute a new hardware test.

## Get started
1. Download the repository using **Code → Download ZIP** and extract it.
2. Read the [wiring table](hardware/README.md); it retains the recovered pin map.
3. Follow the [build and configuration guide](docs/README.md).
4. Open `firmware/RA_BOT_NEO_ESP32S3_v2/RA_BOT_NEO_ESP32S3_v2.ino` in Arduino IDE. Keep the containing folder and companion headers together.

## Code and validation
- Main source: [RA_BOT_NEO_ESP32S3_v2.ino](firmware/RA_BOT_NEO_ESP32S3_v2/RA_BOT_NEO_ESP32S3_v2.ino)
- [Dependency versions](DEPENDENCIES.md)
- [Fixes and validation record](docs/VALIDATION.md)
- [Build workflow](.github/workflows/build.yml) / [live build results](https://github.com/Rohibuilds/ra-bot-neo/actions)

## Source provenance
Recovered from the earlier RA TECH deliverables `RA_BOT_NEO_ESP32S3_v2.ino and RA_BOT_NEO_Complete_Build_Guide.pdf`. The wiring was cross-checked against those files. This update preserves the project's original purpose and identifies later repairs separately.

## Recent repairs
Added touch debounce, handled release during the wake animation, and refreshed the clock after a blocking hold reaction to prevent an immediate unintended sleep.
