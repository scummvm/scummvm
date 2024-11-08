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
#include "chewy/rooms/room54.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

void Room54::entry(int16 eib_nr) {
	_G(gameState).ScrollxStep = 2;
	_G(SetUpScreenFunc) = setup_func;
	_G(zoom_horizont) = 106;
	_G(flags).ZoomMov = true;
	_G(zoom_mov_fak) = 3;
	_G(gameState).ZoomXy[P_HOWARD][0] = 30;
	_G(gameState).ZoomXy[P_HOWARD][1] = 66;

	if (_G(gameState).R54FputzerWeg)
		_G(det)->showStaticSpr(9);

	_G(spieler_mi)[P_HOWARD].Mode = true;

	if (!_G(flags).LoadGame) {
		if (_G(gameState).R48TaxiEntry) {
			_G(gameState).R48TaxiEntry = false;

			if (_G(gameState)._personRoomNr[P_HOWARD] == 54) {
				_G(gameState).R54HowardVorne = 0;
				setPersonPos(290, 61, P_HOWARD, P_RIGHT);
			}

			setPersonPos(300, 80, P_CHEWY, P_LEFT);
			_G(mouseLeftClick) = false;
			_G(gameState).scrollx = 134;
			_G(gameState).scrolly = 0;

		} else if (_G(gameState).R55Location) {
			aufzug_ab();

		} else if (eib_nr == 90 || _G(gameState).R55ExitDia) {
			if (_G(gameState)._personRoomNr[P_HOWARD] == 54) {
				_G(gameState).R54HowardVorne = 0;
				setPersonPos(212, 61, P_HOWARD, P_RIGHT);
			}

			_G(gameState).scrollx = 66;
			setPersonPos(241, 85, P_CHEWY, P_LEFT);
		}

		if (_G(gameState).R55ExitDia) {
			startAadWait(_G(gameState).R55ExitDia);
			showCur();
			_G(gameState).R55ExitDia = false;
		}

		if (_G(gameState).R55R54First) {
			_G(gameState).R55R54First = false;
		}
	}
}

void Room54::xit(int16 eib_nr) {
	_G(gameState).ScrollxStep = 1;

	if (eib_nr == 89 && _G(gameState)._personRoomNr[P_HOWARD] == 54) {
		_G(gameState)._personRoomNr[P_HOWARD] = 55;
		_G(spieler_mi)[P_HOWARD].Mode = false;
	}
}

void Room54::setup_func() {
	if (_G(gameState)._personRoomNr[P_HOWARD] == 54) {
		calc_person_look();

		if (_G(gameState).R54HowardVorne != 255) {
			const int16 ch_x = _G(moveState)[P_CHEWY].Xypos[0];
			int16 x, y;
			if (_G(gameState).R54HowardVorne) {
				if (ch_x < 218) {
					x = 150;
					y = 113;
				} else {
					x = 287;
					y = 115;
				}
			} else {
				y = 61;
				if (ch_x < 167) {
					x = 122;
				} else if (ch_x < 251) {
					x = 212;
				} else {
					x = 290;
				}
			}

			goAutoXy(x, y, P_HOWARD, ANI_GO);
		}
	}
}

