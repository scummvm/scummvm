/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/file.h"
#include "common/system.h"
#include "graphics/paletteman.h"
#include "ultima/ultima2/data/data.h"
#include "ultima/ultima2/gfx/pic_decoder.h"
#include "ultima/ultima2/views/dungeon_map.h"
#include "ultima/ultima2/ultima2.h"
#include "ultima/ultima2/metaengine.h"

namespace Ultima {
namespace Ultima2 {
namespace Views {

namespace {

// The corridor is drawn in a 256x128 area, offset from the top left corner
constexpr int VIEW_OFFSET = 16;
constexpr int COLOR_LINE = 3;
constexpr int MAX_DEPTH = 8;

// Cell terrain bits
constexpr byte TERRAIN_WALL = 0x80;
constexpr byte TERRAIN_DOOR = 0x40;
constexpr byte TERRAIN_SECRET = 0x20;
constexpr byte TERRAIN_LADDER_DOWN = 0x20;
constexpr byte TERRAIN_LADDER_UP = 0x10;
constexpr byte TERRAIN_CHEST = 0x40;

// Wall corner positions for each depth, one table per edge or feature.
// The tables sit back to back as in the original, and the drawing routines
// index some of them past their eighth entry, so they're kept as one block
constexpr int TABLE_BASE = 0x1620D;
const byte TABLE[] = {
	// 1620D
	0x00, 0x40, 0x60, 0x70, 0x78, 0x7C, 0x7E, 0x7F, 0x7F, 0x7F,
	// 16217
	0xFF, 0xBF, 0x9F, 0x8F, 0x87, 0x83, 0x81, 0x80, 0x80, 0x80,
	// 16221
	0x00, 0x20, 0x30, 0x38, 0x3C, 0x3E, 0x3F, 0x3F,
	// 16229
	0x7F, 0x5F, 0x4F, 0x47, 0x43, 0x41, 0x40, 0x40,
	// 16231
	0x10, 0x48, 0x64, 0x72, 0x79, 0x7C, 0x7F, 0x7F,
	// 16239
	0x30, 0x58, 0x6C, 0x76, 0x7B, 0x7C, 0x7F, 0x7F,
	// 16241
	0xEF, 0xB7, 0x9B, 0x8D, 0x86, 0x83, 0x80, 0x80,
	// 16249
	0xCF, 0xA7, 0x93, 0x89, 0x84, 0x83, 0x80, 0x80,
	// 16251
	0x77, 0x5B, 0x4D, 0x46, 0x42, 0x40, 0x40, 0x40,
	// 16259
	0x67, 0x53, 0x49, 0x44, 0x42, 0x40, 0x40, 0x40,
	// 16261
	0x2A, 0x35, 0x3B, 0x3D, 0x3E, 0x3F, 0x3F, 0x3F,
	// 16269
	0x08, 0x24, 0x32, 0x39, 0x3C, 0x3E, 0x3F, 0x3F,
	// 16271
	0x18, 0x2C, 0x36, 0x3A, 0x3D, 0x3E, 0x3F, 0x3F,
	// 16279
	0x77, 0x5B, 0x4D, 0x46, 0x43, 0x41, 0x40, 0x40,
	// 16281
	0x67, 0x53, 0x49, 0x45, 0x42, 0x41, 0x40, 0x40,
	// 16289
	0x6F, 0x57, 0x4B, 0x45, 0x42, 0x40, 0x40, 0x40
};

// Table addresses, named for the edge they hold
enum {
	LEFT_X = 0x1620D,        // x of the left edge at each depth
	LEFT_X_NEXT = 0x1620E,   // ... and at the next depth in
	LEFT_X_NEXT2 = 0x1620F,
	LEFT_X_NEXT3 = 0x16210,
	RIGHT_X = 0x16217,
	RIGHT_X_NEXT = 0x16218,
	RIGHT_X_NEXT2 = 0x16219,
	RIGHT_X_NEXT3 = 0x1621A,
	TOP_Y = 0x16221,
	TOP_Y_NEXT = 0x16222,
	BOTTOM_Y = 0x16229,
	BOTTOM_Y_NEXT = 0x1622A,
	LADDER_LEFT_A = 0x16232,
	LADDER_LEFT_B = 0x1623A,
	LADDER_RIGHT_B = 0x16242,
	LADDER_RIGHT_A = 0x1624A,
	LEFT_DOOR_X = 0x16231,
	LEFT_DOOR_X2 = 0x16239,
	AHEAD_DOOR_X = 0x16241,
	AHEAD_DOOR_X2 = 0x16249,
	DOOR_BOTTOM_A = 0x16251,
	DOOR_BOTTOM_B = 0x16259,
	DOOR_TOP_B = 0x16261,
	RAIL_TOP_A = 0x16269,
	RAIL_TOP_B = 0x16271,
	RAIL_BOTTOM_A = 0x16279,
	RAIL_BOTTOM_B = 0x16281,
	CHEST_BOTTOM = 0x16289
};

// The four-pixel patterns a monster sprite's blocks are drawn with
const byte SPRITE_PATTERNS[8] = { 0xFF, 0x11, 0x88, 0xCC, 0x33, 0x00, 0x00, 0x00 };

// Where a monster's sprite starts within its data page, by depth. The
// nearest depth uses a page of its own per monster type instead
const byte SPRITE_BAND_OFFSETS[MAX_DEPTH] = { 0, 0, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

/**
 * Draws lines the way the original does. Its routines share one pair of
 * end points that each line leaves standing at its far end, so the wall
 * shapes below only set the coordinates that change between lines
 */
class Painter {
private:
	Shared::Gfx::GfxSurface &_s;

	void plot(byte x, byte y) {
		int px = x + VIEW_OFFSET, py = y + VIEW_OFFSET;
		if (px < 320 && py < 160)
			_s.setPixel(px, py, COLOR_LINE);
	}

public:
	byte _x1 = 0, _y1 = 0, _x2 = 0, _y2 = 0;
	int _depth = 0;

	Painter(Shared::Gfx::GfxSurface &s) : _s(s) {
	}

	/**
	 * Returns an entry of the corner position tables for the current depth
	 */
	byte t(int address) const {
		return TABLE[address - TABLE_BASE + _depth];
	}

	void line() {
		if (_x1 == _x2 && _y1 == _y2)
			return;

		byte adx, ady;
		int sx, sy;
		if (_x2 >= _x1) {
			adx = _x2 - _x1;
			sx = 1;
		} else {
			adx = _x1 - _x2;
			sx = -1;
		}
		if (_y2 >= _y1) {
			ady = _y2 - _y1;
			sy = 1;
		} else {
			ady = _y1 - _y2;
			sy = -1;
		}

		if (adx >= ady) {
			byte count = adx, err = adx >> 1;
			do {
				err += ady;
				if (err >= adx) {
					err -= adx;
					_y1 += sy;
				}
				_x1 += sx;
				plot(_x1, _y1);
			} while (--count != 0);
		} else {
			byte count = ady, err = ady >> 1;
			do {
				err += adx;
				if (err >= ady) {
					err -= ady;
					_x1 += sx;
				}
				_y1 += sy;
				plot(_x1, _y1);
			} while (--count != 0);
		}
	}

	void leftOpen() {
		_x1 = _x2 = t(LEFT_X);
		_y1 = t(TOP_Y);
		_y2 = t(BOTTOM_Y);
		line();
		_y1 = _y2 = t(TOP_Y_NEXT);
		_x2 = t(LEFT_X_NEXT);
		line();
		_y2 = t(BOTTOM_Y_NEXT);
		line();
		_x2 = t(LEFT_X);
		line();
	}

	void rightOpen() {
		_x1 = _x2 = t(RIGHT_X);
		_y1 = t(TOP_Y);
		_y2 = t(BOTTOM_Y);
		line();
		_y1 = _y2 = t(TOP_Y_NEXT);
		_x2 = t(RIGHT_X_NEXT);
		line();
		_y2 = t(BOTTOM_Y_NEXT);
		line();
		_x2 = t(RIGHT_X);
		line();
	}

	void leftWall() {
		_x1 = t(LEFT_X);
		_x2 = t(LEFT_X_NEXT);
		_y1 = t(TOP_Y);
		_y2 = t(TOP_Y_NEXT);
		line();
		_x2 = t(LEFT_X);
		_y2 = t(BOTTOM_Y);
		_y1 = t(BOTTOM_Y_NEXT);
		line();
	}

	void rightWall() {
		_x1 = t(RIGHT_X);
		_x2 = t(RIGHT_X_NEXT);
		_y1 = t(TOP_Y);
		_y2 = t(TOP_Y_NEXT);
		line();
		_x2 = t(RIGHT_X);
		_y2 = t(BOTTOM_Y);
		_y1 = t(BOTTOM_Y_NEXT);
		line();
	}

	void frontWall() {
		_x1 = t(LEFT_X);
		_y1 = t(TOP_Y);
		_y2 = t(TOP_Y);
		_x2 = t(RIGHT_X);
		line();
		_y2 = t(BOTTOM_Y);
		line();
		_x2 = t(LEFT_X);
		line();
		_y2 = t(TOP_Y);
		line();
	}

	void frontDoor() {
		_x1 = _x2 = t(LEFT_X_NEXT);
		_y1 = t(BOTTOM_Y);
		_y2 = t(TOP_Y_NEXT);
		line();
		_x2 = t(RIGHT_X_NEXT);
		line();
		_y2 = t(BOTTOM_Y);
		line();
	}

	void leftDoor() {
		_x1 = _x2 = t(LEFT_DOOR_X);
		_y1 = t(DOOR_BOTTOM_A);
		_y2 = t(TOP_Y_NEXT);
		line();
		_x2 = t(LEFT_DOOR_X2);
		_y2 = t(DOOR_TOP_B);
		line();
		_y2 = t(DOOR_BOTTOM_B);
		line();
	}

	void rightDoor() {
		_x1 = _x2 = t(AHEAD_DOOR_X);
		_y1 = t(DOOR_BOTTOM_A);
		_y2 = t(TOP_Y_NEXT);
		line();
		_x2 = t(AHEAD_DOOR_X2);
		_y2 = t(DOOR_TOP_B);
		line();
		_y2 = t(DOOR_BOTTOM_B);
		line();
	}

	void ladderRails() {
		_x1 = _x2 = 0x7F;
		_y1 = t(RAIL_TOP_A);
		_y2 = t(RAIL_BOTTOM_A);
		line();
		_x1 = _x2 = 0x80;
		_y1 = t(RAIL_TOP_A);
		_y2 = t(RAIL_BOTTOM_A);
		line();
	}

	void ladderDown() {
		_x1 = t(LADDER_LEFT_A);
		_x2 = t(LADDER_LEFT_B);
		_y1 = t(RAIL_BOTTOM_A);
		_y2 = t(RAIL_BOTTOM_B);
		line();
		_x2 = t(LADDER_RIGHT_A);
		line();
		_x2 = t(LADDER_RIGHT_B);
		_y2 = t(RAIL_BOTTOM_A);
		line();
		_x2 = t(LADDER_LEFT_A);
		line();
		ladderRails();
	}

	void ladderUp() {
		_x1 = t(LADDER_LEFT_A);
		_x2 = t(LADDER_LEFT_B);
		_y1 = t(RAIL_TOP_A);
		_y2 = t(RAIL_TOP_B);
		line();
		_x2 = t(LADDER_RIGHT_A);
		line();
		_x2 = t(LADDER_RIGHT_B);
		_y2 = t(RAIL_TOP_A);
		line();
		_x2 = t(LADDER_LEFT_A);
		line();
		ladderRails();
	}

	void chest() {
		_x1 = t(LEFT_X_NEXT2);
		_y1 = _y2 = t(CHEST_BOTTOM);
		_x2 = t(RIGHT_X_NEXT3);
		line();
		_y2 = t(DOOR_BOTTOM_A);
		line();
		_x2 = t(LEFT_X_NEXT2);
		line();
		_y2 = t(CHEST_BOTTOM);
		line();
		_x2 = t(LEFT_X_NEXT3);
		_y2 = t(DOOR_BOTTOM_B);
		line();
		_x2 = t(RIGHT_X_NEXT2);
		line();
		_y2 = t(CHEST_BOTTOM);
		line();
		_x2 = t(RIGHT_X_NEXT3);
		_y2 = t(DOOR_BOTTOM_A);
		line();
		_x1 = t(RIGHT_X_NEXT3);
		_y1 = t(CHEST_BOTTOM);
		_x2 = t(RIGHT_X_NEXT2);
		_y2 = t(DOOR_BOTTOM_B);
		line();
	}
};

} // namespace

DungeonMap::DungeonMap() : Map("DungeonMap") {
	Common::File f;
	if (f.open("MONSTERS"))
		f.read(_monsterSprites, sizeof(_monsterSprites));
	else
		warning("Could not open MONSTERS");
}

bool DungeonMap::msgFocus(const FocusMessage &msg) {
	MetaEngine::setKeybindingMode(KBMODE_GAMEPLAY);
	g_system->getPaletteManager()->setPalette(Graphics::Palette(Data::CGA_PALETTE1, 4));
	return Map::msgFocus(msg);
}

bool DungeonMap::msgUnfocus(const UnfocusMessage &msg) {
	MetaEngine::setKeybindingMode(KBMODE_MINIMAL);
	return Map::msgUnfocus(msg);
}

bool DungeonMap::tick() {
	// There's no water or forcefield to animate here
	return Shared::Gfx::View::tick();
}

int DungeonMap::drawCorridor(Shared::Gfx::GfxSurface &s) {
	Data::Savegame &sg = _G(savegame);
	Data::MapDungeon &dungeon = _G(dungeon);
	int level = sg._dungeonLevel;
	int fx = sg._facingX, fy = sg._facingY;

	// Read the cells down the corridor and to either side of it
	byte left[MAX_DEPTH], center[MAX_DEPTH], right[MAX_DEPTH], monsters[MAX_DEPTH];
	byte lx = sg._mapX + fy, ly = sg._mapY - fx;
	byte cx = sg._mapX, cy = sg._mapY;
	byte rx = sg._mapX - fy, ry = sg._mapY + fx;

	for (int d = 0; d < MAX_DEPTH; ++d) {
		byte x = lx & 0x3F, y = ly & 0x3F;
		left[d] = dungeon.cell(level, x, y) & 0xF0;
		lx = x + fx;
		ly = y + fy;

		x = cx & 0x3F;
		y = cy & 0x3F;
		byte c = dungeon.cell(level, x, y);
		center[d] = c & 0xF0;
		monsters[d] = c & 7;
		cx = x + fx;
		cy = y + fy;

		x = rx & 0x3F;
		y = ry & 0x3F;
		right[d] = dungeon.cell(level, x, y) & 0xF0;
		rx = x + fx;
		ry = y + fy;
	}

	Painter p(s);
	p._depth = 0;
	p.frontWall();

	int d = 0;
	for (;;) {
		p._depth = d;
		byte c = center[d];

		if (c & TERRAIN_WALL) {
			p.frontWall();
			if ((c & TERRAIN_DOOR) == 0 || (c & TERRAIN_SECRET) != 0)
				break;

			p.frontDoor();
			if (d != 0 || ++d >= MAX_DEPTH)
				break;
			continue;
		}

		if (c & TERRAIN_LADDER_DOWN)
			p.ladderDown();
		if (c & TERRAIN_LADDER_UP)
			p.ladderUp();
		else if (c & TERRAIN_CHEST)
			p.chest();

		byte l = left[d];
		if (l & TERRAIN_WALL) {
			p.leftWall();
			if ((l & TERRAIN_DOOR) && !(l & TERRAIN_SECRET))
				p.leftDoor();
		} else {
			p.leftOpen();
		}

		byte r = right[d];
		if (r & TERRAIN_WALL) {
			p.rightWall();
			if ((r & TERRAIN_DOOR) && !(r & TERRAIN_SECRET))
				p.rightDoor();
		} else {
			p.rightOpen();
		}

		if (++d >= MAX_DEPTH - 1)
			break;
	}

	drawMonster(s, monsters, d + 1);
	return d;
}

void DungeonMap::drawMonster(Shared::Gfx::GfxSurface &s, const byte *monsters, int depth) {
	for (int d = 1; d < depth && d < MAX_DEPTH; ++d) {
		if (monsters[d] == 0)
			continue;

		// Only the nearest monster in view is drawn
		int offset = (d == 1) ? monsters[d] * 256 : SPRITE_BAND_OFFSETS[d];
		while (offset + 1 < (int)sizeof(_monsterSprites) && _monsterSprites[offset] != 0) {
			int x = (_monsterSprites[offset] + 4) * 4;
			int y = (_monsterSprites[offset + 1] & 0x1F) * 4 + VIEW_OFFSET;
			byte pattern = SPRITE_PATTERNS[_monsterSprites[offset + 1] >> 5];
			offset += 2;

			for (int row = 0; row < 4; ++row) {
				for (int col = 0; col < 4; ++col)
					s.setPixel(x + col, y + row, (pattern >> ((3 - col) * 2)) & 3);
			}
		}

		return;
	}
}

void DungeonMap::draw() {
	auto s = getSurface();
	s.clear();

	// Nothing can be seen without a light
	if (_G(savegame)._lightTurns > 0)
		drawCorridor(s);
}

} // namespace Views
} // namespace Ultima2
} // namespace Ultima
