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
#include "chewy/rooms/room93.h"

namespace Chewy {
namespace Rooms {

void Room93::entry() {
	if (flags.LoadGame)
		return;

	_G(spieler).scrollx = 0;
	hide_person();
	start_detail_wait(3, 0, ANI_GO);
	det->set_static_ani(0, -1);
	start_aad_wait(616, -1);
	det->del_static_ani(0);
	start_detail_wait(3, 1, ANI_VOR);
	det->set_static_ani(1, -1);
	start_ads_wait(27);

	if (!_G(spieler).flags37_40) {
		det->del_static_ani(1);
		hide_cur();
		start_detail_wait(3, 1, ANI_GO);
		det->set_static_ani(0, -1);
		start_aad_wait(549, -1);
		det->del_static_ani(0);
		start_detail_wait(3, 1, ANI_VOR);
		start_detail_wait(6, 1, ANI_VOR);
		det->set_static_ani(7, -1);
		start_aad_wait(550, -1);
		det->del_static_ani(7);
		start_detail_wait(6, 1, ANI_GO);
		start_detail_wait(2, 1, ANI_VOR);
		set_up_screen(DO_SETUP);
		show_cur();
	}

	_G(spieler).flags35_40 = true;
	show_person();
	switch_room(94);
}

void Room93::xit() {
	_G(spieler).PersonRoomNr[P_HOWARD] = 94;
	_G(spieler).scrollx = _G(spieler).r94Scrollx;
	menu_item = CUR_WALK;
	cursor_wahl(CUR_WALK);
}

} // namespace Rooms
} // namespace Chewy
