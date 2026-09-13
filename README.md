<div align="center">

# RA BOT NEO

**Expressive ESP32-S3 desktop robot with animated OLED faces, touch interaction and smart sleep behavior.**

![Status](https://img.shields.io/badge/status-working_prototype-00D9A5?style=flat-square)
![Platform](https://img.shields.io/badge/platform-ESP32-S3_%C2%B7_Robotics-101820?style=flat-square)
![Brand](https://img.shields.io/badge/by-RA_TECH-101820?style=flat-square)

</div>

## Overview

RA BOT NEO is a compact expressive desk robot built under RA TECH. It uses animated OLED faces, touch input, and timed sleep behavior to create a simple but engaging personality on resource-constrained hardware.

> **Project status:** Working prototype

## Highlights

- Multiple expressive OLED faces
- Touch-triggered reactions
- Automatic sleep after inactivity
- Touch-to-wake behavior
- Random eye-look animations
- Compact ESP32-S3 architecture

## Hardware

| Component | Role |
|---|---|
| ESP32-S3 development board | Main processing and control |
| 0.96-inch I2C OLED | Project subsystem |
| Touch input | Project subsystem |
| Buzzer or optional audio feedback | Project subsystem |
| Li-ion power system | Project subsystem |
| Compact custom enclosure | Project subsystem |

## Repository structure

```text
ra-bot-neo/
├── firmware/   Tested source code and configuration notes
├── hardware/   Wiring, components, PCB, and enclosure information
├── docs/       Build guide, calibration, results, and troubleshooting
├── media/      Prototype images, diagrams, and demo links
└── README.md   Project overview and release status
```

## Current public release

This initial release establishes the verified project overview and a clean documentation structure. Firmware, wiring diagrams, and media will be added only after each item is checked for accuracy and private credentials are removed.

## Roadmap

- [ ] Publish the final v3 animation firmware
- [ ] Add the verified wiring diagram
- [ ] Document every touch reaction
- [ ] Add enclosure files and a high-quality demo GIF

## Safety and reproducibility

- Verify every supply voltage before powering the controller or modules.
- Use a common ground and a power source sized for peak motor or audio current.
- Never commit Wi-Fi passwords, API keys, personal contact details, or certificates.
- Recheck the published pin map against the tested hardware before assembly.

---

<div align="center">

**Designed and developed by [Rohi · RA TECH](https://github.com/Rohibuilds)**

<sub>Build. Test. Improve. Share.</sub>

</div>
