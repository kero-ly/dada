# Flappy Bird

使用 `contracts/app.h` 的、与硬件无关的原生 C 应用。
English: [README.md](README.md)。

从作者早期的 Arduino + TFT_eSPI 版本移植到 dada 框架，物理参数沿用原版调好的值，
手感一致。

- 点一下开始，点一下拍翅膀，撞死后停顿约 0.7 秒再点重开。
- 用固定 20 ms 步长 + 累加器推进模拟，因此行为不随帧率变化。
- 最高分存进 storage 的 `flappy.best`。
- 需要 RGB565 显示（≥200×260）、指针输入和 `time` 能力；
  storage 可选，没有它只是不记最高分。
- 渲染是每帧整屏重绘（用 `app_support` 的 canvas）。原版用脏矩形做到约 50 fps，
  这个移植版用一些性能余量换了更小的渲染代码。
- 未移植：原版用来打 99 分的 MPC 自动驾驶。

```sh
python tools/dada.py build --app flappy
python tools/dada.py flash --app flappy --port YOUR_PORT
```

主机回归测试见 `tests/host/test_flappy.c`。
