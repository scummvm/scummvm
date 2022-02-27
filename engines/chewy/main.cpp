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

#include "common/config-manager.h"
#include "chewy/main.h"
#include "chewy/chewy.h"
#include "chewy/events.h"
#include "chewy/menus.h"
#include "chewy/ngshext.h"
#include "chewy/dialogs/files.h"
#include "chewy/dialogs/inventory.h"
#include "chewy/dialogs/main_menu.h"

namespace Chewy {

#define AUSGANG_CHECK_PIX 8
#define SCROLL_LEFT 120
#define SCROLL_RIGHT SCREEN_WIDTH-SCROLL_LEFT
#define SCROLL_UP 80
#define SCROLL_DOWN SCREEN_HEIGHT-SCROLL_UP

static const int16 invent_display[4][2] = {
	{5, 0}, { 265, 0 }, { 265, 149 }, { 5, 149 }
};

extern void switch_room(int16 nr);

void game_main() {
	_G(fontMgr) = new FontMgr();

	_G(font8) = new ChewyFont("TXT/8X8.TFF");
	_G(font6) = new ChewyFont("TXT/6X8.TFF");
	_G(font6)->setDisplaySize(_G(font6)->getDataWidth() - 2, _G(font6)->getDataHeight());
	_G(font8)->setDeltaX(10);
	_G(fontMgr)->setFont(_G(font8)); // set default font

	_G(room_start_nr) = 0;
	standard_init();
	_G(out)->cls();
	cursorChoice(CUR_WALK);
	_G(workptr) = _G(workpage) + 4l;

	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1) {
		(void)g_engine->loadGameState(saveSlot);
		Dialogs::MainMenu::playGame();
		return;
	} else {
		Dialogs::MainMenu::execute();
	}

	remove(ADSH_TMP);
	tidy();
	_G(out)->rest_palette();
}

void alloc_buffers() {
	_G(workpage) = (byte *)MALLOC(64004l);
	_G(pal) = (byte *)MALLOC(768l);
	_G(cur_back) = (byte *)MALLOC(16 * 16 + 4);
	_G(Ci).VirtScreen = _G(workpage);
	_G(Ci).TempArea = (byte *)MALLOC(64004l);
	_G(det)->set_taf_ani_mem(_G(Ci).TempArea);
	_G(Ci).MusicSlot = (byte *)MALLOC(MUSIC_SLOT_SIZE);
	_G(Ci).MaxMusicSize = MUSIC_SLOT_SIZE;
}

void free_buffers() {
	_G(det)->del_dptr();
	for (int16 i = 0; i < MAX_PERSON; i++)
		free((char *)_G(PersonTaf)[i]);
	free((char *)_G(spz_tinfo));
	free(_G(spblende));
	free((char *)_G(menutaf));
	free((char *)_G(chewy));

	free((char *)_G(curtaf));
	free(_G(Ci).MusicSlot);
	free(_G(Ci).TempArea);
	free(_G(cur_back));
	free(_G(pal));
	free(_G(workpage));
}

void cursorChoice(int16 nr) {
	int16 ok = true;
	if (nr != CUR_USER) {
		_G(curblk).sprite = _G(curtaf)->image;
		_G(curani)._delay = (1 + _G(spieler).DelaySpeed) * 5;
	}
	switch (nr) {
	case CUR_WALK:
		_G(curani)._start = 0;
		_G(curani)._end = 3;
		break;

	case CUR_NO_WALK:
		_G(curani)._start = 8;
		_G(curani)._end = 8;
		break;

	case CUR_USE:
		_G(curani)._start = 4;
		_G(curani)._end = 7;
		break;

	case CUR_NO_USE:
		_G(curani)._start = 4;
		_G(curani)._end = 4;
		break;

	case CUR_NOPE:
		_G(curani)._start = 9;
		_G(curani)._end = 12;
		break;

	case CUR_LOOK:
		_G(curani)._start = 13;
		_G(curani)._end = 16;
		break;

	case CUR_NO_LOOK:
		_G(curani)._start = 16;
		_G(curani)._end = 16;
		break;

	case CUR_TALK:
		_G(curani)._start = 17;
		_G(curani)._end = 20;
		break;

	case CUR_NO_TALK:
		_G(curani)._start = 17;
		_G(curani)._end = 17;
		break;

	case CUR_INVENT:
		_G(curani)._start = 21;
		_G(curani)._end = 24;
		break;

	case CUR_AK_INVENT:
		_G(curani)._start = _G(spieler).AkInvent;
		_G(curani)._end = _G(spieler).AkInvent;
		_G(curblk).sprite = &_G(inv_spr)[0];
		_G(spieler).inv_cur = true;
		break;

	case CUR_SAVE:
		_G(curani)._start = 25;
		_G(curani)._end = 25;
		break;

	case CUR_AUSGANG_LINKS:
		_G(curani)._start = AUSGANG_LINKS_SPR;
		_G(curani)._end = AUSGANG_LINKS_SPR;
		break;

	case CUR_AUSGANG_RECHTS:
		_G(curani)._start = AUSGANG_RECHTS_SPR;
		_G(curani)._end = AUSGANG_RECHTS_SPR;
		break;

	case CUR_AUSGANG_OBEN:
		_G(curani)._start = AUSGANG_OBEN_SPR;
		_G(curani)._end = AUSGANG_OBEN_SPR;
		break;

	case CUR_AUSGANG_UNTEN:
		_G(curani)._start = AUSGANG_UNTEN_SPR;
		_G(curani)._end = AUSGANG_UNTEN_SPR;
		break;

	case CUR_DISK:
		_G(curani)._start = 30;
		_G(curani)._end = 30;
		break;

	case CUR_HOWARD:
		_G(curani)._start = 31;
		_G(curani)._end = 31;
		break;

	case CUR_NICHELLE:
		_G(curani)._start = 37;
		_G(curani)._end = 37;
		break;

	case CUR_ZEIGE:
		_G(curani)._start = 9;
		_G(curani)._end = 9;
		break;
	case CUR_USER:

		break;

	default:
		ok = false;
		break;
	}

	if (ok) {
		_cursorMoveFl = true;
		_G(cur)->set_cur_ani(&_G(curani));
		_G(spieler)._curWidth = READ_LE_INT16(_G(curblk).sprite[_G(curani)._start]);
		_G(spieler)._curHeight = READ_LE_INT16(_G(curblk).sprite[_G(curani)._start] + 2);
	}
}

void hideCur() {
	if (!_G(cur_hide_flag)) {
		_G(cur_hide_flag) = true;
		_G(flags).ShowAtsInvTxt = false;
		_G(cur)->hide_cur();
		_G(flags).CursorStatus = false;
	}
}

void showCur() {
	_G(flags).ShowAtsInvTxt = true;
	_G(cur)->show_cur();
	_G(flags).CursorStatus = true;
}

void menuEntry() {
	_G(det)->freezeAni();
	_G(uhr)->set_all_status(TIMER_FREEZE);
}

void menuExit() {
	_G(det)->unfreeze_ani();
	_G(uhr)->set_all_status(TIMER_UNFREEZE);
	_G(uhr)->reset_timer(0, 0);
	_G(FrameSpeed) = 0;
}

