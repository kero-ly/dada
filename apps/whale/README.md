# Pixel portrait

Hardware-independent native C application using `contracts/app.h`.
Chinese: [README.zh-CN.md](README.zh-CN.md).

Displays one stored 160×120 frame of 15 colours, rotated onto a portrait panel
so the picture is upright when the board is held sideways.

**No artwork ships with this repository.** The frames are third-party fan art;
the app therefore reads them from storage and draws a flat placeholder when they
are absent. This keeps the CI build meaningful and the repository free of images
it has no right to redistribute.

- Frame format: 160×120 cells, 4 bits per cell (9600 bytes), same palette as
  `paint`.
- Stored under `whale.f0` in the NVS namespace `dada`.
- To supply your own: draw something in the `paint` app, or generate a packed
  frame with a script, then write it to `whale.f0` once. A frame whose length does
  not match the grid is rejected.
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
