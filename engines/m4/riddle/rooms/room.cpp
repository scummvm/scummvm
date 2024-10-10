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

#include "m4/riddle/rooms/room.h"
#include "m4/riddle/vars.h"
#include "m4/riddle/riddle.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

int Room::_ripSketching;

void Room::preload() {
	_G(player).walker_type = 1;
	_G(player).shadow_type = 1;
	LoadWSAssets("OTHER SCRIPT");
}

void Room::restoreAutosave() {
	if (g_engine->autosaveExists()) {
		_G(kernel).restore_slot = 0;
		auto oldMode = _G(kernel).trigger_mode;
		_G(kernel).trigger_mode = KT_DAEMON;
		disable_player_commands_and_fade_init(255);

		_G(kernel).trigger_mode = oldMode;
		player_set_commands_allowed(false);
	}
}

int Room::checkFlags(bool flag) {
	int count = 0;

	if (_G(flags)[V364] == 1) {
		++count;
		switch (_G(flags)[V005]) {
		case 1:
			_G(flags)[V351] = 1;
			break;
		case 2:
			_G(flags)[V352] = 1;
			break;
		case 3:
			_G(flags)[V353] = 1;
			break;
		case 4:
			_G(flags)[V354] = 1;
			break;
		default:
			break;
		}
	}

	if (_G(flags)[V365] == 1) {
		_G(flags)[V355] = 1;
		++count;
	}
	if (_G(flags)[V366] == 1) {
		_G(flags)[V356] = 1;
		++count;
	}

	if (flag) {
		if (_G(flags)[V367] == 1) {
			_G(flags)[V357] = 1;
			++count;
		}
		if (_G(flags)[V368] == 1) {
			_G(flags)[V358] = 1;
			++count;
		}
		if (_G(flags)[V369] == 1) {
			_G(flags)[V359] = 1;
			++count;
		}
	}

	if (_G(game).room_id == 501 && _G(flags)[V370] == 1) {
		_G(flags)[V360] = 1;
		++count;
	}

	if (_G(game).room_id != 401 && _G(flags)[V373] == 1) {
		_G(flags)[V363] = 1;
		++count;
	}

	if (_G(game).room_id != 501 && _G(flags)[V371] == 1) {
		_G(flags)[V361] = 1;
		++count;
	}

	if (_G(flags)[V372] == 1) {
		_G(flags)[V362] = 1;
		++count;
	}

	return count;
}

void Room::setFlag45() {
	_G(flags)[V045] = 1;
}

int Room::getNumKeyItemsPlaced() const {
	static const char *const ITEMS[11] = {
		"SHRUNKEN HEAD", "INCENSE BURNER", "CRYSTAL SKULL",
		"WHALE BONE HORN", "WHEELED TOY", "SILVER BUTTERFLY",
		"REBUS AMULET", "CHISEL", "GERMAN BANKNOTE",
		"POSTAGE STAMP", "STICK AND SHELL MAP"
	};

	int total = 0;
	for (int i = 0; i < 11; ++i) {
		if (inv_object_in_scene(ITEMS[i], 305))
			++total;
	}

	return total;
}

bool Room::setItemsPlacedFlags() {
	int count = getNumKeyItemsPlaced();

	switch (_G(flags)[V005]) {
	case 0:
		if (_G(flags)[V006] >= 2 && getNumKeyItemsPlaced() == 0) {
			_G(flags)[V005] += 1;
			return true;
		}
		break;

	case 1:
	case 2:
	case 3:
		if ((_G(flags)[V006] >= 15 && count < 9) ||
				(_G(flags)[V006] >= 10 && count < 6) ||
				(_G(flags)[V006] >= 5 && count < 3)) {
			_G(flags)[V005] += 1;
			return true;
		}
		break;

	case 4:
	case 5:
		if ((_G(flags)[V006] >= 15 && count < 9) ||
				(_G(flags)[V006] >= 10 && count < 6) ||
				(_G(flags)[V006] >= 5 && count < 3)) {
			if (_G(flags)[V005] == 4)
				_G(flags)[V005]++;
			return true;
		}
		break;

	default:
		break;
	}

	_G(flags)[GLB_TEMP_1] = 0;
	return false;
}

const char *Room::getItemsPlacedDigi() const {
	static const char *NAMES[5] = {
		"301r18", "301r19", "301r20", "301r21", "301r38"
	};
	int val = _G(flags)[V005];
	return (val >= 1 && val <= 5) ? NAMES[val - 1] : nullptr;
}

void Room::disableHotspots() {
	for (auto *hs = _G(currentSceneDef).hotspots; hs; hs = hs->next)
		hs->active = false;
}

void Room::enableHotspots() {
	for (auto *hs = _G(currentSceneDef).hotspots; hs; hs = hs->next)
		hs->active = true;
}

bool Room::checkStrings() const {
	return _G(string1).empty() && _G(string2).empty() && _G(string3).empty();
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
