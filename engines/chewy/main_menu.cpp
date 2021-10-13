/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/textconsole.h"
#include "chewy/main_menu.h"
#include "chewy/global.h"
#include "chewy/ngshext.h"

namespace Chewy {

int MainMenu::_val1;

void MainMenu::execute() {
#ifdef TODO_REENABLE
	mem->file->select_pool_item(music_handle, EndOfPool - 17);
	mem->file->load_tmf(music_handle, (tmf_header *)Ci.MusicSlot);
	if (!modul)
		ailsnd->play_mod((tmf_header *)Ci.MusicSlot);

	flic_cut(200, 0);
	ailsnd->stop_mod();
#endif

	cur->move(152, 92);
	minfo.x = 152;
	minfo.y = 92;
	spieler.inv_cur = 0;
	menu_display = 0;
	spieler.SVal5 = 1;

	do {
		ailsnd->stop_mod();
		ailsnd->end_sound();
		SetUpScreenFunc = screenFunc;

		cursor_wahl(20);
		_val1 = -1;
		spieler.scrollx = spieler.scrolly = 0;
		spieler.PersonRoomNr[0] = 98;
		room->load_room(&room_blk, 98, &spieler);
		ERROR

		CurrentSong = -1;
		load_room_music(98);
		fx->border(workpage, 100, 0, 0);

		out->set_palette(pal);
		spieler.PersonHide[0] = 1;
		show_cur();

		do {
			proc5();
		} while (_val1 == -1);

	} while (1);
	 
	// TODO
}

void MainMenu::screenFunc() {

}

void MainMenu::proc5() {

}

} // namespace Chewy
