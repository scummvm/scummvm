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
#include "chewy/room.h"
#include "chewy/rooms/room56.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

void Room56::entry() {
	_G(gameState).ScrollxStep = 2;
	_G(flags).ZoomMov = true;
	_G(zoom_mov_fak) = 4;
	_G(spieler_mi)[P_HOWARD].Mode = true;
	int mode = 0;

	if (_G(gameState).flags32_10) {
		_G(det)->showStaticSpr(10);
		_G(room)->set_timer_status(0, TIMER_STOP);
		_G(det)->del_static_ani(0);
		_G(det)->startDetail(13, 255, ANI_FRONT);
	} else
		_G(timer_nr)[0] = _G(room)->set_timer(255, 25);

	if (!_G(flags).LoadGame) {
		if (_G(gameState).R48TaxiEntry) {
			hideCur();
			_G(gameState).R48TaxiEntry = false;
			_G(gameState).scrollx = 0;
			_G(gameState).scrolly = 0;
			_G(gameState)._personHide[P_CHEWY] = true;
			_G(gameState)._personHide[P_HOWARD] = true;
			_G(det)->hideStaticSpr(2);
			_G(zoom_horizont) = 0;
			setPersonPos(-6, 16, P_HOWARD, P_RIGHT);
			setPersonPos(3, 42, P_CHEWY, P_RIGHT);
			g_engine->_sound->stopSound(0);
			g_engine->_sound->playSound(7, 1);
			startSetAILWait(7, 1, ANI_BACK);
			startSetAILWait(8, 1, ANI_FRONT);
			g_engine->_sound->stopSound(1);
			g_engine->_sound->playSound(7, 0);
			startSetAILWait(7, 1, ANI_FRONT);
			setupScreen(DO_SETUP);

			if (!_G(gameState).R56GetTabak) {
				flic_cut(FCUT_074);
				_G(det)->showStaticSpr(2);
			}

			_G(room)->set_zoom(23);
			_G(gameState).ZoomXy[P_HOWARD][0] = 17;
			_G(gameState).ZoomXy[P_HOWARD][1] = 37;
			_G(gameState)._personHide[P_CHEWY] = false;
			_G(gameState)._personHide[P_HOWARD] = false;
			_G(SetUpScreenFunc) = setup_func;
			_G(spieler_mi)[P_CHEWY].Mode = true;
			autoMove(1, P_CHEWY);
			_G(spieler_mi)[P_CHEWY].Mode = false;
			_G(mouseLeftClick) = false;
			showCur();
		} else if (_G(gameState).R62Flucht && !_G(gameState).flags32_10) {
			_G(mouseLeftClick) = false;
			_G(gameState).ZoomXy[P_HOWARD][0] = 40;
			_G(gameState).ZoomXy[P_HOWARD][1] = 86;
			_G(zoom_horizont) = 114;
			_G(room)->set_zoom(70);
			_G(gameState).R62Flucht = false;
			setPersonPos(308, 97, P_HOWARD, P_RIGHT);
			setPersonPos(429, 146, P_CHEWY, P_LEFT);
			_G(gameState).scrollx = 262;
			_G(gameState)._personHide[P_HOWARD] = false;
			_G(det)->showStaticSpr(9);
			_G(det)->showStaticSpr(8);
			_G(room)->set_timer_status(0, TIMER_STOP);
			_G(det)->del_static_ani(0);
			_G(det)->set_static_ani(3, -1);
			_G(mouseLeftClick) = false;
			_G(atds)->stopAad();
			hideCur();
			startAadWait(306);
			showCur();
			flic_cut(FCUT_076);
			mode = 1;
			cur_2_inventory();
			remove_inventory(56);
			remove_inventory(66);
			remove_inventory(49);
			remove_inventory(65);
			remove_inventory(77);
			remove_inventory(82);
		} else if (_G(gameState).flags32_10) {
			if (!_G(gameState).flags34_8) {
				_G(gameState).flags34_8 = true;
				mode = 2;
			} else if (_G(gameState).flags34_40) {
				_G(atds)->delControlBit(362, ATS_ACTIVE_BIT);
				_G(atds)->setControlBit(367, ATS_ACTIVE_BIT);
				_G(atds)->setControlBit(366, ATS_ACTIVE_BIT);
				_G(gameState).room_e_obj[137].Attribut = 3;
				if (_G(gameState).flags33_80) {
					_G(out)->setPointer(nullptr);
					_G(out)->cls();
					flic_cut(FCUT_113);
					_G(gameState)._personRoomNr[P_HOWARD] = 89;
					load_chewy_taf(CHEWY_NORMAL);
					_G(gameState).mi[P_HOWARD] = 0;
					_G(gameState).SVal2 = 0;
					_G(gameState).flags35_2 = true;
					mode = 3;
				} else {
					hideCur();
					_G(gameState).scrollx = _G(gameState).scrolly = 0;
					_G(zoom_horizont) = 0;
					setPersonPos(-6, 16, P_HOWARD, P_RIGHT);
					setPersonPos(3, 42, P_CHEWY, P_RIGHT);
					_G(room)->set_zoom(23);
					_G(gameState).ZoomXy[P_HOWARD][0] = 17;
					_G(gameState).ZoomXy[P_HOWARD][1] = 37;
					_G(SetUpScreenFunc) = setup_func;
					_G(spieler_mi)[P_CHEWY].Mode = true;
					autoMove(1, P_CHEWY);
					_G(spieler_mi)[P_CHEWY].Mode = false;
				}
				showCur();
			} else {
				mode = 2;
			}
		}
	}

	_G(SetUpScreenFunc) = setup_func;
	_G(gameState).ZoomXy[P_HOWARD][0] = 40;
	_G(gameState).ZoomXy[P_HOWARD][1] = 86;
	_G(zoom_horizont) = 114;
	_G(room)->set_zoom(70);

	switch(mode) {
	case 1:
		_G(gameState)._personRoomNr[P_HOWARD] = 66;
		_G(gameState)._personRoomNr[P_NICHELLE] = 66;
		_G(gameState).r88DestRoom = 82;
		_G(out)->setPointer(nullptr);
		_G(out)->cls();
		_G(flags).NoPalAfterFlc = true;
		flic_cut(FCUT_116);
		register_cutscene(21);
		_G(out)->setPointer(nullptr);
		_G(out)->cls();
		_G(gameState).PersonGlobalDia[P_HOWARD] = 10025;
		_G(gameState).PersonDiaRoom[P_HOWARD] = 1;
		switchRoom(66);
		break;
	case 2:
		_G(out)->setPointer(nullptr);
		_G(out)->cls();
		flic_cut(FCUT_110);
		_G(gameState).flags34_20 = true;
		_G(gameState)._personRoomNr[P_HOWARD] = 90;
		switchRoom(90);
		break;
	case 3:
		switchRoom(89);
		break;
	default:
		break;
	}

	g_engine->_sound->playSound(9, 0);
	g_engine->_sound->playSound(9);
}

