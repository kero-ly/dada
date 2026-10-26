# 验收记录 · 2026-10-26 · 番茄钟首版（gate 3）

对应路线图 **Acceptance gate 3**（offline pomodoro）的第一版：注入时钟契约、
跑通开始/暂停/重置，计时边界由主机测试覆盖。

- 板卡：金逸晨 ESP32-C3 2.4 寸触摸开发板
- 工具链：ESP-IDF v5.4.2，`DADA_APP=focus-clock`
- 主持：`apps/focus-clock`（25 分钟，纯离线）

## 结果一：启动与显示 —— 通过

```
I (206) display_st7789: ST7789 240x320 ready
I (286) touch_cst836u: CST836U at 0x15, touches=0
focus: ready 25:00, tap to start
I (376) app_host: focus-clock started (240x320)
```

屏幕显示标题 `DADA FOCUS`、大号 `25:00`、进度条、状态词 `TAP`，底部 `RESET` 条。
用户确认显示正常、按开始后确实在倒计时。

## 结果二：交互与状态机 —— 通过

用户操作序列与串口记录一致：

| 操作 | 期望 | 串口 |
| --- | --- | --- |
| 点中部 | 开始 | `toggle -> state 1` (RUNNING) |
| 再点 | 暂停 | `toggle -> state 2` (PAUSED) |
| 再点 | 继续 | `toggle -> state 1` (RUNNING) |
| 点底部 | 重置 | `reset`，界面回 `25:00` / `TAP` |

用户确认状态词与数字按预期变化（`TAP → RUN → PAUSE → RUN → 25:00`）。

## 结果三：计时规则 —— 由主机测试覆盖

实机只验证了"能走、能停、能重置"；边界规则在 `tests/host/test_clock.c`：

- 暂停期间时间流逝不计入；
- 剩余时间正好为 0 才进入 `DONE`，超出不会重复计数；
- 时钟回退（`now < last`）不补时间，下一次正常采样继续；
- `DONE` 后再点会开始新一轮，并把完成次数加一；
- 重置保留已完成次数。

`ctest` 结果：`counter` 与 `clock` 两个用例全部通过。

## 时间契约

应用只拿到 `dada_time_t`，实现位于板级绑定：

- `monotonic_ms` = `esp_timer_get_time() / 1000`，来自 SoC 定时器；
- `wall_clock` 返回 `false`：这块板没有 RTC、也不联网，番茄钟不需要墙钟。

应用代码里没有 `esp_timer.h`，也没有引脚、板名或控制器型号。

## 未覆盖

- 完整走完一轮 25 分钟（只验证了短时间倒计时与状态切换）
- 长时间运行（>30 分钟）的稳定性与内存
- 断电后行为：预期从 `25:00` 冷启动，未实测
- 结束提示：目前只有状态词 `DONE` 变色，没有声音或闪烁

## 复现

```sh
python tools/dada.py build --app focus-clock
python tools/dada.py flash --app focus-clock --port YOUR_PORT
```

主机测试：

```sh
cmake -S tests/host -B tests/host/build
cmake --build tests/host/build
ctest --test-dir tests/host/build --output-on-failure
```
