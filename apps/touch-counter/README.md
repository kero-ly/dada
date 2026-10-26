# Touch counter

Hardware-independent native C application using `contracts/app.h`.
Chinese: [README.zh-CN.md](README.zh-CN.md).

- Starts at `0000`; tap the upper three quarters to add one, saturating at `9999`.
- A held touch counts once; dragging into RESET does not reset. Release and tap again.
- The bottom quarter is RESET. No persistence: restarting returns to zero.
- Requires RGB565 display ≥160×160 and pointer input. No ESP-IDF, pins or board IDs in app code.
- Private renderer uses an eight-row stripe buffer (3,840 bytes at width 240), not a full framebuffer.
- Invalid samples do not rearm a press; render errors propagate to the firmware host for cleanup.

```sh
python tools/dada.py build --app touch-counter
python tools/dada.py flash --app touch-counter --port YOUR_PORT
```

Host regression tests: `tests/host/test_counter.c`. Visual orientation, touch correspondence,
and cold-start behaviour require physical acceptance, not just a successful build.
