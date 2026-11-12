#!/usr/bin/env python3
"""
Generates the stand-in pictures for the Ultima III "textured dungeon" renderer, which is used with
the Ultima III Upgrade's VGA graphics mode.

Every picture is a 128x128 layer in the geometry of the original first person view, and uses the
256 colors of the Upgrade's VGA palette (files/ultima3/u3vga.pal). Palette index 0 is transparent.
The shapes come from the renderer's own tables in gfx/dungeon_view.cpp, so a face covers exactly
the part of the view that the original line drawing did.

  face_00..face_31.png   the wall seen from each place in view
  face_32..face_63.png   the doors of each place: face_32 is the opening cut out of a wall
                         the party stands in, and the rest are doors set in the walls
  ladder_00..20.png      a ladder going up, which is flipped upside down for one going down
  chest_00..20.png       a chest

These are stand-in pictures made by a program, written with the help of an AI assistant (Claude).
They are meant to be replaced by hand drawn art: any picture can be swapped for one of the same
name, size and palette, and the renderer falls back to its lines for any that are missing.

Usage: gen_ultima3_dungeons.py <dungeon_view.cpp> <u3vga.pal> <output folder> [<dungeon .ULT> <preview.png>]
"""
import math
import os
import random
import re
import struct
import sys
import zlib

SIZE = 128


# ------------------------------------------------------------------- palette

class Palette:
    # the entries in the file that are not used for anything are this color
    UNUSED = (45, 255, 210)

    def __init__(self, path):
        data = open(path, "rb").read()
        self.colors = [tuple(round(v * 255 / 63) for v in data[i * 3:i * 3 + 3]) for i in range(256)]

        # the colors that a picture can use, which is each distinct one that's in use
        self.usable = []
        seen = set()
        for index, color in enumerate(self.colors):
            if index == 0 or color == self.UNUSED or color in seen:
                continue
            seen.add(color)
            self.usable.append((index, color))

        # the browns, which are used for wood so that it doesn't pick up olive or yellow speckles
        self.browns = [(i, c) for (i, c) in self.usable
                       if c[0] > c[1] > c[2] and 0.4 < c[1] / c[0] < 0.75 and c[0] - c[2] > 24]
        self.cache = {}

    def nearest(self, r, g, b, pool=None):
        r, g, b = (max(0, min(255, int(v))) for v in (r, g, b))
        key = (r >> 1, g >> 1, b >> 1, id(pool))
        found = self.cache.get(key)
        if found is None:
            best, best_d = 1, 1 << 30
            for index, (cr, cg, cb) in (pool or self.usable):
                d = 3 * (cr - r) ** 2 + 5 * (cg - g) ** 2 + 2 * (cb - b) ** 2
                if d < best_d:
                    best, best_d = index, d
            found = self.cache[key] = best
        return found


# ---------------------------------------------------------------- PNG output

def chunk(tag, data):
    body = tag + data
    return struct.pack(">I", len(data)) + body + struct.pack(">I", zlib.crc32(body) & 0xFFFFFFFF)


def write_indexed(path, pixels, w, h, palette):
    """Writes an 8 bit paletted PNG, in which index 0 is transparent"""
    raw = b"".join(b"\x00" + bytes(pixels[y * w:(y + 1) * w]) for y in range(h))
    plte = b"".join(bytes(c) for c in palette.colors)
    data = b"\x89PNG\r\n\x1a\n"
    data += chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 3, 0, 0, 0))
    data += chunk(b"PLTE", plte)
    data += chunk(b"tRNS", b"\x00")
    data += chunk(b"IDAT", zlib.compress(raw, 9))
    data += chunk(b"IEND", b"")
    with open(path, "wb") as f:
        f.write(data)


def write_rgb(path, rgb, w, h):
    raw = b"".join(b"\x00" + bytes(rgb[y * w * 3:(y + 1) * w * 3]) for y in range(h))
    data = b"\x89PNG\r\n\x1a\n"
    data += chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0))
    data += chunk(b"IDAT", zlib.compress(raw, 9))
    data += chunk(b"IEND", b"")
    with open(path, "wb") as f:
        f.write(data)


