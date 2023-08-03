# WARNING: Work in Progress

This Repository is a work in progress and may not build or compile. It does not yet contain key information such as schematic diagrams or hardware information and makes no garantees about the fitness or quality of the code.

# PicoEPiano - A Raspberry Pi Pico RP2040 Electric Piano Synthesizer

## Hardware Components

- Raspberry PI Pico
- Pico Audio module (Pimoroni Audio Pack)
- 128x64 OLED screen (SH1106)
- 1 rotary encoder (KY-040)
- MIDI In via USB or Serial

## Software Components

The firmware is build around the famous software sampler VST MDA ePiano. The basic sceleton for the i2s sound output, freeRTOS tasks and USB midi input are taken from the PicoADK project (https://github.com/DatanoiseTV/PicoADK-Firmware-Template.git). The RP2040 is overclocked to 402MHz. At the moment the max. polyphony is set to 96 instead of 8 in the original software.

## Compiling the firmware 
```bash
git clone --recursive https://github.com/Michi71/PicoEPiano.git picoepiano
cd picoepiano
export PICO_SDK_FETCH_FROM_GIT=1
mkdir build && cd build
cmake .. (optionally add -GNinja)
make (or ninja when you have used -GNinja)
```
Now you can find a main.uf2 in the build folder, which is your firmware.

## Copying the Firmware to the Raspberry Pi Pico

Plug in the USB cable while holding the **BOOT** button.
After that, a **RPI-RP2** disk volume will appear. Simply drag and drop the UF2 file to this drive and the Pico will
reboot after a moment, the drive will disappear and your firmware will be running.

## More information

Please check the [Pico Getting Started Guide](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf) on how to install the toolchain and required libraries for your OS.
