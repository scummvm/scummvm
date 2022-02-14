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
#include "chewy/rooms/room24.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

static const uint8 KRISTALL_SPR[3][3] = {
	{ 14, 20, 13 },
	{ 20, 13, 14 },
	{ 13, 14, 20 },
};

void Room24::entry() {
	flags.MainInput = false;
	_G(spieler).PersonHide[P_CHEWY] = true;
	set_person_pos(0, 0, P_CHEWY, -1);
	_G(spieler).scrollx = 0;
	_G(spieler).scrolly = 0;
	curblk.sprite = room_blk.DetImage;
	curani.ani_anf = 7;
	curani.ani_end = 10;
	_G(menu_item) = CUR_USER;
	cursor_wahl(CUR_USER);

	if (_G(spieler).R16F5Exit)
		_G(det)->show_static_spr(10);
	else
		_G(det)->hide_static_spr(10);

	calc_hebel_spr();
	calc_animation(255);

	for (int16 i = 0; i < 3; i++) {
		if (KRISTALL_SPR[i][_G(spieler).R24Hebel[i]] == 20)
			_G(det)->start_detail(5 + i * 4, 255, ANI_RUECK);
	}
}

void Room24::xit() {
	_G(spieler).PersonHide[P_CHEWY] = false;
	_G(menu_item) = CUR_WALK;
	cursor_wahl(_G(menu_item));
	set_person_pos(263, 144, P_CHEWY, -1);
	_G(spieler).scrollx = 88;
	_G(maus_links_click) = false;
	flags.MainInput = true;
}

void Room24::use_hebel(int16 txt_nr) {
	if (!_G(spieler).R24Hebel[txt_nr - 161] ||
		_G(spieler).R24Hebel[txt_nr - 161] == 2) {
		_G(spieler).R24Hebel[txt_nr - 161] = 1;
		_G(spieler).R24HebelDir[txt_nr - 161] ^= 1;
	} else {
		if (_G(spieler).R24HebelDir[txt_nr - 161])
			_G(spieler).R24Hebel[txt_nr - 161] = 0;
		else
			_G(spieler).R24Hebel[txt_nr - 161] = 2;
	}
	calc_hebel_spr();
	calc_animation(txt_nr - 161);

	if (_G(spieler).R24Hebel[0] == 1 && _G(spieler).R24Hebel[1] == 0 && _G(spieler).R24Hebel[2] == 2) {
		_G(spieler).R16F5Exit = true;
		g_engine->_sound->playSound(1, 0);
		g_engine->_sound->stopSound(1);
		_G(det)->start_detail(1, 1, ANI_VOR);
		_G(det)->show_static_spr(10);
		_G(atds)->set_ats_str(164, TXT_MARK_NAME, 1, ATS_DATEI);

	} else if (_G(spieler).R16F5Exit) {
		_G(det)->hide_static_spr(10);
		g_engine->_sound->playSound(1, 1);
		g_engine->_sound->stopSound(0);
		_G(det)->start_detail(1, 1, ANI_RUECK);
		_G(spieler).R16F5Exit = false;
		_G(atds)->set_ats_str(164, TXT_MARK_NAME, 0, ATS_DATEI);
	}
}

void Room24::calc_hebel_spr() {
	if (!_G(spieler).R24FirstEntry) {
		_G(spieler).R24FirstEntry = true;
		_G(spieler).R24Hebel[0] = 2;
		_G(spieler).R24HebelDir[0] = 0;
		_G(spieler).R24Hebel[1] = 1;
		_G(spieler).R24HebelDir[1] = 0;
		_G(spieler).R24Hebel[2] = 0;
		_G(spieler).R24HebelDir[2] = 1;
	}

	for (int16 i = 0; i < 3; i++) {
		for (int16 j = 0; j < 3; j++)
			_G(det)->hide_static_spr(1 + j + i * 3);

		_G(det)->show_static_spr(1 + _G(spieler).R24Hebel[i] + i * 3);
		_G(atds)->set_ats_str(166 + i, TXT_MARK_NAME, _G(spieler).R24Hebel[i], ATS_DATEI);
	}
}

void Room24::calc_animation(int16 kristall_nr) {
	if (kristall_nr != 255) {
		hide_cur();

		if (KRISTALL_SPR[kristall_nr][_G(spieler).R24Hebel[kristall_nr]] == 20) {
			int16 ani_nr = _G(spieler).R24KristallLast[kristall_nr] == 13 ? 7 : 8;
			g_engine->_sound->playSound(ani_nr + kristall_nr * 4, 0);
			g_engine->_sound->stopSound(0);
			_G(det)->hide_static_spr(_G(spieler).R24KristallLast[kristall_nr] + kristall_nr * 2);
			start_detail_wait(ani_nr + kristall_nr * 4, 1, ANI_RUECK);
			start_detail_wait(6 + kristall_nr * 4, 1, ANI_RUECK);
			_G(det)->start_detail(5 + kristall_nr * 4, 255, ANI_RUECK);

		} else if (_G(spieler).R24KristallLast[kristall_nr] == 20) {
			int16 ani_nr = KRISTALL_SPR[kristall_nr][_G(spieler).R24Hebel[kristall_nr]] == 13 ? 7 : 8;
			g_engine->_sound->stopSound(0);
			g_engine->_sound->playSound(5 + ani_nr + kristall_nr * 4, 0);
			_G(det)->stop_detail(5 + kristall_nr * 4);
			start_detail_wait(6 + kristall_nr * 4, 1, ANI_VOR);
			start_detail_wait(ani_nr + kristall_nr * 4, 1, ANI_VOR);
		}

		show_cur();
	}

	for (int16 i = 0; i < 6; i++)
		_G(det)->hide_static_spr(13 + i);

	for (int16 i = 0; i < 3; i++) {
		_G(det)->show_static_spr(KRISTALL_SPR[i][_G(spieler).R24Hebel[i]] + i * 2);
		_G(spieler).R24KristallLast[i] = KRISTALL_SPR[i][_G(spieler).R24Hebel[i]];
	}
}

} // namespace Rooms
} // namespace Chewy