int16 Room54::use_schalter() {
	int16 action_ret = false;

	if (!_G(cur)->usingInventoryCursor()) {
		hideCur();
		action_ret = true;

		if (!_G(gameState).R54FputzerWeg) {
			autoMove(1, P_CHEWY);
			_G(gameState).R54HowardVorne = 255;
			goAutoXy(127, 76, P_HOWARD, ANI_WAIT);
			goAutoXy(14, 72, P_HOWARD, ANI_GO);
			start_spz_wait(CH_ROCK_GET2, 1, false, P_CHEWY);
			_G(det)->showStaticSpr(0);
			autoMove(2, P_CHEWY);
			_G(det)->playSound(1, 0);
			_G(det)->playSound(0, 1);
			_G(det)->stopSound(2);

			startSetAILWait(1, 1, ANI_FRONT);
			_G(det)->startDetail(3, 255, ANI_FRONT);
			startAadWait(292 + _G(gameState).R54LiftCount);
			_G(det)->stopDetail(3);
			++_G(gameState).R54LiftCount;

			int16 aad_nr;
			if (_G(gameState).R54LiftCount < 3) {
				startSetAILWait(2, 1, ANI_FRONT);
				_G(det)->hideStaticSpr(0);
				_G(det)->stopSound(0);
				_G(det)->stopSound(1);
				_G(det)->playSound(1, 2);
				startSetAILWait(1, 1, ANI_BACK);
				_G(det)->stopSound(2);
				aad_nr = 295;

			} else {
				startSetAILWait(5, 1, ANI_FRONT);
				_G(det)->showStaticSpr(9);
				startSetAILWait(4, 1, ANI_FRONT);
				aad_nr = 296;
				_G(gameState).R54FputzerWeg = true;
				_G(atds)->delControlBit(345, ATS_ACTIVE_BIT);
				_G(atds)->set_all_ats_str(349, 1, ATS_DATA);
				_G(atds)->set_all_ats_str(351, 1, ATS_DATA);
			}

			startAadWait(aad_nr);
			_G(gameState).R54HowardVorne = 0;

		} else {
			startAadWait(297);
		}

		showCur();
	}

	return action_ret;
}

void Room54::talk_verkauf() {
	hideCur();

	if (!_G(gameState).R54HotDogOk) {
		if (_G(gameState).R45MagOk) {
			if (_G(cur)->getInventoryCursor() == DOLLAR175_INV)
				delInventory(DOLLAR175_INV);
			else
				remove_inventory(DOLLAR175_INV);
		}

		_G(gameState).R54HotDogOk = true;
		autoMove(3, P_CHEWY);
		_G(gameState).R54HowardVorne = 1;
		startAadWait(299);
		_G(room)->set_timer_status(6, TIMER_STOP);
		_G(det)->del_static_ani(6);
		startSetAILWait(7, 1, ANI_FRONT);
		_G(det)->startDetail(8, 255, ANI_FRONT);
		startAadWait(310);

		_G(det)->stopDetail(8);
		startSetAILWait(9, 1, ANI_FRONT);
		startSetAILWait(10, 1, ANI_FRONT);
		_G(det)->startDetail(11, 255, ANI_FRONT);
		startAadWait(311);
		_G(det)->stopDetail(11);
		_G(room)->set_timer_status(6, TIMER_START);
		_G(det)->set_static_ani(6, -1);
		autoMove(4, P_CHEWY);

		start_aad(_G(gameState).R45MagOk ? 312 : 578, -1);
		_G(obj)->addInventory(BURGER_INV, &_G(room_blk));
		inventory_2_cur(BURGER_INV);
	} else {
		startAadWait(313);
	}

	showCur();
}

int16 Room54::use_zelle() {
	int16 action_ret = false;
	hideCur();

	if (_G(cur)->usingInventoryCursor()) {
		if (isCurInventory(JMKOST_INV)) {
			action_ret = true;

			if (!_G(gameState).R54Schild) {
				startAadWait(318);
			} else {
				autoMove(5, P_CHEWY);
				_G(gameState).R54HowardVorne = 1;
				_G(SetUpScreenFunc) = nullptr;
				auto_scroll(176, 0);
				goAutoXy(239, 101, P_HOWARD, ANI_WAIT);
				flic_cut(FCUT_069);

				delInventory(_G(cur)->getInventoryCursor());
				invent_2_slot(LEDER_INV);
				load_chewy_taf(CHEWY_JMANS);
				_G(zoom_horizont) = 90;

				setPersonPos(283, 93, P_CHEWY, P_LEFT);
				setPersonPos(238, 99, P_HOWARD, P_RIGHT);
				startAadWait(315);
				goAutoXy(241, 74, P_CHEWY, ANI_WAIT);
				startAadWait(316);

				_G(spieler_mi)[P_HOWARD].Mode = true;
				switchRoom(55);
			}
		}
	} else {
		action_ret = true;
		autoMove(6, P_CHEWY);
		_G(gameState).R54HowardVorne = 1;
		startAadWait(319);
	}

	showCur();
	return action_ret;
}

