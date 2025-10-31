## Description
Compass is a DIY handheld CNC router designed to make CNC machining more accessible.

![v1](./img/v1-alpha.png)

Unlike traditional CNC machines that move a cutting tool within a fixed workspace, Compass relies on users to guide the device around the workpiece directly. It automatically adjusts the cutting tool to stay on the programmed design path, enabling a significantly smaller device footprint while still handling large-scale cuts.

This is all accomplished using optical flow mouse sensors to track the position of the device, as well as a belt driven coreXY gantry to adjust the tool. The brains are a Teensy 4.1 microcontroller.

## Getting Started
This repo contains all of the source to build a Handheld CNC Router: the firmware, CAD files for 3D printing/manufacturing, BOM, and cable diagrams.

### Navigation
Internal
- [main](main/) - main firmware (written for [PlatformIO](https://docs.platformio.org/en/latest/core/quickstart.html) usage)
- [cad](cad/) - custom hardware (3D prints, laser cut sheet metal, cable harnesses)
- [pcb](pcb/) - printed circuit board designs for motherboard and sensors
- [dev](dev/) - development folder for debugging and testing
- [examples](examples/) - example gcode files to test with your machine!

External
- [BOM](https://docs.google.com/spreadsheets/d/12QS1CGSAB-lqKIOotCXDaXmJv9cjfRvVEAgQbRd7MnY/edit?usp=sharing)
- [Assembly Instructions](https://drive.google.com/drive/folders/1fIALrTWUbJRqTl93RGrUaR8dMfO7AI_g?usp=sharing)

## Disclaimer
This project is a work-in-progress. Development is rapidly occurring, so design files and firmware may change.
