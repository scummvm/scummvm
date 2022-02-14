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
#include "chewy/rooms/room48.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

void Room48::entry() {
	_G(maus_links_click) = false;
	_G(spieler).scrollx = 0;
	_G(spieler).scrolly = 0;
	show_cur();
	calc_pic();
	_G(SetUpScreenFunc) = setup_func;
	_G(timer_nr)[0] = _G(room)->set_timer(255, 20);
}

bool Room48::timer(int16 t_nr, int16 ani_nr) {
	if (t_nr == _G(timer_nr)[0])
		frage();
	else
		return true;

	return false;
}

void Room48::calc_pic() {
	for (int16 i = 0; i < 2; i++) {
		_G(atds)->set_steuer_bit(312 + i, ATS_AKTIV_BIT, ATS_DATEI);
		_G(spieler).R48Auswahl[3 + i] = 0;
	}

	_G(spieler).R48Auswahl[0] = 1;
	_G(spieler).R48Auswahl[1] = 1;
	_G(spieler).R48Auswahl[2] = 1;

	if (_G(obj)->check_inventar(VCARD_INV)) {
		_G(spieler).R48Auswahl[3] = 1;
		_G(atds)->del_steuer_bit(312, ATS_AKTIV_BIT, ATS_DATEI);
		_G(det)->show_static_spr(4);
	}

	if (_G(obj)->check_inventar(KAPPE_INV)) {
		_G(spieler).R48Auswahl[4] = 1;
		_G(atds)->del_steuer_bit(313, ATS_AKTIV_BIT, ATS_DATEI);
		_G(det)->show_static_spr(4);
	}
}

void Room48::frage() {
	if (!_G(flags).AutoAniPlay) {
		_G(flags).AutoAniPlay = true;
		hide_cur();
		start_detail_wait(1, 1, ANI_VOR);
		_G(det)->show_static_spr(6);
		start_detail_wait(1, 1, ANI_RUECK);
		_G(det)->hide_static_spr(6);
		_G(uhr)->reset_timer(_G(timer_nr)[0], 0);
		show_cur();
		_G(flags).AutoAniPlay = false;
	}
}

void Room48::setup_func() {
	for (int16 i = 0; i < 5; i++)
		_G(det)->hide_static_spr(1 + i);

	if (_G(flags).ShowAtsInvTxt) {
		if (_G(menu_display) == 0) {
			_G(menu_item) = CUR_USE;
			cur_2_inventory();
			cursor_wahl(CUR_ZEIGE);
			const int16 idx = _G(det)->maus_vector(_G(minfo).x, _G(minfo).y);

			if (idx != -1) {
				if (_G(spieler).R48Auswahl[idx]) {
					_G(det)->show_static_spr(1 + idx);

					if (_G(maus_links_click)) {
						int16 r_nr;
						switch (idx) {
						case 0:
							r_nr = 45;
							break;

						case 1:
							r_nr = 49;
							break;

						case 2:
							r_nr = 54;
							break;

						case 3:
							r_nr = 57;
							break;

						case 4:
							r_nr = 56;
							break;

						default:
							r_nr = -1;
							break;
						}

						if (r_nr != -1) {
							_G(SetUpScreenFunc) = nullptr;
							_G(det)->hide_static_spr(1 + idx);
							hide_cur();
							_G(room)->set_timer_status(255, TIMER_STOP);
							_G(room)->set_timer_status(0, TIMER_STOP);
							_G(det)->stop_detail(0);
							_G(det)->del_static_ani(0);
							start_detail_wait(2, 1, ANI_VOR);
							g_engine->_sound->stopSound(0);
							_G(menu_item) = CUR_WALK;
							cursor_wahl(_G(menu_item));
							show_cur();
							_G(spieler).R48TaxiEntry = true;
							_G(maus_links_click) = false;
							set_up_screen(DO_SETUP);

							for (int16 i = 0; i < MAX_PERSON; i++) {
								if (_G(spieler).R48TaxiPerson[i]) {
									_G(spieler).PersonHide[i] = false;
									_G(spieler).R48TaxiPerson[i] = false;
								}
							}

							if (_G(spieler).PersonRoomNr[P_HOWARD] == 48) {
								_G(spieler).PersonRoomNr[P_HOWARD] = r_nr;
							}
							switch_room(r_nr);
						}
					}
				}
			}
		}
	}
}

} // namespace Rooms
} // namespace Chewy
