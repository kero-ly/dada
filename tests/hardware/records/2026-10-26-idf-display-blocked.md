# ESP-IDF 黑屏排查与 SPI 模式修复 · 2026-10-26

## 最新结论：黑屏与颜色已修复，触摸事件正常

Mac 实机单变量对照中，保持引脚、40MHz 时钟、esp_lcd 初始化及应用不变，
仅修改板级绑定的 `spi_mode`：

| 实验 | 烧录与日志 | 用户观察 |
| --- | --- | --- |
| mode 3 | 编译、烧录、校验通过；五条色带传输完成 | 出现色带 |
| 回退 mode 0 | 编译、烧录、校验通过；同样报告色带传输完成 | 再次黑屏 |
| 恢复 mode 3 | 编译、烧录、校验通过；复位后色带传输完成 | 出现黄、绿、紫、黑、白，方向以用户观察为准 |

因此本板黑屏定位为 SPI 模式不匹配，修复保留在板级绑定，不更改通用驱动。
黑屏解除后继续进行颜色单变量测试：

1. 商家 ST7789 初始化显式发送 `INVON`。只将本板 `invert_color` 改为 true，
   用户观察从“黄、绿、紫、黑、白”变为“蓝、红、绿、白、黑”，黑白已正确，彩色仍异常。
2. `contracts/display.h` 明确要求小端 RGB565，而 esp_lcd 面板配置默认大端。
   在通用 ST7789 驱动中显式配置 `LCD_RGB_DATA_ENDIAN_LITTLE`，不在应用层交换字节。
   编译、烧录、校验及复位后色带传输通过，用户确认颜色为“红、绿、蓝、白、黑”。

当前设备固件为 mode 3、反色开启、小端 RGB565，最新日志为
`/tmp/dada-rgb565-flash.log`、`/tmp/dada-rgb565-serial.log`。
已收到五次按下/松开触摸事件，
例如 `(47,110)`、`(79,52)`、`(76,136)`、`(79,214)`、`(98,87)`。
色带颜色已人工确认；触摸坐标与屏幕位置的对应、断电冷启动仍待验收；
初始化日志中的 `touch ok` 本身只证明芯片有应答，不能代替触摸坐标测试。
串口为 `/dev/cu.usbmodem1101`（303A:1001）；构建与串口日志暂存
`/tmp/dada-mode3-final-flash.log`、`/tmp/dada-mode3-final-serial.log`，
mode 0 对照日志为 `/tmp/dada-mode0-flash.log`、`/tmp/dada-mode0-serial.log`。

以下保留修复前的排查历史，历史中的“排除”结论不能代替本次对照证据。

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
| 4 | SPI 时钟 / 模式 | 旧记录称 40MHz、mode 0 与 Arduino 相同；但下方源码核对发现商家示例默认 mode 3。先前改 mode 的具体固件和日志未保留 | **尚未排除，需重新单变量测试** |
| 5 | IOMUX 与 DC 冲突（GPIO10 是 C3 的 FSPICS0） | `spicommon_bus_initialize_io` 只在 `cs_io_num>=0` 时初始化 CS 脚；且已强制 `SPICOMMON_BUSFLAG_GPIO_PINS` 走 GPIO 矩阵 | 仍黑，**未能排除是别的原因** |
| 6 | esp_lcd 抽象层问题 | 完全绕开 esp_lcd，改用 `spi_master` + 手动 D/C 重写驱动 | 仍黑 |
| 7 | 硬件损坏 | 刷回 Arduino 固件 | **正常显示**，硬件排除 |

## 待验证的方向（明天从这里继续）

优先执行文末的 mode 3 对照实验，再决定是否继续以下方向。

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

## Mac 接手续查：SPI 模式对照实验准备记录

- 核对商家 `最新版程序优化后.rar` 中的 TFT_eSPI 源码：
  `User_Setup_Select.h` 使用 `User_Setup.h`，后者启用 `ST7789_DRIVER`，
  不覆盖 `TFT_SPI_MODE`；`TFT_eSPI.h` 第 130–137 行因此选择 `SPI_MODE3`，
  `Processors/TFT_eSPI_ESP32_C3.c` 将其用于 SPI 设备配置。
  示例 `platformio.ini` 也未覆盖 SPI 模式。
- 本仓库板级绑定原来是 mode 0。实验只将该板的 `spi_mode` 改为 3，
  保留 40MHz、引脚、初始化序列及颜色配置。**这是待验证假设，不是已确认修复。**
- Mac 使用 `/Users/ly/esp/esp-idf-v5.4.2`，`idf.py -C firmware build`
  成功，应用镜像大小 `0x3b220`。SDK 显示 dirty，来自嵌套测试子模块未完整下载，
  未修改 SDK 驱动源码。
- 本次基线烧录失败：`/dev/cu.usbmodem101` 已不存在。再次枚举仅见
  蓝牙及系统调试串口；mode 3 固件尚未烧录，也没有新的屏幕实测结论。
- 设备接回后，先烧录 mode 3 并记录启动日志，再人工确认是否出现五条色带。
  若能显示，还需记录颜色、复位后显示、触摸表现；SPI 传输成功日志不能证明面板显示正常。
  如需确认因果，再回到 mode 0 重测原症状。

```bash
source /Users/ly/esp/esp-idf-v5.4.2/export.sh
python -m serial.tools.list_ports -v
# PORT 使用实际枚举出的设备路径
idf.py -C firmware -p PORT flash monitor
```

接手时已读取的原固件全量备份保留在 Git 仓库外：
`/Users/ly/esp/backups/dada-c3-2026-10-26/flash-before-tests.bin`（4MB）。
SHA-256：`7d33d0f56c6b28a482b9a4375b7e4ac9591968bc5944ee9a66324c4fa0c9e997`。
需要恢复时使用 ESP32-C3 的 `write_flash 0x0` 写入该备份。
