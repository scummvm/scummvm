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
#include "chewy/room.h"
#include "chewy/rooms/room45.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

void Room45::entry(int16 eib_nr) {
	_G(gameState).ScrollxStep = 2;
	_G(SetUpScreenFunc) = setup_func;
	_G(r45_delay) = 0;
	_G(zoom_horizont) = 150;
	_G(flags).ZoomMov = true;
	_G(zoom_mov_fak) = 4;
	_G(gameState).ZoomXy[P_HOWARD][0] = 80;
	_G(gameState).ZoomXy[P_HOWARD][1] = 70;

	if (_G(gameState)._personRoomNr[P_HOWARD] == 46) {
		_G(spieler_mi)[P_HOWARD].Mode = true;
		_G(gameState)._personRoomNr[P_HOWARD] = 45;
	}

	_G(spieler_mi)[P_HOWARD].Mode = true;
	if (!_G(flags).LoadGame) {
		int16 ch_x, ch_y;
		int16 ho_x, ho_y;
		if (_G(gameState).R48TaxiEntry) {
			ch_x = 67;
			ch_y = 146;
			ho_x = 43;
			ho_y = 129;

			_G(gameState).R48TaxiEntry = false;
			_G(mouseLeftClick) = false;
		} else {
			if (eib_nr == 72) {
				ch_x = 259;
				ch_y = 146;
				ho_x = 304;
				ho_y = 130;
				_G(gameState).scrollx = 130;
			} else {
				ch_x = 68;
				ch_y = 132;
				ho_x = 45;
				ho_y = 124;
			}
		}

		setPersonPos(ch_x, ch_y, P_CHEWY, P_LEFT);
		setPersonPos(ho_x, ho_y, P_HOWARD, P_LEFT);
	}
}

void Room45::xit(int16 eib_nr) {
	_G(gameState).ScrollxStep = 1;

	if (_G(gameState)._personRoomNr[P_HOWARD] == 45) {
		_G(spieler_mi)[P_HOWARD].Mode = false;

		if (eib_nr == 87) {
			_G(gameState)._personRoomNr[P_HOWARD] = 40;
		}
	}

	if (eib_nr == 87) {
		_G(flags).NoPalAfterFlc = true;
		flic_cut(FCUT_073);
	}
}

void Room45::setup_func() {
	int16 x;
	int16 y;

	if (_G(menu_display) == 0) {
		if (!_G(r45_delay)) {
			_G(r45_delay) = _G(gameState).DelaySpeed / 2;

			for (int16 i = 0; i < R45_MAX_PERSON; i++) {
				if (_G(r45_pinfo)[i][0] == 1) {
					y = _G(Adi)[3 + i].y;
					x = _G(Adi)[3 + i].x;

					if (i < 6) {
						x -= _G(r45_pinfo)[i][1];
						if (x < -30) {
							_G(r45_pinfo)[i][0] = 0;
							_G(det)->stopDetail(3 + i);
						}
					} else {
						x += _G(r45_pinfo)[i][1];
						if (x > 540) {
							_G(r45_pinfo)[i][0] = 0;
							_G(det)->stopDetail(3 + i);
						}
					}
					_G(det)->setDetailPos(3 + i, x, y);
				} else {
					++_G(r45_pinfo)[i][2];

					if (_G(r45_pinfo)[i][2] >= _G(r45_pinfo)[i][3]) {
						_G(r45_pinfo)[i][2] = 0;
						_G(r45_pinfo)[i][0] = 1;
						y = _G(Adi)[3 + i].y;
						if (i < 6) {
							x = 500;
						} else {
							x = 0;
						}
						_G(det)->setDetailPos(3 + i, x, y);
						_G(det)->startDetail(3 + i, 255, ANI_FRONT);
					}
				}
			}
		} else {
			--_G(r45_delay);
		}

		if (_G(gameState)._personRoomNr[P_HOWARD] == 45 && _G(HowardMov) != 2) {
			calc_person_look();
			const int16 ch_x = _G(moveState)[P_CHEWY].Xypos[0];

			if (ch_x < 95) {
				x = 18;
			} else if (ch_x > 240) {
				x = 304;
			} else {
				x = 176;
			}

			y = 130;

			if (_G(HowardMov) && _G(flags).ExitMov) {
				x = 56;
				y = 122;
			}

			goAutoXy(x, y, P_HOWARD, ANI_GO);
		}
	}
}

