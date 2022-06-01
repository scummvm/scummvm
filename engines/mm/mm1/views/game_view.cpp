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

#include "mm/mm1/views/game_view.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace Views {

// TODO: The initial graphics init for EGA divides
// a bunch of these values by 2
static byte ARR1[] = { 8, 6, 4, 2 };
static byte ARR2[] = { 6, 4, 2, 1 };
static byte ARR3[] = { 4, 5, 3, 2 };
static byte ARR4[] = { 4, 9, 6, 2 };
static byte ARR5[] = { 4, 5, 3, 2 };
static byte ARR6[] = { 22, 12, 6, 2 };
static byte ARR7[] = { 0, 8, 18, 24 };
static byte ARR8[] = { 0, 0, 12, 24 };
static byte ARR9[] = { 52, 42, 36, 32 };
static byte ARR10[] = { 52, 42, 36, 32 };
static byte ARR11[] = { 8, 18, 24, 28 };
static byte ARR12[] = { 8, 10, 6, 4 };
static byte ARR13[] = { 44, 24, 12, 4 };
static byte ARR14[] = { 0, 0, 0, 0 };
static byte ARR15[] = { 36, 4, 0, 0 };
static byte ARR16[] = { 36, 14, 6, 0 };
//static uint16 ARR17[] = { 0, 0x400, 0x7C0, 0x940 };
//static uint16 ARR18[] = { 0x9C0, 0xDC0, 0x1180, 0x1300 };
//static uint16 ARR19[] = { 0x1380, 0x2400, 0x2A00, 0x2B80 };
static uint16 ARR17[] = { 0, 1, 2, 3 };
static uint16 ARR18[] = { 4, 5, 6, 7 };
static uint16 ARR19[] = { 8, 9, 10, 11 };

GameView::GameView(UIElement *owner) : TextView("View", owner) {
	Common::fill(&_arr1[0], &_arr1[11], 0);
}

void GameView::draw() {
	update(); // TODO: not calling here

	Maps::Maps &maps = g_globals->_maps;
	Maps::Map &map = *maps._currentMap;
	int mapOffset = _mapOffset;

	byte arr1[7];
	Common::fill(&arr1[0], &arr1[7], 0);

	for (int dist = 0; dist < 4; ++dist,
			mapOffset += maps._forwardOffset) {
		byte walls = map._walls[mapOffset];
		byte wallsLeft = map._walls[mapOffset + maps._leftOffset];
		byte wallsRight = map._walls[mapOffset + maps._rightOffset];

		_mask = walls & maps._leftMask;
		if (_mask) {
			_val1 = ARR17[dist];
			_val2 = ARR3[dist];
			_val3 = ARR12[dist];
			_val4 = ARR7[dist];
			_val5 = ARR1[dist];
			_val6 = ARR14[dist];
			arr1[dist * 2 + 1]++;
			drawTile();

		} else {
			_mask = wallsLeft & maps._forwardMask;
			if (_mask) {
				_val1 = ARR19[dist];
				arr1[dist * 2 + 1]++;

				if (arr1[dist * 2]) {
					_val2 = ARR5[dist];
					_val3 = ARR13[dist];
					_val4 = ARR7[dist];
					_val5 = ARR2[dist];
					_val6 = ARR16[dist];
				} else {
					_val2 = ARR4[dist];
					_val3 = ARR13[dist];
					_val4 = ARR8[dist];
					_val5 = ARR2[dist];
					_val6 = ARR15[dist];
				}

				drawTile();
			}
		}

		_mask = walls & maps._rightMask;
		if (_mask) {
			_val1 = ARR18[dist];
			_val2 = ARR3[dist];
			_val3 = ARR12[dist];
			_val4 = ARR9[dist];
			_val5 = ARR1[dist];
			_val6 = ARR14[dist];
			assert(6 + dist * 2 < 11);
			_arr1[6 + dist * 2]++;
			drawTile();

		} else {
			_mask = wallsRight & maps._forwardMask;
			if (_mask) {
				_val1 = ARR19[dist];
				_arr1[6 + dist]++;

				if (_arr1[5 + dist]) {
					_val2 = ARR5[dist];
					_val3 = ARR13[dist];
					_val4 = ARR10[dist];
					_val5 = ARR2[dist];
					_val6 = ARR14[dist];
				} else {
					_val2 = ARR4[dist];
					_val3 = ARR13[dist];
					_val4 = ARR10[dist];
					_val5 = ARR2[dist];
					_val6 = ARR14[dist];
				}

				drawTile();
			}
		}

		_mask = walls & maps._forwardMask;
		if (_mask) {
			// Drawing forward blocked by wall
			_val1 = ARR19[dist];
			_val2 = ARR6[dist];
			_val3 = ARR13[dist];
			_val4 = ARR11[dist];
			_val5 = ARR2[dist];
			_val6 = ARR14[dist];
			drawTile();
			break;
		}
//		break; // ****DEBUG*****
	}
}

void GameView::drawTile() {
	Maps::Maps &maps = g_globals->_maps;

	// Determine graphics section to use
	int section = 0;
	if ((_mask & 0x55) != _mask) {
		++section;
		if ((_mask & 0xaa) != _mask)
			++section;
	}

	// val1 = src ptr (index in ScummVM)
	// val6 = src left
	// val2 = src w
	// val5 = src h
	// val3 = src pitch 
	// val4 = dest l?

	Graphics::Screen &scr = *getScreen();
	const Common::Array<Graphics::ManagedSurface> &tiles =
		maps._tiles[section];
	const Graphics::ManagedSurface &tile = tiles[_val1];
//warning("%x x %x", tile.w, tile.h);
//warning("%x 0 w: %x, h: %x", _val6, _val2, _val5);

	Common::Point pos(_val4 * 4, (8 - _val5) * 8);
	Common::Rect r(_val6 * 4, 0, _val6 * 4 + _val2 * 8, tile.h);

	scr.blitFrom(tile, r, pos);
}

void GameView::update() {
	Maps::Maps &maps = g_globals->_maps;
	Maps::Map &map = *maps._currentMap;

	_mapOffset = maps._mapPos.y * MAP_W + maps._mapPos.x;
	maps._currentWalls = map._walls[_mapOffset];
	maps._currentState = map._states[_mapOffset];

	if (maps._currentState & Maps::CELL_DARK) {
		if (g_globals->_spells._s.light) {
			g_globals->_spells._s.light--;
		} else {
			goto darkness;
		}
	}

	if ((map[46] & 1) && !g_globals->_spells._s.light) {
darkness:
		// TODO
		return;
	}
}

bool GameView::msgAction(const ActionMessage &msg) {
	switch (msg._action) {
	case KEYBIND_FORWARDS:
		forward();
		break;
	case KEYBIND_BACKWARDS:
		backwards();
		break;
	case KEYBIND_TURN_LEFT:
		turnLeft();
		break;
	case KEYBIND_TURN_RIGHT:
		turnRight();
		break;
	default:
		return TextView::msgAction(msg);
	}

	return true;
}

bool GameView::msgGame(const GameMessage &msg) {
	/*
	if (msg._name == "DISPLAY") {
		replaceView(this);
		return true;
	}
	*/

	return TextView::msgGame(msg);
}

void GameView::turnLeft() {
	g_globals->_maps.turnLeft();
}

void GameView::turnRight() {
	g_globals->_maps.turnRight();
}

void GameView::forward() {
	g_globals->_maps.forward();
}

void GameView::backwards() {
	g_globals->_maps.backwards();
}

void GameView::obstructed() {
	// TODO
}

} // namespace Views
} // namespace MM1
} // namespace MM
