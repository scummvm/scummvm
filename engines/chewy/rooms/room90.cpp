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
#include "chewy/rooms/room90.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

int Room90::_delay;

void Room90::entry(int16 eib_nr) {
	_G(spieler).ScrollxStep = 2;
	_G(spieler_mi)[P_HOWARD].Mode = true;
	_delay = 0;
	_G(SetUpScreenFunc) = setup_func;
	_G(spieler).PersonRoomNr[P_HOWARD] = 90;
	_G(zoom_horizont) = 150;
	flags.ZoomMov = true;
	_G(zoom_mov_fak) = 4;
	_G(spieler).ZoomXy[P_HOWARD][0] = 60;
	_G(spieler).ZoomXy[P_HOWARD][1] = 90;

	if (!flags.LoadGame) {
		switch (eib_nr) {
		case 78:
		case 136:
			_G(spieler).scrollx = 0;
			set_person_pos(32, 137, P_CHEWY, P_RIGHT);
			set_person_pos(20, 121, P_HOWARD, P_RIGHT);
			break;
		case 137:
			_G(spieler).scrollx = 176;
			set_person_pos(411, 146, P_CHEWY, P_RIGHT);
			set_person_pos(440, 132, P_HOWARD, P_RIGHT);
			break;
		default:
			break;
		}
	}

	hide_cur();
	_G(HowardMov) = 1;

	if (_G(spieler).flags34_40 && !_G(spieler).flags33_40) {
		_G(det)->set_detail_pos(12, 329, 15);
		_G(det)->start_detail(12, 255, false);
		g_engine->_sound->playSound(12, 0);
		g_engine->_sound->playSound(12);
	}

	if (_G(spieler).flags33_40 && !_G(spieler).flags34_10) {
		_G(det)->show_static_spr(4);
		_G(det)->show_static_spr(5);
	}

	if (!_G(spieler).flags33_10) {
		flags.ZoomMov = false;
		start_aad_wait(498, -1);
		go_auto_xy(-20, 121, P_HOWARD, ANI_GO);
		auto_move(1, P_CHEWY);
		start_detail_frame(2, 1, ANI_VOR, 3);
		_G(det)->start_detail(0, 1, false);
		start_detail_wait(1, 1, ANI_VOR);
		_G(spieler).flags33_10 = true;
		_G(spieler).PersonRoomNr[P_HOWARD] = 91;
		switch_room(91);

	} else if (!_G(spieler).flags33_20) {
		start_aad_wait(499, -1);
		_G(spieler).flags33_20 = true;

	} else if (_G(spieler).flags34_20) {
		set_person_pos(411, 146, P_CHEWY, P_RIGHT);
		set_person_pos(440, 132, P_HOWARD, P_RIGHT);
		_G(spieler).scrollx = 176;
		_G(spieler).flags34_20 = false;
		start_aad_wait(520, -1);
	}

	_G(HowardMov) = 0;
	show_cur();
}

void Room90::xit(int16 eib_nr) {
	_G(spieler).ScrollxStep = 1;
	if (eib_nr == 134)
		_G(spieler).PersonRoomNr[P_HOWARD] = 91;
	else if (eib_nr == 135)
		_G(spieler).PersonRoomNr[P_HOWARD] = 56;
}

void Room90::setup_func() {
	if (_G(menu_display))
		return;

	if (_delay)
		--_delay;
	else {
		_delay = _G(spieler).DelaySpeed - 1;
		for (int i = 0; i < 8; ++i) {
			if (_G(spieler).r90_Array187030[i][0] == 1) {
				int destY = _G(Adi)[i + 4].y;
				int destX = _G(Adi)[i + 4].x;
				if (i >= 5) {
					destX += _G(spieler).r90_Array187030[i][1];
					if (destX > 540) {
						_G(spieler).r90_Array187030[i][0] = 0;
						_G(det)->stop_detail(i + 4);
					}
				} else {
					destX -= _G(spieler).r90_Array187030[i][1];
					if (destX < -30) {
						_G(spieler).r90_Array187030[i][0] = 0;
						_G(det)->stop_detail(i + 4);
					}
				}
				_G(det)->set_detail_pos(i + 4, destX, destY);
			} else {
				++_G(spieler).r90_Array187030[i][2];
				if (_G(spieler).r90_Array187030[i][2] < _G(spieler).r90_Array187030[i][3])
					continue;
				_G(spieler).r90_Array187030[i][2] = 0;
				_G(spieler).r90_Array187030[i][0] = 1;
				int destX;
				if (i >= 6)
					destX = 0;
				else
					destX = 500;

				_G(det)->set_detail_pos(i + 4, destX, _G(Adi)[i + 4].y);
				
				if (!_G(spieler).flags34_40)
					_G(det)->start_detail(i + 4, 255, false);
			}
		}
	}

	if (_G(spieler).PersonRoomNr[P_HOWARD] != 90 || _G(HowardMov) == 1)
		return;

	calc_person_look();
	int xyPos = _G(spieler_vector)[P_CHEWY].Xypos[0];
	int destX;

	if (xyPos > 400)
		destX = 440;
	else if (xyPos > 240)
		destX = 304;
	else if (xyPos > 95)
		destX = 176;
	else
		destX = 18;

	if (_G(HowardMov) == 2)
		destX = 18;

	go_auto_xy(destX, 132, P_HOWARD, ANI_GO);
}

