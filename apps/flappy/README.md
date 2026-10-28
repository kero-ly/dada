# Flappy Bird

Hardware-independent native C application using `contracts/app.h`.
Chinese: [README.zh-CN.md](README.zh-CN.md).

Ported from the author's earlier Arduino + TFT_eSPI build. The physics
constants are the ones tuned there, so it plays the same.

- Tap to start, tap to flap, tap again after the death pause to retry.
- Simulation runs on a fixed 20 ms step with an accumulator, so behaviour does
  not change with frame rate.
- The best score persists in storage under `flappy.best`.
- Requires RGB565 display ≥200×260, pointer input and the `time` capability.
  Storage is optional: without it the best score simply is not kept.
- Rendering is a full repaint per frame (`canvas` in `app_support`). The original
  build used dirty rectangles for ~50 fps; this port trades some headroom for a
  much smaller renderer.
- Not ported: the MPC autopilot that the original build used to reach 99 points.

```sh
python tools/dada.py build --app flappy
python tools/dada.py flash --app flappy --port YOUR_PORT
```

Host regression tests: `tests/host/test_flappy.c`.
