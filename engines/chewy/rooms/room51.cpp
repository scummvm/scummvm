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
#include "chewy/rooms/room51.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

static const AniBlock ABLOCK37[5] = {
	{ 6, 1, ANI_FRONT, ANI_WAIT, 0 },
	{ 7, 3, ANI_FRONT, ANI_WAIT, 0 },
	{ 6, 1, ANI_BACK, ANI_WAIT, 0 },
	{ 5, 1, ANI_BACK, ANI_WAIT, 0 },
	{ 2, 1, ANI_BACK, ANI_WAIT, 0 },
};

bool Room51::_flag;
bool Room51::_enemyFlag[2];
int16 Room51::_tmpx;
int16 Room51::_tmpy;
int Room51::_index;


void Room51::entry() {
	_G(zoom_horizont) = 140;
	_G(flags).ZoomMov = true;
	_G(zoom_mov_fak) = 4;

	if (_G(gameState).flags32_10) {
		_G(atds)->enableEvents(false);
		_G(gameState)._personHide[P_CHEWY] = true;
		_G(gameState)._personHide[P_HOWARD] = true;
		_G(mouseLeftClick) = false;
		_G(gameState).scrollx = 0;
		setPersonPos(34, 120, P_HOWARD, P_RIGHT);
		setPersonPos(234, 69, P_CHEWY, P_LEFT);
		_G(SetUpScreenFunc) = setup_func;
		_G(det)->showStaticSpr(17);
		_index = 0;
		hideCur();

		for (int i = 0; i < 2; ++i) {
			_enemyFlag[i] = false;
			_G(timer_nr)[i] = _G(room)->set_timer(i + 9, i * 2 + 6);
		}

		_G(flags).MainInput = false;
		_flag = false;

	} else {
		_G(det)->hideStaticSpr(17);

		for (int i = 0; i < 2; i++)
			_G(det)->startDetail(3 + i, 1, ANI_FRONT);

		if (_G(gameState)._personRoomNr[P_HOWARD] == 51) {
			_G(gameState).ZoomXy[P_HOWARD][0] = 40;
			_G(gameState).ZoomXy[P_HOWARD][1] = 30;

			if (!_G(flags).LoadGame) {
				setPersonPos(88, 93, P_HOWARD, P_RIGHT);
			}

			if (!_G(gameState).R51FirstEntry) {
				hideCur();
				_G(gameState).R51FirstEntry = true;
				setPersonSpr(P_LEFT, P_CHEWY);
				startAadWait(283);
				showCur();
			}

			_G(SetUpScreenFunc) = setup_func;
			_G(spieler_mi)[P_HOWARD].Mode = true;
		}
	}
}

void Room51::xit(int16 eib_nr) {
	_G(atds)->enableEvents(true);

	if (_G(gameState).flags32_10) {
		_G(flags).MainInput = true;
		_G(gameState)._personHide[P_CHEWY] = false;
		_G(gameState)._personHide[P_HOWARD] = false;
		_G(gameState)._personRoomNr[P_HOWARD] = 91;
		_G(menu_item) = CUR_WALK;
		cursorChoice(_G(menu_item));
		showCur();

	} else if (_G(gameState)._personRoomNr[P_HOWARD] == 51) {
		if (eib_nr == 85) {
			_G(gameState)._personRoomNr[P_HOWARD] = 50;
		} else {
			_G(gameState)._personRoomNr[P_HOWARD] = 52;
		}

		_G(spieler_mi)[P_HOWARD].Mode = false;
	}
}

bool Room51::timer(int16 t_nr, int16 ani_nr) {
	if (_G(gameState).flags32_10)
		timer_action(t_nr, _G(room)->_roomTimer._objNr[ani_nr]);
	else
		return true;

	return false;
}

void Room51::setup_func() {
	if (_G(gameState).flags32_10) {
		_tmpx = g_events->_mousePos.x;
		_tmpy = g_events->_mousePos.y;
		if (_tmpx > 215)
			_tmpx = 215;
		if (_tmpy < 81)
			_tmpy = 81;

		_G(det)->setStaticPos(17, _tmpx, _tmpy, false, false);

		if ((_G(minfo).button == 1 || g_events->getSwitchCode() == 28) && !_flag) {
			_flag = true;
			_G(det)->setDetailPos(8, _tmpx - 20, _tmpy + 41);
			startSetAILWait(8, 1, ANI_FRONT);
			_flag = false;
			++_index;

			switch (_index) {
			case 2:
				startAadWait(512);
				_index = 1000;
				break;

			case 1006:
				startAadWait(513);
				_index = 2000;
				break;

			case 2003:
				startAadWait(615);
				_index = 10000;
				break;

			case 10012:
				startAadWait(514);
				waitShowScreen(5);
				_G(flags).NoPalAfterFlc = true;
				_G(out)->setPointer(nullptr);
				_G(out)->cls();
				flic_cut(FCUT_115);
				register_cutscene(28);
				
				switchRoom(91);
				break;

			default:
				break;
			}
		}
	} else if (_G(gameState)._personRoomNr[P_HOWARD] == 51) {
		calc_person_look();
		const int16 ch_y = _G(moveState)[P_CHEWY].Xypos[1];

		int16 x, y;
		if (ch_y < 129) {
			x = 56;
			y = 106;
		} else {
			x = 31;
			y = 118;
		}

		if (_G(HowardMov) && _G(flags).ExitMov) {
			_G(SetUpScreenFunc) = nullptr;
			_G(HowardMov) = 0;
			autoMove(9, P_HOWARD);
		} else {
			goAutoXy(x, y, P_HOWARD, ANI_GO);
		}
	}
}

