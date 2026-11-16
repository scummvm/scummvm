#!/usr/bin/env python3
"""
Generates the stand-in pictures for the Ultima II "textured dungeon" renderer, which is used with
any of the EGA graphics modes.

The EGA modes only use 16 colors, in palette entries 16 to 31, and the others in the 256 color
palette are free. This defines a standard set of colors for entries 48 to 255 (see the ramps
below, and the palette.pal that's written with the pictures, which the game loads) that the
pictures are drawn with. The pictures only use those, so they look the same in every EGA mode.
Palette index 0 is transparent.

Every picture is a 256x128 layer in the geometry of the original wire frame view of the corridor,
which is worked out from the same tables as the game's (views/dungeon_map.cpp). The pictures of
the places at each depth, 0 being the square the party is on, are:

  front_D        the wall in front of the party at depth D
  left_D/right_D the wall to a side at depth D
  leftopen_D/rightopen_D  an opening in the side at depth D
  frontdoor_D    a door in the wall in front
  doorway        the wall seen from a doorway the party is standing in, with the opening cut out
  leftdoor_D/rightdoor_D  a door in the wall at a side
  ladderup_D/ladderdown_D a ladder up through the ceiling, and one down through the floor
  chest_D        a chest

Usage: gen_ultima2_dungeons.py <dungeon_map.cpp> <output folder> [<preview.png>]
"""
import math
import os
import random
import re
import struct
import sys
import zlib

WIDTH, HEIGHT = 256, 128

# The column that the corridor recedes to, which other games with the same sort of view change
CENTER = 128
PALETTE_BASE = 48

# The EGA palette of the game, which lives at entries 16 to 31
EGA = [
    (0x00, 0x00, 0x00), (0x00, 0x00, 0xaa), (0x00, 0xaa, 0x00), (0x00, 0xaa, 0xaa),
    (0xaa, 0x00, 0x00), (0xaa, 0x00, 0xaa), (0xaa, 0x55, 0x00), (0xaa, 0xaa, 0xaa),
    (0x55, 0x55, 0x55), (0x55, 0x55, 0xff), (0x55, 0xff, 0x55), (0x55, 0xff, 0xff),
    (0xff, 0x55, 0x55), (0xff, 0x55, 0xff), (0xff, 0xff, 0x55), (0xff, 0xff, 0xff)
]

# The colors in the standard set, as ramps from dark to light through a middle color. Each is
# (name, dark, middle, light, how many). They fill the entries from PALETTE_BASE onwards.
RAMPS = [
    ("grey", (8, 8, 10), (118, 118, 122), (235, 235, 240), 22),
    ("stone", (24, 20, 16), (118, 110, 98), (210, 200, 184), 22),
    ("wood", (28, 14, 6), (150, 86, 42), (232, 170, 104), 24),
    ("rust", (40, 16, 8), (122, 66, 32), (200, 120, 70), 12),
    ("moss", (8, 24, 8), (52, 98, 42), (150, 200, 110), 16),
    ("gold", (60, 40, 0), (230, 190, 60), (255, 245, 160), 12),
    ("iron", (14, 16, 24), (90, 94, 108), (200, 205, 220), 12),
    ("flesh", (50, 20, 16), (190, 110, 90), (250, 200, 170), 16),
    ("red", (40, 0, 0), (190, 30, 24), (255, 150, 130), 16),
    ("green", (6, 30, 12), (40, 150, 60), (170, 240, 150), 12),
    ("blue", (6, 10, 50), (50, 90, 210), (160, 200, 255), 12),
    ("pale", (60, 56, 20), (230, 220, 110), (255, 255, 230), 12),
    ("purple", (30, 10, 50), (130, 60, 170), (220, 170, 250), 12),
    ("cyan", (0, 60, 60), (60, 200, 210), (200, 255, 255), 8),
]


