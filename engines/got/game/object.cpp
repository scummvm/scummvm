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
#include "got/game/back.h"
#include "got/game/move.h"
#include "got/game/panel.h"
#include "got/game/status.h"
#include "got/data/flags.h"
#include "got/gfx/image.h"
#include "got/events.h"
#include "got/vars.h"

namespace Got {

void not_enough_magic();
void cannot_carry_more();

void pick_up_object(int p) {
	int r, x, y, s;

	switch (_G(object_map)[p]) {
	case 1:           //red jewel
		if (_G(thor_info).jewels >= 999) {
			cannot_carry_more();
			return;
		}
		add_jewels(10);
		break;
	case 2:           //blue jewel
		if (_G(thor_info).jewels >= 999) {
			cannot_carry_more();
			return;
		}
		add_jewels(1);
		break;
	case 3:           //red potion
		if (_G(thor_info).magic >= 150) {
			cannot_carry_more();
			return;
		}
		add_magic(10);
		break;
	case 4:           //blue potion
		if (_G(thor_info).magic >= 150) {
			cannot_carry_more();
			return;
		}
		add_magic(3);
		break;
	case 5:          //good apple
		if (_G(thor)->health >= 150) {
			cannot_carry_more();
			return;
		}
		play_sound(GULP, 0);
		s = 1;
		add_health(5);
		break;
	case 6:           //bad apple
		play_sound(OW, 0);
		s = 1;
		add_health(-10);
		break;
	case 7:           //key (reset on exit)
		//           if(_G(scrn).reset) r=0;
		add_keys(1);
		break;
	case 8:          //treasure
		if (_G(thor_info).jewels >= 999) {
			cannot_carry_more();
			return;
		}
		add_jewels(50);
		break;
	case 9:          //trophy
		add_score(100);
		break;
	case 10:         //crown
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
		if (_G(object_map)[p] == 13 && HERMIT_HAS_DOLL) return;
		_G(thor)->num_moves = 1;
		_G(hammer)->num_moves = 2;
		_G(actor)[2].used = 0;
		_G(shield_on) = 0;
		_G(tornado_used) = 0;
		_G(thor_info).inventory |= 64;
		_G(thor_info).item = 7;
		_G(thor_info).object = _G(object_map)[p] - 11;
		display_item();
		_G(thor_info).object_name = OBJECT_NAMES[_G(thor_info).object - 1];
		odin_speaks((_G(object_map)[p] - 12) + 501, _G(object_map)[p] - 1);
		break;
	case 27:
	case 28:
	case 29:
	case 30:
	case 31:
	case 32:
		_G(hourglass_flag) = 0;
		_G(thunder_flag) = 0;
		_G(shield_on) = 0;
		_G(lightning_used) = 0;
		_G(tornado_used) = 0;
		_G(hammer)->num_moves = 2;
		_G(thor)->num_moves = 1;
		_G(actor)[2].used = 0;
		s = 1 << (_G(object_map)[p] - 27);
		_G(thor_info).inventory |= s;
		odin_speaks((_G(object_map)[p] - 27) + 516, _G(object_map)[p] - 1);
		s = 1;
		_G(thor_info).item = _G(object_map)[p] - 26;
		display_item();
		add_magic(150);
		fill_score(5);
		break;
	}
	x = p % 20;
	y = p / 20;

	_G(ox) = x * 16;
	_G(oy) = y * 16;
	_G(of) = 1;

	//xfput(_G(ox), _G(oy), PAGE2, (char far *) (bg_pics + (_G(scrn).bg_color * 262)));
	//xfput(_G(ox), _G(oy), PAGE2, (char far *) (bg_pics + (_G(scrn).icon[y][x] * 262)));
	//xcopyd2d(_G(ox), _G(oy), _G(ox) + 16, _G(oy) + 16, _G(ox), _G(oy), PAGE2, draw_page, 320, 320);

	r = 1;
	s = 0;
	if (!s)
		play_sound(YAH, 0);
	_G(object_map)[p] = 0;

	if (r) {
		// reset so it doesn't reappear on reentry to screen
		if (_G(object_index)[p] < 30)
			_G(scrn).static_obj[_G(object_index)[p]] = 0;
		_G(object_index)[p] = 0;
	}
}

int drop_object(ACTOR *actr) {
	int o, rnd1, rnd2;

	rnd1 = g_events->getRandomNumber(99);
	rnd2 = g_events->getRandomNumber(99);

	if (rnd1 < 25) o = 5;      //apple
	else if (rnd1 & 1) {    //jewels
		if (rnd2 < 10) o = 1;      //red
		else o = 2;           //blue
	} else {                 //potion
		if (rnd2 < 10) o = 3;    //red
		else o = 4;           //blue
	}
	_drop_obj(actr, o);
	return 1;
}

int _drop_obj(ACTOR *actr, int o) {
	int p;

	p = (actr->x + (actr->size_x / 2)) / 16 + (((actr->y + (actr->size_y / 2)) / 16) * 20);
	if (!_G(object_map)[p] && _G(scrn).icon[p / 20][p % 20] >= 140) {  //nothing there and solid
		_G(object_map)[p] = o;
		_G(object_index)[p] = 27 + actr->actor_num;  //actor is 3-15
#if 0
		x = (p % 20) * 16;
		y = (p / 20) * 16;
		xfput(x, y, PAGE2, (char far *) objects[o - 1]);
		xcopyd2d(x, y, x + 16, y + 16, x, y, PAGE2, draw_page, 320, 320);
		xcopyd2d(x, y, x + 16, y + 16, x, y, PAGE2, display_page, 320, 320);
#endif
		return 1;
	}

	return 0;
}

int use_apple(int flag) {
	if (_G(thor)->health == 150)
		return 0;

	if (flag && _G(thor_info).magic > 0) {
		if (!_G(apple_flag)) {
			_G(magic_cnt) = 0;
			add_magic(-2);
			add_health(1);
			play_sound(ANGEL, 0);
		} else if (_G(magic_cnt) > 8) {
			_G(magic_cnt) = 0;
			add_magic(-2);
			add_health(1);
			if (!sound_playing())
				play_sound(ANGEL, 0);
		}
		_G(apple_flag) = 1;
		return 1;

	} else {
		_G(apple_flag) = 0;
		if (flag)
			not_enough_magic();
	}
	return 0;
}

int use_thunder(int flag) {

	if (flag && _G(thor_info).magic > 29) {
		if (!_G(thunder_flag)) {
			add_magic(-30);
			play_sound(THUNDER, 0);
			_G(thunder_flag) = 60;
		}
		return 1;
	} else if (flag && !_G(thunder_flag)) not_enough_magic();

	if (_G(thunder_flag)) return 1;
	return 0;
}

int use_hourglass(int flag) {
	int hour_time[] = { 0,60,120,180,240,300,360,420,480,
					 510,540,570,600,630,660,690 };

	if (_G(hourglass_flag)) {
		if ((int)_G(magic_cnt) > hour_time[_G(hourglass_flag)]) {
			_G(hourglass_flag)++;
			if (_G(hourglass_flag) == 16) {
				_G(hourglass_flag) = 0;
				music_resume();
			}
			play_sound(WOOP, 1);
		}
		return 1;
	}

	if (flag && _G(thor_info).magic > 29) {
		if (!_G(hourglass_flag)) {
			_G(magic_cnt) = 0;
			add_magic(-30);
			music_pause();
			play_sound(WOOP, 1);
			_G(hourglass_flag) = 1;
			return 1;
		}
	} else {
		_G(hourglass_flag) = 0;
		if (flag) not_enough_magic();
	}
	return 0;
}

int use_boots(int flag) {

	if (flag) {
		if (_G(thor_info).magic > 0) {
			if (_G(thor)->num_moves == 1) {
				_G(magic_cnt) = 0;
				add_magic(-1);
			} else if (_G(magic_cnt) > 8) {
				_G(magic_cnt) = 0;
				add_magic(-1);
			}
			_G(thor)->num_moves = 2;
			_G(hammer)->num_moves = 3;
			return 1;
		} else {
			not_enough_magic();
			_G(thor)->num_moves = 1;
			_G(hammer)->num_moves = 2;
		}
	} else {
		_G(thor)->num_moves = 1;
		_G(hammer)->num_moves = 2;
	}
	return 0;
}

int use_shield(int flag) {
	int f;

	f = 0;
	if (flag) {
		if (_G(thor_info).magic) {
			if (!_G(shield_on)) {
				_G(magic_cnt) = 0;
				add_magic(-1);
				setup_magic_item(1);

				_G(actor)[2] = _G(magic_item)[1];

				setup_actor(&_G(actor)[2], 2, 0, _G(thor)->x, _G(thor)->y);
				_G(actor)[2].speed_count = 1;
				_G(actor)[2].speed = 1;
				_G(shield_on) = 1;
			} else if (_G(magic_cnt) > 8) {
				_G(magic_cnt) = 0;
				add_magic(-1);
			}
			return 1;
		} else {
			f = 1;
			not_enough_magic();
		}

	} else f = 1;
	if (f == 1) {
		if (_G(shield_on)) {
			_G(actor)[2].dead = 2;
			_G(actor)[2].used = 0;
			_G(shield_on) = 0;
		}
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
			if (!_G(tornado_used) && !_G(actor)[2].dead && _G(magic_cnt) > 20) {
				_G(magic_cnt) = 0;
				add_magic(-10);
				setup_magic_item(0);
				_G(actor)[2] = _G(magic_item)[0];

				setup_actor(&_G(actor)[2], 2, 0, _G(thor)->x, _G(thor)->y);
				_G(actor)[2].last_dir = _G(thor)->dir;
				_G(actor)[2].move = 16;
				_G(tornado_used) = 1;
				play_sound(WIND, 0);
			}
		} else if (_G(tornado_used) == 0) {
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
			actor_destroyed(&_G(actor)[2]);
			_G(tornado_used) = 0;
			not_enough_magic();
			return 0;
		}
		return 1;
	}
	return 0;
}

int use_object(int flag) {
	if (!flag) return 0;
	if (!(_G(thor_info).inventory & 64))
		return 0;

	odin_speaks((_G(thor_info).object - 1) + 5501, _G(thor_info).object - 1);
	return 1;
}

void use_item() {
	int kf, ret = 0, mf;

	kf = _G(key_flag)[key_magic];

	if (!kf && _G(tornado_used)) {
		actor_destroyed(&_G(actor)[2]);
		_G(tornado_used) = 0;
	}

	mf = _G(magic_inform);
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
				play_sound(BRAAPP, 0);
			_G(useItemFlag) = true;
		}
	} else {
		_G(useItemFlag) = false;
	}
}

void not_enough_magic() {
	if (!_G(magic_inform))
		odin_speaks(2006, 0);

	_G(magic_inform) = 1;
}

void cannot_carry_more() {
	if (!_G(carry_inform))
		odin_speaks(2007, 0);
	_G(carry_inform) = 1;
}

void delete_object() {

	_G(thor_info).inventory &= 0xbf;
	_G(thor_info).item = 1;
	display_item();
}

} // namespace Got
