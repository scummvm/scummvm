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
#include "chewy/rooms/room17.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

#undef CHEWY_OBJ
#define CHEWY_OBJ 0

static const int16 CHEWY_PHASEN[4][2] = {
	{ 0, 0 },
	{ 0, 0 },
	{ 141, 144 },
	{ 177, 177 }
};

static const MovLine CHEWY_MPKT[2] = {
	{ { 241, 150, 0 }, 3, 8 },
	{ { 241, 350, 0 }, 3, 8 }
};

static const MovLine CHEWY_MPKT1[2] = {
	{ { 243, 334, 0 }, 2, 6 },
	{ { 243, 150, 0 }, 2, 6 }
};

void Room17::entry() {
	if (!_G(gameState).R17EnergyOut) {
		_G(det)->startDetail(1, 255, ANI_FRONT);
		for (int i = 0; i < 3; ++i)
			_G(det)->startDetail(6 + i, 255, ANI_FRONT);
	}

	plot_seil();

	if (_G(gameState).R17GridWeg)
		_G(det)->hideStaticSpr(5);

	if (_G(gameState).R17DoorCommand)
		_G(det)->showStaticSpr(7);

	if (_G(gameState).R17Location == 1) {
		_G(flags).ZoomMov = true;
		_G(zoom_mov_fak) = 3;
		_G(room)->set_zoom(25);
		_G(zoom_horizont) = 0;
		_G(gameState).scrollx = 0;
		_G(gameState).scrolly = 60;
		setPersonPos(242, 146, P_CHEWY, P_LEFT);
		xit();
	} else if (_G(gameState).R17Location == 3) {
		_G(room)->set_zoom(32);
		_G(zoom_horizont) = 399;
		xit();
	} else {
		_G(room)->set_zoom(15);
		_G(zoom_horizont) = 0;
		_G(gameState).room_e_obj[36].Attribut = 255;
		_G(gameState).room_e_obj[38].Attribut = 255;

		if (_G(gameState).R6DoorLeftF)
			_G(gameState).room_e_obj[39].Attribut = EXIT_RIGHT;
		else
			_G(gameState).room_e_obj[39].Attribut = 255;

		if (_G(gameState).R18DoorBridge)
			_G(gameState).room_e_obj[35].Attribut = EXIT_LEFT;
		else
			_G(gameState).room_e_obj[35].Attribut = 255;
	}
}

void Room17::xit() {
	if (_G(gameState).R17DoorCommand)
		_G(gameState).room_e_obj[36].Attribut = EXIT_TOP;
	else
		_G(gameState).room_e_obj[36].Attribut = 255;

	_G(gameState).room_e_obj[35].Attribut = 255;
	_G(gameState).room_e_obj[39].Attribut = 255;
	_G(gameState).room_e_obj[38].Attribut = EXIT_TOP;
}

bool Room17::timer(int16 t_nr, int16 ani_nr) {
	if (_G(room)->_roomTimer._objNr[ani_nr] == 2 ||
		_G(room)->_roomTimer._objNr[ani_nr] == 3) {
		if (_G(gameState).R17EnergyOut)
			_G(uhr)->resetTimer(t_nr, 0);
		else
			return true;
	}

	return false;
}

void Room17::gedAction(int index) {
	switch (index) {
	case 0:
		door_kommando(0);
		break;

	case 1:
		door_kommando(1);
		break;

	default:
		break;
	}
}

int16 Room17::use_seil() {
	int16 action_flag = false;

	if (!_G(flags).AutoAniPlay && _G(gameState).R17Location == 1 && isCurInventory(SEIL_INV)) {
		action_flag = true;
		hideCur();

		delInventory(_G(cur)->getInventoryCursor());
		_G(flags).AutoAniPlay = true;
		autoMove(5, P_CHEWY);
		_G(gameState)._personHide[P_CHEWY] = true;
		startSetAILWait(10, 1, ANI_FRONT);
		_G(gameState).R17Rope = true;
		_G(atds)->delControlBit(139, ATS_ACTIVE_BIT);
		plot_seil();
		_G(gameState)._personHide[P_CHEWY] = false;
		_G(flags).AutoAniPlay = false;
		start_spz(CH_TALK6, 255, false, P_CHEWY);
		startAadWait(119);

		showCur();
	}

	return action_flag;
}

void Room17::plot_seil() {
	if (_G(gameState).R17Rope) {
		for (int16 i = 0; i < 3; i++)
			_G(det)->showStaticSpr(8 + i);
	}
}

void Room17::kletter_down() {
	autoMove(5, P_CHEWY);
	_G(det)->load_taf_seq(177, 1, nullptr);
	_G(gameState)._personHide[P_CHEWY] = true;
	startSetAILWait(14, 1, ANI_FRONT);
	_G(flags).ZoomMov = false;
	_G(zoom_mov_fak) = 1;
	_G(gameState).ScrollyStep = 2;
	_G(room)->set_zoom(32);
	_G(spieler_mi)->Vorschub = 8;
	_G(zoom_horizont) = 399;
	_G(auto_obj) = 1;
	init_auto_obj(CHEWY_OBJ, &CHEWY_PHASEN[0][0], _G(mov_phasen)[CHEWY_OBJ].Lines, (const MovLine *)CHEWY_MPKT);
	setPersonPos(242, 350, P_CHEWY, P_LEFT);
}

