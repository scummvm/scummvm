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

#include "chewy/defines.h"
#include "chewy/events.h"
#include "chewy/globals.h"
#include "chewy/ani_dat.h"
#include "chewy/room.h"
#include "chewy/rooms/room40.h"
#include "chewy/rooms/room43.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

#define SPEED 3
#define POLICE_LEFT 8
#define POLICE_OFFEN 9
#define POLICE_WUERG 10
#define POLICE_RIGHT 11
#define POLICE_FLASCHE 16

void Room40::entry(int16 eib_nr) {
	_G(gameState).R40HoUse = false;
	_G(zoom_horizont) = 130;
	_G(gameState).ScrollxStep = 2;

	if (_G(gameState).R40Geld) {
		_G(det)->del_static_ani(6);
		_G(room)->set_timer_status(6, TIMER_STOP);
	}

	if (_G(gameState).R40HaendlerOk) {
		_G(det)->del_static_ani(4);
		_G(room)->set_timer_status(4, TIMER_STOP);
	}

	if (_G(gameState).R40PoliceWeg == false) {
		_G(timer_nr)[0] = _G(room)->set_timer(255, 10);
		_G(atds)->delControlBit(275, ATS_ACTIVE_BIT);
	} else {
		_G(det)->hideStaticSpr(15);
	}

	_G(gameState).R40PoliceAniStatus = 255;
	_G(gameState).R40PoliceStart = false;
	_G(spieler_mi)[P_HOWARD].Mode = true;

	if (_G(gameState)._personRoomNr[P_HOWARD] == 41)
		_G(gameState)._personRoomNr[P_HOWARD] = 40;

	if (_G(gameState)._personRoomNr[P_HOWARD] == 40) {
		_G(gameState).ZoomXy[P_HOWARD][0] = 40;
		_G(gameState).ZoomXy[P_HOWARD][1] = 40;

		if (!_G(flags).LoadGame) {
			switch (eib_nr) {
			case 69:
				setPersonPos(30, 105, P_HOWARD, P_RIGHT);
				goAutoXy(158, 99, P_HOWARD, ANI_GO);
				break;

			case 73:
			case 74:
			case 87:
				setPersonPos(158, 93, P_HOWARD, P_LEFT);
				break;
			default:
				break;
			}
		}
	}

	_G(SetUpScreenFunc) = setup_func;
	if (_G(gameState).R40TrainMove)
		move_train(0);
}

void Room40::xit(int16 eib_nr) {
	hideCur();
	_G(gameState).ScrollxStep = 1;
	_G(gameState).R40PoliceAb = false;
	_G(gameState).R40HoUse = false;
	stop_spz();
	_G(SetUpScreenFunc) = nullptr;

	if (_G(gameState)._personRoomNr[P_HOWARD] == 40) {
		if (eib_nr == 70 || eib_nr == 77) {
			_G(gameState)._personRoomNr[P_HOWARD] = 28;

		} else if (eib_nr == 72) {
			if ((_G(obj)->checkInventory(HOTEL_INV) && _G(obj)->checkInventory(TICKET_INV) && _G(gameState).R42BriefOk && _G(gameState).R28Manuskript)
				|| _G(gameState).R40TrainOk) {
				_G(gameState).R40TrainOk = true;
				_G(gameState)._personRoomNr[P_HOWARD] = 45;
				_G(gameState).room_e_obj[72].Exit = 45;
				_G(obj)->hide_sib(SIB_MUENZE_R40);

				_G(uhr)->disableTimer();
				_G(out)->fadeOut();
				hide_person();
				setupScreen(DO_SETUP);
				_G(out)->fadeIn(_G(pal));
				_G(uhr)->enableTimer();

				_G(mouseLeftClick) = false;
				startAadWait(238);
				move_train(1);
				register_cutscene(15);
				
				_G(flags).NoPalAfterFlc = true;
				flic_cut(FCUT_073);

				if (_G(gameState).ChewyAni != CHEWY_ROCKER)
					_G(gameState).PersonGlobalDia[1] = 10023;

				cur_2_inventory();
				remove_inventory(57);
				_G(gameState).PersonDiaRoom[P_HOWARD] = true;
				show_person();

			} else {
				_G(gameState)._personRoomNr[P_HOWARD] = 42;
			}
		} else {
			_G(gameState)._personRoomNr[P_HOWARD] = 41;
		}
	}

	_G(spieler_mi)[P_HOWARD].Mode = false;
	showCur();
}

