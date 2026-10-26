# 团队开发与首次启动

## 工具链

使用 **ESP-IDF v5.4.2** 及配套 Python、RISC-V 工具链。macOS Intel/Apple Silicon 均可；
Linux/Windows 依照该版本官方指南安装：https://docs.espressif.com/projects/esp-idf/en/v5.4.2/esp32c3/get-started/index.html 。
SDK 不放入项目仓库；已有其他版本时单独安装，不直接切换团队环境。

以下仅在目标目录不存在时安装：

```sh
mkdir -p ~/esp
git clone --branch v5.4.2 --recursive https://github.com/espressif/esp-idf.git ~/esp/esp-idf-v5.4.2
cd ~/esp/esp-idf-v5.4.2
./install.sh esp32c3
. ./export.sh
```

每个新终端运行 `. ~/esp/esp-idf-v5.4.2/export.sh`；Windows 使用对应的 ESP-IDF 终端或 `export.ps1`。

## 克隆与构建

```sh
git clone https://github.com/kero-ly/dada.git
cd dada
git switch -c codex/my-change
idf.py --version
python tools/dada.py check
python tools/dada.py build --app touch-counter
```

版本应为 v5.4.2。构建不需要插板。默认板为 `jinyichen-esp32-c3-lcd-2.4`，当前仅这一块板有实现。
产物在 `firmware/build/<board>--<app>/`。首次构建自动选择清单中的 esp32c3 目标，
无需先手工运行会重置生成配置的 `set-target`。
本地 `firmware/sdkconfig` 仍由同一 C3 工程共享，不能据此宣称已经支持跨芯片隔离。

## 连接、烧录与日志

1. 使用支持数据传输的 USB 线连接开发板。当前首板使用原生 USB Serial/JTAG。
2. 执行 `python tools/dada.py ports`，比较插拔前后的设备，明确选择端口。
   macOS 常见 `/dev/cu.usbmodem...`，Windows 常见 `COM...`；编号可能变化。
3. 保留原固件备份（如需恢复商家示例），将下方 `YOUR_PORT` 替换为实际端口。

```sh
python tools/dada.py flash --app touch-counter --port YOUR_PORT
python tools/dada.py monitor --app touch-counter --port YOUR_PORT
```

烧录会覆盖原程序。用 `Ctrl+]` 退出监视。CLI 不自动挑选端口、不自动安装驱动。
若无法进入下载模式，按已确认的板卡 BOOT/RESET 操作，不猜测 GPIO 短接。

预期启动日志包含 `touch-counter started (240x320)`；屏幕显示 `0000`、`TAP +` 和 `RESET`。
上方四分之三区域点一下加一、长按仅一次，底部清零；断电重新上电归零。
操作验收必须实际观察，启动日志和编译成功不能替代它。

硬件问题先切换诊断应用：

```sh
python tools/dada.py build --app bringup
python tools/dada.py flash --app bringup --port YOUR_PORT
python tools/dada.py monitor --app bringup --port YOUR_PORT
```

预期五条色带红、绿、蓝、白、黑（按显示坐标方向），触摸按下/松开打印日志。
验收后用 `--app touch-counter` 切回。板级参数与历史证据见 `tests/hardware/records/`。

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

主机测试不需要 ESP-IDF。清单校验需要 `jsonschema`；构建、烧录、端口枚举使用激活后的 IDF 环境。
CI 分别编译两个应用，运行主机和板级清单检查；实机显示/触摸/冷启动另附验证记录。
不要提交 `sdkconfig`、构建产物、密钥或无合适许可证的商家源码。

## 协作边界

- 应用成员改 `apps/`，仅使用契约；硬件成员改 `hardware/` 与 `components/`；
  修改公共契约需要同时检查两个应用和主机测试。
- 以短期分支和 PR 协作。推送后查看实际 Actions 结果，本地通过不代表远端 CI 已通过。
- 需求、实现和待验收项目分开记录；现阶段 `dada verify` 尚未实现。
- 后续提取服务与第二块板的验收门槛见[增量架构路线](architecture/incremental-roadmap.zh-CN.md)。
