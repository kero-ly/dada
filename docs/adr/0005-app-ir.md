# ADR 0005 · 应用形态：声明式 IR + 编译期生成

**Status:** Proposed · 2026-10-25

> **Summary (EN).** An app is expressed as declarative YAML — the **intermediate
> representation (IR)** — and compiled at build time into LVGL C code. The IR is
> side-effect-free and framework-neutral so that other front-ends (block editor, visual
> designer) can target it, and so a runtime interpreter can be added later. A `native:` escape
> hatch allows embedding C for things the IR cannot express.

## 背景

要让生态长得起来，应用必须能被**非嵌入式工程师**写出来。这里有一个常见的误判：
以为「语言简单」就能扩大社区，于是选 Lua / MicroPython。

真实规律是：**社区规模取决于「能否复用他已会的技能」**。

| 应用形态 | 目标人群 | 他已有的技能 | 量级 |
| --- | --- | --- | --- |
| C / C++ | 嵌入式工程师 | 寄存器、RTOS、构建系统 | 万 |
| **声明式 YAML** | ESPHome / Home Assistant 用户 | 写配置、读文档 | 十万 |
| 拖拽积木 | 学生、非程序员 | 无 | 百万 |
| Web / JS | 前端开发者 | HTML/CSS/JS | 千万 |

YAML 处在「成本最低且有足够表达力」的位置，是现实的起点。

## 决策

### 1. YAML 是 IR，不是唯一前端

```
Blockly 拖拽 ─┐
手写 YAML  ──┼→  dada IR (YAML) ─┬→ 编译期 codegen → LVGL C 代码
SquareLine ──┘                   └→ （将来）运行时解释器
```

**不赌编辑器**。今天没人写的可视化编辑器，明天可能成为主流；只要它产出同一份 IR，
后端一行不用改。

### 2. IR 必须无副作用、纯声明

允许：描述界面树、绑定事件、声明需要的能力、常量数据。
禁止：循环、任意表达式求值、I/O、时间等待。

理由有两个：

- 无副作用的 IR 才能被**静态校验**（能力断言、资源估算、循环引用检测）。
- 无副作用的 IR 才能被**编译期完全展开**，从而零运行时开销。

### 3. 编译期生成，暂不做解释器

| | 编译期 codegen | 运行时解释器 |
| --- | --- | --- |
| 应用分发 | 每个 app = 一次固件构建 | 一份固件 + 换 app 数据 |
| 内存开销 | 0 | IR 解析器 + 堆 |
| 应用商店 | 做不了 | 做得了 |
| ESP32-C3（320KB） | ✅ | ❌ 不现实 |
| ESP32-S3 + PSRAM | ✅ | 可行 |

**先 codegen**，但 IR 按「将来可解释」的约束设计。等 S3 板到位后再评估解释器，
那是第二块板要验证的假设之一。

### 4. 必须留逃生舱

```yaml
screen:
  - label: { text: "25:00", align: center, font: montserrat_48 }
  - native: |
      /* 少量 C，编译进固件；由宿主 app 提供符号 */
      lv_obj_set_style_text_color(label_0, lv_color_hex(0xE85D75), 0);
```

没有逃生舱的项目会在「稍微复杂一点」的需求面前被弃坑。代价是 IR 不再纯粹，
所以 `native:` 块必须：① 显式标注，② 在 CI 中统计用量（滥用即预警），
③ 不参与能力推断。

### 5. 应用声明自己需要的能力

```yaml
app:
  id: focus-clock
  requires: [display, input, time]
  optional: [storage, audio]
```

构建系统据此做编译期断言（见 ADR 0002）：目标板没有 `input` 就直接失败。

## 后果

- 同一个 app 的 YAML 可以在 C3 / S3 上编译出不同固件。
- 应用可以像代码一样 review、diff、fork。
- 代价：要写一个 YAML → C 的生成器（编译器），工作量不小，且要长期维护。
- 代价：IR 的表达力有天花板，复杂交互必须走 `native:`，这会削弱「纯配置」的纯粹性。

## 备选方案

| 方案 | 否决原因 |
| --- | --- |
| 只支持 C 应用 | 贡献者池子只有嵌入式工程师，社区规模上不去 |
| 直接上 MicroPython | C3 只有 320KB RAM，跑 LVGL + Python 解释器体验会很差；且与「离线、开机即用」矛盾 |
| 直接上 Web/JS 远程面板 | 需要 WiFi 常连，与「离线番茄钟」定位冲突；可作为**第二条产品线**，不是 v1 |
| 只做可视化拖拽编辑器 | 编辑器是产品不是平台，先做编辑器会把资源全耗在 UI 上 |

## 未决问题

1. IR 的 schema 版本与迁移策略（`app_version` + 自动迁移还是只报错）。
2. `native:` 的安全边界：是否允许访问文件系统/网络？
3. 是否需要一个「IR 转 LVGL」之外的第二个后端（例如远程帧缓冲），以证明 IR 真的中立。
4. 资源估算：编译期能否静态算出 Flash/RAM 占用并在超限时报错。
