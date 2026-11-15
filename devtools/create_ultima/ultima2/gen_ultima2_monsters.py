#!/usr/bin/env python3
"""
Generates the stand-in pictures of the monsters in the dungeons of Ultima II, for the "textured
dungeon" renderer that is used with the EGA graphics modes.

There is a picture of each of the 7 kinds of monster at each of the depths 1 to 7 down the
corridor, named monster_<kind>_<depth>.png. Each is a 256x128 layer in the geometry of the
corridor, like the pictures of the walls, and uses the same standard colors (palette.pal, which
gen_ultima2_dungeons.py writes). The monsters stand in the middle of the corridor, with their
feet on the floor of the depth they're at, and are as tall as the corridor is there.

The kinds, which match the order in the game, are:
  1 imp, 2 ghost, 3 jellyfish, 4 serpent, 5 gremlin, 6 horned demon, 7 winged demon

These are stand-in pictures made by a program, written with the help of an AI assistant (Claude).
They are meant to be replaced by hand drawn art: any picture can be swapped for one of the same
name, size and palette, and the game falls back to its blocky sprites for any that are missing.

Usage: gen_ultima2_monsters.py <dungeon_map.cpp> <output folder> [<preview.png>]
"""
import math
import os
import sys

import gen_ultima2_dungeons as g

WIDTH, HEIGHT = g.WIDTH, g.HEIGHT
LIGHT = (-0.45, -0.6, 0.66)
SUPERSAMPLE = 4

# ------------------------------------------------------------------- shapes
# Everything is designed in a box 100 wide and 100 high, with the feet at the bottom in the middle


class Part:
    """A piece of a monster, in the ramp of colors it's painted from"""

    def __init__(self, ramp, base, gloss=0.0, glow=False):
        self.ramp = ramp
        self.base = base
        self.gloss = gloss
        self.glow = glow

    def hit(self, x, y):
        """Returns the normal of the surface if the point is on the part, or None"""
        raise NotImplementedError


class Ellipse(Part):
    def __init__(self, cx, cy, rx, ry, ramp, base, **kw):
        super().__init__(ramp, base, **kw)
        self.cx, self.cy, self.rx, self.ry = cx, cy, rx, ry

    def hit(self, x, y):
        dx, dy = (x - self.cx) / self.rx, (y - self.cy) / self.ry
        r2 = dx * dx + dy * dy
        if r2 > 1.0:
            return None
        return (dx, dy, math.sqrt(1.0 - r2))


class Capsule(Part):
    """A line with a round end at each, which can be thicker at one end"""

    def __init__(self, x1, y1, x2, y2, r1, r2, ramp, base, **kw):
        super().__init__(ramp, base, **kw)
        self.x1, self.y1, self.x2, self.y2, self.r1, self.r2 = x1, y1, x2, y2, r1, r2

    def hit(self, x, y):
        vx, vy = self.x2 - self.x1, self.y2 - self.y1
        length2 = vx * vx + vy * vy
        t = 0.0 if length2 == 0 else max(0.0, min(1.0, ((x - self.x1) * vx + (y - self.y1) * vy) / length2))
        qx, qy = self.x1 + vx * t, self.y1 + vy * t
        r = self.r1 + (self.r2 - self.r1) * t
        dx, dy = x - qx, y - qy
        d2 = dx * dx + dy * dy
        if d2 > r * r:
            return None
        return (dx / r, dy / r, math.sqrt(max(0.0, 1.0 - d2 / (r * r))))


class Poly(Part):
    """A flat piece with any outline, lit a little more at the top"""

    def __init__(self, points, ramp, base, **kw):
        super().__init__(ramp, base, **kw)
        self.points = points
        xs = [p[0] for p in points]
        ys = [p[1] for p in points]
        self.cx, self.cy = (min(xs) + max(xs)) / 2, (min(ys) + max(ys)) / 2
        self.w, self.h = max(max(xs) - min(xs), 1), max(max(ys) - min(ys), 1)

    def hit(self, x, y):
        inside = False
        pts = self.points
        j = len(pts) - 1
        for i in range(len(pts)):
            xi, yi = pts[i]
            xj, yj = pts[j]
            if (yi > y) != (yj > y) and x < (xj - xi) * (y - yi) / (yj - yi) + xi:
                inside = not inside
            j = i
        if not inside:
            return None
        nx, ny = 0.5 * (x - self.cx) / self.w, 0.5 * (y - self.cy) / self.h
        return (nx, ny, math.sqrt(max(0.0, 1.0 - nx * nx - ny * ny)))


