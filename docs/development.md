# 团队开发与首次启动

## 工具链

- ESP-IDF：**v5.4.2**，使用此版本自带的 Python 环境与 RISC-V 工具链。
- macOS（Intel / Apple Silicon）或 Linux；Git、Python 3、CMake、Ninja 的具体依赖以对应版本官方安装指南为准。
- 官方指南：https://docs.espressif.com/projects/esp-idf/en/v5.4.2/esp32c3/get-started/index.html

当前只是初始化工程，本次未在开发机安装 SDK。可按官方指南安装到 `~/esp/esp-idf`，或使用已有安装；不要在仓库中保存 SDK。

```sh
mkdir -p ~/esp
git clone --branch v5.4.2 --recursive https://github.com/espressif/esp-idf.git ~/esp/esp-idf
cd ~/esp/esp-idf
./install.sh esp32c3
. ./export.sh
```

上述 clone 仅适用于目录不存在时。每个新终端构建前运行 `. ~/esp/esp-idf/export.sh`。已有其他版本的 SDK 可独立保留，不在原目录直接切换团队工具链。

## 构建

在本仓库根目录：

```sh
idf.py --version
idf.py -C firmware set-target esp32c3
idf.py -C firmware build
```

版本应为 v5.4.2。`set-target` 会重置生成配置，只需首次配置或切换目标时执行。产物位于 `firmware/build/`。

## 烧录与串口

先拿到卖家固件或按官方 esptool 文档备份原固件，再烧录：烧录会覆盖板子原有程序。这里不包含恢复厂商 demo 的固件。

1. 用支持数据传输的 USB 线连接 Mac；确认厂商的 USB 接口用途。
2. 插拔前后比较 `ls /dev/cu.*`，选择新增的设备端口。若为桥接芯片且未识别，确认型号后安装官方 macOS 驱动。
3. 在以下命令中替换 `YOUR_PORT`：

```sh
idf.py -C firmware -p YOUR_PORT flash monitor
```

使用 `Ctrl+]` 退出监视。若无法进入下载模式，按卖家文档操作 BOOT/RESET，不按未经确认的 GPIO 短接。
预期日志包含 `C3 bootstrap ready`；若无日志，先核对 Console output channel 与 USB/串口接线。屏幕不变化是当前骨架的预期行为。

## GitHub 与团队协作

项目仓库：https://github.com/kero-ly/dada 。团队成员克隆后创建功能分支：

```sh
git clone https://github.com/kero-ly/dada.git
cd dada
git switch -c feat/c3-display
```

完成改动后提交分支并创建 PR。推送后查看 Actions 的 C3 firmware 工作流；工程初始化不代表远端 CI 已通过。
建议为 main 配置 PR 审查与必需 CI 检查，成员用短期分支开发。此仓库没有自动配置远端权限。

CI 只验证编译。显示、触摸、USB、电源和内存稳定性必须有实机记录。CI 产物仅是串口 smoke 固件，不是可用番茄钟。