bool mainLoop(int16 mode) {
	bool retValue = false;

	mouseAction();
	if (_G(flags).MainInput) {
		switch (_G(kbinfo).scan_code) {
		case Common::KEYCODE_F1:
			_G(spieler).inv_cur = false;
			_G(menu_item) = CUR_WALK;
			cursorChoice(_G(menu_item));
			if (_G(menu_display) == MENU_EINBLENDEN)
				_G(menu_display) = MENU_AUSBLENDEN;
			break;

		case Common::KEYCODE_F2:
			_G(spieler).inv_cur = false;
			_G(menu_item) = CUR_USE;
			cursorChoice(_G(menu_item));
			if (_G(menu_display) == MENU_EINBLENDEN)
				_G(menu_display) = MENU_AUSBLENDEN;
			break;

		case Common::KEYCODE_F3:
			_G(spieler).inv_cur = false;
			_G(menu_item) = CUR_LOOK;
			cursorChoice(_G(menu_item));
			if (_G(menu_display) == MENU_EINBLENDEN)
				_G(menu_display) = MENU_AUSBLENDEN;
			break;

		case Common::KEYCODE_F4:
			_G(spieler).inv_cur = false;
			_G(menu_item) = CUR_TALK;
			cursorChoice(_G(menu_item));
			if (_G(menu_display) == MENU_EINBLENDEN)
				_G(menu_display) = MENU_AUSBLENDEN;
			break;

		case Common::KEYCODE_F5:
		case Common::KEYCODE_SPACE:
			_G(tmp_menu_item) = _G(menu_item);
			_G(maus_old_x) = _G(minfo).x;
			_G(maus_old_y) = _G(minfo).y;
			_G(menu_item) = CUR_USE;
			menuEntry();
			Dialogs::Inventory::menu();
			menuExit();
			_G(menu_flag) = MENU_AUSBLENDEN;
			_G(menu_display) = 0;
			_G(cur_display) = true;
			if (_G(spieler).AkInvent == -1) {
				_G(menu_item) = _G(tmp_menu_item);
				cursorChoice(_G(menu_item));
				_G(spieler).inv_cur = false;
			} else {
				_G(menu_item) = CUR_USE;
				cursorChoice(CUR_AK_INVENT);

				get_display_xy(&_G(spieler).DispZx, &_G(spieler).DispZy, _G(spieler).AkInvent);
			}

			_G(kbinfo).key_code = '\0';
			break;

		case Common::KEYCODE_F6:
			_G(flags).SaveMenu = true;

			_G(out)->setPointer(_G(screen0));
			_G(fontMgr)->setFont(_G(font6));
			cursorChoice(CUR_SAVE);
			if (Dialogs::Files::execute(true) == 1) {
				retValue = true;
				_G(fx_blend) = BLEND4;
			}
			if (_G(spieler).inv_cur && _G(spieler).AkInvent != -1 && _G(menu_item) == CUR_USE) {
				cursorChoice(CUR_AK_INVENT);
			} else
				cursorChoice(_G(menu_item));
			_G(cur_display) = true;
			_G(flags).SaveMenu = false;
			_G(cur)->show_cur();
			_G(out)->setPointer(_G(workptr));
			break;

		case Common::KEYCODE_ESCAPE:
			if (_G(menu_display) == 0) {
				menuEntry();
				_G(tmp_menu_item) = _G(menu_item);
				_G(maus_old_x) = _G(minfo).x;
				_G(maus_old_y) = _G(minfo).y;
				_G(menu_display) = MENU_EINBLENDEN;
				_G(maus_menu_x) = (MAUS_MENU_MAX_X / 5) * (_G(menu_item));
				_G(cur_display) = false;
				_G(cur)->move(_G(maus_menu_x), 100);
			} else {
				menuExit();
				_G(menu_item) = _G(tmp_menu_item);
				_G(menu_display) = MENU_AUSBLENDEN;
				if (_G(spieler).inv_cur && _G(spieler).AkInvent != -1 && _G(menu_item) == CUR_USE) {
					cursorChoice(CUR_AK_INVENT);
				} else
					cursorChoice(_G(menu_item));
			}
			break;

		case Common::KEYCODE_RETURN:
			switch (_G(menu_item)) {
			case CUR_INVENT:

				_G(menu_item) = CUR_USE;
				menuEntry();
				Dialogs::Inventory::menu();
				menuExit();
				_G(menu_flag) = MENU_AUSBLENDEN;
				_G(menu_display) = 0;
				_G(cur_display) = true;
				if (_G(spieler).AkInvent == -1) {
					_G(menu_item) = _G(tmp_menu_item);
					cursorChoice(_G(menu_item));
					_G(spieler).inv_cur = false;
				} else {
					_G(menu_item) = CUR_USE;
					cursorChoice(CUR_AK_INVENT);
					get_display_xy(&_G(spieler).DispZx, &_G(spieler).DispZy, _G(spieler).AkInvent);
				}
				break;

			case CUR_SAVE: {
				_G(flags).SaveMenu = true;
				_G(menu_display) = MENU_EINBLENDEN;
				_G(cur)->move(152, 92);
				_G(minfo).x = 152;
				_G(minfo).y = 92;
				_G(fontMgr)->setFont(_G(font6));

				_G(out)->setPointer(_G(screen0));
				cursorChoice(CUR_SAVE);
				int16 ret = Dialogs::Files::execute(true);
				if (ret == IOG_END) {
					retValue = true;
					_G(fx_blend) = BLEND4;
				}

				_G(out)->setPointer(_G(workptr));
				_G(menu_item) = _G(tmp_menu_item);
				_G(menu_display) = MENU_AUSBLENDEN;

				if (_G(spieler).inv_cur && _G(spieler).AkInvent != -1 && _G(menu_item) == CUR_USE) {
					cursorChoice(CUR_AK_INVENT);
				} else
					cursorChoice(_G(tmp_menu_item));
				_G(cur_display) = true;

				_G(flags).SaveMenu = false;
				_G(cur)->show_cur();
				}
				break;

			default:
				if (_G(menu_display) != 0) {
					menuExit();
					_G(menu_flag) = MENU_AUSBLENDEN;
					_G(menu_display) = 0;
					_G(cur_display) = true;
					_G(cur)->move(_G(maus_old_x), _G(maus_old_y));
					_G(minfo).x = _G(maus_old_x);
					_G(minfo).y = _G(maus_old_y);
					_G(spieler).inv_cur = false;
					cursorChoice(_G(menu_item));
				}
				break;
			}
			break;

		case Common::KEYCODE_RIGHT:
		case Common::KEYCODE_LEFT:
		case Common::KEYCODE_UP:
		case Common::KEYCODE_DOWN:
			kb_cur_action(_G(kbinfo).scan_code, 0);
			break;

		case 41:
			_G(spieler).DispFlag ^= 1;
			break;

		case Common::KEYCODE_TAB:
			if (_G(menu_display) == 0 && _G(spieler).DispFlag) {
				if (_G(spieler).InvDisp < 3)
					++_G(spieler).InvDisp;
				else
					_G(spieler).InvDisp = 0;
			}
			break;

		default:
			break;
		}

		if (_G(menu_display) == MENU_AUSBLENDEN) {
			menuExit();
			_G(menu_flag) = MENU_AUSBLENDEN;
			_G(menu_display) = 0;
			_G(cur_display) = true;
			_G(cur)->move(_G(maus_old_x), _G(maus_old_y));
			_G(minfo).x = _G(maus_old_x);
			_G(minfo).y = _G(maus_old_y);
		}
	}

	_G(kbinfo).scan_code = Common::KEYCODE_INVALID;
	if (mode == DO_SETUP)
		set_up_screen(DO_MAIN_LOOP);

	return retValue;
}

static void showWalkAreas() {
	int xs = (_G(spieler).scrollx / 8) * 8,
		ys = (_G(spieler).scrolly / 8) * 8;

	for (int y = 0, yp = ys; y < 200 / 8; ++y, yp += 8) {
		for (int x = 0, xp = xs; x < 320 / 8; ++x, xp += 8) {
			int idx = _G(ged)->ged_idx(xp, yp,
				_G(room)->_gedXAnz[_G(room_blk).AkAblage],
				_G(ged_mem)[_G(room_blk).AkAblage]);

			if (idx) {
				Common::Rect r(xp, yp, xp + 8, yp + 8);
				r.translate(-_G(spieler).scrollx, -_G(spieler).scrolly);
				r.clip(Common::Rect(0, 0, 320, 200));

				g_screen->frameRect(r, 0xff);
			}
		}
	}
}

