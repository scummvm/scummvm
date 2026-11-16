#!/usr/bin/env python3
"""
Generates the stand-in pictures of the monsters in the dungeons of Ultima I, for the "textured
dungeon" renderer that is used with the EGA graphics.

There is a picture of each kind of monster at each of the distances 1 to 4 that the game draws
them at, named monster_<kind>_<distance>.png. Each is a 288x144 layer in the geometry of the view
of the dungeon, like the pictures of the walls (gen_ultima1_dungeons.py, which also writes the
gelatinous cube), and uses the same standard colors (palette.pal, which that script writes).

The kinds have the numbers that they have in the game: 0 ranger, 1 skeleton, 2 thief, 3 giant rat,
4 bat, 5 spider, 6 viper, 7 orc, 8 cyclops, 9 gelatinous cube, 10 ettin, 11 mimic (which is drawn
as a chest), 12 lizard man, 13 minotaur, 14 carrion creeper, 15 tangler, 16 gremlin, 17 wandering
eyes, 18 wraith, 19 lich, 20 invisible seeker (which isn't drawn), 21 mind whipper, 22 zorn, 23
daemon and 24 balron.

Each is designed in the units of the game's own line drawings of the monsters, where the monster
stands on the floor in the middle at (0, 0) with up as negative y, and is as big as the line
drawing is, which the game draws at half the size for each of the distances after the first.

These are stand-in pictures made by a program, written with the help of an AI assistant (Claude).
They are meant to be replaced by hand drawn art: any picture can be swapped for one of the same
name, size and palette, and the game falls back to its lines for any that are missing.

Usage: gen_ultima1_monsters.py <output folder> [<preview.png>]
"""
import math
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "ultima2"))
import gen_ultima2_dungeons as g
import gen_ultima2_monsters as m
import gen_ultima1_dungeons as d1

WIDTH, HEIGHT = d1.WIDTH, d1.HEIGHT
LIGHT = m.LIGHT
Ellipse, Capsule, Poly = m.Ellipse, m.Capsule, m.Poly

FLESH = (232, 170, 130)
BONE = (226, 222, 200)
DARK = (24, 20, 26)
GOLD = (230, 190, 60)
STEEL = (200, 205, 220)
WOODC = (150, 86, 42)
RED_GLOW = (255, 80, 60)
YELLOW_GLOW = (250, 235, 110)


def eyes(x, y, spread, rx=2.2, ry=2.6, color=YELLOW_GLOW, pupil=True):
    """A pair of glowing eyes"""
    out = []
    for s in (-1, 1):
        out.append(Ellipse(x + s * spread, y, rx, ry, "pale", color, glow=True))
        if pupil:
            out.append(Ellipse(x + s * spread, y + 0.4, rx * 0.4, ry * 0.8, "iron", (20, 0, 0)))
    return out


def tusks(x, y, spread, size):
    return [Poly([(x + s * spread, y), (x + s * (spread + size * 0.5), y), (x + s * spread, y + size)],
                 "pale", BONE) for s in (-1, 1)]


# ---------------------------------------------------------------- the monsters

def ranger():
    p = []
    cloth, leather = (60, 130, 70), (120, 80, 40)
    p += [Poly([(-17, -62), (17, -62), (22, -8), (-22, -8)], "moss", (40, 70, 45))]
    for s in (1, -1):
        p += [Capsule(s * 8, -30, s * 9, -5, 6, 5, "moss", (70, 100, 55)),
              Ellipse(s * 10, -3, 8, 4, "wood", (110, 70, 38))]
    p += [Ellipse(0, -45, 15, 22, "green", cloth),
          Poly([(-15, -33), (15, -33), (15, -28), (-15, -28)], "wood", leather),
          Ellipse(0, -30.5, 3, 3, "gold", GOLD, gloss=0.6)]
    # a bow in one hand, and a sword in the other
    p += [Capsule(-14, -58, -30, -46, 5, 4, "green", cloth), Ellipse(-32, -45, 3.5, 3.5, "flesh", FLESH),
          Capsule(-34, -76, -42, -46, 2.2, 2.2, "wood", WOODC), Capsule(-42, -46, -34, -14, 2.2, 2.2, "wood", WOODC),
          Capsule(-34, -76, -34, -14, 0.7, 0.7, "pale", (230, 225, 200))]
    # the sword, with a short hilt that the hand is round
    p += [Capsule(31, -76, 31, -49, 1.7, 1.4, "iron", STEEL, gloss=0.9),
          Capsule(24, -48, 38, -48, 1.5, 1.5, "gold", GOLD),
          Capsule(31, -48, 31, -37, 1.5, 1.5, "gold", GOLD), Ellipse(31, -36, 2.2, 2.2, "gold", GOLD, gloss=0.6)]
    p += [Capsule(14, -58, 28, -44, 5, 4, "green", cloth), Ellipse(30, -43, 3.5, 3.5, "flesh", FLESH)]
    p += [Ellipse(0, -69, 12, 12, "moss", (50, 90, 50)), Ellipse(0, -65, 8, 8, "flesh", FLESH),
          Ellipse(-3, -65, 1.2, 1.5, "iron", DARK), Ellipse(3, -65, 1.2, 1.5, "iron", DARK)]
    return p


