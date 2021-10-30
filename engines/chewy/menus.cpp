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

#include "chewy/defines.h"
#include "chewy/events.h"
#include "chewy/global.h"
#include "chewy/menus.h"

namespace Chewy {

void r44_look_news();
void r58_look_cut_mag(int16 r_nr);

int16 hot_inventar[HOT_INVENT_ANZ][4] = {

	{WIN_INF_X + 6, WIN_INF_Y + 10, WIN_INF_X + 6 + 30, WIN_INF_Y + 10 + 14},

	{WIN_INF_X + 6 + 32, WIN_INF_Y + 10, WIN_INF_X + 6 + 62, WIN_INF_Y + 10 + 14},

	{ -1, -1, -1, -1},

	{WIN_INF_X + 198, WIN_INF_Y + 10, WIN_INF_X + 198 + 30, WIN_INF_Y + 10 + 14},

	{WIN_INF_X + 198 + 40, WIN_INF_Y + 10, WIN_INF_X + 198 + 70, WIN_INF_Y + 10 + 14},

	{WIN_INF_X + 6, WIN_INF_Y + 4 + 26, WIN_INF_X + 268, WIN_INF_Y + 4 + 26 + 90},

	{WIN_INF_X + 242, WIN_INF_Y + 136, WIN_INF_X + 292, WIN_INF_Y + 136 + 14},

	{WIN_INF_X + 242, WIN_INF_Y + 156, WIN_INF_X + 292, WIN_INF_Y + 156 + 14},
};

char m_flip = 0;
int16 tmp_menu;
int16 inv_rand_x;
int16 inv_rand_y;
int16 show_invent_menu;

void plot_main_menu() {
	static const int IMAGES[] = { 7, 8, 9, 10, 12, 11 };
	int16 i;
	int16 zoomx, zoomy;
	int16 *korrektur;

	if (menu_item != tmp_menu) {
		m_flip = 0;
		tmp_menu = menu_item;
	}

	maus_mov_menu();
	korrektur = (int16 *)menutaf->korrektur;

	for (i = MENU_START_SPRITE; i < MAX_MENU_SPRITE; i++) {
		int deltaX = 0;

		if (i <= 8) {
			zoomx = 0;
			zoomy = 0;
		} else {
			zoomx = -3;
			zoomy = -3;

			if (i == 11)
				deltaX = 40;
			else if (i == 12)
				deltaX = -40;
		}

		out->scale_set(menutaf->image[i],
			MENU_X + deltaX + korrektur[i * 2],
		    spieler.MainMenuY + korrektur[i * 2 + 1],
			zoomx, zoomy, 0);
	}

	zoomx = 16;
	zoomy = 16;
	++m_flip;
	if (m_flip < 12 * (spieler.DelaySpeed + 1)) {
		int deltaX = 0;
		if (menu_item == CUR_INVENT)
			deltaX = -40;
		else if (menu_item == CUR_SAVE)
			deltaX = 40;

		int img = IMAGES[menu_item];
		out->scale_set(menutaf->image[img],
		    MENU_X + deltaX + korrektur[img * 2] - 5,
		    spieler.MainMenuY + korrektur[img * 2 + 1] - 10,
			zoomx, zoomy, 0);
	} else {
		if (m_flip > 15 * (spieler.DelaySpeed + 1))
			m_flip = 0;
	}
}

void maus_mov_menu() {
	maus_menu_x = g_events->_mousePos.x;
	if (maus_menu_x > 200) {
		g_events->warpMouse(Common::Point(200, g_events->_mousePos.y));
		maus_menu_x = 200;
	}

	menu_item = (maus_menu_x / (MAUS_MENU_MAX_X / 5));
}

void plot_inventar_menu() {
	int16 i, j, k;
	int16 *xy;
	int16 x, y;
	int16 x1, y1;
	out->setze_zeiger(workptr);
	build_menu(WIN_INVENTAR);

	for (j = 0; j < 3; j++)
		for (i = 0; i < 5; i++)
			out->box_fill(WIN_INF_X + 14 + i * 54, WIN_INF_Y + 6 + 30 + j * 32,
			               WIN_INF_X + 14 + i * 54 + 40, WIN_INF_Y + 6 + 30 + j * 32 + 24, 12);
	k = in->maus_vector(minfo.x, minfo.y, (int16 *)hot_inventar, HOT_INVENT_ANZ);
	if (k != -1) {
		if (k < 5)
			out->box_fill(hot_inventar[k][0], hot_inventar[k][1],
			               hot_inventar[k][2] + 1, hot_inventar[k][3] + 5, 41);
		else {
			x = (minfo.x - (WIN_INF_X)) / 54;
			y = (minfo.y - (WIN_INF_Y + 4 + 30)) / 30;
			k = x + (y * 5);
			k += spieler.InventY * 5;
			if (k < (spieler.InventY + 3) * 5)
				out->box_fill(WIN_INF_X + 14 + x * 54, WIN_INF_Y + 6 + 30 + y * 32,
				               WIN_INF_X + 14 + x * 54 + 40, WIN_INF_Y + 6 + 30 + y * 32 + 24, 41);
		}
	}

	if (inv_rand_x != -1) {
		out->box_fill(WIN_INF_X + 14 + inv_rand_x * 54, WIN_INF_Y + 6 + 30 + inv_rand_y * 32,
		               WIN_INF_X + 14 + inv_rand_x * 54 + 40, WIN_INF_Y + 6 + 30 + inv_rand_y * 32 + 24, 41);
		out->box(WIN_INF_X + 14 + inv_rand_x * 54 - 1, WIN_INF_Y + 6 + 30 + inv_rand_y * 32 - 1,
		          WIN_INF_X + 14 + inv_rand_x * 54 + 40 + 1, WIN_INF_Y + 6 + 30 + inv_rand_y * 32 + 24 + 1, 14);
	}

	for (i = 0; i < 2; i++) {
		ani_invent_delay[i][0] -= 1;
		if (ani_invent_delay[i][0] <= 0) {
			ani_invent_delay[i][0] = ani_invent_delay[i][1];
			++ani_count[i];
			if (ani_count[i] > ani_invent_end[i]) {
				ani_count[i] = ani_invent_anf[i];
				ani_invent_delay[i][0] = 30000;
			}
		}
		if (i == 2)
			y = 3;
		else
			y = 0;
		out->sprite_set(curtaf->image[ani_count[i]],
		                 WIN_INF_X + 8 + i * 32, WIN_INF_Y + 12 - y, scr_width);
	}

	for (i = 0; i < 2; i++) {
		out->sprite_set(menutaf->image[PFEIL_UP + i],
		                 WIN_INF_X + 200 + i * 40, WIN_INF_Y + 12, scr_width);
	}

	y = WIN_INF_Y + 6 + 30;
	for (j = 0; j < 3; j++) {
		for (i = 0; i < 5; i++) {
			if (spieler.InventSlot[(spieler.InventY + j) * 5 + i] != -1) {
				xy = (int16 *)inv_spr[spieler.InventSlot[(spieler.InventY + j) * 5 + i]];
				x1 = 40 - xy[0];
				x1 /= 2;
				y1 = 24 - xy[1];
				y1 /= 2;
				out->sprite_set(inv_spr[spieler.InventSlot[(spieler.InventY + j) * 5 + i]],
				                 x1 + WIN_INF_X + 14 + i * 54,
				                 y1 + y + 32 * j, scr_width);
			}
		}
	}
}

void invent_menu() {
	int16 menu_flag1;
	int16 maus_flag;
	int16 taste_flag;

	int16 i, k;

	int16 abfrage;
	int16 tmp, tmp1;
	int16 disp_tmp;
	int16 ret_look;
	int16 ani_tmp;
	int16 menu_first;
	flags.InventMenu = true;
	disp_tmp = spieler.DispFlag;
	spieler.DispFlag = false;
	ani_tmp = flags.AutoAniPlay;
	flags.AutoAniPlay = true;
	flags.StopAutoObj = true;
	menu_display = 0;
	tmp = spieler.MausSpeed;
	if (tmp > 3) {
		tmp1 = tmp - 2;
	} else
		tmp1 = tmp;
	in->speed(tmp1, tmp1 * 2);
	cur->move(152, 92);
	minfo.x = 152;
	minfo.y = 92;

	invent_cur_mode = CUR_USE;
	if (spieler.AkInvent != -1) {
		cursor_wahl(CUR_AK_INVENT);

	} else {
		invent_cur_mode = CUR_USE;
		cursor_wahl(CUR_USE);
	}
	menu_flag1 = MENU_EINBLENDEN;
	taste_flag = 28;
	kbinfo.key_code = '\0';
	maus_flag = 1;
	for (i = 0; i < 3; i++) {
		ani_invent_delay[i][0] = 30000;
		ani_count[i] = ani_invent_anf[i];
	}
	ret_look = -1;
	menu_first = false;
	show_invent_menu = 1;
	while (show_invent_menu == 1) {

		if (!minfo.button)
			maus_flag = 0;
		if (minfo.button == 1 || kbinfo.key_code == ENTER) {
			if (!maus_flag) {
				maus_flag = 1;
				kbinfo.key_code = '\0';
				k = in->maus_vector(minfo.x, minfo.y, (int16 *)hot_inventar, HOT_INVENT_ANZ);
				if (in->hot_key == F1_KEY)
					k = 0;
				else if (in->hot_key == F2_KEY)
					k = 1;
				switch (k) {
				case 0:
					invent_cur_mode = CUR_USE;
					menu_item = CUR_USE;
					if (spieler.AkInvent == -1) {
						cursor_wahl(invent_cur_mode);
					} else {
						cursor_wahl(CUR_AK_INVENT);
					}
					break;

				case 1:
					if (spieler.AkInvent != -1) {
						inv_rand_x = -1;
						inv_rand_y = -1;
						ret_look = look_invent(spieler.AkInvent, INV_ATS_MODE, -1);

						taste_flag = ESC;
					} else {
						invent_cur_mode = CUR_LOOK;
						menu_item = CUR_LOOK;
						cursor_wahl(invent_cur_mode);
					}
					break;

				case 3:
					in->hot_key = PAGE_UP;
					break;

				case 4:
					in->hot_key = PAGE_DOWN;
					break;

				case 5:
					inv_rand_x = (minfo.x - (WIN_INF_X)) / 54;
					inv_rand_y = (minfo.y - (WIN_INF_Y + 4 + 30)) / 30;
					k = inv_rand_x + (inv_rand_y * 5);
					k += spieler.InventY * 5;
					if (invent_cur_mode == CUR_USE) {
						if (spieler.AkInvent == -1) {

							if (spieler.InventSlot[k] != -1) {

								if (calc_use_invent(spieler.InventSlot[k]) == false) {
									menu_item = CUR_USE;
									spieler.AkInvent = spieler.InventSlot[k];
									cursor_wahl(CUR_AK_INVENT);
									del_invent_slot(spieler.InventSlot[k]);
								}
							}
						} else {
							if (spieler.InventSlot[k] != -1)
								obj_auswerten(spieler.InventSlot[k], INVENTAR_NORMAL);
							else {
								spieler.InventSlot[k] = spieler.AkInvent;
								obj->sort();
								spieler.AkInvent = -1;
								menu_item = invent_cur_mode;
								cursor_wahl(invent_cur_mode);
							}
						}
					}

					else if (invent_cur_mode == CUR_LOOK) {

						if (spieler.InventSlot[k] != -1) {
							if (calc_use_invent(spieler.InventSlot[k]) == false) {
								spieler.AkInvent = spieler.InventSlot[k];
								ret_look = look_invent(spieler.InventSlot[k], INV_ATS_MODE, -1);
								spieler.AkInvent = -1;
								cursor_wahl(invent_cur_mode);
								taste_flag = ESC;
							}
						}
					}
					break;

				}
			}
		} else if (minfo.button == 2 || kbinfo.key_code == ESC) {
			if (!maus_flag) {
				in->hot_key = ESC;
				maus_flag = 1;
			}
		}

		if (ret_look == 0) {

			invent_cur_mode = CUR_USE;
			menu_item = CUR_USE;
			if (spieler.AkInvent == -1)
				cursor_wahl(invent_cur_mode);
			else
				cursor_wahl(CUR_AK_INVENT);
		}
		else if (ret_look == 5) {
			taste_flag = false;
			maus_flag = 0;
			minfo.button = 1;
		}
		ret_look = -1;
		abfrage = in->get_switch_code();
		cur->hide_cur();
		if (taste_flag) {
			if (abfrage != taste_flag)
				taste_flag = 0;
		} else {
			switch (abfrage) {
			case F1_KEY:
				in->hot_key = F1_KEY;
				break;

			case F2_KEY:
				in->hot_key = F2_KEY;
				break;

			case ESC:
				if (!menu_first) {
					menu_first = true;
					cur->show_cur();
					while (in->get_switch_code() == ESC) {
						set_up_screen(NO_SETUP);
						inv_rand_x = -1;
						inv_rand_y = -1;
						plot_inventar_menu();
						cur->plot_cur();
						out->back2screen(workpage);
					}
				} else {
					if (menu_flag1 != MENU_EINBLENDEN) {
						menu_flag1 = MENU_AUSBLENDEN;
						show_invent_menu = false;

					}
				}
				break;

			case CURSOR_RIGHT:
				if (minfo.x < 320 - spieler.CurBreite)
					minfo.x += 3;
				break;

			case CURSOR_LEFT:
				if (minfo.x > 2)
					minfo.x -= 3;
				break;

			case CURSOR_UP:
				if (minfo.y > 2)
					minfo.y -= 3;
				break;

			case CURSOR_DOWN:
				if (minfo.y < 197 - spieler.CurHoehe)
					minfo.y += 3;
				break;

			case PAGE_UP:
				if (spieler.InventY > 0)
					--spieler.InventY;
				kbinfo.key_code = '\0';
				break;

			case PAGE_DOWN:
				if (spieler.InventY < (MAX_MOV_OBJ / 5) - 3)
					++spieler.InventY;
				kbinfo.key_code = '\0';
				break;

			}
			menu_first = true;
		}
		if (show_invent_menu != 2) {
			set_up_screen(NO_SETUP);
			cur->move(minfo.x, minfo.y);
			cur->show_cur();
			if (menu_flag1 != MENU_AUSBLENDEN) {
				inv_rand_x = -1;
				inv_rand_y = -1;
				plot_inventar_menu();
			}
			if (menu_flag1 == false)
				cur->plot_cur();
			out->setze_zeiger(0);
			if (menu_flag1 == MENU_EINBLENDEN) {
				fx->blende1(workptr, screen0, 0, 200, 0, 300);
			} else if (menu_flag1 == MENU_AUSBLENDEN)
				fx->blende1(workptr, screen0, 0, 200, 1, 300);
			menu_flag1 = false;
			out->set_clip(0, 0, 320, 200);
			out->back2screen(workpage);
		} else
			show_cur();
	}
	cur->move(maus_old_x, maus_old_y);
	minfo.x = maus_old_x;
	minfo.y = maus_old_y;
	while (in->get_switch_code() == ESC) {
		set_up_screen(NO_SETUP);
		cur->plot_cur();
		out->back2screen(workpage);
	}
	in->speed(tmp, tmp * 2);
	flags.InventMenu = false;
	flags.AutoAniPlay = ani_tmp;
	spieler.DispFlag = disp_tmp;
	menu_display = tmp_menu;
	flags.StopAutoObj = false;
}

int16 look_invent(int16 invent_nr, int16 mode, int16 ats_nr) {
	int16 ende;
	int16 txt_start;
	int16 txt_anz = 0;
	int16 maus_flag;
	int16 i, k;
	int16 ret;
	int16 xoff = 0;
	int16 yoff = 0;
	int16 txt_zeilen = 0;
	int16 rect;
	char *txt_adr = nullptr;
	char *txt_name_adr = nullptr;
	char c[2] = {0};
	ret = -1;
	ende = 0;
	txt_start = 0;
	maus_flag = 1;

	if (mode == INV_ATS_MODE) {
		atds->load_atds(invent_nr, INV_ATS_DATEI);
		ERROR
		txt_name_adr = atds->ats_get_txt(invent_nr, TXT_MARK_NAME, &txt_anz, INV_ATS_DATEI);
		txt_adr = atds->ats_get_txt(invent_nr, TXT_MARK_LOOK, &txt_anz, INV_ATS_DATEI);
		xoff = strlen(txt_name_adr);
		xoff *= fvorx8x8;
		xoff = (254 - xoff) / 2;
		txt_zeilen = 2;
		yoff = 10;
	}
	else if (mode == INV_USE_ATS_MODE) {
		txt_zeilen = 3;
		yoff = 0;
		if (ats_nr >= 15000) {
			txt_adr = atds->ats_get_txt(ats_nr - 15000, TXT_MARK_USE, &txt_anz, INV_USE_DEF);
		} else {
			txt_adr = atds->ats_get_txt(ats_nr, TXT_MARK_USE, &txt_anz, INV_USE_DATEI);
		}
		if (!txt_adr) {
			ende = 1;
		}
		ERROR
	}
	else
		ende = 1;
	while (!ende) {

		rect = in->maus_vector(minfo.x, minfo.y, (int16 *)hot_inventar, HOT_INVENT_ANZ);
		if (minfo.button) {
			if (minfo.button == 2) {
				if (!maus_flag)
					kbinfo.scan_code = ESC;
			} else if (minfo.button == 1) {
				if (!maus_flag) {
					switch (rect) {
					case 0:
						ende = 1;
						ret = 0;
						break;

					case 1:
						ende = 1;
						ret = 1;
						break;

					case 3:
					case 6:
						kbinfo.scan_code = CURSOR_UP;
						break;

					case 4:
					case 7:
						kbinfo.scan_code = CURSOR_DOWN;
						break;

					case 5:
						ret = 5;
						ende = 1;
						break;

					}
				}
			}
			maus_flag = 1;
		} else
			maus_flag = 0;
		switch (kbinfo.scan_code) {
		case F1_KEY:
			in->hot_key = F1_KEY;
			break;

		case F2_KEY:
			in->hot_key = F2_KEY;
			break;

		case ESC:
			ende = 1;
			break;
		case CURSOR_UP:
			if (txt_start > 0)
				--txt_start;
			break;

		case CURSOR_DOWN:
			if (txt_start < txt_anz - txt_zeilen)
				++txt_start;
			break;

		}
		kbinfo.scan_code = Common::KEYCODE_INVALID;
		set_up_screen(NO_SETUP);
		plot_inventar_menu();
		out->set_fontadr(font8x8);
		out->set_vorschub(fvorx8x8, fvory8x8);

		if (mode == INV_ATS_MODE)
			out->printxy(WIN_LOOK_X + xoff, WIN_LOOK_Y, 255, 300,
			              scr_width, txt_name_adr);
		out->set_fontadr(font6x8);
		out->set_vorschub(fvorx6x8, fvory6x8);
		if (txt_anz > txt_zeilen) {
			if (txt_start > 0) {
				if (rect == 6)
					out->box_fill(WIN_INF_X + 262, WIN_INF_Y + 136, WIN_INF_X + 272,
					               WIN_INF_Y + 136 + 14, 41);
				c[0] = 24;
				out->printxy(WIN_LOOK_X + 250, WIN_LOOK_Y + 4, 14, 300,
				              scr_width, c);
			}
			if (txt_start < txt_anz - txt_zeilen) {
				if (rect == 7)
					out->box_fill(WIN_INF_X + 262, WIN_INF_Y + 156, WIN_INF_X + 272,
					               WIN_INF_Y + 156 + 14, 41);
				c[0] = 25;
				out->printxy(WIN_LOOK_X + 250, WIN_LOOK_Y + 24, 14, 300, scr_width, c);
			}
		}
		k = 0;
		for (i = txt_start; i < txt_anz && i < txt_start + txt_zeilen; i++) {
			out->printxy(WIN_LOOK_X, WIN_LOOK_Y + yoff + k * 10, 14, 300,
			              scr_width, "%s", txt->str_pos(txt_adr, i));
			++k;
		}
		cur->plot_cur();

		out->back2screen(workpage);
	}
	while (in->get_switch_code() == ESC) {
		set_up_screen(NO_SETUP);
		plot_inventar_menu();
		cur->plot_cur();
		out->back2screen(workpage);
	}
	return ret;
}

void look_invent_screen(int16 txt_mode, int16 txt_nr) {
	char *str_adr;
	int16 ok;
	int16 m_mode = 0;
	int16 action_flag;
	if (!flags.AtsAction) {
		str_adr = 0;
		if (txt_nr != -1) {

			switch (txt_mode) {

			case INVENTAR_NORMAL:
			case INVENTAR_STATIC:
				ok = true;
				switch (menu_item) {
				case CUR_LOOK:
					m_mode = TXT_MARK_LOOK;
					break;

				case CUR_USE:
				case CUR_USER:
				case CUR_HOWARD:
				case CUR_NICHELLE:
					m_mode = TXT_MARK_USE;
					if (spieler.inv_cur)
						ok = false;
					break;

				case CUR_WALK:
					m_mode = TXT_MARK_WALK;
					break;

				case CUR_TALK:
					m_mode = TXT_MARK_TALK;
					break;

				}

				action_flag = 0 ;
				if (atds->get_steuer_bit(txt_nr, ATS_ACTION_BIT, ATS_DATEI)) {
					ats_action(txt_nr, m_mode, ATS_ACTION_VOR);
					action_flag = 1;
				}
				if (ok) {
					start_ats_wait(txt_nr, m_mode, 14, ATS_DATEI);
				}

				if (atds->get_steuer_bit(txt_nr, ATS_ACTION_BIT, ATS_DATEI))
					ats_action(txt_nr, m_mode, ATS_ACTION_NACH);
				if (menu_item == CUR_USE)
					flags.StaticUseTxt = true;

				break;

			}
		}
	}
}

int16 calc_use_invent(int16 inv_nr) {
	int16 benutzt;
	int16 ret;
	benutzt = false;
	if (menu_item == CUR_LOOK) {
		switch (inv_nr) {
		case ZEITUNG_INV:
			r44_look_news();
			break;

		case CUTMAG_INV:
			show_invent_menu = 2;
			benutzt = true;
			r58_look_cut_mag(58);
			break;

		case SPARK_INV:
			show_invent_menu = 2;
			benutzt = true;
			save_person_rnr();
			r58_look_cut_mag(60);
			break;

		}
	}
	else if (menu_item == CUR_USE) {
		switch (inv_nr) {
		case GBUCH_INV:
			ret = del_invent_slot(GBUCH_INV);
			spieler.InventSlot[ret] = GBUCH_OPEN_INV;
			obj->change_inventar(GBUCH_INV, GBUCH_OPEN_INV, &room_blk);
			benutzt = true;
			break;

		}
	}

	return benutzt;
}

void calc_txt_xy(int16 *x, int16 *y, char *txt_adr, int16 txt_anz) {
	int16 vorx, vory, fntbr, fnth;
	int16 len;
	int16 pix_len;
	int16 tmp_len;
	int16 i;
	out->get_fontinfo(&vorx, &vory, &fntbr, &fnth);
	len = 0;
	for (i = 0; i < txt_anz; i++) {

		tmp_len = strlen(txt->str_pos((char *)txt_adr, i));
		if (tmp_len > len)
			len = tmp_len;
	}
	len = len * vorx;
	pix_len = len / 2;
	*x = *x - pix_len + 12;
	if (*x > (SCREEN_WIDTH - len))
		*x = SCREEN_WIDTH - len;
	else if (*x < 0)
		*x = 0;
	*y = *y - (10 * txt_anz);
	if (*y < 0)
		*y = 0;
}

void get_display_xy(int16 *x, int16 *y, int16 nr) {
	int16 *xy;
	int16 x1, y1;
	xy = (int16 *)inv_spr[nr];
	x1 = 48 - xy[0];
	x1 /= 2;
	*x = x1;
	y1 = 48 - xy[1];
	y1 /= 2;
	*y = y1;
}

void build_menu(int16 x, int16 y, int16 xanz, int16 yanz, int16 col, int16 mode) {
#define BAU_MENU_ECKE_LO 15
#define BAU_MENU_ECKE_RO 16
#define BAU_MENU_ECKE_LU 17
#define BAU_MENU_ECKE_RU 18
#define BAU_MENU_UNTEN 19
#define BAU_MENU_SEITE_L 20
#define BAU_MENU_SEITE_R 21
#define BAU_MENU_OBEN 22
#define BAU_MENU_OBEN_L 23
#define BAU_MENU_OBEN_R 24
#define BAU_MENU_OBEN_M 25
	int16 i, j;
	int16 leer;
	int16 mitte;
	int16 s_nr;
	int16 xy[4][2] = {{0, 0}, {0, 0}, {0, 0}, {0, 0}};
	uint8 sprite_eckenr[4] = {BAU_MENU_ECKE_LO, BAU_MENU_ECKE_RO,
	                         BAU_MENU_ECKE_LU, BAU_MENU_ECKE_RU
	                        };

	xy[0][0] = x;
	xy[0][1] = y;
	xy[1][0] = x + 16 * (xanz - 1);
	xy[1][1] = y;
	xy[2][0] = x;
	xy[2][1] = y + 16 * (yanz - 1);
	xy[3][0] = x + 16 * (xanz - 1);
	xy[3][1] = y + 16 * (yanz - 1);

	for (i = 0; i < 4; i++)
		out->sprite_set(menutaf->image[(int16)sprite_eckenr[i]],
		                 xy[i][0], xy[i][1], scr_width);

	s_nr = BAU_MENU_SEITE_L;
	for (j = 0; j < 2; j++) {
		y = xy[j][1] + 16;
		for (i = 0; i < yanz - 2; i++) {
			out->sprite_set(menutaf->image[s_nr], xy[j][0], y + i * 16, scr_width);
		}
		++s_nr;
	}

	s_nr = BAU_MENU_OBEN;
	for (j = 0; j < 2; j++) {
		x = xy[j * 2][0] + 16;
		if ((!mode) || (mode == 1 && j == 1)) {
			for (i = 0; i < xanz - 2; i++) {
				out->sprite_set(menutaf->image[s_nr],
				                 xy[2][0] + 16 + i * 16, xy[j * 2][1], scr_width);
			}
		}
		s_nr -= 3;
	}

	if (mode) {
		leer = 0;
		mitte = 0;
		i = xanz - 7;
		if (i == 0)
			mitte = 1;
		else if (i < 6)
			mitte = i + 1;
		else if (i < 8) {
			leer = 1;
			mitte = i - 1;
		} else {
			leer = 2;
			mitte = i - 3;
		}

		x = xy[0][0] + 16;
		for (j = 0; j < 2; j++) {
			for (i = 0; i < leer; i++)
				out->sprite_set(menutaf->image[BAU_MENU_OBEN],
				                 x + i * 16, xy[0][1], scr_width);
			x = xy[1][0] - leer * 16;
		}
		out->sprite_set(menutaf->image[BAU_MENU_OBEN_L],
		                 xy[0][0] + 16 + leer * 16, xy[0][1], scr_width);

		x = xy[0][0] + 16 + leer * 16 + 32;
		for (i = 0; i < mitte; i++)
			out->sprite_set(menutaf->image[BAU_MENU_OBEN_M],
			                 x + i * 16, xy[0][1], scr_width);
		out->sprite_set(menutaf->image[BAU_MENU_OBEN_R],
		                 x + i * 16, xy[0][1], scr_width);
	}

	out->box_fill(xy[0][0] + 16, xy[0][1] + 16, xy[0][0] + 16 + (xanz - 2) * 16,
	               xy[0][1] + 16 + (yanz - 2) * 16, col);
}

void auto_menu(int16 *x, int16 *y, int16 zeilen_anz, int16 hoehe, char *text, int16 mode) {
	int16 x_pix;
	int16 tmp;
	int16 tmp1;
	int16 i;

	x_pix = 0;
	for (i = 0; i < zeilen_anz; i++) {
		tmp = strlen(txt->str_pos(text, i));
		if (x_pix < tmp)
			x_pix = tmp;
	}
	x_pix *= fvorx6x8;
	x_pix += 12;
	tmp = x_pix;
	if (x_pix % 16)
		tmp += 16;
	tmp1 = zeilen_anz * hoehe + 6 + 11;
	if (tmp1 % 16)
		tmp1 += 16;
	*x -= ((tmp / 16) * 16) / 2;
	*y -= ((tmp1 / 16) * 16) / 2;
	build_menu(*x, *y, tmp / 16, tmp1 / 16, 60, mode);
	*x += 6 + (((tmp / 16) * 16) - x_pix) / 2;
	*y += 5 + ((((tmp1 / 16) * 16) - 4) - (zeilen_anz * hoehe)) / 2;
}

#define ADS_WIN 0,153,20,3,60,1

void ads_menu() {
	int16 i;
	int16 cur_y;
	int16 cur_y_start;
	int16 col;
	AdsNextBlk *an_blk;
	if (flags.AdsDialog) {

		flags.ShowAtsInvTxt = false;
		flags.MainInput = false;
		if (ads_item_anz > 4)
			cur_y_start = 190;
		else
			cur_y_start = 190 - (4 - ads_item_anz) * 10;
		cur_y = minfo.y;
		if (cur_y < 160 || cur_y > cur_y_start + 10)
			cur_y = 255;
		else
			cur_y = (cur_y_start + 5 - cur_y) / 10;

		if (atds->aad_get_status() == -1 && ads_push == false &&
		        flags.NoDiaBox == false) {
			cur_display = true;

			build_menu(ADS_WIN);
			out->set_fontadr(font6x8);
			out->set_vorschub(fvorx6x8, fvory6x8);
			if (ads_item_anz > 4)
				cur_y_start = 190;
			else
				cur_y_start = 190 - (4 - ads_item_anz) * 10;
			for (i = 0; i < ads_item_anz && i < 4; i++) {
				if (cur_y == i)
					col = 255;
				else
					col = 14;
				out->printxy(4, cur_y_start - i * 10, col, 300, 0, ads_item_ptr[i]);
			}
		}
		switch (in->get_switch_code()) {
		case 255:
		case ENTER:
			if (cur_y < ads_item_anz && cur_y >= 0 && ads_push == false) {
				cur_display = false;
				ads_push = true;
				minfo.y = 159;
				an_blk = atds->ads_item_choice(ads_blk_nr, cur_y);
				if (an_blk->BlkNr == -1) {
					ads_action(ads_dia_nr, ads_blk_nr, an_blk->EndNr);
					ads_ende(ads_dia_nr, ads_blk_nr, an_blk->EndNr);
					stop_ads_dialog();
				} else {
					an_blk = atds->calc_next_block(ads_blk_nr, cur_y);
					ads_action(ads_dia_nr, ads_blk_nr, an_blk->EndNr);
					ads_blk_nr = an_blk->BlkNr;
					ads_item_ptr = atds->ads_item_ptr(ads_blk_nr,
					                                  &ads_item_anz);
				}
				det->stop_detail(talk_start_ani);
				det->show_static_spr(talk_hide_static);
				talk_start_ani = -1;
				talk_hide_static = -1;
				if (flags.AdsDialog == false) {
					atds->save_ads_header(ads_dia_nr);
					ERROR;
				}
			}
			break;

		default:
			ads_push = false;
			break;

		}
	}
}

void stop_ads_dialog() {
	aad_wait(-1);
	spieler.DispFlag = ads_tmp_dsp;
	cur_display = true;
	flags.ShowAtsInvTxt = true;
	flags.MainInput = true;
	flags.AdsDialog = false;
	maus_links_click = false;
	atds->stop_ads();
	if (minfo.button)
		flags.main_maus_flag = 1;
}

void cur_2_inventory() {
	if (spieler.AkInvent != -1) {
		invent_2_slot(spieler.AkInvent);
		spieler.AkInvent = -1;
		menu_item = CUR_WALK;
		cursor_wahl(menu_item);
	}
	spieler.inv_cur = 0;
}

void inventory_2_cur(int16 nr) {
	if (spieler.AkInvent == -1) {
		if (obj->check_inventar(nr)) {
			del_invent_slot(nr);
			menu_item = CUR_USE;
			spieler.AkInvent = nr;
			cursor_wahl(CUR_AK_INVENT);
			get_display_xy(&spieler.DispZx, &spieler.DispZy, spieler.AkInvent);
		}
	}
}

void new_invent_2_cur(int16 inv_nr) {
	cur_2_inventory();
	obj->add_inventar(inv_nr, &room_blk);
	inventory_2_cur(inv_nr);
}

void invent_2_slot(int16 nr) {
	int16 i;
	int16 ok;
	ok = 0;
	for (i = 0; i < MAX_MOV_OBJ && !ok; i++) {
		if (spieler.InventSlot[i] == -1) {
			spieler.InventSlot[i] = nr;
			ok = true;
		}
	}
	obj->add_inventar(nr, &room_blk);
}

int16 del_invent_slot(int16 nr) {
	int16 i;
	int16 ok;
	ok = -1;
	for (i = 0; i < MAX_MOV_OBJ; i++) {
		if (spieler.InventSlot[i] == nr) {
			spieler.InventSlot[i] = -1;
			if (ok == -1)
				ok = i;
		}
	}

	return ok;
}

} // namespace Chewy
