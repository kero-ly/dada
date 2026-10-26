# Focus clock

Hardware-independent native C application using `contracts/app.h`.
Chinese: [README.zh-CN.md](README.zh-CN.md).

- Starts at `25:00` in `TAP` state; tap anywhere above the bottom quarter to start,
  pause or resume. A session that ran out shows `DONE`; tapping again starts a new one.
- The bottom quarter is RESET: back to `25:00`, keeping the completed-session count.
- Offline only. Timing comes from `dada_time_t.monotonic_ms`; there is no RTC and no
  network, so the countdown survives nothing but its own process.
- Requires RGB565 display ≥160×200, pointer input and the `time` capability. No ESP-IDF,
  pins or board IDs appear in app code.
- `clock.c` holds the timing rules with no UI or hardware: pause does not accumulate,
  a session ends at exactly zero, overshoot does not double count, and a clock that
  steps backwards never credits time.
- Private renderer keeps a full-frame canvas and pushes only the changed blocks
  (time block, progress bar, state word).

```sh
python tools/dada.py build --app focus-clock
python tools/dada.py flash --app focus-clock --port YOUR_PORT
```

Host regression tests: `tests/host/test_clock.c`. Visual layout and long-session
behaviour require physical acceptance, not just a successful build.