class Palette:
    def __init__(self):
        self.colors = [(0, 0, 0)] * 256
        for i, c in enumerate(EGA):
            self.colors[16 + i] = c

        self.ramps = {}
        index = PALETTE_BASE
        for name, dark, middle, light, count in RAMPS:
            entries = []
            for i in range(count):
                t = i / (count - 1)
                if t < 0.5:
                    c = [dark[k] + (middle[k] - dark[k]) * t * 2 for k in range(3)]
                else:
                    c = [middle[k] + (light[k] - middle[k]) * (t - 0.5) * 2 for k in range(3)]
                self.colors[index] = tuple(round(v) for v in c)
                entries.append(index)
                index += 1
            self.ramps[name] = entries
        assert index == 256, index

        self.cache = {}

    def nearest(self, r, g, b, ramps=None):
        """The entry nearest a color, out of the whole set or just the named ramps"""
        r, g, b = (max(0, min(255, int(v))) for v in (r, g, b))
        key = (r >> 1, g >> 1, b >> 1, tuple(ramps) if ramps else None)
        found = self.cache.get(key)
        if found is None:
            pool = [i for name in ramps for i in self.ramps[name]] if ramps else range(PALETTE_BASE, 256)
            best, best_d = PALETTE_BASE, 1 << 30
            for i in pool:
                cr, cg, cb = self.colors[i]
                d = 3 * (cr - r) ** 2 + 5 * (cg - g) ** 2 + 2 * (cb - b) ** 2
                if d < best_d:
                    best, best_d = i, d
            found = self.cache[key] = best
        return found

    def ramp_color(self, name, t):
        """An entry of a ramp, for a position along it from 0 (dark) to 1 (light)"""
        entries = self.ramps[name]
        return entries[max(0, min(len(entries) - 1, round(t * (len(entries) - 1))))]

    def write(self, path):
        with open(path, "wb") as f:
            for i in range(PALETTE_BASE, 256):
                f.write(bytes(self.colors[i]))


# ---------------------------------------------------------------- PNG output

def chunk(tag, data):
    body = tag + data
    return struct.pack(">I", len(data)) + body + struct.pack(">I", zlib.crc32(body) & 0xFFFFFFFF)


def write_indexed(path, pixels, w, h, palette):
    raw = b"".join(b"\x00" + bytes(pixels[y * w:(y + 1) * w]) for y in range(h))
    data = b"\x89PNG\r\n\x1a\n"
    data += chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 3, 0, 0, 0))
    data += chunk(b"PLTE", b"".join(bytes(c) for c in palette.colors))
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

TABLE_BASE = 0x1620D
ADDR = dict(
    LEFT_X=0x1620D, LEFT_X_NEXT=0x1620E, LEFT_X_NEXT2=0x1620F, LEFT_X_NEXT3=0x16210,
    RIGHT_X=0x16217, RIGHT_X_NEXT=0x16218, RIGHT_X_NEXT2=0x16219, RIGHT_X_NEXT3=0x1621A,
    TOP_Y=0x16221, TOP_Y_NEXT=0x16222, BOTTOM_Y=0x16229, BOTTOM_Y_NEXT=0x1622A,
    LADDER_LEFT_A=0x16232, LADDER_LEFT_B=0x1623A, LADDER_RIGHT_B=0x16242, LADDER_RIGHT_A=0x1624A,
    LEFT_DOOR_X=0x16231, LEFT_DOOR_X2=0x16239, AHEAD_DOOR_X=0x16241, AHEAD_DOOR_X2=0x16249,
    DOOR_BOTTOM_A=0x16251, DOOR_BOTTOM_B=0x16259, DOOR_TOP_B=0x16261,
    RAIL_TOP_A=0x16269, RAIL_TOP_B=0x16271, RAIL_BOTTOM_A=0x16279, RAIL_BOTTOM_B=0x16281,
    CHEST_BOTTOM=0x16289
)


def parse_table(source):
    text = open(source).read()
    m = re.search(r"const byte TABLE\[\] = \{(.*?)\n\};", text, re.S)
    body = re.sub(r"//[^\n]*", "", m.group(1))
    return [int(v, 16) for v in re.findall(r"0x[0-9A-Fa-f]+", body)]


