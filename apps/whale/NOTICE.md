# Third-party artwork notice

This directory contains two files derived from the same third-party image:

- `source.png` — the original illustration, re-encoded to strip metadata.
- `whale-f0.h` — a 160×120, 15-colour downscaled pixel version of it.
- `preview.png` — the two side by side, used in the README.

The illustration is fan art of the DeepSeek "whale girl" character.

- **Author and licence: unknown.** The source image was supplied by the
  repository owner; no licence statement accompanied it.
- **What was done to it:** the original is redistributed as-is (metadata
  stripped); the pixel version was resized to 160×120 cells, quantised to a fixed
  15-colour palette and packed 4 bits per cell.
- **Scope:** provided for technical demonstration of the `whale` app only.
- **Licence:** this asset is **not** covered by the repository's Apache-2.0
  licence. The surrounding code remains Apache-2.0.
- **Removal:** if you hold rights to the original and object to this use, open an
  issue and the file will be removed.

If you want a clean, unquestionably licensed build, delete `frames/whale-f0.h`
and the `#include` of it; the app then shows a placeholder until a frame is
stored, exactly as before.

Keep the same note in mind when replacing the frame with your own artwork: only
use images you have the right to redistribute.
