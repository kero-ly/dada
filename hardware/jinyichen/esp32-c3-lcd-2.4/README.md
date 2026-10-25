# 金逸晨 ESP32-C3 2.4 寸触摸开发板

**状态：硬件已实测（2026-10-24），驱动尚未进入本仓库。**

> 本次验证使用的是 PlatformIO + Arduino 工具链，**不是** ESP-IDF v5.4.2 基线。
> 结论只覆盖硬件本身：板子、屏、触摸、USB 都正常。
> 在 ESP-IDF 下能否跑通显示与触摸，仍需单独验证，不能由本记录推断。

## 已核验事实

| 项目 | 结论 | 证据 |
| --- | --- | --- |
| SoC | ESP32-C3FH4，QFN32，rev v0.4，160MHz | esptool 连接信息 + 原理图 U3 |
| Flash | 4MB，XMC，片内 | esptool `flash-id`（Manufacturer 46 / Device 4016） |
| USB | 原生 USB-Serial/JTAG，**无桥接芯片**，`303A:1001` | 设备管理器 + esptool `USB mode: USB-Serial/JTAG` |
| 烧录 | 原生 USB 直烧，无需手动按 BOOT/RST | 实测 |
| 屏幕 | ST7789，240×320，四线 SPI | 原理图 J4 + 卖家 `User_Setup.h` |
| 触摸 | CST836U，I²C，地址 `0x15`（部分批次 0x5D） | I²C 扫描 + `CST836U数据手册V1.2.pdf` |
| 供电 | USB 5V → 1N5819 → VCC_USB → ME6211C33（3.3V，500mA） | 原理图 |

### 引脚

| 功能 | GPIO | 说明 |
| --- | --- | --- |
| LCD SCLK / MOSI / DC / RST | 6 / 7 / 10 / 8 | |
| LCD CS | — | **PCB 上直接接 GND**，固定选中 → 驱动按无片选实现 |
| LCD 背光 | 0 | 高电平点亮，经 AO3402 控制 LEDK |
| 触摸 SCL / SDA / INT / RST | 5 / 4 / 3 / 2 | INT 触摸时拉低 |
| 板载 LED | 1 | **低电平点亮** |
| BOOT 键 | 9 | 低有效，10K 上拉 |
| 排针（未焊） | — | 5V / GND / 3.3V / TX0(21) / RX0(20) / GND |

> **扩展能力硬约束**：屏幕占 0/6/7/8/10，触摸占 2/3/4/5，USB 占 18/19，BOOT 占 9，LED 占 1。
> 只剩排针上的 GPIO20 / GPIO21。这块板只能当“纯 HMI 终端”，不适合堆外设。

## 厂商资料与授权

卖家资料包含：`原理图.pdf`、`定义.xlsx`、结构图 DXF、ST7789 与 CST836U 数据手册，
以及 `最新版程序优化后.rar`——一份**可编译的完整 PlatformIO 工程**（Arduino 框架 +
TFT_eSPI 2.5.43 + LVGL 8.4.0）。

**授权核查结果：卖家自有工程内没有任何 LICENSE / COPYING / NOTICE 文件，文档中亦未声明许可。**
依赖库许可清晰：TFT_eSPI 为 BSD 系，LVGL 8.4 为 MIT，ESP-IDF / Arduino-ESP32 为 Apache-2.0。

因此：**卖家源码只能用于私有研究，不得复制进本仓库**（见 [ADR 0004](../../../docs/adr/0004-license.md)）。
允许的做法是阅读其引脚与初始化参数，按 ESP-IDF 组件接口重写。

> ⚠️ 同一网盘中还有一个 `ESP32-ISP屏接线图-库文件.zip`，内容其实是
> **ESP32-WROOM + SSD1351 OLED** 的通用接线图，与这块板无关，勿据此接线。

## 亲历的坑（写下来给后来人）

1. **附送的 USB-C 线只有电源没有数据**：插上后屏幕会亮（供电正常），但电脑没有任何 USB 设备。
   换一根能传数据的线后立即出现 `COM3`。这是最容易误判为“板子坏了”的问题。
2. **排针未焊**：6P 排针孔是裸通孔，接任何外设都要先焊。
3. **无音频**：原理图无扬声器、无麦克风、无音频器件，也没有多余引脚接 I²S 麦克风。
4. **SPI 时钟**：请求 60MHz 实际落到 40MHz（C3 SPI 整数分频），按 40MHz 规划带宽。
5. **全屏刷新瓶颈**：240×320×2B = 153KB，40MHz 下约 30ms，全屏重绘上限约 33fps。

## 待办

- [ ] 在 ESP-IDF v5.4.2 下复现显示与触摸
- [ ] 确认 ST7789 色序（RGB/BGR）与偏移（Arduino 下 rotation 0 正常）
- [ ] PCB 修订号：板上未标注，暂记 `null`
