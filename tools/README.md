# 主机工具

本地交付入口是 `dada.py`，清单检查和配置生成由另外两个脚本完成。
校验脚本依赖 `jsonschema`；交付 CLI 使用标准库，构建/烧录时需要激活 ESP-IDF v5.4.2，
端口枚举使用该环境附带的 pyserial。

```sh
pip install -r tools/requirements.txt
```

## dada.py：最小本地交付闭环

```sh
python tools/dada.py check --app touch-counter
python tools/dada.py build --app touch-counter
python tools/dada.py ports
python tools/dada.py flash --app touch-counter --port YOUR_PORT
python tools/dada.py monitor --app touch-counter --port YOUR_PORT
python tools/dada.py build --app bringup
# first diagnostic build on a new, declared but unverified board
python tools/dada.py build --board YOUR_BOARD_ID --app bringup --allow-unverified
```

默认板 `jinyichen-esp32-c3-lcd-2.4`，可用 `--board` 显式指定。
构建前检查板级绑定、原生 C 应用格式、所需能力与最小屏幕尺寸，然后调用 ESP-IDF；
不自动下载 SDK 或选择串口。正常模式要求所需能力已声明并经过硬件验证。
新板本地诊断时可显式使用 `--allow-unverified`：只能使用清单已声明的硬件能力，
CLI 和 CMake 都会携带该 opt-in；它不授予支持状态或绕过编译检查。
各选择的构建产物放在 `firmware/build/<board>--<app>/`。
`sdkconfig` 仍由当前 C3 集成工程共享；完整资源匹配和跨芯片配置隔离尚未实现。
`check --allow-unverified` 检查声明是否足以构建 bring-up，不等同于实机认证，也不替代完整板级 Schema 检查。

## validate_manifests.py

校验 `hardware/**/board.json`，两层检查：

1. **JSON Schema** —— `contracts/schema/board.schema.json`，字段类型、枚举、`null` 规则。
2. **语义规则**（schema 表达不了的）：

| 检查 | 规则 |
| --- | --- |
| id 与目录一致 | `hardware/<vendor>/<dir>/board.json` 的 id 必须是 `<vendor>-<dir>` |
| 状态与能力自洽 | `status: verified` 必须有能力；`planned`/`purchased` 必须为空 |
| 能力可信 | 声明 `display` 就必须有 display 段，`touch`/`input` 必须有 touch 段，`usb-serial-jtag` 必须是 `serial-jtag` |
| 证据覆盖 | 每个非 null 叶子字段都要有 `provenance` 条目（`notes` 免检） |
| GPIO 冲突 | 屏幕和触摸不能占用同一个引脚（几乎总是抄错） |

```sh
python tools/validate_manifests.py            # 扫描全部
python tools/validate_manifests.py --strict   # 警告也算失败
```

## gen_board_config.py

把清单编译成 C 头文件。设备端不解析 JSON，配置错误在编译期暴露。

```sh
# 单块板
python tools/gen_board_config.py hardware/jinyichen/esp32-c3-lcd-2.4/board.json \
    -o build/board-config/dada_board_config.h

# 全部板子，一块一个文件
python tools/gen_board_config.py --all -o build/board-config
```

生成的宏（节选）：

```c
#define DADA_BOARD_ID          "jinyichen-esp32-c3-lcd-2.4"
#define DADA_BOARD_FLASH_MB    4
#define DADA_BOARD_VERIFIED_DISPLAY 1     /* 编译期断言用 */
#define DADA_BOARD_LCD_SCLK    (6)
#define DADA_BOARD_LCD_CS      (-1)       /* null → -1，不会静默变成 GPIO0 */
#define DADA_BOARD_MANIFEST_SHA256 "6488863…"  /* 清单变了能发现 */
```

## 尚未实现

- `dada verify`：实机自检并产出报告（见 ADR 0003），需要先有驱动与自检固件。
- 完整跨板匹配、版本兼容、资源估算与包分发：在第二块板及外部贡献验证后推进。

## Add a board

当前 ESP32-C3 板卡的接入步骤、bring-up 命令、证据等级和限制见
[English](../docs/hardware/board-contribution.md) / [简体中文](../docs/hardware/board-contribution.zh-CN.md)。
新贡献者可先提交 [New board proposal](../.github/ISSUE_TEMPLATE/new_board.md) 收集板卡资料。
