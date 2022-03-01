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
	_G(spieler).PersonHide[P_CHEWY] = true;
	setPersonPos(135, 69, P_CHEWY, -1);
}

void Room23::cockpit() {
	_G(spieler).scrollx = 0;
	_G(spieler).scrolly = 0;
	_G(maus_links_click) = false;
	switch_room(23);

	if (!_G(spieler).R23Cartridge || !_G(spieler).R25GleiteLoesch)
		_G(det)->hideStaticSpr(3);
	else
		_G(det)->showStaticSpr(3);
}

int16 Room23::start_gleiter() {
	int16 action_flag = false;

	if (!_G(spieler).inv_cur) {
		action_flag = true;
		if (!_G(spieler).R23FluxoFlex)
			startAadWait(23);
		else {
			bool start_ok = true;

			if (_G(spieler).R23GleiterExit == 16) {
				if (!_G(spieler).R16F5Exit) {
					start_ok = false;
					startAadWait(35);
				} else if (!_G(spieler).R23Cartridge || !_G(spieler).R18CartSave) {
					start_ok = false;
					startAadWait(41);
				} else if (!_G(spieler).R17EnergieOut) {
					start_ok = false;
					startAadWait(300);
				}
			}

			if (start_ok) {
				hideCur();
				startAniBlock(4, ABLOCK15);
				waitShowScreen(30);

				for (int16 i = 0; i < 4; i++)
					_G(det)->stop_detail(i);

				if (_G(spieler).R23GleiterExit == 14) {
					_G(out)->setPointer(nullptr);
					_G(out)->cls();
					_G(flags).NoPalAfterFlc = true;
					flic_cut(FCUT_011);
					register_cutscene(7);
					_G(out)->cls();
					_G(spieler).R23GleiterExit = 16;
					setPersonPos(126, 110, P_CHEWY, P_RIGHT);

					switch_room(_G(spieler).R23GleiterExit);
					start_spz_wait(CH_WONDER1, 2, false, P_CHEWY);
					start_spz(CH_TALK2, 255, ANI_FRONT, P_CHEWY);

					_G(spieler).DelaySpeed = 10;
					startAadWait(59);
					stopPerson(P_CHEWY);
					_G(maus_links_click) = false;

				} else if (_G(spieler).R23GleiterExit == 16) {
					_G(out)->setPointer(nullptr);
					_G(out)->cls();
					flic_cut(FCUT_SPACECHASE_18);
					_G(spieler).R23GleiterExit = 25;
					register_cutscene(9);

					cur_2_inventory();
					remove_inventory(5);
					remove_inventory(6);
					remove_inventory(4);
					remove_inventory(15);
					remove_inventory(16);
					switch_room(_G(spieler).R23GleiterExit);
				}

				showCur();
			}
		}
	}

	return action_flag;
}

void Room23::use_cartridge() {
	del_inventar(_G(spieler).AkInvent);
	_G(spieler).R23Cartridge = true;

	if (_G(spieler).R18CartSave) {
		_G(atds)->del_steuer_bit(171, ATS_AKTIV_BIT, ATS_DATA);
		_G(atds)->set_ats_str(111, 2, ATS_DATA);
		startSetailWait(4, 1, ANI_FRONT);
		_G(det)->showStaticSpr(3);
	} else {
		_G(atds)->set_ats_str(111, 1, ATS_DATA);
	}

	_G(menu_item_vorwahl) = CUR_USE;
}

void Room23::get_cartridge() {
	_G(atds)->set_ats_str(111, 0, ATS_DATA);
	_G(atds)->set_steuer_bit(171, ATS_AKTIV_BIT, ATS_DATA);

	_G(spieler).R23Cartridge = false;
	_G(det)->hideStaticSpr(3);
}

} // namespace Rooms
} // namespace Chewy