def skeleton():
    p = []
    for s in (1, -1):
        p += [Capsule(s * 8, -50, s * 9, -6, 3.2, 2.6, "pale", BONE), Ellipse(s * 11, -3, 7, 3, "pale", BONE)]
    p += [Ellipse(0, -46, 8, 6, "pale", BONE), Capsule(0, -50, 0, -76, 2.2, 2.2, "pale", BONE)]
    for y in (-72, -66, -60, -54):
        p += [Ellipse(0, y, 13 - abs(y + 63) * 0.4, 2.8, "pale", BONE), Ellipse(0, y + 2.6, 10, 0.8, "iron", DARK)]
    p += [Capsule(-12, -74, -30, -62, 2.4, 2.2, "pale", BONE), Capsule(-30, -62, -44, -52, 2, 2, "pale", BONE),
          Capsule(12, -74, 26, -62, 2.4, 2.2, "pale", BONE), Capsule(26, -62, 31, -46, 2, 2, "pale", BONE)]
    p += [Capsule(-46, -92, -45, -50, 1.6, 1.3, "iron", STEEL, gloss=0.9), Capsule(-50, -56, -40, -55, 1.6, 1.6, "gold", GOLD),
          Ellipse(33, -46, 5, 12, "wood", (140, 90, 50)), Ellipse(33, -46, 1.6, 2.2, "gold", GOLD)]
    p += [Ellipse(0, -84, 9, 9, "pale", BONE), Ellipse(0, -77, 6, 3.5, "pale", BONE),
          Ellipse(-3.5, -85, 2.3, 2.8, "iron", DARK), Ellipse(3.5, -85, 2.3, 2.8, "iron", DARK),
          Poly([(-1.3, -80), (1.3, -80), (0, -83)], "iron", DARK)]
    for x in (-3, 0, 3):
        p += [Capsule(x, -78, x, -75.5, 0.7, 0.7, "iron", DARK)]
    return p


def thief():
    p = []
    cloak, dark = (60, 40, 90), (30, 24, 50)
    p += [Poly([(-18, -76), (18, -76), (30, -4), (-30, -4)], "purple", dark)]
    for s in (1, -1):
        p += [Capsule(s * 8, -34, s * 9, -5, 5.5, 5, "iron", (50, 46, 60)), Ellipse(s * 11, -3, 7.5, 3.5, "wood", (80, 52, 30))]
    p += [Ellipse(0, -55, 17, 24, "purple", cloak), Poly([(-14, -42), (14, -42), (14, -37), (-14, -37)], "wood", (100, 70, 40))]
    p += [Capsule(-16, -66, -30, -48, 5, 4, "purple", cloak), Ellipse(-31, -46, 3.5, 3.5, "flesh", FLESH),
          Capsule(16, -66, 30, -52, 5, 4, "purple", cloak), Ellipse(31, -50, 3.5, 3.5, "flesh", FLESH),
          Poly([(31, -50), (36, -66), (34, -50)], "iron", STEEL, gloss=0.9)]
    p += [Ellipse(0, -86, 13, 14, "purple", cloak), Ellipse(0, -84, 8.5, 8, "iron", (14, 10, 20)),
          Ellipse(-3.4, -84, 1.8, 1.4, "pale", YELLOW_GLOW, glow=True), Ellipse(3.4, -84, 1.8, 1.4, "pale", YELLOW_GLOW, glow=True),
          Ellipse(-22, -30, 7, 9, "wood", (130, 90, 50)), Ellipse(-22, -37, 3, 1.6, "gold", GOLD)]
    return p


def giant_rat():
    p = []
    fur = (130, 96, 70)
    p += [Capsule(14, -12, 24, -26, 2.4, 1.6, "rust", (200, 140, 130)), Capsule(24, -26, 20, -44, 1.6, 1.2, "rust", (200, 140, 130))]
    for s in (1, -1):
        p += [Capsule(s * 9, -18, s * 10, -3, 6, 4.6, "rust", fur), Ellipse(s * 11, -2.4, 6.5, 3, "rust", (150, 110, 90))]
    p += [Ellipse(0, -30, 17, 22, "rust", fur), Ellipse(0, -26, 9, 14, "flesh", (210, 170, 140))]
    for s in (1, -1):
        p += [Capsule(s * 13, -38, s * 20, -26, 3.2, 2.6, "rust", fur), Ellipse(s * 21, -24, 3, 2.4, "flesh", (220, 150, 130))]
    p += [Ellipse(0, -52, 11, 9, "rust", fur)]
    for s in (1, -1):
        p += [Ellipse(s * 9, -60, 5.6, 6.2, "flesh", (220, 150, 140)),
              Ellipse(s * 9, -59.5, 3.2, 3.8, "rust", (200, 120, 110))]
    p += [Poly([(-4, -54), (4, -54), (0, -44)], "flesh", (230, 150, 140)), Ellipse(0, -46, 2.2, 1.6, "iron", DARK)]
    p += eyes(0, -54, 5, 1.8, 1.8, RED_GLOW, False)
    p += [Poly([(-2.5, -44), (-1, -40), (0.2, -44)], "pale", BONE), Poly([(0.2, -44), (1.6, -40), (3, -44)], "pale", BONE)]
    return p