# ----------------------------------------------------------- renderer tables

def parse_tables(source):
    text = open(source).read()

    def table(name):
        m = re.search(name + r"\[(\d+)\](?:\[(\d+)\])? = \{(.*?)\n\};", text, re.S)
        body = m.group(3)
        if m.group(2):
            rows = re.findall(r"\{([^}]*)\}", body)
            return [[int(v, 0) for v in r.split(",") if v.strip()] for r in rows]
        return [int(v, 0) for v in re.findall(r"0x[0-9A-Fa-f]+", body)]

    return {
        "walls": table("WALLS"), "ladders": table("LADDERS"), "chests": table("CHESTS"),
        "dx": table("CELL_DX"), "dy": table("CELL_DY")
    }


def signed(v):
    return v - 256 if v > 127 else v


def face_columns(rec):
    """The columns of a face, as the renderer works them out: (x, top, bottom, step)"""
    x, top, bottom, step, top_n, bottom_n, count = rec[:7]
    step = signed(step)
    top_c, bottom_c = top_n, bottom_n
    out = []
    while True:
        out.append((x, top, bottom, step))
        count -= 1
        if count == 0:
            break

        x = (x + step) & 0xFF
        for _ in range(1 if step == 1 else 2):
            top_c = (top_c - 1) & 0xFF
            if top_c == 0:
                top_c = top_n
                top = (top + 1) & 0xFF
            bottom_c = (bottom_c - 1) & 0xFF
            if bottom_c == 0:
                bottom_c = bottom_n
                bottom = (bottom - 1) & 0xFF
    return out


class Face:
    """The shape of a face, with where each of its pixels is on the wall"""

    def __init__(self, rec):
        self.rec = rec
        self.cols = face_columns(rec)
        self.frontal = rec[4] == 0xFF and rec[5] == 0xFF
        self.cells = {}          # (x, y) -> (u, v, height)

        heights = [(b - t) & 0xFF for (_, t, b, _) in self.cols]
        self.h_near = max(heights[0], 1)
        self.h_far = max(heights[-1], 1)
        n = len(self.cols)

        for i, (x, top, bottom, step) in enumerate(self.cols):
            h = max((bottom - top) & 0xFF, 1)
            if self.frontal or self.h_near == self.h_far:
                u = (i + 0.5) / n
            else:
                # the distance along a side wall goes with 1 / height
                u = (1.0 / h - 1.0 / self.h_near) / (1.0 / self.h_far - 1.0 / self.h_near)

            xs = (x, x + 1) if step > 0 else (x - 1, x)
            for xx in xs:
                for y in range(top, top + h):
                    if 0 <= xx < SIZE and 0 <= y < SIZE:
                        self.cells[(xx, y)] = (u, (y - top + 0.5) / h, h)

    def width_px(self, h):
        """About how wide a cell of the wall is on the screen where the wall is h high"""
        if self.frontal:
            return len(self.cols) * 2
        return max(len(self.cols) * 2 * h / max(self.h_near, 1), 1.0)


# ------------------------------------------------------------------ painting

def depth_level(h):
    """How lit stone is, by how big it is in view: far stone is darker"""
    return max(0.0, min(1.0, (h - 10) / 110.0))


def mix(a, b, t):
    return tuple(a[i] + (b[i] - a[i]) * t for i in range(3))


def scale(c, k):
    return tuple(v * k for v in c)


STONE = (118, 110, 98)
MORTAR = (36, 33, 30)
RUST = (122, 66, 32)
MOSS = (52, 98, 42)
WOOD = (124, 82, 44)
IRON = (76, 78, 88)
FOG = (6, 6, 14)


def put(img, palette, x, y, color, wood=False):
    img[y * SIZE + x] = palette.nearest(*color, pool=palette.browns if wood else None)


