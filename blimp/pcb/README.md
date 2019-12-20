# Blimpy PCB

![alt text](../../assets/pix/controller.jpg)

## TODO

- On/Off switch needs replacement
- thicker Board
- current limiting for H-Bridges need resistors between pin x and y
- PWM for driving H-Bridges need higher frequency out of the audible range
- better battery state of charge estimation
- RGB LEDS on the backside for illuminating blimp

## Connectors

All:

- Type: JST PH RA (S2B-PH-SM4)
- Pitch: 2.00mm (0.079in)
- Current: 2 A
- Wire Size: 32 to 24 AWG

## Cables

Actors:

- Wire Gauge: 26 (28)
- Cross Section: 0.129mm2 (0.0810mm2)
- Max Current: 2.2A (1.4A)

Battery:

- Wire Gauge: 24
- Cross Section: 0.205mm2
- Max Current: 3.5A

## Board Settings

Trace Widths for 1oz Copper:

- Logic: 0.2mm, ~8mil (.8A)
- Power: 0.4mm, 15mil (1A)
- Actors: 0.8mm, 30mil (2A)
- Battery: 1.2mm, 60mil (3A)

Text Size: Vector, 0.6, 15%

Manufacturing Requirements:

- Min Trace: 6mil
- Min Space: 6mil
- Min Hole: 0.3mm

## Assemblies

We might want to consider ordering ready made cable assemblies:

- https://www.adafruit.com/category/598
- https://www.fabrimex.com/products/cable-assembly.html

## Required Tools

- JST Crimp Tool