def bat():
    p = []
    wing, mem = (80, 50, 110), (120, 70, 150)
    for s in (1, -1):
        p += [Poly([(s * 6, -76), (s * 30, -86), (s * 54, -72), (s * 44, -68), (s * 40, -62), (s * 30, -66),
                    (s * 22, -58), (s * 12, -64)], "purple", wing),
              Capsule(s * 6, -76, s * 30, -86, 1.4, 1.2, "purple", mem), Capsule(s * 30, -86, s * 54, -72, 1.2, 1, "purple", mem)]
    p += [Ellipse(0, -70, 8, 12, "purple", (90, 60, 120)), Ellipse(0, -62, 3, 4, "purple", (70, 40, 100))]
    for s in (1, -1):
        p += [Poly([(s * 4, -78), (s * 9, -92 + 8), (s * 7, -76)], "purple", (90, 60, 120))]
    p += [Ellipse(0, -76, 6.5, 6, "purple", (100, 70, 130))]
    p += eyes(0, -77, 3, 1.6, 1.6, RED_GLOW, False)
    p += [Poly([(-2.4, -72), (-1.4, -68), (-0.4, -72)], "pale", BONE), Poly([(0.4, -72), (1.4, -68), (2.4, -72)], "pale", BONE)]
    return p


def spider():
    p = []
    body = (40, 36, 46)
    for s in (1, -1):
        for k, (ox, mid, end) in enumerate(((10, (36, -112), (70, -108)), (14, (62, -100), (100, -56)),
                                            (16, (70, -74), (112, -20)), (14, (56, -44), (88, -3)))):
            p += [Capsule(s * ox, -52 + k * 4, s * mid[0], mid[1], 5, 3.6, "iron", body),
                  Capsule(s * mid[0], mid[1], s * end[0], end[1], 3.6, 2, "iron", (60, 50, 70))]
    p += [Ellipse(0, -38, 28, 24, "iron", (52, 44, 60)), Ellipse(0, -62, 18, 14, "iron", body)]
    p += [Poly([(-6, -44), (6, -44), (3, -36), (6, -28), (-6, -28), (-3, -36)], "red", (190, 30, 24))]
    p += eyes(0, -66, 6, 2.4, 2.4, RED_GLOW, False) + eyes(0, -72, 11, 1.8, 1.8, RED_GLOW, False)
    p += [Capsule(-4, -56, -3, -48, 1.8, 1.4, "pale", BONE), Capsule(4, -56, 3, -48, 1.8, 1.4, "pale", BONE)]
    return p


def viper():
    p = []
    green, spots = (60, 150, 60), (30, 90, 35)
    spine = [(-30, -4, 6), (-12, -8, 6.5), (8, -4, 7), (28, -12, 7), (30, -28, 7), (16, -40, 7), (8, -50, 7)]
    for i in range(len(spine) - 1):
        x1, y1, r1 = spine[i]
        x2, y2, r2 = spine[i + 1]
        p += [Capsule(x1, y1, x2, y2, r1, r2, "green", green),
              Ellipse((x1 + x2) / 2, (y1 + y2) / 2, 2.2, (r1 + r2) / 2 * 0.9, "moss", spots)]
    p += [Ellipse(8, -54, 13, 8.5, "green", green), Poly([(-4, -52), (20, -52), (18, -46), (0, -46)], "iron", DARK)]
    p += [Capsule(20, -48, 32, -44, 1, 0.8, "red", RED_GLOW), Poly([(32, -44), (37, -48), (36, -44), (37, -41)], "red", RED_GLOW)]
    p += eyes(8, -58, 5, 2, 2.4, YELLOW_GLOW)
    p += [Poly([(-2, -50), (0, -45), (2, -50)], "pale", BONE), Poly([(14, -50), (16, -45), (18, -50)], "pale", BONE)]
    return p


def orc():
    p = []
    skin, armor = (90, 140, 70), (120, 80, 50)
    for s in (1, -1):
        p += [Capsule(s * 10, -44, s * 11, -8, 8, 6.5, "moss", skin), Ellipse(s * 12, -4, 9, 5, "wood", (80, 50, 30))]
    p += [Ellipse(0, -54, 21, 24, "moss", skin), Poly([(-20, -66), (20, -66), (22, -32), (-22, -32)], "wood", armor),
          Poly([(-22, -38), (22, -38), (22, -33), (-22, -33)], "iron", (70, 70, 80)),
          Ellipse(0, -36, 3.5, 3.5, "gold", GOLD, gloss=0.6)]
    p += [Capsule(-20, -68, -34, -52, 7, 5.5, "moss", skin), Ellipse(-35, -50, 5, 5, "moss", (110, 160, 90)),
          Capsule(20, -68, 30, -50, 7, 5.5, "moss", skin), Ellipse(31, -48, 5, 5, "moss", (110, 160, 90)),
          Capsule(31, -48, 28, -92, 4, 5.5, "wood", WOODC), Ellipse(28, -92, 6.5, 5.5, "wood", (110, 70, 40))]
    p += [Ellipse(0, -78, 14, 13, "moss", skin), Poly([(-12, -80), (-18, -88), (-8, -84)], "moss", skin),
          Poly([(12, -80), (18, -88), (8, -84)], "moss", skin), Poly([(-8, -70), (8, -70), (6, -65), (-6, -65)], "iron", (36, 14, 14))]
    p += eyes(0, -80, 5, 2, 2, YELLOW_GLOW, False) + tusks(0, -71, 4.5, 6)
    return p