void set_up_screen(SetupScreenMode mode) {
	int16 i;
	int16 txt_nr;

	bool isMainLoop = mode == DO_MAIN_LOOP;
	if (isMainLoop)
		mode = DO_SETUP;

	_G(uhr)->calc_timer();

	if (_G(ani_timer)[0].TimeFlag) {
		_G(uhr)->reset_timer(0, 0);
		_G(spieler).DelaySpeed = _G(FrameSpeed) / _G(spieler).FramesPerSecond;
		_G(spieler_vector)[P_CHEWY].Delay = _G(spieler).DelaySpeed + _G(spz_delay)[P_CHEWY];
		_G(FrameSpeed) = 0;
		_G(det)->set_global_delay(_G(spieler).DelaySpeed);
	}
	++_G(FrameSpeed);
	_G(out)->setPointer(_G(workptr));
	_G(out)->map_spr2screen(_G(ablage)[_G(room_blk).AkAblage], _G(spieler).scrollx, _G(spieler).scrolly);

	for (i = 0; i < MAX_PERSON; i++)
		zoom_mov_anpass(&_G(spieler_vector)[i], &_G(spieler_mi)[i]);

	if (_G(SetUpScreenFunc) && _G(menu_display) == 0 && !_G(flags).InventMenu) {
		_G(SetUpScreenFunc)();
		_G(out)->setPointer(_G(workptr));
	}

	sprite_engine();
	if (_G(menu_display) == MENU_EINBLENDEN || _G(flags).InventMenu) {
		if (!_G(flags).InventMenu)
			plot_main_menu();
	} else {
		kb_mov(1);
		_G(det)->unfreeze_ani();
		check_mouse_ausgang(_G(minfo).x + _G(spieler).scrollx, _G(minfo).y + _G(spieler).scrolly);

		if (!_G(flags).SaveMenu)
			calc_ani_timer();

		if (_G(spieler).AkInvent != -1 && _G(spieler).DispFlag) {
			build_menu(invent_display[_G(spieler).InvDisp][0],
			           invent_display[_G(spieler).InvDisp][1], 3, 3, 60, 0);
			_G(out)->sprite_set(_G(inv_spr)[_G(spieler).AkInvent],
			                 invent_display[_G(spieler).InvDisp][0] + 1 + _G(spieler).DispZx,
			                 invent_display[_G(spieler).InvDisp][1] + 1 + _G(spieler).DispZy
			                 , _G(scr_width));
		}

		if (_G(flags).AdsDialog)
			ads_menu();
		if (_G(maus_links_click)) {
			if (_G(menu_item) == CUR_WALK) {
				if (_G(cur_ausgang_flag)) {
					calc_ausgang(_G(minfo).x + _G(spieler).scrollx, _G(minfo).y + _G(spieler).scrolly);
				} else {
					if (!_G(flags).ChewyDontGo) {
						_G(gpkt).Dx = _G(minfo).x - _G(spieler_mi)[P_CHEWY].HotMovX +
						          _G(spieler).scrollx + _G(spieler_mi)[P_CHEWY].HotX;
						_G(gpkt).Dy = _G(minfo).y - _G(spieler_mi)[P_CHEWY].HotMovY +
						          _G(spieler).scrolly + _G(spieler_mi)[P_CHEWY].HotY;
						_G(gpkt).Sx = _G(spieler_vector)[P_CHEWY].Xypos[0] +
						          _G(spieler_mi)[P_CHEWY].HotX;
						_G(gpkt).Sy = _G(spieler_vector)[P_CHEWY].Xypos[1] +
						          _G(spieler_mi)[P_CHEWY].HotY;
						_G(gpkt).Breite = _G(room)->_gedXAnz[_G(room_blk).AkAblage];
						_G(gpkt).Hoehe = _G(room)->_gedYAnz[_G(room_blk).AkAblage];
						_G(gpkt).Mem = _G(ged_mem)[_G(room_blk).AkAblage];
						_G(gpkt).Ebenen = _G(room)->_gedInfo[_G(room_blk).AkAblage].Ebenen;
						_G(gpkt).AkMovEbene = _G(ged_mov_ebene);
						_G(mov)->goto_xy(&_G(gpkt));
						_G(spieler_mi)[P_CHEWY].XyzStart[0] = _G(spieler_vector)[P_CHEWY].Xypos[0];
						_G(spieler_mi)[P_CHEWY].XyzStart[1] = _G(spieler_vector)[P_CHEWY].Xypos[1];
						_G(spieler_mi)[P_CHEWY].XyzEnd[0] = _G(gpkt).Dx - _G(spieler_mi)[P_CHEWY].HotX;
						_G(spieler_mi)[P_CHEWY].XyzEnd[1] = _G(gpkt).Dy - _G(spieler_mi)[P_CHEWY].HotY;
						_G(mov)->get_mov_vector((int16 *)_G(spieler_mi)[P_CHEWY].XyzStart, _G(spieler_mi)[P_CHEWY].Vorschub, &_G(spieler_vector)[P_CHEWY]);
						get_phase(&_G(spieler_vector)[P_CHEWY], &_G(spieler_mi)[P_CHEWY]);
						_G(spieler_vector)[P_CHEWY].DelayCount = 0;
						_G(auto_p_nr) = P_CHEWY;
					}
				}
			}
		}

		calc_auto_go();

		if (_G(fx_blend)) {
			int16 idx = _G(ged)->ged_idx(
				_G(spieler_vector)[P_CHEWY].Xypos[0] + _G(spieler_mi)[P_CHEWY].HotX,
				_G(spieler_vector)[P_CHEWY].Xypos[1] + _G(spieler_mi)[P_CHEWY].HotY,
				_G(room)->_gedXAnz[_G(room_blk).AkAblage],
				_G(ged_mem)[_G(room_blk).AkAblage]);
			check_shad(idx, 0);
		} else {
			for (i = 0; i < MAX_PERSON; i++) {
				mov_objekt(&_G(spieler_vector)[i], &_G(spieler_mi)[i]);
				_G(spieler_mi)[i].XyzStart[0] = _G(spieler_vector)[i].Xypos[0];
				_G(spieler_mi)[i].XyzStart[1] = _G(spieler_vector)[i].Xypos[1];
			}
		}

		for (i = 0; i < _G(auto_obj) && !_G(flags).StopAutoObj; i++)
			mov_objekt(&_G(auto_mov_vector)[i], &_G(auto_mov_obj)[i]);

		int16 nr = _G(obj)->is_iib_mouse(_G(minfo).x + _G(spieler).scrollx, _G(minfo).y + _G(spieler).scrolly);
		if (nr != -1) {
			txt_nr = _G(obj)->iib_txt_nr(nr);
			mous_obj_action(nr, mode, INVENTAR_NORMAL, txt_nr);
		} else {
			int16 tmp = calcMouseText(_G(minfo).x, _G(minfo).y, mode);
			if (tmp == -1 || tmp == 255) {

				nr = _G(obj)->is_sib_mouse(_G(minfo).x + _G(spieler).scrollx, _G(minfo).y + _G(spieler).scrolly);
				if (nr != -1) {
					txt_nr = _G(obj)->sib_txt_nr(nr);
					mous_obj_action(nr, mode, INVENTAR_STATIC, txt_nr);
				} else
					calc_mouse_person(_G(minfo).x, _G(minfo).y);
			}
		}
		if (_G(cur_display) == true && mode == DO_SETUP) {
			_G(cur)->plot_cur();

			if ((_G(spieler).inv_cur) && (_G(flags).CursorStatus == true))
				_G(out)->sprite_set(_G(curtaf)->image[_G(pfeil_ani) + 32], _G(minfo).x, _G(minfo).y,
				                _G(scr_width));
			if (_G(pfeil_delay) == 0) {
				_G(pfeil_delay) = _G(spieler).DelaySpeed;
				if (_G(pfeil_ani) < 4)
					++_G(pfeil_ani);
				else
					_G(pfeil_ani) = 0;
			} else {
				--_G(pfeil_delay);
			}
		}
	}

	_G(atds)->print_aad(_G(spieler).scrollx, _G(spieler).scrolly);
	_G(atds)->print_ats(_G(spieler_vector)[P_CHEWY].Xypos[0] + CH_HOT_X,
	                _G(spieler_vector)[P_CHEWY].Xypos[1], _G(spieler).scrollx, _G(spieler).scrolly);
	_G(maus_links_click) = false;
	_G(menu_flag) = false;
	if (mode == DO_SETUP) {
		_G(out)->setPointer(nullptr);
		switch (_G(fx_blend)) {
		case BLEND1:
			_G(fx)->blende1(_G(workptr), _G(screen0), _G(pal), 150, 0, 0);
			break;

		case BLEND2:
			_G(fx)->blende1(_G(workptr), _G(screen0), _G(pal), 150, 1, 0);
			break;

		case BLEND3:
			_G(fx)->rnd_blende(_G(spblende), _G(workptr), _G(screen0), _G(pal), 0, 10);
			break;

		case BLEND4:
			_G(out)->setPointer(_G(workptr));
			_G(out)->cls();
			_G(out)->setPointer(nullptr);
			_G(fx)->blende1(_G(workptr), _G(screen0), _G(pal), 150, 0, 0);
			break;

		default:
			_G(out)->back2screen(_G(workpage));
			break;
		}

		_G(fx_blend) = BLEND_NONE;
	}

	if (g_engine->_showWalkAreas)
		showWalkAreas();

	_G(cur_hide_flag) = false;
	int16 *ScrXy = (int16 *)_G(ablage)[_G(room_blk).AkAblage];
	if (!_G(menu_display))
		calc_scroll(_G(spieler_vector)[P_CHEWY].Xypos[0] + _G(spieler_mi)[P_CHEWY].HotX,
		            _G(spieler_vector)[P_CHEWY].Xypos[1] + _G(spieler_mi)[P_CHEWY].HotY,
		            ScrXy[0], ScrXy[1],
		            &_G(spieler).scrollx, &_G(spieler).scrolly);

	g_screen->update();

	g_engine->setCanLoadSave(isMainLoop);
	EVENTS_UPDATE;
	g_engine->setCanLoadSave(false);
}

void mous_obj_action(int16 nr, int16 mode, int16 txt_mode, int16 txt_nr) {
	int16 anz = 0;
	char *str_adr = nullptr;

	if (mode == DO_SETUP) {

		if (txt_nr != -1 && _G(flags).ShowAtsInvTxt) {

			switch (txt_mode) {
			case INVENTAR_NORMAL:
			case INVENTAR_STATIC:
				str_adr = _G(atds)->ats_get_txt(txt_nr, TXT_MARK_NAME, &anz, ATS_DATEI);
				break;
			default:
				break;
			}

			if (str_adr) {
				_G(fontMgr)->setFont(_G(font8));
				int16 x = _G(minfo).x;
				int16 y = _G(minfo).y;
				calc_txt_xy(&x, &y, str_adr, anz);
				for (int16 i = 0; i < anz; i++)
					printShadowed(x, y + i * 10, 255, 300, 0, _G(scr_width), _G(txt)->str_pos(str_adr, i));
			}
		}
	}

	if (_G(maus_links_click)) {
		if (_G(menu_item) != CUR_USE)
			Dialogs::Inventory::look_screen(txt_mode, txt_nr);
		else {
			if (_G(spieler).inv_cur) {
				obj_auswerten(nr, txt_mode);
			} else {
				if (txt_mode == INVENTAR_NORMAL) {
					if (!_G(flags).ChAutoMov) {
						_G(maus_links_click) = false;
						autoMove(_G(spieler).room_m_obj[nr].AutoMov, P_CHEWY);
						Dialogs::Inventory::look_screen(txt_mode, txt_nr);

						if (_G(spieler).AkInvent != -1)
							_G(spieler).room_m_obj[_G(spieler).AkInvent].RoomNr = -1;
						if (_G(spieler).room_m_obj[nr].AniFlag == 255) {
							invent_2_slot(nr);
						} else {
							_G(spieler).PersonHide[P_CHEWY] = _G(spieler).room_m_obj[nr].HeldHide;
							play_scene_ani(_G(spieler).room_m_obj[nr].AniFlag, ANI_FRONT);
							invent_2_slot(nr);

							_G(spieler).PersonHide[P_CHEWY] = false;
						}

						_G(spieler).inv_cur = false;
						_G(menu_item) = CUR_WALK;
						cursorChoice(_G(menu_item));
						_G(spieler_vector)[P_CHEWY].DelayCount = 0;

						if (_G(spieler).AkInvent != -1)
							_G(spieler).room_m_obj[_G(spieler).AkInvent].RoomNr = 255;

					}
				} else if (txt_mode == INVENTAR_STATIC) {
					obj_auswerten(nr, STATIC_USE);
				}
			}
		}
	}
}

void kb_mov(int16 mode) {
	bool ende = false;
	while (!ende) {
		switch (_G(in)->get_switch_code()) {
		case Common::KEYCODE_RIGHT:
			if (_G(minfo).x < 320 - _G(spieler)._curWidth)
				_G(minfo).x += 2;
			break;

		case Common::KEYCODE_LEFT:
			if (_G(minfo).x > 1)
				_G(minfo).x -= 2;
			break;

		case Common::KEYCODE_UP:
			if (_G(minfo).y > 1)
				_G(minfo).y -= 2;
			break;

		case Common::KEYCODE_DOWN:
			if (_G(minfo).y < 210 - _G(spieler)._curHeight)
				_G(minfo).y += 2;
			break;

		default:
			ende = true;
			break;

		}
		_G(cur)->move(_G(minfo).x, _G(minfo).y);
		if (mode)
			ende = true;
		else
			set_up_screen(DO_SETUP);
		SHOULD_QUIT_RETURN;
	}
}