void Room56::xit() {
	_G(spieler_mi)[P_HOWARD].Mode = false;
	_G(gameState).ScrollxStep = 1;
}

bool Room56::timer(int16 t_nr, int16 ani_nr) {
	if (t_nr == _G(timer_nr)[0])
		start_flug();
	else
		return true;

	return false;
}

int16 Room56::use_taxi() {
	int16 action_ret = false;
	if (!_G(gameState).inv_cur) {
		action_ret = true;
		hideCur();
		autoMove(1, P_CHEWY);
		g_engine->_sound->stopSound(0);
		g_engine->_sound->playSound(7, 1);
		startSetAILWait(7, 1, ANI_BACK);
		_G(det)->startDetail(8, 1, ANI_FRONT);
		_G(zoom_horizont) = 0;
		_G(room)->set_zoom(23);
		_G(gameState).ZoomXy[P_HOWARD][0] = 17;
		_G(gameState).ZoomXy[P_HOWARD][1] = 37;
		_G(spieler_mi)[P_CHEWY].Mode = true;
		goAutoXy(3, 42, P_CHEWY, ANI_WAIT);
		_G(spieler_mi)[P_CHEWY].Mode = false;
		_G(gameState)._personHide[P_CHEWY] = true;
		_G(gameState).R48TaxiPerson[P_CHEWY] = true;
		if (_G(gameState)._personRoomNr[P_HOWARD] == 56) {
			_G(gameState)._personHide[P_HOWARD] = true;
			_G(gameState).R48TaxiPerson[P_HOWARD] = true;
			_G(gameState)._personRoomNr[P_HOWARD] = 48;
		}
		showCur();
		switchRoom(48);
	}
	return action_ret;
}