def mirror(parts_x):
    """The reflection of a point list across the middle"""
    return [(100 - x, y) for (x, y) in parts_x]


# ---------------------------------------------------------------- the monsters

RED, FLESH, GOLD = "red", "flesh", "gold"
BOOT = (130, 80, 40)


def imp():
    p = []
    base = (190, 40, 30)
    horn = (240, 232, 190)
    # the tail, behind
    p += [Capsule(60, 84, 78, 82, 3.2, 2.6, "red", base),
          Capsule(78, 82, 86, 66, 2.6, 2.2, "red", base),
          Poly([(84, 66), (92, 56), (90, 70)], "red", (120, 20, 16))]
    for sx in (1, -1):
        def X(v, s=sx):
            return 50 + (v - 50) * s
        p += [Capsule(X(44), 84, X(42), 98, 6.5, 5.0, "red", base),
              Ellipse(X(39), 98, 7.5, 3.4, "red", (130, 22, 18)),
              Capsule(X(38), 58, X(20), 74, 5.2, 3.6, "red", base),
              Ellipse(X(18), 76, 4.5, 3.2, "gold", (210, 180, 60))]
    p += [Ellipse(50, 70, 17, 21, "red", base),
          Ellipse(50, 75, 10, 12, "flesh", (232, 150, 120)),
          Ellipse(50, 42, 16, 15, "red", base)]
    for sx in (1, -1):
        def X(v, s=sx):
            return 50 + (v - 50) * s
        p += [Capsule(X(40), 32, X(33), 14, 4.4, 1.4, "pale", horn),
              Poly([(X(37), 42), (X(22), 36), (X(35), 50)], "red", (150, 30, 24)),
              Ellipse(X(43), 41, 4.0, 4.6, "pale", (250, 240, 150), glow=True),
              Ellipse(X(43), 42, 1.5, 2.6, "red", (30, 0, 0))]
    p += [Poly([(41, 53), (59, 53), (56, 60), (50, 57), (44, 60)], "red", (40, 4, 4)),
          Poly([(43, 53), (45, 57), (47, 53)], "pale", (250, 250, 240)),
          Poly([(53, 53), (55, 57), (57, 53)], "pale", (250, 250, 240))]
    return p


def ghost():
    p = []
    base = (236, 230, 150)
    cloak = [(34, 30), (66, 30), (80, 100), (72, 90), (64, 100), (56, 90), (48, 100), (40, 90), (32, 100), (22, 98)]
    p += [Poly(cloak, "pale", base),
          Ellipse(50, 30, 19, 22, "pale", base)]
    for sx in (1, -1):
        def X(v, s=sx):
            return 50 + (v - 50) * s
        p += [Capsule(X(30), 52, X(14), 76, 6, 4, "pale", base),
              Ellipse(X(13), 78, 4.5, 3.5, "pale", (250, 250, 220))]
    p += [Ellipse(50, 34, 12, 14, "iron", (22, 22, 34)),
          Ellipse(45, 33, 3.2, 2.4, "pale", (255, 255, 235), glow=True),
          Ellipse(55, 33, 3.2, 2.4, "pale", (255, 255, 235), glow=True),
          Ellipse(50, 42, 3.5, 4.5, "iron", (6, 6, 14))]
    return p


def jellyfish():
    p = []
    blue, green = (60, 100, 220), (50, 180, 70)
    for i in range(7):
        x0 = 24 + i * 8.7
        sw = 4 if i % 2 else -4
        p += [Capsule(x0, 52, x0 + sw, 70, 4.0, 3.2, "blue", blue),
              Capsule(x0 + sw, 70, x0 - sw * 0.5, 86, 3.2, 2.4, "blue", blue),
              Capsule(x0 - sw * 0.5, 86, x0 + sw * 0.8, 99, 2.4, 1.4, "green", green)]
    p += [Ellipse(50, 34, 28, 30, "blue", blue)]
    for x in (34, 42, 50, 58, 66):
        p += [Capsule(x, 12 + abs(x - 50) * 0.5, x, 52 - abs(x - 50) * 0.3, 2.2, 2.2, "green", green)]
    for sx in (1, -1):
        def X(v, s=sx):
            return 50 + (v - 50) * s
        p += [Ellipse(X(41), 44, 6.5, 7.5, "pale", (250, 250, 230), gloss=0.6),
              Ellipse(X(41), 45, 3, 4, "green", (20, 120, 40))]
    p += [Poly([(44, 56), (56, 56), (50, 62)], "iron", (20, 20, 40))]
    return p