def cyclops():
    p = []
    skin, cloth = (210, 160, 120), (120, 80, 50)
    for s in (1, -1):
        p += [Capsule(s * 10, -40, s * 11, -8, 8, 6.5, "flesh", skin), Ellipse(s * 12, -3, 9, 4, "flesh", (190, 140, 110))]
    p += [Ellipse(0, -50, 22, 26, "flesh", skin), Poly([(-22, -34), (22, -34), (24, -16), (-24, -16)], "wood", cloth)]
    p += [Capsule(-20, -64, -34, -46, 7, 5.5, "flesh", skin), Ellipse(-35, -44, 5, 5, "flesh", (190, 140, 110)),
          Capsule(20, -64, 30, -48, 7, 5.5, "flesh", skin), Ellipse(31, -46, 5, 5, "flesh", (190, 140, 110)),
          Capsule(33, -46, 30, -84, 3.5, 4.5, "wood", WOODC)]
    p += [Ellipse(0, -72, 15, 14, "flesh", skin), Ellipse(0, -74, 8, 6.5, "pale", (250, 250, 240), glow=True),
          Ellipse(0, -74, 3.8, 3.8, "red", (190, 40, 30)), Ellipse(0, -74, 1.6, 1.6, "iron", (10, 0, 0)),
          Poly([(-8, -62), (8, -62), (6, -58), (-6, -58)], "iron", (60, 20, 20)),
          Poly([(-9, -80), (0, -84), (9, -80), (0, -79)], "wood", (80, 50, 30))]
    return p


def ettin():
    p = []
    skin, cloth = (150, 140, 120), (110, 70, 45)
    for s in (1, -1):
        p += [Capsule(s * 12, -42, s * 13, -8, 9, 7, "stone", skin), Ellipse(s * 14, -4, 10, 5, "stone", (110, 100, 90))]
    p += [Ellipse(0, -56, 28, 28, "stone", skin), Poly([(-26, -40), (26, -40), (28, -22), (-28, -22)], "wood", cloth)]
    p += [Capsule(-26, -70, -40, -48, 8, 6, "stone", skin), Ellipse(-41, -46, 5.5, 5.5, "stone", (130, 120, 105)),
          Capsule(26, -70, 40, -52, 8, 6, "stone", skin), Ellipse(42, -50, 5.5, 5.5, "stone", (130, 120, 105))]
    # the club, which is in front of the hand
    p += [Capsule(-40, -40, -50, -82, 4, 7, "wood", (110, 70, 40)), Ellipse(-50, -86, 8, 7, "wood", (90, 56, 30))]
    for hx in (-17, 20):
        p += [Ellipse(hx, -80, 12, 12, "stone", skin), Poly([(hx - 8, -70), (hx + 8, -70), (hx + 6, -66), (hx - 6, -66)], "iron", (50, 24, 24))]
        p += eyes(hx, -82, 5, 2, 2, YELLOW_GLOW, False) + tusks(hx, -71, 3.5, 5)
    return p


def lizard_man():
    p = []
    green, belly = (70, 150, 80), (190, 200, 120)
    p += [Capsule(8, -10, 30, -8, 7, 3.4, "green", green), Capsule(30, -8, 42, -22, 3.4, 1.4, "green", (50, 120, 60))]
    for s in (1, -1):
        p += [Capsule(s * 8, -34, s * 8, -6, 6, 4.6, "green", green), Ellipse(s * 9, -3, 9, 3.6, "green", (50, 120, 60))]
    p += [Ellipse(0, -44, 15, 22, "green", green), Ellipse(2, -40, 8, 16, "pale", belly)]
    p += [Capsule(-14, -54, -29, -40, 4.6, 3.6, "green", green), Capsule(14, -54, 26, -42, 4.6, 3.6, "green", green),
          Ellipse(27, -41, 3.6, 3.6, "green", (50, 120, 60)), Capsule(27, -41, 34, -72, 1.3, 1.3, "wood", WOODC),
          Poly([(30, -68), (34, -78), (38, -68)], "iron", STEEL, gloss=0.9)]
    p += [Ellipse(4, -64, 11, 8.5, "green", green), Poly([(10, -66), (24, -62), (22, -57), (8, -58)], "green", (50, 120, 60)),
          Capsule(6, -58, 22, -58, 0.8, 0.8, "iron", DARK)]
    p += [Ellipse(5, -67, 2.2, 2.4, "pale", YELLOW_GLOW, glow=True), Ellipse(5.4, -67, 0.7, 1.8, "iron", DARK)]
    for k in range(4):
        p += [Poly([(-10 + k * 3, -72), (-8 + k * 3, -80), (-6 + k * 3, -72)], "green", (50, 120, 60))]
    return p


