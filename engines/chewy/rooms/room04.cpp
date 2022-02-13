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
#include "chewy/rooms/room04.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

#define HAND_NORMAL 68
#define HAND_CLICK 69
#define RAHMEN_ROT 70
#define RAHMEN_GELB 71

int16 Room4::sonde_comp() {
	static const int16 CUR_POS[3][2] = {
		{  83, 106 },
		{ 136, 103 },
		{ 188, 101 }
	};
	static const int16 CONSOLE[3][4] = {
		{  82, 158, 143, 199 },
		{ 150, 159, 194, 193 },
		{ 201, 154, 262, 193 }
	};

	cur_2_inventory();
	_G(spieler).PersonHide[P_CHEWY] = true;
	_G(cur_display) = false;
	switch_room(4);
	_G(cur_display) = true;
	int16 ende = 0;
	curblk.sprite = room_blk.DetImage;
	int16 cur_x = 1;
	int16 spr_nr = RAHMEN_ROT;
	_G(cur)->move(160, 160);

	start_aad(46);
	while (!ende) {
		maus_action();
		if (_G(maus_links_click)) {
			switch (_G(in)->maus_vector(minfo.x + 17, minfo.y + 7, &CONSOLE[0][0], 3)) {
			case 0:
				if (cur_x > 0)
					--cur_x;
				else
					cur_x = 2;
				g_engine->_sound->playSound(0, 1);
				break;

			case 1:
				ende = 1;
				spr_nr = RAHMEN_GELB;
				g_engine->_sound->playSound(0, 0);
				break;

			case 2:
				if (cur_x < 2)
					++cur_x;
				else
					cur_x = 0;
				g_engine->_sound->playSound(0, 2);
				break;

			default:
				break;
			}
		}

		spr_info[0].Image = room_blk.DetImage[spr_nr];
		spr_info[0].ZEbene = 0;
		spr_info[0].X = CUR_POS[cur_x][0];
		spr_info[0].Y = CUR_POS[cur_x][1];

		if (minfo.button == 1 || kbinfo.key_code == Common::KEYCODE_RETURN) {
			curani.ani_anf = HAND_CLICK;
			curani.ani_end = HAND_CLICK;
		} else {
			curani.ani_anf = HAND_NORMAL;
			curani.ani_end = HAND_NORMAL;
		}
		cursor_wahl(CUR_USER);
		_G(spieler).CurHoehe = 16;
		_G(in)->rectangle(0, 123, 320 - _G(spieler).CurBreite, 194);

		if (minfo.y < 124)
			minfo.y = 123;

		set_up_screen(DO_SETUP);
		SHOULD_QUIT_RETURN0;
	}
	g_events->delay(500);

	clear_prog_ani();
	_G(spieler).PersonHide[P_CHEWY] = false;
	menu_item = CUR_WALK;
	cursor_wahl(menu_item);

	spieler_vector[P_CHEWY].DelayCount = 0;
	_G(maus_links_click) = false;
	minfo.button = 0;
	_G(spieler).PersonRoomNr[P_CHEWY] = 3;
	room->load_room(&room_blk, 3, &_G(spieler));
	set_person_pos(110, 139, P_CHEWY, P_LEFT);

	fx_blend = BLEND1;
	atds->stop_aad();

	return cur_x;
}

} // namespace Rooms
} // namespace Chewy
