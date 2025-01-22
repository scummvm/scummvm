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

#include "got/game/move_patterns.h"
#include "got/events.h"
#include "got/game/back.h"
#include "got/game/boss1.h"
#include "got/game/boss2.h"
#include "got/game/boss3.h"
#include "got/game/move.h"
#include "got/game/object.h"
#include "got/game/script.h"
#include "got/game/special_tile.h"
#include "got/vars.h"

namespace Got {

#define TILE_SOLID 80
#define TILE_FLY 140
#define TILE_SPECIAL 200

int checkSpecialMove1(int x, int y, Actor *actor);

int checkMove0(int x, int y, Actor *actor);
int checkMove1(int x, int y, Actor *actor);

int movementZero(Actor *actor);
int movementOne(Actor *actor);
int movementTwo(Actor *actor);
int movementThree(Actor *actor);
int movementFour(Actor *actor);
int movementFive(Actor *actor);
int movementSix(Actor *actor);
int movementSeven(Actor *actor);
int movementEight(Actor *actor);
int movementNine(Actor *actor);
int movementTen(Actor *actor);
int movementEleven(Actor *actor);
int movementTwelve(Actor *actor);      // Horizontal straight until bump
int movementThirteen(Actor *actor);    // Pause-seek (mushroom)
int movementFourteen(Actor *actor);    // Move-bump-stop (boulder)
int movementFifteen(Actor *actor);     // No move, no frame cycle
int movementSixteen(Actor *actor);     // Tornado 1
int movementSeventeen(Actor *actor);   // Tornado 2
int movementEighteen(Actor *actor);    // Random-seek-bite-run
int movementNineteen(Actor *actor);    // Tornado 2
int movementTwenty(Actor *actor);      // Tornado 2
int movementTwentyOne(Actor *actor);   // Eyeball
int movementTwentyTwo(Actor *actor);   // Spear
int movementTwentyThree(Actor *actor); // Spinball cw
int movementTwentyFour(Actor *actor);  // Spinball ccw
int movementTwentyFive(Actor *actor);  //
int movementTwentySix(Actor *actor);   //
int movementTwentySeven(Actor *actor); //
int movementTwentyEight(Actor *actor); // Tree boss
int movementTwentyNine(Actor *actor);  // Horizontal or vertical (pass_val)
int movementThirty(Actor *actor);      // Vertical straight
int movementThirtyOne(Actor *actor);   // Drop (stalagtite)
int movementThirtyTwo(Actor *actor);   // Bomb 1
int movementThirtyThree(Actor *actor); // Bomb 2
int movementThirtyFour(Actor *actor);  // Gun (4-dir)
int movementThirtyFive(Actor *actor);  // Gun (4-dir)
int movementThirtySix(Actor *actor);   // Acid drop
int movementThirtySeven(Actor *actor); // 4-way rnd,rnd len
int movementThirtyEight(Actor *actor); // Timed darting
int movementThirtyNine(Actor *actor);  // Troll 1
int movementForty(Actor *actor);       // Troll 2

int (*const _movementFunc[])(Actor *actor) = {
	movementZero,
	movementOne,
	movementTwo,
	movementThree,
	movementFour,
	movementFive,
	movementSix,
	movementSeven,
	movementEight,
	movementNine,
	movementTen,
	movementEleven,
	movementTwelve,
	movementThirteen,
	movementFourteen,
	movementFifteen,
	movementSixteen,
	movementSeventeen,
	movementEighteen,
	movementNineteen,
	movementTwenty,
	movementTwentyOne,
	movementTwentyTwo,
	movementTwentyThree,
	movementTwentyFour,
	movementTwentyFive,
	movementTwentySix,
	movementTwentySeven,
	movementTwentyEight,
	movementTwentyNine,
	movementThirty,
	movementThirtyOne,
	movementThirtyTwo,
	movementThirtyThree,
	movementThirtyFour,
	movementThirtyFive,
	movementThirtySix,
	movementThirtySeven,
	movementThirtyEight,
	movementThirtyNine,
	movementForty};

int specialMovementOne(Actor *actor);
int specialMovementTwo(Actor *actor);
int specialMovementThree(Actor *actor);
int specialMovementFour(Actor *actor);
int specialMovementFive(Actor *actor);
int specialMovementSix(Actor *actor);
int specialMovementSeven(Actor *actor);
int specialMovementEight(Actor *actor);
int specialMovementNine(Actor *actor);
int specialMovementTen(Actor *actor);
int specialMovementEleven(Actor *actor);

int (*specialMovementFunc[])(Actor *actor) = {
	nullptr,
	specialMovementOne,
	specialMovementTwo,
	specialMovementThree,
	specialMovementFour,
	specialMovementFive,
	specialMovementSix,
	specialMovementSeven,
	specialMovementEight,
	specialMovementNine,
	specialMovementTen,
	specialMovementEleven};

// Check Thor move
int checkMove0(const int x, const int y, Actor *actor) {
	_G(thor_icon1) = 0;
	_G(thor_icon2) = 0;
	_G(thor_icon3) = 0;
	_G(thor_icon4) = 0;

	if (x < 0) {
		if (_G(current_level) > 0) {
			_G(new_level) = _G(current_level) - 1;
			actor->_x = 304;
			actor->_lastX[0] = 304;
			actor->_lastX[1] = 304;
			actor->_show = 0;
			actor->_moveCount = 0;
			setThorVars();
			return 1;
		}

		return 0;
	}
	
	if (x > 306) {
		if (_G(current_level) < 119) {
			_G(new_level) = _G(current_level) + 1;
			actor->_x = 0;
			actor->_lastX[0] = 0;
			actor->_lastX[1] = 0;
			actor->_show = 0;
			actor->_moveCount = 0;
			setThorVars();
			return 1;
		}

		return 0;
	}
	
	if (y < 0) {
		if (_G(current_level) > 9) {
			_G(new_level) = _G(current_level) - 10;
			actor->_y = 175;
			actor->_lastY[0] = 175;
			actor->_show = 0;
			actor->_lastY[1] = 175;
			actor->_moveCount = 0;
			setThorVars();
			return 1;
		}

		return 0;
	}
	
	if (y > 175) {
		if (_G(current_level) < 110) {
			_G(new_level) = _G(current_level) + 10;
			actor->_y = 0;
			actor->_lastY[0] = 0;
			actor->_lastY[1] = 0;
			actor->_show = 0;
			actor->_moveCount = 0;
			setThorVars();
			return 1;
		}

		return 0;
	}
	
	int x1 = (x + 1) >> 4;
	int y1 = (y + 8) >> 4;
	int x2 = (_G(thor)->_dir > 1) ? (x + 12) >> 4 : (x + 10) >> 4;
	int y2 = (y + 15) >> 4;

	_G(slipFlag) = false;

	// Check for cheat flying mode
	if (!actor->_flying) {
		byte icn1 = _G(scrn)._iconGrid[y1][x1];
		byte icn2 = _G(scrn)._iconGrid[y2][x1];
		byte icn3 = _G(scrn)._iconGrid[y1][x2];
		byte icn4 = _G(scrn)._iconGrid[y2][x2];
		int ti = 0;

		if (icn1 < TILE_FLY) {
			_G(thor_icon1) = 1;
			ti = 1;
		}
		
		if (icn2 < TILE_FLY) {
			_G(thor_icon2) = 1;
			ti = 1;
		}
		
		if (icn3 < TILE_FLY) {
			_G(thor_icon3) = 1;
			ti = 1;
		}
		
		if (icn4 < TILE_FLY) {
			_G(thor_icon4) = 1;
			ti = 1;
		}
		
		if (ti)
			return 0;

		if (icn1 > TILE_SPECIAL) {
			if (!specialTileThor(y1, x1, icn1))
				return 0;
			icn2 = _G(scrn)._iconGrid[y2][x1];
			icn3 = _G(scrn)._iconGrid[y1][x2];
			icn4 = _G(scrn)._iconGrid[y2][x2];
		}

		if (icn2 > TILE_SPECIAL) {
			if (!specialTileThor(y2, x1, icn2))
				return 0;
			icn3 = _G(scrn)._iconGrid[y1][x2];
			icn4 = _G(scrn)._iconGrid[y2][x2];
		}

		if (icn3 > TILE_SPECIAL) {
			if (!specialTileThor(y1, x2, icn3))
				return 0;
			icn4 = _G(scrn)._iconGrid[y2][x2];
		}

		if (icn4 > TILE_SPECIAL && !specialTileThor(y2, x2, icn4))
			return 0;
	}

	if (!_G(slipFlag)) {
		_G(slipping) = false;
		_G(slip_cnt) = 0;
	}
	
	if (_G(slipFlag) && !_G(slipping))
		_G(slip_cnt++);
	
	if (_G(slip_cnt) > 8)
		_G(slipping) = true;
	
	_G(slipFlag) = false;

	x1 = x + 1;
	y1 = y + 8;
	
	x2 = x + 12;	
	y2 = y + 15;

	_G(thor_special_flag) = false;
	for (int i = 3; i < MAX_ACTORS; i++) {
		Actor *act = &_G(actor[i]);
		if ((act->_solid & 128) || !act->_active)
			continue;

		const int x3 = act->_x + 1;
		const int y3 = act->_y + 1;
		
		if (ABS(x3 - x1) > 16 || ABS(y3 - y1) > 16)
			continue;

		const int x4 = act->_x + act->_sizeX - 1;
		const int y4 = act->_y + act->_sizeY - 1;
		if (overlap(x1, y1, x2, y2, x3, y3, x4, y4)) {
			if (act->_funcNum > 0) { // 255=explosion
				if (act->_funcNum == 255)
					return 0;
				act->_temp1 = x;
				act->_temp2 = y;
				_G(thor_special_flag) = true;
				
				return specialMovementFunc[act->_funcNum](act);
			}

			_G(thor_special_flag) = false;
			thorDamaged(act);
			if (act->_solid < 2) {
				if (!act->_vulnerableCountdown && (!(act->_type & 1)))
					playSound(PUNCH1, false);

				if (!_G(hammer)->_active && _G(key_flag[key_fire]))
					actorDamaged(act, _G(hammer)->_hitStrength);
				else
					actorDamaged(act, _G(thor)->_hitStrength);
			}
			return 1;
		}
	}
	actor->_x = x;
	actor->_y = y;
	return 1;
}

// Check hammer move
int checkMove1(const int x, const int y, Actor *actor) {
	if (x < 0 || x > 306 || y < 0 || y > 177)
		return 0;

	int x1 = (x + 1) >> 4;
	int y1 = (y + 6) >> 4;
	int x2 = (x + 10) >> 4;
	int y2 = (y + 10) >> 4;

	// Check for solid or fly over
	int icn = TILE_FLY;
	if (actor->_flying)
		icn = TILE_SOLID;

	const byte icn1 = _G(scrn)._iconGrid[y1][x1];
	const byte icn2 = _G(scrn)._iconGrid[y2][x1];
	const byte icn3 = _G(scrn)._iconGrid[y1][x2];
	const byte icn4 = _G(scrn)._iconGrid[y2][x2];
	if (icn1 < icn || icn2 < icn || icn3 < icn || icn4 < icn) {
		if (actor->_actorNum == 1 && actor->_moveType == 2)
			playSound(CLANG, false);

		return 0;
	}

	if (icn1 > TILE_SPECIAL && !specialTile(actor, y1, x1, icn1))
		return 0;

	if (icn2 > TILE_SPECIAL && !specialTile(actor, y2, x1, icn2))
		return 0;

	if (icn3 > TILE_SPECIAL && !specialTile(actor, y1, x2, icn3))
		return 0;

	if (icn4 > TILE_SPECIAL && !specialTile(actor, y2, x2, icn4))
		return 0;

	x1 = x + 1;
	y1 = y + 1;
	x2 = x + 10;
	y2 = y + 10;

	int f = 0;
	for (int i = 3; i < MAX_ACTORS; i++) {
		Actor *act = &_G(actor[i]);
		if (!act->_active || act->_type == 3)
			continue;

		const int x3 = act->_x;
		const int y3 = act->_y;

		if (ABS(x3 - x1) > 16 || ABS(y3 - y1) > 16)
			continue;

		const int x4 = act->_x + act->_sizeX - 1;
		const int y4 = act->_y + act->_sizeY - 1;

		if (overlap(x1, y1, x2, y2, x3, y3, x4, y4)) {
			if (_G(boss_active) && !GAME3) {
				switch (_G(area)) {
				case 1:
					boss1CheckHit(act, x1, y1, x2, y2, i);
					break;
				case 2:
					boss2CheckHit(act, x1, y1, x2, y2, i);
					break;
				default:
					// Area 3 boss Loki isn't checked here
					break;
				}
			} else {
				if (act->_solid == 2 && (actor->_moveType == 16 || actor->_moveType == 17))
					return 0;
				actorDamaged(act, actor->_hitStrength);
			}
			f++;
		}
	}
	if (f && actor->_moveType == 2)
		return 0;

	actor->_x = x;
	actor->_y = y;
	return 1;
}

// Check enemy move
int checkMove2(const int x, const int y, Actor *actor) {
	if (actor->_actorNum < 3)
		return checkMove1(x, y, actor);

	if (x < 0 || x > (319 - actor->_sizeX) || y < 0 || y > 175)
		return 0;

	int x1 = (x + 1) >> 4;
	int y1;
	if (!actor->_funcNum)
		y1 = (y + (actor->_sizeY / 2)) >> 4;
	else
		y1 = (y + 1) >> 4;

	int x2 = ((x + actor->_sizeX) - 1) >> 4;
	int y2 = ((y + actor->_sizeY) - 1) >> 4;

	// Check for solid or fly over

	int icn = TILE_FLY;
	if (actor->_flying)
		icn = TILE_SOLID;

	const byte icn1 = _G(scrn)._iconGrid[y1][x1];
	const byte icn2 = _G(scrn)._iconGrid[y2][x1];
	const byte icn3 = _G(scrn)._iconGrid[y1][x2];
	const byte icn4 = _G(scrn)._iconGrid[y2][x2];
	if (icn1 < icn || icn2 < icn || icn3 < icn || icn4 < icn)
		return 0;

	if (icn1 > TILE_SPECIAL)
		if (!specialTile(actor, y1, x1, icn1))
			return 0;
	if (icn2 > TILE_SPECIAL)
		if (!specialTile(actor, y2, x1, icn2))
			return 0;
	if (icn3 > TILE_SPECIAL)
		if (!specialTile(actor, y1, x2, icn3))
			return 0;
	if (icn4 > TILE_SPECIAL)
		if (!specialTile(actor, y2, x2, icn4))
			return 0;

	x1 = x + 1;
	y1 = y + 1;
	x2 = (x + actor->_sizeX) - 1;
	y2 = (y + actor->_sizeY) - 1;

	for (int i = 0; i < MAX_ACTORS; i++) {
		Actor *act = &_G(actor[i]);
		if (act->_actorNum == actor->_actorNum || act->_actorNum == 1 || !act->_active)
			continue;
		if (act->_type == 3)
			continue; // Shot

		if (i == 0) {
			if (overlap(x1, y1, x2, y2, _G(thor_x1), _G(thor_y1), _G(thor_x2), _G(thor_y2))) {
				thorDamaged(actor);
				return 0;
			}
		} else {
			const int x3 = act->_x;
			const int y3 = act->_y;
			
			if (ABS(x3 - x1) > 16 || ABS(y3 - y1) > 16)
				continue;

			const int x4 = act->_x + act->_sizeX;
			const int y4 = act->_y + act->_sizeY;
			if (overlap(x1, y1, x2, y2, x3, y3, x4, y4)) {
				if (actor->_moveType == 38) {
					if (act->_funcNum == 4)
						_G(switch_flag) = 1;
					else if (act->_funcNum == 7)
						_G(switch_flag) = 2;
				}
				return 0;
			}
		}
	}
	actor->_x = x;
	actor->_y = y;
	return 1;
}

// Check enemy shot move
int checkMove3(const int x, const int y, Actor *actor) {
	if (x < 0 || x > (319 - actor->_sizeX) || y < 0 || y > 175)
		return 0;

	int x1 = (x + 1) >> 4;
	int y1 = (y + (actor->_sizeY / 2)) >> 4;
	int x2 = ((x + actor->_sizeX) - 1) >> 4;
	int y2 = ((y + actor->_sizeY) - 1) >> 4;

	// Check for solid or fly over

	int icn = TILE_FLY;
	if (actor->_flying)
		icn = TILE_SOLID;

	const byte icn1 = _G(scrn)._iconGrid[y1][x1];
	const byte icn2 = _G(scrn)._iconGrid[y2][x1];
	const byte icn3 = _G(scrn)._iconGrid[y1][x2];
	const byte icn4 = _G(scrn)._iconGrid[y2][x2];
	if (icn1 < icn || icn2 < icn || icn3 < icn || icn4 < icn)
		return 0;

	if (icn1 > TILE_SPECIAL && !specialTile(actor, y1, x1, icn1))
		return 0;
	if (icn2 > TILE_SPECIAL && !specialTile(actor, y2, x1, icn2))
		return 0;
	if (icn3 > TILE_SPECIAL && !specialTile(actor, y1, x2, icn3))
		return 0;
	if (icn4 > TILE_SPECIAL && !specialTile(actor, y2, x2, icn4))
		return 0;

	// Check for solid or fly over
	x1 = x + 1;
	y1 = y + 1;
	x2 = (x + actor->_sizeX) - 1;
	y2 = (y + actor->_sizeY) - 1;

	if (overlap(x1, y1, x2, y2, _G(thor_x1), _G(thor_real_y1), _G(thor_x2), _G(thor_y2))) {
		thorDamaged(actor);
		return 0;
	}
	for (int i = 3; i < MAX_ACTORS; i++) {
		if (i == actor->_actorNum)
			continue;

		Actor *act = &_G(actor[i]);

		if (!act->_active)
			continue;
		if (act->_solid < 2)
			continue;
		if (act->_type == 3)
			continue; // Shot
		if (act->_actorNum == actor->_creator)
			continue;

		const int x3 = act->_x;
		const int y3 = act->_y;

		if (ABS(x3 - x1) > 16 || ABS(y3 - y1) > 16)
			continue;

		const int x4 = x3 + 15;
		const int y4 = y3 + 15;
		if (overlap(x1, y1, x2, y2, x3, y3, x4, y4))
			return 0;
	}
	actor->_x = x;
	actor->_y = y;
	return 1;
}

// Flying enemies
int checkMove4(const int x, const int y, Actor *actor) {
	if (x < 0 || x > (319 - actor->_sizeX) || y < 0 || y > 175)
		return 0;
	if (overlap(x, y, x + actor->_sizeX - 1, y + actor->_sizeY - 1,
				_G(thor_x1), _G(thor_y1), _G(thor_x2), _G(thor_y2))) {
		if (actor->_type == 3)
			thorDamaged(actor);
		return 0;
	}
	actor->_x = x;
	actor->_y = y;
	return 1;
}

#define THOR_PAD1 2
#define THOR_PAD2 4

int check_thor_move(const int x, const int y, Actor *actor) {
	if (checkMove0(x, y, actor))
		return 1;
	
	if (_G(diag_flag) || _G(thor_special_flag))
		return 0;

	if (_G(thor_icon1) + _G(thor_icon2) + _G(thor_icon3) + _G(thor_icon4) > 1)
		return 0;

	switch (actor->_dir) {
	case 0:
		if (_G(thor_icon1)) {
			actor->_dir = 3;
			if (checkMove0(x + THOR_PAD1, y + 2, actor)) {
				actor->_dir = 0;
				return 1;
			}
		} else if (_G(thor_icon3)) {
			actor->_dir = 2;
			if (checkMove0(x - THOR_PAD1, y + 2, actor)) {
				actor->_dir = 0;
				return 1;
			}
		}
		actor->_dir = 0;
		break;
		
	case 1:
		if (_G(thor_icon2)) {
			actor->_dir = 3;
			if (checkMove0(x + THOR_PAD1, y - 2, actor)) {
				actor->_dir = 1;
				return 1;
			}
		} else if (_G(thor_icon4)) {
			actor->_dir = 2;
			if (checkMove0(x - THOR_PAD1, y - 2, actor)) {
				actor->_dir = 1;
				return 1;
			}
		}
		actor->_dir = 1;
		break;
		
	case 2:
		if (_G(thor_icon1)) {
			if (checkMove0(x + 2, y + THOR_PAD1, actor))
				return 1;
		} else if (_G(thor_icon2)) {
			if (checkMove0(x + 2, y - THOR_PAD1, actor))
				return 1;
		}
		break;
		
	case 3:
		if (_G(thor_icon3)) {
			if (checkMove0(x - 2, y + THOR_PAD1, actor))
				return 1;
		} else if (_G(thor_icon4)) {
			if (checkMove0(x - 2, y - THOR_PAD1, actor))
				return 1;
		}
		break;

	default:
		break;
	}

	return 0;
}

// Player control
int movementZero(Actor *actor) {
	int d = actor->_dir;
	const int oldDir = d;

	setThorVars();

	if (_G(hammer)->_active && _G(hammer)->_moveType == 5) {
		if (overlap(_G(thor_x1), _G(thor_y1), _G(thor_x2), _G(thor_y2),
					_G(hammer)->_x, _G(hammer)->_y, _G(hammer)->_x + 13, _G(hammer)->_y + 13)) {
			actorDestroyed(_G(hammer));
		}
	}
	int x = actor->_x;
	int y = actor->_y;
	_G(diag_flag) = false;
	if (actor->_moveCounter)
		actor->_moveCounter--;

	if (_G(slipping)) {
		if (_G(slip_cnt) == 8)
			playSound(FALL, true);

		y += 2;
		_G(slip_cnt--);
		if (!_G(slip_cnt))
			_G(slipping) = false;

		check_thor_move(x, y, actor);
		_G(thor)->_moveCountdown = 4;
		return d;
	}

	if (_G(key_flag[key_up]) && _G(key_flag[key_left])) {
		d = 2;
		actor->_dir = d;
		_G(diag) = 1;
		_G(diag_flag) = true;
		if (check_thor_move(x - 2, y - 2, actor)) {
			nextFrame(actor);
			return d;
		}
	} else if (_G(key_flag[key_up]) && _G(key_flag[key_right])) {
		d = 3;
		actor->_dir = d;
		_G(diag) = 2;
		_G(diag_flag) = true;
		if (check_thor_move(x + 2, y - 2, actor)) {
			nextFrame(actor);
			return d;
		}
	} else if (_G(key_flag[key_down]) && _G(key_flag[key_left])) {
		d = 2;
		actor->_dir = d;
		_G(diag) = 4;
		_G(diag_flag) = true;
		if (check_thor_move(x - 2, y + 2, actor)) {
			nextFrame(actor);
			return d;
		}
	} else if (_G(key_flag[key_down]) && _G(key_flag[key_right])) {
		d = 3;
		actor->_dir = d;
		_G(diag) = 3;
		_G(diag_flag) = true;
		if (check_thor_move(x + 2, y + 2, actor)) {
			nextFrame(actor);
			return d;
		}
	}
	_G(diag) = 0;
	if (_G(key_flag[key_right])) {
		if (!_G(key_flag[key_left])) {
			d = 3;
			actor->_dir = d;
			if (check_thor_move(x + 2, y, actor)) {
				nextFrame(actor);
				return d;
			}
		}
	}
	if (_G(key_flag[key_left])) {
		if (!_G(key_flag[key_right])) {
			d = 2;
			actor->_dir = d;
			if (check_thor_move(x - 2, y, actor)) {
				nextFrame(actor);
				return d;
			}
		}
	}
	if (_G(key_flag[key_down])) {
		if (!_G(key_flag[key_up])) {
			d = 1;
			actor->_dir = d;
			if (check_thor_move(x, y + 2, actor)) {
				nextFrame(actor);
				return d;
			}
		}
	}
	if (_G(key_flag[key_up])) {
		if (!_G(key_flag[key_down])) {
			d = 0;
			actor->_dir = d;
			if (check_thor_move(x, y - 2, actor)) {
				nextFrame(actor);
				return d;
			}
		}
	}
	actor->_moveCounter = 5;
	actor->_nextFrame = 0;
	actor->_dir = oldDir;
	return d;
}

int checkSpecialMove1(const int x, const int y, Actor *actor) {
	int x3, y3, x4, y4;

	Actor *act;

	if (actor->_actorNum < 3)
		return checkMove1(x, y, actor);

	if (x < 0 || x > 304 || y < 0 || y > 176)
		return 0;

	int x1 = x >> 4;
	int y1 = y >> 4;
	int x2 = (x + 15) >> 4;
	int y2 = (y + 15) >> 4;

	// Check for solid or fly over

	int icn = TILE_FLY;
	if (actor->_flying)
		icn = TILE_SOLID;

	const byte icn1 = _G(scrn)._iconGrid[y1][x1];
	const byte icn2 = _G(scrn)._iconGrid[y2][x1];
	const byte icn3 = _G(scrn)._iconGrid[y1][x2];
	const byte icn4 = _G(scrn)._iconGrid[y2][x2];
	if (icn1 < icn || icn2 < icn || icn3 < icn || icn4 < icn)
		return 0;

	if (icn1 > TILE_SPECIAL && !specialTile(actor, y1, x1, icn1))
		return 0;
	if (icn2 > TILE_SPECIAL && !specialTile(actor, y2, x1, icn2))
		return 0;
	if (icn3 > TILE_SPECIAL && !specialTile(actor, y1, x2, icn3))
		return 0;
	if (icn4 > TILE_SPECIAL && !specialTile(actor, y2, x2, icn4))
		return 0;

	x1 = x;
	y1 = y;
	x2 = (x + 15);
	y2 = (y + 15);

	for (int i = 3; i < MAX_ACTORS; i++) {
		act = &_G(actor[i]);
		if (act->_actorNum == actor->_actorNum)
			continue;
		if (!act->_active)
			continue;
		if (act->_type == 3)
			continue; //shot
		x3 = act->_x;
		if ((ABS(x3 - x1)) > 16)
			continue;
		y3 = act->_y;
		if ((ABS(y3 - y1)) > 16)
			continue;
		x4 = act->_x + act->_sizeX;
		y4 = act->_y + 15;
		if (overlap(x1, y1, x2, y2, x3, y3, x4, y4))
			return 0;
	}
	for (int i = 3; i < MAX_ACTORS; i++) {
		act = &_G(actor[i]);
		if (act->_actorNum == actor->_actorNum)
			continue;
		if (!act->_active)
			continue;
		if (act->_type == 3)
			continue; // Shot
		x3 = act->_x;
		if ((ABS(x3 - x1)) > 16)
			continue;
		y3 = act->_y;
		if ((ABS(y3 - y1)) > 16)
			continue;
		x4 = act->_x + act->_sizeX;
		y4 = act->_y + act->_sizeY;
		if (overlap(_G(thor_x1), _G(thor_y1), _G(thor_x2), _G(thor_y2), x3, y3, x4, y4))
			return 0;
	}
	actor->_x = x;
	actor->_y = y;
	return 1;
}

//*==========================================================================

// Block
int specialMovementOne(Actor *actor) {
	if (_G(diag_flag))
		return 0;

	const int d = _G(thor)->_dir;
	int x1 = actor->_x;
	int y1 = actor->_y;
	const int sd = actor->_lastDir;
	actor->_lastDir = d;

	switch (d) {
	case 0:
		y1 -= 2;
		if (!checkSpecialMove1(x1, y1, actor)) {
			actor->_lastDir = sd;
			return 0;
		}
		break;
		
	case 1:
		y1 += 2;
		if (!checkSpecialMove1(x1, y1, actor)) {
			actor->_lastDir = sd;
			return 0;
		}
		break;
		
	case 2:
		x1 -= 2;
		if (!checkSpecialMove1(x1, y1, actor)) {
			actor->_lastDir = sd;
			return 0;
		}
		break;
		
	case 3:
		x1 += 2;
		if (!checkSpecialMove1(x1, y1, actor)) {
			actor->_lastDir = sd;
			return 0;
		}
		break;
		
	default:
		break;
	}
	nextFrame(actor);
	actor->_lastDir = d;
	return 1;
}

// Angle
int specialMovementTwo(Actor *actor) {
	const int x1 = actor->_temp1; // Calc thor pos
	const int y1 = actor->_temp2;
	const int x2 = x1 + 13;
	const int y2 = y1 + 14;

	for (int i = 3; i < MAX_ACTORS; i++) {
		Actor *act = &_G(actor[i]);
		if (actor->_actorNum == act->_actorNum)
			continue;
		if (!act->_active)
			continue;
		const int x3 = act->_x;
		if ((ABS(x3 - x1)) > 16)
			continue;
		const int y3 = act->_y;
		if ((ABS(y3 - y1)) > 16)
			continue;
		const int x4 = act->_x + act->_sizeX;
		const int y4 = act->_y + act->_sizeY;
		if (overlap(x1, y1, x2, y2, x3, y3, x4, y4))
			return 0;
	}
	if (!actor->_funcPass) {
		if (_G(thor)->_health < 150) {
			if (!soundPlaying())
				playSound(ANGEL, false);
			_G(thor)->_health += 1;
		}
	} else if (_G(thor_info)._magic < 150) {
		if (!soundPlaying())
			playSound(ANGEL, false);
		_G(thor_info)._magic += 1;
	}

	return 1;
}

// Yellow globe
int specialMovementThree(Actor *actor) {
	if (_G(thunder_flag))
		return 0;

	long lind = (long)_G(current_level);
	lind *= 1000;
	lind += (long)actor->_actorNum;
	executeScript(lind, _G(odin));

	return 0;
}

// Peg switch
int specialMovementFour(Actor *actor) {
	if (actor->_shotCountdown != 0)
		return 0;
	actor->_shotCountdown = 30;

	_G(switch_flag) = 1;
	return 0;
}

// Boulder roll
int specialMovementFive(Actor *actor) {
	int d = _G(thor)->_dir;

	if (_G(diag_flag)) {
		switch (_G(diag)) {
		case 1:
			if (_G(thor_x1) < (actor->_x + 15))
				d = 0;
			else
				d = 2;
			break;
			
		case 2:
			if (_G(thor_x2) < actor->_x)
				d = 3;
			else
				d = 0;
			break;
			
		case 3:
			if (_G(thor_x2) > (actor->_x))
				d = 1;
			else
				d = 3;
			break;
			
		case 4:
			if (_G(thor_x1) > (actor->_x + 15))
				d = 2;
			else
				d = 1;
			break;

		default:
			break;
		}
	}

	actor->_lastDir = d;
	actor->_moveType = 14;
	return 0;
}

int specialMovementSix(Actor *actor) {
	thorDamaged(actor);
	return 0;
}

int specialMovementSeven(Actor *actor) {
	if (actor->_shotCountdown != 0)
		return 0;

	actor->_shotCountdown = 30;

	_G(switch_flag) = 2;
	return 0;
}

int specialMovementEight(Actor *actor) {
	if (_G(thor)->_dir < 2 || _G(diag_flag))
		return 0;

	actor->_lastDir = _G(thor)->_dir;
	actor->_moveType = 14;
	return 0;
}

int specialMovementNine(Actor *actor) {
	if (_G(thor)->_dir > 1 || _G(diag_flag))
		return 0;

	actor->_lastDir = _G(thor)->_dir;
	actor->_moveType = 14;
	return 0;
}

int specialMovementTen(Actor *actor) {
	byte &actor_ctr = GAME1 ? actor->_temp6 : actor->_talkCounter;

	if (actor_ctr) {
		actor_ctr--;
		return 0;
	}

	if (_G(thunder_flag))
		return 0;

	actor_ctr = 10;
	actorSpeaks(actor, 0 - actor->_passValue, 0);
	return 0;
}

// Red guard
int specialMovementEleven(Actor *actor) {
	if (actor->_talkCounter) {
		actor->_talkCounter--;
		return 0;
	}

	const int oldType = actor->_type;
	actor->_type = 4;
	actorSpeaks(actor, 0, 0);
	actor->_type = oldType;
	actor->_talkCounter = 10;

	return 0;
}

// No movement - frame cycle
int movementOne(Actor *actor) {
	nextFrame(actor);
	return actor->_dir;
}

// Hammer only
int movementTwo(Actor *actor) {
	int d = actor->_lastDir;
	int x1 = actor->_x;
	int y1 = actor->_y;

	switch (d) {
	case 0:
		y1 -= 2;
		break;
		
	case 1:
		y1 += 2;
		break;
		
	case 2:
		x1 -= 2;
		break;
		
	case 3:
		x1 += 2;
		break;
		
	default:
		break;
	}
	if (!checkMove2(x1, y1, actor)) {
		if (actor->_actorNum == 1) {
			_G(hammer)->_moveType = 5;
			d = reverseDirection(_G(hammer));
			_G(hammer)->_dir = d;
		}
		if (actor->_actorNum == 2) {
			actor->_active = false;
			actor->_dead = 2;
			_G(lightning_used) = false;
			_G(tornado_used) = false;
		}
	}
	nextFrame(actor);
	actor->_lastDir = d;
	if (actor->_directions == 1)
		return 0;
	return d;
}

// Walk-bump-random turn
int movementThree(Actor *actor) {
	int d = actor->_lastDir;
	int x1 = actor->_x;
	int y1 = actor->_y;

	switch (d) {
	case 0:
		y1 -= 2;
		if (!checkMove2(x1, y1, actor)) {
			d = g_events->getRandomNumber(3);
		}
		break;
		
	case 1:
		y1 += 2;
		if (!checkMove2(x1, y1, actor)) {
			d = g_events->getRandomNumber(3);
		}
		break;
		
	case 2:
		x1 -= 2;
		if (!checkMove2(x1, y1, actor)) {
			d = g_events->getRandomNumber(3);
		}
		break;
		
	case 3:
		x1 += 2;
		if (!checkMove2(x1, y1, actor)) {
			d = g_events->getRandomNumber(3);
		}
		break;

	default:
		break;
	}
	nextFrame(actor);
	actor->_lastDir = d;
	if (actor->_directions == 1)
		return 0;
	return d;
}

// Simple tracking
int movementFour(Actor *actor) {
	int d = actor->_lastDir;

	int x1 = actor->_x;
	int y1 = actor->_y;

	int f = 0;
	if (x1 > _G(thor_x1) - 1) {
		x1 -= 2;
		d = 2;
		f = 1;
	} else if (x1 < _G(thor_x1) - 1) {
		x1 += 2;
		d = 3;
		f = 1;
	}

	if (f)
		f = checkMove2(x1, y1, actor);

	if (!f) {
		if (y1 < (_G(thor_real_y1))) {
			d = (_G(thor_real_y1)) - y1;
			if (d > 2)
				d = 2;
			y1 += d;
			d = 1;
			f = 1;
		} else if (y1 > (_G(thor_real_y1))) {
			d = y1 - (_G(thor_real_y1));
			if (d > 2)
				d = 2;
			y1 -= d;
			d = 0;
			f = 1;
		}
		if (f)
			f = checkMove2(actor->_x, y1, actor);
		if (!f)
			checkMove2(actor->_x, actor->_y, actor);
	}
	nextFrame(actor);
	actor->_lastDir = d;
	if (actor->_directions == 1)
		return 0;
	return d;
}

int movementFive(Actor *actor) {
	int x1 = actor->_x;
	int y1 = actor->_y;
	int xd = 0;
	int yd = 0;
	int d = actor->_lastDir;

	if (x1 > (_G(thor)->_x + 1))
		xd = -2; //+1
	else if (x1 < (_G(thor)->_x - 1))
		xd = 2;

	if (actor->_actorNum == 1) {
		if (y1 < (_G(thor_y1) - 6))
			yd = 2;
		else if (y1 > (_G(thor_y1) - 6))
			yd = -2;
	} else {
		if (y1 < (_G(thor_real_y1) - 1))
			yd = 2;
		else if (y1 > (_G(thor_real_y1) + 1))
			yd = -2;
	}

	if (xd && yd) {
		if (xd == -2 && ABS(yd) == 2)
			d = 2;
		else if (xd == 2 && ABS(yd) == 2)
			d = 3;

		x1 += xd;
		y1 += yd;
		if (checkMove2(x1, y1, actor)) {
			nextFrame(actor);
			actor->_lastDir = d;
			if (actor->_directions == 1)
				return 0;
			return d;
		}
	} else {
		if (xd == 0 && yd == 2)
			d = 1;
		else if (xd == 0 && yd == -2)
			d = 0;
		else if (xd == 2 && yd == 0)
			d = 3;
		else if (xd == -2 && yd == 0)
			d = 2;
	}
	x1 = actor->_x;
	y1 = actor->_y;
	actor->_toggle ^= 1;

	if (actor->_toggle) {
		if (xd) {
			x1 += xd;
			if (checkMove2(x1, y1, actor)) {
				if (xd > 0)
					d = 3;
				else
					d = 2;
				nextFrame(actor);
				actor->_lastDir = d;
				if (actor->_directions == 1)
					return 0;
				return d;
			}
			x1 = actor->_x;
		}
		if (yd) {
			y1 += yd;
			if (checkMove2(x1, y1, actor)) {
				if (yd > 0)
					d = 1;
				else
					d = 0;
				nextFrame(actor);
				actor->_lastDir = d;
				if (actor->_directions == 1)
					return 0;
				return d;
			}
		}
	} else {
		if (yd) {
			y1 += yd;
			if (checkMove2(x1, y1, actor)) {
				if (yd > 0)
					d = 1;
				else
					d = 0;
				nextFrame(actor);
				actor->_lastDir = d;
				if (actor->_directions == 1)
					return 0;
				return d;
			}
			y1 = actor->_y;
		}
		if (xd) {
			x1 += xd;
			if (checkMove2(x1, y1, actor)) {
				if (xd > 0)
					d = 3;
				else
					d = 2;
				nextFrame(actor);
				actor->_lastDir = d;
				if (actor->_directions == 1)
					return 0;
				return d;
			}
		}
	}
	checkMove2(actor->_x, actor->_y, actor);
	nextFrame(actor);
	actor->_lastDir = d;
	if (actor->_directions == 1)
		return 0;
	return d;
}

// Explosion only
int movementSix(Actor *actor) {
	if (actor->_currNumShots > 0) {
		actor->_nextFrame++;
		if (actor->_nextFrame > 2) {
			actor->_nextFrame = 0;
			if (_G(boss_dead))
				playSound(EXPLODE, false);
		}
		actor->_currNumShots--;
	} else {
		actor->_dead = 2;
		actor->_active = false;
		if (!_G(boss_dead) && !_G(endgame)) {
			if (actor->_type == 2)
				dropRandomObject(actor);
		}
	}

	nextFrame(actor);
	return 0;
}

// Walk-bump-random turn (pause also)
int movementSeven(Actor *actor) {
	if (actor->_nextFrame == 0 && actor->_frameCount == actor->_frameSpeed) {
		actor->_moveCountdown = 12;
		actor->_lastDir = g_events->getRandomNumber(3);
	}
	return movementThree(actor);
}

// Follow thor
int movementEight(Actor *actor) {
	if (_G(thor)->_x > 0)
		actor->_x = _G(thor)->_x - 1;
	else
		actor->_x = _G(thor)->_x;
	actor->_y = _G(thor)->_y;
	nextFrame(actor);
	return 0;
}

// 4-way straight (random length) change
int movementNine(Actor *actor) {
	int d = actor->_lastDir;
	int x1 = actor->_x;
	int y1 = actor->_y;

	int f = 0;
	if (actor->_counter) {
		actor->_counter--;
		switch (d) {
		case 0:
			y1 -= 2;
			if (!checkMove4(x1, y1, actor))
				f = 1;
			break;
			
		case 1:
			y1 += 2;
			if (!checkMove4(x1, y1, actor))
				f = 1;
			break;
			
		case 2:
			x1 -= 2;
			if (!checkMove4(x1, y1, actor))
				f = 1;
			break;
			
		case 3:
			x1 += 2;
			if (!checkMove4(x1, y1, actor))
				f = 1;
			break;

		default:
			break;
		}
	} else
		f = 1;

	if (f == 1) {
		actor->_counter = g_events->getRandomNumber(10, 99);
		d = g_events->getRandomNumber(3);
	}
	nextFrame(actor);
	actor->_lastDir = d;
	if (actor->_directions == 1)
		return 0;
	return d;
}

// Vertical straight (random length) change
int movementTen(Actor *actor) {
	int lastDir = actor->_lastDir;
	const int x1 = actor->_x;
	int y1 = actor->_y;

	bool setRandomDirFl = false;
	if (actor->_counter) {
		if (actor->_passValue != 1)
			actor->_counter--;
		switch (lastDir) {
		case 0:
		case 2:
			y1 -= 2;
			if (!checkMove2(x1, y1, actor))
				setRandomDirFl = true;
			break;
		case 1:
		case 3:
			y1 += 2;
			if (!checkMove2(x1, y1, actor))
				setRandomDirFl = true;
			break;

		default:
			break;
		}
	} else
		setRandomDirFl = true;

	if (setRandomDirFl) {
		actor->_counter = g_events->getRandomNumber(10, 99);
		lastDir = g_events->getRandomNumber(1);
	}

	if (lastDir > 1)
		lastDir -= 2;

	nextFrame(actor);
	actor->_lastDir = lastDir;
	if (actor->_directions == 1)
		return 0;
	return lastDir;
}

// Horizontal only (bats)
int movementEleven(Actor *actor) {
	int d = actor->_lastDir;

	switch (d) {
	case 0:
		if (checkMove2(actor->_x - 2, actor->_y - 2, actor))
			break;
		d = 1;
		if (checkMove2(actor->_x - 2, actor->_y + 2, actor))
			break;
		d = 2;
		break;
		
	case 1:
		if (checkMove2(actor->_x - 2, actor->_y + 2, actor))
			break;
		d = 0;
		if (checkMove2(actor->_x - 2, actor->_y - 2, actor))
			break;
		d = 3;
		break;
		
	case 2:
		if (checkMove2(actor->_x + 2, actor->_y - 2, actor))
			break;
		d = 3;
		if (checkMove2(actor->_x + 2, actor->_y + 2, actor))
			break;
		d = 0;
		break;
		
	case 3:
		if (checkMove2(actor->_x + 2, actor->_y + 2, actor))
			break;
		d = 2;
		if (checkMove2(actor->_x + 2, actor->_y - 2, actor))
			break;
		d = 1;
		break;

	default:
		break;
		
	}

	nextFrame(actor);
	actor->_lastDir = d;
	if (actor->_directions == 1)
		return 0;
	return d;
}

// Horizontal straight until bump
int movementTwelve(Actor *actor) {
	int d = actor->_lastDir;

	switch (d) {
	case 0:
	case 2:
		if (checkMove2(actor->_x - 2, actor->_y, actor))
			break;
		d = 3;
		break;
		
	case 1:
	case 3:
		if (checkMove2(actor->_x + 2, actor->_y, actor))
			break;
		d = 2;
		break;
		
	default:
		break;
	}
	nextFrame(actor);
	actor->_lastDir = d;
	if (actor->_directions == 1)
		return 0;
	return d;
}

// Pause-seek (mushroom)
int movementThirteen(Actor *actor) {
	const int d = actor->_lastDir;

	if (actor->_counter == 0 && actor->_unpauseCountdown == 0) {
		actor->_unpauseCountdown = 60;
		return d;
	}
	if (actor->_unpauseCountdown > 0) {
		actor->_unpauseCountdown--;
		if (!actor->_unpauseCountdown)
			actor->_counter = 60;
		actor->_vulnerableCountdown = 5;
		actor->_hitStrength = 0;
		return d;
	}
	if (actor->_counter > 0) {
		actor->_counter--;
		if (!actor->_counter)
			actor->_unpauseCountdown = 60;
		actor->_hitStrength = 10;
		return movementFive(actor);
	}
	return d;
}

// Move-bump-stop (boulder)
int movementFourteen(Actor *actor) {
	const int d = actor->_lastDir;
	actor->_dir = d;
	int x1 = actor->_x;
	int y1 = actor->_y;

	switch (d) {
	case 0:
		y1 -= 2;
		if (!checkMove2(x1, y1, actor)) {
			actor->_moveType = 15;
			return 0;
		}
		break;
		
	case 1:
		y1 += 2;
		if (!checkMove2(x1, y1, actor)) {
			actor->_moveType = 15;
			return 0;
		}
		break;
		
	case 2:
		x1 -= 2;
		if (!checkMove2(x1, y1, actor)) {
			actor->_moveType = 15;
			return 0;
		}
		break;
		
	case 3:
		x1 += 2;
		if (!checkMove2(x1, y1, actor)) {
			actor->_moveType = 15;
			return 0;
		}
		break;

	default:
		break;
	}
	nextFrame(actor);
	actor->_lastDir = d;
	if (actor->_directions == 1)
		return 0;
	return d;
}

// No movement - no frame cycle
int movementFifteen(Actor *actor) {
	return actor->_dir;
}

// Tornado 1
int movementSixteen(Actor *actor) {
	int d = actor->_lastDir;
	int x1 = actor->_x;
	int y1 = actor->_y;

	switch (d) {
	case 0:
		y1 -= 2;
		break;
		
	case 1:
		y1 += 2;
		break;
		
	case 2:
		x1 -= 2;
		break;
		
	case 3:
		x1 += 2;
		break;
		
	default:
		break;
	}

	if (!checkMove1(x1, y1, actor)) {
		actor->_moveType = 17;
		d = g_events->getRandomNumber(3);
	}

	nextFrame(actor);
	actor->_lastDir = d;
	if (actor->_directions == 1)
		return 0;
	return d;
}

// Tornado 2
int movementSeventeen(Actor *actor) {
	int d = actor->_lastDir;

	switch (d) {
	case 0:
		if (checkMove1(actor->_x - 2, actor->_y - 2, actor))
			break;
		d = 1;
		if (checkMove1(actor->_x - 2, actor->_y + 2, actor))
			break;
		d = 2;
		break;
		
	case 1:
		if (checkMove1(actor->_x - 2, actor->_y + 2, actor))
			break;
		d = 0;
		if (checkMove1(actor->_x - 2, actor->_y - 2, actor))
			break;
		d = 3;
		break;
		
	case 2:
		if (checkMove1(actor->_x + 2, actor->_y - 2, actor))
			break;
		d = 3;
		if (checkMove1(actor->_x + 2, actor->_y + 2, actor))
			break;
		d = 0;
		break;
		
	case 3:
		if (checkMove1(actor->_x + 2, actor->_y + 2, actor))
			break;
		d = 2;
		if (checkMove1(actor->_x + 2, actor->_y - 2, actor))
			break;
		d = 1;
		break;
		
	default:
		break;
	}
	
	nextFrame(actor);
	actor->_lastDir = d;
	if (actor->_directions == 1)
		return 0;
	return d;
}

// No movement - frame cycle
int movementEighteen(Actor *actor) {
	int d;

	if (actor->_temp5) {
		actor->_temp5--;
		if (!actor->_temp5)
			actor->_numMoves = 1;
	}
	if (actor->_temp1) {
		d = movementFive(actor);
		actor->_rand--;
		if (actor->_hitThor || !actor->_rand) {
			if (actor->_hitThor) {
				actor->_temp5 = 50;
				actor->_numMoves = 2;
				actor->_hitThor = false;
				actor->_dir = d;
				d = reverseDirection(actor);
			}
			actor->_temp1 = 0;
			actor->_rand = g_events->getRandomNumber(50, 149);
		}
	} else {
		d = movementThree(actor);
		actor->_rand--;
		if (!actor->_rand) {
			actor->_temp5 = 0;
			actor->_temp1 = 1;
			actor->_rand = g_events->getRandomNumber(50, 149);
		}
		if (actor->_hitThor) {
			actor->_temp5 = 50;
			actor->_numMoves = 2;
			actor->_hitThor = false;
			actor->_dir = d;
			d = reverseDirection(actor);
		}
	}
	nextFrame(actor);
	return d;
}

// No movement - frame cycle
int movementNineteen(Actor *actor) {
	return movementSeven(actor);
}

int movementTwenty(Actor *actor) {
	if (GAME1)
		// Boss - snake
		return boss1Movement(actor);

	return movementOne(actor);
}

// No movement - frame cycle
int movementTwentyOne(Actor *actor) {
	return movementThree(actor);
}

// Spear
int movementTwentyTwo(Actor *actor) {
	int d = actor->_lastDir;
	if (actor->_directions == 1)
		d = 0;

redo:

	switch (actor->_temp2) {
	case 0:
		if (backgroundTile(actor->_x, actor->_y) >= TILE_SOLID)
			actor->_nextFrame = 1;
		else {
			actor->_temp2 = 6;
			actor->_temp1 = 1;
			goto redo;
		}
		actor->_temp2++;
		break;
		
	case 1:
		actor->_nextFrame = 2;
		actor->_temp2++;
		break;
		
	case 2:
		actor->_nextFrame = 3;
		actor->_hitStrength = 255;
		actor->_temp2++;
		actor->_temp1 = 10;
		break;
		
	case 3:
		checkMove2(actor->_x, actor->_y, actor);
		actor->_temp1--;
		if (actor->_temp1)
			break;
		actor->_temp2++;
		actor->_nextFrame = 2;
		break;
		
	case 4:
		actor->_hitStrength = 0;
		actor->_temp2++;
		actor->_nextFrame = 1;
		break;
		
	case 5:
		actor->_temp2++;
		actor->_nextFrame = 0;
		actor->_temp1 = 10;
		break;
		
	case 6:
		actor->_temp1--;
		if (actor->_temp1)
			break;
		actor->_temp2 = 0;
		actor->_nextFrame = 0;
		switch (d) {
		case 0:
			actor->_x += 16;
			actor->_y += 16;
			d = 3;
			if (backgroundTile(actor->_x, actor->_y) < TILE_SOLID)
				goto redo;
			break;
			
		case 1:
			actor->_x -= 16;
			actor->_y -= 16;
			d = 2;
			if (backgroundTile(actor->_x, actor->_y) < TILE_SOLID)
				goto redo;
			break;
			
		case 2:
			actor->_x += 16;
			actor->_y -= 16;
			d = 0;
			if (backgroundTile(actor->_x, actor->_y) < TILE_SOLID)
				goto redo;
			break;
			
		case 3:
			actor->_x -= 16;
			actor->_y += 16;
			d = 1;
			if (backgroundTile(actor->_x, actor->_y) < TILE_SOLID)
				goto redo;
			break;
		default:
			break;
		}
		actor->_dir = d;
		actor->_lastDir = d;
		break;
		
	default:
		break;
	}
	return d;
}

// Spinball counter-clockwise
int movementTwentyThree(Actor *actor) {
	int d = actor->_lastDir;
	nextFrame(actor);
	if (actor->_passValue & 2)
		actor->_numMoves = 2;

	switch (d) {
	case 0:
		if (backgroundTile(actor->_x - 2, actor->_y) >= TILE_FLY &&
			backgroundTile(actor->_x - 2, actor->_y + actor->_sizeY - 1) >= TILE_FLY) {
			d = 2;
			actor->_x -= 2;
		} else {
			if (backgroundTile(actor->_x, actor->_y - 2) < TILE_FLY ||
				backgroundTile(actor->_x + actor->_sizeX - 1, actor->_y - 2) < TILE_FLY) {
				if (backgroundTile(actor->_x + actor->_sizeX + 1, actor->_y) >= TILE_FLY &&
					backgroundTile(actor->_x + actor->_sizeX + 1, actor->_y + actor->_sizeY - 1) >= TILE_FLY) {
					d = 3;
					actor->_x += 2;
				} else {
					d = 1;
					break;
				}
			} else
				actor->_y -= 2;
		}
		break;
		
	case 1:
		if (backgroundTile(actor->_x + actor->_sizeX + 1, actor->_y) >= TILE_FLY &&
			backgroundTile(actor->_x + actor->_sizeX + 1, actor->_y + actor->_sizeY - 1) >= TILE_FLY) {
			d = 3;
			actor->_x += 2;
		} else {
			if (backgroundTile(actor->_x, actor->_y + actor->_sizeY + 1) < TILE_FLY ||
				backgroundTile(actor->_x + actor->_sizeX - 1, actor->_y + actor->_sizeY + 1) < TILE_FLY) {
				if (backgroundTile(actor->_x - 2, actor->_y) >= TILE_FLY &&
					backgroundTile(actor->_x - 2, actor->_y + actor->_sizeY - 1) >= TILE_FLY) {
					d = 2;
					actor->_x -= 2;
				} else {
					d = 0;
					break;
				}
			} else
				actor->_y += 2;
		}
		break;
		
	case 2:
		if (backgroundTile(actor->_x, actor->_y + actor->_sizeY + 1) >= TILE_FLY &&
			backgroundTile(actor->_x + actor->_sizeX - 1, actor->_y + actor->_sizeY + 1) >= TILE_FLY) {
			d = 1;
			actor->_y += 2;
		} else {
			if (backgroundTile(actor->_x - 2, actor->_y) < TILE_FLY ||
				backgroundTile(actor->_x - 2, actor->_y + actor->_sizeY - 1) < TILE_FLY) {
				if (backgroundTile(actor->_x, actor->_y - 2) >= TILE_FLY &&
					backgroundTile(actor->_x + actor->_sizeX - 1, actor->_y - 2) >= TILE_FLY) {
					d = 0;
					actor->_y -= 2;
				} else {
					d = 3;
					break;
				}
			} else
				actor->_x -= 2;
		}
		break;
		
	case 3:
		if (backgroundTile(actor->_x, actor->_y - 2) >= TILE_FLY &&
			backgroundTile(actor->_x + actor->_sizeX - 1, actor->_y - 2) >= TILE_FLY) {
			d = 0;
			actor->_y -= 2;
		} else {
			if (backgroundTile(actor->_x + actor->_sizeX + 1, actor->_y) < TILE_FLY ||
				backgroundTile(actor->_x + actor->_sizeX + 1, actor->_y + actor->_sizeY - 1) < TILE_FLY) {
				if (backgroundTile(actor->_x, actor->_y + actor->_sizeY + 1) >= TILE_FLY &&
					backgroundTile(actor->_x + actor->_sizeX - 1, actor->_y + actor->_sizeY + 1) >= TILE_FLY) {
					d = 1;
					actor->_y += 2;
				} else {
					d = 2;
					break;
				}
			} else
				actor->_x += 2;
		}
		break;

	default:
		break;
	}
	
	checkMove2(actor->_x, actor->_y, actor);
	actor->_lastDir = d;
	if (actor->_directions == 1)
		return 0;
	return d;
}

// Spinball  clockwise
int movementTwentyFour(Actor *actor) {
	int d = actor->_lastDir;
	nextFrame(actor);
	if (actor->_passValue & 2)
		actor->_numMoves = 2;

	switch (d) {
	case 0:
		if (backgroundTile(actor->_x + actor->_sizeX + 1, actor->_y) >= TILE_FLY &&
			backgroundTile(actor->_x + actor->_sizeX + 1, actor->_y + actor->_sizeY - 1) >= TILE_FLY) {
			d = 3;
			actor->_x += 2;
		} else {
			if (backgroundTile(actor->_x, actor->_y - 2) < TILE_FLY ||
				backgroundTile(actor->_x + actor->_sizeX - 1, actor->_y - 2) < TILE_FLY) {
				if (backgroundTile(actor->_x - 2, actor->_y) >= TILE_FLY &&
					backgroundTile(actor->_x - 2, actor->_y + actor->_sizeY - 1) >= TILE_FLY) {
					d = 2;
					actor->_x -= 2;
				} else {
					d = 1;
					break;
				}
			} else
				actor->_y -= 2;
		}
		break;
		
	case 1:
		if (backgroundTile(actor->_x - 2, actor->_y) >= TILE_FLY &&
			backgroundTile(actor->_x - 2, actor->_y + actor->_sizeY - 1) >= TILE_FLY) {
			d = 2;
			actor->_x -= 2;
		} else {
			if (backgroundTile(actor->_x, actor->_y + actor->_sizeY + 1) < TILE_FLY ||
				backgroundTile(actor->_x + actor->_sizeX - 1, actor->_y + actor->_sizeY + 1) < TILE_FLY) {
				if (backgroundTile(actor->_x + actor->_sizeX + 1, actor->_y) >= TILE_FLY &&
					backgroundTile(actor->_x + actor->_sizeX + 1, actor->_y + actor->_sizeY - 1) >= TILE_FLY) {
					d = 3;
					actor->_x += 2;
				} else {
					d = 0;
					break;
				}
			} else
				actor->_y += 2;
		}
		break;
		
	case 2:
		if (backgroundTile(actor->_x, actor->_y - 2) >= TILE_FLY &&
			backgroundTile(actor->_x + actor->_sizeX - 1, actor->_y - 2) >= TILE_FLY) {
			d = 0;
			actor->_y -= 2;
		} else {
			if (backgroundTile(actor->_x - 2, actor->_y) < TILE_FLY ||
				backgroundTile(actor->_x - 2, actor->_y + actor->_sizeY - 1) < TILE_FLY) {
				if (backgroundTile(actor->_x, actor->_y + actor->_sizeY + 1) >= TILE_FLY &&
					backgroundTile(actor->_x + actor->_sizeX - 1, actor->_y + actor->_sizeY + 1) >= TILE_FLY) {
					d = 1;
					actor->_y += 2;
				} else {
					d = 3;
					break;
				}
			} else
				actor->_x -= 2;
		}
		break;
		
	case 3:
		if (backgroundTile(actor->_x, actor->_y + actor->_sizeY + 1) >= TILE_FLY &&
			backgroundTile(actor->_x + actor->_sizeX - 1, actor->_y + actor->_sizeY + 1) >= TILE_FLY) {
			d = 1;
			actor->_y += 2;
		} else {
			if (backgroundTile(actor->_x + actor->_sizeX + 1, actor->_y) < TILE_FLY ||
				backgroundTile(actor->_x + actor->_sizeX + 1, actor->_y + actor->_sizeY - 1) < TILE_FLY) {
				if (backgroundTile(actor->_x, actor->_y - 2) >= TILE_FLY &&
					backgroundTile(actor->_x + actor->_sizeX - 1, actor->_y - 2) >= TILE_FLY) {
					d = 0;
					actor->_y -= 2;
				} else {
					d = 2;
					break;
				}
			} else
				actor->_x += 2;
		}
		break;

	default:
		break;
	}
	
	checkMove2(actor->_x, actor->_y, actor);
	actor->_lastDir = d;
	
	if (actor->_directions == 1)
		return 0;
	return d;
}

// Acid puddle
int movementTwentyFive(Actor *actor) {
	if (actor->_temp2) {
		actor->_temp2--;
		return movementOne(actor);
	}
	if (!actor->_temp1) {
		actor->_lastDir = g_events->getRandomNumber(3);
		int i = 4;
		while (i--) {
			int ret = movementThree(actor);
			if (ret)
				return ret;
			actor->_lastDir++;
			if (actor->_lastDir > 3)
				actor->_lastDir = 0;
		}
		actor->_temp1 = 16;
	}
	actor->_temp1--;
	return movementThree(actor);
}

int movementTwentySix(Actor *actor) {
	if (GAME2)
		return boss2Movement(actor);
	if (GAME3)
		return boss3Movement(actor);

	return movementOne(actor);
}

int movementTwentySeven(Actor *actor) {
	if (GAME2)
		return boss2Movement(actor);

	return movementOne(actor);
}

void setThorVars() {
	_G(thor_x1) = _G(thor)->_x + 1;
	_G(thor_y1) = _G(thor)->_y + 8;

	_G(thor_real_y1) = _G(thor)->_y;
	_G(thor_x2) = (_G(thor)->_x + 12);
	_G(thor_y2) = _G(thor)->_y + 15;
}

// Fish
int movementTwentyEight(Actor *actor) {
	const int d = actor->_lastDir;
	int x1 = actor->_x;
	int y1 = actor->_y;
	int ret;
	
	if (actor->_i1)
		actor->_i1--;
	else {
		if (!actor->_temp3) {
			if (!actor->_nextFrame) {
				actor->_frameCount = 1;
				actor->_frameSpeed = 4;
			}

			nextFrame(actor);
			if (actor->_nextFrame == 3) {
				if (actor->_currNumShots < actor->_numShotsAllowed)
					actorShoots(actor, 0);
				actor->_temp3 = 1;
			}
		} else {
			const int fcount = actor->_frameCount - 1;
			if (fcount <= 0) {
				actor->_nextFrame--;
				actor->_frameCount = actor->_frameSpeed;
				if (!actor->_nextFrame) {
					actor->_temp3 = 0;
					actor->_frameSpeed = 4;
					actor->_i1 = g_events->getRandomNumber(60, 159);
				}
			} else
				actor->_frameCount = fcount;
		}
		goto done;
	}
	switch (actor->_temp2) {
	case 0:
		y1 -= 2;
		break;
	case 1:
		y1 += 2;
		break;
	case 2:
		x1 -= 2;
		break;
	case 3:
		x1 += 2;
		break;
	default:
		break;
	}
	
	ret = backgroundTile(x1, y1);
	if (ret != 100 && ret != 106 && ret != 110 && ret != 111 && ret != 113)
		goto chg_dir;
	ret = backgroundTile((x1 + actor->_sizeX) - 1, y1);
	if (ret != 100 && ret != 106 && ret != 110 && ret != 111 && ret != 113)
		goto chg_dir;
	ret = backgroundTile(x1, (y1 + actor->_sizeY) - 1);
	if (ret != 100 && ret != 106 && ret != 110 && ret != 111 && ret != 113)
		goto chg_dir;
	ret = backgroundTile((x1 + actor->_sizeX) - 1, (y1 + actor->_sizeY) - 1);
	if (ret != 100 && ret != 106 && ret != 110 && ret != 111 && ret != 113)
		goto chg_dir;

	actor->_x = x1;
	actor->_y = y1;

	goto done;

chg_dir:
	actor->_temp2 = _G(rand1) % 4;

done:
	if (actor->_nextFrame) {
		x1 = actor->_x;
		y1 = actor->_y;
		actor->_solid = 1;
		checkMove2(x1, y1, actor);
		actor->_x = x1;
		actor->_y = y1;
	} else
		actor->_solid = 2;
	if (actor->_directions == 1)
		return 0;
	return d;
}

// Horizontal or vertical (pass_val)
int movementTwentyNine(Actor *actor) {
	if (!actor->_passValue)
		return movementThirty(actor);

	return movementTwelve(actor);
}

// Vertical straight
int movementThirty(Actor *actor) {
	int d = actor->_lastDir;
	const int x1 = actor->_x;
	int y1 = actor->_y;

	switch (d) {
	case 0:
		y1 -= 2;
		break;
	case 1:
		y1 += 2;
		break;
	default:
		break;
	}
	if (!checkMove2(x1, y1, actor))
		d ^= 1;

	nextFrame(actor);
	actor->_lastDir = d;
	if (actor->_directions == 1)
		return 0;
	return d;
}

// Drop (stalagtite)
int movementThirtyOne(Actor *actor) {
	const int d = actor->_lastDir;
	int x1 = actor->_x;
	int y1 = actor->_y;

	if (actor->_temp1) {
		y1 += 2;
		if (!checkMove2(x1, y1, actor))
			actorDestroyed(actor);
	} else if (_G(thor_y1) > y1 && ABS(x1 - _G(thor_x1)) < 16) {
		const int cx = (actor->_x + (actor->_sizeX / 2)) >> 4;
		const int cy = ((actor->_y + actor->_sizeY) - 2) >> 4;
		const int ty = _G(thor)->_centerY;
		for (int i = cy; i <= ty; i++)
			if (_G(scrn)._iconGrid[i][cx] < TILE_SOLID)
				goto done;
		actor->_numMoves = actor->_passValue + 1;
		actor->_temp1 = 1;
	}

done:
	nextFrame(actor);
	actor->_lastDir = d;
	if (actor->_directions == 1)
		return 0;
	return d;
}

// Unused
int movementThirtyTwo(Actor *actor) {
	return 0;
}

// Unused
int movementThirtyThree(Actor *actor) {
	return 0;
}

// Unused
int movementThirtyFour(Actor *actor) {
	return 0;
}

// Gun (single)
int movementThirtyFive(Actor *actor) {
	actor->_nextFrame = actor->_lastDir;
	return actor->_dir;
}

// Acid drop
int movementThirtySix(Actor *actor) {
	actor->_speed = actor->_passValue;
	nextFrame(actor);
	if (actor->_nextFrame == 0 && actor->_frameCount == actor->_frameSpeed) {
		actorAlwaysShoots(actor, 1);
		_G(actor[actor->_shotActor])._x -= 2;
	}
	return 0;
}

// 4-way straight (random length) change
int movementThirtySeven(Actor *actor) {
	int d = actor->_lastDir;
	int x1 = actor->_x;
	int y1 = actor->_y;

	bool check = false;
	if (actor->_counter) {
		actor->_counter--;
		switch (d) {
		case 0:
			y1 -= 2;
			if (!checkMove2(x1, y1, actor))
				check = true;
			break;
			
		case 1:
			y1 += 2;
			if (!checkMove2(x1, y1, actor))
				check = true;
			break;
			
		case 2:
			x1 -= 2;
			if (!checkMove2(x1, y1, actor))
				check = true;
			break;
			
		case 3:
			x1 += 2;
			if (!checkMove2(x1, y1, actor))
				check = true;
			break;

		default:
			break;
		}
	} else
		check = true;

	if (check) {
		actor->_counter = g_events->getRandomNumber(10, 99);
		d = g_events->getRandomNumber(3);
	}
	
	nextFrame(actor);
	actor->_lastDir = d;
	if (actor->_directions == 1)
		return 0;
	return d;
}

// Timed darting
#define TIMER actor->_i1
#define INIT_DIR actor->_temp1
#define OTHER_DIR actor->_temp2
#define FLAG actor->_temp3
#define OX actor->_i2
#define OY actor->_i3
#define CNT actor->_i4
int movementThirtyEight(Actor *actor) {
	int d = actor->_lastDir;
	int x1 = actor->_x;
	int y1 = actor->_y;

	if (!FLAG) {
		FLAG = 1;
		if (actor->_passValue)
			TIMER = actor->_passValue * 15;
		else
			TIMER = g_events->getRandomNumber(5, 364);
		INIT_DIR = actor->_lastDir;
		OX = x1;
		OY = y1;
		CNT = 0;
		switch (INIT_DIR) {
		case 0:
			OTHER_DIR = 1;
			break;
		case 1:
			OTHER_DIR = 0;
			break;
		case 2:
			OTHER_DIR = 3;
			break;
		case 3:
			OTHER_DIR = 2;
			break;
		default:
			break;
		}
	}
	if (TIMER) {
		TIMER--;
		goto done;
	}

	switch (d) {
	case 0:
		y1 -= 2;
		break;
	case 1:
		y1 += 2;
		break;
	case 2:
		x1 -= 2;
		break;
	case 3:
		x1 += 2;
		break;
	default:
		break;
	}

	switch (FLAG) {
	case 1:
		if (!checkMove2(x1, y1, actor)) {
			if (CNT) {
				d = OTHER_DIR;
				actor->_lastDir = d;
				FLAG = 2;
			} else {
				actor->_nextFrame = 0;
				FLAG = 0;
				goto done;
			}
		} else
			CNT++;
		break;
		
	case 2:
		checkMove2(x1, y1, actor);
		if (x1 == OX && y1 == OY) {
			FLAG = 0;
			d = INIT_DIR;
			actor->_lastDir = d;
			actor->_nextFrame = 0;
			goto done;
		}
		break;

	default:
		break;
	}
	nextFrame(actor);

done:
	actor->_lastDir = d;
	if (actor->_directions == 1)
		return 0;
	return d;
}

// Troll 1
int movementThirtyNine(Actor *actor) {
	if (_G(setup)._difficultyLevel == 0) {
		actor->_speed = 3;
		actor->_numMoves = 1;
	} else if (_G(setup)._difficultyLevel == 1) {
		actor->_speed = 2;
		actor->_numMoves = 1;
	} else if (_G(setup)._difficultyLevel == 2) {
		actor->_speed = 1;
		actor->_numMoves = 1;
	}

	if (actor->_passValue < 5)
		return movementForty(actor);
	
	if (actor->_passValue == 10) {
		if (overlap(actor->_x + 1, actor->_y + 1, actor->_x + actor->_sizeX - 1,
					actor->_y + actor->_sizeY - 1, _G(thor_x1), _G(thor_y1), _G(thor_x2), _G(thor_y2))) {
			actor->_hitStrength = 255;
			thorDamaged(actor);
		}
		return actor->_dir;
	}
	if (actor->_actorNum != 3)
		return actor->_dir;

	if (actor->_i1) {
		actor->_i1--;
		actor->_x -= 2;
		checkMove2(actor->_x, actor->_y, actor);
		_G(actor[4])._x -= 2;
		_G(actor[5])._x -= 2;
		_G(actor[6])._x -= 2;
	}
	nextFrame(actor);
	if (actor->_nextFrame == 3)
		actor->_nextFrame = 0;
	_G(actor[4])._nextFrame = _G(actor[3])._nextFrame;
	_G(actor[5])._nextFrame = 0;
	_G(actor[6])._nextFrame = 0;
	return actor->_dir;
}

// Troll 2
int movementForty(Actor *actor) {
	if (overlap(actor->_x + 1, actor->_y + 1, actor->_x + actor->_sizeX + 3,
				actor->_y + actor->_sizeY - 1, _G(thor_x1), _G(thor_y1), _G(thor_x2), _G(thor_y2))) {
		actor->_hitStrength = 150;
		thorDamaged(actor);
	}
	int a = 5 + (actor->_passValue * 4);
	const int x1 = actor->_x;
	int d = actor->_lastDir;

	if (actor->_lastDir == 2) {
		if (backgroundTile(x1 - 2, actor->_y) >= TILE_SOLID) {
			_G(actor[a]._x) -= 2;
			_G(actor[a - 1])._x -= 2;
			_G(actor[a - 2])._x -= 2;
			_G(actor[a + 1])._x -= 2;
		} else
			d = 3;
	} else if (backgroundTile(_G(actor[a + 1])._x + 14, _G(actor[a + 1])._y) >= TILE_SOLID) {
		_G(actor[a])._x += 2;
		_G(actor[a - 1])._x += 2;
		_G(actor[a - 2])._x += 2;
		_G(actor[a + 1])._x += 2;
	} else
		d = 2;

	if (actor->_nextFrame == 3 && !actor->_currNumShots && actor->_frameCount == actor->_frameSpeed) {
		actorAlwaysShoots(actor, 1);
		_G(actor[actor->_shotActor])._x += 6;
	}

	nextFrame(actor);
	_G(actor[a - 2])._nextFrame = actor->_nextFrame;
	_G(actor[a - 1])._nextFrame = actor->_nextFrame;
	_G(actor[a + 1])._nextFrame = actor->_nextFrame;
	_G(actor[a - 2])._lastDir = d;
	_G(actor[a - 1])._lastDir = d;
	_G(actor[a + 1])._lastDir = d;
	actor->_lastDir = d;
	if (actor->_directions == 1)
		return 0;
	return d;
}

} // namespace Got
