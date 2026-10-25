# ADR 0001 · 分层与能力契约边界

**Status:** Proposed · 2026-10-25

> **Summary (EN).** dada is split into five layers (hardware → board support → reusable
> components → runtime → apps). The only stable interface between apps and boards is a
> **capability contract**, and those contracts are deliberately **sink-level** — a display
> contract accepts pixel buffers and controls a backlight; it does *not* expose drawing
> primitives. UI frameworks (LVGL) sit above the contract and are replaceable. Contracts must
> not include ESP-IDF or FreeRTOS headers.

## 背景

目标是一份应用可以跑在不同开发板上。这个目标在嵌入式领域被尝试过很多次，失败的原因
高度一致：**把「抽象」放在了错误的边界上**。

两种典型错误：

1. **抽象成绘图原语**：`dada_draw_rounded_rect()`、`dada_draw_text()`。
   后果是平台开始重新实现一个 UI 框架，然后永远追不上 LVGL 的更新，最终没人用。
2. **抽象成具体器件**：`st7789_init()`。
   后果是换一块屏就要改应用，等于没抽象。

正确的边界在两者之间：**「往哪块面板送什么像素」** 和 **「当前按没按、按在哪」**。

## 决策

### 1. 五层

```
L4  apps/          业务与界面（番茄钟…）        只依赖 contracts/ + 钦定 UI 运行时
L3  runtime/       生命周期、能力发现、资源仲裁  依赖 contracts/
L2  components/    可复用驱动（display/touch/…） 实现 contracts/
L1  hardware/      板级绑定：引脚、初始化序列、清单
L0  硬件
```

依赖方向单向向下，**禁止反向依赖**：`components/` 不得 include `apps/`，
`contracts/` 不得 include 任何实现。

### 2. 契约是 sink 级

```c
/* contracts/display.h —— 只回答「往哪块面板送什么像素」 */
typedef struct {
    uint16_t width;
    uint16_t height;
    uint8_t  color_depth;          /* 位/像素 */
    /* area 为逻辑坐标，像素为 RGB565 小端 */
    int (*flush)(void *ctx, const dada_area_t *area, const void *pixels);
    int (*set_backlight)(void *ctx, uint8_t percent);   /* 0..100 */
} dada_display_t;

/* contracts/input.h —— 只回答「当前按没按、按在哪」 */
typedef struct {
    bool (*poll)(void *ctx, dada_point_t *point, bool *pressed);
} dada_input_t;

/* contracts/time.h —— 单调时钟与墙钟分离 */
typedef struct {
    uint64_t (*monotonic_ms)(void *ctx);   /* 用于倒计时 */
    bool     (*wall_clock)(void *ctx, dada_datetime_t *out); /* 可能不可用 */
} dada_time_t;
```

**不放进契约的东西**：绘图原语、字体、控件、动画、颜色空间转换。
这些属于 UI 运行时（见 ADR 0005），换 UI 框架时不应该动契约。

### 3. 契约不得泄漏框架类型

`contracts/*.h` 中**禁止**出现：

- `#include <esp_*.h>`、`freertos/*.h`
- `esp_err_t`、`TickType_t`、`QueueHandle_t`

统一用 `int`（0 成功）或自定义 `dada_err_t`。理由：将来接 RP2040 / STM32 时，
契约头文件不需要改一行；应用代码也不用跟着改。

这是一条**便宜的保险**：现在多写一行 typedef，以后省一次大重构。

### 4. 板级知识只允许出现在 L1 与 L2 的适配层

`apps/` 与 `runtime/` 中不得出现 GPIO 编号、屏幕控制器型号、I²C 地址。
这些只能来自 `hardware/<vendor>/<board>/` 的清单（见 ADR 0002）。

## 后果

- 应用可以在不同板子间迁移，前提是目标板声明了相同能力。
- 换 UI 框架（LVGL → 其他）只影响 `components/ui_*` 与 `apps/`，不动契约。
- 代价：多一层间接调用（`flush` 函数指针），对 240×320 这个量级可忽略
  （一次 flush 传 4~20KB，函数指针开销在纳秒级）。
- 代价：契约一旦发布就要兼容，所以**第一版契约必须由真实应用倒推**，
  而不是照抄现有框架的 API。

## 备选方案

| 方案 | 否决原因 |
| --- | --- |
| 不做契约，用宏配置板子 | 就是现在的 Arduino 做法：全局宏、隐式包含、换板要重编且难排障 |
| 契约做到 widget 级 | 平台被迫实现 UI 框架，永远追不上上游，且应用被锁死在平台的 UI 能力里 |
| 直接采用 LVGL 的 `lv_disp_drv_t` 作为契约 | 等于把 LVGL 变成不可替换的依赖，违反「UI 可换」的目标 |

## 未决问题

1. **多屏**：一个板子接两块屏时，契约是数组还是「按 id 取」？（等真实需求，先不做）
2. **DMA / 双缓冲**：`flush` 是同步阻塞还是允许异步 + 完成回调？
   240×320 下同步够用；S3 + PSRAM 做双缓冲时需要异步接口，可能要在 v2 契约里引入。
3. **错误处理粒度**：`int` 返回值目前只有「成功/失败」，是否需要错误码枚举？