class Tables:
    def __init__(self, source):
        self.table = parse_table(source)

    def t(self, name, d):
        return self.table[ADDR[name] - TABLE_BASE + d]


# -------------------------------------------------------------------- shapes

def lerp(a, b, t):
    return a + (b - a) * t


class Shape:
    """The pixels of a piece, each with where it is on the surface: (u across, v down, height)"""

    def __init__(self):
        self.cells = {}
        self.slope = {}
        self.width = 1
        self.height = 1

    def add_columns(self, x0, x1, top0, top1, bottom0, bottom1, perspective=None):
        """A piece with vertical sides, whose top and bottom run straight between two columns.
        Perspective is the distance of the near and far edge from the vanishing point, to
        space what's drawn on it as it is in depth"""
        if x1 < x0:
            x0, x1, top0, top1, bottom0, bottom1 = x1, x0, top1, top0, bottom1, bottom0
            if perspective:
                perspective = (perspective[1], perspective[0])
        span = max(x1 - x0, 1)
        self.width = max(self.width, x1 - x0 + 1)

        for x in range(x0, x1 + 1):
            t = (x - x0) / span
            top = lerp(top0, top1, t)
            bottom = lerp(bottom0, bottom1, t)
            h = max(bottom - top, 1.0)
            self.height = max(self.height, h)

            if perspective and perspective[0] != perspective[1]:
                near, far = perspective

                def along(px):
                    distance = abs(CENTER - px)
                    v = (1 / max(distance, 0.5) - 1 / near) / (1 / far - 1 / near)
                    return max(0.0, min(1.0, v))

                u = along(x + 0.5)
                self.slope[x] = max(abs(along(x + 1) - along(x)), 0.0005)
            else:
                u = (x - x0 + 0.5) / (x1 - x0 + 1)
                self.slope[x] = 1.0 / (x1 - x0 + 1)

            for y in range(int(round(top)), int(round(bottom)) + 1):
                if 0 <= x < WIDTH and 0 <= y < HEIGHT:
                    self.cells[(x, y)] = (u, (y - top + 0.5) / h, h)

    def add_polygon(self, points):
        """A piece with any outline, mapped in a simple way by where it is within its extent"""
        xs = [p[0] for p in points]
        ys = [p[1] for p in points]
        x0, x1, y0, y1 = min(xs), max(xs), min(ys), max(ys)
        self.width = max(self.width, x1 - x0 + 1)
        self.height = max(self.height, y1 - y0 + 1)

        for y in range(int(y0), int(y1) + 1):
            hits = []
            for i in range(len(points)):
                ax, ay = points[i]
                bx, by = points[(i + 1) % len(points)]
                if (ay <= y + 0.5 < by) or (by <= y + 0.5 < ay):
                    hits.append(ax + (y + 0.5 - ay) / (by - ay) * (bx - ax))
            hits.sort()
            for i in range(0, len(hits) - 1, 2):
                for x in range(int(math.ceil(hits[i] - 0.5)), int(math.floor(hits[i + 1] - 0.5)) + 1):
                    if 0 <= x < WIDTH and 0 <= y < HEIGHT:
                        self.cells[(x, y)] = ((x - x0 + 0.5) / (x1 - x0 + 1), (y - y0 + 0.5) / (y1 - y0 + 1),
                                              y1 - y0 + 1)

    def border(self, x, y):
        c = self.cells
        return ((x - 1, y) not in c) or ((x + 1, y) not in c) or ((x, y - 1) not in c) or ((x, y + 1) not in c)


# ------------------------------------------------------------------ painting

def mix(a, b, t):
    return tuple(a[i] + (b[i] - a[i]) * t for i in range(3))


def scale(c, k):
    return tuple(v * k for v in c)


STONE = (118, 110, 98)
MORTAR = (36, 33, 30)
RUST = (122, 66, 32)
MOSS = (52, 98, 42)
WOOD = (150, 86, 42)
IRON = (76, 78, 88)
GOLD = (232, 190, 62)


