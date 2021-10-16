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
#include "chewy/events.h"
#include "chewy/global.h"
#include "chewy/main.h"
#include "chewy/ngshext.h"

namespace Chewy {

int MainMenu::_selection;

void MainMenu::execute() {
#ifdef TODO_REENABLE
	// TODO: Currently disabled so it doesn't keep playing on startup
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

	while (!SHOULD_QUIT) {
		ailsnd->stop_mod();
		ailsnd->end_sound();
		SetUpScreenFunc = screenFunc;

		cursor_wahl(20);
		_selection = -1;
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

		// Wait for a selection to be made on the main menu
		do {
			animate();
			if (SHOULD_QUIT)
				return;
		} while (_selection == -1);

		switch (_selection) {
		case MM_VIEW_INTRO:
			fx->border(workpage, 100, 0, 0);
			out->setze_zeiger(workptr);
			flags.NoPalAfterFlc = true;
			flic_cut(135, 0);
			break;

		default:
			break;
		}
	}
	 
	// TODO
}

void MainMenu::screenFunc() {
	int vec = det->maus_vector(minfo.x + spieler.scrollx, minfo.y + spieler.scrolly);

	if (in->get_switch_code() == 28 || minfo.button == 1) {
		_selection = vec;
	}
}

void MainMenu::animate() {
	if (ani_timer->TimeFlag) {
		uhr->reset_timer(0, 0);
		spieler.DelaySpeed = FrameSpeed / spieler.FramesPerSecond;
		spieler_vector->Delay = spieler.DelaySpeed + spz_delay[0];
		FrameSpeed = 0;
		det->set_global_delay(spieler.DelaySpeed);
	}

	++FrameSpeed;
	out->setze_zeiger(workptr);
	out->map_spr2screen(ablage[room_blk.AkAblage],
		spieler.scrollx, spieler.scrolly);

	if (SetUpScreenFunc && !menu_display && !flags.InventMenu) {
		SetUpScreenFunc();
		out->setze_zeiger(workptr);
	}

	sprite_engine();
	kb_mov(1);
	calc_maus_txt(minfo.x, minfo.y, 1);
	cur->plot_cur();
	maus_links_click = 0;
	menu_flag = 0;
	out->setze_zeiger(nullptr);
	out->back2screen(workpage);

	g_screen->update();
	g_events->update();
}

} // namespace Chewy
