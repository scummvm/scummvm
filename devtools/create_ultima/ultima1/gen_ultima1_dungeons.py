#!/usr/bin/env python3
"""
Generates the stand-in pictures for the Ultima I "textured dungeon" renderer, which is used with
the EGA graphics.

The EGA graphics only use 16 colors, in palette entries 0 to 15, and the others in the 256 color
palette are free. This uses the same standard set of colors for entries 48 to 255 as the pictures
of the Ultima II dungeons (see gen_ultima2_dungeons.py, which this builds on, and the palette.pal
that's written with the pictures, which the game loads). The pictures only use those, and palette
index 0 is transparent.

Every picture is a 288x144 layer, which the game draws at (16, 8) on the screen, in the geometry
of the original wire frame view of the dungeon, which is worked out from the same tables as the
game's (views/dungeon_map.cpp). The wire frame draws the planes of the corridor at 6 distances
from the party, 0 being right at the party, so the pictures of the parts of the corridor are:

  front_D        the wall that faces the party at plane D
  doorway_0      the wall seen from a doorway that the party is standing in, with the way through
  frontdoor_D    a door in the wall facing the party at plane D
  left_D/right_D the wall at a side, between plane D-1 and plane D
  leftopen_D/rightopen_D   an opening at the side, between plane D-1 and plane D
  leftedge_D/rightedge_D   the corner where a wall at the side starts or stops, at plane D
  leftdoor_D/rightdoor_D   a door in the wall at a side
  beams_D        the beams across the ceiling

and the things in it are drawn at one of the sizes that the game draws its shapes (tier 0 is the
biggest, and half the size of the one before it at each of the others):

  ladderuphole_T/ladderdownhole_T   the hole in the ceiling or the floor
  ladderupfaceon_T/ladderdownfaceon_T   a ladder up through the ceiling, or down through the
                 floor, seen from the end of its rungs
  ladderupsideways_T/ladderdownsideways_T   the same, with its rungs edge-on
  chest_T, coffin_T

The gelatinous cube, which fills the corridor, is written as monsters/monster_9_D, along with the
monster pictures made by gen_ultima1_monsters.py.

These are stand-in pictures made by a program, written with the help of an AI assistant (Claude).
They are meant to be replaced by hand drawn art: any picture can be swapped for one of the same
name, size and palette, and the game falls back to its lines for any that are missing.

Usage: gen_ultima1_dungeons.py <output folder> [<preview.png>]
"""
import math
import os
import random
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "ultima2"))
import gen_ultima2_dungeons as g

WIDTH, HEIGHT = 288, 144
g.WIDTH, g.HEIGHT = WIDTH, HEIGHT
g.CENTER = WIDTH / 2

RIGHT, BOTTOM = WIDTH - 1, HEIGHT - 1

# The perspective tables of the game (DUNGEON_ARRAY_X and DUNGEON_ARRAY_Y), which give the distance
# of each plane from the edge of the view and from its top
ARRAY_X = [0, 72, 108, 126, 135, 144]
ARRAY_Y = [0, 36, 54, 63, 68, 72]

# Where the floor is for each of the sizes that the game draws its shapes, in rows of the pictures
FLOOR_Y = [139 - 8, 112 - 8, 96 - 8, 88 - 8, 84 - 8]
TIERS = len(FLOOR_Y)


def trunc_div(a, b):
    """Divides the way that C does, which goes to zero"""
    q = abs(a) // abs(b)
    return q if (a < 0) == (b < 0) else -q


# ------------------------------------------------------------------- the planes

def plane(d):
    """The left, right, top and bottom of the wall that faces the party at a plane"""
    if d == 0:
        # The original inset this one, to leave room for the frame of the view
        return 8, RIGHT - 8, 8, BOTTOM - 8
    return ARRAY_X[d], RIGHT - ARRAY_X[d], ARRAY_Y[d], BOTTOM - ARRAY_Y[d]


def far_distance(d):
    return max(g.CENTER - ARRAY_X[d], 0.5)


def front_wall(palette, seed, d):
    left, right, top, bottom = plane(d)
    shape = g.Shape()
    shape.add_columns(left, right, top, top, bottom, bottom)
    return shape, g.stone(shape, palette, seed, d, group="front_%d" % d)