int16 Room51::use_door(int16 txt_nr) {
	int16 action_ret = false;

	if (isCurInventory(KEY_INV)) {
		hideCur();
		action_ret = true;

		switch (txt_nr) {
		case 329:
			autoMove(8, P_CHEWY);
			_G(SetUpScreenFunc) = nullptr;
			_G(det)->showStaticSpr(0);

			if (!_G(gameState).R51HotelRoom) {
				autoMove(11, P_HOWARD);
				setPersonSpr(P_LEFT, P_HOWARD);
				_G(gameState).R51HotelRoom = true;
				_G(gameState).room_e_obj[86].Attribut = EXIT_LEFT;
				startAadWait(285);
				_G(atds)->set_all_ats_str(329, 1, ATS_DATA);
				_G(SetUpScreenFunc) = setup_func;

			} else {
				showCur();
				switchRoom(52);
			}
			break;

		case 330:
			autoMove(9, P_CHEWY);

			if (!_G(gameState).R51KillerWeg) {
				_G(det)->playSound(2, 0);
				_G(det)->showStaticSpr(1);
				startSetAILWait(2, 1, ANI_FRONT);
				_G(det)->startDetail(5, 255, ANI_FRONT);

				if (!_G(gameState).R52HotDogOk) {
					startAadWait(287);
					autoMove(12, P_CHEWY);
					_G(det)->stopDetail(5);
					startAniBlock(5, ABLOCK37);
					_G(det)->hideStaticSpr(1);
					_G(det)->stopSound(0);
					startAadWait(284);
				} else {
					_G(gameState).R51KillerWeg = true;
					startAadWait(290);
					_G(det)->stopSound(0);
					_G(out)->fadeOut();
					_G(out)->setPointer(nullptr);
					_G(out)->cls();
					_G(out)->fadeIn(_G(pal));
					_G(flags).NoPalAfterFlc = true;
					_G(det)->showStaticSpr(16);
					flic_cut(FCUT_068);

					_G(det)->hideStaticSpr(16);
					_G(flags).NoPalAfterFlc = false;
					_G(det)->stopDetail(5);
					_G(obj)->show_sib(SIB_AUSRUEST_R52);
					_G(obj)->calc_rsi_flip_flop(SIB_AUSRUEST_R52);
					_G(det)->hideStaticSpr(1);

					_G(fx_blend) = BLEND3;
					setupScreen(DO_SETUP);
					startAadWait(291);
				}
			} else {
				startAadWait(401);
			}
			break;

		case 331:
		case 334:
			autoMove((txt_nr == 331) ? 10 : 7, P_CHEWY);
			_G(out)->setPointer(nullptr);
			_G(out)->cls();
			_G(flags).NoPalAfterFlc = true;
			flic_cut(FCUT_114);
			setPersonPos(115, 144, P_CHEWY, P_LEFT);
			_G(fx_blend) = BLEND3;
			setupScreen(NO_SETUP);
			startAadWait(564);
			break;

		case 332:
			autoMove(6, P_CHEWY);
			startAadWait(286);
			break;

		case 333:
			autoMove(4, P_CHEWY);

			switch (_G(gameState).R51DoorCount) {
			case 0:
				_G(det)->showStaticSpr(3);
				startAadWait(278);
				startDetailFrame(0, 1, ANI_FRONT, 3);
				start_spz(HO_BRILL_JMP, 1, ANI_FRONT, P_HOWARD);
				waitDetail(0);

				_G(det)->showStaticSpr(14);
				startAadWait(279);
				++_G(gameState).R51DoorCount;
				_G(obj)->show_sib(SIB_FLASCHE_R51);
				_G(obj)->calc_rsi_flip_flop(SIB_FLASCHE_R51);
				_G(det)->hideStaticSpr(3);
				break;

			case 1:
				_G(det)->showStaticSpr(3);
				startAadWait(280);
				startSetAILWait(1, 1, ANI_FRONT);
				++_G(gameState).R51DoorCount;
				_G(obj)->show_sib(SIB_KAPPE_R51);
				_G(obj)->calc_rsi_flip_flop(SIB_KAPPE_R51);
				_G(det)->hideStaticSpr(3);
				_G(det)->showStaticSpr(15);
				break;

			default:
				startAadWait(281);
				if (_G(gameState)._personRoomNr[P_HOWARD] == 51)
					startAadWait(282);
				break;
			}
			break;

		default:
			break;
		}

		showCur();
	}

	return action_ret;
}

void Room51::timer_action(int16 t_nr, int16 obj_nr) {
	if (obj_nr == 9 || obj_nr == 10) {
		if (!_enemyFlag[obj_nr - 9]) {
			_G(det)->startDetail(obj_nr, 1, ANI_FRONT);
			_enemyFlag[obj_nr - 9] = true;

		} else if (!_G(det)->get_ani_status(obj_nr)) {
			_G(det)->startDetail(obj_nr, 1, ANI_GO);
			_G(det)->startDetail(obj_nr + 2, 1, ANI_FRONT);
			_G(uhr)->resetTimer(t_nr, 0);
			_enemyFlag[obj_nr - 9] = false;
		}
	}
}

} // namespace Rooms
} // namespace Chewy