bool Room40::timer(int16 t_nr, int16 ani_nr) {
	if (t_nr == _G(timer_nr)[0])
		_G(gameState).R40PoliceStart = true;
	else
		return true;

	return false;
}

void Room40::move_train(int16 mode) {
	_G(gameState).R40TrainMove = false;
	hideCur();
	autoMove(9, P_CHEWY);
	_G(flags).NoScroll = true;
	auto_scroll(232, 0);

	if (!mode)
		startAadWait(206);

	int16 lx = -40;
	int16 ax = -230;

	_G(det)->startDetail(7, 20, ANI_FRONT);
	_G(det)->showStaticSpr(11);

	if (mode && _G(gameState).ChewyAni == CHEWY_PUMPKIN)
		_G(det)->showStaticSpr(12);

	g_engine->_sound->playSound(7, 0);
	int16 delay = 0;

	while (ax < 560) {
		_G(det)->setSetailPos(7, lx, 46);
		_G(det)->setStaticPos(11, ax, 62, false, false);

		if (mode && _G(gameState).ChewyAni == CHEWY_PUMPKIN)
			_G(det)->setStaticPos(12, ax, 62, false, true);

		if (!delay) {
			lx += SPEED;
			ax += SPEED;
			delay = _G(gameState).DelaySpeed / 2;
		} else {
			--delay;
		}

		setupScreen(DO_SETUP);
		SHOULD_QUIT_RETURN;
	}

	_G(det)->stop_detail(7);
	_G(det)->hideStaticSpr(11);
	_G(det)->hideStaticSpr(12);

	if (!mode) {
		startAadWait(207);
		auto_scroll(180, 0);
	}
	
	_G(flags).NoScroll = false;
	showCur();
}

void Room40::setup_func() {
	if (!_G(gameState).R40HoUse && _G(gameState)._personRoomNr[P_HOWARD] == 40) {
		calc_person_look();

		int16 x, y;
		const int16 sp_x = _G(moveState)[P_CHEWY].Xypos[0];
		if (sp_x > 170 && sp_x < 255) {
			x = 248;
			y = 97;
		} else if (sp_x > 255 && sp_x < 350) {
			x = 310;
			y = 90;
		} else if (sp_x > 350) {
			x = 428;
			y = 90;
		} else if (sp_x < 170) {
			x = 166;
			y = 99;
		} else {
			x = _G(moveState)[P_HOWARD].Xypos[0];
			y = _G(moveState)[P_HOWARD].Xypos[1];
		}

		goAutoXy(x, y, P_HOWARD, ANI_GO);
	}

	if (_G(gameState).R40PoliceWeg == false) {
		if (_G(gameState).R40PoliceStart) {
			_G(gameState).R40PoliceStart = false;
			_G(gameState).R40PoliceAniStatus = POLICE_LEFT;
			_G(room)->set_timer_status(255, TIMER_STOP);
			_G(uhr)->resetTimer(_G(timer_nr)[0], 0);
			_G(det)->hideStaticSpr(15);
			_G(det)->startDetail(POLICE_LEFT, 1, ANI_FRONT);
			_G(atds)->setControlBit(275, ATS_ACTIVE_BIT);
		}

		switch (_G(gameState).R40PoliceAniStatus) {
		case POLICE_LEFT:
			if (_G(det)->get_ani_status(POLICE_LEFT) == false) {
				_G(det)->startDetail(POLICE_OFFEN, 1, ANI_FRONT);
				_G(gameState).R40PoliceAniStatus = POLICE_OFFEN;
			}
			break;

		case POLICE_OFFEN:
			if (_G(det)->get_ani_status(POLICE_OFFEN) == false) {
				_G(det)->showStaticSpr(0);
				_G(det)->startDetail(POLICE_FLASCHE, 1, ANI_FRONT);
				_G(gameState).R40PoliceAniStatus = POLICE_FLASCHE;
			}
			break;

		case POLICE_RIGHT:
			if (_G(det)->get_ani_status(POLICE_RIGHT) == false) {
				_G(det)->showStaticSpr(15);
				_G(gameState).R40PoliceAniStatus = 255;
				_G(room)->set_timer_status(255, TIMER_START);
				_G(uhr)->resetTimer(_G(timer_nr)[0], 0);
				_G(atds)->delControlBit(275, ATS_ACTIVE_BIT);
			}
			break;

		case POLICE_FLASCHE:
			if (_G(det)->get_ani_status(POLICE_FLASCHE) == false) {
				_G(det)->hideStaticSpr(0);
				if (_G(gameState).R40DuengerTele) {
					hideCur();
					_G(gameState).R40PoliceWeg = true;
					_G(det)->startDetail(17, 255, ANI_FRONT);
					startAadWait(226);
					_G(det)->stop_detail(17);
					_G(gameState).R40HoUse = true;
					_G(person_end_phase)[P_CHEWY] = P_RIGHT;
					startSetAILWait(10, 1, ANI_FRONT);
					_G(person_end_phase)[P_HOWARD] = P_RIGHT;
					startAadWait(224);
					_G(gameState).R40PoliceWeg = true;
					showCur();

					_G(flags).MouseLeft = false;
					_G(flags).MainInput = true;
					_G(gameState).R40HoUse = false;
					_G(atds)->setControlBit(276, ATS_ACTIVE_BIT);

				} else {
					_G(det)->startDetail(POLICE_RIGHT, 1, ANI_FRONT);
					_G(gameState).R40PoliceAniStatus = POLICE_RIGHT;
				}
			}
			break;

		default:
			break;
		}
	}
}

