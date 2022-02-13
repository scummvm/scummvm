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

#include "chewy/dialogs/inventory.h"
#include "chewy/rooms/room44.h"
#include "chewy/rooms/room58.h"
#include "chewy/events.h"
#include "chewy/file.h"
#include "chewy/global.h"
#include "chewy/menus.h"

namespace Chewy {
namespace Dialogs {

static const int16 INVENTORY_HOTSPOTS[INVENTORY_HOTSPOTS_COUNT][4] = {
	{ WIN_INF_X + 6, WIN_INF_Y + 10, WIN_INF_X + 6 + 30, WIN_INF_Y + 10 + 14 },
	{ WIN_INF_X + 6 + 32, WIN_INF_Y + 10, WIN_INF_X + 6 + 62, WIN_INF_Y + 10 + 14 },
	{ -1, -1, -1, -1 },
	{ WIN_INF_X + 198, WIN_INF_Y + 10, WIN_INF_X + 198 + 30, WIN_INF_Y + 10 + 14 },
	{ WIN_INF_X + 198 + 40, WIN_INF_Y + 10, WIN_INF_X + 198 + 70, WIN_INF_Y + 10 + 14 },
	{ WIN_INF_X + 6, WIN_INF_Y + 4 + 26, WIN_INF_X + 268, WIN_INF_Y + 4 + 26 + 90 },
	{ WIN_INF_X + 242, WIN_INF_Y + 136, WIN_INF_X + 292, WIN_INF_Y + 136 + 14 },
	{ WIN_INF_X + 242, WIN_INF_Y + 156, WIN_INF_X + 292, WIN_INF_Y + 156 + 14 }
};

static const int16 ANI_INVENT_END[3] = { 7, 16, 24 };


int16 Inventory::inv_rand_x;
int16 Inventory::inv_rand_y;
int Inventory::keyVal;


void Inventory::plot_menu() {
	int16 i, j, k;
	int16 *xy;
	int16 x, y;
	int16 x1, y1;
	_G(out)->setze_zeiger(workptr);
	build_menu(WIN_INVENTAR);

	for (j = 0; j < 3; j++) {
		for (i = 0; i < 5; i++)
			_G(out)->box_fill(WIN_INF_X + 14 + i * 54, WIN_INF_Y + 6 + 30 + j * 32,
				WIN_INF_X + 14 + i * 54 + 40, WIN_INF_Y + 6 + 30 + j * 32 + 24, 12);
	}

	k = _G(in)->maus_vector(minfo.x, minfo.y, &INVENTORY_HOTSPOTS[0][0], INVENTORY_HOTSPOTS_COUNT);
	if (k != -1) {
		if (k < 5)
			_G(out)->box_fill(INVENTORY_HOTSPOTS[k][0], INVENTORY_HOTSPOTS[k][1],
				INVENTORY_HOTSPOTS[k][2] + 1, INVENTORY_HOTSPOTS[k][3] + 5, 41);
		else {
			x = (minfo.x - (WIN_INF_X)) / 54;
			y = (minfo.y - (WIN_INF_Y + 4 + 30)) / 30;
			k = x + (y * 5);
			k += _G(spieler).InventY * 5;
			if (k < (_G(spieler).InventY + 3) * 5)
				_G(out)->box_fill(WIN_INF_X + 14 + x * 54, WIN_INF_Y + 6 + 30 + y * 32,
					WIN_INF_X + 14 + x * 54 + 40, WIN_INF_Y + 6 + 30 + y * 32 + 24, 41);
		}
	}

	if (inv_rand_x != -1) {
		_G(out)->box_fill(WIN_INF_X + 14 + inv_rand_x * 54, WIN_INF_Y + 6 + 30 + inv_rand_y * 32,
			WIN_INF_X + 14 + inv_rand_x * 54 + 40, WIN_INF_Y + 6 + 30 + inv_rand_y * 32 + 24, 41);
		_G(out)->box(WIN_INF_X + 14 + inv_rand_x * 54 - 1, WIN_INF_Y + 6 + 30 + inv_rand_y * 32 - 1,
			WIN_INF_X + 14 + inv_rand_x * 54 + 40 + 1, WIN_INF_Y + 6 + 30 + inv_rand_y * 32 + 24 + 1, 14);
	}

	for (i = 0; i < 2; i++) {
		_G(ani_invent_delay)[i][0] -= 1;
		if (_G(ani_invent_delay)[i][0] <= 0) {
			_G(ani_invent_delay)[i][0] = _G(ani_invent_delay)[i][1];
			++_G(ani_count)[i];
			if (_G(ani_count)[i] > ANI_INVENT_END[i]) {
				_G(ani_count)[i] = _G(ani_invent_anf)[i];
				_G(ani_invent_delay)[i][0] = 30000;
			}
		}
		if (i == 2)
			y = 3;
		else
			y = 0;
		_G(out)->sprite_set(curtaf->image[_G(ani_count)[i]],
			WIN_INF_X + 8 + i * 32, WIN_INF_Y + 12 - y, scr_width);
	}

	for (i = 0; i < 2; i++) {
		_G(out)->sprite_set(menutaf->image[PFEIL_UP + i],
			WIN_INF_X + 200 + i * 40, WIN_INF_Y + 12, scr_width);
	}

	y = WIN_INF_Y + 6 + 30;
	for (j = 0; j < 3; j++) {
		for (i = 0; i < 5; i++) {
			if (_G(spieler).InventSlot[(_G(spieler).InventY + j) * 5 + i] != -1) {
				xy = (int16 *)inv_spr[_G(spieler).InventSlot[(_G(spieler).InventY + j) * 5 + i]];
				x1 = 40 - xy[0];
				x1 /= 2;
				y1 = 24 - xy[1];
				y1 /= 2;
				_G(out)->sprite_set(inv_spr[_G(spieler).InventSlot[(_G(spieler).InventY + j) * 5 + i]],
					x1 + WIN_INF_X + 14 + i * 54,
					y1 + y + 32 * j, scr_width);
			}
		}
	}
}

void Inventory::menu() {
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

	keyVal = 0;
	flags.InventMenu = true;
	disp_tmp = _G(spieler).DispFlag;
	_G(spieler).DispFlag = false;
	ani_tmp = flags.AutoAniPlay;
	flags.AutoAniPlay = true;
	flags.StopAutoObj = true;
	menu_display = 0;
	tmp = _G(spieler).MausSpeed;
	if (tmp > 3) {
		tmp1 = tmp - 2;
	} else
		tmp1 = tmp;
	_G(in)->speed(tmp1, tmp1 * 2);
	_G(cur)->move(152, 92);
	minfo.x = 152;
	minfo.y = 92;

	invent_cur_mode = CUR_USE;
	if (_G(spieler).AkInvent != -1) {
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
		_G(ani_invent_delay)[i][0] = 30000;
		_G(ani_count)[i] = _G(ani_invent_anf)[i];
	}

	ret_look = -1;
	menu_first = false;
	_G(show_invent_menu) = 1;

	while (_G(show_invent_menu) == 1 && !SHOULD_QUIT) {
		if (!minfo.button)
			maus_flag = 0;
		if (minfo.button == 1 || kbinfo.key_code == Common::KEYCODE_RETURN || keyVal) {
			if (!maus_flag) {
				maus_flag = 1;
				kbinfo.key_code = '\0';

				k = _G(in)->maus_vector(minfo.x, minfo.y, &INVENTORY_HOTSPOTS[0][0], INVENTORY_HOTSPOTS_COUNT);
				if (keyVal == Common::KEYCODE_F1)
					k = 0;
				else if (keyVal == Common::KEYCODE_F2)
					k = 1;
				else if (keyVal == Common::KEYCODE_RETURN)
					k = 5;

				keyVal = 0;

				switch (k) {
				case 0:
					invent_cur_mode = CUR_USE;
					menu_item = CUR_USE;
					if (_G(spieler).AkInvent == -1) {
						cursor_wahl(CUR_USE);
					} else {
						cursor_wahl(CUR_AK_INVENT);
					}
					break;

				case 1:
					if (_G(spieler).AkInvent != -1) {
						inv_rand_x = -1;
						inv_rand_y = -1;
						ret_look = look(_G(spieler).AkInvent, INV_ATS_MODE, -1);

						taste_flag = Common::KEYCODE_ESCAPE;
					} else {
						invent_cur_mode = CUR_LOOK;
						menu_item = CUR_LOOK;
						cursor_wahl(CUR_LOOK);
					}
					break;

				case 3:
					_G(in)->_hotkey = Common::KEYCODE_PAGEUP;
					break;

				case 4:
					_G(in)->_hotkey = Common::KEYCODE_PAGEDOWN;
					break;

				case 5:
					inv_rand_x = (minfo.x - (WIN_INF_X)) / 54;
					inv_rand_y = (minfo.y - (WIN_INF_Y + 4 + 30)) / 30;
					k = inv_rand_x + (inv_rand_y * 5);
					k += _G(spieler).InventY * 5;
					if (invent_cur_mode == CUR_USE) {
						if (_G(spieler).AkInvent == -1) {
							if (_G(spieler).InventSlot[k] != -1) {
								if (calc_use_invent(_G(spieler).InventSlot[k]) == false) {
									menu_item = CUR_USE;
									_G(spieler).AkInvent = _G(spieler).InventSlot[k];
									cursor_wahl(CUR_AK_INVENT);
									del_invent_slot(_G(spieler).InventSlot[k]);
								}
							}
						} else {
							if (_G(spieler).InventSlot[k] != -1)
								obj_auswerten(_G(spieler).InventSlot[k], INVENTAR_NORMAL);
							else {
								_G(spieler).InventSlot[k] = _G(spieler).AkInvent;
								obj->sort();
								_G(spieler).AkInvent = -1;
								menu_item = invent_cur_mode;
								cursor_wahl(invent_cur_mode);
							}
						}
					} else if (invent_cur_mode == CUR_LOOK) {
						if (_G(spieler).InventSlot[k] != -1) {
							if (calc_use_invent(_G(spieler).InventSlot[k]) == false) {
								_G(spieler).AkInvent = _G(spieler).InventSlot[k];
								ret_look = look(_G(spieler).InventSlot[k], INV_ATS_MODE, -1);
								_G(spieler).AkInvent = -1;
								cursor_wahl(invent_cur_mode);
								taste_flag = Common::KEYCODE_ESCAPE;
							}
						}
					}
					break;

				default:
					break;
				}
			}
		} else if (minfo.button == 2 || kbinfo.key_code == Common::KEYCODE_ESCAPE) {
			if (!maus_flag) {
				_G(in)->_hotkey = Common::KEYCODE_ESCAPE;
				maus_flag = 1;
			}
		}

		if (ret_look == 0) {
			invent_cur_mode = CUR_USE;
			menu_item = CUR_USE;
			if (_G(spieler).AkInvent == -1)
				cursor_wahl(CUR_USE);
			else
				cursor_wahl(CUR_AK_INVENT);
		} else if (ret_look == 5) {
			taste_flag = false;
			maus_flag = 0;
			minfo.button = 1;
			keyVal = Common::KEYCODE_RETURN;
		}

		ret_look = -1;
		abfrage = _G(in)->get_switch_code();
		_G(cur)->hide_cur();

		if (taste_flag) {
			if (abfrage != taste_flag)
				taste_flag = 0;
		} else {
			switch (abfrage) {
			case Common::KEYCODE_F1:
				keyVal = Common::KEYCODE_F1;
				break;

			case Common::KEYCODE_F2:
				keyVal = Common::KEYCODE_F2;
				break;

			case Common::KEYCODE_ESCAPE:
				if (!menu_first) {
					menu_first = true;
					_G(cur)->show_cur();
					while (_G(in)->get_switch_code() == Common::KEYCODE_ESCAPE) {
						set_up_screen(NO_SETUP);
						inv_rand_x = -1;
						inv_rand_y = -1;
						Dialogs::Inventory::plot_menu();
						_G(cur)->plot_cur();
						_G(out)->back2screen(workpage);
					}
				} else {
					if (menu_flag1 != MENU_EINBLENDEN) {
						menu_flag1 = MENU_AUSBLENDEN;
						_G(show_invent_menu) = false;
					}
				}
				break;

			case Common::KEYCODE_RIGHT:
				if (minfo.x < 320 - _G(spieler).CurBreite)
					minfo.x += 3;
				break;

			case Common::KEYCODE_LEFT:
				if (minfo.x > 2)
					minfo.x -= 3;
				break;

			case Common::KEYCODE_UP:
				if (minfo.y > 2)
					minfo.y -= 3;
				break;

			case Common::KEYCODE_DOWN:
				if (minfo.y < 197 - _G(spieler).CurHoehe)
					minfo.y += 3;
				break;

			case Common::KEYCODE_PAGEUP:
				if (_G(spieler).InventY > 0)
					--_G(spieler).InventY;
				kbinfo.key_code = '\0';
				break;

			case Common::KEYCODE_PAGEDOWN:
				if (_G(spieler).InventY < (MAX_MOV_OBJ / 5) - 3)
					++_G(spieler).InventY;
				kbinfo.key_code = '\0';
				break;

			default:
				break;
			}
			menu_first = true;
		}

		if (_G(show_invent_menu) != 2) {
			set_up_screen(NO_SETUP);
			_G(cur)->move(minfo.x, minfo.y);
			_G(cur)->show_cur();
			if (menu_flag1 != MENU_AUSBLENDEN) {
				inv_rand_x = -1;
				inv_rand_y = -1;
				Dialogs::Inventory::plot_menu();
			}
			if (menu_flag1 == false)
				_G(cur)->plot_cur();
			_G(out)->setze_zeiger(nullptr);
			if (menu_flag1 == MENU_EINBLENDEN) {
				fx->blende1(workptr, screen0, 0, 200, 0, 300);
			} else if (menu_flag1 == MENU_AUSBLENDEN)
				fx->blende1(workptr, screen0, 0, 200, 1, 300);
			menu_flag1 = false;
			_G(out)->set_clip(0, 0, 320, 200);
			_G(out)->back2screen(workpage);
		} else {
			show_cur();
		}
	}

	_G(cur)->move(maus_old_x, maus_old_y);
	minfo.x = maus_old_x;
	minfo.y = maus_old_y;
	while (_G(in)->get_switch_code() == Common::KEYCODE_ESCAPE && !SHOULD_QUIT) {
		set_up_screen(NO_SETUP);
		_G(cur)->plot_cur();
		_G(out)->back2screen(workpage);
	}

	_G(in)->speed(tmp, tmp * 2);
	flags.InventMenu = false;
	flags.AutoAniPlay = ani_tmp;
	_G(spieler).DispFlag = disp_tmp;
	menu_display = _G(tmp_menu);
	flags.StopAutoObj = false;
}

int16 Inventory::look(int16 invent_nr, int16 mode, int16 ats_nr) {
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
	char c[2] = { 0 };
	ret = -1;
	ende = 0;
	txt_start = 0;
	maus_flag = 1;

	if (mode == INV_ATS_MODE) {
		atds->load_atds(invent_nr, INV_ATS_DATEI);
		txt_name_adr = atds->ats_get_txt(invent_nr, TXT_MARK_NAME, &txt_anz, INV_ATS_DATEI);
		txt_adr = atds->ats_get_txt(invent_nr, TXT_MARK_LOOK, &txt_anz, INV_ATS_DATEI);
		xoff = strlen(txt_name_adr);
		xoff *= fvorx8x8;
		xoff = (254 - xoff) / 2;
		txt_zeilen = 2;
		yoff = 10;

	} else if (mode == INV_USE_ATS_MODE) {
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
	} else {
		ende = 1;
	}

	while (!ende) {
		rect = _G(in)->maus_vector(minfo.x, minfo.y, (const int16 *)INVENTORY_HOTSPOTS, INVENTORY_HOTSPOTS_COUNT);

		if (minfo.button) {
			if (minfo.button == 2) {
				if (!maus_flag)
					kbinfo.scan_code = Common::KEYCODE_ESCAPE;
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
						kbinfo.scan_code = Common::KEYCODE_UP;
						break;

					case 4:
					case 7:
						kbinfo.scan_code = Common::KEYCODE_DOWN;
						break;

					case 5:
						ret = 5;
						ende = 1;
						break;

					default:
						break;
					}
				}
			}

			maus_flag = 1;
		} else {
			maus_flag = 0;
		}