def serpent():
    p = []
    red = (190, 40, 30)
    spine = [(14, 94, 7), (28, 90, 7.5), (42, 94, 8), (56, 90, 8.5), (68, 78, 8.5), (66, 62, 8.5),
             (54, 54, 8.5), (46, 44, 8.5)]
    for i in range(len(spine) - 1):
        x1, y1, r1 = spine[i]
        x2, y2, r2 = spine[i + 1]
        p.append(Capsule(x1, y1, x2, y2, r1, r2, "red", red))
        # stripes across the body
        mx, my = (x1 + x2) / 2, (y1 + y2) / 2
        p.append(Ellipse(mx, my, 2.4, (r1 + r2) / 2 * 0.9, "red", (110, 14, 10)))
    p += [Ellipse(47, 36, 15, 11, "red", red),
          Poly([(35, 40), (61, 40), (63, 48), (51, 45), (37, 50)], "iron", (24, 4, 4)),
          Poly([(39, 42), (42, 50), (45, 42)], "pale", (250, 250, 240)),
          Poly([(53, 42), (56, 50), (59, 42)], "pale", (250, 250, 240)),
          Ellipse(53, 31, 3.4, 3.4, "pale", (250, 235, 110), glow=True),
          Ellipse(53, 31, 1.2, 2.4, "red", (20, 0, 0)),
          Ellipse(41, 31, 3.4, 3.4, "pale", (250, 235, 110), glow=True),
          Ellipse(41, 31, 1.2, 2.4, "red", (20, 0, 0)),
          Capsule(61, 45, 77, 48, 1.6, 1.2, "red", (255, 90, 80)),
          Poly([(77, 48), (85, 44), (83, 48), (85, 52)], "red", (255, 90, 80))]
    return p


def gremlin():
    p = []
    base = (190, 40, 30)
    for sx in (1, -1):
        def X(v, s=sx):
            return 50 + (v - 50) * s
        p += [Capsule(X(43), 82, X(41), 98, 5.5, 4.5, "red", base),
              Ellipse(X(38), 98, 6.5, 3.2, "red", (130, 22, 18)),
              Capsule(X(40), 62, X(22), 52, 4.6, 3.2, "red", base),
              Ellipse(X(20), 50, 4.0, 3.5, "gold", (210, 180, 60)),
              Poly([(X(38), 56), (X(14), 54), (X(32), 66)], "red", (150, 30, 24))]
    p += [Ellipse(50, 72, 14, 18, "red", base),
          Ellipse(50, 76, 8, 10, "flesh", (232, 150, 120)),
          Ellipse(50, 50, 14, 13, "red", base)]
    for sx in (1, -1):
        def X(v, s=sx):
            return 50 + (v - 50) * s
        p += [Poly([(X(38), 50), (X(24), 38), (X(32), 56)], "red", (150, 30, 24)),
              Ellipse(X(44), 49, 4.4, 5.0, "pale", (250, 250, 240)),
              Ellipse(X(44), 50, 1.8, 2.6, "iron", (10, 10, 20))]
    p += [Poly([(42, 58), (58, 58), (54, 64), (50, 62), (46, 64)], "iron", (36, 4, 4)),
          Poly([(44, 58), (46, 61), (48, 58)], "pale", (250, 250, 240)),
          Poly([(52, 58), (54, 61), (56, 58)], "pale", (250, 250, 240))]
    # the jester's hat, with bells
    p += [Poly([(36, 42), (64, 42), (58, 24), (50, 32), (42, 18)], "pale", (240, 230, 110)),
          Ellipse(42, 17, 3.2, 3.2, "gold", (230, 190, 60), gloss=0.7),
          Ellipse(60, 23, 3.2, 3.2, "gold", (230, 190, 60), gloss=0.7)]
    # sparks of magic
    for (x, y, ramp, c) in ((20, 22, "green", (60, 220, 90)), (80, 24, "gold", (255, 230, 90)),
                            (14, 40, "purple", (210, 110, 240)), (84, 42, "cyan", (110, 240, 250))):
        p += [Poly([(x, y - 5), (x + 2.2, y), (x, y + 5), (x - 2.2, y)], ramp, c, glow=True),
              Poly([(x - 5, y), (x, y - 2.2), (x + 5, y), (x, y + 2.2)], ramp, c, glow=True)]
    return p