def stone(face, palette, seed, rows=8, bricks=4):
    """Rough stone blocks laid in courses, dirtier and darker the further away they are"""
    rng = random.Random(seed)
    img = bytearray(SIZE * SIZE)
    block = {}

    def block_info(key):
        if key not in block:
            block[key] = (rng.uniform(-0.22, 0.22), rng.random(), rng.random(), rng.uniform(-1, 1))
        return block[key]

    for (x, y), (u, v, h) in face.cells.items():
        row = int(v * rows)
        bu = u * bricks + (0.5 if row & 1 else 0.0)
        col = int(bu)
        tone, stain, moss, hue = block_info((row, col))

        brick_h = h / rows
        fy = v * rows - row
        fx = bu - int(bu)
        brick_w = face.width_px(h) / bricks

        mortar = (brick_h >= 3 and fy * brick_h < 1.0) or (brick_w >= 3 and fx * brick_w < 1.0)
        light = 0.34 + 0.66 * depth_level(h)

        # shadow where the wall meets the ceiling and floor, and in the corners
        light *= 1.0 - 0.45 * max(0.0, (0.12 - v) / 0.12)
        light *= 1.0 - 0.35 * max(0.0, (v - 0.85) / 0.15)
        light *= 1.0 - 0.30 * max(0.0, (0.08 - u) / 0.08) - 0.30 * max(0.0, (u - 0.92) / 0.08)

        if mortar:
            color = scale(MORTAR, light * 1.4)
        else:
            color = (STONE[0] * (1 + 0.10 * hue), STONE[1], STONE[2] * (1 - 0.10 * hue))
            color = scale(color, (1 + tone) * light)

            # the top of each block catches the light, and the bottom is in shadow
            if brick_h >= 4:
                if 1.0 <= fy * brick_h < 2.0:
                    color = scale(color, 1.18)
                elif fy > 1 - 1.5 / brick_h:
                    color = scale(color, 0.82)

            # stains and moss on some blocks, and damp that runs down the lower walls
            if stain > 0.9:
                color = mix(color, scale(RUST, light), 0.55 * (0.6 + 0.4 * math.sin(x * 1.7)))
            elif stain > 0.78:
                color = scale(color, 0.8)
            if moss > 0.92 and v > 0.5:
                color = mix(color, scale(MOSS, light * 1.3), 0.65 * min(1.0, (v - 0.5) * 3))
            color = mix(color, scale((70, 62, 50), light), 0.35 * max(0.0, (v - 0.8) / 0.2))

        # the grain of the stone
        color = scale(color, 1.0 + rng.gauss(0, 0.045))
        put(img, palette, x, y, mix(color, FOG, 0.2 * (1 - depth_level(h))))

    return img


DOOR_WOOD = (150, 86, 42)


def door(face, palette, seed):
    """A wooden door of planks with iron bands, inside a dark frame"""
    rng = random.Random(seed)
    img = bytearray(SIZE * SIZE)
    planks = 4

    for (x, y), (u, v, h) in face.cells.items():
        edge = (((x - 1, y) not in face.cells) or ((x + 1, y) not in face.cells) or
                ((x, y - 1) not in face.cells) or ((x, y + 1) not in face.cells))

        # doors stay as bright and rich a brown at a distance, so that they stand out from the walls
        light = 0.85 + 0.25 * depth_level(h)

        if edge:
            put(img, palette, x, y, scale((50, 34, 22), light))
            continue

        pixels_per_plank = max(face.width_px(h) / planks, 1.0)
        plank = int(u * planks)
        fu = (u * planks - plank) * pixels_per_plank
        grain = 0.07 * math.sin(y * 0.8 + plank * 2.7) + 0.05 * math.sin(y * 2.3 + x * 0.3 + plank)
        color = scale(DOOR_WOOD, light * (1.0 + grain + (0.14 if plank & 1 else -0.08)))

        # the gaps between the planks, and the lit edge of each
        if fu < 1.3:
            color = scale((46, 22, 10), light)
        elif fu < 2.4:
            color = scale(color, 1.2)

        # iron bands across the door, edged dark, with rivets
        on_wood = True
        for band in (0.2, 0.8):
            d = abs(v - band)
            if d < 0.045:
                color = scale((196, 196, 208), light)
                on_wood = False
                if int(u * 14) & 1 and d < 0.015 and h > 24:
                    color = scale((120, 120, 132), light)
            elif d < 0.065:
                color = scale((52, 52, 60), light)
                on_wood = False

        color = scale(color, 1.0 + rng.gauss(0, 0.035))
        put(img, palette, x, y, color, wood=on_wood)

    return img


