# Desktop pet

Hardware-independent native C application using `contracts/app.h`.
Chinese: [README.zh-CN.md](README.zh-CN.md).

Keeps a character alive on the panel: it breathes, sinks while the screen is
held, and shifts sideways now and then. Everything is driven by the `time`
contract, so it works on boards with no touch panel at all.

- Breathing: a 6-pixel triangle wave over 3 s, applied while scanning out.
- Press: the portrait sinks 6 pixels and eases back when released.
- Fidget: every 8–20 s a 1-pixel sideways nudge, then back.
- Frame: `pet.frame` in storage if present, otherwise the shared built-in
  portrait in `apps/app_support/portrait/`.
- Requires RGB565 display ≥200×200 and `time`. Input and storage are optional;
  without input the pet simply never reacts to being touched.

## Why the motion is an offset, not extra frames

At 160×120 with 15 colours, one extra pose costs 9.6 KB. Breathing as a pixel
offset costs nothing and reads correctly, so the frame budget stays available for
expressions instead.

Note that the portrait is rotated onto a portrait panel: the vertical motion the
user sees is an offset in *logical y*. Getting that axis wrong makes the pet
slide sideways instead of breathing.

```sh
pio run -d firmware -e dada-c3-pet
pio run -d firmware -e dada-s3-pet      # Waveshare S3 board, no touch
```
