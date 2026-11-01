# Waveshare ESP32-S3-AUDIO-Board

ESP32-S3R8 (8 MB PSRAM, 16 MB flash) AI speaker board with a 2-inch ST7789
module wired to the general-purpose header. No touch panel.

Chinese: [README.zh-CN.md](README.zh-CN.md).

## What is verified

`whale` and `pet` display correctly from the stock bootloader. The same app
source that runs on the C3 board runs here unchanged.

## Deployment

Standard PlatformIO flow:

```sh
pio run -d firmware -e dada-s3-pet -t upload --upload-port COM4
```

One requirement: `firmware/platformio.ini` must pin `board_build.partitions =
partitions.csv`. Without it PlatformIO flashes the app at its own default offset
(`0x10000`) while the partition table declares `0x20000`, and the image never
starts. That mismatch looks exactly like a broken bootloader, which cost an
evening of chasing the wrong suspect.

To go back to the factory firmware, keep a full 16 MB dump of the board before
touching it and restore with `write_flash 0x0 backup.bin`.

## Wiring

| Module | Pin | Header position |
| --- | --- | --- |
| VCC | 3V3 | right column, 1st |
| GND | GND | right column, 2nd |
| DIN | GPIO9 | right column, 6th |
| CLK | GPIO4 | left column, 6th |
| CS | GPIO3 | left column, 5th |
| DC | GPIO7 | left column, 9th |
| RST | GPIO6 | left column, 8th |
| BL | GPIO5 | left column, 7th |

The board's own 18-pin LCD connector routes RST to TCA9555 EXIO0, which is not
driven yet; RST is wired to GPIO6 instead so the driver can do a hardware reset.

## Not available

- **Touch**: the module has no touch controller, so `input` is absent and
  input-driven apps are refused by the capability check.
- Audio and networking are outside the current contract set.

## Note on serial output

The app prints at startup only, so a monitor attached later sees nothing. `pet`
prints a heartbeat every five seconds to make "alive" distinguishable from
"hung". Add the same to any long-running app before concluding it crashed.
