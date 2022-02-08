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

#include "common/system.h"
#include "chewy/chewy.h"
#include "chewy/events.h"
#include "chewy/file.h"
#include "chewy/io_game.h"

namespace Chewy {

extern int16 scr_w;

io_game::io_game(mcga_grafik *iout, maus *iin, cursor *curp) {
	out = iout;
	in = iin;
	cur = curp;
}

io_game::~io_game() {
}

char file_find_g[20][USER_NAME + 4];

short file_xy_g[7][4] = {
	{140, 6, 156, 51},
	{140, 55, 156, 100},
	{163, 6, 241, 18},
	{163, 22, 241, 34},
	{163, 38, 241, 50},
	{163, 54, 241, 66},
	{ 6, 6, 133, 97}
};

byte code_tbl_g[6] = {72, 80, 59, 60, 61, 62};

char auf_tbl_g[3][3] = { {FSTRING8 },
	{FSTRING9 },
	{FSTRING10},
};
char ab_tbl_g[4][3] = { {FSTRING11},
	{FSTRING12},
	{FSTRING13},
	{FSTRING14},
};

int16 io_game::io_menu(iog_init *iostruc) {
	int16 max_scroll = 20;
	int16 i, j, y;
	char ende = 0, mouse_f = 1;
	char a = 0;
	int16 y1 = 106;
	int16 ret = 0;
	char io_flag = 0;

	io = iostruc;
	inzeig = in->get_in_zeiger();
	cur->hide_cur();
	if (inzeig->minfo)
		minfo = inzeig->minfo;
	if (!inzeig->kbinfo)
		in->neuer_kb_handler(kbinfo);
	else
		kbinfo = inzeig->kbinfo;
	scr_width = scr_w << 2;
	d_klick = DOPPEL_KLICK;
	int16 cur_y1 = io->popy + 4;
	int16 cur_y = io->popy + 8;
	plot_io();
	schalter_aus();
	for (i = 0; i < 20; i++)
		file_find_g[i][0] = 0;
	int16 scroll_flag = 0;
	i = 0;

	int16 auswahl;
	if (!io->key_nr) {
		out->box(io->popx + 161, io->popy + 4, io->popx + 243, io->popy + 21, io->m_col[1]);
		auswahl = 1;
	} else {
		out->box(io->popx + 161, io->popy + 4 + ((io->key_nr - 1) * 16),
		          io->popx + 243, io->popy + 21 + ((io->key_nr - 1) * 16), io->m_col[1]);
		auswahl = (int16)io->key_nr;
		cur_y1 = (auswahl - 1) * 16 + io->popy + 4;
	}
	cur->show_cur();
	switch_code = 1;
	cur->wait_taste_los(true);
	kbinfo->key_code = '\0';
	kbinfo->scan_code = Common::KEYCODE_INVALID;
	switch ((int16)io->key_nr) {
	case 0:
		cur->hide_cur();
		get_savegame_files();
		i = 0;
		cur_y = io->popy + 8;
		scroll_flag = 0;
		if (file_find_g[0][0] != 0) {
			if (strlen(&file_find_g[0][1]) >= 17)
				out->
				printnxy(io->popx + 28, io->popy + 10, io->m_col[5], 300, 17, scr_width, &file_find_g[0][1]
				        );
			else
				out->
				printxy(io->popx + 28, io->popy + 10, io->m_col[5], 300, scr_width, &file_find_g[0][1]);
		}
		plot_dir_liste(io->popy + 18, i + 1);
		cur->show_cur();
		break;
	case 1:
		in->hot_key = 59;
		break;
	case 2:
		in->hot_key = 60;
		break;
	case 3:
		in->hot_key = 61;
		break;
	case 4:
		in->hot_key = 62;
		break;
	}
	switch_code = 0;
	while (ende == 0) {
		if (mouse_f != 0) {
			mouse_f = 0;
			kbinfo->key_code = '\0';
		}
		cur->plot_cur();

		if (minfo->button == 1) {
			int16 x = minfo->x;
			y = minfo->y;

			j = in->maus_vector(x - io->popx, y - io->popy, (int16 *)file_xy_g, 7);

			if (j != -1) {
				mouse_f = 1;
				if (j < 6)

					in->hot_key = code_tbl_g[j];
				if ((j == 6) && (y >= io->popy + 8)) {
					y -= io->popy + 8;
					y /= 10;

					if (y < (max_scroll)) {

						if (y + scroll_flag != i) {
							mouse_links_los = false;
							cur_move = true;
							cur->hide_cur();

							if (io_flag != 0)
								unmark_eintrag(cur_y, i);
							dklick_start = g_system->getMillis(); //clock();
							i = y;
							cur_y = i * 10 + io->popy + 8;
							i += scroll_flag;
							if (io_flag != 0)
								mark_eintrag(cur_y, i);
							cur->show_cur();
						} else if (mouse_links_los) {
							mouse_links_los = false;
							dklick_end = g_system->getMillis(); //clock();
							if ((dklick_end - dklick_start) / 1000 < d_klick)
								kbinfo->key_code = 28;
							else
								dklick_start = g_system->getMillis(); //clock();
						}
					}
				} else {
					if (j > 2) {
						y -= io->popy + 8;
						y /= 15;
						if (auswahl != (y + 1)) {
							cur->hide_cur();
							out->box(io->popx + 161, cur_y1, io->popx + 243, cur_y1 + 17, io->m_col[5]);
							auswahl = y + 1;
							cur_y1 = (auswahl - 1) * 16 + io->popy + 4;

							out->box(io->popx + 161, cur_y1, io->popx + 243, cur_y1 + 17, io->m_col[1]);
							cur->show_cur();
						}
					}
				}
			}

		}

		switch_code = in->get_switch_code();
		switch (switch_code) {

		case 72:
			cur->hide_cur();
			if (!io_flag) {
				if (auswahl > 1) {
					out->box(io->popx + 161, cur_y1, io->popx + 243, cur_y1 + 17, io->m_col[5]);
					--auswahl;
					cur_y1 -= 16;
					out->box(io->popx + 161, cur_y1, io->popx + 243, cur_y1 + 17, io->m_col[1]);
				}
			} else {

				if (a != 2) {
					out->pop_box(io->popx + 140, io->popy + 55, io->popx + 156, io->popy + y1 - 6,
					              io->m_col[0], io->m_col[1], io->m_col[5]);
					plot_ab_txt(io->m_col[1]);
					a = 2;

					out->pop_box(io->popx + 140, io->popy + 6, io->popx + 156, io->popy + 51, io->m_col[1], io->m_col[0], io->m_col[3]);
					plot_auf_txt(io->m_col[2]);
				}
				if (cur_y > io->popy + 8) {
					unmark_eintrag(cur_y, i);
					--i;
					cur_y -= 10;
					mark_eintrag(cur_y, i);
				} else if (i > 0) {
					--i;
					--scroll_flag;
					mark_eintrag(cur_y, i);
					plot_dir_liste(io->popy + 18, i + 1);
				}
			}
			cur->show_cur();
			break;

		case 80:
			cur->hide_cur();
			if (!io_flag) {
				if (auswahl < 4) {
					out->box(io->popx + 161, cur_y1, io->popx + 243, cur_y1 + 17, io->m_col[5]);
					++auswahl;
					cur_y1 += 16;
					out->box(io->popx + 161, cur_y1, io->popx + 243, cur_y1 + 17, io->m_col[1]);
				}
			} else {

				if (a != 1) {
					out->pop_box(io->popx + 140, io->popy + 6, io->popx + 156, io->popy + 51, io->m_col[0], io->m_col[1], io->m_col[5]);
					plot_auf_txt(io->m_col[1]);
					a = 1;

					out->pop_box(io->popx + 140, io->popy + 55, io->popx + 156, io->popy + y1 - 6, io->m_col[1], io->m_col[0], io->m_col[3]);
					plot_ab_txt(io->m_col[2]);
				}
				if ((cur_y < io->popy + 6 + 9 * 9) && (i < (max_scroll - 1))) {
					unmark_eintrag(cur_y, i);
					++i;
					cur_y += 10;
					mark_eintrag(cur_y, i);
				} else if (i < (max_scroll - 1)) {
					++i;
					++scroll_flag;
					plot_dir_liste(io->popy + 8, i - 8);
					mark_eintrag(cur_y, i);
				}
			}
			cur->show_cur();
			break;

		case 59:
			if (io->f1) {
				io_flag = 1;
				cur->hide_cur();
				schalter_aus();
				out->pop_box(io->popx + 163, io->popy + 6, io->popx + 241, io->popy + 18, io->m_col[1], io->m_col[0], io->m_col[3]);
				out->printxy(io->popx + 167, io->popy + 9, io->m_col[2], 300, scr_width, FSTRING1);

				get_savegame_files();
				i = 0;
				cur_y = io->popy + 8;
				scroll_flag = 0;
				plot_dir_liste(io->popy + 18, i + 1);
				mark_eintrag(cur_y, i);
				cur->show_cur();
				cur->wait_taste_los(true);
			}
			break;

		case 60:
			if (io->f2) {
				io_flag = 2;
				cur->hide_cur();
				schalter_aus();
				out->pop_box(io->popx + 163, io->popy + 6 + 1 * 16, io->popx + 241, io->popy + 18 + 1 * 16, io->m_col[1], io->m_col[0], io->m_col[3]);
				out->printxy(io->popx + 167, io->popy + 25, io->m_col[2], 300, scr_width, FSTRING2);

				get_savegame_files();
				i = 0;
				cur_y = io->popy + 8;
				scroll_flag = 0;
				plot_dir_liste(io->popy + 18, i + 1);
				mark_eintrag(cur_y, i);
				cur->show_cur();
				cur->wait_taste_los(true);
			}
			break;

		case 61:
			if (io->f3) {
				io_flag = 0;
				cur->hide_cur();
				schalter_aus();
				unmark_eintrag(cur_y, i);
				out->pop_box(io->popx + 163, io->popy + 6 + 2 * 16, io->popx + 241, io->popy + 18 + 2 * 16, io->m_col[1], io->m_col[0], io->m_col[3]);
				out->printxy(io->popx + 167, io->popy + 41, io->m_col[2], 300, scr_width, FSTRING3);

				out->printxy(io->popx + 167, io->popy + 75, io->m_col[1], 300, scr_width, FSTRING5);
				out->printxy(io->popx + 167, io->popy + 85, io->m_col[1], 300, scr_width, FSTRING6);
				cur->show_cur();
				ende = 0;
				kbinfo->key_code = '\0';
				kbinfo->scan_code = Common::KEYCODE_INVALID;
				while (!ende) {
					cur->plot_cur();
					SHOULD_QUIT_RETURN0;

					switch_code = in->get_switch_code();
					if ((kbinfo->scan_code == 36) ||
					        (kbinfo->scan_code == 44) ||
					        (kbinfo->scan_code == 21)) {
						ende = 1;
						ret = IOG_END;
					} else if ((switch_code == 1) ||
					           (kbinfo->scan_code == 49))
						break;
				}
				cur->wait_taste_los(true);
				cur->hide_cur();
				out->box_fill(io->popx + 167, io->popy + 70, io->popx + 244, io->popy + 100, io->m_col[5]);
				out->pop_box(io->popx + 163, io->popy + 6 + 2 * 16, io->popx + 241, io->popy + 18 + 2 * 16, io->m_col[0], io->m_col[1], io->m_col[5]);
				out->printxy(io->popx + 167, io->popy + 41, io->m_col[1], 300, scr_width, FSTRING3);

				cur->show_cur();
			}
			break;

		case 62:
			if (io->f4) {
				io_flag = 0;
				ende = 1;
				ret = IOG_BACK;
			}
			break;

		case 28:
			if (io_flag != 0) {

				if (io_flag == 1) {
					io_flag = 0;
					save(cur_y, i, io->save_path);
					cur->hide_cur();
					get_savegame_files();
					mark_eintrag(cur_y, i);
					schalter_aus();

					cur->show_cur();
					ret = IOG_SAVE;
					ende = 1;
				}

				if ((io_flag == 2) && (file_find_g[i][0] == 1)) {

					load(i, io->save_path);

					ret = IOG_LOAD;
					ende = 1;
					io_flag = 0;
				}
			} else
				in->hot_key = auswahl + 58;
			g_events->delay(200);
			kbinfo->key_code = '\0';
			break;

		case 1:
			if (io_flag != 0) {
				io_flag = 0;
				cur->hide_cur();
				schalter_aus();
				unmark_eintrag(cur_y, i);
				cur->show_cur();
				cur->wait_taste_los(true);
				kbinfo->key_code = '\0';
			} else {
				ende = 1;
				ret = IOG_BACK;
			}
			break;

		default:
			if (a != 0) {
				a = 0;
				cur->hide_cur();
				out->pop_box(io->popx + 140, io->popy + 6, io->popx + 156, io->popy + 51, io->m_col[0], io->m_col[1], io->m_col[5]);
				plot_auf_txt(io->m_col[1]);
				out->pop_box(io->popx + 140, io->popy + 55, io->popx + 156, io->popy + y1 - 6, io->m_col[0], io->m_col[1], io->m_col[5]);
				plot_ab_txt(io->m_col[1]);
				cur->show_cur();
			}

		}
	}
	cur->wait_taste_los(true);
	cur->hide_cur();
	return ret;
}

void io_game::mark_eintrag(int16 y, int16 nr) {
	out->pop_box(io->popx + 8, y, io->popx + 131, y + 10,
	              io->m_col[1], io->m_col[0], io->m_col[4]);
	out->printxy(io->popx + 10, y + 2, io->m_col[2], 300, scr_width, "%d.", nr + 1);
	if (file_find_g[nr][0] != 0) {
		if (strlen(&file_find_g[nr][1]) >= 17)
			out->printnxy(io->popx + 28, y + 2, io->m_col[2], 300, 17,
			               scr_width, &file_find_g[nr][1]);
		else
			out->printxy(io->popx + 28, y + 2, io->m_col[2], 300,
			              scr_width, &file_find_g[nr][1]);
		out->box_fill(io->popx + 8, io->popy + 106, io->popx + 244, io->popy + 117,
		               io->m_col[5]);
		print_shad(io->popx + 8, io->popy + 106, io->m_col[0], 300, io->m_col[1],
		           scr_width, &file_find_g[nr][1]);
	} else {
		out->move(io->popx + 8, io->popy + 106);
		out->box_fill(io->popx + 8, io->popy + 106, io->popx + 244, io->popy + 117,
		               io->m_col[5]);

	}
}

void io_game::unmark_eintrag(int16 y, int16 nr) {
	out->pop_box(io->popx + 8, y, io->popx + 131, y + 10, io->m_col[3],
	              io->m_col[3], io->m_col[3]);
	out->printxy(io->popx + 10, y + 2, io->m_col[5], 300, scr_width, "%d.", nr + 1);
	if (file_find_g[nr][0] != 0) {
		if (strlen(&file_find_g[nr][1]) >= 17)
			out->printnxy(io->popx + 28, y + 2, io->m_col[5], 300, 17,
			               scr_width, &file_find_g[nr][1]);
		else
			out->printxy(io->popx + 28, y + 2, io->m_col[5], 300,
			              scr_width, &file_find_g[nr][1]);
	}
}

void io_game::plot_dir_liste(int16 cur_y, int16 start) {
	for (int16 i = start; i < start + 8; i++) {
		if (i < 20) {

			out->pop_box(io->popx + 8, cur_y + 1, io->popx + 131, cur_y + 10,
			              io->m_col[3], io->m_col[3], io->m_col[3]);
			out->printxy(io->popx + 10, cur_y + 2, io->m_col[5], 300,
			              scr_width, "%d.", i + 1);
			if (file_find_g[i][0] != 0) {
				if (strlen(&file_find_g[i][1]) >= 17)
					out->printnxy(io->popx + 28, cur_y + 2, io->m_col[5], 300, 17,
					               scr_width, &file_find_g[i][1]);
				else
					out->printxy(io->popx + 28, cur_y + 2, io->m_col[5], 300,
					              scr_width, &file_find_g[i][1]);
			}
			cur_y += 10;
		}
	}
}

void io_game::schalter_aus() {
	out->pop_box(io->popx + 163, io->popy + 6, io->popx + 241, io->popy + 18,
	              io->m_col[0], io->m_col[1], io->m_col[5]);
	if (io->f1)
		out->printxy(io->popx + 167, io->popy + 9, io->m_col[1], 300, scr_width,
		              FSTRING1);
	out->pop_box(io->popx + 163, io->popy + 6 + 16, io->popx + 241, io->popy + 18 + 16,
	              io->m_col[0], io->m_col[1], io->m_col[5]);
	if (io->f2)
		out->printxy(io->popx + 167, io->popy + 25, io->m_col[1], 300, scr_width,
		              FSTRING2);
	out->pop_box(io->popx + 163, io->popy + 6 + 16 * 2, io->popx + 241, io->popy + 18 + 16 * 2,
	              io->m_col[0], io->m_col[1], io->m_col[5]);
	if (io->f3)
		out->printxy(io->popx + 167, io->popy + 41, io->m_col[1], 300, scr_width,
		              FSTRING3);
	out->pop_box(io->popx + 163, io->popy + 6 + 16 * 3, io->popx + 241, io->popy + 18 + 16 * 3,
	              io->m_col[0], io->m_col[1], io->m_col[5]);
	if (io->f4)
		out->printxy(io->popx + 167, io->popy + 57, io->m_col[1], 300, scr_width,
		              FSTRING4);
}

void io_game::plot_io() {
	const int16 y1 = 120;

	out->pop_box(io->popx, io->popy, io->popx + 248, io->popy + y1, io->m_col[0], io->m_col[1], io->m_col[5]);
	out->pop_box(io->popx + 1, io->popy + 1, io->popx - 1 + 248, io->popy - 1 + y1, io->m_col[0], io->m_col[1], 300);
	out->pop_box(io->popx + 6, io->popy + 6, io->popx + 133, io->popy + y1 - 20, io->m_col[1], io->m_col[0], io->m_col[3]);
	out->pop_box(io->popx + 140, io->popy + 6, io->popx + 156, io->popy + 51, io->m_col[0], io->m_col[1], 300);
	plot_auf_txt(io->m_col[1]);

	out->pop_box(io->popx + 140, io->popy + 55, io->popx + 156, io->popy + y1 - 20,io->m_col[0], io->m_col[1], 300);
	plot_ab_txt(io->m_col[1]);

	for (int16 i = 0; i < 9; i++) {
		out->printxy(io->popx + 10, io->popy + 10 + i * 10,io->m_col[5], 300, scr_width, "%d.", i + 1);
	}
}

void io_game::plot_auf_txt(int16 farbe) {
	for (int16 i = 0; i < 3; i++)
		out->printxy(io->popx + 146, io->popy + 15 + i * 10, farbe, 300, scr_width, auf_tbl_g[i]);
}

void io_game::plot_ab_txt(int16 farbe) {
	for (int16 i = 0; i < 4; i++)
		out->printxy(io->popx + 146, io->popy + 59 + i * 10, farbe, 300, scr_width, ab_tbl_g[i]);
}

void io_game::itoa(int N, char *str, int base) {
	sprintf(str, "%d", N);
}

int16 io_game::get_savegame_files() {
	SaveStateList saveList = g_engine->listSaves();
	int ret = 0;

	for (int i = 0; i < 20; i++) {
		file_find_g[i][0] = 0;

		for (uint j = 0; j < saveList.size(); ++j) {
			if (saveList[j].getSaveSlot() == i) {
				Common::String name = saveList[j].getDescription();
				file_find_g[i][0] = 1;
				strncpy(&file_find_g[i][1], name.c_str(), USER_NAME + 3);
				file_find_g[i][USER_NAME + 3] = '\0';
				++ret;
				break;
			}
		}
	}

	return ret;
}

void io_game::save(int16 y, int16 slotNum, char *fname) {
	cur->wait_taste_los(true);
	in->alter_kb_handler();
	cur->hide_cur();
	out->pop_box
	(io->popx + 8, y, io->popx + 131, y + 10, io->m_col[1], io->m_col[0], io->m_col[4]);

	out->printxy(io->popx + 10, y + 2, io->m_col[0], 300, scr_width, "%d.", slotNum + 1);
	if (file_find_g[slotNum][0] == 0) {
		out->scanxy(io->popx + 28, y + 2, io->m_col[0], io->m_col[4], io->m_col[2], scr_width, "%36s15", &file_find_g[slotNum][1]);
	} else {
		out->printxy(io->popx + 167, io->popy + 85, io->m_col[1], 300, scr_width, FSTRING7);
		out->scanxy(io->popx + 28, y + 2, io->m_col[0], io->m_col[4], io->m_col[2], scr_width, "%36s15", &file_find_g[slotNum][1]);

	}
	in->neuer_kb_handler(kbinfo);
	mark_eintrag(y, slotNum);
	out->box_fill(io->popx + 167, io->popy + 70, io->popx + 244, io->popy + 100, io->m_col[5]);

	cur->show_cur();

	Common::String desc(&file_find_g[slotNum][1]);
	(void)g_engine->saveGameState(slotNum, desc);
}

void io_game::load(int16 slotNum, char *fname) {
	get_savegame_files();

	cur->hide_cur();
	if (file_find_g[slotNum][0] == 1) {
		(void)g_engine->loadGameState(slotNum);
	}
}

void io_game::print_shad(int16 x, int16 y, int16 fcol, int16 bcol, int16 scol, int16 width, char *name) {
	out->printxy(x + 1, y + 1, scol, bcol, width, name);
	out->printxy(x, y, fcol, bcol, width, name);
}

char *io_game::io_init(iog_init *iostruc) {
	io = iostruc;
	for (int16 i = 0; i < 20; i++)
		file_find_g[i][0] = 0;
	get_savegame_files();

	return &file_find_g[0][0];
}

void io_game::save_entry(int16 slotNum, char *fname) {
	Common::String desc(&file_find_g[slotNum][1]);
	g_engine->saveGameState(slotNum, desc);
}

} // namespace Chewy
