# 主机工具

目前只有两个脚本，都是纯 Python（仅依赖 `jsonschema`），在 CI 里跑，本地也能跑。

```sh
pip install -r tools/requirements.txt
```

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
- 交付 CLI / 跨板构建工具：等第二块板接入后再抽取。