		switch (kbinfo.scan_code) {
		case Common::KEYCODE_F1:
			_G(in)->_hotkey = Common::KEYCODE_F1;
			break;

		case Common::KEYCODE_F2:
			_G(in)->_hotkey = Common::KEYCODE_F2;
			break;

		case Common::KEYCODE_ESCAPE:
			ende = 1;
			break;

		case Common::KEYCODE_UP:
			if (txt_start > 0)
				--txt_start;
			break;

		case Common::KEYCODE_DOWN:
			if (txt_start < txt_anz - txt_zeilen)
				++txt_start;
			break;

		default:
			break;
		}

		kbinfo.scan_code = Common::KEYCODE_INVALID;
		set_up_screen(NO_SETUP);
		Dialogs::Inventory::plot_menu();
		_G(out)->set_fontadr(font8x8);
		_G(out)->set_vorschub(fvorx8x8, fvory8x8);

		if (mode == INV_ATS_MODE)
			_G(out)->printxy(WIN_LOOK_X + xoff, WIN_LOOK_Y, 255, 300,
				scr_width, txt_name_adr);

		_G(out)->set_fontadr(font6x8);
		_G(out)->set_vorschub(fvorx6x8, fvory6x8);

		if (txt_anz > txt_zeilen) {
			if (txt_start > 0) {
				if (rect == 6)
					_G(out)->box_fill(WIN_INF_X + 262, WIN_INF_Y + 136, WIN_INF_X + 272,
						WIN_INF_Y + 136 + 14, 41);
				c[0] = 24;
				_G(out)->printxy(WIN_LOOK_X + 250, WIN_LOOK_Y + 4, 14, 300,
					scr_width, c);
			}

			if (txt_start < txt_anz - txt_zeilen) {
				if (rect == 7)
					_G(out)->box_fill(WIN_INF_X + 262, WIN_INF_Y + 156, WIN_INF_X + 272,
						WIN_INF_Y + 156 + 14, 41);
				c[0] = 25;
				_G(out)->printxy(WIN_LOOK_X + 250, WIN_LOOK_Y + 24, 14, 300, scr_width, c);
			}
		}