def minotaur():
    p = []
    fur, skin = (140, 84, 50), (190, 130, 90)
    for s in (1, -1):
        p += [Capsule(s * 10, -44, s * 11, -8, 8, 6.5, "rust", fur), Ellipse(s * 12, -3, 8, 4, "iron", (40, 30, 30))]
    p += [Ellipse(0, -56, 23, 27, "rust", fur), Poly([(-20, -42), (20, -42), (22, -28), (-22, -28)], "wood", (90, 60, 40))]
    p += [Capsule(-20, -70, -34, -52, 7.5, 6, "rust", fur), Capsule(20, -70, 30, -54, 7.5, 6, "rust", fur),
          Capsule(33, -50, 30, -86, 1.6, 1.6, "wood", WOODC),
          Poly([(30, -84), (42, -94), (42, -78)], "iron", STEEL, gloss=0.9),
          Poly([(30, -84), (20, -94), (20, -78)], "iron", STEEL, gloss=0.9)]
    p += [Ellipse(0, -80, 13, 13, "rust", fur), Ellipse(0, -73, 7.5, 6, "flesh", skin),
          Ellipse(0, -72, 1.6, 1.4, "iron", DARK), Ellipse(0, -66, 3, 1.4, "gold", GOLD, gloss=0.6)]
    for s in (1, -1):
        p += [Capsule(s * 8, -88, s * 20, -98, 3.4, 1, "pale", BONE), Capsule(s * 20, -98, s * 26, -86, 1, 1, "pale", BONE)]
    p += eyes(0, -82, 5, 2, 1.8, RED_GLOW, False)
    return p


def carrion_creeper():
    p = []
    body = (100, 60, 130)
    for k, x in enumerate((-36, -18, 0, 18, 36)):
        lean = 6 if k % 2 else -6
        p += [Capsule(x * 0.8, -104, x, -78, 5, 3.6, "purple", body), Capsule(x, -78, x + lean, -56, 3.6, 2.6, "purple", (130, 80, 160)),
              Capsule(x + lean, -56, x - lean * 0.5, -38, 2.6, 1.6, "green", (60, 170, 70))]
    p += [Ellipse(0, -108, 34, 18, "purple", body), Poly([(-40, -122), (40, -122), (30, -108), (-30, -108)], "stone", (90, 80, 70))]
    p += [Ellipse(0, -100, 12, 7, "iron", (40, 10, 30))]
    for x in (-8, -3, 3, 8):
        p += [Poly([(x - 1.6, -104), (x, -97), (x + 1.6, -104)], "pale", BONE)]
    p += eyes(0, -112, 14, 3, 3, YELLOW_GLOW)
    return p


def tangler():
    p = []
    stem, bulb = (60, 120, 50), (90, 160, 60)
    p += [Capsule(0, -4, 0, -52, 9, 6, "moss", stem), Ellipse(0, -3, 16, 5, "wood", (80, 50, 30))]
    for s in (1, -1):
        p += [Capsule(s * 6, -20, s * 24, -34, 3.4, 2.4, "moss", stem), Capsule(s * 24, -34, s * 34, -22, 2.4, 1.6, "moss", stem),
              Capsule(s * 6, -34, s * 28, -62, 3.4, 2.4, "moss", stem), Capsule(s * 28, -62, s * 36, -50, 2.4, 1.6, "moss", (80, 150, 60)),
              Poly([(s * 8, -10), (s * 28, -4), (s * 12, -22)], "green", (50, 150, 60))]
    p += [Ellipse(0, -72, 20, 24, "moss", bulb), Ellipse(0, -66, 14, 10, "iron", (60, 10, 20))]
    for x in (-9, -4.5, 0, 4.5, 9):
        p += [Poly([(x - 2, -72), (x, -64), (x + 2, -72)], "pale", BONE), Poly([(x - 2, -60), (x, -67), (x + 2, -60)], "pale", BONE)]
    p += eyes(0, -86, 8, 2.8, 2.8, YELLOW_GLOW)
    return p


def gremlin():
    p = []
    skin = (110, 150, 70)
    for s in (1, -1):
        p += [Capsule(s * 5, -12, s * 6, -3, 3.6, 3, "moss", skin), Ellipse(s * 7, -1.6, 5, 2.4, "moss", (80, 120, 50))]
    p += [Ellipse(0, -16, 9, 11, "moss", skin), Ellipse(0, -13, 5, 7, "flesh", (210, 190, 130))]
    for s in (1, -1):
        p += [Capsule(s * 8, -20, s * 16, -12, 2.2, 1.6, "moss", skin)]
    p += [Ellipse(0, -26, 9, 7, "moss", skin)]
    for s in (1, -1):
        p += [Poly([(s * 7, -28), (s * 22, -34), (s * 8, -23)], "moss", skin)]
    p += eyes(0, -27, 3.4, 1.8, 2, RED_GLOW, False)
    p += [Poly([(-4, -23), (4, -23), (0, -20)], "iron", (40, 10, 10)), Poly([(-3, -23), (-2, -21), (-1, -23)], "pale", BONE),
          Poly([(1, -23), (2, -21), (3, -23)], "pale", BONE)]
    return p


def wandering_eyes():
    p = []
    stalk = (160, 90, 100)
    balls = [(0, -58, 15), (-21, -48, 10), (20, -50, 10.5), (-14, -72, 8.5), (13, -73, 8)]
    for x, y, r in balls[1:]:
        p += [Capsule(0, -56, x, y, 3.4, 2.4, "flesh", stalk)]
    for x, y, r in balls:
        p += [Ellipse(x, y, r, r, "pale", (245, 245, 235), gloss=0.5), Ellipse(x + 1, y + 1, r * 0.5, r * 0.5, "red", (190, 40, 30)),
              Ellipse(x + 1.2, y + 1.2, r * 0.25, r * 0.25, "iron", (10, 0, 0))]
        p += [Capsule(x - r * 0.7, y - r * 0.3, x - r * 0.2, y + r * 0.1, 0.5, 0.5, "red", RED_GLOW)]
    return p