def depth_light(d):
    """Stone is lit less the further away it is"""
    return 1.0 - 0.1 * d


def put(img, palette, x, y, color, ramps=None):
    img[y * WIDTH + x] = palette.nearest(*color, ramps=ramps)


# The stone is laid the same size everywhere, in courses of ROWS to the height of a square and
# BRICKS to its length, so a wall carries straight on from one square to the next
ROWS = 8
BRICKS = 4


def brick_info(group, row, col):
    rng = random.Random("%s/%d/%d" % (group, row, col))
    return (rng.uniform(-0.22, 0.22), rng.random(), rng.random(), rng.uniform(-1, 1))


def stone(shape, palette, seed, d, tone=1.0, rows=ROWS, bricks=BRICKS, group=None, flip=False, side=False,
          unit_width=None):
    """Rough stone blocks laid in courses, dirtier and darker the further away they are.

    A side wall is drawn with side set: it's a piece of a wall that runs on through every square,
    so the courses, the bricks and the light go on from the piece before it to the next. Flip is
    for one whose far end is at the start of its surface. A wall in front can carry on past its
    edges, if it is given the width of the wall as the unit width"""
    rng = random.Random(seed)
    img = bytearray(WIDTH * HEIGHT)
    group = group or str(seed)

    for (x, y), (u, v, h) in shape.cells.items():
        row = int(v * rows)
        along = (1.0 - u if flip else u) if side else u
        position = (d + along) * bricks if side else u * bricks
        bu = position + (0.5 if row & 1 else 0.0)
        col = math.floor(bu)
        shade, stain, moss, hue = brick_info(group, row, col)

        brick_h = h / rows
        fy = v * rows - row
        fx = bu - col
        brick_w = 1.0 / (shape.slope[x] * bricks) if side else (unit_width or shape.width) / bricks
        mortar = (brick_h >= 3 and fy * brick_h < 1.0) or (brick_w >= 3 and fx * brick_w < 1.0)

        light = depth_light(d + along if side else d) * tone
        light *= 1.0 - 0.45 * max(0.0, (0.12 - v) / 0.12)
        light *= 1.0 - 0.35 * max(0.0, (v - 0.85) / 0.15)
        if not side and 0.0 <= u <= 1.0:
            light *= 1.0 - 0.28 * max(0.0, (0.08 - u) / 0.08) - 0.28 * max(0.0, (u - 0.92) / 0.08)

        if mortar:
            color = scale(MORTAR, light * 1.5)
        else:
            color = (STONE[0] * (1 + 0.10 * hue), STONE[1], STONE[2] * (1 - 0.10 * hue))
            color = scale(color, (1 + shade) * light)
            if brick_h >= 4:
                if 1.0 <= fy * brick_h < 2.0:
                    color = scale(color, 1.18)
                elif fy > 1 - 1.5 / brick_h:
                    color = scale(color, 0.82)
            if stain > 0.78:
                color = scale(color, 0.8)
            if moss > 0.92 and v > 0.5:
                color = mix(color, scale(MOSS, light * 1.3), 0.65 * min(1.0, (v - 0.5) * 3))
            color = mix(color, scale((70, 62, 50), light), 0.35 * max(0.0, (v - 0.8) / 0.2))

        color = scale(color, 1.0 + rng.gauss(0, 0.045))
        put(img, palette, x, y, color)

    return img