def ladder(rec, palette, seed):
    """A ladder with wooden rails and rungs, and nothing between them"""
    rng = random.Random(seed)
    left, right, top_l, top_r, edge_far, edge_near = rec[:6]
    img = bytearray(SIZE * SIZE)

    def dot(x, y, c):
        if 0 <= x < SIZE and 0 <= y < SIZE:
            put(img, palette, x, y, c, wood=True)

    width = right - left
    light = 0.5 + 0.5 * min(1.0, width / 32.0)
    middle = 0x40

    # the rails, which are missing where the ladder is out of view
    rail_w = max(1, round(width / 12))
    rails = []
    if top_l != 0:
        rails.append((left, top_l))
    if top_r != 0:
        rails.append((right - rail_w + 1, top_r))
    if not rails:
        return img

    top = min(start for (_, start) in rails)
    for rx, start in rails:
        for y in range(start, middle + 1):
            for w in range(rail_w):
                shade = 1.15 - 0.4 * (w / rail_w)
                dot(rx + w, y, scale(WOOD, light * shade * (1.0 + rng.gauss(0, 0.05))))

    # the rungs are spread evenly down the rails, as far apart as the ladder is wide allows
    length = middle - top
    count = max(2, round(length / max(3.0, width * 0.45)))
    for i in range(count):
        y = int(top + (i + 0.5) * length / count)
        for x in range(max(left, 0), right + 1):
            if any(rx <= x < rx + rail_w for rx, _ in rails):
                continue
            dot(x, y, scale((154, 108, 62), light * 1.1))
            if width >= 12:
                dot(x, y + 1, scale((110, 74, 40), light))

    return img


