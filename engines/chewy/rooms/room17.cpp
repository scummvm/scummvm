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
#include "chewy/ani_dat.h"
#include "chewy/room.h"
#include "chewy/rooms/room17.h"
#include "chewy/sound.h"

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
	if (!_G(spieler).R17EnergieOut) {
		_G(det)->start_detail(1, 255, ANI_FRONT);
		for (int i = 0; i < 3; ++i)
			_G(det)->start_detail(6 + i, 255, ANI_FRONT);
	}

	plot_seil();

	if (_G(spieler).R17GitterWeg)
		_G(det)->hideStaticSpr(5);

	if (_G(spieler).R17DoorKommand)
		_G(det)->showStaticSpr(7);

	if (_G(spieler).R17Location == 1) {
		_G(flags).ZoomMov = true;
		_G(zoom_mov_fak) = 3;
		_G(room)->set_zoom(25);
		_G(zoom_horizont) = 0;
		_G(spieler).scrollx = 0;
		_G(spieler).scrolly = 60;
		set_person_pos(242, 146, P_CHEWY, P_LEFT);
		xit();
	} else if (_G(spieler).R17Location == 3) {
		_G(room)->set_zoom(32);
		_G(zoom_horizont) = 399;
		xit();
	} else {
		_G(room)->set_zoom(15);
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
	if (_G(room)->_roomTimer.ObjNr[ani_nr] == 2 ||
		_G(room)->_roomTimer.ObjNr[ani_nr] == 3) {
		if (_G(spieler).R17EnergieOut)
			_G(uhr)->reset_timer(t_nr, 0);
		else
			return true;
	}

	return false;
}

void Room17::gedAction(int index) {
	switch (index) {
	case 0:
		door_kommando(0);
		break;

	case 1:
		door_kommando(1);
		break;

	default:
		break;
	}
}

int16 Room17::use_seil() {
	int16 action_flag = false;

	if (!_G(flags).AutoAniPlay && _G(spieler).R17Location == 1 && is_cur_inventar(SEIL_INV)) {
		action_flag = true;
		hideCur();

		del_inventar(_G(spieler).AkInvent);
		_G(flags).AutoAniPlay = true;
		autoMove(5, P_CHEWY);
		_G(spieler).PersonHide[P_CHEWY] = true;
		start_detail_wait(10, 1, ANI_FRONT);
		_G(spieler).R17Seil = true;
		_G(atds)->del_steuer_bit(139, ATS_AKTIV_BIT, ATS_DATEI);
		plot_seil();
		_G(spieler).PersonHide[P_CHEWY] = false;
		_G(flags).AutoAniPlay = false;
		start_spz(CH_TALK6, 255, false, P_CHEWY);
		start_aad_wait(119, -1);

		showCur();
	}

	return action_flag;
}

void Room17::plot_seil() {
	if (_G(spieler).R17Seil) {
		for (int16 i = 0; i < 3; i++)
			_G(det)->showStaticSpr(8 + i);
	}
}

void Room17::kletter_down() {
	autoMove(5, P_CHEWY);
	_G(det)->load_taf_seq(177, 1, nullptr);
	_G(spieler).PersonHide[P_CHEWY] = true;
	start_detail_wait(14, 1, ANI_FRONT);
	_G(flags).ZoomMov = false;
	_G(zoom_mov_fak) = 1;
	_G(spieler).ScrollyStep = 2;
	_G(room)->set_zoom(32);
	_G(spieler_mi)->Vorschub = 8;
	_G(zoom_horizont) = 399;
	_G(auto_obj) = 1;
	init_auto_obj(CHEWY_OBJ, &CHEWY_PHASEN[0][0], _G(mov_phasen)[CHEWY_OBJ].Lines, (const MovLine *)CHEWY_MPKT);
	set_person_pos(242, 350, P_CHEWY, P_LEFT);
}

void Room17::kletter_up() {
	autoMove(6, P_CHEWY);
	_G(det)->load_taf_seq(141, 4, nullptr);
	_G(spieler).PersonHide[P_CHEWY] = true;
	start_detail_wait(11, 1, ANI_FRONT);
	_G(flags).ZoomMov = true;
	_G(zoom_mov_fak) = 3;
	_G(spieler).ScrollyStep = 1;
	_G(room)->set_zoom(25);
	_G(zoom_horizont) = 0;
	_G(auto_obj) = 1;
	init_auto_obj(CHEWY_OBJ, &CHEWY_PHASEN[0][0], _G(mov_phasen)[CHEWY_OBJ].Lines, (const MovLine *)CHEWY_MPKT1);
	set_person_pos(243, 146, P_CHEWY, P_LEFT);
}