def door(shape, palette, seed, d, planks=4):
    """A wooden door of planks with iron bands, inside a dark frame"""
    rng = random.Random(seed)
    img = bytearray(WIDTH * HEIGHT)
    light = 0.85 + 0.2 * depth_light(d)

    # a narrow door has no room for a frame at its sides
    roomy = shape.width >= 8

    for (x, y), (u, v, h) in shape.cells.items():
        if shape.border(x, y) and (roomy or not ((x - 1, y) not in shape.cells or (x + 1, y) not in shape.cells)):
            put(img, palette, x, y, scale((50, 34, 22), light))
            continue

        per_plank = max(shape.width / planks, 1.0)
        plank = int(u * planks)
        fu = (u * planks - plank) * per_plank
        grain = 0.07 * math.sin(y * 0.8 + plank * 2.7) + 0.05 * math.sin(y * 2.3 + x * 0.3 + plank)
        color = scale(WOOD, light * (1.0 + grain + (0.14 if plank & 1 else -0.08)))

        # the seams and the edges of the planks only show if they're wide enough
        if per_plank >= 3:
            if fu < 1.3:
                color = scale((46, 22, 10), light)
            elif fu < 2.4:
                color = scale(color, 1.2)

        ramps = ["wood"]
        for band in (0.2, 0.8):
            dist = abs(v - band)
            if dist < 0.045:
                color = scale((196, 196, 208), light)
                ramps = None
                if int(u * 14) & 1 and dist < 0.015 and h > 24:
                    color = scale((120, 120, 132), light)
            elif dist < 0.065:
                color = scale((52, 52, 60), light)
                ramps = None

        put(img, palette, x, y, scale(color, 1.0 + rng.gauss(0, 0.035)), ramps)

    return img


def wood_box(shape, palette, seed, d, lit=1.0, bands=True, lock=False):
    """A face of a chest"""
    rng = random.Random(seed)
    img = bytearray(WIDTH * HEIGHT)
    light = (0.7 + 0.3 * depth_light(d)) * lit

    for (x, y), (u, v, h) in shape.cells.items():
        if shape.border(x, y):
            put(img, palette, x, y, scale((40, 26, 14), light))
            continue
        color = scale(WOOD, light * (1.0 + 0.08 * math.sin(x * 1.9 + y * 0.4)))
        ramps = ["wood"]
        if bands and (abs(u - 0.2) < 0.05 or abs(u - 0.8) < 0.05):
            color = scale(IRON, light * 1.5)
            ramps = None
        if lock and abs(u - 0.5) < 0.06 and 0.2 < v < 0.6:
            color = scale(GOLD, light)
            ramps = None
        put(img, palette, x, y, scale(color, 1.0 + rng.gauss(0, 0.04)), ramps)

    return img


def tint(img, palette, other, mask_img=None):
    """Lays one picture over another"""
    for i, c in enumerate(other):
        if c:
            img[i] = c


def dark_stone(shape, palette, seed, d, amount=0.45):
    return stone(shape, palette, seed, d, tone=amount)


# ------------------------------------------------------------------- pieces