void kb_cur_action(int16 key, int16 mode) {
	switch (key) {
	case Common::KEYCODE_RIGHT:
		if (_G(menu_display) == MENU_EINBLENDEN) {
			if (_G(menu_item) < 5)
				++_G(menu_item);
			else
				_G(menu_item) = CUR_WALK;
			_G(maus_menu_x) = (_G(menu_item)) * (MAUS_MENU_MAX_X / 5);
			_G(cur)->move(_G(maus_menu_x), 100);
		}
		break;

	case Common::KEYCODE_LEFT:
		if (_G(menu_display) == MENU_EINBLENDEN) {
			if (_G(menu_item) > 0)
				--_G(menu_item);
			else
				_G(menu_item) = CUR_INVENT;
			_G(maus_menu_x) = (_G(menu_item)) * (MAUS_MENU_MAX_X / 5);
			_G(cur)->move(_G(maus_menu_x), 100);
		}
		break;

	case Common::KEYCODE_UP:
		if (_G(menu_display) == MENU_EINBLENDEN) {
			if (_G(spieler).MainMenuY > 1)
				_G(spieler).MainMenuY -= 2;
		}
		break;

	case Common::KEYCODE_DOWN:
		if (_G(menu_display) == MENU_EINBLENDEN) {
			if (_G(spieler).MainMenuY < 163)
				_G(spieler).MainMenuY += 2;
		}
		break;

	default:
		break;
	}
}

void mouseAction() {
	int16 x = _G(minfo).x;
	int16 y = _G(minfo).y;
	if (x > invent_display[_G(spieler).InvDisp][0] &&
	        x < invent_display[_G(spieler).InvDisp][0] + 48 &&
	        y > invent_display[_G(spieler).InvDisp][1] &&
	        y < invent_display[_G(spieler).InvDisp][1] + 48) {
		if (!_G(spieler).inv_cur && !_G(inv_disp_ok) && _G(spieler).AkInvent != -1) {
			cursorChoice(CUR_USE);
		}
		_G(inv_disp_ok) = true;
	} else {
		if (!_G(spieler).inv_cur && _G(inv_disp_ok)) {
			cursorChoice(_G(menu_item));
		}
		_G(inv_disp_ok) = false;
	}
	if (_G(atds)->aad_get_status() == -1) {
		if (_G(minfo).button || _G(kbinfo).key_code == Common::KEYCODE_ESCAPE || _G(kbinfo).key_code == Common::KEYCODE_RETURN) {

			if (_G(minfo).button == 2 || _G(kbinfo).key_code == Common::KEYCODE_ESCAPE) {
				if (!_G(flags).main_maus_flag) {
					_G(kbinfo).scan_code = Common::KEYCODE_ESCAPE;
				}
			} else if (_G(minfo).button == 1 || _G(kbinfo).key_code == Common::KEYCODE_RETURN) {
				if (!_G(flags).main_maus_flag) {
					if (_G(menu_display) == MENU_EINBLENDEN)
						_G(kbinfo).scan_code = Common::KEYCODE_RETURN;
					else if (_G(spieler).AkInvent != -1) {
						if (_G(inv_disp_ok)) {
							if (_G(spieler).inv_cur) {
								_G(menu_item) = CUR_USE;
								cursorChoice(_G(menu_item));
								_G(spieler).inv_cur = false;
							} else {
								_G(menu_item) = CUR_USE;
								cursorChoice(CUR_AK_INVENT);
								_G(kbinfo).scan_code = Common::KEYCODE_INVALID;
							}
						} else if (!_G(flags).MausLinks)
							_G(maus_links_click) = true;
					} else if (!_G(flags).MausLinks)
						_G(maus_links_click) = true;
				}
			}
			_G(flags).main_maus_flag = 1;
		} else
			_G(flags).main_maus_flag = 0;
	}
}

void obj_auswerten(int16 test_nr, int16 mode) {
	int16 ani_nr;
	int16 txt_nr;
	int16 tmp = _G(spieler).AkInvent;
	int16 ret = NO_ACTION;
	int16 action_flag = false;

	switch (mode) {
	case INVENTAR_NORMAL:
		ret = _G(obj)->action_iib_iib(_G(spieler).AkInvent, test_nr);
		if (ret != NO_ACTION) {
			hideCur();
			if (_G(flags).InventMenu == false) {
				if (_G(spieler).room_m_obj[_G(spieler).AkInvent].AutoMov != 255) {
					_G(maus_links_click) = false;
					autoMove(_G(spieler).room_m_obj[test_nr].AutoMov, P_CHEWY);
				}
				txt_nr = _G(obj)->iib_txt_nr(test_nr);
				Dialogs::Inventory::look_screen(INVENTAR_NORMAL, txt_nr);
				if (_G(spieler).room_m_obj[test_nr].AniFlag != 255) {
					_G(spieler).PersonHide[P_CHEWY] = _G(spieler).room_m_obj[test_nr].HeldHide;
					play_scene_ani(_G(spieler).room_m_obj[test_nr].AniFlag, ANI_FRONT);
					_G(spieler).PersonHide[P_CHEWY] = false;
				}
			}
			showCur();
		}
		break;

	case INVENTAR_STATIC:
		ret = _G(obj)->action_iib_sib(_G(spieler).AkInvent, test_nr);
		if (ret != NO_ACTION) {
			_G(maus_links_click) = false;
			hideCur();
			if (_G(spieler).room_m_obj[_G(spieler).AkInvent].AutoMov != 255) {
				autoMove(_G(spieler).room_s_obj[test_nr].AutoMov, P_CHEWY);
			}
			txt_nr = _G(obj)->sib_txt_nr(test_nr);
			Dialogs::Inventory::look_screen(INVENTAR_STATIC, txt_nr);
			if (_G(spieler).room_s_obj[test_nr].AniFlag != 255) {
				_G(spieler).PersonHide[P_CHEWY] = _G(spieler).room_s_obj[test_nr].HeldHide;
				tmp = getAniDirection((int16)_G(spieler).room_s_obj[test_nr].ZustandAk);
				ani_nr = _G(spieler).room_s_obj[test_nr].AniFlag;

				if (ani_nr >= 150) {
					start_spz_wait(ani_nr - 150, 1, false, P_CHEWY);
					ani_nr = -1;
				} else if (ani_nr >= 100) {
					ani_nr -= 100;
					_G(obj)->calc_static_detail(test_nr);
				}
				if (ani_nr != -1)
					play_scene_ani(ani_nr, tmp);
				_G(spieler).PersonHide[P_CHEWY] = false;
			}
			_G(menu_item_vorwahl) = CUR_WALK;
			showCur();
			sib_event_inv(test_nr);

			if (!_G(spieler).inv_cur) {
				_G(menu_item) = _G(menu_item_vorwahl);
				cursorChoice(_G(menu_item));
			}
			_G(obj)->calc_all_static_detail();
		}
		break;

	case STATIC_USE:
		ret = _G(obj)->calc_static_use(test_nr);
		if (ret == OBJECT_1) {
			_G(maus_links_click) = false;
			hideCur();
			if (_G(spieler).room_s_obj[test_nr].AutoMov != 255) {

				autoMove(_G(spieler).room_s_obj[test_nr].AutoMov, P_CHEWY);
			}
			txt_nr = _G(obj)->sib_txt_nr(test_nr);
			Dialogs::Inventory::look_screen(INVENTAR_STATIC, txt_nr);
			if (_G(spieler).room_s_obj[test_nr].AniFlag != 255) {
				_G(spieler).PersonHide[P_CHEWY] = _G(spieler).room_s_obj[test_nr].HeldHide;
				tmp = getAniDirection((int16)_G(spieler).room_s_obj[test_nr].ZustandAk);

				ani_nr = _G(spieler).room_s_obj[test_nr].AniFlag;

				if (ani_nr >= 150) {
					start_spz_wait(ani_nr - 150, 1, false, P_CHEWY);
					ani_nr = -1;
				} else if (ani_nr >= 100) {
					ani_nr -= 100;
					_G(obj)->calc_static_detail(test_nr);
				}
				if (ani_nr != -1)
					play_scene_ani(ani_nr, tmp);
				_G(spieler).PersonHide[P_CHEWY] = false;
			}

			if (_G(spieler).room_s_obj[test_nr].InvNr != -1) {
				invent_2_slot(_G(spieler).room_s_obj[test_nr].InvNr);
				action_flag = true;
			}
			_G(menu_item_vorwahl) = CUR_WALK;
			showCur();
			int16 sib_ret = sib_event_no_inv(test_nr);

			_G(obj)->calc_all_static_detail();

			if (!_G(spieler).inv_cur) {

				if (sib_ret || action_flag) {
					_G(menu_item) = _G(menu_item_vorwahl);
					cursorChoice(_G(menu_item));
				}
			}
		} else if (ret == SIB_GET_INV) {
			_G(maus_links_click) = false;
			hideCur();
			if (_G(spieler).room_s_obj[test_nr].AutoMov != 255) {
				autoMove(_G(spieler).room_s_obj[test_nr].AutoMov, P_CHEWY);
			}
			txt_nr = _G(obj)->sib_txt_nr(test_nr);
			Dialogs::Inventory::look_screen(INVENTAR_STATIC, txt_nr);
			if (_G(spieler).room_s_obj[test_nr].AniFlag != 255) {
				_G(spieler).PersonHide[P_CHEWY] = _G(spieler).room_s_obj[test_nr].HeldHide;
				tmp = getAniDirection((int16)_G(spieler).room_s_obj[test_nr].ZustandAk);

				ani_nr = _G(spieler).room_s_obj[test_nr].AniFlag;

				if (ani_nr >= 150) {
					start_spz_wait(ani_nr - 150, 1, false, P_CHEWY);
					ani_nr = -1;
				} else if (ani_nr >= 100) {
					ani_nr -= 100;
					_G(obj)->calc_static_detail(test_nr);
				}
				if (ani_nr != -1) {
					play_scene_ani(ani_nr, tmp);
				}
				_G(spieler).PersonHide[P_CHEWY] = false;
			}

			if (_G(spieler).room_s_obj[test_nr].InvNr != -1)
				invent_2_slot(_G(spieler).room_s_obj[test_nr].InvNr);
			_G(obj)->calc_rsi_flip_flop(test_nr);
			_G(menu_item_vorwahl) = CUR_WALK;
			showCur();
			sib_event_no_inv(test_nr);
			_G(obj)->calc_all_static_detail();
			if (!_G(spieler).inv_cur) {
				_G(menu_item) = _G(menu_item_vorwahl);
				cursorChoice(_G(menu_item));
			}
		} else if (ret == NO_ACTION) {
			txt_nr = _G(obj)->sib_txt_nr(test_nr);
			Dialogs::Inventory::look_screen(INVENTAR_STATIC, txt_nr);
		}
		break;

	default:
		break;
	}

	switch (ret) {
	case OBJECT_1:
	case OBJECT_2:
		if (mode == INVENTAR_NORMAL)
			calc_inv_use_txt(test_nr);
		break;

	case NO_ACTION:
		if (mode == STATIC_USE && _G(flags).StaticUseTxt == true)
			_G(flags).StaticUseTxt = false;
		else if (mode != STATIC_USE)
			calc_inv_no_use(test_nr, mode);

		break;

	default:
		break;
	}
}