def beyond_front(shape, d):
    """Gives a piece the places on the wall in front at a plane that it carries on to, so what's
    at the side of the wall in front is the same wall"""
    left, right, _, _ = plane(d)
    width = right - left + 1
    for (x, y), (u, v, h) in list(shape.cells.items()):
        shape.cells[(x, y)] = ((x + 0.5 - left) / width, v, h)
    return width


def side_door(palette, seed, d, left):
    """A door in the wall at a side, which is where the game's lines for one put it"""
    diff_x = abs(trunc_div(ARRAY_X[d - 1] - ARRAY_X[d], 9))
    diff_y = abs(trunc_div(ARRAY_Y[d - 1] - ARRAY_Y[d], 5))

    near_x = ARRAY_X[d - 1] + diff_x * 2
    far_x = ARRAY_X[d - 1] + diff_x * 6
    near_bottom = BOTTOM - 1 - diff_y - ARRAY_Y[d - 1]
    near_top = ARRAY_Y[d] - diff_y
    far_top = ARRAY_Y[d] + diff_y
    far_bottom = BOTTOM - ARRAY_Y[d] + diff_y * 2 - (2 if d == 1 else 0)

    shape = g.Shape()
    if left:
        near_d, far_d = g.CENTER - near_x, g.CENTER - far_x
        shape.add_columns(near_x, far_x, near_top, far_top, near_bottom, far_bottom,
                          perspective=(max(near_d, 0.5), max(far_d, 0.5)))
    else:
        near_x, far_x = RIGHT - near_x, RIGHT - far_x
        near_d, far_d = near_x - g.CENTER, far_x - g.CENTER
        shape.add_columns(far_x, near_x, far_top, near_top, far_bottom, near_bottom,
                          perspective=(max(far_d, 0.5), max(near_d, 0.5)))

    if len(set(x for (x, _) in shape.cells)) < 3:
        return None
    return g.door(shape, palette, seed, d - 1)


def beams(palette, seed, d):
    """Beams across the ceiling, at the heights that the game puts its lines"""
    img = bytearray(WIDTH * HEIGHT)
    rng = random.Random(seed)
    height = abs(trunc_div(2 * ARRAY_Y[d] - 143, 4))
    left, right = ARRAY_X[d], RIGHT - ARRAY_X[d]
    thick = max(1, 8 >> d)

    for k in (1, 2, 3):
        y = ARRAY_Y[d] + height * k + d // 2
        for row in range(thick):
            t = row / max(thick - 1, 1)
            for x in range(left, right + 1):
                if 0 <= y + row < HEIGHT:
                    shade = 1.15 - 0.55 * t + rng.gauss(0, 0.04)
                    img[(y + row) * WIDTH + x] = palette.nearest(*g.scale(g.WOOD, shade * (0.75 + 0.25 * g.depth_light(d))),
                                                                  ramps=["wood"])
    return img


def edge(palette, d, left):
    """The corner where the wall at a side starts or stops"""
    img = bytearray(WIDTH * HEIGHT)
    x = ARRAY_X[d] if left else RIGHT - ARRAY_X[d]
    color = palette.nearest(*g.scale((150, 142, 128), g.depth_light(d)))
    thick = 2 if d <= 1 else 1
    for k in range(thick):
        xx = x + k if left else x - k
        for y in range(ARRAY_Y[d], BOTTOM - ARRAY_Y[d] + 1):
            if 0 <= xx < WIDTH and 0 <= y < HEIGHT:
                img[y * WIDTH + xx] = color
    return img


# -------------------------------------------------------------------- widgets

def wscale(tier):
    """How much the game shrinks its shapes at a tier"""
    return 1.0 / (1 << (5 if tier == 4 else tier))


def wpoint(tier, x, y):
    return WIDTH / 2 + x * wscale(tier), FLOOR_Y[tier] + y * wscale(tier)