def make_pieces(tb, palette):
    """Every picture, as a dictionary of name to pixels"""
    t = tb.t
    pieces = {}
    seed = [100]

    def next_seed():
        seed[0] += 1
        return seed[0]

    for d in range(8):
        # the wall in front, which is the whole of the plane at that depth
        lx, rx, ty, by = t("LEFT_X", d), t("RIGHT_X", d), t("TOP_Y", d), t("BOTTOM_Y", d)
        shape = Shape()
        shape.add_columns(lx, rx, ty, ty, by + 1, by + 1)
        pieces["front_%d" % d] = stone(shape, palette, next_seed(), d, group="front_%d" % d)

        # a door in it
        lxn, rxn, tyn = t("LEFT_X_NEXT", d), t("RIGHT_X_NEXT", d), t("TOP_Y_NEXT", d)
        shape = Shape()
        shape.add_columns(lxn, rxn, tyn, tyn, by + 1, by + 1)
        pieces["frontdoor_%d" % d] = door(shape, palette, next_seed(), d)

    # standing in a doorway, with the wall all round it and the way through open
    lx, rx, ty, by = t("LEFT_X", 0), t("RIGHT_X", 0), t("TOP_Y", 0), t("BOTTOM_Y", 0)
    lxn, rxn, tyn = t("LEFT_X_NEXT", 0), t("RIGHT_X_NEXT", 0), t("TOP_Y_NEXT", 0)
    shape = Shape()
    shape.add_columns(lx, rx, ty, ty, by + 1, by + 1)
    for (x, y) in list(shape.cells):
        if lxn <= x <= rxn and tyn <= y <= by:
            del shape.cells[(x, y)]
    pieces["doorway"] = stone(shape, palette, next_seed(), 0)

    for d in range(7):
        lx, lxn = t("LEFT_X", d), t("LEFT_X_NEXT", d)
        rx, rxn = t("RIGHT_X", d), t("RIGHT_X_NEXT", d)
        ty, tyn = t("TOP_Y", d), t("TOP_Y_NEXT", d)
        by, byn = t("BOTTOM_Y", d), t("BOTTOM_Y_NEXT", d)

        near_l, far_l = 128 - lx, max(128 - lxn, 0.5)
        near_r, far_r = rx - 127, max(rxn - 127, 0.5)

        # the walls to the sides, which go from the near plane to the far one
        shape = Shape()
        shape.add_columns(lx, lxn, ty, tyn, by + 1, byn + 1, perspective=(near_l, far_l))
        pieces["left_%d" % d] = stone(shape, palette, 1, d, tone=1.0, group="left", side=True)
        shape = Shape()
        shape.add_columns(rxn, rx, tyn, ty, byn + 1, by + 1, perspective=(far_r, near_r))
        pieces["right_%d" % d] = stone(shape, palette, 2, d, tone=1.0, group="right", side=True, flip=True)

        # an opening at the side shows the far wall of the passage, and the edge of the wall in
        # front of it
        for side in ("left", "right"):
            near, far = (lx, lxn) if side == "left" else (rx, rxn)
            x0, x1 = min(near, far), max(near, far)
            img = bytearray(WIDTH * HEIGHT)

            far_wall = Shape()
            far_wall.add_columns(x0, x1, tyn, tyn, byn + 1, byn + 1)
            front_left, front_right = t("LEFT_X", d + 1), t("RIGHT_X", d + 1)
            front_width = front_right - front_left + 1
            for (x, y), (u, v, h) in list(far_wall.cells.items()):
                far_wall.cells[(x, y)] = ((x - front_left + 0.5) / front_width, v, h)
            wall = stone(far_wall, palette, next_seed(), d + 1, group="front_%d" % (d + 1),
                         unit_width=front_width)
            for (x, y) in far_wall.cells:
                img[y * WIDTH + x] = wall[y * WIDTH + x]

            # the edge nearest the party, which is lit
            jamb_x = near
            for y in range(ty, by + 1):
                if 0 <= jamb_x < WIDTH:
                    img[y * WIDTH + jamb_x] = palette.nearest(*scale((150, 142, 128), depth_light(d)))
            pieces["%sopen_%d" % (side, d)] = img

        # doors in the walls at the sides
        for side, xa, xb in (("left", "LEFT_DOOR_X", "LEFT_DOOR_X2"), ("right", "AHEAD_DOOR_X", "AHEAD_DOOR_X2")):
            x0, x1 = t(xa, d), t(xb, d)
            shape = Shape()
            if x0 == x1:
                pieces["%sdoor_%d" % (side, d)] = bytearray(WIDTH * HEIGHT)
                continue
            top0, top1 = tyn, t("DOOR_TOP_B", d)
            bot0, bot1 = t("DOOR_BOTTOM_A", d), t("DOOR_BOTTOM_B", d)
            near = 128 - x0 if side == "left" else x0 - 127
            far = max(128 - x1 if side == "left" else x1 - 127, 0.5)
            shape.add_columns(x0, x1, top0, top1, bot0 + 1, bot1 + 1,
                              perspective=(max(near, 0.5), far))
            pieces["%sdoor_%d" % (side, d)] = door(shape, palette, next_seed(), d)

        pieces["ladderup_%d" % d] = ladder(tb, palette, d, True, next_seed())
        pieces["ladderdown_%d" % d] = ladder(tb, palette, d, False, next_seed())
        pieces["chest_%d" % d] = chest(tb, palette, d, next_seed())

    return pieces


