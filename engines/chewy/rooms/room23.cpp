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
#include "chewy/rooms/room23.h"

namespace Chewy {
namespace Rooms {

static const AniBlock ABLOCK15[4] = {
	{ 0, 2, ANI_FRONT, ANI_GO, 0 },
	{ 1, 255, ANI_FRONT, ANI_GO, 0 },
	{ 2, 255, ANI_FRONT, ANI_GO, 0 },
	{ 3, 255, ANI_FRONT, ANI_GO, 0 },
};


void Room23::entry() {
	_G(gameState)._personHide[P_CHEWY] = true;
	setPersonPos(135, 69, P_CHEWY, -1);
}

void Room23::cockpit() {
	_G(gameState).scrollx = 0;
	_G(gameState).scrolly = 0;
	_G(mouseLeftClick) = false;
	switchRoom(23);

	if (!_G(gameState).R23Cartridge || !_G(gameState).R25GliderFlamesExtinguished)
		_G(det)->hideStaticSpr(3);
	else
		_G(det)->showStaticSpr(3);
}

int16 Room23::start_gleiter() {
	int16 action_flag = false;

	if (!_G(cur)->usingInventoryCursor()) {
		action_flag = true;
		if (!_G(gameState).R23FluxoFlex)
			startAadWait(23);
		else {
			bool start_ok = true;

			if (_G(gameState).R23GliderExit == 16) {
				if (!_G(gameState).R16F5Exit) {
					start_ok = false;
					startAadWait(35);
				} else if (!_G(gameState).R23Cartridge || !_G(gameState).R18CartSave) {
					start_ok = false;
					startAadWait(41);
				} else if (!_G(gameState).R17EnergyOut) {
					start_ok = false;
					startAadWait(300);
				}
			}

			if (start_ok) {
				hideCur();
				startAniBlock(4, ABLOCK15);
				waitShowScreen(30);

				for (int16 i = 0; i < 4; i++)
					_G(det)->stopDetail(i);

				if (_G(gameState).R23GliderExit == 14) {
					_G(out)->setPointer(nullptr);
					_G(out)->cls();
					_G(flags).NoPalAfterFlc = true;
					flic_cut(FCUT_011);
					register_cutscene(7);
					_G(out)->cls();
					_G(gameState).R23GliderExit = 16;
					setPersonPos(126, 110, P_CHEWY, P_RIGHT);

					switchRoom(_G(gameState).R23GliderExit);
					start_spz_wait(CH_WONDER1, 2, false, P_CHEWY);
					start_spz(CH_TALK2, 255, ANI_FRONT, P_CHEWY);

					_G(gameState).DelaySpeed = 10;
					startAadWait(59);
					stopPerson(P_CHEWY);
					_G(mouseLeftClick) = false;

				} else if (_G(gameState).R23GliderExit == 16) {
					_G(out)->setPointer(nullptr);
					_G(out)->cls();
					flic_cut(FCUT_SPACECHASE_18);
					_G(gameState).R23GliderExit = 25;
					register_cutscene(9);

					cur_2_inventory();
					remove_inventory(5);
					remove_inventory(6);
					remove_inventory(4);
					remove_inventory(15);
					remove_inventory(16);
					switchRoom(_G(gameState).R23GliderExit);
				}

				showCur();
			}
		}
	}

	return action_flag;
}

void Room23::use_cartridge() {
	delInventory(_G(cur)->getInventoryCursor());
	_G(gameState).R23Cartridge = true;

	if (_G(gameState).R18CartSave) {
		_G(atds)->delControlBit(171, ATS_ACTIVE_BIT);
		_G(atds)->set_all_ats_str(111, 2, ATS_DATA);
		startSetAILWait(4, 1, ANI_FRONT);
		_G(det)->showStaticSpr(3);
	} else {
		_G(atds)->set_all_ats_str(111, 1, ATS_DATA);
	}

	_G(menu_item_vorwahl) = CUR_USE;
}

void Room23::get_cartridge() {
	_G(atds)->set_all_ats_str(111, 0, ATS_DATA);
	_G(atds)->setControlBit(171, ATS_ACTIVE_BIT);

	_G(gameState).R23Cartridge = false;
	_G(det)->hideStaticSpr(3);
}

} // namespace Rooms
} // namespace Chewy