void Room17::calc_seil() {
	if (_G(spieler).R17Seil) {
		if (_G(spieler).R17Location == 2) {
			start_aad_wait(619, -1);
		} else if (!_G(flags).AutoAniPlay && !_G(spieler).inv_cur) {
			close_door();
			_G(flags).AutoAniPlay = true;
			_G(mov_phasen)[CHEWY_OBJ].AtsText = 0;
			_G(mov_phasen)[CHEWY_OBJ].Lines = 2;
			_G(mov_phasen)[CHEWY_OBJ].Repeat = 1;
			_G(mov_phasen)[CHEWY_OBJ].ZoomFak = 0;
			_G(auto_mov_obj)[CHEWY_OBJ].Id = AUTO_OBJ0;
			_G(auto_mov_vector)[CHEWY_OBJ].Delay = _G(spieler).DelaySpeed;
			_G(auto_mov_obj)[CHEWY_OBJ].Mode = true;
			hideCur();

			if (_G(spieler).R17Location == 1) {
				kletter_down();
				_G(spieler).R17Location = 3;
			} else if (_G(spieler).R17Location == 3) {
				kletter_up();
				_G(spieler).R17Location = 1;
			}

			_G(menu_item) = CUR_WALK;
			cursorChoice(_G(menu_item));
			wait_auto_obj(CHEWY_OBJ);
			showCur();
			set_person_spr(P_LEFT, P_CHEWY);
			_G(spieler).ScrollyStep = 1;
			_G(spieler).PersonHide[P_CHEWY] = false;
			_G(flags).AutoAniPlay = false;
			_G(auto_obj) = 0;
			xit();
		}
	}
}

void Room17::door_kommando(int16 mode) {
	if (!_G(flags).AutoAniPlay) {
		_G(flags).AutoAniPlay = true;

		if (!mode) {
			if (!_G(spieler).R17DoorKommand) {
				_G(spieler).room_e_obj[36].Attribut = AUSGANG_OBEN;
				_G(spieler).R17DoorKommand = true;
				start_detail_wait(4, 1, ANI_FRONT);
				stop_person(P_CHEWY);
				_G(det)->showStaticSpr(7);
			}
		} else {
			close_door();
		}

		_G(flags).AutoAniPlay = false;
		_G(atds)->set_ats_str(144, _G(spieler).R17DoorKommand, ATS_DATEI);
	}
}

void Room17::close_door() {
	if (_G(spieler).R17DoorKommand) {
		_G(spieler).room_e_obj[36].Attribut = 255;
		_G(spieler).R17DoorKommand = false;
		_G(atds)->set_ats_str(144, _G(spieler).R17DoorKommand ? 1 : 0, ATS_DATEI);
		_G(det)->hideStaticSpr(7);
		_G(det)->start_detail(4, 1, ANI_BACK);
	}
}

int16 Room17::energie_hebel() {
	int16 action_flag = false;

	hideCur();
	autoMove(7, P_CHEWY);

	if (!_G(spieler).R17HebelOk) {
		if (is_cur_inventar(BECHER_VOLL_INV)) {
			del_inventar(_G(spieler).AkInvent);
			_G(spieler).R17HebelOk = true;
			start_aad_wait(38, -1);
			action_flag = true;
		} else if (!_G(spieler).inv_cur) {
			start_aad_wait(37, -1);
			action_flag = true;
		}

	} else if (!_G(spieler).inv_cur) {
		action_flag = true;

		_G(obj)->calc_rsi_flip_flop(SIB_HEBEL_R17);
		_G(spieler).R17EnergieOut ^= 1;

		if (!_G(spieler).R17EnergieOut) {
			_G(det)->start_detail(1, 255, ANI_FRONT);

			for (int i = 0; i < 3; ++i)
				_G(det)->start_detail(i + 6, 255, ANI_FRONT);
		}

		_G(atds)->set_ats_str(142, _G(spieler).R17EnergieOut ? 1 : 0, ATS_DATEI);
		_G(atds)->set_ats_str(140, _G(spieler).R17EnergieOut ? 1 : 0, ATS_DATEI);
		g_engine->_sound->playSound(12);

		if (_G(spieler).R17EnergieOut) {
			g_engine->_sound->stopSound(0);
		} else {
			g_engine->_sound->playSound(15, 0);
		}
	}

	showCur();
	return action_flag;
}

int16 Room17::get_oel() {
	int16 action_flag = false;
	hideCur();

	if (!_G(spieler).inv_cur) {
		action_flag = true;
		autoMove(4, P_CHEWY);
		start_spz_wait(CH_EKEL, 3, false, P_CHEWY);
		start_aad_wait(60, -1);
	} else if (is_cur_inventar(BECHER_LEER_INV)) {
		action_flag = true;
		close_door();
		autoMove(4, P_CHEWY);
		_G(spieler).PersonHide[P_CHEWY] = true;
		start_detail_wait(13, 1, ANI_FRONT);
		_G(spieler).PersonHide[P_CHEWY] = false;
		del_inventar(_G(spieler).AkInvent);
		_G(obj)->addInventory(BECHER_VOLL_INV, &_G(room_blk));
		inventory_2_cur(BECHER_VOLL_INV);
	}

	showCur();
	return action_flag;
}

} // namespace Rooms
} // namespace Chewy