def wraith():
    p = []
    robe, dark = (60, 70, 100), (24, 28, 44)
    p += [Poly([(-16, -86), (16, -86), (30, -6), (22, -14), (14, -3), (6, -14), (-2, -2), (-10, -14), (-18, -3), (-26, -12), (-34, -4)], "iron", robe)]
    p += [Capsule(-14, -70, -30, -58, 5, 3.6, "iron", robe), Capsule(-30, -58, -46, -64, 3.6, 2, "iron", (70, 80, 110))]
    for k in range(4):
        p += [Capsule(-46 + k * 3, -64, -48 + k * 3, -72 + k, 0.9, 0.5, "pale", (200, 205, 225))]
    p += [Capsule(14, -70, 26, -52, 5, 3.6, "iron", robe), Ellipse(0, -80, 14, 16, "iron", robe), Ellipse(0, -78, 9.5, 11, "iron", (6, 8, 16))]
    p += [Ellipse(-3.8, -79, 2.2, 1.6, "cyan", (110, 240, 250), glow=True), Ellipse(3.8, -79, 2.2, 1.6, "cyan", (110, 240, 250), glow=True)]
    return p


def lich():
    p = []
    robe = (90, 50, 130)
    p += [Poly([(-18, -62), (18, -62), (24, -42), (14, -46), (8, -41), (0, -46), (-8, -41), (-14, -46), (-24, -43)], "purple", robe)]
    p += [Ellipse(0, -76, 14, 15, "pale", BONE), Ellipse(0, -66, 9, 5, "pale", BONE)]
    p += [Ellipse(-5, -77, 3.4, 4, "iron", DARK), Ellipse(5, -77, 3.4, 4, "iron", DARK),
          Ellipse(-5, -77, 1.4, 1.6, "cyan", (110, 240, 250), glow=True), Ellipse(5, -77, 1.4, 1.6, "cyan", (110, 240, 250), glow=True),
          Poly([(-2, -71), (2, -71), (0, -75)], "iron", DARK)]
    for x in (-5, -2, 1, 4):
        p += [Capsule(x, -68, x, -64.5, 0.8, 0.8, "iron", DARK)]
    p += [Poly([(-12, -86), (-9, -92), (-5, -86), (0, -92), (5, -86), (9, -92), (12, -86)], "gold", GOLD, gloss=0.5)]
    return p


def mind_whipper():
    p = []
    robe, skin = (80, 40, 110), (150, 90, 170)
    p += [Poly([(-16, -72), (16, -72), (34, -4), (-34, -4)], "purple", robe)]
    p += [Capsule(-14, -66, -30, -52, 5, 3.6, "purple", robe), Ellipse(-32, -50, 4, 4, "purple", skin),
          Capsule(14, -66, 30, -52, 5, 3.6, "purple", robe), Ellipse(32, -50, 4, 4, "purple", skin)]
    p += [Ellipse(0, -80, 13, 14, "purple", skin)]
    p += eyes(0, -83, 5, 2.4, 2, (250, 250, 230), False)
    for s in (-1, 1):
        for k in range(2):
            p += [Capsule(s * (3 + k * 3), -72, s * (5 + k * 3), -54, 2.4, 1.4, "flesh", (220, 150, 150)),
                  Capsule(s * (5 + k * 3), -54, s * (2 + k * 4), -42, 1.4, 0.9, "flesh", (230, 170, 170))]
    return p


def zorn():
    p = []
    hide, light = (70, 90, 150), (110, 130, 190)
    for s in (1, -1):
        p += [Capsule(s * 22, -40, s * 26, -6, 12, 9, "blue", hide), Ellipse(s * 28, -3, 13, 5, "iron", (30, 30, 50))]
    p += [Ellipse(0, -58, 42, 38, "blue", hide), Ellipse(0, -48, 24, 24, "blue", light)]
    for s in (1, -1):
        p += [Capsule(s * 40, -72, s * 58, -58, 9, 7, "blue", hide), Capsule(s * 58, -58, s * 60, -42, 7, 5, "blue", hide),
              Poly([(s * 52, -44), (s * 67, -30), (s * 58, -42)], "pale", BONE), Poly([(s * 58, -40), (s * 66, -24), (s * 62, -40)], "pale", BONE)]
    p += [Ellipse(0, -84, 18, 14, "blue", hide), Poly([(-14, -76), (14, -76), (11, -68), (-11, -68)], "iron", (20, 10, 30))]
    for s in (1, -1):
        p += [Capsule(s * 12, -90, s * 28, -98, 5, 1.2, "pale", BONE)]
    p += eyes(0, -86, 8, 3, 3, RED_GLOW) + tusks(0, -70, 8, 6)
    return p


