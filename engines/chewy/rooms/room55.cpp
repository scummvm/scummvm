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
#include "chewy/rooms/room55.h"

namespace Chewy {
namespace Rooms {

void Room55::entry() {
	_G(gameState).ScrollxStep = 2;
	_G(SetUpScreenFunc) = setup_func;
	_G(zoom_horizont) = 140;
	_G(flags).ZoomMov = true;
	_G(zoom_mov_fak) = 3;
	_G(gameState).R55ExitDia = 0;
	_G(gameState).ZoomXy[P_HOWARD][0] = 20;
	_G(gameState).ZoomXy[P_HOWARD][1] = 22;

	if (!_G(gameState).R54FputzerWeg)
		_G(det)->startDetail(6, 255, ANI_FRONT);

	if (_G(gameState).R54FputzerWeg && !_G(gameState).R55Location)
		_G(det)->hideStaticSpr(6);

	if (_G(gameState).R55RaumOk || _G(gameState).R55ScriptWeg)
		_G(det)->hideStaticSpr(0);

	if (!_G(gameState).R55SekWeg) {
		_G(det)->startDetail(8, 255, ANI_FRONT);
		_G(det)->startDetail(9, 255, ANI_FRONT);
		_G(gameState).R55Entry ^= 1;
		const int aniNr = 19 + (_G(gameState).R55Entry ? 1 : 0);
		_G(timer_nr)[0] = _G(room)->set_timer(aniNr, 10);
		_G(det)->set_static_ani(aniNr, -1);
	}

	_G(spieler_mi)[P_HOWARD].Mode = true;

	if (_G(gameState).R55Job) {
		rock2mans();
		_G(gameState)._personRoomNr[P_HOWARD] = 54;
	}

	if (_G(gameState).ChewyAni == CHEWY_JMANS) {
		setPersonPos(51, 75, P_CHEWY, P_RIGHT);
		_G(room)->set_zoom(10);
	}

	if (_G(gameState).R55EscScriptOk && !_G(gameState).R55RaumOk)
		_G(det)->showStaticSpr(0);

	if (!_G(flags).LoadGame) {
		if (_G(gameState).R55Location) {
			_G(gameState).scrollx = 136;
			setPersonPos(404, 66, P_CHEWY, P_RIGHT);
			startSetAILWait(3, 1, ANI_FRONT);
			_G(gameState)._personHide[P_CHEWY] = false;
			_G(zoom_horizont) = 1;
			_G(atds)->delControlBit(340, ATS_ACTIVE_BIT);
		} else {
			_G(gameState).scrollx = 0;

			if (_G(gameState)._personRoomNr[P_HOWARD] == 55) {
				setPersonPos(21, 77, P_HOWARD, P_RIGHT);
			}

			_G(det)->showStaticSpr(8);
			autoMove(1, P_CHEWY);
			hideCur();
			_G(det)->hideStaticSpr(8);

			if (_G(gameState).R55SekWeg && !_G(gameState).R55Job) {
				setPersonSpr(P_LEFT, P_CHEWY);

				if (_G(gameState).ChewyAni == CHEWY_ROCKER) {
					verleger_mov(0);
					talk_line();
					_G(gameState).R54Schild = true;
				} else {
					verleger_mov(1);
					get_job();
				}
			}

			showCur();
		}
	}
}

void Room55::xit(int16 eib_nr) {
	if (eib_nr == 90) {
		mans2rock();

		if (_G(gameState)._personRoomNr[P_HOWARD] == 55) {
			_G(gameState)._personRoomNr[P_HOWARD] = 54;
			_G(spieler_mi)[P_HOWARD].Mode = false;
		}
	}

	_G(gameState).ScrollxStep = 1;
}

void Room55::gedAction(int index) {
	if (!index)
		talk_line();
}

int16 Room55::use_stapel1() {
	int16 action_ret = false;
	hideCur();
	
	if (!_G(cur)->usingInventoryCursor()) {
		if (!_G(gameState).R55ScriptWeg) {
			action_ret = true;
			_G(gameState).R55ScriptWeg = true;
			autoMove(4, P_CHEWY);
			_G(det)->hideStaticSpr(0);
			autoMove(5, P_CHEWY);
			_G(det)->showStaticSpr(1);
			_G(atds)->set_all_ats_str(354, 1, ATS_DATA);
			_G(atds)->set_all_ats_str(355, 1, ATS_DATA);

		} else if (_G(gameState).R55EscScriptOk && !_G(gameState).R55RaumOk) {
			action_ret = true;
			startAadWait(333);
		}
	} else if (isCurInventory(MANUSKRIPT_INV)) {
		action_ret = true;

		if (_G(gameState).R55ScriptWeg) {
			autoMove(4, P_CHEWY);
			_G(gameState).R55EscScriptOk = true;
			_G(det)->showStaticSpr(0);
			delInventory(_G(cur)->getInventoryCursor());
			_G(atds)->set_all_ats_str(354, 2, ATS_DATA);
		} else {
			startAadWait(326);
		}
	}
	showCur();

	return action_ret;
}

int16 Room55::use_stapel2() {
	int16 action_ret = false;

	if (isCurInventory(MANUSKRIPT_INV)) {
		action_ret = true;
		hideCur();
		startAadWait(327);
		showCur();
	}

	return action_ret;
}

int16 Room55::use_telefon() {
	int16 action_ret = false;

	if (!_G(cur)->usingInventoryCursor()) {
		action_ret = true;
		if (_G(gameState).R55EscScriptOk) {
			if (!_G(gameState).R55RaumOk) {
				hideCur();
				_G(gameState).R55RaumOk = true;
				autoMove(6, P_CHEWY);
				_G(gameState)._personHide[P_CHEWY] = true;
				startSetAILWait(10, 1, ANI_FRONT);
				_G(det)->startDetail(11, 255, ANI_FRONT);
				startAadWait(329);
				_G(det)->stopDetail(11);
				startSetAILWait(10, 1, ANI_BACK);

				_G(gameState)._personHide[P_CHEWY] = false;
				autoMove(7, P_CHEWY);
				_G(flags).NoScroll = true;
				auto_scroll(0, 0);
				startAadWait(330);
				_G(det)->showStaticSpr(8);
				startSetAILWait(0, 1, ANI_FRONT);
				_G(det)->startDetail(1, 255, ANI_FRONT);
				startAadWait(331);
				_G(det)->stopDetail(1);

				_G(det)->showStaticSpr(16);
				startAadWait(608);
				_G(det)->hideStaticSpr(16);
				startSetAILWait(2, 1, ANI_FRONT);
				_G(det)->hideStaticSpr(0);
				_G(det)->hideStaticSpr(8);
				_G(atds)->set_all_ats_str(354, 1, ATS_DATA);
				flic_cut(FCUT_071);
				_G(gameState)._personRoomNr[P_HOWARD] = 55;

				setPersonPos(178, 75, P_CHEWY, P_LEFT);
				setPersonPos(66, 73, P_HOWARD, P_RIGHT);
				_G(SetUpScreenFunc) = nullptr;
				_G(det)->set_static_ani(18, -1);
				startAadWait(334);
				startAadWait(335);
				startAadWait(336);
				const int16 tmp_delay = _G(gameState).DelaySpeed;

				for (int16 i = 0; i < 7; i++) {
					int16 delay = tmp_delay;
					if (!_G(gameState).scrollx)
						_G(gameState).scrollx = 8;
					else
						_G(gameState).scrollx = 0;
					while (delay) {
						--delay;
					}

					setupScreen(DO_SETUP);
					SHOULD_QUIT_RETURN0;
				}

				flic_cut(FCUT_072);
				register_cutscene(19);
				
				_G(flags).NoScroll = false;
				invent_2_slot(SACKGELD_INV);
				invent_2_slot(EINLAD_INV);
				del_invent_slot(LEDER_INV);
				load_chewy_taf(CHEWY_ROCKER);
				_G(gameState)._personRoomNr[P_HOWARD] = 54;
				_G(spieler_mi)[P_HOWARD].Mode = false;
				showCur();
				_G(gameState).R55R54First = true;
				_G(gameState).R55ExitDia = 337;
				_G(gameState).room_e_obj[89].Attribut = 255;
				_G(gameState).PersonGlobalDia[P_HOWARD] = 10024;
				_G(gameState).PersonDiaRoom[P_HOWARD] = true;

				switchRoom(54);
			} else {
				hideCur();
				startAadWait(332);
				showCur();
			}
		} else {
			hideCur();
			startAadWait(328);
			showCur();
		}
	}

	return action_ret;
}

void Room55::get_job() {
	_G(gameState)._personHide[P_CHEWY] = true;
	const int16 oldScrollx = _G(gameState).scrollx;
	const int16 oldScrolly = _G(gameState).scrolly;
	_G(gameState).scrollx = 0;
	_G(gameState).scrolly = 0;
	switchRoom(61);

	showCur();
	startDialogCloseupWait(15);
	_G(gameState)._personHide[P_CHEWY] = false;
	_G(flags).LoadGame = true;
	_G(gameState).scrollx = oldScrollx;
	_G(gameState).scrolly = oldScrolly;

	int16 r_nr;
	if (_G(gameState).R55Job) {
		r_nr = 55;
		_G(atds)->delControlBit(357, ATS_ACTIVE_BIT);
		_G(atds)->delControlBit(354, ATS_ACTIVE_BIT);
		_G(atds)->delControlBit(355, ATS_ACTIVE_BIT);
	} else {
		r_nr = 54;
		mans2rock();
	}

	_G(gameState)._personRoomNr[P_HOWARD] = r_nr;
	_G(spieler_mi)[P_HOWARD].Mode = true;

	if (r_nr == 54)
		_G(flags).LoadGame = false;

	setPersonPos(118, 96, P_CHEWY, P_LEFT);
	switchRoom(r_nr);
	_G(flags).LoadGame = false;
}

void Room55::mans2rock() {
	if (_G(gameState).ChewyAni == CHEWY_JMANS) {
		_G(gameState).R55ExitDia = 317;
		load_chewy_taf(CHEWY_ROCKER);
		remove_inventory(LEDER_INV);
		invent_2_slot(JMKOST_INV);
	}
}

void Room55::rock2mans() {
	if (_G(gameState).ChewyAni == CHEWY_ROCKER) {
		load_chewy_taf(CHEWY_JMANS);
		remove_inventory(JMKOST_INV);
		invent_2_slot(LEDER_INV);
	}
}

void Room55::verleger_mov(int16 mode) {
	startSetAILWait(7, 1, ANI_FRONT);
	_G(det)->showStaticSpr(11);
	startSetAILWait(13, 1, ANI_FRONT);
	startSetAILWait(14, 1, ANI_FRONT);

	if (_G(gameState).ChewyAni == CHEWY_JMANS)
		start_spz(CH_JM_TITS, 1, ANI_FRONT, P_CHEWY);
	
	if (mode)
		startSetAILWait(15, 1, ANI_FRONT);

	_G(det)->set_static_ani(16, -1);
}

void Room55::strasse(int16 mode) {
	if (!_G(cur)->usingInventoryCursor() || mode) {
		_G(gameState)._personHide[P_CHEWY] = true;
		_G(room)->set_timer_status(4, TIMER_STOP);
		_G(det)->del_static_ani(4);
		_G(det)->stopDetail(4);
		_G(atds)->setControlBit(340, ATS_ACTIVE_BIT);
		startSetAILWait(3, 1, ANI_BACK);

		_G(gameState).scrollx = 0;
		switchRoom(54);
	}
}

int16 Room55::use_kammeraus() {
	int16 action_ret = false;

	if (_G(gameState).R55Location && isCurInventory(KILLER_INV)) {
		action_ret = true;

		if (!_G(gameState).R52KakerWeg)
			startAadWait(325);
		else if (!_G(gameState).R55SekWeg) {
			hideCur();
			_G(gameState)._personHide[P_CHEWY] = true;
			startSetAILWait(5, 1, ANI_FRONT);
			_G(det)->showStaticSpr(10);
			flic_cut(FCUT_070);
			register_cutscene(18);
		
			const int aniNr = 19 + (_G(gameState).R55Entry ? 1 : 0);
			_G(room)->set_timer_status(aniNr, TIMER_STOP);
			_G(det)->stopDetail(aniNr);
			_G(det)->del_static_ani(aniNr);
			_G(det)->hideStaticSpr(10);
			_G(gameState).R55ExitDia = 322;
			_G(gameState).R55SekWeg = true;
			_G(atds)->setControlBit(352, ATS_ACTIVE_BIT);
			delInventory(_G(cur)->getInventoryCursor());
			_G(atds)->setControlBit(345, ATS_ACTIVE_BIT);
			_G(atds)->setControlBit(346, ATS_ACTIVE_BIT);
			strasse(1);
			showCur();
		}
	}

	return action_ret;
}

void Room55::setup_func() {
	if (_G(gameState)._personRoomNr[P_HOWARD] == 55) {
		calc_person_look();
		const int16 ch_x = _G(moveState)[P_CHEWY].Xypos[0];

		int16 x, y;
		if (ch_x < 100) {
			x = 62;
			y = 101;
		} else if (ch_x < 187) {
			x = 139;
			y = 119;
		} else if (ch_x < 276) {
			x = 235;
			y = 119;
		} else {
			x = 318;
			y = 110;
		}

		goAutoXy(x, y, P_HOWARD, ANI_GO);
	}
}

void Room55::talk_line() {
	if (_G(gameState).R55Location != 0 || _G(gameState).ChewyAni != CHEWY_ROCKER)
		return;
	
	hideCur();

	int16 aad_nr = 0;
	if (!_G(gameState).R55SekWeg) {
		autoMove(2, P_CHEWY);
		_G(flags).NoScroll = true;
		auto_scroll(136, 0);
		aad_nr = 320;
		_G(gameState).R55ExitDia = 321;
		const int aniNr = 19 + (_G(gameState).R55Entry ? 1 : 0);
		_G(room)->set_timer_status(aniNr, TIMER_STOP);
		_G(det)->stopDetail(aniNr);
		_G(det)->del_static_ani(aniNr);
		startSetAILWait(22, 1, ANI_FRONT);
		_G(det)->set_static_ani(21, -1);
	} else if (!_G(gameState).R55RaumOk) {
		autoMove(3, P_CHEWY);
		aad_nr = 323;
		_G(gameState).R55ExitDia = 324;
	}

	startAadWait(aad_nr);

	if (_G(gameState).R55SekWeg) {
		_G(det)->del_static_ani(16);
		startSetAILWait(14, 1, ANI_FRONT);
	} else {
		_G(det)->del_static_ani(21);
		startSetAILWait(22, 1, ANI_FRONT);
	}

	_G(flags).NoScroll = false;
	_G(gameState)._personRoomNr[P_HOWARD] = 54;
	_G(spieler_mi)[P_HOWARD].Mode = false;
	switchRoom(54);
	showCur();
}

} // namespace Rooms
} // namespace Chewy