void swap_if_l(int16 *x1, int16 *x2) {
	if (abs(*x1) < abs(*x2)) {
		int16 x1_s;
		if (*x1 < 0)
			x1_s = 0;
		else
			x1_s = 1;
		*x1 = abs(*x2);
		if (!x1_s)
			*x1 = -*x1;
	}
}

void palcopy(byte *destPal, const byte *srcPal, int16 destStartIndex, int16 srcStartIndex, int16 colorNbr) {
	int16 j = srcStartIndex;
	for (int16 i = destStartIndex; i < destStartIndex + colorNbr; ++i) {
		destPal[i * 3] = srcPal[j * 3];
		destPal[i * 3 + 1] = srcPal[j * 3 + 1];
		destPal[i * 3 + 2] = srcPal[j * 3 + 2];
		++j;
	}
}

void check_shad(int16 g_idx, int16 mode) {
	static const uint8 PAL_0[] = {
		0, 0, 0,
		39, 0, 26,
		43, 0, 29,
		51, 42, 29,
		51, 0, 34,
		49, 13, 34,
		55, 0, 37,
		63, 54, 40,
		63, 0, 42,
		63, 30, 42,
		63, 12, 46,
		63, 24, 50
	};
	static const uint8 PAL_1[] = {
		0, 0, 0,
		34, 0, 21,
		38, 0, 24,
		51, 42, 29,

		46, 0, 29,
		44, 8, 29,
		50, 0, 32,
		63, 54, 40,

		58, 0, 37,
		58, 25, 37,
		58, 7, 41,
		58, 19, 45
	};
	static const uint8 PAL_2[] = {
		0, 0, 0,
		26, 0, 13,
		30, 0, 16,
		51, 42, 29,

		38, 0, 21,
		36, 0, 21,
		42, 0, 24,
		63, 54, 40,

		50, 0, 29,
		50, 17, 29,
		50, 0, 33,
		50, 11, 37
	};
	static const uint8 PAL_3[] = {
		0, 0, 0,
		21, 3, 8,
		25, 3, 11,
		51, 42, 29,

		33, 3, 16,
		31, 3, 16,
		37, 3, 19,
		63, 54, 40,

		45, 3, 24,
		45, 12, 24,
		45, 3, 28,
		45, 6, 32
	};

	switch (g_idx) {
	case 1:
		if (mode)
			_G(out)->set_teilpalette(PAL_1, 1, 11);
		palcopy(_G(pal), PAL_1, 0, 0, 12);
		break;

	case 2:
		if (mode)
			_G(out)->set_teilpalette(PAL_2, 1, 11);
		palcopy(_G(pal), PAL_2, 0, 0, 12);
		break;

	case 3:
		if (mode)
			_G(out)->set_teilpalette(PAL_3, 1, 11);
		palcopy(_G(pal), PAL_3, 0, 0, 12);
		break;

	case 4:
		if (mode)
			_G(out)->set_teilpalette(PAL_0, 1, 11);
		palcopy(_G(pal), PAL_0, 0, 0, 12);
		break;

	default:
		break;
	}
}

void printShadowed(int16 x, int16 y, int16 fgCol, int16 bgCol, int16 shadowFgCol, int16 scrWidth, char *txtPtr) {
	_G(out)->printxy(x + 1, y + 1, shadowFgCol, bgCol, scrWidth, txtPtr);
	_G(out)->printxy(x, y, fgCol, bgCol, scrWidth, txtPtr);
}

bool autoMove(int16 movNr, int16 playerNum) {
	bool movingFl = false;
	if (movNr < MAX_AUTO_MOV) {
		if (!_G(flags).ChAutoMov) {
			int16 key = false;
			movingFl = true;
			_G(flags).ChAutoMov = true;
			_G(auto_p_nr) = playerNum;
			int16 tmp = _G(maus_links_click);
			_G(maus_links_click) = false;
			_G(gpkt).Dx = _G(Rdi)->AutoMov[movNr].X -
						  _G(spieler_mi)[playerNum].HotMovX + _G(spieler_mi)[playerNum].HotX;
			_G(gpkt).Dy = _G(Rdi)->AutoMov[movNr].Y -
						  _G(spieler_mi)[playerNum].HotMovY + _G(spieler_mi)[playerNum].HotY;
			_G(gpkt).Sx = _G(spieler_vector)[playerNum].Xypos[0] + _G(spieler_mi)[playerNum].HotX;
			_G(gpkt).Sy = _G(spieler_vector)[playerNum].Xypos[1] + _G(spieler_mi)[playerNum].HotY;
			_G(gpkt).Breite = _G(room)->_gedXAnz[_G(room_blk).AkAblage];
			_G(gpkt).Hoehe = _G(room)->_gedYAnz[_G(room_blk).AkAblage];
			_G(gpkt).Mem = _G(ged_mem)[_G(room_blk).AkAblage];
			_G(gpkt).Ebenen = _G(room)->_gedInfo[_G(room_blk).AkAblage].Ebenen;
			_G(gpkt).AkMovEbene = _G(ged_mov_ebene);
			_G(mov)->goto_xy(&_G(gpkt));

			_G(spieler_mi)[playerNum].XyzStart[0] = _G(spieler_vector)[playerNum].Xypos[0];
			_G(spieler_mi)[playerNum].XyzStart[1] = _G(spieler_vector)[playerNum].Xypos[1];
			_G(spieler_mi)[playerNum].XyzEnd[0] = _G(gpkt).Dx - _G(spieler_mi)[playerNum].HotX;
			_G(spieler_mi)[playerNum].XyzEnd[1] = _G(gpkt).Dy - _G(spieler_mi)[playerNum].HotY;
			_G(mov)->get_mov_vector((int16 *)_G(spieler_mi)[playerNum].XyzStart,
				_G(spieler_mi)[playerNum].Vorschub, &_G(spieler_vector)[playerNum]);
			get_phase(&_G(spieler_vector)[playerNum], &_G(spieler_mi)[playerNum]);
			_G(spieler_vector)[playerNum].DelayCount = 0;

			if (_G(mov)->auto_go_status()) {
				while (_G(mov)->auto_go_status()) {
					if (SHOULD_QUIT)
						return 0;
					if (_G(in)->get_switch_code() == Common::KEYCODE_ESCAPE) {
						if (_G(flags).ExitMov || _G(flags).BreakAMov) {
							key = Common::KEYCODE_ESCAPE;
							_G(mov)->stop_auto_go();
							movingFl = false;
						}
					}
					set_up_screen(DO_SETUP);
				}
			}
			if (_G(flags).ChAutoMov) {
				bool endLoopFl = false;
				_G(spieler_mi)[playerNum].XyzStart[0] = _G(spieler_vector)[playerNum].Xypos[0];
				_G(spieler_mi)[playerNum].XyzStart[1] = _G(spieler_vector)[playerNum].Xypos[1];
				_G(spieler_mi)[playerNum].XyzEnd[0] = _G(gpkt).Dx - _G(spieler_mi)[playerNum].HotX;
				_G(spieler_mi)[playerNum].XyzEnd[1] = _G(gpkt).Dy - _G(spieler_mi)[playerNum].HotY;
				_G(mov)->get_mov_vector((int16 *)_G(spieler_mi)[playerNum].XyzStart, _G(spieler_mi)[playerNum].Vorschub, &_G(spieler_vector)[playerNum]);
				get_phase(&_G(spieler_vector)[playerNum], &_G(spieler_mi)[playerNum]);
				while (!endLoopFl) {
					if (_G(in)->get_switch_code() == Common::KEYCODE_ESCAPE || key == Common::KEYCODE_ESCAPE) {
						if (_G(flags).ExitMov || _G(flags).BreakAMov) {
							_G(spieler_vector)[playerNum].Count = 0;
							movingFl = false;
						}
					}
					if (!_G(spieler_vector)[playerNum].Count) {
						if (movingFl) {
							if (_G(flags).ExitMov == false && _G(flags).ChAutoMov) {

								set_person_pos(_G(spieler_mi)[playerNum].XyzEnd[0],
								               _G(spieler_mi)[playerNum].XyzEnd[1],
								               playerNum, _G(Rdi)->AutoMov[movNr].SprNr);
							}
						}
						endLoopFl = true;
					}
					set_up_screen(DO_SETUP);
					SHOULD_QUIT_RETURN0;
				}
			}
			_G(auto_p_nr) = P_CHEWY;
			_G(maus_links_click) = tmp;
			_G(flags).ChAutoMov = false;
		}
	}

	return movingFl;
}

