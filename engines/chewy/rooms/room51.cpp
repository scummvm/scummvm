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
#include "chewy/rooms/room51.h"
#include "chewy/rooms/room65.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

static const AniBlock ABLOCK37[5] = {
	{ 6, 1, ANI_VOR, ANI_WAIT, 0 },
	{ 7, 3, ANI_VOR, ANI_WAIT, 0 },
	{ 6, 1, ANI_RUECK, ANI_WAIT, 0 },
	{ 5, 1, ANI_RUECK, ANI_WAIT, 0 },
	{ 2, 1, ANI_RUECK, ANI_WAIT, 0 },
};

bool Room51::_flag;
bool Room51::_enemyFlag[2];
int16 Room51::_tmpx;
int16 Room51::_tmpy;
int Room51::_index;


void Room51::entry() {
	_G(zoom_horizont) = 140;
	_G(flags).ZoomMov = true;
	_G(zoom_mov_fak) = 4;

	if (_G(spieler).flags32_10) {
		_G(atds)->enableEvents(false);
		_G(spieler).PersonHide[P_CHEWY] = true;
		_G(spieler).PersonHide[P_HOWARD] = true;
		_G(maus_links_click) = false;
		_G(spieler).scrollx = 0;
		set_person_pos(34, 120, P_HOWARD, P_RIGHT);
		set_person_pos(234, 69, P_CHEWY, P_LEFT);
		_G(SetUpScreenFunc) = setup_func;
		_G(det)->show_static_spr(17);
		_index = 0;
		hide_cur();

		for (int i = 0; i < 2; ++i) {
			_enemyFlag[i] = false;
			_G(timer_nr)[i] = _G(room)->set_timer(i + 9, i * 2 + 6);
		}

		_G(flags).MainInput = false;
		_flag = false;

	} else {
		_G(det)->hide_static_spr(17);

		for (int i = 0; i < 2; i++)
			_G(det)->start_detail(3 + i, 1, ANI_VOR);

		if (_G(spieler).PersonRoomNr[P_HOWARD] == 51) {
			_G(spieler).ZoomXy[P_HOWARD][0] = 40;
			_G(spieler).ZoomXy[P_HOWARD][1] = 30;

			if (!_G(flags).LoadGame) {
				set_person_pos(88, 93, P_HOWARD, P_RIGHT);
			}

			if (!_G(spieler).R51FirstEntry) {
				hide_cur();
				_G(spieler).R51FirstEntry = true;
				set_person_spr(P_LEFT, P_CHEWY);
				start_aad_wait(283, -1);
				show_cur();
			}

			_G(SetUpScreenFunc) = setup_func;
			_G(spieler_mi)[P_HOWARD].Mode = true;
		}
	}
}

void Room51::xit(int16 eib_nr) {
	_G(atds)->enableEvents(true);

	if (_G(spieler).flags32_10) {
		_G(flags).MainInput = true;
		_G(spieler).PersonHide[P_CHEWY] = false;
		_G(spieler).PersonHide[P_HOWARD] = false;
		_G(spieler).PersonRoomNr[P_HOWARD] = 91;
		_G(menu_item) = CUR_WALK;
		cursor_wahl(_G(menu_item));
		show_cur();

	} else if (_G(spieler).PersonRoomNr[P_HOWARD] == 51) {
		if (eib_nr == 85) {
			_G(spieler).PersonRoomNr[P_HOWARD] = 50;
		} else {
			_G(spieler).PersonRoomNr[P_HOWARD] = 52;
		}

		_G(spieler_mi)[P_HOWARD].Mode = false;
	}
}

bool Room51::timer(int16 t_nr, int16 ani_nr) {
	if (_G(spieler).flags32_10)
		timer_action(t_nr, _G(room)->_roomTimer.ObjNr[ani_nr]);
	else
		return true;

	return false;
}

void Room51::setup_func() {
	if (_G(spieler).flags32_10) {
		_tmpx = _G(minfo).x;
		_tmpy = _G(minfo).y;
		if (_tmpx > 215)
			_tmpx = 215;
		if (_tmpy < 81)
			_tmpy = 81;

		_G(det)->set_static_pos(17, _tmpx, _tmpy, false, false);

		if ((_G(minfo).button == 1 || _G(in)->get_switch_code() == 28) && !_flag) {
			_flag = true;
			_G(det)->set_detail_pos(8, _tmpx - 20, _tmpy + 41);
			start_detail_wait(8, 1, ANI_VOR);
			_flag = false;
			++_index;

			switch (_index) {
			case 2:
				start_aad_wait(512, -1);
				_index = 1000;
				break;

			case 1006:
				start_aad_wait(513, -1);
				_index = 2000;
				break;

			case 2003:
				start_aad_wait(615, -1);
				_index = 10000;
				break;

			case 10012:
				start_aad_wait(514, -1);
				wait_show_screen(5);
				_G(flags).NoPalAfterFlc = true;
				_G(out)->setze_zeiger(nullptr);
				_G(out)->cls();
				flic_cut(115, 0);
				register_cutscene(28);
				
				switch_room(91);
				break;

			default:
				break;
			}
		}
	} else if (_G(spieler).PersonRoomNr[P_HOWARD] == 51) {
		calc_person_look();
		const int16 ch_y = _G(spieler_vector)[P_CHEWY].Xypos[1];

		int16 x, y;
		if (ch_y < 129) {
			x = 56;
			y = 106;
		} else {
			x = 31;
			y = 118;
		}

		if (_G(HowardMov) && _G(flags).ExitMov) {
			_G(SetUpScreenFunc) = nullptr;
			_G(HowardMov) = 0;
			auto_move(9, P_HOWARD);
		} else {
			go_auto_xy(x, y, P_HOWARD, ANI_GO);
		}
	}
}