def ladder(tb, palette, d, up, seed):
    """A hole in the ceiling or the floor, with a ladder to it"""
    t = tb.t
    rng = random.Random(seed)
    img = bytearray(WIDTH * HEIGHT)
    near_y = t("RAIL_TOP_A" if up else "RAIL_BOTTOM_A", d)
    far_y = t("RAIL_TOP_B" if up else "RAIL_BOTTOM_B", d)
    left_near, left_far = t("LADDER_LEFT_A", d), t("LADDER_LEFT_B", d)
    right_far, right_near = t("LADDER_RIGHT_A", d), t("LADDER_RIGHT_B", d)

    hole = Shape()
    hole.add_polygon([(left_near, near_y), (left_far, far_y), (right_far, far_y), (right_near, near_y)])
    hole_light = depth_light(d)
    for (x, y), (u, v, h) in hole.cells.items():
        edge = hole.border(x, y)
        c = scale((150, 142, 128), hole_light * (0.6 if edge else 1.0)) if edge else             scale((16, 14, 22), 1.0 - 0.4 * v)
        put(img, palette, x, y, scale(c, 1.0 + rng.gauss(0, 0.04)))

    # the ladder is two rails with rungs between them, from the floor or the ceiling into the
    # hole, as far as the line round the end of it
    hole_rows = [y for (x, y) in hole.cells]
    top, bottom = t("RAIL_TOP_A", d), t("RAIL_BOTTOM_A", d)
    gap = max(1.0, 9.0 / (2 ** d))
    rail_w = max(1, round(3.0 / (2 ** d)) or 1)
    if up:
        y0, y1 = (min(hole_rows) + 1 if hole_rows else far_y), bottom
    else:
        y0, y1 = top, (max(hole_rows) - 1 if hole_rows else near_y)
    if y1 <= y0:
        return img

    for rail in (127.5 - gap, 127.5 + gap):
        for y in range(int(y0), int(y1) + 1):
            for w in range(rail_w):
                x = int(round(rail)) - rail_w // 2 + w
                if 0 <= x < WIDTH and 0 <= y < HEIGHT:
                    put(img, palette, x, y, scale(WOOD, depth_light(d) * (1.0 + rng.gauss(0, 0.05))), ["wood"])

    count = max(2, round((y1 - y0) / max(3.0, gap * 1.6)))
    for i in range(count):
        y = int(y0 + (i + 0.5) * (y1 - y0) / count)
        for x in range(int(round(127.5 - gap)) + rail_w, int(round(127.5 + gap)) - rail_w + 1):
            if 0 <= x < WIDTH and 0 <= y < HEIGHT:
                put(img, palette, x, y, scale((154, 108, 62), depth_light(d)), ["wood"])

    return img


def chest(tb, palette, d, seed):
    """A chest on the floor: the front, the lid and the side"""
    t = tb.t
    lx2, lx3 = t("LEFT_X_NEXT2", d), t("LEFT_X_NEXT3", d)
    rx2, rx3 = t("RIGHT_X_NEXT2", d), t("RIGHT_X_NEXT3", d)
    cb, dba, dbb = t("CHEST_BOTTOM", d), t("DOOR_BOTTOM_A", d), t("DOOR_BOTTOM_B", d)

    img = bytearray(WIDTH * HEIGHT)

    def paint(points, lit, lock=False, bands=True):
        shape = Shape()
        shape.add_polygon(points)
        part = wood_box(shape, palette, seed, d, lit, bands, lock)
        for (x, y) in shape.cells:
            img[y * WIDTH + x] = part[y * WIDTH + x]

    paint([(rx3, cb), (rx2, dbb), (rx2, cb), (rx3, dba)], 0.55, bands=False)
    paint([(lx2, cb), (rx3, cb), (rx2, dbb), (lx3, dbb)], 1.15, bands=True)
    paint([(lx2, cb), (rx3, cb), (rx3, dba), (lx2, dba)], 0.9, lock=True)
    return img


# -------------------------------------------------------------- the preview