void goAutoXy(int16 x, int16 y, int16 personNum, int16 mode) {
	if (!_G(stopAutoMove)[personNum] && _G(spieler).PersonRoomNr[personNum] == _G(spieler).PersonRoomNr[P_CHEWY]) {
		int16 move_status = true;
		int16 tmp = _G(maus_links_click);
		_G(maus_links_click) = false;

		_G(spieler_mi)[personNum].XyzStart[0] = _G(spieler_vector)[personNum].Xypos[0];
		_G(spieler_mi)[personNum].XyzStart[1] = _G(spieler_vector)[personNum].Xypos[1];
		_G(spieler_mi)[personNum].XyzEnd[0] = x;
		_G(spieler_mi)[personNum].XyzEnd[1] = y;
		_G(mov)->get_mov_vector((int16 *)_G(spieler_mi)[personNum].XyzStart,
			_G(spieler_mi)[personNum].Vorschub, &_G(spieler_vector)[personNum]);

		if (_G(spieler_vector)[personNum].Count)
			get_phase(&_G(spieler_vector)[personNum], &_G(spieler_mi)[personNum]);
		if (mode == ANI_WAIT) {
			bool endLoopFl = false;
			while (!endLoopFl) {
				if (_G(in)->get_switch_code() == Common::KEYCODE_ESCAPE) {
					if (_G(flags).ExitMov || _G(flags).BreakAMov) {
						_G(spieler_vector)[personNum].Count = 0;
						move_status = false;
					}
				}
				if (!_G(spieler_vector)[personNum].Count) {
					if (move_status) {
						set_person_pos(_G(spieler_mi)[personNum].XyzEnd[0],
						               _G(spieler_mi)[personNum].XyzEnd[1], personNum, -1);
					}
					endLoopFl = true;
				}
				set_up_screen(DO_SETUP);
				SHOULD_QUIT_RETURN;
			}
		}

		_G(maus_links_click) = tmp;
	}
}

int16 getAniDirection(int16 zustand) {
	int16 ret = ANI_FRONT;
	switch (zustand) {
	case OBJZU_ZU:
	case OBJZU_VERSCHLOSSEN:
		ret = ANI_BACK;
		break;
	default:
		break;
	}
	return ret;
}

int16 calcMouseText(int16 x, int16 y, int16 mode) {
	int16 txtMode = 0;
	int16 ret = -1;

	if (mode == DO_SETUP) {
		if (_G(flags).ShowAtsInvTxt) {
			int16 inv_no_use_mode = AUTO_OBJ;
			int16 idx;
			int16 txtNr = calc_mouse_mov_obj(&idx);

			if (txtNr == -1) {
				idx = _G(det)->maus_vector(x + _G(spieler).scrollx, y + _G(spieler).scrolly);
				if (idx != -1) {
					txtNr = _G(Rdi)->mtxt[idx];
					inv_no_use_mode = DETEDIT_REC;
				}
			}

			if (txtNr != -1) {
				ret = -1;
				bool dispFl = true;
				bool actionFl = false;

				if (_G(maus_links_click) && !_G(flags).MausTxt) {
					int16 ok = true;
					_G(flags).MausTxt = true;

					switch (_G(menu_item)) {
					case CUR_LOOK:
						txtMode = TXT_MARK_LOOK;
						break;

					case CUR_NICHELLE:
					case CUR_HOWARD:
					case CUR_USER:
					case CUR_USE:
						txtMode = TXT_MARK_USE;
						if (_G(spieler).inv_cur)
							ok = false;
						break;

					case CUR_WALK:
						txtMode = TXT_MARK_WALK;
						break;

					case CUR_TALK:
						txtMode = TXT_MARK_TALK;
						break;

					default:
						break;
					}

					int16 action_ret = 0;
					if (!_G(atds)->get_steuer_bit(txtNr, ATS_AKTIV_BIT, ATS_DATEI)) {
						if (_G(menu_item) != CUR_WALK && _G(menu_item) != CUR_USE) {
							if (x + _G(spieler).scrollx > _G(spieler_vector)[P_CHEWY].Xypos[0])
								set_person_spr(P_RIGHT, P_CHEWY);
							else
								set_person_spr(P_LEFT, P_CHEWY);
						}
					}

					if (_G(atds)->get_steuer_bit(txtNr, ATS_ACTION_BIT, ATS_DATEI)) {
						action_ret = ats_action(txtNr, txtMode, ATS_ACTION_VOR);
					}
					
					if (ok && !_G(atds)->get_steuer_bit(txtNr, ATS_AKTIV_BIT, ATS_DATEI)) {
						if (start_ats_wait(txtNr, txtMode, 14, ATS_DATEI))
							dispFl = false;
					} else {
						ret = -1;
					}
					
					if (_G(atds)->get_steuer_bit(txtNr, ATS_ACTION_BIT, ATS_DATEI)) {
						action_ret = ats_action(txtNr, txtMode, ATS_ACTION_NACH);
						actionFl = true;
						if (action_ret)
							ret = 1;
					}
					
					if (!ok && !action_ret) {
						if (inv_no_use_mode != -1 && !_G(atds)->get_steuer_bit(txtNr, ATS_AKTIV_BIT, ATS_DATEI)) {
							actionFl = calc_inv_no_use(idx + (_G(spieler).PersonRoomNr[P_CHEWY] * 100), inv_no_use_mode);
							if (actionFl)
								ret = txtNr;
						}
					}
					
					if (ok && !action_ret && txtMode == TXT_MARK_USE && dispFl) {
						if (!_G(atds)->get_steuer_bit(txtNr, ATS_AKTIV_BIT, ATS_DATEI)) {
							if (_G(menu_item) != CUR_WALK) {
								if (x + _G(spieler).scrollx > _G(spieler_vector)[P_CHEWY].Xypos[0])
									set_person_spr(P_RIGHT, P_CHEWY);
								else
									set_person_spr(P_LEFT, P_CHEWY);
							}
							int16 r_val = g_engine->_rnd.getRandomNumber(MAX_RAND_NO_USE - 1);
							actionFl = start_ats_wait(RAND_NO_USE[r_val], TXT_MARK_USE, 14, INV_USE_DEF);
							if (actionFl)
								ret = txtNr;
						}
					}

					_G(flags).MausTxt = false;
				} else {
					ret = -1;
				}

				if (dispFl && !actionFl) {
					int16 anz;
					char *str_ = _G(atds)->ats_get_txt(txtNr, TXT_MARK_NAME, &anz, ATS_DATEI);
					if (str_ != 0) {
						ret = txtNr;
						_G(fontMgr)->setFont(_G(font8));
						calc_txt_xy(&x, &y, str_, anz);
						for (int16 i = 0; i < anz; i++)
							printShadowed(x, y + i * 10, 255, 300, 0, _G(scr_width), _G(txt)->str_pos((char *)str_, i));
					}
				}
			} else {
				ret = -1;
			}
		}
	}

	return ret;
}

int16 is_mouse_person(int16 x, int16 y) {
	int16 *xy = nullptr;
	int16 is_person = -1;
	if (_G(flags).ShowAtsInvTxt) {

		for (int16 i = 0; i < MAX_PERSON && is_person == -1; i++) {
			if (_G(spieler_mi)[i].Id != NO_MOV_OBJ) {
				bool check = true;
				if (!_G(spz_ani)[i]) {
					switch (i) {
					case P_CHEWY:
						xy = (int16 *)_G(chewy)->image[_G(chewy_ph)[_G(spieler_vector)[P_CHEWY].Phase * 8 + _G(spieler_vector)[P_CHEWY].PhNr]];
						break;

					case P_HOWARD:
					case P_NICHELLE:
						if (_G(spieler).PersonRoomNr[i] != _G(spieler).PersonRoomNr[P_CHEWY])
							check = false;
						xy = (int16 *)_G(PersonTaf)[i]->image[_G(PersonSpr)[i][_G(spieler_vector)[i].PhNr]];
						break;

					default:
						break;
					}
				} else
					xy = (int16 *)_G(spz_tinfo)->image[_G(spz_spr_nr)[_G(spieler_vector)[i].PhNr]];
				if (check) {
					if (x + _G(spieler).scrollx >= _G(spieler_vector)[i].Xypos[0] &&
					        x + _G(spieler).scrollx <= _G(spieler_vector)[i].Xypos[0] + xy[0] + _G(spieler_vector)[i].Xzoom &&
					        y + _G(spieler).scrolly >= _G(spieler_vector)[i].Xypos[1] &&
					        y + _G(spieler).scrolly <= _G(spieler_vector)[i].Xypos[1] + xy[1] + _G(spieler_vector)[i].Yzoom) {
						is_person = i;
					}
				}
			}
		}
	}

	return is_person;
}

