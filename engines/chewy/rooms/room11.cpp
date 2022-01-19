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
#include "chewy/rooms/room11.h"
#include "chewy/rooms/room12.h"

namespace Chewy {
namespace Rooms {

AniBlock ABLOCK17[2] = {
	{ 8, 1, ANI_VOR, ANI_WAIT, 0 },
	{ 9, 255, ANI_VOR, ANI_GO, 0 },
};

AniBlock ABLOCK18[2] = {
	{ 7, 1, ANI_VOR, ANI_WAIT, 0 },
	{ 7, 1, ANI_RUECK, ANI_WAIT, 0 },
};


void Room11::entry() {
	_G(zoom_horizont) = 80;
	flags.ZoomMov = true;
	_G(zoom_mov_fak) = 2;

	if (_G(spieler).R12ChewyBork) {
		if (!_G(spieler).R11DoorRightB) {
			obj->calc_rsi_flip_flop(SIB_TKNOPF2_R11);
			_G(spieler).R11DoorRightB = exit_flip_flop(5, 22, -1, 98, -1, -1,
				AUSGANG_OBEN, -1, (int16)_G(spieler).R11DoorRightB);
			obj->calc_all_static_detail();
		}

		obj->hide_sib(SIB_TKNOPF1_R11);
		obj->hide_sib(SIB_SCHLITZ_R11);
		obj->hide_sib(SIB_TKNOPF2_R11);
		obj->hide_sib(SIB_TKNOPF3_R11);
		_G(spieler).room_e_obj[20].Attribut = 255;
		_G(spieler).room_e_obj[21].Attribut = 255;
		atds->del_steuer_bit(121, ATS_AKTIV_BIT, ATS_DATEI);

		if (!flags.LoadGame) {
			start_ani_block(2, ABLOCK17);
			auto_move(8, P_CHEWY);
			start_aad_wait(31, -1);
			det->stop_detail(9);
		}
		det->show_static_spr(8);
		if (!flags.LoadGame)
			auto_move(6, P_CHEWY);
		_G(timer_nr)[0] = room->set_timer(255, 10);
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
	if (!flags.AutoAniPlay) {
		flags.AutoAniPlay = true;
		det->hide_static_spr(8);
		start_ani_block(2, ABLOCK18);
		uhr->reset_timer(_G(timer_nr)[0], 0);
		det->show_static_spr(8);
		flags.AutoAniPlay = false;
	}
}

void Room11::talk_debug() {
	if (_G(spieler).R12ChewyBork) {
		flags.AutoAniPlay = true;
		auto_move(8, P_CHEWY);
		start_ads_wait(5);
		menu_item = CUR_WALK;
		cursor_wahl(CUR_WALK);

		flags.AutoAniPlay = false;
	}
}

void Room11::chewy_bo_use() {
	if (_G(spieler).R12ChewyBork) {
		hide_cur();
		flags.AutoAniPlay = true;

		stop_person(P_CHEWY);
		det->hide_static_spr(8);
		start_ani_block(2, ABLOCK17);
		start_aad_wait(32, -1);
		det->stop_detail(9);
		det->show_static_spr(8);
		auto_move(6, P_CHEWY);

		flags.AutoAniPlay = false;
		show_cur();
	}
}

int16 Room11::scanner() {
	int16 action_flag = false;

	if (!_G(spieler).R12ChewyBork) {
		auto_move(7, P_CHEWY);

		if (!_G(spieler).R11CardOk) {
			action_flag = true;
			start_aad_wait(13, -1);
		} else {
			if (is_cur_inventar(BORK_INV)) {
				hide_cur();
				action_flag = true;

				flc->set_custom_user_function(Room12::cut_serv);
				start_aad(105, 0);
				flic_cut(FCUT_011, CFO_MODE);
				flc->remove_custom_user_function();
				_G(spieler).R11TerminalOk = true;
				cur_2_inventory();
				menu_item = CUR_TALK;
				cursor_wahl(menu_item);
				start_aad_wait(12, -1);
				load_ads_dia(3);
				show_cur();
			} else if (!_G(spieler).inv_cur) {
				if (!_G(spieler).R11TerminalOk) {
					action_flag = true;
					flc->set_custom_user_function(cut_serv);
					flic_cut(FCUT_010, CFO_MODE);
					flc->remove_custom_user_function();
					start_aad_wait(20, -1);
				} else {
					action_flag = true;
					start_aad_wait(12, -1);
					menu_item = CUR_TALK;
					cursor_wahl(menu_item);
					load_ads_dia(3);
				}
			}
		}
	}

	return action_flag;
}

void Room11::get_card() {
	if (_G(spieler).R11CardOk) {
		_G(spieler).R11CardOk = false;
		obj->add_inventar(_G(spieler).R11IdCardNr, &room_blk);

		_G(spieler).AkInvent = _G(spieler).R11IdCardNr;
		cursor_wahl(CUR_AK_INVENT);
		cursor_wahl(CUR_AK_INVENT);
		det->stop_detail(0);
		atds->set_ats_str(83, TXT_MARK_LOOK, 0, ATS_DATEI);
		atds->set_ats_str(84, TXT_MARK_LOOK, 0, ATS_DATEI);
	}
}

void Room11::put_card() {
	if (is_cur_inventar(RED_CARD_INV) || is_cur_inventar(YEL_CARD_INV)) {
		_G(spieler).R11IdCardNr = _G(spieler).AkInvent;
		del_inventar(_G(spieler).R11IdCardNr);
		det->start_detail(0, 255, ANI_VOR);
		atds->set_ats_str(83, TXT_MARK_LOOK, 1, ATS_DATEI);
		atds->set_ats_str(84, TXT_MARK_LOOK, 1, ATS_DATEI);
		_G(spieler).R11CardOk = true;

		if (!_G(spieler).R11TerminalOk)
			start_aad_wait(16, -1);
	}
}

int16 Room11::cut_serv(int16 frame) {
	if (_G(spieler).R11DoorRightF)
		det->plot_static_details(0, 0, 0, 0);
	if (_G(spieler).R11DoorRightB)
		det->plot_static_details(0, 0, 6, 6);
	if (_G(spieler).R45MagOk)
		det->plot_static_details(0, 0, 7, 7);

	return 0;
}

} // namespace Rooms
} // namespace Chewy