int16 Room40::use_mr_pumpkin() {
	int16 action_ret = false;

	if (_G(menu_item) != CUR_HOWARD) {
		hideCur();

		if (!_G(gameState).inv_cur) {
			action_ret = use_schalter(205);

		} else {
			switch (_G(gameState).AkInvent) {
			case CENT_INV:
				action_ret = true;
				autoMove(5, P_CHEWY);
				delInventory(_G(gameState).AkInvent);
				startSetAILWait(15, 1, ANI_FRONT);
				start_spz(CH_PUMP_TALK, 255, ANI_FRONT, P_CHEWY);
				startAadWait(200);
				break;

			case RECORDER_INV:
				action_ret = true;

				if (_G(gameState).R39TvRecord == 6) {
					if (_G(gameState).R40PoliceWeg == false)
						use_schalter(227);
					else {
						hideCur();
						autoMove(8, P_CHEWY);
						start_spz_wait(CH_PUMP_GET1, 1, false, P_CHEWY);
						delInventory(_G(gameState).AkInvent);
						_G(out)->fadeOut();
						Room43::catch_pg();
						remove_inventory(LIKOER_INV);
						_G(obj)->addInventory(LIKOER2_INV, &_G(room_blk));
						inventory_2_cur(LIKOER2_INV);
						switchRoom(40);
						startAadWait(236);
					}
				} else {
					startAadWait(228 + _G(gameState).R39TvRecord);
				}
				break;

			default:
				break;
			}
		}

		showCur();
	}

	return action_ret;
}

int16 Room40::use_schalter(int16 aad_nr) {
	int16 action_flag = false;

	if (_G(menu_item) != CUR_HOWARD &&_G(gameState).R40PoliceWeg == false) {
		action_flag = true;

		hideCur();
		autoMove(8, P_CHEWY);

		if (_G(gameState).R40PoliceAniStatus != 255) {
			start_spz(CH_PUMP_TALK, 255, ANI_FRONT, P_CHEWY);
			startAadWait(204);

			while (_G(gameState).R40PoliceAniStatus != 255) {
				setupScreen(DO_SETUP);
				SHOULD_QUIT_RETURN0;
			}
		}

		_G(room)->set_timer_status(255, TIMER_STOP);
		_G(gameState).R40PoliceStart = false;
		stop_spz();
		start_spz_wait(CH_PUMP_GET1, 1, false, P_CHEWY);

		if (_G(gameState).R40PoliceAb) {
			_G(gameState).R40PoliceAb = false;
			stop_spz();
			goAutoXy(308, 100, P_HOWARD, ANI_WAIT);
			_G(gameState).R40HoUse = false;
		}

		_G(det)->hideStaticSpr(15);
		startSetAILWait(12, 1, ANI_FRONT);
		_G(det)->set_static_ani(14, -1);
		startAadWait(aad_nr);
		_G(det)->del_static_ani(14);
		startSetAILWait(13, 1, ANI_FRONT);
		_G(det)->showStaticSpr(15);
		_G(room)->set_timer_status(255, TIMER_START);
		_G(uhr)->resetTimer(_G(timer_nr)[0], 0);
		showCur();
	}

	return action_flag;
}