void calc_mouse_person(int16 x, int16 y) {
	int16 mode = 0;
	char ch_txt[MAX_PERSON][9] = {"Chewy", "Howard", "Nichelle"};
	int16 dia_nr = -1;
	if (_G(flags).ShowAtsInvTxt && !_G(flags).InventMenu) {
		int16 p_nr = is_mouse_person(x, y);
		if (p_nr != -1) {
			if (!_G(spieler).PersonHide[p_nr]) {
				_G(fontMgr)->setFont(_G(font8));
				char *str_ = ch_txt[p_nr];
				calc_txt_xy(&x, &y, str_, 1);
				printShadowed(x, y, 255, 300, 0, _G(scr_width), str_);
				if (_G(maus_links_click) == 1) {
					int16 def_nr = -1;
					if (!_G(spieler).inv_cur) {
						int16 txt_nr = calc_person_txt(p_nr);
						switch (_G(menu_item)) {
						case CUR_LOOK:
							mode = TXT_MARK_LOOK;
							switch (p_nr) {
							case P_CHEWY:
								def_nr = 18;
								break;

							case P_HOWARD:
								def_nr = 21;
								break;

							case P_NICHELLE:
								def_nr = 24;
								break;

							default:
								break;
							}
							break;

						case CUR_USE:
							mode = TXT_MARK_USE;
							switch (p_nr) {
							case P_CHEWY:
								def_nr = 19;
								break;

							case P_HOWARD:
								def_nr = 22;
								break;

							case P_NICHELLE:
								def_nr = 23;
								break;

							default:
								break;
							}
							break;

						case CUR_TALK:
							switch (p_nr) {
							case P_HOWARD:
							case P_NICHELLE:
								dia_nr = true;
								calc_person_dia(p_nr);
								break;

							default:
								def_nr = -1;
								txt_nr = -1;
								break;

							}
							break;

						default:
							def_nr = -1;
							txt_nr = -1;
							break;

						}
						if (dia_nr == -1) {
							if (txt_nr != 30000) {
								if (_G(menu_item) != CUR_WALK) {
									if (x + _G(spieler).scrollx > _G(spieler_vector)[P_CHEWY].Xypos[0])
										set_person_spr(P_RIGHT, P_CHEWY);
									else
										set_person_spr(P_LEFT, P_CHEWY);
								}
								if (!start_ats_wait(txt_nr, mode, 14, ATS_DATEI))
									start_ats_wait(def_nr, TXT_MARK_USE, 14, INV_USE_DEF);
							}
						}
					} else {
						if (_G(menu_item) != CUR_WALK) {
							if (x + _G(spieler).scrollx > _G(spieler_vector)[P_CHEWY].Xypos[0])
								set_person_spr(P_RIGHT, P_CHEWY);
							else
								set_person_spr(P_LEFT, P_CHEWY);
						}
						calc_inv_no_use(p_nr, SPIELER_OBJ);
					}
				}
			}
		}
	}
}

int16 calc_mouse_mov_obj(int16 *auto_nr) {
	int16 txt_nr = -1;
	*auto_nr = -1;
	int16 ok = 0;
	for (int16 i = 0; i < _G(auto_obj) && !ok; i++) {
		if (mouse_auto_obj(i, 0, 0)) {
			ok = 1;
			txt_nr = _G(mov_phasen)[i].AtsText;
			*auto_nr = i;
		}
	}

	return txt_nr;
}

void calc_ani_timer() {
	for (int16 i = _G(room)->_roomTimer.TimerStart;
	     i < _G(room)->_roomTimer.TimerStart + _G(room)->_roomTimer.TimerAnz; i++) {
		if (_G(ani_timer)[i].TimeFlag)
			timer_action(i);
	}
}

void get_user_key(int16 mode) {
	_G(flags).StopAutoObj = true;
	mouseAction();
	_G(maus_links_click) = false;

	if (!_G(inv_disp_ok)) {
		switch (_G(in)->get_switch_code()) {
		case Common::KEYCODE_F5:
		case Common::KEYCODE_SPACE:
		case Common::KEYCODE_ESCAPE:
			_G(maus_old_x) = _G(minfo).x;
			_G(maus_old_y) = _G(minfo).y;

			_G(tmp_menu_item) = _G(menu_item);
			_G(menu_item) = CUR_USE;
			Dialogs::Inventory::menu();
			_G(menu_flag) = MENU_AUSBLENDEN;
			_G(menu_display) = 0;
			_G(cur_display) = true;
			if (_G(spieler).AkInvent == -1) {
				_G(menu_item) = _G(tmp_menu_item);
				cursorChoice(_G(menu_item));
				_G(spieler).inv_cur = false;
			} else {
				_G(menu_item) = CUR_USE;
				cursorChoice(CUR_AK_INVENT);

				get_display_xy(&_G(spieler).DispZx, &_G(spieler).DispZy, _G(spieler).AkInvent);
			}

			_G(kbinfo).key_code = '\0';
			break;

		default:
			break;
		}
	}

	_G(flags).StopAutoObj = false;
}

void clear_prog_ani() {
	for (int16 i = 0; i < MAX_PROG_ANI; i++) {
		_G(spr_info)[i].Image = nullptr;
		_G(spr_info)[i].ZEbene = 255;
	}
}

void set_ani_screen() {
	if (_G(flags).AniUserAction)
		get_user_key(NO_SETUP);
	set_up_screen(DO_SETUP);
}

void del_inventar(int16 nr) {
	_G(obj)->delInventory(nr, &_G(room_blk));
	_G(spieler).inv_cur = false;
	_G(menu_item) = CUR_WALK;
	_G(spieler).AkInvent = -1;
	cursorChoice(_G(menu_item));
	del_invent_slot(nr);
	_G(maus_links_click) = false;
}

bool is_cur_inventar(int16 nr) {
	int16 ret = false;

	if (_G(spieler).AkInvent == nr && _G(spieler).inv_cur)
		ret = true;

	return ret;
}

void check_mouse_ausgang(int16 x, int16 y) {
	if (_G(menu_item) == CUR_WALK) {
		bool found = true;
		int16 nr = _G(obj)->is_exit(x, y);
		int16 attr = (nr >= 0 && nr < MAX_EXIT) ? _G(spieler).room_e_obj[nr].Attribut : 0;

		switch (attr) {
		case AUSGANG_LINKS:
			_G(cur_ausgang_flag) = AUSGANG_LINKS;
			cursorChoice(CUR_AUSGANG_LINKS);
			break;

		case AUSGANG_RECHTS:
			_G(cur_ausgang_flag) = AUSGANG_RECHTS;
			cursorChoice(CUR_AUSGANG_RECHTS);
			break;

		case AUSGANG_OBEN:
			_G(cur_ausgang_flag) = AUSGANG_OBEN;
			cursorChoice(CUR_AUSGANG_OBEN);
			break;

		case AUSGANG_UNTEN:
			_G(cur_ausgang_flag) = AUSGANG_UNTEN;
			cursorChoice(CUR_AUSGANG_UNTEN);
			break;

		default:
			found = false;
			break;
		}

		if (_G(cur_ausgang_flag) && !found) {
			cursorChoice(_G(menu_item));
			_G(cur_ausgang_flag) = false;
		}
	}
}

void calc_ausgang(int16 x, int16 y) {
	if (!_G(flags).ExitMov) {
		_G(maus_links_click) = false;
		int16 nr = _G(obj)->is_exit(x, y);
		if (nr != -1) {
			_G(flags).ExitMov = true;
			if (autoMove(_G(spieler).room_e_obj[nr].AutoMov, P_CHEWY) == true) {
				_G(flags).ShowAtsInvTxt = false;
				_G(menu_item) = CUR_DISK;
				cursorChoice(CUR_DISK);
				set_up_screen(DO_SETUP);
				_G(cur_hide_flag) = true;
				exit_room(nr);
				_G(spieler).PersonRoomNr[P_CHEWY] = _G(spieler).room_e_obj[nr].Exit;
				_G(room)->load_room(&_G(room_blk), _G(spieler).PersonRoomNr[P_CHEWY], &_G(spieler));
				set_person_pos(_G(Rdi)->AutoMov[_G(spieler).room_e_obj[nr].ExitMov].X -
				               _G(spieler_mi)[_G(auto_p_nr)].HotMovX,
				               _G(Rdi)->AutoMov[_G(spieler).room_e_obj[nr].ExitMov].Y - _G(spieler_mi)[_G(auto_p_nr)].HotMovY
				               , P_CHEWY, -1);
				int16 *ScrXy = (int16 *)_G(ablage)[_G(room_blk).AkAblage];
				get_scroll_off(_G(spieler_vector)[P_CHEWY].Xypos[0] + _G(spieler_mi)[P_CHEWY].HotX,
				               _G(spieler_vector)[P_CHEWY].Xypos[1] + _G(spieler_mi)[P_CHEWY].HotY,
				               ScrXy[0], ScrXy[1],
				               &_G(spieler).scrollx, &_G(spieler).scrolly);

				int16 u_idx = _G(ged)->ged_idx(_G(spieler_vector)[P_CHEWY].Xypos[0] + _G(spieler_mi)[P_CHEWY].HotX,
				                               _G(spieler_vector)[P_CHEWY].Xypos[1] + _G(spieler_mi)[P_CHEWY].HotY,
				                               _G(room)->_gedXAnz[_G(room_blk).AkAblage],
				                               _G(ged_mem)[_G(room_blk).AkAblage]);
				check_shad(u_idx, 0);
				set_person_spr(_G(Rdi)->AutoMov[_G(spieler).room_e_obj[nr].ExitMov].SprNr, P_CHEWY);
				_G(spieler_vector)[P_CHEWY].DelayCount = 0;
				_G(fx_blend) = BLEND1;
				_G(auto_obj) = 0;
				_G(spieler_vector)[P_CHEWY].Xzoom = 0;
				_G(spieler_vector)[P_CHEWY].Yzoom = 0;
				_G(flags).ShowAtsInvTxt = true;
				_G(menu_item) = CUR_WALK;
				cursorChoice(_G(menu_item));
				_G(flags).ExitMov = false;
				enter_room(nr);
			}
			_G(flags).ExitMov = false;
			set_up_screen(DO_SETUP);
		}
	}
}

