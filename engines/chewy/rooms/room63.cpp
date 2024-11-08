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
#include "chewy/rooms/room63.h"

namespace Chewy {
namespace Rooms {

void Room63::entry() {
	_G(zoom_horizont) = 76;
	_G(gameState).ScrollxStep = 2;
	_G(r63Schalter) = false;
	_G(r63RunDia) = 0;
	if (_G(gameState).R63FxManAway)
		_G(det)->del_static_ani(5);
	if (!_G(gameState).R63Uhr) {
		hideCur();
		_G(det)->startDetail(12, 255, ANI_FRONT);
		_G(det)->startDetail(10, 255, ANI_FRONT);
		_G(det)->startDetail(18, 255, ANI_FRONT);
		autoMove(4, P_CHEWY);
		_G(det)->stopDetail(10);
		_G(det)->startDetail(11, 255, ANI_FRONT);
		auto_scroll(58, 0);
		startAadWait(355);
		_G(det)->stopDetail(11);
		_G(det)->startDetail(10, 255, ANI_FRONT);
		showCur();
	} else if (!_G(gameState).R63Fire) {
		_G(det)->showStaticSpr(10);
		_G(det)->showStaticSpr(12);
	} else if (_G(gameState).R62LauraTransformation) {
		_G(atds)->setControlBit(383, ATS_ACTIVE_BIT);
		_G(SetUpScreenFunc) = setup_func;
		cur_2_inventory();
		_G(gameState).scrollx = 176;
		setPersonPos(424, 78, P_CHEWY, P_LEFT);
		_G(gameState)._personHide[P_CHEWY] = true;
		_G(gameState).room_e_obj[95].Attribut = 255;
		_G(r63ChewyAni) = 0;
		_G(det)->startDetail(0, 1, ANI_FRONT);
	}
}

void Room63::setup_func() {
	if (!_G(gameState).R62LauraTransformation)
		return;

	cur_2_inventory();
	_G(menu_item) = CUR_USE;
	cursorChoice(CUR_USE);
	
	switch (_G(r63ChewyAni)) {
	case 0:
		if (_G(det)->get_ani_status(0) == false) {
			if (_G(r63RunDia) < 4)
				++_G(r63RunDia);
			start_aad(370 + _G(r63RunDia));
			_G(det)->startDetail(1, 1, ANI_FRONT);
			_G(r63ChewyAni) = 1;
		}
		break;

	case 1:
		if (_G(det)->get_ani_status(1) == false) {
			_G(gameState).ScrollxStep = 4;
			setPersonPos(0, 0, P_CHEWY, P_RIGHT);
			_G(det)->startDetail(22, 1, ANI_FRONT);
			_G(det)->startDetail(2, 1, ANI_FRONT);
			_G(atds)->stopAad();
			_G(r63ChewyAni) = 2;
		}
		break;

	case 2:
		if (_G(det)->get_ani_status(2) == false) {
			_G(det)->startDetail(3, 1, ANI_FRONT);
			_G(r63ChewyAni) = 3;
		}
		break;

	case 3:
		if (_G(det)->get_ani_status(3) == false) {
			if (!_G(flags).AutoAniPlay) {
				_G(flags).AutoAniPlay = true;
				_G(flags).NoScroll = true;

				_G(gameState).ScrollxStep = 16;
				_G(gameState).scrollx -= _G(gameState).scrollx % 16;
				auto_scroll(176, 0);
				setPersonPos(424, 78, P_CHEWY, P_LEFT);
				_G(flags).NoScroll = false;
				_G(gameState).ScrollxStep = 4;
				if (!_G(r63Schalter)) {
					_G(det)->startDetail(0, 1, ANI_FRONT);
					_G(r63ChewyAni) = 0;
				} else
					bork_platt();
				_G(flags).AutoAniPlay = false;
			}
		}
		break;

	default:
		break;
	}
}

void Room63::bork_platt() {
	_G(gameState).R62LauraTransformation = false;
	_G(r63Schalter) = false;
	_G(gameState).room_e_obj[95].Attribut = EXIT_TOP;
	flic_cut(FCUT_081);
	flic_cut(FCUT_082);
	_G(gameState)._personHide[P_CHEWY] = false;
	setShadowPalette(4, true);
	_G(spieler_mi)[P_CHEWY].Mode = true;
	autoMove(6, P_CHEWY);
	_G(spieler_mi)[P_CHEWY].Mode = false;
	startAadWait(370);
	startSetAILWait(4, 1, ANI_FRONT);
	_G(det)->showStaticSpr(13);
	startAadWait(361);
	_G(out)->cls();
	_G(flags).NoPalAfterFlc = true;
	flic_cut(FCUT_079);
	_G(fx_blend) = BLEND3;
	showCur();
	_G(flags).MainInput = true;
	_G(gameState).R62Flucht = true;
	_G(gameState)._personRoomNr[P_HOWARD] = 56;
	switchRoom(56);
}

void Room63::talk_hunter() {
	hideCur();
	autoMove(3, P_CHEWY);
	if (_G(gameState).R63Uhr)
		_G(det)->hideStaticSpr(10);
	_G(det)->startDetail(10, 255, ANI_FRONT);
	startAadWait(356);
	if (_G(gameState).R63Uhr) {
		_G(det)->showStaticSpr(10);
		_G(det)->stopDetail(10);
	}
	showCur();
}

void Room63::talk_regie() {
	hideCur();
	autoMove(3, P_CHEWY);
	if (_G(gameState).R63Uhr)
		_G(det)->hideStaticSpr(12);
	_G(det)->stopDetail(18);
	_G(det)->startDetail(19, 255, ANI_FRONT);
	startAadWait(357);
	_G(det)->stopDetail(19);
	if (_G(gameState).R63Uhr) {
		_G(det)->showStaticSpr(12);
	} else {
		_G(det)->startDetail(18, 255, ANI_FRONT);
	}
	showCur();
}

void Room63::talk_fx_man() {
	hideCur();
	autoMove(1, P_CHEWY);
	startAadWait(358);
	showCur();
}

int16 Room63::use_fx_man() {
	int16 action_ret = false;
	if (isCurInventory(MASKE_INV)) {
		action_ret = true;
		hideCur();
		autoMove(1, P_CHEWY);
		delInventory(_G(cur)->getInventoryCursor());
		remove_inventory(34);
		startAadWait(359);
		_G(det)->del_static_ani(5);
		startSetAILWait(6, 1, ANI_FRONT);
		_G(det)->startDetail(7, 255, ANI_FRONT);
		startAadWait(362);
		_G(det)->stopDetail(7);
		startSetAILWait(8, 1, ANI_FRONT);
		_G(gameState).R63FxManAway = true;
		_G(atds)->setControlBit(384, ATS_ACTIVE_BIT);
		showCur();
	}
	return action_ret;
}

int16 Room63::use_schalter() {
	int16 action_ret = false;
	if (!_G(cur)->usingInventoryCursor()) {
		action_ret = true;
		if (_G(gameState).R63FxManAway) {
			if (_G(gameState).R62LauraTransformation) {
				_G(r63Schalter) = true;
				hideCur();
				_G(flags).MainInput = false;
			} else {
				hideCur();
				autoMove(1, P_CHEWY);
				start_spz_wait(CH_ROCK_GET2, 1, false, P_CHEWY);
				_G(det)->showStaticSpr(2);
				setPersonSpr(P_LEFT, P_CHEWY);
				startSetAILWait(21, 1, ANI_FRONT);
				_G(det)->showStaticSpr(14);
				waitShowScreen(18);
				_G(det)->hideStaticSpr(14);
				startSetAILWait(24, 1, ANI_FRONT);
				_G(det)->showStaticSpr(1);
				_G(det)->hideStaticSpr(2);
				startAadWait(364);
				_G(atds)->set_all_ats_str(385, 1, ATS_DATA);
				showCur();
			}
		} else {
			hideCur();
			startAadWait(363);
			showCur();
		}
	}
	return action_ret;
}

void Room63::talk_girl() {
	autoMove(2, P_CHEWY);
	_G(det)->stopDetail(12);
	startSetAILWait(13, 1, ANI_FRONT);
	_G(det)->set_static_ani(14, -1);
	startDialogCloseupWait(17);
	_G(det)->del_static_ani(14);
	_G(det)->startDetail(12, 255, ANI_FRONT);
}

int16 Room63::use_girl() {
	int16 action_ret = false;
	if (isCurInventory(UHR_INV)) {
		action_ret = true;
		hideCur();
		autoMove(2, P_CHEWY);
		delInventory(_G(cur)->getInventoryCursor());
		_G(det)->stopDetail(12);
		startSetAILWait(13, 1, ANI_FRONT);
		_G(det)->set_static_ani(14, -1);
		startAadWait(365);
		_G(det)->del_static_ani(14);
		_G(det)->startDetail(15, 255, ANI_FRONT);
		startAadWait(360);
		_G(det)->stopDetail(15);
		startSetAILWait(16, 1, ANI_FRONT);
		_G(gameState).R63Uhr = true;
		_G(det)->stopDetail(10);
		_G(det)->stopDetail(18);
		_G(det)->showStaticSpr(10);
		_G(det)->showStaticSpr(12);
		_G(atds)->set_all_ats_str(381, 1, ATS_DATA);
		_G(atds)->set_all_ats_str(382, 1, ATS_DATA);
		_G(atds)->setControlBit(380, ATS_ACTIVE_BIT);
		startAadWait(367);
		showCur();
	}
	return action_ret;
}

int16 Room63::use_aschenbecher() {
	int16 action_ret = false;
	if (isCurInventory(ASCHE_INV)) {
		action_ret = true;
		hideCur();
		if (_G(gameState).R63Uhr) {
			if (_G(gameState).R63FxManAway) {
				autoMove(5, P_CHEWY);
				delInventory(_G(cur)->getInventoryCursor());
				_G(flags).NoScroll = true;
				auto_scroll(70, 0);
				autoMove(1, P_CHEWY);
				_G(gameState)._personHide[P_CHEWY] = true;
				_G(det)->startDetail(20, 255, ANI_FRONT);
				_G(det)->startDetail(10, 255, ANI_FRONT);
				_G(det)->startDetail(18, 255, ANI_FRONT);
				_G(det)->hideStaticSpr(10);
				_G(det)->hideStaticSpr(12);
				startAadWait(368);
				flic_cut(FCUT_080);
				_G(atds)->setControlBit(381, ATS_ACTIVE_BIT);
				_G(atds)->setControlBit(382, ATS_ACTIVE_BIT);
				_G(atds)->set_all_ats_str(383, 1, ATS_DATA);
				_G(gameState).R63Fire = true;
				_G(gameState)._personHide[P_CHEWY] = false;
				_G(gameState).scrollx = 0;
				setPersonPos(187, 42, P_CHEWY, P_RIGHT);
				switchRoom(64);
				_G(flags).NoScroll = false;
			} else
				startAadWait(369);
		} else
			startAadWait(366);
		
		showCur();
	}
	return action_ret;
}

} // namespace Rooms
} // namespace Chewy