def horned_demon():
    p = []
    base = (190, 40, 30)
    horn = (60, 100, 230)
    for sx in (1, -1):
        def X(v, s=sx):
            return 50 + (v - 50) * s
        p += [Capsule(X(40), 70, X(38), 92, 8.5, 6.0, "red", base),
              Ellipse(X(36), 96, 10, 5, "wood", BOOT),
              Capsule(X(32), 42, X(14), 62, 8.0, 6.0, "red", base),
              Ellipse(X(12), 66, 6.5, 6.5, "red", (150, 30, 24))]
    p += [Ellipse(50, 56, 23, 28, "red", base),
          Ellipse(50, 22, 14, 14, "red", base)]
    for sx in (1, -1):
        def X(v, s=sx):
            return 50 + (v - 50) * s
        p += [Capsule(X(41), 14, X(30), -2, 4.5, 1.2, "blue", horn),
              Ellipse(X(44), 21, 4.2, 3.4, "pale", (250, 235, 100), glow=True),
              Ellipse(X(44), 21.5, 1.4, 2.2, "red", (20, 0, 0))]
    p += [Poly([(41, 30), (59, 30), (55, 38), (50, 34), (45, 38)], "iron", (36, 4, 4)),
          Poly([(43, 30), (45, 34), (47, 30)], "pale", (250, 250, 240)),
          Poly([(53, 30), (55, 34), (57, 30)], "pale", (250, 250, 240))]
    return p


def winged_demon():
    p = []
    base = (190, 40, 30)
    horn = (60, 100, 230)
    wing = (120, 20, 28)
    left = [(36, 40), (2, 10), (12, 34), (4, 44), (16, 52), (12, 64), (30, 56), (38, 66)]
    p += [Poly(left, "red", wing), Poly(mirror(left), "red", wing)]
    for sx in (1, -1):
        def X(v, s=sx):
            return 50 + (v - 50) * s
        p += [Capsule(X(36), 42, X(4), 12, 1.6, 1.2, "flesh", (226, 120, 100)),
              Capsule(X(36), 46, X(8), 46, 1.4, 1.2, "flesh", (226, 120, 100)),
              Capsule(X(38), 56, X(14), 62, 1.4, 1.2, "flesh", (226, 120, 100))]
    for sx in (1, -1):
        def X(v, s=sx):
            return 50 + (v - 50) * s
        p += [Capsule(X(41), 72, X(39), 93, 7.5, 5.5, "red", base),
              Ellipse(X(37), 97, 9, 4.2, "wood", BOOT),
              Capsule(X(33), 44, X(22), 60, 6.5, 5.0, "red", base)]
    p += [Ellipse(50, 58, 19, 26, "red", base),
          Ellipse(50, 26, 12, 12, "red", base)]
    for sx in (1, -1):
        def X(v, s=sx):
            return 50 + (v - 50) * s
        p += [Capsule(X(42), 19, X(32), 4, 4.0, 1.2, "blue", horn),
              Ellipse(X(45), 25, 3.6, 3.0, "pale", (250, 235, 100), glow=True)]
    # a staff in one hand and a blade in the other
    p += [Capsule(15, 30, 15, 98, 1.8, 1.8, "wood", (150, 86, 42)),
          Ellipse(15, 26, 5, 5, "cyan", (80, 230, 240), glow=True, gloss=0.8),
          Capsule(23, 62, 16, 62, 4.6, 4.0, "red", base),
          Capsule(85, 62, 85, 58, 3.6, 3.2, "red", base),
          Capsule(85, 56, 80, 14, 2.0, 1.2, "iron", (200, 205, 220), gloss=0.9),
          Poly([(80, 56), (92, 56), (90, 60), (82, 60)], "gold", (230, 190, 60))]
    p += [Poly([(44, 33), (56, 33), (53, 40), (50, 37), (47, 40)], "iron", (36, 4, 4))]
    return p


MONSTERS = [imp, ghost, jellyfish, serpent, gremlin, horned_demon, winged_demon]


# -------------------------------------------------------------------- render

