# 验收记录 · 2026-11-01 · ESP32-S3 跨板验证（部分）

第一次让 dada 在 C3 以外的芯片上跑起来。目标不是完整 gate 4，是先回答"契约在换芯片后还成立吗"。

- 板卡：微雪 ESP32-S3-AUDIO-Board（S3R8，8 MB PSRAM，16 MB flash）
- 屏幕：微雪 2inch LCD Module（ST7789V，240×320，**无触摸**），飞线到通用排针
- 工具链：PlatformIO Core 6.2.0 / Espressif32 6.8.1 / ESP-IDF 5.3.0

## 结果：whale 和 pet 都能显示 —— 通过（显示部分）

| 应用 | 结果 |
| --- | --- |
| `whale` | ✅ 画面正常（160×120 头像，2px 格） |
| `pet` | ✅ 画面正常，呼吸可见 |

**同一份应用源码，C3 和 S3 都跑起来了，应用代码一行没改。** 这是契约层第一次跨芯片生效。

## 关键发现：不能烧自己的 bootloader

排查过程很长，结论很干脆：

| 组合 | 结果 |
| --- | --- |
| 我们的 bootloader + 我们的分区表 + 我们的 app | ❌ 不启动（背光不亮、串口静默） |
| **原厂 bootloader + 原厂分区表 + 我们的 app** | ✅ 正常 |

排除过的假设：

- ❌ 不是分区表错位（app 确实在 `0x20000`）
- ❌ 不是 PSRAM（关掉 PSRAM 同样不启动）
- ❌ 不是 app 本身（换成原厂 bootloader 就活了）
- ❌ 不是 console 配置（USB-Serial/JTAG 和 USB CDC 都试过）

**原因尚未定位**。可能是这块板的 flash 模式或 PSRAM 的 IO 配置与我们 `esp32-s3-devkitc-1` 的默认 bootloader 不符。

**在查清之前，S3 板只能用"只烧 app"的方式部署**：

```sh
pio run -d firmware -e dada-s3-pet          # 只构建
esptool --chip esp32s3 -p COM4 -b 921600 \
    write_flash 0x20000 .pio/build/dada-s3-pet/firmware.bin
```

这也意味着 **`pio run -t upload` 不能用**——它会连 bootloader 一起烧。

## 接线（ST7789 模块 → 通用排针）

| 屏 | 引脚 | 位置 |
| --- | --- | --- |
| VCC | 3V3 | 右列第 1 |
| GND | GND | 右列第 2 |
| DIN | GPIO9 | 右列第 6 |
| CLK | GPIO4 | 左列第 6 |
| CS | GPIO3 | 左列第 5 |
| DC | GPIO7 | 左列第 9 |
| RST | **GPIO6** | 左列第 8 |
| BL | GPIO5 | 左列第 7 |

板子自带的 18-pin LCD 座把 RST 接到 TCA9555 的 EXIO0，那个还没驱动，所以 RST 改接 GPIO6 拿硬复位。

## 未验证 / 未做

- 触摸：这块屏没有触控 IC，`input` 能力缺失。`pet` 因此改成 input 可选（只呼吸、不能摸头）。
- 串口日志：app 运行但 USB 串口无输出，原因未定位。不影响显示。
- PSRAM 实际可用量：`CONFIG_SPIRAM` 开着且不崩，但没有实测分配。
- 音频、网络：不在本次范围。
- 自己的 bootloader 为什么失败：未定位。

## 桌宠方向的结论

`pet` 能做到"角色在那儿、轻微起伏"，但**做不到"活"**。

原因是素材形态：单张平铺位图，所有变换（平移、垂直缩放都试过）只能作用于整幅画面，所以观感是"图片在动"。真正的桌宠需要分层素材（眼睛/头发/身体独立变换）或多帧精灵图。

这个方向暂停，等有合适素材再说。
