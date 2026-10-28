# Pixel paint

Hardware-independent native C application using `contracts/app.h`.
Chinese: [README.zh-CN.md](README.zh-CN.md).

- 48×64 grid of 15 colours; the whole panel is the canvas (no permanent toolbar).
- Tap the button in the bottom-right corner to slide a two-row palette over the
  bottom; picking a colour restores the rows it covered.
- Swipes are joined with Bresenham so fast strokes leave no gaps; a finger that
  leaves the canvas clips the line instead of aborting it.
- The bitmap is packed 4 bits per cell (1536 bytes) and saved to storage when the
  finger lifts, then restored on start.
- Requires RGB565 display ≥160×200 and pointer input; storage is optional and
  only affects persistence.
- A stored bitmap whose packed length does not match the current grid is rejected
  rather than drawn half-applied.

```sh
python tools/dada.py build --app paint
python tools/dada.py flash --app paint --port YOUR_PORT
```

Host regression tests: `tests/host/test_paint.c`.
