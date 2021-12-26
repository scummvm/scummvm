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
#include "chewy/global.h"
#include "chewy/debug.h"

namespace Chewy {

void look_debug_action(int16 key_nr) {
	int16 ende;
#define TXT_MARK_NAME 0
#define TXT_MARK_LOOK 1
#define TXT_MARK_USE 2
#define TXT_MARK_WALK 3
#define TXT_MARK_TALK 4
	switch (key_nr) {
	case I_KEY:
	case I_KEY + ALT:
		ende = 0;
		while (!ende) {
			switch (in->get_switch_code()) {
			case ENTER:
			case ESC:
				ende = 1;
				break;

			case ALT + ENTER:
//				look_icm_action();
				break;

			}
		}
		break;

	case F1_KEY + ALT:
		warning("Removed : get memory");
		break;

	case F2_KEY +ALT:
//		get_maus_speed();
		break;

	case F3_KEY +ALT:
//		room->cache_test(&room_blk);
		break;

	case F4_KEY +ALT:
//		taf_sequence_test();
		break;

	case T_KEY + ALT:
//		timer_test();
		break;

	case H_KEY + ALT:
//		draw_help_win(1);
		break;

	case H_KEY:
//		draw_help_win(2);
		break;

	case F_KEY + ALT:
		show_frame ^= 1;
		cur->wait_taste_los(0);
		break;

	}
}

void test_load(void *handle, taf_info *Tt, int16 anz) {
	warning("STUB - test_load");
#if 0
	int16 ende;
	int16 ende1;
	int16 i;
	int16 tmp1, tmp2;
	int16 str_anz;
	int16 x, y;
	int16 abfrage;
	int16 spr;
	int16 action;
	int16 *Cxy;
	char c[] = {
		"Erstes Sprite : \0"
		"Anzahl Sprites : \0"
	};
	str_anz = 2;
	ende = 0;
	out->setze_zeiger(0);
	while (!ende) {
		out->printxy(0, 0, 255, 0, 0, "F1 Bereich laden\0");
		out->printxy(0, 10, 255, 0, 0, "F2 Sprites anzeigen\0");
		out->printxy(0, 20, 255, 0, 0, "F3 Bereich löschen\0");
		switch (in->get_switch_code()) {
		case ESC :
			ende = 1;
			break;

		case F1_KEY:
			x = 160;
			y = 100;
			auto_menu(&x, &y, str_anz, 10, c, 0);
			in ->alter_kb_handler();
			tmp1 = 0;
			tmp2 = 0;
			for (i = 0; i < str_anz; i++)
				out->printxy(x, y + i * 10, 14, 300, scr_width, txt->str_pos(c, i));
			abfrage = out->scanxy(x + 96, y, 14, 60, 6, scr_width, "%[0-9]3d\0", &tmp1);
			abfrage = out->scanxy(x + 96, y + 10, 14, 60, 6, scr_width, "%[0-9]3d\0", &tmp2);
			if (abfrage != 27) {
				det->load_taf_seq(handle, tmp1, tmp2, Tt);
				ERROR
			}
			out->map_spr2screen(ablage[room_blk.AkAblage], 0, 0);
			in ->neuer_kb_handler(&kbinfo);
			break;

		case F2_KEY:
			x = 160;
			y = 100;
			auto_menu(&x, &y, 1, 10, c, 0);
			in ->alter_kb_handler();
			spr = 0;
			for (i = 0; i < 1; i++)
				out->printxy(x, y + i * 10, 14, 300, scr_width, txt->str_pos(c, i));
			abfrage = out->scanxy(x + 96, y, 14, 60, 6, scr_width, "%[0-9]3d\0", &spr);
			in ->neuer_kb_handler(&kbinfo);
			out->map_spr2screen(ablage[room_blk.AkAblage], 0, 0);
			if (abfrage != 27) {
				ende1 = 0;
				action = 1;
				while (!ende1) {
					switch (kbinfo.scan_code) {
					case ESC:
						ende1 = 1;
						break;

					case PLUS:
						if (spr >= anz - 1)
							spr = 0;
						else
							++spr;
						action = 1;
						break;

					case MINUS:
						if (spr > 0)
							--spr;
						else
							spr = anz - 1;
						action = 1;
						break;

					}
					if (action) {
						kbinfo.scan_code = Common::KEYCODE_INVALID;
						action = 0;
						out->map_spr2screen(ablage[room_blk.AkAblage], 0, 0);
						if (Tt->image[spr] != 0) {
							Cxy = Tt->korrektur + (spr << 1);
							out->sprite_set(Tt->image[spr], 100 + Cxy[0], 60 + Cxy[1], 0);
						}
						out->printxy(0, 0, 255, 0, 0, "Sprite Nr:%d\0", spr);
					}
				}
			}
			in ->neuer_kb_handler(&kbinfo);
			break;

		case F3_KEY:
			x = 160;
			y = 100;
			auto_menu(&x, &y, str_anz, 10, c, 0);
			in ->alter_kb_handler();
			tmp1 = 0;
			tmp2 = 0;
			for (i = 0; i < str_anz; i++)
				out->printxy(x, y + i * 10, 14, 300, scr_width, txt->str_pos(c, i));
			abfrage = out->scanxy(x + 96, y, 14, 60, 6, scr_width, "%[0-9]3d\0", &tmp1);
			abfrage = out->scanxy(x + 96, y + 10, 14, 60, 6, scr_width, "%[0-9]3d\0", &tmp2);
			if (abfrage != 27) {
				det->del_taf_tbl(tmp1, tmp2, Tt);
				ERROR
			}
			out->map_spr2screen(ablage[room_blk.AkAblage], 0, 0);
			in ->neuer_kb_handler(&kbinfo);
			break;

		}
	}
#endif
}

} // namespace Chewy