void Room56::talk_man() {
	autoMove(3, P_CHEWY);
	if (!_G(gameState).R56AbfahrtOk) {
		startAdsWait(16);
	} else if (!_G(gameState).R62Flucht) {
		hideCur();
		startAadWait(343);
		showCur();
	}
}

int16 Room56::use_man() {
	int16 action_ret = false;
	if (_G(gameState).flags32_10 || !isCurInventory(FLASCHE_INV))
		return action_ret;
	
	action_ret = true;
	hideCur();
	autoMove(3, P_CHEWY);
	start_spz_wait(CH_ROCK_GET2, 1, false, P_CHEWY);
	_G(room)->set_timer_status(0, TIMER_STOP);
	_G(det)->del_static_ani(0);

	if (!_G(gameState).R56WhiskyMix) {
		startSetAILWait(4, 1, ANI_FRONT);

		_G(det)->set_static_ani(5, -1);
		startAadWait(304);
		_G(det)->del_static_ani(5);
		_G(room)->set_timer_status(0, TIMER_START);
		_G(det)->set_static_ani(0, -1);
	} else {
		delInventory(_G(gameState).AkInvent);
		_G(gameState).R56AbfahrtOk = true;
		startSetAILWait(6, 1, ANI_FRONT);

		_G(det)->set_static_ani(1, -1);
		startAadWait(305);
		_G(det)->del_static_ani(1);
	}
	_G(room)->set_timer_status(0, TIMER_START);
	_G(det)->set_static_ani(0, -1);

	showCur();
	return action_ret;
}

int16 Room56::use_kneipe() {
	int16 action_ret = false;
	if (!_G(gameState).flags32_10) {
		if (_G(menu_item) == CUR_WALK && !_G(gameState).inv_cur && _G(atds)->getControlBit(362, ATS_ACTIVE_BIT) == 0) {
			action_ret = true;
			hideCur();
			if (_G(gameState).R56Kneipe) {
				startAadWait(344);
			} else {
				_G(SetUpScreenFunc) = nullptr;
				autoMove(4, P_CHEWY);
				_G(gameState)._personHide[P_CHEWY] = true;
				goAutoXy(160, 58, P_HOWARD, ANI_FRONT);
				_G(gameState)._personHide[P_HOWARD] = true;
				_G(gameState).R56Kneipe = true;
				_G(flags).NoScroll = true;
				auto_scroll(0, 0);
				startSetAILWait(12, 3, ANI_FRONT);
				flic_cut(FCUT_075);
				g_engine->_sound->stopSound(0);
				g_engine->_sound->playSound(9);
				_G(det)->startDetail(10, 6, ANI_FRONT);
				startAadWait(307);
				_G(det)->stop_detail(10);
				g_engine->_sound->playSound(10, 0);
				_G(out)->ausblenden(0);
				setupScreen(DO_SETUP);
				_G(gameState)._personHide[P_CHEWY] = false;
				_G(gameState)._personHide[P_HOWARD] = false;
				_G(gameState).scrollx = 0;
				setPersonPos(23, 70, P_HOWARD, P_RIGHT);
				setPersonPos(50, 81, P_CHEWY, P_LEFT);
				_G(fx_blend) = BLEND3;
				startAadWait(308);

				_G(SetUpScreenFunc) = setup_func;
				_G(flags).NoScroll = false;
				if (_G(obj)->checkInventory(SACKGELD_INV)) {
					remove_inventory(SACKGELD_INV);
					startAadWait(309);
				}
			}
			showCur();
		}
	} else if (isCurInventory(18)) {
		hideCur();
		if (_G(gameState).flags34_10) {
			_G(gameState).flags33_80 = true;
			autoMove(4, P_CHEWY);
			flic_cut(FCUT_111);
			_G(fx_blend) = BLEND3;
			startAadWait(522);
			start_spz_wait(66, 1, false, P_CHEWY);
			start_spz(67, 255, false, P_CHEWY);
			startAadWait(524);
			_G(gameState).SVal1 = 56;
			_G(gameState).SVal2 = 523;
			cur_2_inventory();
			switchRoom(92);
		} else {
			startAadWait(518);
		}
		showCur();
	} else if (_G(menu_item) == 0 || _G(menu_item) == 2 || (_G(menu_item) == 1 && !_G(gameState).inv_cur)){
		hideCur();
		action_ret = 1;
		_G(mouseLeftClick) = false;
		autoMove(4, P_CHEWY);
		startAadWait(521);
		_G(out)->setPointer(nullptr);
		_G(out)->cls();
		_G(flags).NoPalAfterFlc = true;
		flic_cut(FCUT_112);
		register_cutscene(31);

		setupScreen(NO_SETUP);
		_G(fx_blend) = BLEND3;
		showCur();
		g_engine->_sound->playSound(9, 0);
		g_engine->_sound->playSound(9);
	}
	return action_ret;
}

