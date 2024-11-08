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
#include "chewy/rooms/room32.h"

namespace Chewy {
namespace Rooms {

void Room32::entry() {
	if (_G(gameState).R32HowardWeg)
		_G(det)->hideStaticSpr(0);
	if (!_G(gameState).R32Script && _G(gameState).R32UseSchreib)
		_G(det)->showStaticSpr(5);
}

int16 Room32::use_howard() {
	int16 dia_nr = 0;
	int16 ani_nr = 0;
	int16 action_flag = false;
	hideCur();

	if (isCurInventory(TRICHTER_INV)) {
		if (_G(gameState).R33MunterGet) {
			autoMove(1, P_CHEWY);
			cur_2_inventory();
			remove_inventory(MUNTER_INV);
			register_cutscene(11);
			flic_cut(FCUT_043);
			_G(atds)->setControlBit(230, ATS_ACTIVE_BIT);
			start_spz(CH_TALK12, 255, ANI_FRONT, P_CHEWY);
			startAadWait(75);
			waitShowScreen(5);
			autoMove(5, P_CHEWY);
			waitShowScreen(10);
			start_spz(CH_TALK12, 255, ANI_FRONT, P_CHEWY);
			startAadWait(125);
			waitShowScreen(10);

			_G(det)->hideStaticSpr(0);
			startDetailFrame(0, 1, ANI_FRONT, 9);
			startSetAILWait(1, 1, ANI_BACK);
			_G(det)->showStaticSpr(7);
			_G(det)->showStaticSpr(6);
			waitShowScreen(20);
			_G(det)->hideStaticSpr(7);
			startSetAILWait(1, 1, ANI_FRONT);
			start_spz(CH_TALK3, 255, ANI_FRONT, P_CHEWY);
			ani_nr = CH_TALK3;
			dia_nr = 164;
			_G(gameState).R32HowardWeg = true;
			_G(gameState).R39HowardDa = true;
			remove_inventory(TRICHTER_INV);
		} else {
			ani_nr = CH_TALK12;
			dia_nr = 73;
		}
	} else if (isCurInventory(MUNTER_INV)) {
		ani_nr = CH_TALK12;
		dia_nr = 74;
	}

	if (dia_nr) {
		start_spz(ani_nr, 255, ANI_FRONT, P_CHEWY);
		startAadWait(dia_nr);
		action_flag = true;
	}

	showCur();
	return action_flag;
}

void Room32::use_schreibmaschine() {
	int16 dia_nr = -1;
	int16 ani_nr = -1;

	hideCur();
	if (_G(gameState).R32HowardWeg) {
		if (_G(cur)->usingInventoryCursor()) {
			switch (_G(cur)->getInventoryCursor()) {
			case CYB_KRONE_INV:
				if (!_G(gameState).R32UseSchreib) {
					if (!_G(gameState).R32PapierOk) {
						ani_nr = CH_TALK12;
						dia_nr = 87;
					} else {
						autoMove(3, P_CHEWY);
						_G(gameState).R32UseSchreib = true;
						cur_2_inventory();
						flic_cut(FCUT_044);
						register_cutscene(12);
						_G(det)->showStaticSpr(5);
						_G(atds)->set_all_ats_str(203, 1, ATS_DATA);
						ani_nr = CH_TALK3;
						dia_nr = 88;
						_G(atds)->set_ats_str(231, TXT_MARK_LOOK, 0, ATS_DATA);
					}
				}
				break;

			case PAPIER_INV:
				autoMove(2, P_CHEWY);
				_G(gameState).R32PapierOk = true;
				start_spz_wait(CH_LGET_O, 1, false, P_CHEWY);
				delInventory(_G(cur)->getInventoryCursor());
				_G(atds)->set_ats_str(231, TXT_MARK_LOOK, 1, ATS_DATA);
				ani_nr = CH_TALK3;
				dia_nr = 86;
				break;

			default:
				ani_nr = CH_TALK12;
				dia_nr = 90;
				break;

			}
		} else {
			ani_nr = CH_TALK12;
			dia_nr = 89;
		}
	} else {
		ani_nr = CH_TALK12;
		dia_nr = 92;
	}

	start_spz(ani_nr, 255, ANI_FRONT, P_CHEWY);
	startAadWait(dia_nr);
	showCur();
}

int16 Room32::get_script() {
	int16 action_flag = false;

	if (!_G(cur)->usingInventoryCursor() && !_G(gameState).R32Script && _G(gameState).R32UseSchreib) {
		action_flag = true;
		_G(gameState).R32Script = true;
		autoMove(4, P_CHEWY);
		invent_2_slot(MANUSKRIPT_INV);
		start_spz_wait(CH_LGET_U, 1, false, P_CHEWY);
		_G(det)->hideStaticSpr(5);
		_G(atds)->set_all_ats_str(203, 0, ATS_DATA);
		start_spz(CH_TALK3, 1, ANI_FRONT, P_CHEWY);
		startAadWait(91);
	}

	return action_flag;
}

} // namespace Rooms
} // namespace Chewy
