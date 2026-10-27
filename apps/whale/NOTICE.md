# Third-party artwork notice

`frames/whale-f0.h` is a 160×120, 15-colour downscaled pixel version of a
fan-made illustration of the DeepSeek "whale girl" character.

- **Author and licence: unknown.** The source image was supplied by the
  repository owner; no licence statement accompanied it.
- **What was done to it:** resized to 160×120 cells and quantised to a fixed
  15-colour palette, then packed 4 bits per cell. It is a derivative work, not a
  copy of the original file.
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