void Room17::kletter_up() {
	autoMove(6, P_CHEWY);
	_G(det)->load_taf_seq(141, 4, nullptr);
	_G(gameState)._personHide[P_CHEWY] = true;
	startSetAILWait(11, 1, ANI_FRONT);
	_G(flags).ZoomMov = true;
	_G(zoom_mov_fak) = 3;
	_G(gameState).ScrollyStep = 1;
	_G(room)->set_zoom(25);
	_G(zoom_horizont) = 0;
	_G(auto_obj) = 1;
	init_auto_obj(CHEWY_OBJ, &CHEWY_PHASEN[0][0], _G(mov_phasen)[CHEWY_OBJ].Lines, (const MovLine *)CHEWY_MPKT1);
	setPersonPos(243, 146, P_CHEWY, P_LEFT);
}

void Room17::calc_seil() {
	if (_G(gameState).R17Rope) {
		if (_G(gameState).R17Location == 2) {
			startAadWait(619);
		} else if (!_G(flags).AutoAniPlay && !_G(cur)->usingInventoryCursor()) {
			close_door();
			_G(flags).AutoAniPlay = true;
			_G(mov_phasen)[CHEWY_OBJ].AtsText = 0;
			_G(mov_phasen)[CHEWY_OBJ].Lines = 2;
			_G(mov_phasen)[CHEWY_OBJ].Repeat = 1;
			_G(mov_phasen)[CHEWY_OBJ].ZoomFak = 0;
			_G(auto_mov_obj)[CHEWY_OBJ].Id = AUTO_OBJ0;
			_G(auto_mov_vector)[CHEWY_OBJ].Delay = _G(gameState).DelaySpeed;
			_G(auto_mov_obj)[CHEWY_OBJ].Mode = true;
			hideCur();

			if (_G(gameState).R17Location == 1) {
				kletter_down();
				_G(gameState).R17Location = 3;
			} else if (_G(gameState).R17Location == 3) {
				kletter_up();
				_G(gameState).R17Location = 1;
			}

			_G(menu_item) = CUR_WALK;
			cursorChoice(_G(menu_item));
			wait_auto_obj(CHEWY_OBJ);
			showCur();
			setPersonSpr(P_LEFT, P_CHEWY);
			_G(gameState).ScrollyStep = 1;
			_G(gameState)._personHide[P_CHEWY] = false;
			_G(flags).AutoAniPlay = false;
			_G(auto_obj) = 0;
			xit();
		}
	}
}

void Room17::door_kommando(int16 mode) {
	if (!_G(flags).AutoAniPlay) {
		_G(flags).AutoAniPlay = true;

		if (!mode) {
			if (!_G(gameState).R17DoorCommand) {
				_G(gameState).room_e_obj[36].Attribut = EXIT_TOP;
				_G(gameState).R17DoorCommand = true;
				startSetAILWait(4, 1, ANI_FRONT);
				stopPerson(P_CHEWY);
				_G(det)->showStaticSpr(7);
			}
		} else {
			close_door();
		}

		_G(flags).AutoAniPlay = false;
		_G(atds)->set_all_ats_str(144, _G(gameState).R17DoorCommand, ATS_DATA);
	}
}

void Room17::close_door() {
	if (_G(gameState).R17DoorCommand) {
		_G(gameState).room_e_obj[36].Attribut = 255;
		_G(gameState).R17DoorCommand = false;
		_G(atds)->set_all_ats_str(144, _G(gameState).R17DoorCommand ? 1 : 0, ATS_DATA);
		_G(det)->hideStaticSpr(7);
		_G(det)->startDetail(4, 1, ANI_BACK);
	}
}

int16 Room17::energy_lever() {
	int16 action_flag = false;

	hideCur();
	autoMove(7, P_CHEWY);

	if (!_G(gameState).R17LeverOk) {
		if (isCurInventory(BECHER_VOLL_INV)) {
			delInventory(_G(cur)->getInventoryCursor());
			_G(gameState).R17LeverOk = true;
			startAadWait(38);
			action_flag = true;
		} else if (!_G(cur)->usingInventoryCursor()) {
			startAadWait(37);
			action_flag = true;
		}

	} else if (!_G(cur)->usingInventoryCursor()) {
		action_flag = true;

		_G(obj)->calc_rsi_flip_flop(SIB_HEBEL_R17);
		_G(gameState).R17EnergyOut ^= 1;

		if (!_G(gameState).R17EnergyOut) {
			_G(det)->startDetail(1, 255, ANI_FRONT);

			for (int i = 0; i < 3; ++i)
				_G(det)->startDetail(i + 6, 255, ANI_FRONT);
		}

		_G(atds)->set_all_ats_str(142, _G(gameState).R17EnergyOut ? 1 : 0, ATS_DATA);
		_G(atds)->set_all_ats_str(140, _G(gameState).R17EnergyOut ? 1 : 0, ATS_DATA);
		_G(det)->playSound(12, 0);

		if (_G(gameState).R17EnergyOut) {
			_G(det)->stopSound(0);
		} else {
			_G(det)->playSound(15, 0);
		}
	}

	showCur();
	return action_flag;
}

int16 Room17::get_oel() {
	int16 action_flag = false;
	hideCur();

	if (!_G(cur)->usingInventoryCursor()) {
		action_flag = true;
		autoMove(4, P_CHEWY);
		start_spz_wait(CH_EKEL, 3, false, P_CHEWY);
		startAadWait(60);
	} else if (isCurInventory(BECHER_LEER_INV)) {
		action_flag = true;
		close_door();
		autoMove(4, P_CHEWY);
		_G(gameState)._personHide[P_CHEWY] = true;
		startSetAILWait(13, 1, ANI_FRONT);
		_G(gameState)._personHide[P_CHEWY] = false;
		delInventory(_G(cur)->getInventoryCursor());
		_G(obj)->addInventory(BECHER_VOLL_INV, &_G(room_blk));
		inventory_2_cur(BECHER_VOLL_INV);
	}

	showCur();
	return action_flag;
}

} // namespace Rooms
} // namespace Chewy