int16 Room51::use_door(int16 txt_nr) {
	int16 action_ret = false;

	if (is_cur_inventar(KEY_INV)) {
		hide_cur();
		action_ret = true;

		switch (txt_nr) {
		case 329:
			auto_move(8, P_CHEWY);
			_G(SetUpScreenFunc) = nullptr;
			_G(det)->show_static_spr(0);

			if (!_G(spieler).R51HotelRoom) {
				auto_move(11, P_HOWARD);
				set_person_spr(P_LEFT, P_HOWARD);
				_G(spieler).R51HotelRoom = true;
				_G(spieler).room_e_obj[86].Attribut = AUSGANG_LINKS;
				start_aad_wait(285, -1);
				_G(atds)->set_ats_str(329, 1, ATS_DATEI);
				_G(SetUpScreenFunc) = setup_func;

			} else {
				show_cur();
				switch_room(52);
			}
			break;

		case 330:
			auto_move(9, P_CHEWY);

			if (!_G(spieler).R51KillerWeg) {
				g_engine->_sound->playSound(2, 0);
				g_engine->_sound->playSound(2);
				_G(det)->show_static_spr(1);
				start_detail_wait(2, 1, ANI_VOR);
				_G(det)->start_detail(5, 255, ANI_VOR);

				if (!_G(spieler).R52HotDogOk) {
					start_aad_wait(287, -1);
					auto_move(12, P_CHEWY);
					_G(det)->stop_detail(5);
					start_ani_block(5, ABLOCK37);
					_G(det)->hide_static_spr(1);
					g_engine->_sound->stopSound(0);
					start_aad_wait(284, -1);
				} else {
					_G(spieler).R51KillerWeg = true;
					start_aad_wait(290, -1);
					g_engine->_sound->stopSound(0);
					_G(out)->ausblenden(1);
					_G(out)->setze_zeiger(nullptr);
					_G(out)->cls();
					_G(out)->einblenden(_G(pal), 0);
					_G(flags).NoPalAfterFlc = true;
					_G(flc)->set_flic_user_function(cut_serv);
					_G(det)->show_static_spr(16);
					flic_cut(FCUT_068, CFO_MODE);
					_G(flc)->remove_flic_user_function();

					_G(det)->hide_static_spr(16);
					_G(flags).NoPalAfterFlc = false;
					_G(det)->stop_detail(5);
					_G(obj)->show_sib(SIB_AUSRUEST_R52);
					_G(obj)->calc_rsi_flip_flop(SIB_AUSRUEST_R52);
					_G(det)->hide_static_spr(1);

					_G(fx_blend) = BLEND3;
					set_up_screen(DO_SETUP);
					start_aad_wait(291, -1);
				}
			} else {
				start_aad_wait(401, -1);
			}
			break;

		case 331:
		case 334:
			auto_move((txt_nr == 331) ? 10 : 7, P_CHEWY);
			_G(out)->setze_zeiger(nullptr);
			_G(out)->cls();
			_G(flags).NoPalAfterFlc = true;
			flic_cut(114, 0);
			set_person_pos(115, 144, P_CHEWY, P_LEFT);
			_G(fx_blend) = BLEND3;
			set_up_screen(NO_SETUP);
			start_aad_wait(564, -1);
			break;

		case 332:
			auto_move(6, P_CHEWY);
			start_aad_wait(286, -1);
			break;

		case 333:
			auto_move(4, P_CHEWY);

			switch (_G(spieler).R51DoorCount) {
			case 0:
				_G(det)->show_static_spr(3);
				start_aad_wait(278, -1);
				start_detail_frame(0, 1, ANI_VOR, 3);
				start_spz(HO_BRILL_JMP, 1, ANI_VOR, P_HOWARD);
				wait_detail(0);

				_G(det)->show_static_spr(14);
				start_aad_wait(279, -1);
				++_G(spieler).R51DoorCount;
				_G(obj)->show_sib(SIB_FLASCHE_R51);
				_G(obj)->calc_rsi_flip_flop(SIB_FLASCHE_R51);
				_G(det)->hide_static_spr(3);
				break;

			case 1:
				_G(det)->show_static_spr(3);
				start_aad_wait(280, -1);
				start_detail_wait(1, 1, ANI_VOR);
				++_G(spieler).R51DoorCount;
				_G(obj)->show_sib(SIB_KAPPE_R51);
				_G(obj)->calc_rsi_flip_flop(SIB_KAPPE_R51);
				_G(det)->hide_static_spr(3);
				_G(det)->show_static_spr(15);
				break;

			default:
				start_aad_wait(281, -1);
				if (_G(spieler).PersonRoomNr[P_HOWARD] == 51)
					start_aad_wait(282, -1);
				break;
			}
			break;

		default:
			break;
		}

		show_cur();
	}

	return action_ret;
}

int16 Room51::cut_serv(int16 frame) {
	_G(det)->plot_static_details(0, 0, 16, 16);
	return 0;
}

void Room51::timer_action(int16 t_nr, int16 obj_nr) {
	if (obj_nr == 9 || obj_nr == 10) {
		if (!_enemyFlag[obj_nr - 9]) {
			_G(det)->start_detail(obj_nr, 1, ANI_VOR);
			_enemyFlag[obj_nr - 9] = true;

		} else if (!_G(det)->get_ani_status(obj_nr)) {
			_G(det)->start_detail(obj_nr, 1, ANI_GO);
			_G(det)->start_detail(obj_nr + 2, 1, ANI_VOR);
			_G(uhr)->reset_timer(t_nr, 0);
			_enemyFlag[obj_nr - 9] = false;
		}
	}
}

} // namespace Rooms
} // namespace Chewy
