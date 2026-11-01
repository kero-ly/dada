# 微雪 ESP32-S3-AUDIO-Board

ESP32-S3R8（8 MB PSRAM、16 MB flash）AI 音箱板，2 寸 ST7789 模块飞线到通用排针。无触摸。

English: [README.md](README.md)。

## 已验证

`whale` 和 `pet` 在原厂 bootloader 下能正常显示。同一份应用源码在 C3 板上跑，
在这里不改一行也能跑。

## 部署：只烧 app

**不要烧这块板的 bootloader。** 用默认 `esp32-s3-devkitm-1` bootloader 编译出来的
固件在这块硬件上起不来，原因尚未定位。原厂 bootloader 是好的，保留它：

```sh
pio run -d firmware -e dada-s3-pet
esptool --chip esp32s3 -p COM4 -b 921600 \
    write_flash 0x20000 .pio/build/dada-s3-pet/firmware.bin
```

这样把 app 放到 `0x20000`，正好是原厂分区表里 `ota_0` 的位置。也因此
**不能用 `pio run -t upload`**——那条路会把 bootloader 一起重写。

想恢复出厂固件，动手前先留一份完整 16 MB 备份，用 `write_flash 0x0 backup.bin` 还原。

## 接线

| 模块 | 引脚 | 排针位置 |
| --- | --- | --- |
| VCC | 3V3 | 右列第 1 |
| GND | GND | 右列第 2 |
| DIN | GPIO9 | 右列第 6 |
| CLK | GPIO4 | 左列第 6 |
| CS | GPIO3 | 左列第 5 |
| DC | GPIO7 | 左列第 9 |
| RST | GPIO6 | 左列第 8 |
| BL | GPIO5 | 左列第 7 |

板子自带的 18-pin LCD 座把 RST 接到 TCA9555 的 EXIO0，那个还没驱动，
所以 RST 改接 GPIO6，让驱动能做硬复位。

## 不可用

- **触摸**：这块屏没有触控 IC，`input` 能力缺失，需要输入的应用会被能力检查拒绝。
- **串口日志**：app 在跑，但 USB 口没有任何输出。未定位。
- 音频与网络不在当前契约范围内。