def daemon():
    p = []
    red, wing = (190, 40, 30), (110, 20, 40)
    for s in (1, -1):
        p += [Poly([(s * 12, -70), (s * 54, -102), (s * 52, -78), (s * 44, -70), (s * 42, -58), (s * 30, -62), (s * 16, -48)], "red", wing)]
        p += [Capsule(s * 14, -64, s * 54, -102, 1.6, 1.2, "iron", (60, 20, 30))]
    for s in (1, -1):
        p += [Capsule(s * 11, -44, s * 12, -8, 8, 6.5, "red", red), Ellipse(s * 13, -3, 9, 4.5, "iron", (40, 16, 16))]
    p += [Ellipse(0, -56, 21, 26, "red", red), Ellipse(0, -52, 12, 16, "flesh", (232, 130, 100))]
    p += [Capsule(-18, -66, -32, -46, 6.5, 5, "red", red), Capsule(18, -66, 32, -46, 6.5, 5, "red", red),
          Capsule(35, -44, 35, -100, 1.4, 1.4, "gold", GOLD), Poly([(30, -92), (35, -104), (40, -92)], "gold", GOLD)]
    p += [Ellipse(0, -80, 13, 13, "red", red)]
    for s in (1, -1):
        p += [Capsule(s * 8, -88, s * 14, -102, 3.6, 1, "pale", BONE)]
    p += eyes(0, -82, 5, 2.4, 2.2, YELLOW_GLOW)
    p += [Poly([(-6, -73), (6, -73), (4, -68), (-4, -68)], "iron", (40, 4, 4))] + tusks(0, -73, 3, 4)
    return p


def balron():
    p = []
    red, wing, flame = (150, 20, 24), (70, 14, 30), (255, 190, 60)
    for s in (1, -1):
        p += [Poly([(s * 14, -84), (s * 56, -110), (s * 86, -96), (s * 70, -84), (s * 76, -66), (s * 60, -72),
                    (s * 56, -54), (s * 40, -64), (s * 20, -50)], "red", wing)]
        p += [Capsule(s * 14, -84, s * 56, -110, 2, 1.4, "iron", (50, 20, 30)), Capsule(s * 56, -110, s * 86, -96, 1.4, 1.2, "iron", (50, 20, 30))]
    for s in (1, -1):
        p += [Capsule(s * 13, -54, s * 14, -10, 10, 8, "red", red), Ellipse(s * 15, -4, 11, 5.5, "iron", (30, 10, 14))]
    p += [Ellipse(0, -68, 25, 32, "red", red), Ellipse(0, -62, 14, 20, "red", (200, 50, 40))]
    p += [Capsule(-22, -84, -40, -62, 8, 6, "red", red), Capsule(22, -84, 40, -62, 8, 6, "red", red),
          Capsule(-42, -60, -60, -26, 2, 1.4, "gold", GOLD), Capsule(-60, -26, -68, -8, 1.4, 1, "gold", GOLD),
          Capsule(40, -58, 52, -90, 1.8, 1.8, "iron", (50, 20, 30))]
    for k in range(5):
        p += [Poly([(44 + k * 3, -94 + k * 3), (50 + k * 3, -108 + k * 3), (54 + k * 3, -92 + k * 3)], "gold", flame, glow=True)]
    p += [Ellipse(0, -98, 15, 14, "red", red)]
    for s in (1, -1):
        p += [Capsule(s * 9, -106, s * 20, -122 + 12, 4.2, 1, "pale", BONE)]
    p += eyes(0, -100, 5.5, 2.6, 2.4, flame)
    p += [Poly([(-7, -91), (7, -91), (5, -86), (-5, -86)], "iron", (30, 0, 0))] + tusks(0, -91, 3.5, 4)
    for x, y in ((-18, -30), (18, -34), (-10, -14), (12, -16)):
        p += [Poly([(x - 2.5, y + 6), (x, y - 5), (x + 2.5, y + 6)], "gold", flame, glow=True)]
    return p


MONSTERS = {
    0: ranger, 1: skeleton, 2: thief, 3: giant_rat, 4: bat, 5: spider, 6: viper, 7: orc, 8: cyclops,
    10: ettin, 12: lizard_man, 13: minotaur, 14: carrion_creeper, 15: tangler, 16: gremlin,
    17: wandering_eyes, 18: wraith, 19: lich, 21: mind_whipper, 22: zorn, 23: daemon, 24: balron
}

# The extents of the game's own lines for each, as (left, right, top, bottom)
BOUNDS = {
    0: (-43, 42, -77, 0), 1: (-49, 36, -93, 0), 2: (-37, 36, -101, 0), 3: (-25, 24, -61, 0),
    4: (-55, 54, -86, -57), 5: (-122, 121, -122, 1), 6: (-37, 36, -61, 0), 7: (-37, 36, -92, 0),
    8: (-37, 36, -86, 0), 10: (-70, 48, -91, 0), 12: (-31, 42, -74, 0), 13: (-43, 42, -92, 0),
    14: (-43, 48, -122, -37), 15: (-37, 36, -98, 0), 16: (-25, 24, -31, 0), 17: (-31, 30, -80, -37),
    18: (-49, 36, -98, 0), 19: (-25, 24, -92, -41), 21: (-40, 39, -93, 0), 22: (-68, 67, -98, 0),
    23: (-55, 54, -104, 0), 24: (-86, 85, -110, 1)
}


# -------------------------------------------------------------------- render

def part_bounds(part):
    """The area that a part can be in, so that the others needn't be tested"""
    if isinstance(part, Ellipse):
        return part.cx - part.rx, part.cx + part.rx, part.cy - part.ry, part.cy + part.ry
    if isinstance(part, Capsule):
        r = max(part.r1, part.r2)
        return min(part.x1, part.x2) - r, max(part.x1, part.x2) + r, min(part.y1, part.y2) - r, max(part.y1, part.y2) + r
    xs = [p[0] for p in part.points]
    ys = [p[1] for p in part.points]
    return min(xs), max(xs), min(ys), max(ys)


