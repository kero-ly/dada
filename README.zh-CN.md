# dada · 搭搭

连接应用与硬件的平台，从一块 ESP32-C3 触摸屏开发板开始。
应用依赖能力契约，板级绑定负责引脚、总线和驱动。

English: [README.md](README.md).

## 当前做到哪一步

- 首板：金逸晨 ESP32-C3 + 2.4 寸 ST7789 屏 + CST836U 触摸，工具链 **ESP-IDF v5.4.2**。
- 已有显示/输入契约、可复用驱动、板级清单及配置头文件生成。
- 两个可选择的原生 C 应用：`touch-counter` 触摸计数器（默认）、`bringup` 色带与触摸诊断。
- 已有实验性的单应用生命周期、服务注入、主机测试与本地交付 CLI。
- 色带颜色和触摸事件已获实机确认；计数器交互与断电重启单独记录在
  [硬件记录](tests/hardware/records/)中。

尚无通用运行时、应用 IR 编译器、自动硬件发现、包市场、AI 连接器和第二块受支持的板子。
先用原生 C 应用检验契约；[ADR 0005](docs/adr/0005-app-ir.md) 的声明式应用仍是提案。

## 快速开始

先激活已安装的 ESP-IDF v5.4.2 环境，再在仓库根目录执行：

```sh
python tools/dada.py check
python tools/dada.py build
python tools/dada.py ports
python tools/dada.py flash --port YOUR_PORT
python tools/dada.py monitor --port YOUR_PORT
# 切换到硬件诊断应用，使用独立构建目录：
python tools/dada.py build --app bringup
```

烧录会替换原固件，需要恢复时请保留备份。
环境安装见[开发指南](docs/development.md)，交互说明见[计数器说明](apps/touch-counter/README.zh-CN.md)。

## 三层如何落地

```text
开发者应用       apps/：业务逻辑、应用私有界面，不出现 GPIO 或 ESP-IDF API
中间平台         contracts/：能力契约和实验性应用生命周期
                 firmware/：选择板卡、注入服务、调度应用
                 hardware/ + components/：板级适配与可复用驱动
                 tools/：构建前检查、配置生成、调用 ESP-IDF 构建和烧录
底层硬件         真实的 ESP32-C3、屏幕、触摸与连线
```

设备运行时，应用调用注入的能力实现；电脑端工具负责准备与交付固件。
`runtime/` 等第二个应用出现实际共性后再提取公共服务；`connectors/` 预留可选的本地/云端服务适配。

[分阶段架构路线](docs/architecture/incremental-roadmap.zh-CN.md)：
**计数器 → 离线番茄钟 → 第二块板 → 外部硬件贡献 → 应用生态**。
每一步均有验收门槛。主机测试中的两种分辨率不等于两块实物板已验证；
第二块板验证前，不建设通用框架和包市场。

## 测试与协作

```sh
cmake -S tests/host -B /tmp/dada-host
cmake --build /tmp/dada-host
ctest --test-dir /tmp/dada-host --output-on-failure
python3 -m unittest discover -s tests/host -p 'test_*.py'
```

见 [CONTRIBUTING.md](CONTRIBUTING.md)、[架构决策](docs/adr/README.md)与[主机工具](tools/README.md)。
CI 编译两个应用并运行主机及清单检查，显示和触摸仍需实机验收记录。
不上传或复制没有合适许可证的商家示例；`dada verify` 尚未实现。

代码、提交信息用英文，文档逐步提供中英版本。
许可证尚待选定，见 [ADR 0004](docs/adr/0004-license.md)。