void Room56::start_flug() {
	if (!_G(gameState).flags32_10 && !_G(r56koch_flug)) {
		_G(r56koch_flug) = 12;
		_G(det)->startDetail(_G(r56koch_flug), 1, ANI_FRONT);
	}
}

void Room56::setup_func() {
	++_G(spieler_mi)[P_HOWARD].Vorschub;
	if (_G(spieler_mi)[P_HOWARD].Vorschub > 8)
		_G(spieler_mi)[P_HOWARD].Vorschub = 8;

	if (!_G(gameState).flags32_10) {
		switch (_G(r56koch_flug)) {
		case 10:
			if (_G(det)->get_ani_status(10) == false) {
				_G(r56koch_flug) = 0;

				_G(uhr)->resetTimer(_G(timer_nr)[0], 0);
			}
			break;

		case 11:
			if (_G(det)->get_ani_status(11) == false) {
				_G(det)->startDetail(10, 1, ANI_FRONT);
				_G(r56koch_flug) = 10;
			}
			break;

		case 12:
			if (_G(det)->get_ani_status(12) == false) {
				_G(det)->startDetail(11, 1, ANI_FRONT);
				_G(r56koch_flug) = 11;
			}
			break;

		default:
			break;
		}
	}

	if (_G(gameState)._personRoomNr[P_HOWARD] == 56) {
		calc_person_look();
		const int16 ch_x = _G(spieler_vector)[P_CHEWY].Xypos[0];
		const int16 ch_y = _G(spieler_vector)[P_CHEWY].Xypos[1];
		int16 x, y;
		if (ch_x < 196) {
			x = 23;
			y = 70;
		} else if (ch_x < 283) {
			x = 119;
			y = 62;
		} else if (ch_y < 120) {
			x = 254;
			y = 65;
		} else {
			x = 308;
			y = 97;
		}
		goAutoXy(x, y, P_HOWARD, ANI_GO);
	}

	if (_G(gameState).flags32_10)
		return;
	
	if (!_G(atds)->getControlBit(362, ATS_ACTIVE_BIT) && _G(menu_item) == CUR_WALK) {
		if (g_events->_mousePos.x + _G(gameState).scrollx >= 157 && g_events->_mousePos.x + _G(gameState).scrollx <= 204 && g_events->_mousePos.y >= 28 && g_events->_mousePos.y <= 89)
			cursorChoice(CUR_EXIT_TOP);
		else
			cursorChoice(CUR_WALK);
	}
}

} // namespace Rooms
} // namespace Chewy
