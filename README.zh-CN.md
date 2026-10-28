# dada · 搭搭

**连接应用与硬件，让开发者专注产品。**

dada 致力于连接应用开发者与硬件生态。
硬件厂商提交硬件描述与驱动，开发者构建硬件无关的应用，dada 负责匹配、构建、验证和交付。

[English](README.md) · **简体中文**

[开发指南](docs/development.md) · [架构路线](docs/architecture/incremental-roadmap.zh-CN.md) · [参与贡献](CONTRIBUTING.md)

> 当前阶段：ESP32-C3 首板验证。以下生态和架构图展示平台愿景，已实现能力见「当前进展」。

## 我可以贡献什么？

**当前支持一块 C3 一体板；“板 + 配件 + 组合”是已确认的扩展方向，尚未实现组合框架。**

| 你想做什么 | 发力点 | 入口 |
| --- | --- | --- |
| 开发应用 | 在 `apps/` 写业务，通过契约使用硬件，不写 GPIO 或板型分支 | [应用与平台边界](docs/architecture/developer-boundaries.zh-CN.md) |
| 适配开发板 | 在 `hardware/<vendor>/<board>/` 提供描述、绑定和实机证据；仍需商家资料 | [新板接入指南](docs/hardware/board-contribution.zh-CN.md) |
| 扩展配件 | 先确认真实模块与可用接口、跑通最小适配，再提取模块包和组合接线 | [配件与组合约定](docs/architecture/developer-boundaries.zh-CN.md#已确认方向板--配件--组合) |
| 改善工具、测试或文档 | 降低构建、刷机和贡献门槛 | [贡献规则](CONTRIBUTING.md) |

板包负责固定板载器件，配件包负责可复用外设，组合记录具体接线，应用声明能力需求。
同一个应用只在满足能力、资源与验证条件的硬件上复用；不是所有 C3 都能直接烧同一份固件。
必需能力缺失时拒绝构建；可选能力降级属于后续设计，音频接口也尚未实现。

现在保留已有一体板结构，不先建空目录或通用组合框架。先改善现有板体验，拿到真实配件后再验证扩展。
开发前请读[完整开发者指南](docs/architecture/developer-boundaries.zh-CN.md)；智能体请读 [AGENT.md](AGENT.md)（自动入口 [AGENTS.md](AGENTS.md)）。

## 平台愿景：应用与硬件自由组合

![dada 平台生态愿景：应用开发者与硬件厂商贡献应用和硬件包，平台负责匹配、构建、验证与交付，产品开发者和用户选择兼容组合；AI 和本地服务按需接入。](docs/assets/architecture/platform-ecosystem-vision.png)

应用开发者描述业务与能力需求；硬件厂商提供芯片、开发板和外设的描述、驱动及验证资料。
平台连接双方，让产品开发者与用户选择应用和兼容硬件组合，减少重复适配与工具切换。
应用可接不同 AI 服务、使用本地服务，也可完全离线运行；本地模型推理受设备资源限制。

## 全局架构：设备平台与交付平台

![dada 目标架构：开发者应用通过统一能力契约使用运行服务和硬件适配；主机交付平台负责包管理、能力及资源匹配、构建、验证和发布；底层可逐步扩展不同芯片、开发板和外设。](docs/assets/architecture/platform-target-architecture.png)

- **开发者应用**：业务、交互和能力需求，尽量复用同一份源码。
- **设备平台**：统一能力接口与契约、通用运行服务、芯片与板级适配、外设驱动。
- **底层硬件**：经过适配和验证的芯片、开发板、显示屏、输入、音频及传感模块组合。
- **主机与交付平台**：应用包与硬件包、能力和资源匹配、构建编排、验证报告、发布与更新。

应用复用以能力兼容、资源匹配和实机验证为基础；针对目标硬件构建固件。
运行服务按需裁剪，主机工具可在本地运行。ESP32 是起点，其他 MCU 与 Linux 板卡将逐步验证。
应用可选择云端 AI、本地服务或完全离线运行。

## 当前进展

- 首板：金逸晨 ESP32-C3 + 2.4 寸 ST7789 屏 + CST836U 触摸，工具链为 PlatformIO Core 6.2.0、Espressif32 平台 6.8.1 和 **ESP-IDF v5.3.0**。
- 已有显示、输入、单调时间、存储契约；提供 NVS 存储及板级清单配置生成。
- 七个可选择的原生 C 应用，全部只依赖能力契约：`touch-counter`（默认）、`bringup`、
  `focus-clock`、`snake`、`paint`、`whale`、`flappy`。各自的依赖与行为见 `apps/`。
- 已有实验性单应用生命周期、服务注入、应用侧共享画布、主机测试与本地交付 CLI。
- 显示、触摸、计数器、番茄钟和贪吃蛇均有对应实机记录，见[硬件记录](tests/hardware/records/)。

尚无通用运行时、应用 IR 编译器、自动硬件发现、包市场、AI 连接器和第二块受支持的板子。
先用原生 C 应用检验契约；[ADR 0005](docs/adr/0005-app-ir.md) 的声明式应用仍是提案。

## 快速开始

先安装 PlatformIO，再在仓库根目录执行：

```sh
python tools/dada.py check
python tools/dada.py ports
# PlatformIO 是可复现的固件基线：
pio run -d firmware -e dada-c3-touch-counter
pio run -d firmware -e dada-c3-touch-counter -t upload --upload-port YOUR_PORT
pio device monitor -d firmware -e dada-c3-touch-counter -p YOUR_PORT
# 切换到硬件诊断应用，使用独立构建目录：
# 兼容已有 ESP-IDF 环境时仍可使用旧的 dada.py 适配器：
python tools/dada.py build --app bringup
# 新 C3 板尚未验收时，显式构建诊断固件：
python tools/dada.py build --board YOUR_BOARD_ID --app bringup --allow-unverified
```

烧录会替换原固件，需要恢复时请保留备份。
环境安装见[开发指南](docs/development.md)，交互说明见[计数器说明](apps/touch-counter/README.zh-CN.md)。
接入 ESP32-C3 新板见[板卡贡献指南](docs/hardware/board-contribution.zh-CN.md)。

## 从首板到平台

**计数器闭环 → 离线番茄钟 → 第二块板 → 外部硬件贡献 → 应用生态**

先用真实应用提取公共服务，再用第二块板检验源码复用，最后开放可复现的第三方接入流程。
每一步的验收条件见[分阶段路线](docs/architecture/incremental-roadmap.zh-CN.md)。

| 想了解什么 | 入口 |
| --- | --- |
| 当前 C3 实现与交付流程 | [实现架构](docs/architecture/current-implementation.md) |
| 环境安装、构建、烧录与团队协作 | [开发指南](docs/development.md) |
| 接入并验收一块 ESP32-C3 板卡 | [板卡贡献指南](docs/hardware/board-contribution.zh-CN.md) |
| 能力契约、板级清单、应用形态 | [架构决策 ADR](docs/adr/README.md) |
| 命令行工具与清单校验 | [主机工具](tools/README.md) |
| 实机测试证据与待验收项目 | [硬件记录](tests/hardware/records/) |

## 测试与协作

```sh
cmake -S tests/host -B /tmp/dada-host
cmake --build /tmp/dada-host
ctest --test-dir /tmp/dada-host --output-on-failure
python3 -m unittest discover -s tests/host -p 'test_*.py'
```

见 [CONTRIBUTING.md](CONTRIBUTING.md)、[架构决策](docs/adr/README.md)与[主机工具](tools/README.md)。
CI 编译七个应用并运行主机及清单检查；实机验收按板卡和能力查看硬件记录。
不上传或复制没有合适许可证的商家示例；`dada verify` 尚未实现。

代码、提交信息用英文，文档逐步提供中英版本。

## 许可证

除另有标注外，dada 采用 [Apache License 2.0](LICENSE)。第三方组件保留各自许可证；
本许可证不授予 dada 商标使用权。授权范围与贡献约定见 [ADR 0004](docs/adr/0004-license.md)。
