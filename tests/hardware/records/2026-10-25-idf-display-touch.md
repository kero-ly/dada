# 硬件验证报告 · 2026-10-25 · ESP-IDF 下点亮

按 [ADR 0003](../../../docs/adr/0003-verification.md) 记录。这是首板在**仓库基线工具链**下的第一次验证。

- 日期 / 测试人：2026-10-25 / 用户 + AI 辅助
- 板卡：金逸晨 ESP32-C3 2.4 寸触摸开发板
- 工具链：**ESP-IDF v5.4.2**（仓库基线），riscv32-esp-elf 14.2.0，CMake 3.30.2，Ninja 1.12.1
- 主机：Windows 10 19043 + MSYS 外壳（IDF 已打两处本地补丁，见 notes）
- 连接方式：USB-C 数据线，COM3，原生 USB-Serial/JTAG

| 项目 | 结果 | 证据 |
| --- | --- | --- |
| 编译 | pass | 920 个目标，`dada.bin` 生成，无警告 |
| 烧录 | pass | `idf.py -p COM3 flash`，`Hash of data verified` |
| 启动日志 | pass | 见下方完整日志，无 panic、无重启循环 |
| 屏幕 | pass | 5 条色带（红绿蓝白黑）40ms 内画完 |
| 触摸 | pass | I²C 0x15 应答，`touch ok` |
| 控制台 | pass | 日志经原生 USB-Serial/JTAG 输出（UART0 未引出） |

## 启动日志（完整）

```
I (86) board: board jinyichen-esp32-c3-lcd-2.4 (flash 4 MB)
I (206) display_st7789: ST7789 240x320 ready
I (286) touch_cst836u: CST836U at 0x15, touches=0
I (286) app: display 240x320, touch ok
I (296) app: band red drawn
I (306) app: band green drawn
I (316) app: band blue drawn
I (326) app: band white drawn
I (336) app: band black drawn
I (336) app: test pattern complete
```

## 这次验证证明了什么

整条链路第一次完整跑通，且**应用代码里没有一个引脚号**：

```
apps → contracts（display/input 接口） → components（st7789 / cst836u 驱动）
     → hardware/<vendor>/<board>/board（板级绑定，读生成的配置头）
     → ESP-IDF → 硬件
```

- `board.json` 的引脚在构建期被 `tools/gen_board_config.py` 编译成 `dada_board_config.h`；
- 板级绑定用它初始化驱动，应用只拿到 `dada_display_t*` / `dada_input_t*`；
- 契约里没有 `esp_err_t`，驱动内部才有。

## notes（环境相关的坑，非常规做法）

1. **IDF 拒绝 MSYS 环境**：本机只能在 MSYS 外壳里跑命令，`tools/idf_tools.py` 与 `tools/idf.py`
   都会直接退出。**这是本机环境限制，不是仓库代码问题。** 为打通验证，对本地 IDF 副本打了两处补丁：
   - `tools/idf_tools.py`：MSYS 检查改为 warning（输出到 stderr，避免破坏 `export` 的解析）；
   - `tools/idf.py`：MSYS 分支原本只打印警告就结束，补上 `main()` 调用。
   两处都留有 `.orig` 备份。**不建议把这类补丁带进 CI**（CI 用官方容器）。
2. **临时目录权限**：本机 `%TEMP%` 目录存在但不可写，gcc 报 `Cannot create temporary file`。
   构建时把 `TEMP/TMP/TMPDIR` 指向 `D:\software\esp32\tmp` 即可。
3. **控制台必须改到 USB**：IDF 默认把日志送 UART0（本板排针未焊）。`sdkconfig.defaults`
   里设 `CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG=y` 才能看到日志。
4. **误判点**：验证过程中一度以为"烧完没输出"，实际是应用只在启动和触摸时打印，
   而监视进程占着 COM3 导致后续读取拿不到数据。

## 未覆盖

- Wi-Fi / BLE
- 触摸坐标精度与边缘（只验证了应答与状态变化）
- 长时间稳定性
- 第二块板（S3）的可迁移性
