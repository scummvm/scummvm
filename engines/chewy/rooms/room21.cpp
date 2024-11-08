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

#include "chewy/cursor.h"
#include "chewy/defines.h"
#include "chewy/events.h"
#include "chewy/globals.h"
#include "chewy/ani_dat.h"
#include "chewy/room.h"
#include "chewy/rooms/room21.h"

namespace Chewy {
namespace Rooms {

#define SPINNE1_OBJ 0
#define SPINNE2_OBJ 1
#define ENERGIE_OBJ 2

static const int16 SPINNE_PHASEN[4][2] = {
	{ 58, 65 },
	{ 50, 57 },
	{ 42, 49 },
	{ 66, 89 }
};

static const MovLine SPINNE_MPKT[3] = {
	{ { 550, 275, 0 }, 0, 2 },
	{ { 396, 275, 0 }, 0, 2 },
	{ { 530, 275, 0 }, 1, 2 }
};

static const MovLine SPINNE_MPKT1[2] = {
	{ { 104,  50, 0 }, 3, 3 },
	{ { 104, 430, 0 }, 3, 3 }
};

static const MovLine SPINNE_MPKT2[2] = {
	{ { 115, 140, 0 }, 2, 1 },
	{ { 115, 190, 0 }, 2, 1 }
};

static const AniBlock ABLOCK19[3] = {
	{ 12, 1, ANI_FRONT, ANI_WAIT, 0 },
	{ 13, 4, ANI_FRONT, ANI_WAIT, 0 },
	{ 14, 1, ANI_FRONT, ANI_WAIT, 0 },
};

void Room21::entry() {
	_G(gameState).ScrollxStep = 2;
	_G(gameState).ScrollyStep = 2;
	load_chewy_taf(CHEWY_MINI);
	calc_laser();
	init_spinne();
	_G(timer_nr)[2] = _G(room)->set_timer(255, 1);
	_G(flags).NoEndPosMovObj = true;
	_G(SetUpScreenFunc) = setup_func;
}

bool Room21::timer(int16 t_nr, int16 ani_nr) {
	if (t_nr == _G(timer_nr)[0]) {
		restart_spinne2();
	} else if (t_nr == _G(timer_nr)[2])
		chewy_kolli();

	return false;
}

void Room21::calc_laser() {
	if (_G(gameState).R21Lever1 && !_G(gameState).R21Lever2 && _G(gameState).R21Lever3) {
		_G(gameState).R21Laser1Weg = true;
		_G(det)->stopDetail(3);
		_G(atds)->setControlBit(134, ATS_ACTIVE_BIT);
		_G(atds)->delControlBit(133, ATS_ACTIVE_BIT);
	} else {
		_G(gameState).R21Laser1Weg = false;
		_G(det)->startDetail(3, 255, ANI_FRONT);
		_G(atds)->delControlBit(134, ATS_ACTIVE_BIT);
		_G(atds)->setControlBit(133, ATS_ACTIVE_BIT);
	}

	if (!_G(gameState).R21Lever1 && _G(gameState).R21Lever2 && !_G(gameState).R21Lever3) {
		if (!_G(obj)->checkInventory(SEIL_INV) && !_G(gameState).R17Rope) {
			_G(obj)->show_sib(SIB_SEIL_R21);
			_G(atds)->delControlBit(129, ATS_ACTIVE_BIT);
		}

		_G(gameState).R21Laser2Weg = true;
		_G(det)->stopDetail(4);
		_G(atds)->setControlBit(135, ATS_ACTIVE_BIT);

	} else {
		_G(obj)->hide_sib(SIB_SEIL_R21);
		_G(atds)->setControlBit(129, ATS_ACTIVE_BIT);
		_G(gameState).R21Laser2Weg = false;
		_G(det)->startDetail(4, 255, ANI_FRONT);
		_G(atds)->delControlBit(135, ATS_ACTIVE_BIT);
	}
}

void Room21::init_spinne() {
	_G(det)->load_taf_seq(42, 48, nullptr);
	_G(auto_obj) = 2;

	_G(mov_phasen)[SPINNE1_OBJ].AtsText = 130;
	_G(mov_phasen)[SPINNE1_OBJ].Lines = 3;
	_G(mov_phasen)[SPINNE1_OBJ].Repeat = 255;
	_G(mov_phasen)[SPINNE1_OBJ].ZoomFak = 0;
	_G(auto_mov_obj)[SPINNE1_OBJ].Id = AUTO_OBJ0;
	_G(auto_mov_vector)[SPINNE1_OBJ].Delay = _G(gameState).DelaySpeed;
	_G(auto_mov_obj)[SPINNE1_OBJ].Mode = true;
	init_auto_obj(SPINNE1_OBJ, &SPINNE_PHASEN[0][0], 3, (const MovLine *)SPINNE_MPKT);

	_G(mov_phasen)[SPINNE2_OBJ].AtsText = 130;
	_G(mov_phasen)[SPINNE2_OBJ].Lines = 2;
	_G(mov_phasen)[SPINNE2_OBJ].Repeat = 1;
	_G(mov_phasen)[SPINNE2_OBJ].ZoomFak = 0;
	_G(auto_mov_obj)[SPINNE2_OBJ].Id = AUTO_OBJ1;
	_G(auto_mov_vector)[SPINNE2_OBJ].Delay = _G(gameState).DelaySpeed;
	_G(auto_mov_obj)[SPINNE2_OBJ].Mode = true;
	init_auto_obj(SPINNE2_OBJ, &SPINNE_PHASEN[0][0], 2, (const MovLine *)SPINNE_MPKT1);
	_G(timer_nr)[0] = _G(room)->set_timer(255, 21);
	_G(e_streifen) = false;
}

void Room21::restart_spinne2() {
	_G(mov_phasen)[SPINNE2_OBJ].Repeat = 1;
	init_auto_obj(SPINNE2_OBJ, &SPINNE_PHASEN[0][0], _G(mov_phasen)[SPINNE2_OBJ].Lines, (const MovLine *)SPINNE_MPKT1);
	_G(uhr)->resetTimer(_G(timer_nr)[0], 0);
	_G(e_streifen) = false;
}

void Room21::setup_func() {
	if (_G(auto_mov_vector)[SPINNE2_OBJ].Xypos[1] >= 190 && !_G(e_streifen)) {
		_G(e_streifen) = true;

		_G(auto_obj) = 3;
		_G(mov_phasen)[ENERGIE_OBJ].AtsText = 0;
		_G(mov_phasen)[ENERGIE_OBJ].Lines = 2;
		_G(mov_phasen)[ENERGIE_OBJ].Repeat = 1;
		_G(mov_phasen)[ENERGIE_OBJ].ZoomFak = 0;
		_G(auto_mov_obj)[ENERGIE_OBJ].Id = AUTO_OBJ2;
		_G(auto_mov_vector)[ENERGIE_OBJ].Delay = _G(gameState).DelaySpeed;
		_G(auto_mov_obj)[ENERGIE_OBJ].Mode = true;
		init_auto_obj(ENERGIE_OBJ, &SPINNE_PHASEN[0][0], 2, (const MovLine *)SPINNE_MPKT2);
	}
}

void Room21::chewy_kolli() {
	int16 kolli = false;

	for (int16 i = 0; i < _G(auto_obj) && !kolli; i++) {
		int16 spr_nr = _G(mov_phasen)[i].Phase[_G(auto_mov_vector)[i].Phase][0] + _G(auto_mov_vector)[i].PhNr;
		int16 *xy = (int16 *)_G(room_blk)._detImage[spr_nr];
		int16 *Cxy = _G(room_blk).DetKorrekt + (spr_nr << 1);
		int16 xoff = xy[0];
		int16 yoff = xy[1];
		if (i == 2)
			xoff += 10;
		xoff += _G(auto_mov_vector)[i].Xzoom;
		yoff += _G(auto_mov_vector)[i].Yzoom;

		if ((!i && _G(moveState)[P_CHEWY].Xypos[0] < 516) ||
			(i == 1 && _G(moveState)[P_CHEWY].Xypos[1] > 70) ||
			(i == 2)) {
			if (_G(moveState)[P_CHEWY].Xypos[0] + 12 >= _G(auto_mov_vector)[i].Xypos[0] + Cxy[0] &&
				_G(moveState)[P_CHEWY].Xypos[0] + 12 <= _G(auto_mov_vector)[i].Xypos[0] + xoff + Cxy[0] &&
				_G(moveState)[P_CHEWY].Xypos[1] + 12 >= _G(auto_mov_vector)[i].Xypos[1] + Cxy[1] &&
				_G(moveState)[P_CHEWY].Xypos[1] + 12 <= _G(auto_mov_vector)[i].Xypos[1] + yoff + Cxy[1] &&
				_G(mov_phasen)[i].Start == 1)
				kolli = true;
		}
	}

	if (kolli && !_G(flags).AutoAniPlay) {
		const int16 tmp = _G(moveState)[P_CHEWY].Count;
		stopPerson(P_CHEWY);
		_G(flags).AutoAniPlay = true;
		_G(gameState)._personHide[P_CHEWY] = true;
		int16 ani_nr = (_G(moveState)[P_CHEWY].Xyvo[0] < 0) ? 10 : 11;
		_G(det)->setDetailPos(ani_nr, _G(moveState)[P_CHEWY].Xypos[0], _G(moveState)[P_CHEWY].Xypos[1]);
		startSetAILWait(ani_nr, 1, ANI_FRONT);
		_G(gameState)._personHide[P_CHEWY] = false;
		_G(flags).AutoAniPlay = false;
		_G(moveState)[P_CHEWY].Count = tmp;
		get_phase(&_G(moveState)[P_CHEWY], &_G(spieler_mi)[P_CHEWY]);
		_G(mov)->continue_auto_go();
	}
}

void Room21::salto() {
	if (!_G(cur)->usingInventoryCursor() && !_G(gameState).R21Salto && !_G(flags).AutoAniPlay) {
		_G(gameState).R21Salto = true;
		_G(flags).AutoAniPlay = true;
		_G(gameState)._personHide[P_CHEWY] = true;

		for (int16 i = 0; i < 3; i++) {
			_G(det)->setDetailPos(12 + i, _G(moveState)[P_CHEWY].Xypos[0],
				_G(moveState)[P_CHEWY].Xypos[1]);
		}

		startAniBlock(3, ABLOCK19);
		_G(gameState)._personHide[P_CHEWY] = false;
		startAadWait(36);
		_G(flags).AutoAniPlay = false;
	}
}

void Room21::use_gitter_energy() {
	_G(gameState).R21EnergyGrid = exit_flip_flop(-1, 47, -1, 131, 138, -1,
		EXIT_BOTTOM, EXIT_TOP, (int16)_G(gameState).R21EnergyGrid);
	_G(auto_obj) = 0;
	_G(gameState).R17Location = 1;
	_G(gameState)._personHide[P_CHEWY] = true;

	switchRoom(17);
	_G(det)->hideStaticSpr(5);
	startSetAILWait(9, 1, ANI_FRONT);
	_G(gameState).R17GridWeg = true;
	_G(gameState)._personHide[P_CHEWY] = false;
}

int16 Room21::use_fenster() {
	int16 action_flag = false;

	if (!_G(cur)->usingInventoryCursor() && !_G(flags).AutoAniPlay && _G(gameState).R21Laser1Weg) {
		action_flag = true;
		_G(flags).AutoAniPlay = true;
		_G(gameState).R18Grid = true;
		autoMove(13, P_CHEWY);
		setPersonPos(541, 66, P_CHEWY, P_LEFT);
		switchRoom(18);

		if (!_G(gameState).R18FirstEntry) {
			startAadWait(39);
			_G(gameState).R18FirstEntry = true;
		}

		_G(gameState).room_e_obj[50].Attribut = EXIT_TOP;
		_G(gameState).room_e_obj[41].Attribut = 255;
		_G(flags).AutoAniPlay = false;
	}

	return action_flag;
}

} // namespace Rooms
} // namespace Chewy
