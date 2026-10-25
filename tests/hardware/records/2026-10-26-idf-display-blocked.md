# 显示未点亮 · ESP-IDF 侧阻塞记录 · 2026-10-26

**结论：ESP-IDF 构建、烧录、日志、触摸都正常，但屏幕始终黑屏（背光正常）。**
**同一块板、同一组引脚，Arduino + TFT_eSPI 固件显示完全正常。**
这条记录用于第二天接手，不是"已解决"。

## 现象

| 项目 | Arduino（TFT_eSPI） | ESP-IDF（本仓库） |
| --- | --- | --- |
| 编译 / 烧录 | 通过 | 通过 |
| 日志 | 正常 | 正常（走原生 USB） |
| 触摸 | 正常 | 正常（I²C 0x15 应答） |
| 背光 | 亮 | 亮（屏幕发灰/发亮，不是死黑） |
| **画面** | **正常显示** | **纯黑** |

关键判断依据：面板上电后 GRAM 是未定义值，如果"显示已开但没有数据"，通常会看到噪点而不是纯黑。
持续纯黑说明 **DISPON 之后的画面没有建立**，即像素没有真正写进 GRAM，或命令根本没有到达面板。

## 已排除的假设

| # | 假设 | 验证方式 | 结果 |
| --- | --- | --- | --- |
| 1 | 引脚配置错误 | 运行时打印生成的宏：`sclk=6 mosi=7 dc=10 rst=8 cs=-1 bl=0` | 与 Arduino 一致，排除 |
| 2 | 背光没开 | 屏幕发亮；且强制 `gpio_set_level(0,1)` 后仍黑 | 背光正常，排除 |
| 3 | 初始化序列不全 | 移植厂商（TFT_eSPI）完整序列：电源/伽马/CASET/RASET/DISPON | 仍黑，排除 |
| 4 | SPI 时钟 / 模式 | 40MHz、mode 0（与 Arduino 相同）；也试过改 mode | 仍黑 |
| 5 | IOMUX 与 DC 冲突（GPIO10 是 C3 的 FSPICS0） | `spicommon_bus_initialize_io` 只在 `cs_io_num>=0` 时初始化 CS 脚；且已强制 `SPICOMMON_BUSFLAG_GPIO_PINS` 走 GPIO 矩阵 | 仍黑，**未能排除是别的原因** |
| 6 | esp_lcd 抽象层问题 | 完全绕开 esp_lcd，改用 `spi_master` + 手动 D/C 重写驱动 | 仍黑 |
| 7 | 硬件损坏 | 刷回 Arduino 固件 | **正常显示**，硬件排除 |

## 待验证的方向（明天从这里继续）

1. **bit-bang 测试做得不严谨**：昨天最后那版纯 GPIO 翻转程序，点亮全屏需要连续翻转约 7~10 秒且不让出 CPU，
   **很可能被任务看门狗复位**，而当时没有确认日志是否打印 `bb: white done`。这一步需要重做：分段翻转、
   每段 `vTaskDelay`，并确认日志。
2. **对比两份固件的 SPI 波形**：重点看 D/C 与 SCLK 的相对时序，以及 CS 是否被意外驱动。
   没有示波器时，可把某个空闲 GPIO 接到 MOSI 上做自环验证发送端。
3. **确认面板 RESET 行为**：ESP-IDF 下 RST 由 `gpio_config` 后手动拉低/拉高；Arduino 也走同一脚，
   但 TFT_eSPI 会在 init 里先做一次软件复位。已包含 SWRESET，仍黑。
4. **检查 IDF 的 GPIO 默认驱动强度 / 上拉**：Arduino 与 IDF 对同一个脚的默认配置可能不同
   （例如 DC 脚是否需要更强的驱动）。
5. **考虑把 Arduino 的 TFT_eSPI 作为参照实现**：它的 `ST7789_Init.h` + `TFT_eSPI.cpp` 是这份代码的
   逐行对照物，可直接比对每条命令与延时。

## 复现步骤

```bash
# ESP-IDF 侧（黑屏）
set IDF_TOOLS_PATH=D:\software\esp32\espressif
D:\software\esp32\esp-idf\export.bat
cd /d D:\software\esp32\dada\firmware
idf.py -p COM3 build flash

# 对照：Arduino 侧（正常）
cd /d D:\software\esp32\firmware\esp32c3-flappy
pio run -t upload --upload-port COM3
```

## 环境备注（不是仓库问题，但会绊人）

- 本机只能在 MSYS 外壳里执行命令，而 IDF 明确拒绝 MSYS，**对本地 IDF 副本打了两处补丁**
  （`tools/idf_tools.py`、`tools/idf.py`，均留 `.orig` 备份）。CI 使用官方容器，不受影响。
- `%TEMP%` 不可写，构建时需把 `TEMP/TMP/TMPDIR` 指向 `D:\software\esp32\tmp`。
- 调试期间的工作副本（原始 SPI 驱动、bit-bang 探针、带诊断日志的板级绑定）暂存在仓库外：
  `D:\software\esp32\_extract\wip\`。