def hole(img, palette, tier, up, rng):
    """The hole in the ceiling or the floor that a ladder goes through"""
    if up:
        points = [(-70, -120), (70, -120), (40, -105), (-40, -105)]
    else:
        points = [(-70, 0), (70, 0), (40, -15), (-40, -15)]

    shape = g.Shape()
    shape.add_polygon([wpoint(tier, x, y) for (x, y) in points])
    light = g.depth_light(tier)
    for (x, y), (u, v, h) in shape.cells.items():
        edged = shape.border(x, y)
        c = g.scale((150, 142, 128), light * (0.6 if edged else 1.0)) if edged else \
            g.scale((16, 14, 22), 1.0 - 0.4 * v)
        g.put(img, palette, x, y, g.scale(c, 1.0 + rng.gauss(0, 0.04)))
    return sorted(set(y for (_, y) in shape.cells))


def ladder(palette, tier, up, mode, seed):
    """A hole, with the ladder going into it: mode is 'hole' for just the hole, 'face' for a ladder
    seen from the end of its rungs, or 'side' for one seen with them edge-on"""
    img = bytearray(WIDTH * HEIGHT)
    rng = random.Random(seed)
    rows = hole(img, palette, tier, up, rng)
    if mode == "hole" or not rows:
        return img

    s = wscale(tier)
    top = int(round(FLOOR_Y[tier] - 120 * s))
    floor = FLOOR_Y[tier]
    gap = max(1.0, 20 * s)
    rail_w = max(1, round(3 * s))

    # From the floor or ceiling, to the line round the end of the hole
    if up:
        y0, y1 = rows[0] + 1, floor
    else:
        y0, y1 = top, rows[-1] - 1
    if y1 <= y0:
        return img

    light = g.depth_light(tier)
    centers = (WIDTH / 2 - gap, WIDTH / 2 + gap) if mode == "face" else (WIDTH / 2,)
    for center in centers:
        for y in range(y0, y1 + 1):
            for w in range(rail_w):
                x = int(round(center - 0.5)) - rail_w // 2 + w
                if 0 <= x < WIDTH and 0 <= y < HEIGHT:
                    g.put(img, palette, x, y, g.scale(g.WOOD, light * (1.0 + rng.gauss(0, 0.05))), ["wood"])

    if mode == "face":
        for y_rung in (-100, -75, -45, -15):
            y = int(round(FLOOR_Y[tier] + y_rung * s))
            if not (y0 <= y <= y1):
                continue
            for x in range(int(round(WIDTH / 2 - gap)) + rail_w, int(round(WIDTH / 2 + gap)) - rail_w):
                if 0 <= x < WIDTH:
                    g.put(img, palette, x, y, g.scale((154, 108, 62), light), ["wood"])
    return img


def paint(img, palette, tier, points, seed, lit, bands=True, lock=False):
    shape = g.Shape()
    shape.add_polygon([wpoint(tier, x, y) for (x, y) in points])
    part = g.wood_box(shape, palette, seed, tier, lit, bands, lock)
    for (x, y) in shape.cells:
        img[y * WIDTH + x] = part[y * WIDTH + x]


def chest(palette, tier, seed):
    """A chest on the floor: the front, the lid and the side"""
    img = bytearray(WIDTH * HEIGHT)
    paint(img, palette, tier, [(24, 5), (36, -13), (36, -37), (24, -25)], seed, 0.55, bands=False)
    paint(img, palette, tier, [(-25, -25), (-13, -37), (36, -37), (24, -25)], seed, 1.15)
    paint(img, palette, tier, [(-25, 5), (24, 5), (24, -25), (-25, -25)], seed, 0.9, lock=True)
    return img


def coffin(palette, tier, seed):
    """A coffin with its lid ajar"""
    img = bytearray(WIDTH * HEIGHT)
    paint(img, palette, tier, [(-50, 5), (-65, -5), (-65, -17), (-50, -7)], seed, 0.4, bands=False)
    paint(img, palette, tier, [(50, 5), (57, 1), (57, -12), (50, -7)], seed, 0.4, bands=False)
    paint(img, palette, tier, [(-50, 5), (50, 5), (50, -7), (-50, -7)], seed, 0.6, bands=False)
    paint(img, palette, tier, [(-50, -7), (-65, -17), (-45, -22), (57, -12), (50, -7)], seed, 0.85, bands=False)
    return img