void Room40::talk_police() {
	if (!_G(gameState).R40PoliceWeg && _G(gameState).R40PoliceAniStatus == 255) {
		hideCur();
		_G(gameState).R40PoliceStart = false;
		_G(room)->set_timer_status(255, TIMER_STOP);
		autoMove(7, P_CHEWY);
		startAadWait(203);
		_G(room)->set_timer_status(255, TIMER_START);
		_G(uhr)->resetTimer(_G(timer_nr)[0], 0);
		showCur();
	}
}

void Room40::talk_handler() {
	if (!_G(gameState).R40HaendlerOk) {
		_G(gameState).flags38_2 = true;
		hideCur();
		autoMove(6, P_CHEWY);
		_G(det)->del_static_ani(4);
		_G(room)->set_timer_status(4, TIMER_STOP);
		_G(det)->set_static_ani(3, -1);
		startAadWait(202);
		_G(det)->set_static_ani(4, -1);
		_G(det)->del_static_ani(3);
		_G(room)->set_timer_status(4, TIMER_START);
		showCur();
	}
}

int16 Room40::use_haendler() {
	int16 action_flag = false;

	if (_G(menu_item) == CUR_HOWARD && !_G(gameState).R40HaendlerOk) {
		action_flag = true;
		if (!_G(gameState).flags38_2) {
			startAadWait(612);
		} else {
			hideCur();
			invent_2_slot(DUENGER_INV);
			_G(gameState).R40HoUse = true;
			_G(gameState).R40HaendlerOk = true;
			_G(gameState).R40TeilKarte = true;
			_G(gameState).R40DuengerMit = true;
			_G(atds)->setControlBit(283, ATS_ACTIVE_BIT);
			autoMove(10, P_CHEWY);
			autoMove(11, P_HOWARD);
			startAadWait(208);
			autoMove(6, P_HOWARD);
			_G(flags).NoScroll = true;

			auto_scroll(270, 0);
			_G(det)->del_static_ani(4);
			_G(room)->set_timer_status(4, TIMER_STOP);
			_G(det)->set_static_ani(3, -1);
			startAadWait(209);
			_G(det)->del_static_ani(3);
			_G(det)->set_static_ani(5, -1);
			startAadWait(213);

			if (_G(gameState).R28RKuerbis) {
				_G(det)->del_static_ani(5);
				_G(det)->set_static_ani(3, -1);
				startAadWait(211);
				_G(out)->fadeOut();
				_G(out)->set_partialpalette(_G(pal), 255, 1);
				startAadWait(212);
				_G(out)->fadeOut();
				_G(gameState).R40Wettbewerb = true;
				_G(gameState)._personRoomNr[P_HOWARD] = 28;
				_G(flags).NoScroll = false;
				_G(out)->setPointer(nullptr);
				_G(out)->cls();
				switchRoom(28);
				_G(SetUpScreenFunc) = setup_func;

			} else {
				autoMove(11, P_HOWARD);
				startAadWait(210);
			}

			_G(menu_item) = CUR_WALK;
			cursorChoice(_G(menu_item));
			showCur();
			_G(flags).NoScroll = false;
			_G(flags).MouseLeft = false;
			_G(gameState).R40HoUse = false;
		}
	}

	return action_flag;
}

int16 Room40::use_bmeister() {
	short action_flag = false;

	if (_G(menu_item) == CUR_HOWARD) {
		action_flag = true;
		hideCur();
		_G(gameState).R40HoUse = true;
		autoMove(9, P_CHEWY);
		autoMove(11, P_HOWARD);
		startAadWait(214);
		bmeister_dia(215);
		startAadWait(216);
		_G(gameState).R40HoUse = false;
		_G(flags).NoScroll = false;
		_G(menu_item) = CUR_WALK;
		cursorChoice(_G(menu_item));
		showCur();

	} else if (isCurInventory(LIKOER2_INV)) {
		action_flag = true;
		if (_G(gameState).flags37_80) {
			startAadWait(605);
		} else {
			hideCur();
			_G(gameState).R40HoUse = true;
			new_invent_2_cur(HOTEL_INV);
			bmeister_dia(237);
			_G(gameState).flags37_80 = true;
			_G(flags).NoScroll = false;
			showCur();
		}
	}

	return action_flag;
}