int16 Room45::use_taxi() {
	int16 action_ret = false;
	hideCur();
	autoMove(1, P_CHEWY);

	if (!_G(cur)->usingInventoryCursor()) {
		if (_G(gameState).ChewyAni == CHEWY_PUMPKIN) {
			action_ret = true;
			talk_taxi(254);
		} else if (!_G(gameState).R45TaxiOk) {
			action_ret = true;
			talk_taxi(260);
		} else if (_G(gameState).R45TaxiOk) {
			action_ret = true;
			taxi_mov();
		}
	} else if (isCurInventory(RING_INV)) {
		action_ret = true;
		delInventory(RING_INV);
		talk_taxi(256);
		_G(gameState).R45TaxiOk = true;
		taxi_mov();
	} else if (isCurInventory(UHR_INV)) {
		action_ret = true;
		talk_taxi(400);
	}

	showCur();
	return action_ret;
}

void Room45::talk_taxi(int16 aad_nr) {
	_G(room)->set_timer_status(12, TIMER_STOP);
	_G(det)->del_static_ani(12);
	startSetAILWait(13, 1, ANI_FRONT);
	_G(det)->set_static_ani(14, -1);
	startAadWait(aad_nr);
	_G(det)->del_static_ani(14);
	startSetAILWait(13, 1, ANI_BACK);
	_G(det)->set_static_ani(12, -1);
	_G(room)->set_timer_status(12, TIMER_START);
}

void Room45::taxi_mov() {
	_G(HowardMov) = 2;
	_G(room)->set_timer_status(12, TIMER_STOP);
	_G(det)->del_static_ani(12);
	_G(det)->playSound(15, 1);
	_G(det)->showStaticSpr(11);
	autoMove(3, P_CHEWY);
	_G(gameState)._personHide[P_CHEWY] = true;
	_G(gameState).R48TaxiPerson[P_CHEWY] = true;

	if (_G(gameState)._personRoomNr[P_HOWARD] == 45) {
		goAutoXy(93, 127, P_HOWARD, ANI_WAIT);
		_G(gameState)._personHide[P_HOWARD] = true;
		_G(gameState).R48TaxiPerson[P_HOWARD] = true;
		_G(gameState)._personRoomNr[P_HOWARD] = 48;
	}

	_G(det)->hideStaticSpr(11);
	_G(det)->playSound(15, 2);
	_G(det)->playSound(15, 0);
	startSetAILWait(15, 1, ANI_FRONT);
	_G(det)->stopSound(0);
	switchRoom(48);
}

int16 Room45::use_boy() {
	int16 action_ret = false;
	hideCur();
	autoMove(2, P_CHEWY);

	if (!_G(gameState).R45MagOk) {
		if (isCurInventory(DOLLAR175_INV)) {
			action_ret = true;
			new_invent_2_cur(CUTMAG_INV);
			_G(atds)->set_all_ats_str(DOLLAR175_INV, 1, INV_ATS_DATA);
			startAadWait(258);
			_G(room)->set_timer_status(0, TIMER_STOP);
			_G(det)->del_static_ani(0);
			startSetAILWait(1, 1, ANI_FRONT);
			_G(room)->set_timer_status(0, TIMER_START);
			_G(det)->set_static_ani(0, -1);
			_G(gameState).R45MagOk = true;
		}
	} else {
		startAadWait(259);
	}

	showCur();
	return action_ret;
}

void Room45::talk_boy() {
	int16 aad_nr;
	hideCur();

	if (!_G(gameState).R45MagOk) {
		autoMove(2, P_CHEWY);
		aad_nr = 257;
	} else {
		aad_nr = 259;
	}

	startAadWait(aad_nr);
	showCur();
}

} // namespace Rooms
} // namespace Chewy