		k = 0;
		for (i = txt_start; i < txt_anz && i < txt_start + txt_zeilen; i++) {
			_G(out)->printxy(WIN_LOOK_X, WIN_LOOK_Y + yoff + k * 10, 14, 300,
				scr_width, "%s", txt->str_pos(txt_adr, i));
			++k;
		}

		_G(cur)->plot_cur();
		_G(out)->back2screen(workpage);
		SHOULD_QUIT_RETURN0;
	}

	while (_G(in)->get_switch_code() == Common::KEYCODE_ESCAPE) {
		set_up_screen(NO_SETUP);
		Dialogs::Inventory::plot_menu();
		_G(cur)->plot_cur();
		_G(out)->back2screen(workpage);
		SHOULD_QUIT_RETURN0;
	}

	return ret;
}

void Inventory::look_screen(int16 txt_mode, int16 txt_nr) {
	int16 ok;
	int16 m_mode = 0;

	if (!flags.AtsAction) {
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
					if (_G(spieler).inv_cur)
						ok = false;
					break;

				case CUR_WALK:
					m_mode = TXT_MARK_WALK;
					break;

				case CUR_TALK:
					m_mode = TXT_MARK_TALK;
					break;

				}

				if (atds->get_steuer_bit(txt_nr, ATS_ACTION_BIT, ATS_DATEI)) {
					ats_action(txt_nr, m_mode, ATS_ACTION_VOR);
				}
				if (ok) {
					start_ats_wait(txt_nr, m_mode, 14, ATS_DATEI);
				}

				if (atds->get_steuer_bit(txt_nr, ATS_ACTION_BIT, ATS_DATEI))
					ats_action(txt_nr, m_mode, ATS_ACTION_NACH);
				if (menu_item == CUR_USE)
					flags.StaticUseTxt = true;
				break;

			default:
				break;
			}
		}
	}
}

