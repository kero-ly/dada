#!/usr/bin/env python3
"""Draw an original chibi whale-girl desktop pet and emit packed frames.

The character is built from primitives rather than traced from any artwork, so
each expression is a real drawing (different eyelid arc, mouth, pupil offset)
instead of a shifted copy of one image. Output matches the 15-colour, 4 bits per
cell format the apps use.

    python tools/art/make_pet_frames.py --preview out.png
    python tools/art/make_pet_frames.py --header apps/app_support/portrait/pet_frames.h
"""
from __future__ import annotations

import argparse
from pathlib import Path

W, H = 160, 120
SS = 4  # supersampling factor

PALETTE = [
    (0xFF, 0xFF, 0xFF), (0xF1, 0xE8, 0xE6), (0xC9, 0xCB, 0xD6), (0x9C, 0xA5, 0xBF),
    (0x7F, 0x88, 0xA7), (0x70, 0x8B, 0xC3), (0x5C, 0x74, 0xB0), (0x58, 0x6C, 0xA1),
    (0x4D, 0x61, 0x96), (0x44, 0x57, 0x8C), (0x38, 0x4C, 0x85), (0x40, 0x4A, 0x73),
    (0x2B, 0x38, 0x67), (0x14, 0x1A, 0x33), (0xF0, 0xB8, 0xC8),
]
SKIN = (0xF7, 0xE8, 0xE2)
HAIR = (0x45, 0x5A, 0xA0)
HAIR_DK = (0x2C, 0x38, 0x6C)
HAIR_LT = (0x6E, 0x8A, 0xC8)
EYE = (0x1B, 0x22, 0x3E)
IRIS = (0x4C, 0x7F, 0xD0)
CHEEK = (0xF2, 0xA8, 0xBC)
BOW = (0x53, 0x86, 0xC4)
WHITE = (0xFF, 0xFF, 0xFF)


def px(v):
    return int(v * SS)


def draw(name: str, expression: str, pupil: str = "centre"):
    from PIL import Image, ImageDraw

    img = Image.new("RGB", (W * SS, H * SS), (0, 0, 0))
    d = ImageDraw.Draw(img)

    cx, cy = px(80), px(70)
    rx, ry = px(40), px(37)

    # ---- long hair falling behind the head to the bottom of the frame -------
    d.polygon([(cx - px(46), cy - px(34)), (px(6), px(120)), (px(52), px(120)),
               (cx - px(16), cy - px(6))], fill=HAIR_DK)
    d.polygon([(cx + px(46), cy - px(34)), (px(154), px(120)), (px(108), px(120)),
               (cx + px(16), cy - px(6))], fill=HAIR_DK)
    d.ellipse([cx - px(48), cy - px(46), cx + px(48), cy + px(40)], fill=HAIR)
    # front locks framing the face
    d.polygon([(cx - px(42), cy - px(18)), (px(14), px(120)), (px(44), px(120)),
               (cx - px(20), cy + px(6))], fill=HAIR)
    d.polygon([(cx + px(42), cy - px(18)), (px(146), px(120)), (px(116), px(120)),
               (cx + px(20), cy + px(6))], fill=HAIR)
    d.arc([cx - px(38), cy - px(40), cx + px(38), cy + px(14)],
          start=200, end=340, fill=HAIR_LT, width=px(4))

    # ---- maid headband: a pale ruffle band across the top -------------------
    d.arc([cx - px(44), cy - px(44), cx + px(44), cy + px(22)],
          start=196, end=344, fill=(0xF2, 0xEE, 0xEF), width=px(9))

    # ---- shoulders, then the face on top ------------------------------------
    d.rounded_rectangle([cx - px(34), cy + px(26), cx + px(34), px(124)],
                        radius=px(14), fill=(0xE9, 0xE4, 0xE6))
    d.rounded_rectangle([cx - px(22), px(104), cx + px(22), px(124)],
                        radius=px(8), fill=(0xFB, 0xF4, 0xF2))
    d.ellipse([cx - rx, cy - ry, cx + rx, cy + ry], fill=SKIN)

    # fringe over the forehead
    d.chord([cx - px(44), cy - px(46), cx + px(44), cy + px(10)],
            start=180, end=360, fill=HAIR)
    d.chord([cx - px(30), cy - px(40), cx + px(10), cy + px(6)],
            start=200, end=345, fill=HAIR_LT)

    # ahoge: a single curve, not a bent line
    d.arc([cx + px(2), cy - px(64), cx + px(36), cy - px(30)],
          start=160, end=330, fill=HAIR, width=px(4))

    # ---- eyes ---------------------------------------------------------------
    eye_y = cy + px(2)
    for side, ex in (("l", cx - px(20)), ("r", cx + px(20))):
        if expression == "closed":
            d.arc([ex - px(13), eye_y - px(9), ex + px(13), eye_y + px(9)],
                  start=200, end=340, fill=EYE, width=px(4))
            continue
        if expression == "half":
            d.ellipse([ex - px(13), eye_y - px(14), ex + px(13), eye_y + px(14)], fill=WHITE)
            d.ellipse([ex - px(12), eye_y - px(13), ex + px(12), eye_y + px(13)], fill=EYE)
            d.ellipse([ex - px(8), eye_y - px(6), ex + px(8), eye_y + px(11)], fill=IRIS)
            # skin-coloured lid covering the top half
            d.chord([ex - px(14), eye_y - px(16), ex + px(14), eye_y + px(12)],
                    start=180, end=360, fill=SKIN)
            d.line([(ex - px(13), eye_y - px(2)), (ex + px(13), eye_y - px(2))],
                   fill=EYE, width=px(3))
            continue
        if expression == "happy":
            d.arc([ex - px(13), eye_y - px(10), ex + px(13), eye_y + px(10)],
                  start=200, end=340, fill=EYE, width=px(5))
            continue
        # open eye
        d.ellipse([ex - px(13), eye_y - px(14), ex + px(13), eye_y + px(14)], fill=WHITE)
        d.ellipse([ex - px(12), eye_y - px(13), ex + px(12), eye_y + px(13)], fill=EYE)
        shift = -px(3) if pupil == "left" else (px(3) if pupil == "right" else 0)
        d.ellipse([ex - px(8) + shift, eye_y - px(6), ex + px(8) + shift, eye_y + px(11)],
                  fill=IRIS)
        d.ellipse([ex - px(5) + shift, eye_y + px(1), ex + px(5) + shift, eye_y + px(11)],
                  fill=(0x30, 0x4E, 0x86))
        d.ellipse([ex - px(7) + shift, eye_y - px(9), ex - px(1) + shift, eye_y - px(3)],
                  fill=WHITE)

    # ---- cheeks, mouth ------------------------------------------------------
    if expression != "idle":
        for ex in (cx - px(32), cx + px(32)):
            d.ellipse([ex - px(7), cy + px(14), ex + px(7), cy + px(24)], fill=CHEEK)

    if expression == "happy":
        d.chord([cx - px(9), cy + px(16), cx + px(9), cy + px(30)],
                start=0, end=180, fill=EYE)
    else:
        d.arc([cx - px(6), cy + px(18), cx + px(6), cy + px(28)],
              start=20, end=160, fill=EYE, width=px(3))

    # ---- bow ----------------------------------------------------------------
    bx, by = cx + px(40), cy - px(26)
    d.ellipse([bx - px(12), by - px(8), bx + px(-2), by + px(8)], fill=BOW)
    d.ellipse([bx + px(2), by - px(8), bx + px(12), by + px(8)], fill=BOW)
    d.ellipse([bx - px(3), by - px(4), bx + px(3), by + px(4)], fill=HAIR_LT)

    return img.resize((W, H), Image.LANCZOS)