def chest(rec, palette, seed):
    x0, x1, y0, y1 = rec[:4]
    rng = random.Random(seed)
    img = bytearray(SIZE * SIZE)
    w, h = x1 - x0, y1 - y0
    light = 0.4 + 0.6 * min(1.0, w / 16.0)
    lid = max(1, h // 3)

    for y in range(y0, y1):
        for x in range(x0, x1):
            on_lid = y - y0 < lid
            color = scale(WOOD, light * (1.12 if on_lid else 0.92))
            color = scale(color, 1.0 + 0.08 * math.sin(x * 1.9 + y * 0.4))
            if y == y0 or y == y1 - 1 or x == x0 or x == x1 - 1:
                color = scale((40, 26, 14), light + 0.2)
            elif y - y0 == lid:
                color = scale((50, 32, 16), light + 0.2)
            put(img, palette, x, y, scale(color, 1.0 + rng.gauss(0, 0.04)))

    # iron bands and the lock
    if w >= 6:
        for bx in (x0 + max(1, w // 5), x1 - 1 - max(1, w // 5)):
            for y in range(y0 + 1, y1 - 1):
                put(img, palette, bx, y, scale(IRON, light * 1.5))
        cx = (x0 + x1) // 2
        for y in range(y0 + lid - 1, min(y1 - 1, y0 + lid + 2)):
            if y > y0:
                put(img, palette, cx, y, scale((232, 190, 62), light + 0.1))
                if w >= 12:
                    put(img, palette, cx + 1, y, scale((200, 160, 44), light))
    return img


# -------------------------------------------------------------- the preview

def preview(tables, images, palette, dungeon_path, level, px, py, facing):
    """Draws a view the way the renderer does, with these images, to see how they look"""
    d = open(dungeon_path, "rb").read()
    DT_WALL, DT_DOOR, DT_UP, DT_DOWN, DT_CHEST = 0x80, 0xC0, 0x10, 0x20, 0x40

    def tile(x, y):
        return d[(level << 8) + ((y & 15) << 4) + (x & 15)]

    buf = bytearray(SIZE * SIZE)
    deferred = []

    def blit(img, flip=False, erase=False):
        for y in range(SIZE):
            sy = SIZE - 1 - y if flip else y
            for x in range(SIZE):
                c = img[sy * SIZE + x]
                if c:
                    buf[y * SIZE + x] = 0 if erase else c

    def look(place):
        dx, dy = signed(tables["dx"][place]), signed(tables["dy"][place])
        for _ in range(facing):
            dx, dy = dy, dx
            dx = -dx
        t = tile(px + dx, py + dy)
        if not (t & DT_WALL):
            if place < 21:
                if t & DT_UP:
                    deferred.append((images["ladder_%02d" % place], False))
                if t & DT_DOWN:
                    deferred.append((images["ladder_%02d" % place], True))
                if t & DT_CHEST:
                    deferred.append((images["chest_%02d" % place], False))
            return "open"
        blit(images["face_%02d" % place])
        if t < DT_DOOR:
            return "wall"
        blit(images["face_%02d" % (place + 32)], erase=(place + 32 == 32))
        return "door" if place == 0 else "wall"

    def along(places):
        for p in places:
            if look(p) == "wall":
                return

    left_a, right_a = [1, 3, 6, 10, 15, 21], [2, 5, 9, 14, 20, 27]
    left_b, right_b = [7, 11, 16, 22], [8, 13, 19, 26]
    left_c, right_c = [17, 23, 28], [18, 25, 31]

    here = look(0)
    if here != "wall":
        if here == "open":
            along(left_a)
            along(right_a)
        if look(4) == "open":
            along(left_b)
            along(right_b)
            if look(12) == "open":
                along(left_c)
                along(right_c)
                if look(24) != "wall":
                    look(29)
                    look(30)

    for img, flip in reversed(deferred):
        blit(img, flip)

    rgb = bytearray()
    for y in range(SIZE):
        for x in range(SIZE):
            rgb += bytes(palette.colors[buf[y * SIZE + x]])
    return rgb


def enlarge(rgb, w, h, k):
    out = bytearray()
    for y in range(h):
        row = bytearray()
        for x in range(w):
            row += bytes(rgb[(y * w + x) * 3:(y * w + x) * 3 + 3]) * k
        out += bytes(row) * k
    return out


def main():
    source, palette_path, outdir = sys.argv[1:4]
    tables = parse_tables(source)
    palette = Palette(palette_path)
    os.makedirs(outdir, exist_ok=True)

    images = {}
    for i in range(64):
        face = Face(tables["walls"][i])
        if i < 32:
            images["face_%02d" % i] = stone(face, palette, 1000 + i)
        elif i == 32:
            # the cut out only rubs out the wall, so it can be any color
            cut = bytearray(SIZE * SIZE)
            for (x, y) in face.cells:
                cut[y * SIZE + x] = palette.nearest(60, 60, 60)
            images["face_32"] = cut
        else:
            images["face_%02d" % i] = door(face, palette, 2000 + i)

    for i in range(21):
        images["ladder_%02d" % i] = ladder(tables["ladders"][i], palette, 3000 + i)
        images["chest_%02d" % i] = chest(tables["chests"][i], palette, 4000 + i)

    for name, img in images.items():
        write_indexed(os.path.join(outdir, name + ".png"), img, SIZE, SIZE, palette)

    if len(sys.argv) > 5:
        dungeon, preview_path = sys.argv[4], sys.argv[5]
        shots = [preview(tables, images, palette, dungeon, lvl, x, y, f)
                 for (lvl, x, y, f) in [(0, 1, 3, 1), (0, 3, 1, 3), (0, 13, 2, 0), (0, 2, 1, 3)]]
        k = 3
        w = SIZE * len(shots)
        rgb = bytearray()
        for y in range(SIZE):
            for s in shots:
                rgb += s[y * SIZE * 3:(y + 1) * SIZE * 3]
        write_rgb(preview_path, enlarge(rgb, w, SIZE, k), w * k, SIZE * k)


if __name__ == "__main__":
    main()