def render(parts, palette, size, feet_y, centre_x, seed=0):
    """Draws a monster so that it's size pixels tall, with its feet on a line"""
    size = max(int(round(size)), 1)
    scale = size / 100.0
    left = int(round(centre_x - 50 * scale))
    top = int(round(feet_y - 100 * scale))
    w = max(int(math.ceil(100 * scale)), 1)
    h = size
    S = SUPERSAMPLE if size >= 6 else 8
    lx, ly, lz = LIGHT
    norm = math.sqrt(lx * lx + ly * ly + lz * lz)
    lx, ly, lz = lx / norm, ly / norm, lz / norm

    # the color and part of each sample
    pixels = {}
    for py in range(h):
        for px in range(w):
            hits = 0
            sr = sg = sb = 0.0
            ramp_votes = {}
            for sy in range(S):
                for sx in range(S):
                    x = (px + (sx + 0.5) / S) / scale
                    y = (py + (sy + 0.5) / S) / scale
                    found = None
                    for part in reversed(parts):
                        n = part.hit(x, y)
                        if n is not None:
                            found = (part, n)
                            break
                    if not found:
                        continue

                    part, (nx, ny, nz) = found
                    diffuse = max(0.0, nx * lx + ny * ly + nz * lz)
                    lum = 0.32 + 0.85 * diffuse
                    if part.glow:
                        lum = max(lum, 0.95)
                    r, gg, b = (c * lum for c in part.base)
                    if part.gloss:
                        spec = max(0.0, (nx * -0.3 + ny * -0.4 + nz * 0.85)) ** 14 * part.gloss
                        r, gg, b = r + 255 * spec, gg + 255 * spec, b + 255 * spec
                    sr, sg, sb = sr + r, sg + gg, sb + b
                    hits += 1
                    ramp_votes[part.ramp] = ramp_votes.get(part.ramp, 0) + 1

            if hits * 2 >= S * S:
                ramp = max(ramp_votes, key=ramp_votes.get)
                pixels[(px, py)] = (palette.nearest(sr / hits, sg / hits, sb / hits, ramps=[ramp]), ramp)

    # a dark edge round the outside, where it's big enough to see
    img = bytearray(WIDTH * HEIGHT)
    for (px, py), (c, ramp) in pixels.items():
        x, y = left + px, top + py
        if not (0 <= x < WIDTH and 0 <= y < HEIGHT):
            continue
        if size >= 20 and any((px + dx, py + dy) not in pixels for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))):
            c = palette.ramps[ramp][1]
        img[y * WIDTH + x] = c
    return img


def main():
    source, outdir = sys.argv[1], sys.argv[2]
    tb = g.Tables(source)
    palette = g.Palette()
    os.makedirs(outdir, exist_ok=True)

    images = {}
    for kind, design in enumerate(MONSTERS, 1):
        parts = design()
        for depth in range(1, 8):
            # as tall as the corridor is at that depth, standing on its floor
            height = tb.t("BOTTOM_Y", depth) - tb.t("TOP_Y", depth) + 1
            img = render(parts, palette, max(2.0, height * 0.9), tb.t("BOTTOM_Y", depth) + 1, 128)
            images["monster_%d_%d" % (kind, depth)] = img

    for name, img in images.items():
        g.write_indexed(os.path.join(outdir, name + ".png"), img, WIDTH, HEIGHT, palette)

    if len(sys.argv) > 3:
        # the monsters at the first depths, side by side, with the gaps cropped
        cols = 7
        crop_w, crop_h = 90, 80
        rgb = bytearray(crop_w * cols * crop_h * 3 * 3)
        for kind in range(1, 8):
            for row, depth in enumerate((1, 2, 3)):
                img = images["monster_%d_%d" % (kind, depth)]
                for y in range(crop_h):
                    for x in range(crop_w):
                        sx, sy = 128 - crop_w // 2 + x, 96 - crop_h + y + (0 if depth == 1 else 0)
                        c = img[sy * WIDTH + sx] if 0 <= sy < HEIGHT else 0
                        col = palette.colors[c] if c else (36, 36, 44)
                        o = ((row * crop_h + y) * crop_w * cols + (kind - 1) * crop_w + x) * 3
                        rgb[o:o + 3] = bytes(col)
        k = 3
        write = g.enlarge(bytes(rgb), crop_w * cols, crop_h * 3, k)
        g.write_rgb(sys.argv[3], write, crop_w * cols * k, crop_h * 3 * k)


if __name__ == "__main__":
    main()
