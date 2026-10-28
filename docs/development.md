# 团队开发与首次启动

## 工具链

使用 **PlatformIO Core 6.2.0 + Espressif32 6.8.1 + ESP-IDF v5.3.0**。PlatformIO 会锁定框架和工具链版本，SDK 不放入项目仓库。
macOS Intel/Apple Silicon、Linux 和 Windows 均可使用相同的 `platformio.ini`。

以下是可选的 PlatformIO CLI 安装方式：

```sh
python3 -m venv .venv
. .venv/bin/activate                 # Windows: .venv\\Scripts\\activate
python -m pip install -r tools/requirements.txt
pio --version
```

依赖安装在仓库自己的 `.venv` 中，不需要把 PlatformIO 或 ESP-IDF 放进仓库。
PlatformIO 的下载缓存默认位于用户目录 `~/.platformio`，其中包含 Espressif32 平台、ESP-IDF 5.3.0 和编译工具链。

## 克隆与构建

```sh
git clone https://github.com/kero-ly/dada.git
cd dada
git switch -c codex/my-change
pio --version
python tools/dada.py check
pio run -d firmware -e dada-c3-touch-counter
```

PlatformIO 应报告 Core 6.2.0，构建输出应报告 Espressif32 6.8.1，固件启动日志应报告 ESP-IDF 5.3.0。构建不需要插板。
默认板为 `jinyichen-esp32-c3-lcd-2.4`，当前仅这一块板有实现。
产物在 `firmware/.pio/build/<environment>/`。PlatformIO 会按环境生成独立的构建目录和 `sdkconfig`，
避免不同应用复用错误的配置；这仍不代表已经完成跨芯片配置隔离。

## 连接、烧录与日志

1. 使用支持数据传输的 USB 线连接开发板。当前首板使用原生 USB Serial/JTAG。
2. 执行 `python tools/dada.py ports`，比较插拔前后的设备，明确选择端口。
   macOS 常见 `/dev/cu.usbmodem...`，Windows 常见 `COM...`；编号可能变化。
3. 保留原固件备份（如需恢复商家示例），将下方 `YOUR_PORT` 替换为实际端口。

```sh
pio run -d firmware -e dada-c3-touch-counter -t upload --upload-port YOUR_PORT
pio device monitor -d firmware -e dada-c3-touch-counter -p YOUR_PORT
```

烧录会覆盖原程序。用 `Ctrl+]` 退出监视。CLI 不自动挑选端口、不自动安装驱动。
若无法进入下载模式，按已确认的板卡 BOOT/RESET 操作，不猜测 GPIO 短接。

预期启动日志包含 `touch-counter started (240x320)`；屏幕显示 `0000`、`TAP +` 和 `RESET`。
上方四分之三区域点一下加一、长按仅一次，底部清零；断电重新上电归零。
操作验收必须实际观察，启动日志和编译成功不能替代它。

硬件问题先切换诊断应用：

```sh
pio run -d firmware -e dada-c3-bringup -t upload --upload-port YOUR_PORT
pio device monitor -d firmware -e dada-c3-bringup -p YOUR_PORT
```

预期屏幕出现 **3×3 网格**，每格中央有编号 1~9；触摸时在触点画出红色方块，
串口打印 `bringup: tap (x, y) -> cell N (row r col c)`。
这样可以逐条核对触摸坐标与屏幕位置是否对应，而不是只看"有没有坐标"。

验收后用 `--app touch-counter` 切回。

想把番茄钟一轮缩短到 2 分钟做端到端验证，直接传 CMake 变量
（默认 25 分钟，不影响应用代码）：

```sh
pio run -d firmware -e dada-c3-focus-clock
```
当前 PlatformIO 环境使用默认 25 分钟；短时验收专用的 CMake 参数环境尚未固化。
板级参数与历史证据见 `tests/hardware/records/`。

`DADA_APP` 由 PlatformIO 环境固定传入；每个应用有独立环境和构建目录，避免复用错误的选择。

## 本地检查

```sh
cmake -S tests/host -B /tmp/dada-host
cmake --build /tmp/dada-host
ctest --test-dir /tmp/dada-host --output-on-failure
python3 -m unittest discover -s tests/host -p 'test_*.py'
python -m pip install -r tools/requirements.txt
python tools/validate_manifests.py
python tools/gen_board_config.py --all -o /tmp/dada-board-config
```

主机测试不需要 ESP-IDF，但需要一个本机 C 编译器（Windows 上装 MinGW-w64 或 LLVM，
macOS 用 Xcode 命令行工具，Linux 用 gcc）；构建目录随意，`/tmp` 只是示例。

主机测试不需要 ESP-IDF。清单校验需要 `jsonschema`；构建、烧录、端口枚举由 PlatformIO 管理。
CI 编译七个应用，运行主机及板级清单检查；实机显示/触摸/冷启动另附验证记录。
不要提交 `sdkconfig`、构建产物、密钥或无合适许可证的商家源码。

## 协作边界

- 应用成员改 `apps/`，仅使用契约；硬件成员改 `hardware/` 与 `components/`；
  修改公共契约需要同时检查两个应用和主机测试。
- 以短期分支和 PR 协作。推送后查看实际 Actions 结果，本地通过不代表远端 CI 已通过。
- 需求、实现和待验收项目分开记录；现阶段 `dada verify` 尚未实现。
- 后续提取服务与第二块板的验收门槛见[增量架构路线](architecture/incremental-roadmap.zh-CN.md)。
