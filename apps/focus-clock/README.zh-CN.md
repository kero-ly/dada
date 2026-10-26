# 离线触摸番茄钟

使用 `contracts/app.h` 的、与硬件无关的原生 C 应用。
English: [README.md](README.md)。

- 启动显示 `25:00`、状态 `TAP`；点屏幕下方 1/4 以上的任意位置开始 / 暂停 / 继续。
  计时结束显示 `DONE`，再点一次开始新的一轮。
- 底部 1/4 是重置：回到 `25:00`，**保留已完成次数**。
- 纯离线。时间来自 `dada_time_t.monotonic_ms`，没有 RTC、没有网络，
  因此断电后不保留任何状态。
- 需要 RGB565 显示（≥160×200）、指针输入和 `time` 能力。
  应用代码里没有 ESP-IDF、引脚号或板名。
- `clock.c` 只放计时规则，不含 UI 和硬件：暂停期间不计时、正好归零才算结束、
  超出不重复计数、时钟回退不补时间。
- 私有渲染器保留整帧画布，只推送变化的三块（时间、进度条、状态词）。

```sh
python tools/dada.py build --app focus-clock
python tools/dada.py flash --app focus-clock --port YOUR_PORT
```

主机回归测试见 `tests/host/test_clock.c`。界面布局与长时间运行仍需实机验收，
编译通过不能代替观察。