void Room90::proc2() {
	if (_G(spieler).flags33_80 || flags.AutoAniPlay)
		return;

	flags.AutoAniPlay = true;
	int diaNr = -1;

	switch (_G(spieler).PersonRoomNr[0]) {
	case 46:
		diaNr = 496;
		break;
	case 56:
		diaNr = 506;
		break;
	case 90:
		diaNr = 509;
		break;
	case 91:
		diaNr = 508;
		break;
	case 94:
		diaNr = 572;
		break;
	case 95:
		diaNr = 573;
		break;
	case 96:
		diaNr = 574;
		break;
	case 97:
		diaNr = 575;
		break;
	default:
		break;
	}

	if (diaNr != -1) {
		set_up_screen(DO_SETUP);
		if (_G(spieler).ChewyAni == CHEWY_ANI7) {
			start_spz_wait(66, 1, false, P_CHEWY);
			start_spz(67, 255, false, P_CHEWY);
		} else {
			start_spz_wait(64, 1, false, P_CHEWY);
			start_spz(65, 255, false, P_CHEWY);
		}

		hide_cur();
		start_aad_wait(-1, diaNr);
		show_cur();
	}
	flags.AutoAniPlay = false;
}

int Room90::getHubcaps() {
	if (_G(spieler).inv_cur)
		return 0;

	hide_cur();
	auto_move(3, P_CHEWY);
	start_aad_wait(515, -1);
	start_spz_wait(14, 1, false, P_CHEWY);
	start_spz_wait(43, 1, false, P_CHEWY);
	load_chewy_taf(CHEWY_ANI7);

	while (_G(spieler_vector)[P_HOWARD].Xypos[0] != 176) {
		set_up_screen(DO_SETUP);
		SHOULD_QUIT_RETURN0;
	}

	start_spz_wait(47, 1, false, P_HOWARD);
	_G(spieler).mi[1] = 2;
	start_aad_wait(516, -1);
	_G(spieler).flags34_40 = true;
	_G(spieler).flags35_1 = true;
	auto_move(4, P_CHEWY);
	auto_scroll(176, 0);
	_G(det)->set_detail_pos(12, 495, 15);
	_G(det)->start_detail(12, 255, false);
	g_engine->_sound->playSound(12, 0);
	g_engine->_sound->playSound(12);

	int destX = 495;

	while (destX > 329) {
		destX -= 2;
		_G(det)->set_detail_pos(12, destX, 15);
		set_up_screen(DO_SETUP);
		SHOULD_QUIT_RETURN0;
	}

	start_aad_wait(517, -1);
	_G(spieler).flags35_1 = false;
	_G(atds)->set_steuer_bit(517, ATS_AKTIV_BIT, ATS_DATEI);
	_G(atds)->del_steuer_bit(519, ATS_AKTIV_BIT, ATS_DATEI);
	show_cur();

	return 1;
}	

int Room90::shootControlUnit() {
	if (!is_cur_inventar(111))
		return 0;

	hide_cur();
	del_inventar(_G(spieler).AkInvent);
	_G(HowardMov) = 2;
	flags.ZoomMov = false;
	auto_move(5, P_CHEWY);
	_G(spieler_mi)[P_CHEWY].Mode = true;
	go_auto_xy(232, 142, P_CHEWY, ANI_WAIT);
	flags.NoScroll = true;
	auto_scroll(176, 0);
	start_detail_wait(13, 1, ANI_VOR);
	_G(flc)->set_custom_user_function(Room90::proc5);
	flags.NoPalAfterFlc = true;
	flic_cut(107, CFO_MODE);
	_G(flc)->remove_custom_user_function();
	_G(spieler).scrollx = 0;
	set_person_pos(76, 145, P_CHEWY, P_LEFT);
	_G(maus_links_click) = false;
	_G(det)->show_static_spr(4);
	_G(det)->show_static_spr(5);
	set_up_screen(NO_SETUP);
	_G(fx_blend) = BLEND3;
	_G(spieler_mi)[P_CHEWY].Mode = false;
	flags.NoScroll = false;
	_G(HowardMov) = 0;
	_G(spieler).flags33_40 = true;
	_G(det)->stop_detail(12);
	_G(atds)->set_steuer_bit(519, ATS_AKTIV_BIT, ATS_DATEI);
	_G(atds)->del_steuer_bit(520, ATS_AKTIV_BIT, ATS_DATEI);
	flags.ZoomMov = true;

	show_cur();
	return 1;
}

int16 Room90::proc5(int16 key) {
	_G(det)->plot_static_details(_G(spieler).scrollx, 0, 3, 3);
	
	return 0;
}

int Room90::useSurimyOnWreck() {
	if (!is_cur_inventar(18))
		return 0;

	_G(spieler).flags34_10 = true;
	hide_cur();
	cur_2_inventory();
	auto_move(6, P_CHEWY);
	_G(maus_links_click) = false;
	_G(out)->setze_zeiger(nullptr);
	_G(out)->cls();
	flic_cut(109, CFO_MODE);
	register_cutscene(29);

	_G(det)->hide_static_spr(4);
	_G(det)->hide_static_spr(5);
	_G(spieler).scrollx = 176;
	start_spz(45, 255, false, P_CHEWY);
	start_aad_wait(519, -1);
	_G(atds)->set_steuer_bit(520, ATS_AKTIV_BIT, ATS_DATEI);

	show_cur();
	return 1;
}

} // namespace Rooms
} // namespace Chewy
