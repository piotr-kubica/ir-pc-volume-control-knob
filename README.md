# ir-pc-volume-control-knob
IR and PC volume control knob allows to control 
- prev/next/play/volume on your PC
- tuner/audio-in/on/off/volume of the Denon RCD-M39 amplituner using IR

The goal of this project was to create a volume control knob for multiple devices.

[<img src="/img/knob-front.png" width="500"/>](/img/knob-front.png)

## Hardaware
- SparkFun Pro Micro 5v/16Mhz 
- rotary encoder
- ssd1306 oled display (i2c version)
- IR led, transistor and resistor (used in the IR transmitter)
- 3D printed case (see stl and 3mf files)
- M3 bolts, 2x tactile buttons, JST connector and socket, wires, proto board, USB cable, rubber pad (so the knob does not slip on flat surface)

## Software 

I have used Arduino Studio to upload ir-pc-volume-control-knob.ino to the MCU.
I have also created a custom font using the png2bdf cli tool.

## How the control know works
Volume increase/decrease by turning the knob left/right.

Mute by clicking the mute button (it will mute the device with active mode).

Mode is changed by clicking the mode button.

Press once in clickable area to enter selection for mode.
Navigate by turning knob. Press angain to select.

Double click clickable area to change volume stepping ("S:1" or "S:2") for move volume per knob turn.

[<img src="/img/knob-details.png" width="804"/>](/img/knob-details.png)

[<img src="/img/knob.png" width="400"/>](/img/knob.png )
[<img src="/img/knob-side.png" width="400"/>](/img/knob-side.png)
[<img src="/img/knob-without-wheel.png" width="400"/>](/img/knob-without-wheel.png)
[<img src="/img/transmitter.png" width="400"/>](/img/transmitter.png)
[<img src="/img/knob-rubber.png" width="400"/>](/img/knob-rubber.png)
          