void Room40::bmeister_dia(int16 aad_nr) {
	autoMove(7, P_CHEWY);
	autoMove(12, P_HOWARD);
	_G(flags).NoScroll = true;
	auto_scroll(206, 0);
	_G(det)->del_static_ani(0);
	_G(room)->set_timer_status(0, TIMER_STOP);
	_G(det)->stop_detail(0);
	startSetAILWait(1, 1, ANI_FRONT);
	_G(det)->set_static_ani(2, -1);

	if (aad_nr == 237) {
		startAadWait(aad_nr);
		_G(flags).NoPalAfterFlc = true;
		flic_cut(FCUT_062);
		_G(fx_blend) = BLEND3;
		startAadWait(375);
	} else {
		startAadWait(aad_nr);
	}

	_G(flags).NoPalAfterFlc = false;
	_G(room)->set_timer_status(0, TIMER_START);
	_G(det)->set_static_ani(0, -1);
	_G(det)->del_static_ani(2);
	autoMove(11, P_HOWARD);
	autoMove(9, P_CHEWY);
}

bool Room40::use_police() {
	bool result = false;

	if (_G(menu_item) == CUR_HOWARD) {
		if (!_G(gameState).R40PoliceWeg && _G(gameState).R40PoliceAniStatus == 255) {
			result = true;
			_G(gameState).R40PoliceAb = true;
			hideCur();
			_G(gameState).R40PoliceStart = false;
			_G(room)->set_timer_status(255, TIMER_STOP);
			_G(gameState).R40HoUse = true;
			autoMove(9, P_CHEWY);
			autoMove(11, P_HOWARD);
			startAadWait(217);
			autoMove(8, P_CHEWY);
			goAutoXy(300, 120, P_HOWARD, ANI_WAIT);
			_G(person_end_phase)[P_HOWARD] = P_RIGHT;
			startAadWait(218);
			start_spz(HO_TALK_L, 255, ANI_FRONT, P_HOWARD);
			_G(menu_item) = CUR_WALK;
			cursorChoice(_G(menu_item));
			showCur();
			_G(flags).MouseLeft = false;
		}
	} else {
		startAadWait(225);
	}

	return result;
}

int16 Room40::use_tele() {
	int16 action_flag = false;

	if (!_G(gameState).inv_cur && _G(gameState).R40PoliceWeg == false) {
		action_flag = true;
		hideCur();

		int16 dia_nr1 = -1;
		if (!_G(gameState).R40PoliceAb) {
			startAadWait(219);
			_G(gameState).R40HoUse = false;

		} else {
			autoMove(13, P_CHEWY);
			_G(det)->showStaticSpr(0);

			int16 dia_nr;
			if (!_G(gameState).R40DuengerMit) {
				dia_nr = 220;
				dia_nr1 = 222;
			} else {
				dia_nr = 221;
				dia_nr1 = 223;
			}

			startAadWait(dia_nr);
			autoMove(11, P_HOWARD);
			_G(det)->hideStaticSpr(0);
			autoMove(9, P_CHEWY);
			startAadWait(dia_nr1);
			_G(gameState).R40HoUse = false;
			int16 timer_wert = 0;

			if (dia_nr1 == 223) {
				if (isCurInventory(DUENGER_INV)) {
					delInventory(_G(gameState).AkInvent);
				} else {
					remove_inventory(DUENGER_INV);
				}

				invent_2_slot(LIKOER_INV);
				autoMove(1, P_CHEWY);
				_G(gameState).R40DuengerMit = false;
				_G(gameState).R40DuengerTele = true;
				_G(flags).MouseLeft = true;
				_G(flags).MainInput = false;
				timer_wert = 3;
			}

			_G(gameState).R40PoliceAb = false;
			_G(room)->set_timer_status(255, TIMER_START);
			_G(uhr)->resetTimer(_G(timer_nr)[0], timer_wert);
		}

		if (dia_nr1 != 223)
			showCur();
	}

	return action_flag;
}

} // namespace Rooms
} // namespace Chewy