def cube(palette, d, seed):
    """The gelatinous cube, which fills the corridor"""
    img = bytearray(WIDTH * HEIGHT)
    left, right, top, bottom = plane(d)
    rng = random.Random(seed)
    blobs = [(rng.uniform(left, right), rng.uniform(top, bottom), rng.uniform(0.1, 0.25)) for _ in range(5)]
    size = max(right - left, 1)

    for y in range(top, bottom + 1):
        for x in range(left, right + 1):
            v = (y - top) / max(bottom - top, 1)
            u = (x - left) / size
            wobble = 0.12 * math.sin(u * 11 + v * 5) + 0.08 * math.sin(v * 17 - u * 4)
            shine = 0.0
            for bx, by, br in blobs:
                dist = math.hypot(x - bx, y - by) / (br * size)
                if dist < 1.0:
                    shine += (1.0 - dist) * 0.35
            lit = 0.85 + wobble - 0.3 * v + shine
            if min(x - left, right - x, y - top, bottom - y) < max(1, 4 >> d):
                lit += 0.35
            c = g.scale((50, 180, 70), lit * (0.7 + 0.3 * g.depth_light(d)))
            g.put(img, palette, x, y, g.scale(c, 1.0 + rng.gauss(0, 0.03)), ["green"])
    return img


# ---------------------------------------------------------------------- pieces

def make_pieces(palette):
    """Every picture, as a dictionary of name to pixels"""
    pieces = {}
    seed = [300]

    def next_seed():
        seed[0] += 1
        return seed[0]

    for d in range(5):
        pieces["front_%d" % d] = front_wall(palette, next_seed(), d)[1]

    # standing in a doorway has the wall round it, and the way through open
    left, right, top, bottom = plane(0)
    shape = g.Shape()
    shape.add_columns(left, right, top, top, bottom, bottom)
    for (x, y) in list(shape.cells):
        if ARRAY_X[1] <= x <= RIGHT - ARRAY_X[1] and y >= ARRAY_Y[1]:
            del shape.cells[(x, y)]
    pieces["doorway_0"] = g.stone(shape, palette, next_seed(), 0, group="front_0")

    for d in range(1, 5):
        shape = g.Shape()
        shape.add_columns(ARRAY_X[d + 1], RIGHT - ARRAY_X[d + 1], ARRAY_Y[d + 1], ARRAY_Y[d + 1],
                          BOTTOM - ARRAY_Y[d], BOTTOM - ARRAY_Y[d])
        pieces["frontdoor_%d" % d] = g.door(shape, palette, next_seed(), d)

    for d in range(6):
        pieces["beams_%d" % d] = beams(palette, next_seed(), d)
        pieces["leftedge_%d" % d] = edge(palette, d, True)
        pieces["rightedge_%d" % d] = edge(palette, d, False)

    for d in range(1, 6):
        near, far = g.CENTER - ARRAY_X[d - 1], far_distance(d)
        top_near, top_far = ARRAY_Y[d - 1], ARRAY_Y[d]
        bottom_near, bottom_far = BOTTOM - ARRAY_Y[d - 1], BOTTOM - ARRAY_Y[d]

        # the walls at the sides, from the plane before to this one
        shape = g.Shape()
        shape.add_columns(ARRAY_X[d - 1], ARRAY_X[d], top_near, top_far, bottom_near, bottom_far,
                          perspective=(near, far))
        pieces["left_%d" % d] = g.stone(shape, palette, 1, d - 1, group="left", side=True)

        shape = g.Shape()
        shape.add_columns(RIGHT - ARRAY_X[d], RIGHT - ARRAY_X[d - 1], top_far, top_near, bottom_far,
                          bottom_near, perspective=(far, near))
        pieces["right_%d" % d] = g.stone(shape, palette, 2, d - 1, group="right", side=True, flip=True)

        # an opening at a side shows the wall in front of the next square along
        for name, x0, x1 in (("leftopen", ARRAY_X[d - 1], ARRAY_X[d]),
                             ("rightopen", RIGHT - ARRAY_X[d], RIGHT - ARRAY_X[d - 1])):
            shape = g.Shape()
            shape.add_columns(x0, x1, top_far, top_far, bottom_far, bottom_far)
            width = beyond_front(shape, d)
            if width > 0 and shape.cells:
                pieces["%s_%d" % (name, d)] = g.stone(shape, palette, next_seed(), d,
                                                      group="front_%d" % d, unit_width=width)

        for name, is_left in (("leftdoor", True), ("rightdoor", False)):
            img = side_door(palette, next_seed(), d, is_left)
            if img:
                pieces["%s_%d" % (name, d)] = img

    for tier in range(TIERS):
        for up, name in ((True, "up"), (False, "down")):
            pieces["ladder%shole_%d" % (name, tier)] = ladder(palette, tier, up, "hole", next_seed())
            pieces["ladder%sfaceon_%d" % (name, tier)] = ladder(palette, tier, up, "face", next_seed())
            pieces["ladder%ssideways_%d" % (name, tier)] = ladder(palette, tier, up, "side", next_seed())
        pieces["chest_%d" % tier] = chest(palette, tier, next_seed())
        pieces["coffin_%d" % tier] = coffin(palette, tier, next_seed())

    return pieces


