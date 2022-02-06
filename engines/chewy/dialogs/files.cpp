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

#include "chewy/dialogs/files.h"
#include "chewy/dialogs/options.h"
#include "chewy/events.h"
#include "chewy/file.h"
#include "chewy/global.h"

namespace Chewy {
namespace Dialogs {

static const int16 FILE_ICONS[8 * 4] = {
	14, 73, 32, 94,
	14, 96, 32, 118,
	36, 64, 310, 128,
	16, 143, 76, 193,
	78, 143, 130, 193,
	132, 143, 178, 193,
	180, 143, 228, 193,
	232, 143, 310, 193
};


int16 Files::execute(bool isInGame) {
	//int16 u_index;
	int16 key = 0;
	int16 i, j;
	int16 x[8];
	int16 y[8];
	int16 mode[9];
	int16 ret = 0;
	char *fnames;
	char *tmp;
	int16 text_off, active_slot;
	int16 rect, line;
	bool flag = false;

	taf_info *ti = mem->taf_adr(OPTION_TAF);
	g_events->clearEvents();

	room->open_handle(GBOOK, "rb", 0);
	room->load_tgp(1, &room_blk, GBOOK_TGP, 0, GBOOK);
	out->setze_zeiger(workptr);
	out->map_spr2screen(ablage[room_blk.AkAblage], 0, 0);
	out->setze_zeiger(screen0);
	room->set_ak_pal(&room_blk);
	fnames = iog->io_init(&ioptr);
	fnames += 1;
	ERROR;

	fx->blende1(workptr, screen0, pal, 150, 0, 0);
	out->setze_zeiger(workptr);
	show_cur();

	if (!modul) {
		x[0] = 1;
		x[1] = 1;
		y[0] = 0;
		y[1] = 0;

		for (i = 2; i < 8; i++) {
			x[i] = 5;
			y[i] = 5;
		}
		for (i = 0; i < 8; i++)
			mode[i] = 0;

		mode[8] = 1;
		text_off = 0;
		active_slot = 0;

		while (key != ESC && !SHOULD_QUIT) {
			// Draw the dialog background
			out->map_spr2screen(ablage[room_blk.AkAblage], 0, 0);

			// Draw the buttons at the bottom
			for (i = 28, j = 0; i < 35; i++, j++) {
				if (!mode[j])
					out->sprite_set(ti->image[i], 16 + ti->korrektur[i << 1] + x[j],
						76 + ti->korrektur[(i << 1) + 1] + y[j], 0);
				else
					out->sprite_set(ti->image[i], 16 + ti->korrektur[i << 1],
						76 + ti->korrektur[(i << 1) + 1], 0);
			}

			// Render the list of savegame slots
			tmp = fnames + (text_off * 40);
			for (i = 0; i < 6; i++, tmp += 40) {
				if (i != active_slot) {
					out->printxy(40, 68 + (i * 10), 14, 300, 0, "%2d.", text_off + i);
					out->printxy(70, 68 + (i * 10), 14, 300, 0, tmp);
				} else {
					out->box_fill(40, 68 + (i * 10), 308, 68 + 8 + (i * 10), 42);
					out->printxy(40, 68 + (i * 10), 255, 300, 0, "%2d.", text_off + i);
					out->printxy(70, 68 + (i * 10), 255, 300, 0, tmp);
				}
			}

			key = in->get_switch_code();

			if (mode[0])
				--mode[0];
			if (mode[1])
				--mode[1];
			if (mode[4])
				--mode[4];
			if (mode[4] == 1)
				key = ESC;
			if (mode[5])
				--mode[5];

			if (mode[5] == 1) {
				out->printxy(120, 138, 255, 300, 0, QUIT_MSG);
				out->back2screen(workpage);
				in->alter_kb_handler();

				key = getch();
				if (key == 'j' || key == 'J' || key == 'y' || key == 'Y' || key == 'z' || key == 'Z') {
					ret = 1;
					key = ESC;
				} else {
					key = 0;
				}

				in->neuer_kb_handler(&kbinfo);
			}

			if (mode[6])
				--mode[6];
			if (mode[6] == 1) {
				Dialogs::Options::execute(ti);
			}

			if (!flag && ((minfo.button == 1) || (key == ENTER))) {
				rect = in->maus_vector(minfo.x, minfo.y, FILE_ICONS, 8);
				flag = true;
				key = 0;

				switch (rect) {
				case 0:
					// Scroll up button
					key = CURSOR_UP + ALT;
					break;
				case 1:
					// Scroll down button
					key = CURSOR_DOWN + ALT;
					break;
				case 2:
					// Savegame list area
					line = (minfo.y - 68) / 10;
					if (line == active_slot)
						key = ENTER;
					else
						active_slot = line;
					if (active_slot > 5)
						active_slot = 5;
					break;
				case 3:
					key = F1_KEY;
					break;
				case 4:
					key = F2_KEY;
					break;
				case 5:
					key = F3_KEY;
					break;
				case 6:
					key = F4_KEY;
					break;
				case 7:
					key = F5_KEY;
					break;
				}

				minfo.button = 0;
			} else {
				flag = false;
				EVENTS_UPDATE;
			}

			switch (key) {
			case F1_KEY:
				mode[2] = 1;
				mode[3] = 0;
				mode[8] = 0;
				break;

			case F2_KEY:
				mode[2] = 0;
				mode[3] = 1;
				mode[8] = 0;
				break;

			case F3_KEY:
				mode[4] = 10;
				break;

			case F4_KEY:
				mode[5] = 10;
				break;

			case F5_KEY:
				mode[6] = 10;
				break;

			case CURSOR_UP + ALT:
				if (mode[2] || mode[3]) {
					mode[0] = 10;
					if (active_slot > 0)
						--active_slot;
					else if (text_off > 0)
						--text_off;
				}
				break;

			case CURSOR_DOWN + ALT:
				if (mode[2] || mode[3]) {
					mode[1] = 10;
					if (active_slot < 5)
						++active_slot;
					else if (text_off < (20 - 6))
						++text_off;
				}
				break;

			case ENTER:
			case ENTER + ALT:
				if (mode[3]) {
					tmp = fnames + ((text_off + active_slot) * 40);
					if (tmp[0]) {
						CurrentSong = -1;
						iog->load(text_off + active_slot,
							ioptr.save_path);
						key = ESC;
					}
				} else if (mode[2]) {
					out->back2screen(workpage);
					out->setze_zeiger(screen0);
					in->alter_kb_handler();
					tmp = fnames + ((text_off + active_slot) * 40);
					key = out->scanxy(70, 68 + (active_slot * 10),
						255, 42, 14, 0, "%36s36", tmp);
					in->neuer_kb_handler(&kbinfo);
					out->setze_zeiger(workptr);
					if (key != 27) {
						iog->save_entry(text_off + active_slot,
							ioptr.save_path);
					}
					key = 0;
				}
				break;

			case CURSOR_UP:
				cur->move(minfo.x, --minfo.y);
				break;

			case CURSOR_DOWN:
				cur->move(minfo.x, ++minfo.y);
				break;

			case CURSOR_LEFT:
				cur->move(--minfo.x, minfo.y);
				break;

			case CURSOR_RIGHT:
				cur->move(++minfo.x, minfo.y);
				break;

			default:
				break;
			}

			cur->plot_cur();
			out->back2screen(workpage);

			EVENTS_UPDATE;
		}
	}

	free(ti);

	room->open_handle(EPISODE1, "rb", 0);
	room->load_tgp(_G(spieler).PersonRoomNr[P_CHEWY], &room_blk, EPISODE1_TGP, GED_LOAD, EPISODE1);
	ERROR;

	fx_blend = BLEND1;
	room->set_ak_pal(&room_blk);

	return ret;
}

} // namespace Dialogs
} // namespace Chewy
