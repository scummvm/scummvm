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
#include "chewy/ani_dat.h"
#include "chewy/room.h"
#include "chewy/rooms/room32.h"

namespace Chewy {
namespace Rooms {

void Room32::entry() {
	if (_G(spieler).R32HowardWeg)
		det->hide_static_spr(0);
	if (!_G(spieler).R32Script && _G(spieler).R32UseSchreib) {
		det->show_static_spr(5);
	}
}

int16 Room32::use_howard() {
	int16 dia_nr = 0;
	int16 ani_nr = 0;
	int16 action_flag = false;
	hide_cur();
	if (is_cur_inventar(TRICHTER_INV)) {
		if (_G(spieler).R33MunterGet) {
			auto_move(1, P_CHEWY);
			cur_2_inventory();
			obj->del_inventar(MUNTER_INV, &room_blk);
			del_invent_slot(MUNTER_INV);
			flic_cut(FCUT_043, FLC_MODE);
			atds->set_steuer_bit(230, ATS_AKTIV_BIT, ATS_DATEI);
			start_spz(CH_TALK12, 255, ANI_VOR, P_CHEWY);
			start_aad_wait(75, -1);
			wait_show_screen(5);
			auto_move(5, P_CHEWY);
			wait_show_screen(10);
			start_spz(CH_TALK12, 255, ANI_VOR, P_CHEWY);
			start_aad_wait(125, -1);
			wait_show_screen(10);
			det->hide_static_spr(0);
			start_detail_frame(0, 1, ANI_VOR, 9);
			start_detail_wait(1, 1, ANI_RUECK);
			det->show_static_spr(7);
			det->show_static_spr(6);
			wait_show_screen(20);
			det->hide_static_spr(7);
			start_detail_wait(1, 1, ANI_VOR);
			start_spz(CH_TALK3, 255, ANI_VOR, P_CHEWY);
			ani_nr = CH_TALK3;
			dia_nr = 164;
			_G(spieler).R32HowardWeg = true;
			_G(spieler).R39HowardDa = true;
		} else {
			ani_nr = CH_TALK12;
			dia_nr = 73;
		}
	} else if (is_cur_inventar(MUNTER_INV)) {
		ani_nr = CH_TALK12;
		dia_nr = 74;
	}
	if (dia_nr) {
		start_spz(ani_nr, 255, ANI_VOR, P_CHEWY);
		start_aad_wait(dia_nr, -1);
		action_flag = true;
	}
	show_cur();
	return action_flag;
}

void Room32::use_schreibmaschine() {
	int16 dia_nr = -1;
	int16 ani_nr = -1;

	hide_cur();
	if (_G(spieler).R32HowardWeg) {
		if (_G(spieler).inv_cur) {
			switch (_G(spieler).AkInvent) {
			case PAPIER_INV:
				auto_move(2, P_CHEWY);
				_G(spieler).R32PapierOk = true;
				start_spz_wait(CH_LGET_O, 1, ANI_VOR, P_CHEWY);
				del_inventar(_G(spieler).AkInvent);
				atds->set_ats_str(231, TXT_MARK_LOOK, 1, ATS_DATEI);
				ani_nr = CH_TALK3;
				dia_nr = 86;
				break;

			case CYB_KRONE_INV:
				if (!_G(spieler).R32UseSchreib) {
					if (!_G(spieler).R32PapierOk) {
						ani_nr = CH_TALK12;
						dia_nr = 87;
					} else {
						auto_move(3, P_CHEWY);
						_G(spieler).R32UseSchreib = true;
						cur_2_inventory();
						flic_cut(FCUT_044, FLC_MODE);
						det->show_static_spr(5);
						atds->set_ats_str(203, 1, ATS_DATEI);
						ani_nr = CH_TALK3;
						dia_nr = 88;
						atds->set_ats_str(231, TXT_MARK_LOOK, 0, ATS_DATEI);
					}
				}
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
	start_spz(ani_nr, 255, ANI_VOR, P_CHEWY);
	start_aad_wait(dia_nr, -1);
	show_cur();
}

int16 Room32::get_script() {
	int16 action_flag = false;
	if (!_G(spieler).inv_cur) {
		if (!_G(spieler).R32Script && _G(spieler).R32UseSchreib) {
			action_flag = true;
			_G(spieler).R32Script = true;
			auto_move(4, P_CHEWY);
			invent_2_slot(MANUSKRIPT_INV);
			start_spz_wait(CH_LGET_U, 1, ANI_VOR, P_CHEWY);
			det->hide_static_spr(5);
			atds->set_ats_str(203, 0, ATS_DATEI);
			start_spz(CH_TALK3, 1, ANI_VOR, P_CHEWY);
			start_aad_wait(91, -1);
		}
	}
	return action_flag;
}

} // namespace Rooms
} // namespace Chewy