def make_cubes(palette):
    return {"monster_9_%d" % d: cube(palette, d, 900 + d) for d in range(1, 5)}


# -------------------------------------------------------------------- previews

def compose(pieces, names):
    buf = bytearray(WIDTH * HEIGHT)
    for name in names:
        img = pieces.get(name)
        if img:
            for i, c in enumerate(img):
                if c:
                    buf[i] = c
    return buf


def scenes():
    """A few views of corridors, as the game would draw them: from the far end of what's in view,
    to the near"""
    def sides(upto, open_left=(), open_right=(), door_left=(), door_right=()):
        names = []
        for i in range(upto, 0, -1):
            names.append(("leftopen_%d" if i in open_left else "left_%d") % i)
            names.append(("rightopen_%d" if i in open_right else "right_%d") % i)
            if i in door_left:
                names.append("leftdoor_%d" % i)
            if i in door_right:
                names.append("rightdoor_%d" % i)
        return names

    return [
        # a corridor that ends in a wall
        ["front_4"] + sides(4),
        # openings at the sides, with their corners, and a door ahead
        ["front_3", "frontdoor_3"] + sides(3, open_left=(2,), open_right=(3,)) + ["leftedge_2", "leftedge_1", "rightedge_3"],
        # a doorway, a ladder down, then beams
        ["front_5"] + sides(5, door_left=(3,), door_right=(2,)) + ["beams_2", "ladderdownfaceon_1"],
        # a monster-less hall with a chest, a coffin and a ladder up
        ["front_4"] + sides(4) + ["ladderupfaceon_2", "chest_2", "coffin_3"],
    ]


def make_preview(pieces, path):
    views = [compose(pieces, names) for names in scenes()]
    cols = 2
    rows = (len(views) + cols - 1) // cols
    rgb = bytearray(cols * WIDTH * rows * HEIGHT * 3)
    for i, buf in enumerate(views):
        ox, oy = (i % cols) * WIDTH, (i // cols) * HEIGHT
        for y in range(HEIGHT):
            for x in range(WIDTH):
                c = PALETTE.colors[buf[y * WIDTH + x]]
                o = ((oy + y) * cols * WIDTH + ox + x) * 3
                rgb[o:o + 3] = bytes(c)
    k = 2
    g.write_rgb(path, g.enlarge(bytes(rgb), cols * WIDTH, rows * HEIGHT, k), cols * WIDTH * k, rows * HEIGHT * k)


PALETTE = None


def main():
    global PALETTE
    outdir = sys.argv[1]
    PALETTE = g.Palette()
    os.makedirs(os.path.join(outdir, "dungeons"), exist_ok=True)
    os.makedirs(os.path.join(outdir, "monsters"), exist_ok=True)

    pieces = make_pieces(PALETTE)
    for name, img in pieces.items():
        if any(img):
            g.write_indexed(os.path.join(outdir, "dungeons", name + ".png"), img, WIDTH, HEIGHT, PALETTE)
    PALETTE.write(os.path.join(outdir, "dungeons", "palette.pal"))

    for name, img in make_cubes(PALETTE).items():
        g.write_indexed(os.path.join(outdir, "monsters", name + ".png"), img, WIDTH, HEIGHT, PALETTE)

    if len(sys.argv) > 2:
        make_preview(pieces, sys.argv[2])


if __name__ == "__main__":
    main()