int16 Inventory::calc_use_invent(int16 inv_nr) {
	int16 ret_val;
	int16 ret;
	ret_val = false;

	if (menu_item == CUR_LOOK) {
		switch (inv_nr) {
		case ZEITUNG_INV:
			Rooms::Room44::look_news();
			break;

		case CUTMAG_INV:
			_G(show_invent_menu) = 2;
			ret_val = true;
			Rooms::Room58::look_cut_mag(58);
			break;

		case SPARK_INV:
			_G(show_invent_menu) = 2;
			ret_val = true;
			save_person_rnr();
			Rooms::Room58::look_cut_mag(60);
			break;

		case DIARY_INV:
			showDiary();
			ret_val = true;
			break;

		default:
			break;
		}
	} else if (menu_item == CUR_USE) {
		switch (inv_nr) {
		case GBUCH_INV:
			ret = del_invent_slot(GBUCH_INV);
			_G(spieler).InventSlot[ret] = GBUCH_OPEN_INV;
			obj->change_inventar(GBUCH_INV, GBUCH_OPEN_INV, &room_blk);
			ret_val = true;
			break;

		default:
			break;
		}
	}

	return ret_val;
}

void Inventory::showDiary() {
	int16 scrollx = _G(spieler).scrollx,
		scrolly = _G(spieler).scrolly;
	_G(spieler).scrollx = 0;
	_G(spieler).scrolly = 0;

	room->load_tgp(DIARY_START, &room_blk, GBOOK_TGP, 0, GBOOK);
	_G(out)->setze_zeiger(workptr);
	_G(out)->map_spr2screen(ablage[room_blk.AkAblage], _G(spieler).scrollx, _G(spieler).scrolly);
	_G(out)->back2screen(workpage);
	room->set_ak_pal(&room_blk);
	_G(out)->setze_zeiger(nullptr);
	fx->blende1(workptr, screen0, pal, 150, 0, 0);

	while (_G(in)->get_switch_code() != Common::KEYCODE_ESCAPE) {
		g_events->update();
		SHOULD_QUIT_RETURN;
	}
	while (_G(in)->get_switch_code() != 0) {
		g_events->update();
		SHOULD_QUIT_RETURN;
	}

	room->load_tgp(_G(spieler).PersonRoomNr[P_CHEWY], &room_blk, EPISODE1_TGP, GED_LOAD, EPISODE1);
	_G(spieler).scrollx = scrollx;
	_G(spieler).scrolly = scrolly;
	set_up_screen(NO_SETUP);
	Dialogs::Inventory::plot_menu();
	_G(out)->setze_zeiger(nullptr);
	room->set_ak_pal(&room_blk);
	fx->blende1(workptr, screen0, pal, 150, 0, 0);
}

} // namespace Dialogs
} // namespace Chewy
