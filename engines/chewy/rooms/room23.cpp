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
#include "chewy/global.h"
#include "chewy/room.h"
#include "chewy/rooms/room23.h"

namespace Chewy {
namespace Rooms {

static const AniBlock ABLOCK15[4] = {
	{ 0, 2, ANI_VOR, ANI_GO, 0 },
	{ 1, 255, ANI_VOR, ANI_GO, 0 },
	{ 2, 255, ANI_VOR, ANI_GO, 0 },
	{ 3, 255, ANI_VOR, ANI_GO, 0 },
};


void Room23::entry() {
	_G(spieler).PersonHide[P_CHEWY] = true;
	set_person_pos(135, 69, P_CHEWY, -1);
}

void Room23::cockpit() {
	_G(spieler).scrollx = 0;
	_G(spieler).scrolly = 0;
	_G(maus_links_click) = false;
	switch_room(23);

	if (_G(spieler).R23Cartridge)
		det->show_static_spr(3);
	else
		det->hide_static_spr(3);
}

int16 Room23::start_gleiter() {
	int16 action_flag = false;

	if (!_G(spieler).inv_cur) {
		action_flag = true;
		if (!_G(spieler).R23FluxoFlex)
			start_aad_wait(23, -1);
		else {
			bool start_ok = true;

			if (_G(spieler).R23GleiterExit == 16) {
				if (!_G(spieler).R16F5Exit) {
					start_ok = false;
					start_aad_wait(35, -1);
				} else if ((!_G(spieler).R23Cartridge) || (!_G(spieler).R18CartSave)) {
					start_ok = false;
					start_aad_wait(41, -1);
				} else if (!_G(spieler).R17EnergieOut) {
					start_ok = false;
					start_aad_wait(300, -1);
				}
			}

			if (start_ok) {
				hide_cur();
				start_ani_block(4, ABLOCK15);
				wait_show_screen(30);

				for (int16 i = 0; i < 4; i++)
					det->stop_detail(i);

				if (_G(spieler).R23GleiterExit == 14) {
					out->setze_zeiger(nullptr);
					out->cls();
					flic_cut(FCUT_012, CFO_MODE);
					out->cls();
					_G(spieler).R23GleiterExit = 16;
					set_person_pos(126, 110, P_CHEWY, P_RIGHT);

					switch_room(_G(spieler).R23GleiterExit);
					start_spz_wait(CH_WONDER1, 1, false, P_CHEWY);
					start_spz(CH_TALK2, 255, ANI_VOR, P_CHEWY);

					_G(spieler).DelaySpeed = 10;
					start_aad_wait(59, -1);
					stop_person(P_CHEWY);
					_G(maus_links_click) = false;

				} else if (_G(spieler).R23GleiterExit == 16) {
					out->setze_zeiger(nullptr);
					out->cls();
					flic_cut(FCUT_019_01, CFO_MODE);
					_G(spieler).R23GleiterExit = 25;

					cur_2_inventory();
					remove_inventory(5);
					remove_inventory(6);
					remove_inventory(4);
					remove_inventory(15);
					remove_inventory(16);
					switch_room(_G(spieler).R23GleiterExit);
				}

				show_cur();
			}
		}
	}

	return action_flag;
}

void Room23::use_cartridge() {
	del_inventar(_G(spieler).AkInvent);
	_G(spieler).R23Cartridge = true;

	if (_G(spieler).R18CartSave) {
		atds->del_steuer_bit(171, ATS_AKTIV_BIT, ATS_DATEI);
		atds->set_ats_str(111, 1, ATS_DATEI);
		start_detail_wait(4, 1, ANI_VOR);
		det->show_static_spr(3);
	} else {
		atds->set_ats_str(111, 1, ATS_DATEI);
	}

	menu_item_vorwahl = CUR_USE;
}

void Room23::get_cartridge() {
	atds->set_ats_str(111, 0, ATS_DATEI);
	atds->set_steuer_bit(171, ATS_AKTIV_BIT, ATS_DATEI);

	_G(spieler).R23Cartridge = false;
	det->hide_static_spr(3);
}

} // namespace Rooms
} // namespace Chewy
