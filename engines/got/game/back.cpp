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

#include "got/game/back.h"
#include "common/file.h"
#include "got/events.h"
#include "got/game/boss1.h"
#include "got/game/boss2.h"
#include "got/game/boss3.h"
#include "got/game/move.h"
#include "got/game/object.h"
#include "got/game/script.h"
#include "got/gfx/image.h"
#include "got/vars.h"

namespace Got {

const char *OBJECT_NAMES[] = {
	"Shrub", "Child's Doll", "UNUSED", "FUTURE",
	"FUTURE", "FUTURE", "FUTURE", "FUTURE", "FUTURE",
	"FUTURE", "FUTURE", "FUTURE", "FUTURE", "FUTURE",
	"FUTURE"};
const char *ITEM_NAMES[] = {
	"Enchanted Apple", "Lightning Power",
	"Winged Boots", "Wind Power",
	"Amulet of Protection", "Thunder Power"};

static const char *odinEndMessage;

void showLevel(const int newLevel) {
	_G(boss_active) = false;
	if (!_G(shield_on))
		_G(actor[2])._active = false;
	_G(bomb_flag) = false;
	_G(slipping) = false;

	if (_G(scrn)._iconGrid[_G(thor)->_centerY][_G(thor)->_centerX] == 154)
		_G(thor)->_dir = 0;

	// The original copied 130 bytes from _G(scrn).static_object onwards into sd_data.
	// This doesn't make sense, because that would put the ending in the middle of _staticY.
	// Plus, it follows with an entire copy of scrn into sd_data anyway, so the first
	// move seems entirely redundant.
	_G(scrn).save(_G(current_level));
	_G(scrn).load(_G(new_level));

	_G(levelMusic) = _G(scrn)._music;

	_G(thor)->_nextFrame = 0;

	showObjects();
	showEnemies();

	// The original was probably shortly displaying Thor in direction 0 before switching back to its prior position.
	// This behavior wasn't noticed during initial playthrough by Dreammaster - Warning has been added so it can be checked eventually.
	if (_G(scrn)._iconGrid[_G(thor)->_centerY][_G(thor)->_centerX] == 154)
		warning("showLevel - Potential short move missing");

	if (_G(warp_flag))
		_G(current_level) = newLevel - 5; // Force phase
	_G(warp_flag) = false;

	if (_G(warp_scroll)) {
		_G(warp_scroll) = false;
		if (_G(thor)->_dir == 0)
			_G(current_level) = newLevel + 10;
		else if (_G(thor)->_dir == 1)
			_G(current_level) = newLevel - 10;
		else if (_G(thor)->_dir == 2)
			_G(current_level) = newLevel + 1;
		else if (_G(thor)->_dir == 3)
			_G(current_level) = newLevel - 1;
	}

	if (!_G(setup)._scrollFlag)
		_G(current_level) = newLevel; // Force no scroll

	if (_G(music_current) != _G(levelMusic))
		_G(sound).musicPause();

	switch (_G(new_level) - _G(current_level)) {
	case 0:
		// Nothing to do
		showLevelDone();
		break;
	case -1:
		_G(gameMode) = MODE_AREA_CHANGE;
		_G(transitionDir) = DIR_LEFT;
		break;
	case 1:
		_G(gameMode) = MODE_AREA_CHANGE;
		_G(transitionDir) = DIR_RIGHT;
		break;
	case -10:
		_G(gameMode) = MODE_AREA_CHANGE;
		_G(transitionDir) = DIR_UP;
		break;
	case 10:
		_G(gameMode) = MODE_AREA_CHANGE;
		_G(transitionDir) = DIR_DOWN;
		break;
	default:
		_G(gameMode) = MODE_AREA_CHANGE;
		_G(transitionDir) = DIR_PHASED;
		break;
	}
}

void showLevelDone() {
	_G(current_level) = _G(new_level);

	_G(thor_info)._lastHealth = _G(thor)->_health;
	_G(thor_info)._lastMagic = _G(thor_info)._magic;
	_G(thor_info)._lastJewels = _G(thor_info)._jewels;
	_G(thor_info)._lastKeys = _G(thor_info)._keys;
	_G(thor_info)._lastScore = _G(thor_info)._score;
	_G(thor_info)._lastItem = _G(thor_info)._selectedItem;
	_G(thor_info)._lastScreen = _G(current_level);
	_G(thor_info)._lastIcon = ((_G(thor)->_x + 8) / 16) + (((_G(thor)->_y + 14) / 16) * 20);
	_G(thor_info)._lastDir = _G(thor)->_dir;
	_G(thor_info)._lastInventory = _G(thor_info)._inventory;
	_G(thor_info)._lastObject = _G(thor_info)._object;
	_G(thor_info)._lastObjectName = _G(thor_info)._objectName;

	_G(last_setup) = _G(setup);

	bool f = true;
	if (GAME1 && _G(new_level) == BOSS_LEVEL1) {
		if (!_G(setup)._bossDead[0]) {
			if (!_G(auto_load))
				boss1SetupLevel();
			f = false;
		}
	}
	if (GAME2 && _G(new_level) == BOSS_LEVEL2) {
		if (!_G(setup)._bossDead[1]) {
			if (!_G(auto_load))
				boss2SetupLevel();
			f = false;
		}
	}
	if (GAME3) {
		if (_G(new_level) == BOSS_LEVEL3) {
			if (!_G(setup)._bossDead[2]) {
				if (!_G(auto_load))
					boss3SetupLevel();
				f = false;
			}
		}
		if (_G(current_level) == ENDING_SCREEN) {
			endingScreen();
			f = false;
		}
	}

	if (_G(startup))
		f = false;

	if (f)
		musicPlay(_G(levelMusic), false);
}

static void odin_speaks_end() {
	// In case Thor is now dead, flag as such
	if (!_G(thor)->_health) {
		_G(thor)->_show = 0;
		_G(exit_flag) = 2;
	}

	// If there's an end message, pass it on to the view hierarchy.
	// This is used in cases like the game end where multiple
	// odinSpeaks are done in sequence
	if (odinEndMessage)
		g_events->send(GameMessage(odinEndMessage));
}

void odinSpeaks(const int index, int item, const char *endMessage) {
	odinEndMessage = endMessage;

	executeScript((long)index, _G(odin), odin_speaks_end);
}

int switchIcons() {
	playSound(WOOP, false);

	for (int y = 0; y < 12; y++) {
		for (int x = 0; x < 20; x++) {
			const int ix = x * 16;
			const int iy = y * 16;
			if (_G(scrn)._iconGrid[y][x] == 93) {
				placeTile(x, y, 144);
			} else if (_G(scrn)._iconGrid[y][x] == 144) {
				placeTile(x, y, 93);
				killEnemies(iy, ix);
			}
			
			if (_G(scrn)._iconGrid[y][x] == 94) {
				placeTile(x, y, 146);
			} else if (_G(scrn)._iconGrid[y][x] == 146) {
				placeTile(x, y, 94);
				killEnemies(iy, ix);
			}
		}
	}

	return 0;
}

int rotateArrows() {
	playSound(WOOP, false);

	for (int y = 0; y < 12; y++) {
		for (int x = 0; x < 20; x++) {
			if (_G(scrn)._iconGrid[y][x] == 205)
				placeTile(x, y, 208);
			else if (_G(scrn)._iconGrid[y][x] == 206)
				placeTile(x, y, 207);
			else if (_G(scrn)._iconGrid[y][x] == 207)
				placeTile(x, y, 205);
			else if (_G(scrn)._iconGrid[y][x] == 208)
				placeTile(x, y, 206);
		}
	}

	return 0;
}

void killEnemies(const int iy, const int ix) {
	int x1, y1, x2, y2;

	for (int i = 3; i < MAX_ACTORS; i++) {
		if (_G(actor[i])._active) {
			x1 = _G(actor[i])._x;
			y1 = _G(actor[i])._y + _G(actor[i])._sizeY - 2;
			x2 = (_G(actor[i])._x + _G(actor[i])._sizeX);
			y2 = _G(actor[i])._y + _G(actor[i])._sizeY - 1;

			if (pointWithin(x1, y1, ix, iy, ix + 15, iy + 15))
				actorDestroyed(&_G(actor[i]));
			else if (pointWithin(x2, y1, ix, iy, ix + 15, iy + 15))
				actorDestroyed(&_G(actor[i]));
			else if (pointWithin(x1, y2, ix, iy, ix + 15, iy + 15))
				actorDestroyed(&_G(actor[i]));
			else if (pointWithin(x2, y2, ix, iy, ix + 15, iy + 15))
				actorDestroyed(&_G(actor[i]));
		}
	}

	x1 = _G(thor)->_x;
	y1 = _G(thor)->_y + 11;
	x2 = x1 + 13;
	y2 = y1 + 5;

	if (pointWithin(x1, y1, ix, iy, ix + 15, iy + 15) || pointWithin(x2, y1, ix, iy, ix + 15, iy + 15) || pointWithin(x1, y2, ix, iy, ix + 15, iy + 15) || pointWithin(x2, y2, ix, iy, ix + 15, iy + 15)) {
		if (!_G(cheats)._freezeHealth) {
			_G(thor)->_health = 0;
			g_events->send(GameMessage("THOR_DIES"));
		}
	}
}

void removeObjects(const int y, const int x) {
	const int p = (y * 20) + x;

	if (_G(object_map[p]) > 0) {
		_G(object_map[p]) = 0;
		_G(object_index[p]) = 0;
	}
}

void placeTile(const int x, const int y, const int tile) {
	_G(scrn)._iconGrid[y][x] = tile;
	removeObjects(y, x);
}

int backgroundTile(int x, int y) {
	if (x < 0 || x >= 319 || y < 0 || y >= 191)
		return 0;

	x = (x + 1) >> 4;
	y = (y + 1) >> 4;

	return _G(scrn)._iconGrid[y][x];
}

void selectItem() {
	// Only allow opening the dialog if something isn't currently going on
	if (g_engine->canSaveAutosaveCurrently()) {
		g_events->addView("SelectItem");
	}
}

void actorSpeaks(const Actor *actor, int index, int item) {
	if (actor->_type != 4)
		return;

	const int v = atoi(actor->_name);
	if (v < 1 || v > 20)
		return;

	long lind = (long)_G(current_level);
	lind = lind * 1000;
	lind += (long)actor->_actorNum;

	const Common::String str = Common::String::format("FACE%d", v);
	if (Common::File::exists(Common::Path(str))) {
		Gfx::Pics pics(str, 262);
		executeScript(lind, pics);
	} else {
		executeScript(lind, _G(odin));
	}

	if (!_G(thor)->_health) {
		_G(thor)->_show = 0;
		_G(exit_flag) = 2;
	}
}

} // namespace Got