void get_scroll_off(int16 x, int16 y, int16 pic_x, int16 pic_y, int16 *sc_x, int16 *sc_y) {
	if (pic_x == SCREEN_WIDTH)
		*sc_x = 0;
	else {
		if ((pic_x - x) > (SCREEN_WIDTH / 2)) {
			*sc_x = (((pic_x - 1) / SCREEN_WIDTH) - 1) * SCREEN_WIDTH;
			if (!*sc_x && x > (SCREEN_WIDTH / 2)) {

				*sc_x = x - (SCREEN_WIDTH / 2);
			}
		} else {
			*sc_x = pic_x - SCREEN_WIDTH;
		}
	}

	if (pic_y == SCREEN_HEIGHT)
		*sc_y = 0;
	else {
		if ((pic_y - y) > (SCREEN_HEIGHT / 2)) {
			*sc_y = (((pic_y - 1) / SCREEN_HEIGHT) - 1) * SCREEN_HEIGHT;
			if (!*sc_y && y > (SCREEN_HEIGHT / 2))

				*sc_y = y - (SCREEN_HEIGHT / 2);
		} else {
			*sc_y = pic_y - SCREEN_HEIGHT;
		}
	}
}

void calc_scroll(int16 x, int16 y, int16 pic_x, int16 pic_y, int16 *sc_x, int16 *sc_y) {
	if (!_G(flags).NoScroll) {
		if (!_G(scroll_delay)) {

			if ((_G(spieler).ScrollxStep * _G(spieler).DelaySpeed) > CH_X_PIX)
				_G(scroll_delay) = CH_X_PIX / _G(spieler).ScrollxStep;

			if (x - *sc_x < SCROLL_LEFT) {
				if ((*sc_x - _G(spieler).ScrollxStep) > 0) {
					*sc_x -= _G(spieler).ScrollxStep;
				}
			} else if (x - *sc_x > SCROLL_RIGHT) {
				if ((*sc_x + _G(spieler).ScrollxStep) < pic_x - SCREEN_WIDTH) {
					*sc_x += _G(spieler).ScrollxStep;
				}
			}

			if (y - *sc_y < SCROLL_UP) {
				if ((*sc_y - _G(spieler).ScrollyStep) > 0) {
					*sc_y -= _G(spieler).ScrollyStep;
				}
			} else if (y - *sc_y > SCROLL_DOWN) {
				if ((*sc_y + _G(spieler).ScrollyStep) < pic_y - SCREEN_HEIGHT) {
					*sc_y += _G(spieler).ScrollyStep;
				}
			}
		} else
			--_G(scroll_delay);
	}
}

void auto_scroll(int16 scrx, int16 scry) {
	int16 tmpMouseClick = _G(maus_links_click);
	_G(maus_links_click) = false;
	_G(spieler).scrollx >>= 1;
	_G(spieler).scrollx <<= 1;
	_G(spieler).scrolly >>= 1;
	_G(spieler).scrolly <<= 1;
	bool endLoopFl = false;
	while (!endLoopFl) {
		if (scrx < _G(spieler).scrollx)
			_G(spieler).scrollx -= _G(spieler).ScrollxStep;
		else if (scrx > _G(spieler).scrollx)
			_G(spieler).scrollx += _G(spieler).ScrollxStep;
		if (scry < _G(spieler).scrolly)
			_G(spieler).scrolly -= _G(spieler).ScrollyStep;
		else if (scry > _G(spieler).scrolly)
			_G(spieler).scrolly += _G(spieler).ScrollyStep;
		if (scrx == _G(spieler).scrollx && scry == _G(spieler).scrolly)
			endLoopFl = true;
		set_up_screen(DO_SETUP);
		SHOULD_QUIT_RETURN;
	}
	_G(maus_links_click) = tmpMouseClick;
}

void disable_timer() {
	_G(uhr)->disable_timer();
	_G(FrameSpeedTmp) = _G(FrameSpeed);
}

void enable_timer() {
	_G(uhr)->enable_timer();
	_G(FrameSpeed) = _G(FrameSpeedTmp);
}

void calc_auto_go() {
	int16 x_offset = _G(spieler_mi)[_G(auto_p_nr)].HotMovX + (_G(spieler_mi)[_G(auto_p_nr)].HotX - _G(spieler_mi)[_G(auto_p_nr)].HotMovX - 1);
	int16 y_offset = _G(spieler_mi)[_G(auto_p_nr)].HotMovY + (_G(spieler_mi)[_G(auto_p_nr)].HotY - _G(spieler_mi)[_G(auto_p_nr)].HotMovY - 2);
	if (_G(mov)->calc_auto_go(_G(spieler_mi)[_G(auto_p_nr)].XyzStart[0] + x_offset,
	                       _G(spieler_mi)[_G(auto_p_nr)].XyzStart[1] + y_offset,
	                       &_G(spieler_mi)[_G(auto_p_nr)].XyzEnd[0],
	                       &_G(spieler_mi)[_G(auto_p_nr)].XyzEnd[1]) != -1)
	{
		_G(spieler_mi)[_G(auto_p_nr)].XyzStart[0] = _G(spieler_vector)[_G(auto_p_nr)].Xypos[0];
		_G(spieler_mi)[_G(auto_p_nr)].XyzStart[1] = _G(spieler_vector)[_G(auto_p_nr)].Xypos[1];
		_G(spieler_mi)[_G(auto_p_nr)].XyzEnd[0] -= x_offset;
		_G(spieler_mi)[_G(auto_p_nr)].XyzEnd[1] -= y_offset;
		_G(mov)->get_mov_vector((int16 *)_G(spieler_mi)[_G(auto_p_nr)].XyzStart, _G(spieler_mi)[_G(auto_p_nr)].Vorschub, &_G(spieler_vector)[_G(auto_p_nr)])
		;
		get_phase(&_G(spieler_vector)[_G(auto_p_nr)], &_G(spieler_mi)[_G(auto_p_nr)]);
	}
}

void hide_person() {
	for (int16 i = 0; i < MAX_PERSON; i++) {

		if (!_G(spieler).PersonHide[i]) {
			_G(spieler).PersonHide[i] = true;
			_G(person_tmp_hide)[i] = true;
		} else
			_G(person_tmp_hide)[i] = false;
	}
}

void show_person() {
	for (int16 i = 0; i < MAX_PERSON; i++) {

		if (_G(person_tmp_hide)[i])
			_G(spieler).PersonHide[i] = false;
	}
}

void save_person_rnr() {
	for (int16 i = 0; i < MAX_PERSON; i++)
		_G(person_tmp_room)[i] = _G(spieler).PersonRoomNr[i];
	_G(flags).SavePersonRnr = true;
}

void set_person_rnr() {
	if (_G(flags).SavePersonRnr) {
		for (int16 i = 0; i < MAX_PERSON; i++)
			_G(spieler).PersonRoomNr[i] = _G(person_tmp_room)[i];
		_G(flags).SavePersonRnr = false;
	}
}

bool is_chewy_busy() {
	bool ret = true;
	if (_G(atds)->ats_get_status() == DISPLAY_NONE) {
		if (_G(atds)->aad_get_status() == -1) {
			if (_G(atds)->ads_get_status() == -1) {
				if (!_G(mov)->auto_go_status()) {
					if (!_G(spieler_vector)[P_CHEWY].Count) {
						if (!_G(flags).ExitMov) {
							if (!_G(spz_ani)[P_CHEWY]) {
								ret = false;
							}
						}
					}
				}
			}
		}
	}

	return ret;
}


ChewyFont::ChewyFont(Common::String filename) {
	const uint32 headerFont = MKTAG('T', 'F', 'F', '\0');
	Common::File stream;

	stream.open(filename);

	uint32 header = stream.readUint32BE();

	if (header != headerFont)
		error("Invalid resource - %s", filename.c_str());

	stream.skip(4);	// total memory
	_count = stream.readUint16LE();
	_first = stream.readUint16LE();
	_last = stream.readUint16LE();
	_deltaX = _dataWidth = stream.readUint16LE();
	_dataHeight = stream.readUint16LE();

	_displayWidth = _dataWidth;
	_displayHeight = _dataHeight;

	_fontSurface.create(_dataWidth * _count, _dataHeight, ::Graphics::PixelFormat::createFormatCLUT8());

	int bitIndex = 7;

	byte curr = stream.readByte();

	for (uint n = 0; n < _count; n++) {
		for (uint y = 0; y < _dataHeight; y++) {
			byte *p = (byte *)_fontSurface.getBasePtr(n * _dataWidth, y);

			for (uint x = n * _dataWidth; x < n * _dataWidth + _dataWidth; x++) {
				*p++ = (curr & (1 << bitIndex)) ? 0 : 0xFF;

				bitIndex--;
				if (bitIndex < 0) {
					bitIndex = 7;
					curr = stream.readByte();
				}
			}
		}
	}
}

ChewyFont::~ChewyFont() {
	_fontSurface.free();
}

void ChewyFont::setDisplaySize(uint16 width, uint16 height) {
	_displayWidth = width;
	_displayHeight = height;
}

void ChewyFont::setDeltaX(uint16 deltaX) {
	_deltaX = deltaX;
}

::Graphics::Surface *ChewyFont::getLine(const Common::String &texts) {
	::Graphics::Surface *line = new ::Graphics::Surface();
	line->create(texts.size() * _dataWidth, _dataHeight, ::Graphics::PixelFormat::createFormatCLUT8());

	for (uint i = 0; i < texts.size(); i++) {
		uint x = (texts[i] - _first) * _dataWidth;
		line->copyRectToSurface(_fontSurface, i * _dataWidth, 0, Common::Rect(x, 0, x + _dataWidth, _dataHeight));
	}

	return line;
}

::Graphics::Surface *FontMgr::getLine(const Common::String &texts) {
	return _font->getLine(texts);
}

} // namespace Chewy
