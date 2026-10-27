# 接入一块 ESP32-C3 开发板

本指南帮助新贡献者把另一块 C3 开发板接入工程，先运行诊断应用，再声明已支持能力。
当前固件集成**只面向 ESP32-C3**，并且必须有板级绑定；仅新增 `board.json` 不会自动支持新板。

English: [board-contribution.md](board-contribution.md).

## 1. 确认具体硬件

记录厂商、产品名、PCB 修订号、SoC、Flash 容量、USB 接口与下载模式。
逐项确认屏幕、触摸及输入器件的控制器、总线、分辨率、方向、引脚、复位和背光极性。
优先引用原理图和数据手册；每项事实注明来源。未知或因批次变化的值先保留 `null`，实测后再填写。

没有明确兼容许可证的商家源码包不得上传。可参考接口和初始化事实，依据公开数据手册实现驱动。

## 2. 新增板级包

在 `hardware/<vendor>/<board>/` 下参考最接近的板级目录，再替换所有板卡信息。
清单 id 必须是 `<vendor>-<board>`；目录中需包含 `board.json`、`board/CMakeLists.txt` 和板级绑定。

尚未购入时用 `status: "planned"`，已购入但未验收时用 `"purchased"`，
两种状态均保持 `verified_capabilities` 为空。显示和触摸由硬件章节声明；板级绑定准备提供的
`time`、`storage` 等非结构能力写入 `declared_capabilities`。所有已知硬件字段标注 provenance；未知值保持 `null`。
运行 `tools/validate_manifests.py` 检查清单。

板级绑定将生成的清单头文件映射为具体组件配置。控制器和总线匹配时才复用现有驱动；
新增屏幕或触摸控制器需增加许可证明确的驱动组件，并尽量补充主机测试。应用中不得出现 GPIO。

## 3. 构建诊断固件

安装并激活 ESP-IDF v5.4.2，使用明确的板卡 id 和串口：

```sh
python tools/dada.py check --board <vendor>-<board> --app bringup --allow-unverified
python tools/dada.py build --board <vendor>-<board> --app bringup --allow-unverified
python tools/dada.py ports
python tools/dada.py flash --board <vendor>-<board> --app bringup \
  --allow-unverified --port YOUR_PORT
python tools/dada.py monitor --board <vendor>-<board> --app bringup \
  --allow-unverified --port YOUR_PORT
```

`--allow-unverified` 是显式的本地 bring-up 选项：清单仍须声明应用所需能力（显示/触摸章节或
`declared_capabilities`），板级绑定也必须存在。
它不会添加能力、标记板子为已支持或跳过编译。烧录会覆盖设备上的原固件。

当前 `bringup` 需要显示和触摸输入。3×3 网格用于逐格核对屏幕位置、触摸行列和坐标。
若新板只有其中一项，应新建或调整需求匹配的诊断应用；不要为了让构建通过而弱化应用清单。

## 4. 实机验收并声明支持

在真实开发板运行诊断应用，记录精确 PCB 修订、构建命令、烧录和下载模式、启动日志及能力结果。
检查屏幕方向、颜色、全区域刷新边界、触摸映射和断电冷启动；存储能力还要验证断电后数据保留。
按照 ADR 0002 区分厂商文档、原理图、实测和社区实测证据。

某项能力通过实机检查后，才加入 `verified_capabilities`。移除 `--allow-unverified`，
为声称兼容的应用分别运行正常匹配与构建。编译成功不等同于实机验收记录。
这不代表存在通用二进制、自动识别引脚或任意模块组合能力。

## 5. 运行检查并提交 PR

```sh
python tools/validate_manifests.py
python tools/gen_board_config.py hardware/<vendor>/<board>/board.json \
  -o /tmp/dada-board-config.h
cmake -S tests/host -B /tmp/dada-host
cmake --build /tmp/dada-host
ctest --test-dir /tmp/dada-host --output-on-failure
python3 -m unittest discover -s tests/host -p 'test_*.py'
```

在 `tests/hardware/records/` 增加简明实机记录，分别列出通过和未测试能力；附上资料来源和有许可证的驱动依赖。
需要一起识别开发板或驱动时，使用 **New board proposal** Issue 模板。
当前工程将 ESP32-C3 写死，尚无通用 SoC 选择；其他 ESP32 系列和 Linux 板卡需先单独扩展平台构建支持。
