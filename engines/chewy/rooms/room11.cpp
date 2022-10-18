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
#include "chewy/rooms/room11.h"

namespace Chewy {
namespace Rooms {

AniBlock ABLOCK17[2] = {
	{ 8, 1, ANI_FRONT, ANI_WAIT, 0 },
	{ 9, 255, ANI_FRONT, ANI_GO, 0 },
};

AniBlock ABLOCK18[2] = {
	{ 7, 1, ANI_FRONT, ANI_WAIT, 0 },
	{ 7, 1, ANI_BACK, ANI_WAIT, 0 },
};


void Room11::entry() {
	_G(zoom_horizont) = 80;
	_G(flags).ZoomMov = true;
	_G(zoom_mov_fak) = 2;

	if (_G(gameState).R12ChewyBork) {
		if (!_G(gameState).R11DoorRightB) {
			_G(obj)->calc_rsi_flip_flop(SIB_TBUTTON2_R11);
			_G(gameState).R11DoorRightB = exit_flip_flop(5, 22, -1, 98, -1, -1,
				EXIT_TOP, -1, (int16)_G(gameState).R11DoorRightB);
			_G(obj)->calc_all_static_detail();
		}

		_G(obj)->hide_sib(SIB_TBUTTON1_R11);
		_G(obj)->hide_sib(SIB_SCHLITZ_R11);
		_G(obj)->hide_sib(SIB_TBUTTON2_R11);
		_G(obj)->hide_sib(SIB_TBUTTON3_R11);
		_G(gameState).room_e_obj[20].Attribut = 255;
		_G(gameState).room_e_obj[21].Attribut = 255;
		_G(atds)->delControlBit(121, ATS_ACTIVE_BIT);

		if (!_G(flags).LoadGame) {
			startAniBlock(2, ABLOCK17);
			autoMove(8, P_CHEWY);
			startAadWait(31);
			_G(det)->stopDetail(9);
		}
		_G(det)->showStaticSpr(8);
		if (!_G(flags).LoadGame)
			autoMove(6, P_CHEWY);
		_G(timer_nr)[0] = _G(room)->set_timer(255, 10);
	}
}

bool Room11::timer(int16 t_nr, int16 ani_nr) {
	if (t_nr == _G(timer_nr)[0])
		bork_zwinkert();

	return false;
}

void Room11::gedAction(int index) {
	switch (index) {
	case 0:
		chewy_bo_use();
		break;

	default:
		break;
	}
}

void Room11::bork_zwinkert() {
	if (!_G(flags).AutoAniPlay) {
		_G(flags).AutoAniPlay = true;
		_G(det)->hideStaticSpr(8);
		startAniBlock(2, ABLOCK18);
		_G(uhr)->resetTimer(_G(timer_nr)[0], 0);
		_G(det)->showStaticSpr(8);
		_G(flags).AutoAniPlay = false;
	}
}

void Room11::talk_debug() {
	if (_G(gameState).R12ChewyBork) {
		_G(flags).AutoAniPlay = true;
		autoMove(8, P_CHEWY);
		startDialogCloseupWait(5);
		_G(menu_item) = CUR_WALK;
		cursorChoice(CUR_WALK);

		_G(flags).AutoAniPlay = false;
	}
}

void Room11::chewy_bo_use() {
	if (_G(gameState).R12ChewyBork) {
		hideCur();
		_G(flags).AutoAniPlay = true;

		stopPerson(P_CHEWY);
		_G(det)->hideStaticSpr(8);
		startAniBlock(2, ABLOCK17);
		startAadWait(32);
		_G(det)->stopDetail(9);
		_G(det)->showStaticSpr(8);
		autoMove(6, P_CHEWY);

		_G(flags).AutoAniPlay = false;
		showCur();
	}
}

int16 Room11::scanner() {
	int16 actionFl = false;

	if (!_G(gameState).R12ChewyBork) {
		autoMove(7, P_CHEWY);

		if (!_G(gameState).R11CardOk) {
			actionFl = true;
			startAadWait(13);
		} else if (isCurInventory(BORK_INV)) {
			hideCur();
			setupScreen(DO_SETUP);
			actionFl = true;

			start_aad(105, 0);
			flic_cut(FCUT_010);
			register_cutscene(4);
			_G(gameState).R11TerminalOk = true;
			cur_2_inventory();
			_G(menu_item) = CUR_TALK;
			cursorChoice(_G(menu_item));
			startAadWait(12);
			showCur();
			loadDialogCloseup(3);
		} else if (!_G(cur)->usingInventoryCursor()) {
			if (!_G(gameState).R11TerminalOk) {
				actionFl = true;
				flic_cut(FCUT_009);
				startAadWait(20);
			} else {
				actionFl = true;
				startAadWait(12);
				_G(menu_item) = CUR_TALK;
				cursorChoice(_G(menu_item));
				loadDialogCloseup(3);
			}
		}
	}

	return actionFl;
}

void Room11::get_card() {
	if (_G(gameState).R11CardOk) {
		cur_2_inventory();
		_G(gameState).R11CardOk = false;
		_G(obj)->addInventory(_G(gameState).R11IdCardNr, &_G(room_blk));

		_G(cur)->setInventoryCursor(_G(gameState).R11IdCardNr);
		_G(det)->stopDetail(0);
		_G(atds)->set_ats_str(83, TXT_MARK_LOOK, 0, ATS_DATA);
		_G(atds)->set_ats_str(84, TXT_MARK_LOOK, 0, ATS_DATA);
	}
}

void Room11::put_card() {
	if (isCurInventory(RED_CARD_INV) || isCurInventory(YEL_CARD_INV)) {
		_G(gameState).R11IdCardNr = _G(cur)->getInventoryCursor();
		delInventory(_G(gameState).R11IdCardNr);
		_G(det)->startDetail(0, 255, ANI_FRONT);
		_G(atds)->set_ats_str(83, TXT_MARK_LOOK, 1, ATS_DATA);
		_G(atds)->set_ats_str(84, TXT_MARK_LOOK, 1, ATS_DATA);
		_G(gameState).R11CardOk = true;

		if (!_G(gameState).R11TerminalOk)
			startAadWait(16);
	}
}

} // namespace Rooms
} // namespace Chewy
