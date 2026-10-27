# dada 智能体开发指南

## 开始前

先读 README.zh-CN.md、docs/architecture/developer-boundaries.zh-CN.md、CONTRIBUTING.md 和 docs/development.md，再读任务相关 ADR、清单与代码。历史设计的 Proposed 状态不代表已实现；以当前代码、测试和明确的用户约定核实事实。
检查 git status 与已有 diff，保留用户及其他协作者未提交的改动，不擅自重置、覆盖或混入无关修改。

## 当前边界

- 当前只支持金逸晨 ESP32-C3 + 2.4 寸 ST7789 + CST836U 触摸组合，使用 ESP-IDF v5.4.2。S3 清单是规划资料。
- 当前为静态单板、单应用构建，已有 touch-counter、bringup、focus-clock、snake。
- 板 + 配件 + 组合是已确认方向。尚无模块/组合解析器、音频契约、完整 optional 能力、网页刷机或自动接线识别。
- 现在没有新增硬件，不为愿景建立空目录或提前实现通用组合框架。保持现有板包路径；拿到真实模块后先最小适配、验证，再提取复用结构。用户明确提出新任务时据其范围更新设计。

## 代码分层

- apps/：业务、交互、能力需求；不出现 GPIO、板型判断和具体驱动初始化。
- contracts/：应用可使用的接口；变更时核对全部应用和绑定。
- hardware/<vendor>/<board>/：板载硬件事实、引脚和板级绑定。
- components/：可复用驱动及契约适配；不写特定主板 GPIO。
- firmware/：静态装配与生命周期；tools/：主机检查、生成、构建和交付。
- runtime/ 是预留方向，不要声称已有完整通用运行服务。
- 配件模块描述逻辑信号；组合描述实例与具体接线；这些是未来边界，不是当前可调用接口。

## 硬件事实与证据

不得猜测引脚、控制器、可用接口、电压和许可证。引用资料，未知事实保持未知/null。当前板 GPIO 4/5/6 已用于触摸/显示；不能照抄讨论中的示意接线。
未被清单占用不代表引脚已引出或可用。检查板载设备、Flash/USB、启动引脚、总线共享条件与供电要求；无源扬声器通常需功放。
清单不替代驱动，构建不替代实测，单模块验证不替代组合验证。verified_capabilities 只能包含有真实证据的通过项。
--allow-unverified 是显式诊断入口，不补全缺失实现，不代表已支持。当前检查不覆盖全部电气和资源风险，报告时说明范围。
记录精确板型/修订、源码版本或工作区状态、命令、观察结果、未测项。不得伪造实测、回填未经证明的历史摘要或把主机测试称为硬件验收。

## 验证与交付

按实际改动选择验证，不要求文档修改重复全量编译：

```sh
# 文档和所有修改的基础检查
git diff --check
# 主机测试
cmake -S tests/host -B /tmp/dada-host
cmake --build /tmp/dada-host
ctest --test-dir /tmp/dada-host --output-on-failure
python3 -m unittest discover -s tests/host -p 'test_*.py'
# 清单变更：在独立环境安装 tools/requirements.txt 后执行
python tools/validate_manifests.py
# 固件变更：激活 ESP-IDF v5.4.2，按受影响应用执行
python tools/dada.py check --app touch-counter
python tools/dada.py build --app touch-counter
```

公共接口、板级绑定或固件入口变化时覆盖所有受影响应用。固件构建不要并发写共享 sdkconfig；需要并行时先隔离配置。需要实机确认的交互、颜色、声音、接线及断电恢复，单列待验收事项。
刷机会覆盖当前设备固件，按用户授权范围执行。未获任务授权不自动推送、发布或上传商家资料。
报告变更、实际验证结果、剩余限制及提交/推送状态。

## 文档与来源

面向开发者的入口文档保持中英文一致，区分 implemented / planned / unverified。不要把示意命令或清单写成当前可执行功能。
代码和提交信息使用英文，沿用仓库风格。仓库已采用 Apache-2.0（见 LICENSE 和 ADR 0004）；第三方材料保留原许可，不擅自重新授权。
商家示例可用于核对硬件事实；未经明确许可不得复制源码或上传压缩包。保留工作区商家文件，不加入提交。禁止提交生成产物、SDK、个人凭据和密钥。