int16 Room54::use_azug() {
	int16 action_ret = false;

	if (!_G(cur)->usingInventoryCursor()) {
		action_ret = true;
		hideCur();

		if (!_G(gameState).R54LiftOk) {
			if (_G(gameState).R54FputzerWeg) {
				autoMove(8, P_CHEWY);
				_G(gameState).R54HowardVorne = 0;
				startAadWait(298);
				_G(gameState)._personHide[P_CHEWY] = true;
				_G(det)->showStaticSpr(12);
				_G(gameState).R55Location = true;
				_G(SetUpScreenFunc) = nullptr;
				goAutoXy(91, 62, P_HOWARD, ANI_WAIT);
				_G(det)->playSound(1, 0);

				int16 ch_y = 68;
				int16 ay = 0;
				int16 delay = 0;

				while (ch_y > -48) {
					_G(det)->setStaticPos(12, 125, ch_y, false, false);
					_G(det)->setStaticPos(9, 122, ay, false, false);

					if (!delay) {
						ch_y -= 3;
						ay -= 3;
						delay = _G(gameState).DelaySpeed / 2;
					} else {
						--delay;
					}

					setupScreen(DO_SETUP);
					SHOULD_QUIT_RETURN0;
				}

				switchRoom(55);
			}
		} else {
			startAadWait(314);
		}

		showCur();
	}

	return action_ret;
}

void Room54::aufzug_ab() {
	setPersonPos(91, 62, P_HOWARD, P_RIGHT);
	setPersonPos(99, 82, P_CHEWY, P_RIGHT);
	_G(gameState).scrollx = 0;
	_G(SetUpScreenFunc) = setup_func;
	_G(det)->showStaticSpr(12);
	_G(det)->playSound(1, 0);

	int16 ch_y = -40;
	int16 ay = -108;
	int16 delay = 0;

	while (ch_y < 68) {
		_G(det)->setStaticPos(12, 125, ch_y, false, false);
		_G(det)->setStaticPos(9, 122, ay, false, false);

		if (!delay) {
			ch_y += 3;
			ay += 3;
			delay = _G(gameState).DelaySpeed / 2;
		} else {
			--delay;
		}

		setupScreen(DO_SETUP);
		SHOULD_QUIT_RETURN;
	}

	_G(det)->stopSound(0);
	_G(det)->hideStaticSpr(12);
	setPersonPos(99, 82, P_CHEWY, P_RIGHT);
	_G(gameState)._personHide[P_CHEWY] = false;
	_G(mouseLeftClick) = false;
	stopPerson(P_CHEWY);
	_G(gameState).R55Location = false;
}

short Room54::use_taxi() {
	int16 action_ret = false;

	if (!_G(cur)->usingInventoryCursor()) {
		action_ret = true;
		hideCur();
		autoMove(7, P_CHEWY);
		_G(gameState).R48TaxiPerson[P_CHEWY] = true;

		if (_G(gameState)._personRoomNr[P_HOWARD] == 54) {
			if (_G(gameState).R54HowardVorne) {
				_G(gameState).R54HowardVorne = 0;
				goAutoXy(290, 61, P_HOWARD, ANI_WAIT);
			}

			_G(gameState)._personHide[P_HOWARD] = true;
			_G(gameState).R48TaxiPerson[P_HOWARD] = true;
			_G(gameState)._personRoomNr[P_HOWARD] = 48;
		}

		_G(gameState)._personHide[P_CHEWY] = true;
		switchRoom(48);
	}

	return action_ret;
}

} // namespace Rooms
} // namespace Chewy
