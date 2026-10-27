# ADR 0002 · 板级清单：唯一真源与构建期生成

**Status:** Proposed · 2026-10-25

> **Summary (EN).** `hardware/<vendor>/<board>/board.json` is the single source of truth for a
> board. It is validated against a versioned JSON Schema in CI, and converted at **build time**
> into a generated C header (`dada_board_config.h`). The device never parses JSON at runtime.
> Every field carries provenance (seller listing / vendor schematic / measured / community
> report). Unknown values stay `null` — guessing is forbidden.

## 背景

现在 `board.json` 是「人工资料登记」，有三个问题：

1. **没人校验**：字段拼错、类型错、漏字段，CI 都发现不了。
2. **和代码没有连接**：清单里写着引脚，代码里另写一遍，两边会漂移。
3. **没有可信度标记**：`"controller": "ST7789"` 是卖家说的还是量出来的？看不出来。

## 决策

### 1. 清单是唯一真源，且区分「声明」与「实测」

```json
{
  "manifest_version": "0.1",
  "id": "jinyichen-esp32-c3-lcd-2.4",
  "status": "verified",
  "soc": "esp32c3",
  "flash_mb": 4,
  "usb": { "mode": "serial-jtag", "vid_pid": "303A:1001" },
  "display": {
    "controller": "ST7789",
    "width": 240, "height": 320,
    "bus": "spi",
    "pins": { "sclk": 6, "mosi": 7, "dc": 10, "rst": 8, "cs": null, "backlight": 0 },
    "notes": "CS 在 PCB 上直接接 GND，驱动按无 CS 实现"
  },
  "touch": {
    "controller": "CST836U", "bus": "i2c", "address": "0x15",
    "pins": { "scl": 5, "sda": 4, "int": 3, "rst": 2 }
  },
  "declared_capabilities": ["time", "storage"],
  "verified_capabilities": ["usb-serial-jtag", "display", "touch"],
  "provenance": {
    "flash_mb": "measured",
    "display.pins": "vendor-schematic",
    "touch.address": "measured",
    "display.controller": "vendor-schematic"
  },
  "source_url": "https://item.taobao.com/item.htm?id=…"
}
```

**`provenance` 的取值只有四种**，按可信度从低到高：

| 值 | 含义 | 能否进 `verified_capabilities` |
| --- | --- | --- |
| `seller-listing` | 商品页/客服口述 | 否 |
| `vendor-doc` | 卖家提供的规格书、数据手册 | 否 |
| `vendor-schematic` | 卖家提供的原理图/引脚表 | 否（可作实现依据） |
| `measured` | 本机实测，有验证报告 | **是** |

这条直接继承仓库已有的纪律：**未知就是 `null`，不填猜测值**。

### 2. JSON Schema 版本化，CI 校验

- Schema 放在 `contracts/schema/board.schema.json`，用 `manifest_version` 选择版本。
- `manifest_version` 只在**破坏性变更**时升，新增可选字段不升版本。
- CI 跑 `tools/validate-manifests`：字段类型、枚举、`null` 规则、id 与目录名一致、
  `verified_capabilities` ⊆ 实际拥有的能力、`provenance` 覆盖所有非 null 的叶子字段。

### 3. 构建期生成 C 头文件，设备端不解析 JSON

`tools/gen-board-config` 读清单 → 生成 `build/dada_board_config.h`：

```c
/* 自动生成，勿手改 */
#define DADA_BOARD_ID            "jinyichen-esp32-c3-lcd-2.4"
#define DADA_BOARD_FLASH_MB      4
#define DADA_BOARD_HAS_DISPLAY   1
#define DADA_BOARD_DISPLAY_W     240
#define DADA_BOARD_DISPLAY_H     320
#define DADA_BOARD_LCD_SCLK      6
#define DADA_BOARD_LCD_MOSI      7
#define DADA_BOARD_LCD_DC        10
#define DADA_BOARD_LCD_RST       8
#define DADA_BOARD_LCD_CS        (-1)          /* null → -1（无片选） */
#define DADA_BOARD_LCD_BL        0
#define DADA_BOARD_HAS_TOUCH     1
#define DADA_BOARD_TP_SCL        5
#define DADA_BOARD_TP_SDA        4
#define DADA_BOARD_TP_INT        3
#define DADA_BOARD_TP_RST        2
```

理由：320KB RAM 不该养 JSON 解析器；配置错误在编译期暴露，而不是现场黑屏。

### 4. 声明能力与验证能力分开

`display`、`touch` / `input` 由对应硬件章节声明；不能从硬件结构直接推断的能力
（如 `time`、`storage`、`wifi`）写入 `declared_capabilities`。只有经过实机验收的能力才
写入 `verified_capabilities`。缺少验证字段按空列表处理，不能推断为已验证。
本地诊断构建可显式允许“已声明但未验证”的能力；正常构建必须同时满足声明与验证。

### 5. 能力断言在构建期完成

应用声明它需要的能力，构建系统检查目标板是否满足：

```cmake
# 应用侧
dada_requires(CAP_TOUCH CAP_TIME)      # 声明依赖

# 构建系统内部（伪代码）
foreach(cap ${DADA_REQUIRED_CAPS})
    if(NOT DADA_BOARD_VERIFIED_${cap})
        message(FATAL_ERROR
            "board '${DADA_BOARD_ID}' does not declare verified capability '${cap}'")
    endif()
endforeach()
```

**「这块板没验证过触摸」应该在 `idf.py build` 时炸，而不是烧进去发现点不动。**

## 后果

- 引脚只写一遍，代码与文档不可能漂移。
- 换板 = 换一个清单 + 重新生成，`apps/` 一行不动。
- 代价：新增清单字段需要同步 schema + 生成器（一次性的小成本）。
- 代价：清单变更后必须重新编译，不能「改个 JSON 就生效」。**这是有意的**——
  嵌入式目标不该在运行时接受未经校验的硬件描述。

## 备选方案

| 方案 | 否决原因 |
| --- | --- |
| 运行时读 JSON | 涨 Flash/RAM、错误推迟到运行期、要信任外部输入 |
| 用 Kconfig 代替清单 | Kconfig 是「构建选项」，不是「硬件事实」，没有 provenance 与验证概念 |
| 直接把引脚写进代码 | 就是现状，文档与代码必然漂移 |

## 未决问题

1. **同一块板多个变体**（不同批次换屏）怎么表达？继承（`extends`）还是独立 id？
2. **清单里的二进制资源**（开机图、字体子集）放哪？清单不引用大文件，只引用 `components/` 里的符号。
3. 生成器用什么写？建议 Python（CI 里跑，贡献者不用装额外工具链）。
4. **分离式硬件怎么表达**（2026-10-25 由第二块板暴露）：
   ESP32-S3-DevKitC-1 本身没有屏和触摸，屏与触摸是独立模块。当前模型假设
   「一块板 = 板 + 屏 + 触摸」，在一体板上成立，在分离式上不成立。
   候选方案：① 板与模块分成两类清单，由 `compose.json` 组合；② 清单加 `modules: []` 字段；
   ③ 每种组合单独一份清单（会导致清单爆炸）。倾向 ①。
