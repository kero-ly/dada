# PlatformIO + ESP-IDF 5.3.0 C3 验证

- 日期：2026-10-28
- 板卡：金逸晨 `jinyichen-esp32-c3-lcd-2.4`
- 芯片：ESP32-C3 revision v0.4，4 MB flash，MAC `80:53:e0:ae:ff:0c`
- 主机：macOS，串口 `/dev/cu.usbmodem1101`
- 工具链：PlatformIO Core 6.2.0 / `espressif32@6.8.1` / ESP-IDF 5.3.0
- PlatformIO board 定义：`esp32-c3-devkitm-1`（仅用于工具链，不替代 dada 板包）

## 结果

四个环境均完成构建；每次上传均通过 esptool 的写入和 hash 校验，并通过复位后的串口日志确认启动：

| 应用 | PlatformIO 环境 | 结果 | 启动日志 |
| --- | --- | --- | --- |
| `bringup` | `dada-c3-bringup` | pass | `bringup: 240x320 grid ready, tap each numbered cell` |
| `touch-counter` | `dada-c3-touch-counter` | pass | `touch-counter started (240x320)` |
| `focus-clock` | `dada-c3-focus-clock` | pass | `focus-clock started (240x320)`，`focus: ready 25:00, tap to start` |
| `snake` | `dada-c3-snake` | pass | `snake started (240x320)`，`snake: ready (12x14 cells), tap to start` |

每次启动日志还确认了 `ST7789 240x320 ready`、`CST836U at 0x15` 和 `ESP-IDF: 5.3.0`。

## 范围与未测项

这次验证证明 PlatformIO 基线可以在当前这块 C3 板上构建、烧录并启动四个现有应用；PlatformIO 的通用 `esp32-c3-devkitm-1` 定义没有替代 `hardware/jinyichen/esp32-c3-lcd-2.4/board.json` 的硬件事实。
屏幕画面、触摸坐标和各应用交互仍需使用者在设备前逐项肉眼确认；尚未验证第二块板、外接配件、组合冲突检查或网页刷机。
