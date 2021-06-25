# Wheely: Design and Assembly

## 1. Overview
The wheely robot is a ground robot employing mecanum wheels for instantenous movement in any direction.

## 2. Preparation
The following items need to be ordered before building a new wheely:
- Commercial mecanum wheel robot: [aliexpress](https://www.aliexpress.com/item/4001193081747.html?spm=a2g0s.9042311.0.0.14964c4ddIATUA) (150 USD including shipping with a lead time of about 2-3 weeks)
- ESP32 development kit: [digikey](https://www.digikey.ch/product-detail/de/espressif-systems/ESP32-DEVKITC-32D/1965-1000-ND/9356990) (9 CHF with a lead time of about 2 days)
- Battery: [swaytronic](https://www.swaytronic.ch/LiPo-Akku---Swaytronic/LiPo-Akku-3S-11-1V-248/35C---70C/swaytronic-lipo-3s-11-1v-3400mah-35c-70c-t.html) (50 CHF with a lead time of about 2 days)
- Charger: [swaytronic](https://www.swaytronic.ch/Ladegeraete/Ladegeraete-12V-DC/up100ac-plus.html) (70 CHF with a lead time of about 2 days)
- Connector: [swaytronic](https://www.swaytronic.ch/LiPo---Zubehoer/LiPo-Stecksysteme---Zubehoer/lipo-stecksystem-new-dean-t-plug-mit-schutzkappe.html) (15 CHF with a lead time of about 2 days)
## 3. Frame

## 4. Electronics
The ESP32 acts as a translator between the manager and motor controller. To connect ESP32 and motor controller directly solder several wires on the ESP32 and a pin header with 6 pins, which can be plugged into the motor controller. The connections are as follows:
- 5V <-> 5V
- GND <-> GND
- Pin 13 <-> DAT
- Pin 12 <-> CMD
- Pin 15 <-> CLK
- Pin 14 <-> CS

![alt text](../../assets/pix/robots/wheely/electronics/esp32_motor_controller.png)

## 5. Assembly
