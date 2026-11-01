# 微雪 ESP32-S3-AUDIO-Board

ESP32-S3R8（8 MB PSRAM、16 MB flash）AI 音箱板，2 寸 ST7789 模块飞线到通用排针。无触摸。

English: [README.md](README.md)。

## 已验证

`whale` 和 `pet` 在原厂 bootloader 下能正常显示。同一份应用源码在 C3 板上跑，
在这里不改一行也能跑。

## 部署

标准 PlatformIO 流程：

```sh
pio run -d firmware -e dada-s3-pet -t upload --upload-port COM4
```

有一个前提：`firmware/platformio.ini` 必须显式写 `board_build.partitions =
partitions.csv`。不写的话 PlatformIO 会把 app 烧到它自己的默认偏移（`0x10000`），
而分区表声明的是 `0x20000`，镜像永远起不来。这个错位看起来**非常像 bootloader 坏了**，
我为此追错方向追了一晚上。

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
- 音频与网络不在当前契约范围内。

## 关于串口输出

app 只在启动时打印，之后才连上的监视器什么都看不到。`pet` 每 5 秒打印一次心跳，
让"活着"和"卡死"可以区分。在断定某个应用崩溃之前，先给它加上同样的心跳。