def quantise(img):
    out = img.copy()
    px_map = out.load()
    for y in range(H):
        for x in range(W):
            c = px_map[x, y]
            if c == (0, 0, 0):
                continue
            best = min(PALETTE, key=lambda p: (c[0]-p[0])**2 + (c[1]-p[1])**2 + (c[2]-p[2])**2)
            px_map[x, y] = best
    return out


FRAMES = [
    ("IDLE", "idle", "centre"),
    ("BLINK_HALF", "half", "centre"),
    ("BLINK", "closed", "centre"),
    ("HAPPY", "happy", "centre"),
    ("LOOK_L", "idle", "left"),
    ("LOOK_R", "idle", "right"),
]


def pack(img):
    cells = []
    for y in range(H):
        for x in range(W):
            c = img.getpixel((x, y))
            cells.append(0 if c == (0, 0, 0) else PALETTE.index(c))
    return [(cells[i] << 4) | (cells[i + 1] if i + 1 < len(cells) else 0)
            for i in range(0, len(cells), 2)]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--preview")
    ap.add_argument("--header")
    args = ap.parse_args()

    rendered = [(name, quantise(draw(name, expr, pupil))) for name, expr, pupil in FRAMES]

    if args.preview:
        from PIL import Image
        sheet = Image.new("RGB", (W * 3 + 40, H * 2 + 30), (18, 18, 22))
        for i, (name, img) in enumerate(rendered):
            sheet.paste(img.resize((W * 3, H * 3), Image.NEAREST)
                        if False else img, (10 + (i % 3) * (W + 10), 20 + (i // 3) * (H + 10)))
        sheet = sheet.resize((sheet.width * 2, sheet.height * 2), Image.NEAREST)
        sheet.save(args.preview)
        print("preview ->", args.preview)

    if args.header:
        lines = ["/* Generated by tools/art/make_pet_frames.py - do not edit by hand. */",
                 "#pragma once", "#include <stdint.h>",
                 f"#define PET_GRID_W {W}", f"#define PET_GRID_H {H}",
                 f"#define PET_FRAME_COUNT {len(rendered)}"]
        for name, img in rendered:
            data = pack(img)
            lines.append(f"static const uint8_t PET_{name}[{len(data)}] = {{")
            for i in range(0, len(data), 16):
                lines.append("    " + ", ".join(f"0x{b:02X}" for b in data[i:i+16]) + ",")
            lines.append("};")
        Path(args.header).write_text("\n".join(lines) + "\n", encoding="utf-8")
        print("header ->", args.header)


if __name__ == "__main__":
    main()
