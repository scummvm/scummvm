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
#include "chewy/rooms/room17.h"

namespace Chewy {
namespace Rooms {

#undef CHEWY_OBJ
#define CHEWY_OBJ 0

static const int16 CHEWY_PHASEN[4][2] = {
	{ 0, 0 },
	{ 0, 0 },
	{ 141, 144 },
	{ 177, 177 }
};

static const MovLine CHEWY_MPKT[2] = {
	{ { 241, 150, 0 }, 3, 8 },
	{ { 241, 350, 0 }, 3, 8 }
};

static const MovLine CHEWY_MPKT1[2] = {
	{ { 243, 334, 0 }, 2, 6 },
	{ { 243, 150, 0 }, 2, 6 }
};

void Room17::entry() {
	plot_seil();
	if (_G(spieler).R17GitterWeg)
		det->hide_static_spr(5);
	if (_G(spieler).R17DoorKommand)
		det->show_static_spr(7);
	if (_G(spieler).R17Location == 1) {
		flags.ZoomMov = true;
		_G(zoom_mov_fak) = 3;
		room->set_zoom(25);
		_G(zoom_horizont) = 0;
		_G(spieler).scrollx = 0;
		_G(spieler).scrolly = 60;
		set_person_pos(242, 146, P_CHEWY, P_LEFT);
		xit();
	} else if (_G(spieler).R17Location == 3) {
		room->set_zoom(32);
		_G(zoom_horizont) = 399;
		xit();
	} else {
		room->set_zoom(15);
		_G(zoom_horizont) = 0;
		_G(spieler).room_e_obj[36].Attribut = 255;
		_G(spieler).room_e_obj[38].Attribut = 255;
		if (_G(spieler).R6DoorLeftF)
			_G(spieler).room_e_obj[39].Attribut = AUSGANG_RECHTS;
		else
			_G(spieler).room_e_obj[39].Attribut = 255;
		if (_G(spieler).R18DoorBruecke)
			_G(spieler).room_e_obj[35].Attribut = AUSGANG_LINKS;
		else
			_G(spieler).room_e_obj[35].Attribut = 255;
	}
}

void Room17::xit() {
	if (_G(spieler).R17DoorKommand)
		_G(spieler).room_e_obj[36].Attribut = AUSGANG_OBEN;
	else
		_G(spieler).room_e_obj[36].Attribut = 255;
	_G(spieler).room_e_obj[35].Attribut = 255;
	_G(spieler).room_e_obj[39].Attribut = 255;
	_G(spieler).room_e_obj[38].Attribut = AUSGANG_OBEN;
}

bool Room17::timer(int16 t_nr, int16 ani_nr) {
	if (room->room_timer.ObjNr[ani_nr] == 2 ||
		room->room_timer.ObjNr[ani_nr] == 3) {
		if (_G(spieler).R17EnergieOut)
			uhr->reset_timer(t_nr, 0);
		else
			return true;
	}

	return false;
}

int16 Room17::use_seil() {
	int16 action_flag = false;
	if (!flags.AutoAniPlay) {
		if (_G(spieler).R17Location == 1) {
			if (is_cur_inventar(SEIL_INV)) {
				action_flag = true;
				del_inventar(_G(spieler).AkInvent);
				flags.AutoAniPlay = true;
				auto_move(5, P_CHEWY);
				_G(spieler).PersonHide[P_CHEWY] = true;
				start_detail_wait(10, 1, ANI_VOR);
				_G(spieler).R17Seil = true;
				atds->del_steuer_bit(139, ATS_AKTIV_BIT, ATS_DATEI);
				plot_seil();
				_G(spieler).PersonHide[P_CHEWY] = false;
				flags.AutoAniPlay = false;
				start_aad(119, -1);
			}
		}
	}
	return action_flag;
}

void Room17::plot_seil() {
	int16 i;
	if (_G(spieler).R17Seil) {
		for (i = 0; i < 3; i++)
			det->show_static_spr(8 + i);
	}
}

void Room17::kletter_down() {
	auto_move(5, P_CHEWY);
	det->load_taf_seq(177, 1, 0);
	_G(spieler).PersonHide[P_CHEWY] = true;
	start_detail_wait(14, 1, ANI_VOR);
	flags.ZoomMov = false;
	_G(zoom_mov_fak) = 1;
	_G(spieler).ScrollyStep = 2;
	room->set_zoom(32);
	_G(zoom_horizont) = 399;
	_G(auto_obj) = 1;
	init_auto_obj(CHEWY_OBJ, &CHEWY_PHASEN[0][0], mov_phasen[CHEWY_OBJ].Lines,
		(const MovLine *)CHEWY_MPKT);
	set_person_pos(242, 350, P_CHEWY, P_LEFT);
}

void Room17::kletter_up() {
	auto_move(6, P_CHEWY);
	det->load_taf_seq(141, 4, 0);
	_G(spieler).PersonHide[P_CHEWY] = true;
	start_detail_wait(11, 1, ANI_VOR);
	flags.ZoomMov = true;
	_G(zoom_mov_fak) = 3;
	_G(spieler).ScrollyStep = 1;
	room->set_zoom(25);
	_G(zoom_horizont) = 0;
	_G(auto_obj) = 1;
	init_auto_obj(CHEWY_OBJ, &CHEWY_PHASEN[0][0], mov_phasen[CHEWY_OBJ].Lines,
		(const MovLine *)CHEWY_MPKT1);
	set_person_pos(243, 146, P_CHEWY, P_LEFT);
}

void Room17::calc_seil() {
	if (_G(spieler).R17Seil) {
		if (_G(spieler).R17Location != 2) {
			if (!flags.AutoAniPlay) {
				if (!_G(spieler).inv_cur) {
					close_door();
					flags.AutoAniPlay = true;
					mov_phasen[CHEWY_OBJ].AtsText = 0;
					mov_phasen[CHEWY_OBJ].Lines = 2;
					mov_phasen[CHEWY_OBJ].Repeat = 1;
					mov_phasen[CHEWY_OBJ].ZoomFak = 0;
					auto_mov_obj[CHEWY_OBJ].Id = AUTO_OBJ0;
					auto_mov_vector[CHEWY_OBJ].Delay = _G(spieler).DelaySpeed;
					auto_mov_obj[CHEWY_OBJ].Mode = 1;
					if (_G(spieler).R17Location == 1) {
						kletter_down();
						_G(spieler).R17Location = 3;
					} else if (_G(spieler).R17Location == 3) {
						kletter_up();
						_G(spieler).R17Location = 1;
					}
					menu_item = CUR_WALK;
					cursor_wahl(menu_item);
					wait_auto_obj(CHEWY_OBJ);
					set_person_spr(P_LEFT, P_CHEWY);
					_G(spieler).ScrollyStep = 1;
					_G(spieler).PersonHide[P_CHEWY] = false;
					flags.AutoAniPlay = false;
					_G(auto_obj) = 0;
					xit();
				}
			}
		}
	}
}

void Room17::door_kommando(int16 mode) {
	if (!flags.AutoAniPlay) {
		flags.AutoAniPlay = true;
		if (!mode) {
			if (!_G(spieler).R17DoorKommand) {
				_G(spieler).room_e_obj[36].Attribut = AUSGANG_OBEN;
				_G(spieler).R17DoorKommand = true;
				start_detail_wait(4, 1, ANI_VOR);
				stop_person(P_CHEWY);
				det->show_static_spr(7);
			}
		} else {
			close_door();
		}
		flags.AutoAniPlay = false;
		atds->set_ats_str(144, _G(spieler).R17DoorKommand, ATS_DATEI);
	}
}

void Room17::close_door() {
	if (_G(spieler).R17DoorKommand) {
		_G(spieler).room_e_obj[36].Attribut = 255;
		_G(spieler).R17DoorKommand = false;
		det->hide_static_spr(7);
		det->start_detail(4, 1, ANI_RUECK);
	}
}

int16 Room17::energie_hebel() {
	int16 dia_nr;
	int16 action_flag = false;
	auto_move(7, P_CHEWY);
	if (!_G(spieler).R17HebelOk) {
		action_flag = true;
		if (is_cur_inventar(BECHER_VOLL_INV)) {
			_G(spieler).R17HebelOk = true;
			dia_nr = 38;
		} else
			dia_nr = 37;
		start_aad_wait(dia_nr, -1);
	} else if (!_G(spieler).inv_cur) {
		action_flag = true;
		obj->calc_rsi_flip_flop(SIB_HEBEL_R17);
		_G(spieler).R17EnergieOut ^= 1;
		atds->set_ats_str(142, TXT_MARK_LOOK, _G(spieler).R17EnergieOut, ATS_DATEI);
		det->play_sound(12, 0);
		if (_G(spieler).R17EnergieOut)
			det->disable_sound(15, 0);
		else {
			det->enable_sound(15, 0);
			det->play_sound(15, 0);
		}
	}
	return action_flag;
}

int16 Room17::get_oel() {
	int16 action_flag = false;
	if (!_G(spieler).inv_cur) {
		action_flag = true;
		auto_move(4, P_CHEWY);
		start_spz(CH_EKEL, 3, ANI_VOR, P_CHEWY);
		start_aad_wait(60, -1);
	} else if (is_cur_inventar(BECHER_LEER_INV)) {
		action_flag = true;
		close_door();
		auto_move(4, P_CHEWY);
		_G(spieler).PersonHide[P_CHEWY] = true;
		start_detail_wait(13, 1, ANI_VOR);
		_G(spieler).PersonHide[P_CHEWY] = false;
		del_inventar(_G(spieler).AkInvent);
		obj->add_inventar(BECHER_VOLL_INV, &room_blk);
		inventory_2_cur(BECHER_VOLL_INV);
	}

	return action_flag;
}

} // namespace Rooms
} // namespace Chewy
