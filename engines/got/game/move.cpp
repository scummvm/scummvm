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

#include "got/game/move.h"
#include "got/events.h"
#include "got/game/back.h"
#include "got/game/move_patterns.h"
#include "got/game/shot_movement.h"
#include "got/game/shot_pattern.h"
#include "got/game/status.h"
#include "got/vars.h"

namespace Got {

void next_frame(ACTOR *actr) {
	const int fcount = actr->frame_count - 1;

	if (fcount <= 0) {
		actr->next++;

		if (actr->next > 3)
			actr->next = 0;

		actr->frame_count = actr->frame_speed;
	} else
		actr->frame_count = fcount;
}

bool point_within(int x, int y, int x1, int y1, int x2, int y2) {
	return ((x >= x1) && (x <= x2) && (y >= y1) && (y <= y2));
}

bool overlap(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4) {
	if ((x1 >= x3) && (x1 <= x4) && (y1 >= y3) && (y1 <= y4))
		return true;
	if ((x2 >= x3) && (x2 <= x4) && (y2 >= y3) && (y2 <= y4))
		return true;
	if ((x1 >= x3) && (x1 <= x4) && (y2 >= y3) && (y2 <= y4))
		return true;
	if ((x2 >= x3) && (x2 <= x4) && (y1 >= y3) && (y1 <= y4))
		return true;
	if ((x3 >= x1) && (x3 <= x2) && (y3 >= y1) && (y3 <= y2))
		return true;
	if ((x4 >= x1) && (x4 <= x2) && (y4 >= y1) && (y4 <= y2))
		return true;
	if ((x3 >= x1) && (x3 <= x2) && (y4 >= y1) && (y4 <= y2))
		return true;
	if ((x4 >= x1) && (x4 <= x2) && (y3 >= y1) && (y3 <= y2))
		return true;
	return false;
}

int reverse_direction(ACTOR *actr) {
	if (actr->dir == 1)
		return 0;
	if (actr->dir == 2)
		return 3;
	if (actr->dir == 3)
		return 2;
	return 1;
}

void thor_shoots() {
	if ((_G(hammer)->used != 1) && (!_G(hammer)->dead) && (!_G(thor)->shot_cnt)) {
		play_sound(SWISH, false);
		_G(thor)->shot_cnt = 20;
		_G(hammer)->used = 1;
		_G(hammer)->dir = _G(thor)->dir;
		_G(hammer)->last_dir = _G(thor)->dir;
		_G(hammer)->x = _G(thor)->x;
		_G(hammer)->y = _G(thor)->y + 2;
		_G(hammer)->move = 2;
		_G(hammer)->next = 0;
		_G(hammer)->last_x[0] = _G(hammer)->x;
		_G(hammer)->last_x[1] = _G(hammer)->x;
		_G(hammer)->last_y[0] = _G(hammer)->y;
		_G(hammer)->last_y[1] = _G(hammer)->y;
	}
}

int kill_good_guy(void) {
	if (!_G(killgg_inform) && !_G(thunder_flag)) {
		odin_speaks(2010, 0);
		_G(killgg_inform) = true;
	}

	add_score(-1000);
	return 0;
}

void actor_damaged(ACTOR *actr, int damage) {
	if (!_G(setup).skill)
		damage *= 2;
	else if (_G(setup).skill == 2)
		damage /= 2;

	if (!actr->vunerable && actr->type != 3 && (actr->solid & 0x7f) != 2) {
		actr->vunerable = STAMINA;
		if (damage >= actr->health) {
			if (actr->type != 4) {
				add_score(actr->init_health * 10);

			} else {
				kill_good_guy();
			}

			actor_destroyed(actr);
		} else {
			actr->show = 10;
			actr->health -= damage;
			actr->speed_count += 8;
		}
	} else if (!actr->vunerable) {
		actr->vunerable = STAMINA;

		if (actr->func_num == 4) {
			switch_icons();
		}
		if (actr->func_num == 7) {
			rotate_arrows();
		}
	}
}

void thor_damaged(ACTOR *actr) {
	actr->hit_thor = 1;

	// If we're invincible, ignore any damage
	if (_G(cheats).freezeHealth)
		return;

	if (GAME3 && actr->func_num == 11) {
		if (actr->talk_counter) {
			actr->talk_counter--;
			return;
		}

		int t = actr->type;
		actr->type = 4;
		actor_speaks(actr, 0, 0);
		actr->type = t;
		actr->talk_counter = 30;
		return;
	}

	int damage = actr->strength;
	if (damage != 255) {
		if (!_G(setup).skill)
			damage /= 2;
		else if (_G(setup).skill == 2)
			damage *= 2;
	}
	if ((!_G(thor)->vunerable && !_G(shield_on)) || damage == 255) {
		if (damage >= _G(thor)->health) {
			_G(thor)->vunerable = 40;
			_G(thor)->show = 0;
			_G(thor)->health = 0;
			_G(exit_flag) = 2;
			g_events->send(GameMessage("THOR_DIES"));
		} else if (damage) {
			_G(thor)->vunerable = 40;
			_G(sound).play_sound(OW, 0);
			_G(thor)->show = 10;
			_G(thor)->health -= damage;
		}
	}
}

void actor_destroyed(ACTOR *actr) {
	if (actr->actor_num > 2) {
		int pge = _G(pge);

		int x = actr->last_x[pge ^ 1];
		int y = actr->last_y[pge ^ 1];
		int x1 = actr->last_x[pge];
		int y1 = actr->last_y[pge];
		int r = actr->rating;
		int n = actr->actor_num;
		int t = actr->type;

		if (actr->func_num == 255)
			actr->copyFixedAndPics(_G(explosion));
		else
			actr->copyFixedAndPics(_G(sparkle));

		actr->type = t;
		actr->actor_num = n;
		actr->rating = r;
		actr->x = x;
		actr->y = y;
		actr->last_x[pge] = x1;
		actr->last_x[pge ^ 1] = x;
		actr->last_y[pge] = y1;
		actr->last_y[pge ^ 1] = y;
		actr->speed_count = actr->speed;
		actr->used = 1;
		actr->num_shots = 3; // used to reverse explosion
		actr->vunerable = 255;
	} else {
		actr->dead = 2;
		actr->used = 0;
	}
}

int _actor_shoots(ACTOR *actr, int dir) {
	int t = actr->shot_type - 1;
	for (int i = MAX_ENEMIES + 3; i < MAX_ACTORS; i++) {
		if ((!_G(actor[i]).used) && (!_G(actor[i]).dead)) {
			ACTOR *act = &_G(actor[i]);
			*act = _G(shot[t]);
			int cx, cy;

			if (actr->size_y < act->size_y)
				cy = actr->y - ((act->size_y - actr->size_y) / 2);
			else
				cy = actr->y + ((actr->size_y - act->size_y) / 2);

			if (actr->size_x < act->size_x)
				cx = actr->x - ((act->size_x - actr->size_x) / 2);
			else
				cx = actr->x + ((actr->size_x - act->size_x) / 2);

			if (cy > 174)
				cy = 174;

			if (cx > 304)
				cx = 304;

			act->x = cx;
			act->y = cy;
			act->last_dir = dir;
			act->next = 0;
			act->dir = dir;
			if (act->directions == 1)
				act->dir = 0;
			else if (act->directions == 4 && act->frames == 1) {
				act->next = dir;
				act->dir = 0;
			}
			act->frame_count = act->frame_speed;
			act->speed_count = act->speed;
			act->last_x[0] = cx;
			act->last_x[1] = cx;
			act->last_x[0] = actr->x;
			act->last_x[1] = actr->x;
			act->last_y[0] = cy;
			act->last_y[1] = cy;
			act->used = 1;
			act->creator = actr->actor_num;
			act->move_count = act->num_moves;
			act->dead = 0;
			actr->shot_actor = i;
			actr->num_shots++;
			actr->shot_cnt = 20;
			_G(shot_ok) = false;
			return 1;
		}
	}
	return 0;
}

void actor_always_shoots(ACTOR *actr, int dir) {
	_actor_shoots(actr, dir);
}

int actor_shoots(ACTOR *actr, int dir) {
	int i;

	int cx = (actr->x + (actr->size_x / 2)) >> 4;
	int cy = ((actr->y + actr->size_y) - 2) >> 4;

	int tx = _G(thor)->center_x;
	int ty = _G(thor)->center_y;

	int icn = 140;

	if (_G(shot[actr->shot_type - 1]).flying)
		icn = 80;

	switch (dir) {
	case 0:
		for (i = ty + 1; i <= cy; i++) {
			if (_G(scrn).icon[i][cx] < icn)
				return 0;
		}
		break;
	case 1:
		for (i = cy; i <= ty; i++) {
			if (_G(scrn).icon[i][cx] < icn)
				return 0;
		}
		break;
	case 2:
		for (i = tx; i < cx; i++) {
			if (_G(scrn).icon[cy][i] < icn)
				return 0;
		}
		break;
	case 3:
		for (i = cx; i < tx; i++) {
			if (_G(scrn).icon[cy][i] < icn)
				return 0;
		}
		break;
	}
	return _actor_shoots(actr, dir);
}

void move_actor(ACTOR *actr) {
	if (actr->vunerable != 0)
		actr->vunerable--;
	if (actr->shot_cnt != 0)
		actr->shot_cnt--;
	if (actr->show != 0)
		actr->show--;

	if (!actr->shot_cnt && _G(shot_ok)) {
		if (actr->shots_allowed) {
			if (actr->num_shots < actr->shots_allowed) {
				shot_pattern_func[actr->shot_pattern](actr);
			}
		}
	}

	const int scount = actr->speed_count -1;
	if (scount <= 0) {
		if (!actr->move_counter)
			actr->speed_count = actr->speed;
		else
			actr->speed_count = (actr->speed << 1);
		
		int i;

		if (actr->type == 3)
			i = shot_movement_func[actr->move](actr);
		else
			i = movement_func[actr->move](actr);
		
		if (actr->directions == 2)
			i &= 1;
		
		if (i != actr->dir)
			actr->dir = i;

		if (actr->move == 0 && _G(current_level) != _G(new_level) && _G(shield_on)) {
			_G(actor[2]).x = actr->x - 2;
			if (_G(actor[2]).x < 0)
				_G(actor[2]).x = 0;
			_G(actor[2]).y = actr->y;
			_G(actor[2]).last_x[0] = _G(actor[2]).x;
			_G(actor[2]).last_x[1] = _G(actor[2]).x;
			_G(actor[2]).last_y[0] = _G(actor[2]).y;
			_G(actor[2]).last_y[1] = _G(actor[2]).y;
		}
	} else
		actr->speed_count = scount;

	actr->x &= 0xfffe;
}

} // namespace Got
