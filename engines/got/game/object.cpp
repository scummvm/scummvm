/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property _G(of) its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms _G(of) the GNU General Public License as published by
 * the Free Software Foundation, either version 3 _G(of) the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty _G(of)
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy _G(of) the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "got/game/object.h"
#include "got/data/flags.h"
#include "got/events.h"
#include "got/game/back.h"
#include "got/game/move.h"
#include "got/game/status.h"
#include "got/gfx/image.h"
#include "got/vars.h"

namespace Got {

void not_enough_magic();
void cannot_carry_more();

void show_objects() {
	Common::fill(_G(object_map), _G(object_map) + TILES_COUNT, 0);
	Common::fill(_G(object_index), _G(object_index) + TILES_COUNT, 0);

	for (int i = 0; i < OBJECTS_COUNT; i++) {
		if (_G(scrn).static_obj[i]) {
			int p = _G(scrn).static_x[i] + (_G(scrn).static_y[i] * TILES_X);
			_G(object_index[p]) = i;
			_G(object_map[p]) = _G(scrn).static_obj[i];
		}
	}
}

void pick_up_object(int p) {
	int s; // CHECKME : Maybe it should be initialized to 0, and the assignment before the check should be remove

	switch (_G(object_map[p])) {
	case 1: // Red jewel
		if (_G(thor_info).jewels >= 999) {
			cannot_carry_more();
			return;
		}
		add_jewels(10);
		break;
	case 2: // Blue jewel
		if (_G(thor_info).jewels >= 999) {
			cannot_carry_more();
			return;
		}
		add_jewels(1);
		break;
	case 3: // Red potion
		if (_G(thor_info).magic >= 150) {
			cannot_carry_more();
			return;
		}
		add_magic(10);
		break;
	case 4: // Blue potion
		if (_G(thor_info).magic >= 150) {
			cannot_carry_more();
			return;
		}
		add_magic(3);
		break;
	case 5: // Good apple
		if (_G(thor)->_health >= 150) {
			cannot_carry_more();
			return;
		}
		play_sound(GULP, false);
		add_health(5);
		break;
	case 6: // Bad apple
		play_sound(OW, false);
		add_health(-10);
		break;
	case 7: // Key (reset on exit)
		add_keys(1);
		break;
	case 8: // Treasure
		if (_G(thor_info).jewels >= 999) {
			cannot_carry_more();
			return;
		}
		add_jewels(50);
		break;
	case 9: // Trophy
		add_score(100);
		break;
	case 10: // Crown
		add_score(1000);
		break;
	case 12:
	case 13:
	case 14:
	case 15:
	case 16:
	case 17:
	case 18:
	case 19:
	case 20:
	case 21:
	case 22:
	case 23:
	case 24:
	case 25:
	case 26:
		if (_G(object_map[p]) == 13 && HERMIT_HAS_DOLL)
			return;
		_G(thor)->_numMoves = 1;
		_G(hammer)->_numMoves = 2;
		_G(actor[2])._active = 0;
		_G(shield_on) = false;
		_G(tornado_used) = false;
		_G(thor_info).inventory |= 64;
		_G(thor_info).item = 7;
		_G(thor_info).object = _G(object_map[p]) - 11;
		_G(thor_info).object_name = OBJECT_NAMES[_G(thor_info).object - 1];
		odin_speaks((_G(object_map[p]) - 12) + 501, _G(object_map[p]) - 1);
		break;
	case 27:
	case 28:
	case 29:
	case 30:
	case 31:
	case 32:
		_G(hourglass_flag) = 0;
		_G(thunder_flag) = 0;
		_G(shield_on) = false;
		_G(lightning_used) = false;
		_G(tornado_used) = false;
		_G(hammer)->_numMoves = 2;
		_G(thor)->_numMoves = 1;
		_G(actor[2])._active = 0;
		s = 1 << (_G(object_map[p]) - 27);
		_G(thor_info).inventory |= s;
		odin_speaks((_G(object_map[p]) - 27) + 516, _G(object_map[p]) - 1);
		_G(thor_info).item = _G(object_map[p]) - 26;
		add_magic(150);
		fill_score(5);
		break;
	}
	int x = p % 20;
	int y = p / 20;

	_G(ox) = x * 16;
	_G(oy) = y * 16;
	_G(of) = 1;

	play_sound(YAH, false);
	_G(object_map[p]) = 0;

	// Reset so it doesn't reappear on reentry to screen
	if (_G(object_index[p]) < 30)
		_G(scrn).static_obj[_G(object_index[p])] = 0;
	
	_G(object_index[p]) = 0;
}

int drop_object(Actor *actr) {
	int o;

	int rnd1 = g_events->getRandomNumber(99);
	int rnd2 = g_events->getRandomNumber(99);

	if (rnd1 < 25)
		o = 5; // Apple
	else if (rnd1 & 1) {
		// Jewels
		if (rnd2 < 10)
			o = 1; // Red
		else
			o = 2; // Blue
	} else {
		// Potion
		if (rnd2 < 10)
			o = 3; // Red
		else
			o = 4; // Blue
	}

	_drop_obj(actr, o);
	return 1;
}

int _drop_obj(Actor *actr, int o) {
	int p = (actr->_x + (actr->_sizeX / 2)) / 16 + (((actr->_y + (actr->_sizeY / 2)) / 16) * 20);
	if (!_G(object_map[p]) && _G(scrn).icon[p / 20][p % 20] >= 140) { //nothing there and solid
		_G(object_map[p]) = o;
		_G(object_index[p]) = 27 + actr->_actorNum; //actor is 3-15

		return 1;
	}

	return 0;
}

int use_apple(int flag) {
	if (_G(thor)->_health == 150)
		return 0;

	if (flag && _G(thor_info).magic > 0) {
		if (!_G(apple_flag)) {
			_G(magic_cnt) = 0;
			add_magic(-2);
			add_health(1);
			play_sound(ANGEL, false);
		} else if (_G(magic_cnt) > 8) {
			_G(magic_cnt) = 0;
			add_magic(-2);
			add_health(1);
			if (!sound_playing())
				play_sound(ANGEL, false);
		}
		_G(apple_flag) = true;
		return 1;
	}

	_G(apple_flag) = false;
	if (flag)
		not_enough_magic();

	return 0;
}

int use_thunder(int flag) {

	if (flag && _G(thor_info).magic > 29) {
		if (!_G(thunder_flag)) {
			add_magic(-30);
			play_sound(THUNDER, false);
			_G(thunder_flag) = 60;
		}
		return 1;
	}

	if (flag && !_G(thunder_flag))
		not_enough_magic();

	if (_G(thunder_flag))
		return 1;
	return 0;
}

int use_boots(int flag) {
	if (flag) {
		if (_G(thor_info).magic > 0) {
			if (_G(thor)->_numMoves == 1) {
				_G(magic_cnt) = 0;
				add_magic(-1);
			} else if (_G(magic_cnt) > 8) {
				_G(magic_cnt) = 0;
				add_magic(-1);
			}
			_G(thor)->_numMoves = 2;
			_G(hammer)->_numMoves = 3;
			return 1;
		}

		not_enough_magic();
		_G(thor)->_numMoves = 1;
		_G(hammer)->_numMoves = 2;

	} else {
		_G(thor)->_numMoves = 1;
		_G(hammer)->_numMoves = 2;
	}
	return 0;
}

int use_shield(int flag) {
	if (flag) {
		if (_G(thor_info).magic) {
			if (!_G(shield_on)) {
				_G(magic_cnt) = 0;
				add_magic(-1);
				setup_magic_item(1);

				_G(actor[2]) = _G(magic_item[1]);
				setup_actor(&_G(actor[2]), 2, 0, _G(thor)->_x, _G(thor)->_y);
				_G(actor[2])._moveCountdown = 1;
				_G(actor[2])._speed = 1;
				_G(shield_on) = true;
			} else if (_G(magic_cnt) > 8) {
				_G(magic_cnt) = 0;
				add_magic(-1);
			}

			return 1;
		}

		not_enough_magic();
	}

	if (_G(shield_on)) {
		_G(actor[2])._dead = 2;
		_G(actor[2])._active = 0;
		_G(shield_on) = false;
	}

	return 0;
}

int use_lightning(int flag) {
	if (flag) {
		if (_G(thor_info).magic > 14) {
			add_magic(-15);
			g_events->send("Game", GameMessage("THROW_LIGHTNING"));
		} else {
			not_enough_magic();
			return 0;
		}
	}
	return 1;
}

int use_tornado(int flag) {
	if (flag) {
		if (_G(thor_info).magic > 10) {
			if (!_G(tornado_used) && !_G(actor[2])._dead && _G(magic_cnt) > 20) {
				_G(magic_cnt) = 0;
				add_magic(-10);
				setup_magic_item(0);
				_G(actor[2]) = _G(magic_item[0]);

				setup_actor(&_G(actor[2]), 2, 0, _G(thor)->_x, _G(thor)->_y);
				_G(actor[2])._lastDir = _G(thor)->_dir;
				_G(actor[2])._moveType = 16;
				_G(tornado_used) = true;
				play_sound(WIND, false);
			}
		} else if (!_G(tornado_used)) {
			not_enough_magic();
			return 0;
		}
		if (_G(magic_cnt) > 8) {
			if (_G(tornado_used)) {
				_G(magic_cnt) = 0;
				add_magic(-1);
			}
		}
		if (_G(thor_info).magic < 1) {
			actor_destroyed(&_G(actor[2]));
			_G(tornado_used) = false;
			not_enough_magic();
			return 0;
		}
		return 1;
	}
	return 0;
}

int use_object(int flag) {
	if (!flag)
		return 0;
	if (!(_G(thor_info).inventory & 64))
		return 0;

	odin_speaks((_G(thor_info).object - 1) + 5501, _G(thor_info).object - 1);
	return 1;
}

void use_item() {
	int ret = 0;

	int kf = _G(key_flag[key_magic]);

	if (!kf && _G(tornado_used)) {
		actor_destroyed(&_G(actor[2]));
		_G(tornado_used) = false;
	}

	bool mf = _G(magic_inform);
	switch (_G(thor_info).item) {
	case 1:
		ret = use_apple(kf);
		break;
	case 2:
		ret = use_lightning(kf);
		break;
	case 3:
		ret = use_boots(kf);
		break;
	case 4:
		ret = use_tornado(kf);
		break;
	case 5:
		ret = use_shield(kf);
		break;
	case 6:
		ret = use_thunder(kf);
		break;
	case 7:
		ret = use_object(kf);
		break;
	}
	if (kf) {
		if ((!ret) && (!_G(useItemFlag))) {
			if (mf)
				play_sound(BRAAPP, false);
			_G(useItemFlag) = true;
		}
	} else {
		_G(useItemFlag) = false;
	}
}

void not_enough_magic() {
	if (!_G(magic_inform))
		odin_speaks(2006, 0);

	_G(magic_inform) = true;
}

void cannot_carry_more() {
	if (!_G(carry_inform))
		odin_speaks(2007, 0);
	_G(carry_inform) = true;
}

void delete_object() {

	_G(thor_info).inventory &= 0xbf;
	_G(thor_info).item = 1;
}

} // namespace Got
