# playpack
 A small circuit which will play back audio files.

This project includes instructions to build a small module which will play back audio files autonomically. The field of application varies from music-greeting-cards to door-bells. Main parts of the circuit are the Atmel AVR ATmega48 microcontroller, an Atmel AT45DB161 dataflash for storage and a Philips TDA7052 as amplifier.

To update the device simply convert your preferred audio data into an uncompressed raw audio file (8bit unsigned, mono samples) at a fixed frequency and upload it to the device via EIA-232 (115.2kbaud, 8N1) Z-Modem protocol (e.g. use TeraTerm). Once the device powers up it starts playing the data from it's flash memory as an endless loop.
