# Pixel portrait

Hardware-independent native C application using `contracts/app.h`.
Chinese: [README.zh-CN.md](README.zh-CN.md).

![source artwork and what the panel shows](frames/preview.png)

Displays one 160×120 frame of 15 colours, rotated onto a portrait panel so the
picture is upright when the board is held sideways.

One frame ships in `frames/whale-f0.h`, so a fresh clone builds and displays
without any setup step. See [NOTICE.md](NOTICE.md) for the artwork's provenance
and licensing: it is third-party fan art and **not** Apache-2.0.

- Frame format: 160×120 cells, 4 bits per cell (9600 bytes), same palette as
  `paint`.
- Resolution order: a frame stored under `whale.f0` in NVS wins; otherwise the
  built-in frame is used.
- To use your own image:

  ```sh
  python tools/image_to_frame.py my-art.png apps/whale/frames/whale-f0.h
  ```

  That rewrites the built-in frame. A frame whose length does not match the grid
  is rejected rather than drawn.
- Rendering walks eight-row stripes (3.8 KB) instead of allocating a full
  240×320 framebuffer (150 KB), because the allocator cannot supply a contiguous
  block of that size once other data is resident.
- A blink animation is **not** implemented. Three-frame and drawn-lid attempts
  all read as a pale rectangle over the hair; a believable blink needs
  hand-drawn closed-eye frames.

```sh
python tools/dada.py build --app whale
python tools/dada.py flash --app whale --port YOUR_PORT
```