def preview(pieces, palette, layout, px, py, fx, fy, out):
    """Draws the corridor the way the game does, with these pictures, to see how they look"""
    def cell(x, y):
        if not (0 <= x < 16 and 0 <= y < 16):
            return 0x80
        return layout[y][x]

    MAX = 8
    left, center, right = [0] * MAX, [0] * MAX, [0] * MAX
    lx, ly = px + fy, py - fx
    cx, cy = px, py
    rx, ry = px - fy, py + fx
    for d in range(MAX):
        left[d] = cell(lx, ly) & 0xF0
        lx += fx
        ly += fy
        center[d] = cell(cx, cy) & 0xF0
        cx += fx
        cy += fy
        right[d] = cell(rx, ry) & 0xF0
        rx += fx
        ry += fy

    buf = bytearray(WIDTH * HEIGHT)
    deferred = []

    def blit(name):
        img = pieces.get(name)
        if img:
            for i, c in enumerate(img):
                if c:
                    buf[i] = c

    d = 0
    while True:
        c = center[d]
        if c & 0x80:
            if (c & 0x40) == 0 or (c & 0x20) != 0:
                blit("front_%d" % d)
                break
            if d == 0:
                blit("doorway")
                d += 1
                continue
            blit("front_%d" % d)
            blit("frontdoor_%d" % d)
            break

        if c & 0x20:
            deferred.append("ladderdown_%d" % d)
        if c & 0x10:
            deferred.append("ladderup_%d" % d)
        elif c & 0x40:
            deferred.append("chest_%d" % d)

        for side, v in (("left", left[d]), ("right", right[d])):
            if v & 0x80:
                blit("%s_%d" % (side, d))
                if (v & 0x40) and not (v & 0x20):
                    blit("%sdoor_%d" % (side, d))
            else:
                blit("%sopen_%d" % (side, d))

        d += 1
        if d >= MAX - 1:
            break

    for name in reversed(deferred):
        blit(name)

    rgb = bytearray()
    for i in range(WIDTH * HEIGHT):
        rgb += bytes(palette.colors[buf[i]])
    return rgb


def sample_layout():
    """A dungeon with a corridor, side openings, doors, a ladder and a chest"""
    g = [[0x80] * 16 for _ in range(16)]
    for x in range(1, 15):
        g[7][x] = 0
    for y in range(2, 14):
        g[y][4] = 0
        g[y][10] = 0
    g[7][3] = 0x00
    g[7][12] = 0x40      # a chest
    g[7][9] = 0x10       # ladder up
    g[7][6] = 0x20       # ladder down
    g[6][7] = 0xC0       # a door in the side wall
    g[8][2] = 0xC0
    g[7][14] = 0xC0      # a door at the end
    return g


def enlarge(rgb, w, h, k):
    out = bytearray()
    for y in range(h):
        row = bytearray()
        for x in range(w):
            row += bytes(rgb[(y * w + x) * 3:(y * w + x) * 3 + 3]) * k
        out += bytes(row) * k
    return out


def main():
    source, outdir = sys.argv[1], sys.argv[2]
    tb = Tables(source)
    palette = Palette()
    os.makedirs(outdir, exist_ok=True)

    pieces = make_pieces(tb, palette)
    for name, img in pieces.items():
        write_indexed(os.path.join(outdir, name + ".png"), img, WIDTH, HEIGHT, palette)
    palette.write(os.path.join(outdir, "palette.pal"))

    if len(sys.argv) > 3:
        layout = sample_layout()
        shots = [preview(pieces, palette, layout, x, y, fx, fy, None)
                 for (x, y, fx, fy) in [(1, 7, 1, 0), (8, 7, 1, 0), (14, 7, -1, 0), (3, 7, 1, 0)]]
        k = 2
        w = WIDTH * 2
        rgb = bytearray()
        for row in range(2):
            for y in range(HEIGHT):
                for col in range(2):
                    s = shots[row * 2 + col]
                    rgb += s[y * WIDTH * 3:(y + 1) * WIDTH * 3]
        write_rgb(sys.argv[3], enlarge(rgb, w, HEIGHT * 2, k), w * k, HEIGHT * 2 * k)


if __name__ == "__main__":
    main()
