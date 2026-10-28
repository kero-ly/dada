# 验收记录 · 2026-10-27 · Flappy Bird 移植进 dada

把作者早期的 Arduino + TFT_eSPI 版本移植到 dada 框架，验证"老应用能不能不改玩法地搬家"。

- 板卡：金逸晨 ESP32-C3 2.4 寸触摸开发板
- 工具链：ESP-IDF v5.4.2，`DADA_APP=flappy`
- 能力组合：**display + input + time + storage**

## 结果：可玩，手感与原版一致 —— 通过

```
flappy: ready (240x320), best 0
I (391) app_host: flappy started (240x320)
```

用户确认：能玩，手感接近原版。

## 移植做了什么

| 原版 | 移植版 |
| --- | --- |
| `game.cpp`（含 400 行 MPC 自动驾驶） | `flappy.c`，只保留规则，172 行 |
| `main.cpp` 直接调 TFT_eSPI | `render.c` 用 canvas 绘制，`flappy_app.c` 只管调度 |
| `millis()` 取时间 | `dada_time_t.monotonic_ms` |
| 全局 `tft` | `dada_display_t.flush` |
| Arduino `random()` | 自带 LCG |
| 最高分只在 RAM | 写进 storage 的 `flappy.best` |

物理参数（重力 1750、上冲 -435、管距 150、间隙 96、初速 112）全部沿用原版调好的值。

固定 20 ms 步长 + 累加器的做法保留：行为不随帧率变化。

## 没有移植的部分

**MPC 自动驾驶**（原版用来打 99 分）。它依赖对全局管道数组的完全访问，
属于"作弊模式"而非游戏玩法；要保留的话应该做成独立的调试开关，而不是混在主逻辑里。

## 一个性能取舍

原版用脏矩形重绘做到约 50 fps；移植版每帧整屏重绘（240×320 一次 flush）。
实测可玩，但没有测帧率数字——**这里没有量化证据**，只说"可玩"。

如果要优化，方向是把"只重绘变化的列"加回来，而不是换更快的驱动。

## 内存

`canvas` 需要 150 KB 连续堆。它成功了，因为 flappy 没有大的静态数组；
而 `whale` 在同类分配上失败过（19 KB 静态帧数组把大块切碎）。
这个差异值得记住：**能不能分配，取决于碎片，不只是总量**。

## 复现

```sh
python tools/dada.py build --app flappy
python tools/dada.py flash --app flappy --port YOUR_PORT
```

主机测试（7 个用例，不需要板子）：

```sh
cmake -S tests/host -B tests/host/build
cmake --build tests/host/build
ctest --test-dir tests/host/build --output-on-failure
```