def render(parts, palette, bounds, tier):
    """Draws a monster at one of the sizes of the game's shapes"""
    s = d1.wscale(tier)
    floor = d1.FLOOR_Y[tier]
    boxes = [part_bounds(p) for p in parts]

    # All of the monster is drawn, even where it's bigger than the game's own lines are
    bounds = (min([bounds[0]] + [b[0] for b in boxes]), max([bounds[1]] + [b[1] for b in boxes]),
              min([bounds[2]] + [b[2] for b in boxes]), max([bounds[3]] + [b[3] for b in boxes]))
    left = int(math.floor(WIDTH / 2 + bounds[0] * s - 1))
    right = int(math.ceil(WIDTH / 2 + bounds[1] * s + 1))
    top = int(math.floor(floor + bounds[2] * s - 1))
    bottom = int(math.ceil(floor + bounds[3] * s + 1))
    size = max(right - left, bottom - top)
    S = 4 if size >= 20 else 6

    lx, ly, lz = LIGHT
    norm = math.sqrt(lx * lx + ly * ly + lz * lz)
    lx, ly, lz = lx / norm, ly / norm, lz / norm

    pixels = {}
    for py in range(top, bottom):
        for px in range(left, right):
            hits = 0
            sr = sg = sb = 0.0
            ramp_votes = {}
            for sy in range(S):
                for sx in range(S):
                    x = (px + (sx + 0.5) / S - WIDTH / 2) / s
                    y = (py + (sy + 0.5) / S - floor) / s
                    found = None
                    for i in range(len(parts) - 1, -1, -1):
                        b = boxes[i]
                        if x < b[0] or x > b[1] or y < b[2] or y > b[3]:
                            continue
                        n = parts[i].hit(x, y)
                        if n is not None:
                            found = (parts[i], n)
                            break
                    if not found:
                        continue

                    part, (nx, ny, nz) = found
                    diffuse = max(0.0, nx * lx + ny * ly + nz * lz)
                    lum = (0.32 + 0.85 * diffuse) * (0.55 + 0.45 * g.depth_light(tier))
                    if part.glow:
                        lum = max(lum, 0.95)
                    r, gg, b2 = (c * lum for c in part.base)
                    if part.gloss:
                        spec = max(0.0, (nx * -0.3 + ny * -0.4 + nz * 0.85)) ** 14 * part.gloss
                        r, gg, b2 = r + 255 * spec, gg + 255 * spec, b2 + 255 * spec
                    sr, sg, sb = sr + r, sg + gg, sb + b2
                    hits += 1
                    ramp_votes[part.ramp] = ramp_votes.get(part.ramp, 0) + 1

            if hits * 2 >= S * S:
                ramp = max(ramp_votes, key=ramp_votes.get)
                pixels[(px, py)] = (palette.nearest(sr / hits, sg / hits, sb / hits, ramps=[ramp]), ramp)

    img = bytearray(WIDTH * HEIGHT)
    for (px, py), (c, ramp) in pixels.items():
        if not (0 <= px < WIDTH and 0 <= py < HEIGHT):
            continue
        # a dark edge round the outside, where it's big enough to see
        if size >= 20 and any((px + dx, py + dy) not in pixels for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))):
            c = palette.ramps[ramp][1]
        img[py * WIDTH + px] = c
    return img


def main():
    outdir = sys.argv[1]
    palette = g.Palette()
    os.makedirs(os.path.join(outdir, "monsters"), exist_ok=True)

    images = {}
    for kind, design in sorted(MONSTERS.items()):
        parts = design()
        for distance in range(1, 5):
            images[(kind, distance)] = render(parts, palette, BOUNDS[kind], distance - 1)
            g.write_indexed(os.path.join(outdir, "monsters", "monster_%d_%d.png" % (kind, distance)),
                            images[(kind, distance)], WIDTH, HEIGHT, palette)

    if len(sys.argv) > 2:
        # the monsters at the first three distances, in rows, with the gaps cropped
        kinds = sorted(MONSTERS)
        per_row = 6
        cell_w, cell_h = 150, 124
        rows = (len(kinds) + per_row - 1) // per_row
        w, h = per_row * cell_w, rows * cell_h
        rgb = bytearray(w * h * 3)
        for i, kind in enumerate(kinds):
            img = images[(kind, 1)]
            ox, oy = (i % per_row) * cell_w, (i // per_row) * cell_h
            for y in range(cell_h):
                for x in range(cell_w):
                    sx, sy = WIDTH // 2 - cell_w // 2 + x, d1.FLOOR_Y[0] - cell_h + 6 + y
                    c = img[sy * WIDTH + sx] if 0 <= sy < HEIGHT and 0 <= sx < WIDTH else 0
                    col = palette.colors[c] if c else (36, 36, 44)
                    o = ((oy + y) * w + ox + x) * 3
                    rgb[o:o + 3] = bytes(col)
        k = 2
        g.write_rgb(sys.argv[2], g.enlarge(bytes(rgb), w, h, k), w * k, h * k)


if __name__ == "__main__":
    main()
