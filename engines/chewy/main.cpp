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
#include "chewy/cursor.h"
#include "chewy/events.h"
#include "chewy/mcga_graphics.h"
#include "chewy/menus.h"
#include "chewy/dialogs/files.h"
#include "chewy/dialogs/inventory.h"
#include "chewy/dialogs/main_menu.h"

namespace Chewy {

#define SCROLL_LEFT 120
#define SCROLL_RIGHT SCREEN_WIDTH-SCROLL_LEFT
#define SCROLL_UP 80
#define SCROLL_DOWN SCREEN_HEIGHT-SCROLL_UP

static const int16 invent_display[4][2] = {
	{5, 0}, { 265, 0 }, { 265, 149 }, { 5, 149 }
};

void game_main() {
	_G(fontMgr) = new FontMgr();

	_G(font8) = new ChewyFont(FONT8x8);
	_G(font6) = new ChewyFont(FONT6x8);
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

	tidy();
}

void alloc_buffers() {
	_G(workpage) = (byte *)MALLOC(64004l);
	_G(pal) = (byte *)MALLOC(768l);
	_G(Ci).tempArea = (byte *)MALLOC(64004l);
	_G(det)->set_taf_ani_mem(_G(Ci).tempArea);
}

void free_buffers() {
	_G(det)->del_dptr();
	for (int16 i = 0; i < MAX_PERSON; i++)
		free(_G(PersonTaf)[i]);
	free(_G(spz_tinfo));
	free(_G(spblende));
	free(_G(menutaf));
	free(_G(chewy));

	free(_G(Ci).tempArea);
	free(_G(pal));
	free(_G(workpage));
}

void cursorChoice(int16 nr) {
	int16 delay = -1;

	if (nr != CUR_USER) {
		delay = (1 + _G(gameState).DelaySpeed) * 5;
		_G(cur)->clearCustomCursor();
	}

	_G(cur)->setInventoryCursor(-1);

	switch (nr) {
	case CUR_WALK:
		_G(cur)->setAnimation(0, 3, delay);
		break;
	case CUR_NO_WALK:
		_G(cur)->setAnimation(8, 8, delay);
		break;
	case CUR_USE:
		_G(cur)->setAnimation(4, 7, delay);
		break;
	case CUR_NO_USE:
		_G(cur)->setAnimation(4, 4, delay);
		break;
	case CUR_NOPE:
		_G(cur)->setAnimation(9, 12, delay);
		break;
	case CUR_LOOK:
		_G(cur)->setAnimation(13, 16, delay);
		break;
	case CUR_NO_LOOK:
		_G(cur)->setAnimation(16, 16, delay);
		break;
	case CUR_TALK:
		_G(cur)->setAnimation(17, 20, delay);
		break;
	case CUR_NO_TALK:
		_G(cur)->setAnimation(17, 17, delay);
		break;
	case CUR_INVENT:
		_G(cur)->setAnimation(21, 24, delay);
		break;
	case CUR_SAVE:
		_G(cur)->setAnimation(25, 25, delay);
		break;
	case CUR_EXIT_LEFT:
		_G(cur)->setAnimation(EXIT_LEFT_SPR, EXIT_LEFT_SPR, delay);
		break;
	case CUR_EXIT_RIGHT:
		_G(cur)->setAnimation(EXIT_RIGHT_SPR, EXIT_RIGHT_SPR, delay);
		break;
	case CUR_EXIT_TOP:
		_G(cur)->setAnimation(EXIT_ABOVE_SPR, EXIT_ABOVE_SPR, delay);
		break;
	case CUR_EXIT_BOTTOM:
		_G(cur)->setAnimation(EXIT_BOTTOM_SPR, EXIT_BOTTOM_SPR, delay);
		break;
	case CUR_DISK:
		_G(cur)->setAnimation(30, 30, delay);
		break;
	case CUR_HOWARD:
		_G(cur)->setAnimation(31, 31, delay);
		break;
	case CUR_NICHELLE:
		_G(cur)->setAnimation(37, 37, delay);
		break;
	case CUR_POINT:
		_G(cur)->setAnimation(9, 9, delay);
		break;
	case CUR_BLASTER:
		_G(cur)->setAnimation(40, 40, delay);
		break;
	case CUR_USER:
		break;
	default:
		break;
	}
}

void hideCur() {
	if (!_G(cur_hide_flag)) {
		_G(cur_hide_flag) = true;
		_G(flags).ShowAtsInvTxt = false;
		_G(cur)->hideCursor();
		_G(flags).CursorStatus = false;
	}
}

void showCur() {
	_G(flags).ShowAtsInvTxt = true;
	_G(cur)->showCursor();
	_G(flags).CursorStatus = true;
}

void menuEntry() {
	_G(det)->freezeAni();
	_G(uhr)->setAllStatus(TIMER_FREEZE);
}

void menuExit() {
	_G(det)->unfreeze_ani();
	_G(uhr)->setAllStatus(TIMER_UNFREEZE);
	_G(uhr)->resetTimer(0, 0);
	_G(FrameSpeed) = 0;
}

bool mainLoop(int16 mode) {
	bool retValue = false;

	mouseAction();
	if (_G(flags).MainInput) {
		switch (g_events->_kbInfo._scanCode) {
		case Common::KEYCODE_F1:
			_G(cur)->setInventoryCursor(-1);
			_G(menu_item) = CUR_WALK;
			_G(cur)->showCursor();
			cursorChoice(_G(menu_item));
			if (_G(menu_display) == MENU_DISPLAY)
				_G(menu_display) = MENU_HIDE;
			break;

		case Common::KEYCODE_F2:
			_G(cur)->setInventoryCursor(-1);
			_G(menu_item) = CUR_USE;
			_G(cur)->showCursor();
			cursorChoice(_G(menu_item));
			if (_G(menu_display) == MENU_DISPLAY)
				_G(menu_display) = MENU_HIDE;
			break;

		case Common::KEYCODE_F3:
			_G(cur)->setInventoryCursor(-1);
			_G(menu_item) = CUR_LOOK;
			_G(cur)->showCursor();
			cursorChoice(_G(menu_item));
			if (_G(menu_display) == MENU_DISPLAY)
				_G(menu_display) = MENU_HIDE;
			break;

		case Common::KEYCODE_F4:
			_G(cur)->setInventoryCursor(-1);
			_G(menu_item) = CUR_TALK;
			_G(cur)->showCursor();
			cursorChoice(_G(menu_item));
			if (_G(menu_display) == MENU_DISPLAY)
				_G(menu_display) = MENU_HIDE;
			break;

		case Common::KEYCODE_F5:
		case Common::KEYCODE_SPACE:
			_G(tmp_menu_item) = _G(menu_item);
			_G(maus_old_x) = g_events->_mousePos.x;
			_G(maus_old_y) = g_events->_mousePos.y;
			_G(menu_item) = CUR_USE;
			menuEntry();
			_G(cur)->showCursor();
			Dialogs::Inventory::menu();
			menuExit();
			_G(menu_display) = 0;
			_G(cur_display) = true;
			if (!_G(cur)->usingInventoryCursor()) {
				_G(menu_item) = _G(tmp_menu_item);
				cursorChoice(_G(menu_item));
				_G(cur)->setInventoryCursor(-1);
			} else {
				_G(menu_item) = CUR_USE;

				getDisplayCoord(&_G(gameState).DispZx, &_G(gameState).DispZy, _G(cur)->getInventoryCursor());
			}

			g_events->_kbInfo._keyCode = '\0';
			break;

		case Common::KEYCODE_F6:
			_G(flags).SaveMenu = true;

			_G(out)->setPointer((byte *)g_screen->getPixels());
			_G(fontMgr)->setFont(_G(font6));
			cursorChoice(CUR_SAVE);
			if (Dialogs::Files::execute(true) == 1) {
				retValue = true;
				_G(fx_blend) = BLEND4;
			}
			if (!(_G(cur)->usingInventoryCursor() && _G(menu_item) == CUR_USE))
				cursorChoice(_G(menu_item));
			_G(cur_display) = true;
			_G(flags).SaveMenu = false;
			_G(cur)->showCursor();
			_G(out)->setPointer(_G(workptr));
			break;

		case Common::KEYCODE_ESCAPE:
			if (_G(menu_display) == 0) {
				menuEntry();
				_G(cur)->hideCursor();
				_G(tmp_menu_item) = _G(menu_item);
				_G(maus_old_x) = g_events->_mousePos.x;
				_G(maus_old_y) = g_events->_mousePos.y;
				_G(menu_display) = MENU_DISPLAY;
				_G(cur_display) = false;
				_G(cur)->move((MOUSE_MENU_MAX_X / 5) * (_G(menu_item)), 100);
			} else {
				menuExit();
				_G(cur)->showCursor();
				_G(menu_item) = _G(tmp_menu_item);
				_G(menu_display) = MENU_HIDE;
				if (!(_G(cur)->usingInventoryCursor() && _G(menu_item) == CUR_USE))
					cursorChoice(_G(menu_item));
			}
			break;

		case Common::KEYCODE_RETURN:
			switch (_G(menu_item)) {
			case CUR_INVENT:

				_G(menu_item) = CUR_USE;
				menuEntry();
				_G(cur)->showCursor();
				Dialogs::Inventory::menu();
				menuExit();
				_G(menu_display) = 0;
				_G(cur_display) = true;
				if (!_G(cur)->usingInventoryCursor()) {
					_G(menu_item) = _G(tmp_menu_item);
					cursorChoice(_G(menu_item));
					_G(cur)->setInventoryCursor(-1);
				} else {
					_G(menu_item) = CUR_USE;
					getDisplayCoord(&_G(gameState).DispZx, &_G(gameState).DispZy, _G(cur)->getInventoryCursor());
				}
				break;

			case CUR_SAVE: {
				_G(flags).SaveMenu = true;
				_G(menu_display) = MENU_DISPLAY;
				_G(cur)->move(152, 92);
				g_events->_mousePos.x = 152;
				g_events->_mousePos.y = 92;
				_G(fontMgr)->setFont(_G(font6));

				_G(out)->setPointer((byte *)g_screen->getPixels());
				cursorChoice(CUR_SAVE);
				bool ret = Dialogs::Files::execute(true);
				if (ret) {
					retValue = true;
					_G(fx_blend) = BLEND4;
				}

				_G(out)->setPointer(_G(workptr));
				_G(menu_item) = _G(tmp_menu_item);
				_G(menu_display) = MENU_HIDE;

				if (!(_G(cur)->usingInventoryCursor() && _G(menu_item) == CUR_USE))
					cursorChoice(_G(tmp_menu_item));
				_G(cur_display) = true;

				_G(flags).SaveMenu = false;
				_G(cur)->showCursor();
				}
				break;

			default:
				if (_G(menu_display) != 0) {
					menuExit();
					_G(cur)->showCursor();
					_G(menu_display) = 0;
					_G(cur_display) = true;
					_G(cur)->move(_G(maus_old_x), _G(maus_old_y));
					g_events->_mousePos.x = _G(maus_old_x);
					g_events->_mousePos.y = _G(maus_old_y);
					_G(cur)->setInventoryCursor(-1);
					cursorChoice(_G(menu_item));
				}
				break;
			}
			break;

		case Common::KEYCODE_RIGHT:
		case Common::KEYCODE_LEFT:
		case Common::KEYCODE_UP:
		case Common::KEYCODE_DOWN:
			kb_cur_action(g_events->_kbInfo._scanCode, 0);
			break;

		case 41:
			_G(gameState).DispFlag ^= 1;
			break;

		case Common::KEYCODE_TAB:
			if (_G(menu_display) == 0 && _G(gameState).DispFlag) {
				if (_G(gameState).InvDisp < 3)
					++_G(gameState).InvDisp;
				else
					_G(gameState).InvDisp = 0;
			}
			break;

		default:
			break;
		}

		if (_G(menu_display) == MENU_HIDE) {
			menuExit();
			_G(menu_display) = 0;
			_G(cur_display) = true;
			_G(cur)->move(_G(maus_old_x), _G(maus_old_y));
			g_events->_mousePos.x = _G(maus_old_x);
			g_events->_mousePos.y = _G(maus_old_y);
		}
	}

	g_events->_kbInfo._scanCode	= Common::KEYCODE_INVALID;
	if (mode == DO_SETUP)
		setupScreen(DO_MAIN_LOOP);

	return retValue;
}

static void showWalkAreas() {
	int xs = (_G(gameState).scrollx / 8) * 8,
		ys = (_G(gameState).scrolly / 8) * 8;

	for (int y = 0, yp = ys; y < 200 / 8; ++y, yp += 8) {
		for (int x = 0, xp = xs; x < 320 / 8; ++x, xp += 8) {
			const int barrierId = _G(barriers)->getBarrierId(xp, yp);

			if (barrierId) {
				Common::Rect r(xp, yp, xp + 8, yp + 8);
				r.translate(-_G(gameState).scrollx, -_G(gameState).scrolly);
				r.clip(Common::Rect(0, 0, 320, 200));

				g_screen->frameRect(r, 0xff);
			}
		}
	}
}

void setupScreen(SetupScreenMode mode) {
	int16 i;
	int16 txt_nr;

	bool isMainLoop = mode == DO_MAIN_LOOP;
	if (isMainLoop)
		mode = DO_SETUP;

	_G(uhr)->calcTimer();

	if (_G(ani_timer)[0]._timeFlag) {
		_G(uhr)->resetTimer(0, 0);
		_G(gameState).DelaySpeed = _G(FrameSpeed) / _G(gameState).FramesPerSecond;
		_G(moveState)[P_CHEWY].Delay = _G(gameState).DelaySpeed + _G(spz_delay)[P_CHEWY];
		_G(FrameSpeed) = 0;
		_G(det)->set_global_delay(_G(gameState).DelaySpeed);
	}
	++_G(FrameSpeed);
	_G(out)->setPointer(_G(workptr));
	_G(out)->map_spr2screen(_G(ablage)[_G(room_blk).AkAblage], _G(gameState).scrollx, _G(gameState).scrolly);

	for (i = 0; i < MAX_PERSON; i++)
		zoom_mov_anpass(&_G(moveState)[i], &_G(spieler_mi)[i]);

	if (_G(SetUpScreenFunc) && _G(menu_display) == 0 && !_G(flags).InventMenu) {
		_G(SetUpScreenFunc)();
		_G(out)->setPointer(_G(workptr));
	}

	spriteEngine();
	if (_G(menu_display) == MENU_DISPLAY || _G(flags).InventMenu) {
		if (!_G(flags).InventMenu)
			plotMainMenu();
	} else {
		kb_mov(1);
		_G(det)->unfreeze_ani();
		check_mouse_ausgang(g_events->_mousePos.x + _G(gameState).scrollx, g_events->_mousePos.y + _G(gameState).scrolly);

		if (!_G(flags).SaveMenu)
			calc_ani_timer();

		if (_G(cur)->usingInventoryCursor() && _G(gameState).DispFlag) {
			buildMenu(invent_display[_G(gameState).InvDisp][0],
			           invent_display[_G(gameState).InvDisp][1], 3, 3, 60, 0);
			_G(out)->spriteSet(_G(inv_spr)[_G(cur)->getInventoryCursor()],
			                 invent_display[_G(gameState).InvDisp][0] + 1 + _G(gameState).DispZx,
			                 invent_display[_G(gameState).InvDisp][1] + 1 + _G(gameState).DispZy
			                 , _G(scr_width));
		}

		if (_G(flags).DialogCloseup)
			handleDialogCloseupMenu();
		if (_G(mouseLeftClick)) {
			if (_G(menu_item) == CUR_WALK) {
				if (_G(cur_ausgang_flag)) {
					calc_ausgang(g_events->_mousePos.x + _G(gameState).scrollx, g_events->_mousePos.y + _G(gameState).scrolly);
				} else {
					if (!_G(flags).ChewyDontGo) {
						_G(gpkt).Dx = g_events->_mousePos.x - _G(spieler_mi)[P_CHEWY].HotMovX +
						          _G(gameState).scrollx + _G(spieler_mi)[P_CHEWY].HotX;
						_G(gpkt).Dy = g_events->_mousePos.y - _G(spieler_mi)[P_CHEWY].HotMovY +
						          _G(gameState).scrolly + _G(spieler_mi)[P_CHEWY].HotY;
						_G(gpkt).Sx = _G(moveState)[P_CHEWY].Xypos[0] +
						          _G(spieler_mi)[P_CHEWY].HotX;
						_G(gpkt).Sy = _G(moveState)[P_CHEWY].Xypos[1] +
						          _G(spieler_mi)[P_CHEWY].HotY;
						_G(gpkt).AkMovEbene = 1;
						_G(mov)->goto_xy(&_G(gpkt));
						_G(spieler_mi)[P_CHEWY].XyzStart[0] = _G(moveState)[P_CHEWY].Xypos[0];
						_G(spieler_mi)[P_CHEWY].XyzStart[1] = _G(moveState)[P_CHEWY].Xypos[1];
						_G(spieler_mi)[P_CHEWY].XyzEnd[0] = _G(gpkt).Dx - _G(spieler_mi)[P_CHEWY].HotX;
						_G(spieler_mi)[P_CHEWY].XyzEnd[1] = _G(gpkt).Dy - _G(spieler_mi)[P_CHEWY].HotY;
						_G(mov)->get_mov_vector((int16 *)_G(spieler_mi)[P_CHEWY].XyzStart, (int16 *)_G(spieler_mi)[P_CHEWY].XyzEnd, _G(spieler_mi)[P_CHEWY].Vorschub, &_G(moveState)[P_CHEWY]);
						get_phase(&_G(moveState)[P_CHEWY], &_G(spieler_mi)[P_CHEWY]);
						_G(moveState)[P_CHEWY]._delayCount = 0;
						_G(auto_p_nr) = P_CHEWY;
					}
				}
			}
		}

		calc_auto_go();

		if (_G(fx_blend)) {
			const int16 paletteId = _G(barriers)->getBarrierId(
				_G(moveState)[P_CHEWY].Xypos[0] + _G(spieler_mi)[P_CHEWY].HotX,
				_G(moveState)[P_CHEWY].Xypos[1] + _G(spieler_mi)[P_CHEWY].HotY);
			setShadowPalette(paletteId, false);
		} else {
			for (i = 0; i < MAX_PERSON; i++) {
				mov_objekt(&_G(moveState)[i], &_G(spieler_mi)[i]);
				_G(spieler_mi)[i].XyzStart[0] = _G(moveState)[i].Xypos[0];
				_G(spieler_mi)[i].XyzStart[1] = _G(moveState)[i].Xypos[1];
			}
		}

		for (i = 0; i < _G(auto_obj) && !_G(flags).StopAutoObj; i++)
			mov_objekt(&_G(auto_mov_vector)[i], &_G(auto_mov_obj)[i]);

		int16 nr = _G(obj)->is_iib_mouse(g_events->_mousePos.x + _G(gameState).scrollx, g_events->_mousePos.y + _G(gameState).scrolly);
		if (nr != -1) {
			txt_nr = _G(obj)->iib_txt_nr(nr);
			mous_obj_action(nr, mode, INVENTORY_NORMAL, txt_nr);
		} else {
			int16 tmp = calcMouseText(g_events->_mousePos.x, g_events->_mousePos.y, mode);
			if (tmp == -1 || tmp == 255) {

				nr = _G(obj)->is_sib_mouse(g_events->_mousePos.x + _G(gameState).scrollx, g_events->_mousePos.y + _G(gameState).scrolly);
				if (nr != -1) {
					txt_nr = _G(obj)->sib_txt_nr(nr);
					mous_obj_action(nr, mode, INVENTORY_STATIC, txt_nr);
				} else
					calc_mouse_person(g_events->_mousePos.x, g_events->_mousePos.y);
			}
		}
		if (_G(cur_display) && mode == DO_SETUP) {
			_G(cur)->updateCursor();

			if (_G(cur)->usingInventoryCursor() && _G(flags).CursorStatus)
				_G(out)->spriteSet(
					_G(cur)->getCursorSprite(),
					g_events->_mousePos.x,
					g_events->_mousePos.y,
				    _G(scr_width),
					_G(cur)->getCursorWidth(),
					_G(cur)->getCursorHeight()
				);
			if (_G(pfeil_delay) == 0) {
				_G(pfeil_delay) = _G(gameState).DelaySpeed;
				if (_G(pfeil_ani) < 4)
					++_G(pfeil_ani);
				else
					_G(pfeil_ani) = 0;
			} else {
				--_G(pfeil_delay);
			}
		}
	}

	_G(atds)->print_aad(_G(gameState).scrollx, _G(gameState).scrolly);
	_G(atds)->print_ats(_G(moveState)[P_CHEWY].Xypos[0] + CH_HOT_X,
	                _G(moveState)[P_CHEWY].Xypos[1], _G(gameState).scrollx, _G(gameState).scrolly);
	_G(mouseLeftClick) = false;
	if (mode == DO_SETUP) {
		_G(out)->setPointer(nullptr);
		switch (_G(fx_blend)) {
		case BLEND1:
			_G(fx)->blende1(_G(workptr), _G(pal), 0, 0);
			break;

		case BLEND2:
			_G(fx)->blende1(_G(workptr), _G(pal), 1, 0);
			break;

		case BLEND3:
			_G(fx)->rnd_blende(_G(spblende), _G(workptr), (byte *)g_screen->getPixels(), _G(pal), 0);
			break;

		case BLEND4:
			_G(out)->setPointer(_G(workptr));
			_G(out)->cls();
			_G(out)->setPointer(nullptr);
			_G(fx)->blende1(_G(workptr), _G(pal), 0, 0);
			break;

		default:
			_G(out)->copyToScreen();
			break;
		}

		_G(fx_blend) = BLEND_NONE;
	}

	if (g_engine->_showWalkAreas)
		showWalkAreas();

	_G(cur_hide_flag) = false;
	int16 *ScrXy = (int16 *)_G(ablage)[_G(room_blk).AkAblage];
	if (!_G(menu_display))
		calc_scroll(_G(moveState)[P_CHEWY].Xypos[0] + _G(spieler_mi)[P_CHEWY].HotX,
		            _G(moveState)[P_CHEWY].Xypos[1] + _G(spieler_mi)[P_CHEWY].HotY,
		            ScrXy[0], ScrXy[1],
		            &_G(gameState).scrollx, &_G(gameState).scrolly);

	g_screen->update();

	g_engine->setCanLoadSave(isMainLoop);
	EVENTS_UPDATE;
	g_engine->setCanLoadSave(false);
}

void mous_obj_action(int16 nr, int16 mode, int16 txt_mode, int16 txt_nr) {
	const uint8 roomNum = _G(room)->_roomInfo->_roomNr;
	Common::StringArray desc;

	if (mode == DO_SETUP) {

		if (txt_nr != -1 && _G(flags).ShowAtsInvTxt) {

			switch (txt_mode) {
			case INVENTORY_NORMAL:
			case INVENTORY_STATIC:
				desc = _G(atds)->getTextArray(roomNum, txt_nr, ATS_DATA);
				break;
			default:
				break;
			}

			if (desc.size() > 0) {
				_G(fontMgr)->setFont(_G(font8));
				int16 x = g_events->_mousePos.x;
				int16 y = g_events->_mousePos.y;
				calcTxtXy(&x, &y, desc);
				for (int16 i = 0; i < (int16)desc.size(); i++)
					printShadowed(x, y + i * 10, 255, 300, 0, _G(scr_width), desc[i].c_str());
			}
		}
	}

	if (_G(mouseLeftClick)) {
		if (_G(menu_item) != CUR_USE)
			Dialogs::Inventory::look_screen(txt_mode, txt_nr);
		else {
			if (_G(cur)->usingInventoryCursor()) {
				evaluateObj(nr, txt_mode);
			} else {
				if (txt_mode == INVENTORY_NORMAL) {
					if (!_G(flags).ChAutoMov) {
						_G(mouseLeftClick) = false;
						autoMove(_G(gameState).room_m_obj[nr].AutoMov, P_CHEWY);
						Dialogs::Inventory::look_screen(txt_mode, txt_nr);

						if (_G(cur)->usingInventoryCursor())
							_G(gameState).room_m_obj[_G(cur)->getInventoryCursor()].RoomNr = -1;
						if (_G(gameState).room_m_obj[nr].AniFlag == 255) {
							invent_2_slot(nr);
						} else {
							_G(gameState)._personHide[P_CHEWY] = _G(gameState).room_m_obj[nr].HeldHide;
							play_scene_ani(_G(gameState).room_m_obj[nr].AniFlag, ANI_FRONT);
							invent_2_slot(nr);

							_G(gameState)._personHide[P_CHEWY] = false;
						}

						_G(cur)->setInventoryCursor(-1);
						_G(menu_item) = CUR_WALK;
						cursorChoice(_G(menu_item));
						_G(moveState)[P_CHEWY]._delayCount = 0;

						if (_G(cur)->usingInventoryCursor())
							_G(gameState).room_m_obj[_G(cur)->getInventoryCursor()].RoomNr = 255;

					}
				} else if (txt_mode == INVENTORY_STATIC) {
					evaluateObj(nr, STATIC_USE);
				}
			}
		}
	}
}

void kb_mov(int16 mode) {
	bool ende = false;
	while (!ende) {
		switch (_G(in)->getSwitchCode()) {
		case Common::KEYCODE_RIGHT:
			if (g_events->_mousePos.x < 320 - _G(cur)->getCursorWidth())
				_G(cur)->move(g_events->_mousePos.x + 2, g_events->_mousePos.y);
			break;

		case Common::KEYCODE_LEFT:
			if (g_events->_mousePos.x > 1)
				_G(cur)->move(g_events->_mousePos.x - 2, g_events->_mousePos.y);
			break;

		case Common::KEYCODE_UP:
			if (g_events->_mousePos.y > 1)
				_G(cur)->move(g_events->_mousePos.x, g_events->_mousePos.y - 2);
			break;

		case Common::KEYCODE_DOWN:
			if (g_events->_mousePos.y < 210 - _G(cur)->getCursorHeight())
				_G(cur)->move(g_events->_mousePos.x, g_events->_mousePos.y + 2);
			break;

		default:
			ende = true;
			break;

		}
		
		if (mode)
			ende = true;
		else
			setupScreen(DO_SETUP);
		SHOULD_QUIT_RETURN;
	}
}

void kb_cur_action(int16 key, int16 mode) {
	switch (key) {
	case Common::KEYCODE_RIGHT:
		if (_G(menu_display) == MENU_DISPLAY) {
			if (_G(menu_item) < 5)
				++_G(menu_item);
			else
				_G(menu_item) = CUR_WALK;
			_G(cur)->move((_G(menu_item)) * (MOUSE_MENU_MAX_X / 5), 100);
		}
		break;

	case Common::KEYCODE_LEFT:
		if (_G(menu_display) == MENU_DISPLAY) {
			if (_G(menu_item) > 0)
				--_G(menu_item);
			else
				_G(menu_item) = CUR_INVENT;
			_G(cur)->move((_G(menu_item)) * (MOUSE_MENU_MAX_X / 5), 100);
		}
		break;

	case Common::KEYCODE_UP:
		if (_G(menu_display) == MENU_DISPLAY) {
			if (_G(gameState).MainMenuY > 1)
				_G(gameState).MainMenuY -= 2;
		}
		break;

	case Common::KEYCODE_DOWN:
		if (_G(menu_display) == MENU_DISPLAY) {
			if (_G(gameState).MainMenuY < 163)
				_G(gameState).MainMenuY += 2;
		}
		break;

	default:
		break;
	}
}

void mouseAction() {
	int16 x = g_events->_mousePos.x;
	int16 y = g_events->_mousePos.y;
	if (x > invent_display[_G(gameState).InvDisp][0] &&
	        x < invent_display[_G(gameState).InvDisp][0] + 48 &&
	        y > invent_display[_G(gameState).InvDisp][1] &&
	        y < invent_display[_G(gameState).InvDisp][1] + 48) {
		if (!_G(cur)->usingInventoryCursor() && !_G(inv_disp_ok) && _G(cur)->usingInventoryCursor()) {
			cursorChoice(CUR_USE);
		}
		_G(inv_disp_ok) = true;
	} else {
		if (!_G(cur)->usingInventoryCursor() && _G(inv_disp_ok)) {
			cursorChoice(_G(menu_item));
		}
		_G(inv_disp_ok) = false;
	}
	if (_G(atds)->aadGetStatus() == -1) {
		if (_G(minfo).button || g_events->_kbInfo._keyCode == Common::KEYCODE_ESCAPE || g_events->_kbInfo._keyCode == Common::KEYCODE_RETURN) {

			if (_G(minfo).button == 2 || g_events->_kbInfo._keyCode == Common::KEYCODE_ESCAPE) {
				if (!_G(flags).mainMouseFlag) {
					g_events->_kbInfo._scanCode = Common::KEYCODE_ESCAPE;
				}
			} else if (_G(minfo).button == 1 || g_events->_kbInfo._keyCode == Common::KEYCODE_RETURN) {
				if (!_G(flags).mainMouseFlag) {
					if (_G(menu_display) == MENU_DISPLAY)
						g_events->_kbInfo._scanCode = Common::KEYCODE_RETURN;
					else if (_G(cur)->usingInventoryCursor()) {
						if (_G(inv_disp_ok)) {
							if (_G(cur)->usingInventoryCursor()) {
								_G(menu_item) = CUR_USE;
								cursorChoice(_G(menu_item));
								_G(cur)->setInventoryCursor(-1);
							} else {
								_G(menu_item) = CUR_USE;
								g_events->_kbInfo._scanCode = Common::KEYCODE_INVALID;
							}
						} else if (!_G(flags).MouseLeft)
							_G(mouseLeftClick) = true;
					} else if (!_G(flags).MouseLeft)
						_G(mouseLeftClick) = true;
				}
			}
			_G(flags).mainMouseFlag = 1;
		} else
			_G(flags).mainMouseFlag = 0;
	}
}

void evaluateObj(int16 objectId, int16 mode) {
	int16 animationId;
	int16 txt_nr;
	int16 direction;
	int16 ret = NO_ACTION;
	int16 action_flag = false;

	switch (mode) {
	case INVENTORY_NORMAL:
		ret = _G(obj)->action_iib_iib(_G(cur)->getInventoryCursor(), objectId);
		if (ret != NO_ACTION) {
			hideCur();
			if (_G(flags).InventMenu == false) {
				if (_G(gameState).room_m_obj[_G(cur)->getInventoryCursor()].AutoMov != 255) {
					_G(mouseLeftClick) = false;
					autoMove(_G(gameState).room_m_obj[objectId].AutoMov, P_CHEWY);
				}
				txt_nr = _G(obj)->iib_txt_nr(objectId);
				Dialogs::Inventory::look_screen(INVENTORY_NORMAL, txt_nr);
				if (_G(gameState).room_m_obj[objectId].AniFlag != 255) {
					_G(gameState)._personHide[P_CHEWY] = _G(gameState).room_m_obj[objectId].HeldHide;
					play_scene_ani(_G(gameState).room_m_obj[objectId].AniFlag, ANI_FRONT);
					_G(gameState)._personHide[P_CHEWY] = false;
				}
			}
			showCur();
		}
		break;

	case INVENTORY_STATIC:
		ret = _G(obj)->action_iib_sib(_G(cur)->getInventoryCursor(), objectId);
		if (ret != NO_ACTION) {
			_G(mouseLeftClick) = false;
			hideCur();
			if (_G(gameState).room_m_obj[_G(cur)->getInventoryCursor()].AutoMov != 255) {
				autoMove(_G(gameState).room_s_obj[objectId].AutoMov, P_CHEWY);
			}
			txt_nr = _G(obj)->sib_txt_nr(objectId);
			Dialogs::Inventory::look_screen(INVENTORY_STATIC, txt_nr);
			if (_G(gameState).room_s_obj[objectId].AniFlag != 255) {
				_G(gameState)._personHide[P_CHEWY] = _G(gameState).room_s_obj[objectId].HeldHide;
				direction = getAniDirection((int16)_G(gameState).room_s_obj[objectId].ZustandAk);
				animationId = _G(gameState).room_s_obj[objectId].AniFlag;

				if (animationId >= 150) {
					start_spz_wait(animationId - 150, 1, false, P_CHEWY);
					animationId = -1;
				} else if (animationId >= 100) {
					animationId -= 100;
					_G(obj)->calc_static_detail(objectId);
				}
				if (animationId != -1)
					play_scene_ani(animationId, direction);
				_G(gameState)._personHide[P_CHEWY] = false;
			}
			_G(menu_item_vorwahl) = CUR_WALK;
			showCur();
			sib_event_inv(objectId);

			if (!_G(cur)->usingInventoryCursor()) {
				_G(menu_item) = _G(menu_item_vorwahl);
				cursorChoice(_G(menu_item));
			}
			_G(obj)->calc_all_static_detail();
		}
		break;

	case STATIC_USE:
		ret = _G(obj)->calc_static_use(objectId);
		if (ret == OBJECT_1) {
			_G(mouseLeftClick) = false;
			hideCur();
			if (_G(gameState).room_s_obj[objectId].AutoMov != 255) {

				autoMove(_G(gameState).room_s_obj[objectId].AutoMov, P_CHEWY);
			}
			txt_nr = _G(obj)->sib_txt_nr(objectId);
			Dialogs::Inventory::look_screen(INVENTORY_STATIC, txt_nr);
			if (_G(gameState).room_s_obj[objectId].AniFlag != 255) {
				_G(gameState)._personHide[P_CHEWY] = _G(gameState).room_s_obj[objectId].HeldHide;
				direction = getAniDirection((int16)_G(gameState).room_s_obj[objectId].ZustandAk);

				animationId = _G(gameState).room_s_obj[objectId].AniFlag;

				if (animationId >= 150) {
					start_spz_wait(animationId - 150, 1, false, P_CHEWY);
					animationId = -1;
				} else if (animationId >= 100) {
					animationId -= 100;
					_G(obj)->calc_static_detail(objectId);
				}
				if (animationId != -1)
					play_scene_ani(animationId, direction);
				_G(gameState)._personHide[P_CHEWY] = false;
			}

			if (_G(gameState).room_s_obj[objectId].InvNr != -1) {
				invent_2_slot(_G(gameState).room_s_obj[objectId].InvNr);
				action_flag = true;
			}
			_G(menu_item_vorwahl) = CUR_WALK;
			showCur();
			int16 sib_ret = sib_event_no_inv(objectId);

			_G(obj)->calc_all_static_detail();

			if (!_G(cur)->usingInventoryCursor()) {

				if (sib_ret || action_flag) {
					_G(menu_item) = _G(menu_item_vorwahl);
					cursorChoice(_G(menu_item));
				}
			}
		} else if (ret == SIB_GET_INV) {
			_G(mouseLeftClick) = false;
			hideCur();
			if (_G(gameState).room_s_obj[objectId].AutoMov != 255) {
				autoMove(_G(gameState).room_s_obj[objectId].AutoMov, P_CHEWY);
			}
			txt_nr = _G(obj)->sib_txt_nr(objectId);
			Dialogs::Inventory::look_screen(INVENTORY_STATIC, txt_nr);
			if (_G(gameState).room_s_obj[objectId].AniFlag != 255) {
				_G(gameState)._personHide[P_CHEWY] = _G(gameState).room_s_obj[objectId].HeldHide;
				direction = getAniDirection((int16)_G(gameState).room_s_obj[objectId].ZustandAk);

				animationId = _G(gameState).room_s_obj[objectId].AniFlag;

				if (animationId >= 150) {
					start_spz_wait(animationId - 150, 1, false, P_CHEWY);
					animationId = -1;
				} else if (animationId >= 100) {
					animationId -= 100;
					_G(obj)->calc_static_detail(objectId);
				}
				if (animationId != -1) {
					play_scene_ani(animationId, direction);
				}
				_G(gameState)._personHide[P_CHEWY] = false;
			}

			if (_G(gameState).room_s_obj[objectId].InvNr != -1)
				invent_2_slot(_G(gameState).room_s_obj[objectId].InvNr);
			_G(obj)->calc_rsi_flip_flop(objectId);
			_G(menu_item_vorwahl) = CUR_WALK;
			showCur();
			sib_event_no_inv(objectId);
			_G(obj)->calc_all_static_detail();
			if (!_G(cur)->usingInventoryCursor()) {
				_G(menu_item) = _G(menu_item_vorwahl);
				cursorChoice(_G(menu_item));
			}
		} else if (ret == NO_ACTION) {
			txt_nr = _G(obj)->sib_txt_nr(objectId);
			Dialogs::Inventory::look_screen(INVENTORY_STATIC, txt_nr);
		}
		break;

	default:
		break;
	}

	switch (ret) {
	case OBJECT_1:
	case OBJECT_2:
		if (mode == INVENTORY_NORMAL)
			calc_inv_use_txt(objectId);
		break;

	case NO_ACTION:
		if (mode == STATIC_USE && _G(flags).StaticUseTxt == true)
			_G(flags).StaticUseTxt = false;
		else if (mode != STATIC_USE)
			calc_inv_no_use(objectId, mode);

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

void setShadowPalette(int16 palIdx, bool setPartialPalette) {
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

	const uint8 *palette;
	if (palIdx == 1)
		palette = PAL_1;
	else if (palIdx == 2)
		palette = PAL_2;
	else if (palIdx == 3)
		palette = PAL_3;
	else if (palIdx == 4)
		palette = PAL_0;
	else
		return;

	if (setPartialPalette)
		_G(out)->set_partialpalette(palette, 1, 11);
	memcpy(_G(pal), palette, 12 * 3);
}

void printShadowed(int16 x, int16 y, int16 fgCol, int16 bgCol, int16 shadowFgCol, int16 scrWidth, const char *txtPtr) {
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
			int16 tmp = _G(mouseLeftClick);
			_G(mouseLeftClick) = false;
			_G(gpkt).Dx = _G(Rdi)->AutoMov[movNr]._x -
						  _G(spieler_mi)[playerNum].HotMovX + _G(spieler_mi)[playerNum].HotX;
			_G(gpkt).Dy = _G(Rdi)->AutoMov[movNr]._y -
						  _G(spieler_mi)[playerNum].HotMovY + _G(spieler_mi)[playerNum].HotY;
			_G(gpkt).Sx = _G(moveState)[playerNum].Xypos[0] + _G(spieler_mi)[playerNum].HotX;
			_G(gpkt).Sy = _G(moveState)[playerNum].Xypos[1] + _G(spieler_mi)[playerNum].HotY;
			_G(gpkt).AkMovEbene = 1;
			_G(mov)->goto_xy(&_G(gpkt));

			_G(spieler_mi)[playerNum].XyzStart[0] = _G(moveState)[playerNum].Xypos[0];
			_G(spieler_mi)[playerNum].XyzStart[1] = _G(moveState)[playerNum].Xypos[1];
			_G(spieler_mi)[playerNum].XyzEnd[0] = _G(gpkt).Dx - _G(spieler_mi)[playerNum].HotX;
			_G(spieler_mi)[playerNum].XyzEnd[1] = _G(gpkt).Dy - _G(spieler_mi)[playerNum].HotY;
			_G(mov)->get_mov_vector((int16 *)_G(spieler_mi)[playerNum].XyzStart, (int16 *)_G(spieler_mi)[playerNum].XyzEnd,
				_G(spieler_mi)[playerNum].Vorschub, &_G(moveState)[playerNum]);
			get_phase(&_G(moveState)[playerNum], &_G(spieler_mi)[playerNum]);
			_G(moveState)[playerNum]._delayCount = 0;

			if (_G(mov)->auto_go_status()) {
				while (_G(mov)->auto_go_status()) {
					if (SHOULD_QUIT)
						return 0;
					if (_G(in)->getSwitchCode() == Common::KEYCODE_ESCAPE) {
						if (_G(flags).ExitMov || _G(flags).BreakAMov) {
							key = Common::KEYCODE_ESCAPE;
							_G(mov)->stop_auto_go();
							movingFl = false;
						}
					}
					setupScreen(DO_SETUP);
				}
			}
			if (_G(flags).ChAutoMov) {
				bool endLoopFl = false;
				_G(spieler_mi)[playerNum].XyzStart[0] = _G(moveState)[playerNum].Xypos[0];
				_G(spieler_mi)[playerNum].XyzStart[1] = _G(moveState)[playerNum].Xypos[1];
				_G(spieler_mi)[playerNum].XyzEnd[0] = _G(gpkt).Dx - _G(spieler_mi)[playerNum].HotX;
				_G(spieler_mi)[playerNum].XyzEnd[1] = _G(gpkt).Dy - _G(spieler_mi)[playerNum].HotY;
				_G(mov)->get_mov_vector((int16 *)_G(spieler_mi)[playerNum].XyzStart, (int16 *)_G(spieler_mi)[playerNum].XyzEnd, _G(spieler_mi)[playerNum].Vorschub, &_G(moveState)[playerNum]);
				get_phase(&_G(moveState)[playerNum], &_G(spieler_mi)[playerNum]);
				while (!endLoopFl) {
					if (_G(in)->getSwitchCode() == Common::KEYCODE_ESCAPE || key == Common::KEYCODE_ESCAPE) {
						if (_G(flags).ExitMov || _G(flags).BreakAMov) {
							_G(moveState)[playerNum].Count = 0;
							movingFl = false;
						}
					}
					if (!_G(moveState)[playerNum].Count) {
						if (movingFl) {
							if (_G(flags).ExitMov == false && _G(flags).ChAutoMov) {

								setPersonPos(_G(spieler_mi)[playerNum].XyzEnd[0],
								               _G(spieler_mi)[playerNum].XyzEnd[1],
								               playerNum, _G(Rdi)->AutoMov[movNr]._sprNr);
							}
						}
						endLoopFl = true;
					}
					setupScreen(DO_SETUP);
					SHOULD_QUIT_RETURN0;
				}
			}
			_G(auto_p_nr) = P_CHEWY;
			_G(mouseLeftClick) = tmp;
			_G(flags).ChAutoMov = false;
		}
	}

	return movingFl;
}

void goAutoXy(int16 x, int16 y, int16 personNum, int16 mode) {
	if (!_G(stopAutoMove)[personNum] && _G(gameState)._personRoomNr[personNum] == _G(gameState)._personRoomNr[P_CHEWY]) {
		int16 move_status = true;
		int16 tmp = _G(mouseLeftClick);
		_G(mouseLeftClick) = false;

		_G(spieler_mi)[personNum].XyzStart[0] = _G(moveState)[personNum].Xypos[0];
		_G(spieler_mi)[personNum].XyzStart[1] = _G(moveState)[personNum].Xypos[1];
		_G(spieler_mi)[personNum].XyzEnd[0] = x;
		_G(spieler_mi)[personNum].XyzEnd[1] = y;
		_G(mov)->get_mov_vector((int16 *)_G(spieler_mi)[personNum].XyzStart, (int16 *)_G(spieler_mi)[personNum].XyzEnd,
			_G(spieler_mi)[personNum].Vorschub, &_G(moveState)[personNum]);

		if (_G(moveState)[personNum].Count)
			get_phase(&_G(moveState)[personNum], &_G(spieler_mi)[personNum]);
		if (mode == ANI_WAIT) {
			bool endLoopFl = false;
			while (!endLoopFl) {
				if (_G(in)->getSwitchCode() == Common::KEYCODE_ESCAPE) {
					if (_G(flags).ExitMov || _G(flags).BreakAMov) {
						_G(moveState)[personNum].Count = 0;
						move_status = false;
					}
				}
				if (!_G(moveState)[personNum].Count) {
					if (move_status) {
						setPersonPos(_G(spieler_mi)[personNum].XyzEnd[0],
						               _G(spieler_mi)[personNum].XyzEnd[1], personNum, -1);
					}
					endLoopFl = true;
				}
				setupScreen(DO_SETUP);
				SHOULD_QUIT_RETURN;
			}
		}

		_G(mouseLeftClick) = tmp;
	}
}

int16 getAniDirection(int16 status) {
	int16 ret = ANI_FRONT;
	switch (status) {
	case OBJZU_ZU:
	case OBJZU_LOCKED:
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
				idx = _G(det)->maus_vector(x + _G(gameState).scrollx, y + _G(gameState).scrolly);
				if (idx != -1) {
					txtNr = _G(Rdi)->mtxt[idx];
					inv_no_use_mode = DETEDIT_REC;
				}
			}

			if (txtNr != -1) {
				ret = -1;
				bool dispFl = true;
				bool actionFl = false;

				if (_G(mouseLeftClick) && !_G(flags).MausTxt) {
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
						if (_G(cur)->usingInventoryCursor())
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
					if (!_G(atds)->getControlBit(txtNr, ATS_ACTIVE_BIT)) {
						if (_G(menu_item) != CUR_WALK && _G(menu_item) != CUR_USE) {
							if (x + _G(gameState).scrollx > _G(moveState)[P_CHEWY].Xypos[0])
								setPersonSpr(P_RIGHT, P_CHEWY);
							else
								setPersonSpr(P_LEFT, P_CHEWY);
						}
					}

					if (_G(atds)->getControlBit(txtNr, ATS_ACTION_BIT)) {
						action_ret = atsAction(txtNr, txtMode, ATS_ACTION_VOR);
					}
					
					if (ok && !_G(atds)->getControlBit(txtNr, ATS_ACTIVE_BIT)) {
						if (startAtsWait(txtNr, txtMode, 14, ATS_DATA))
							dispFl = false;
					} else {
						ret = -1;
					}
					
					if (_G(atds)->getControlBit(txtNr, ATS_ACTION_BIT)) {
						action_ret = atsAction(txtNr, txtMode, ATS_ACTION_NACH);
						actionFl = true;
						if (action_ret)
							ret = 1;
					}
					
					if (!ok && !action_ret) {
						if (inv_no_use_mode != -1 && !_G(atds)->getControlBit(txtNr, ATS_ACTIVE_BIT)) {
							actionFl = calc_inv_no_use(idx + (_G(gameState)._personRoomNr[P_CHEWY] * 100), inv_no_use_mode);
							if (actionFl)
								ret = txtNr;
						}
					}
					
					if (ok && !action_ret && txtMode == TXT_MARK_USE && dispFl) {
						if (!_G(atds)->getControlBit(txtNr, ATS_ACTIVE_BIT)) {
							if (_G(menu_item) != CUR_WALK) {
								if (x + _G(gameState).scrollx > _G(moveState)[P_CHEWY].Xypos[0])
									setPersonSpr(P_RIGHT, P_CHEWY);
								else
									setPersonSpr(P_LEFT, P_CHEWY);
							}
							int16 r_val = g_engine->_rnd.getRandomNumber(MAX_RAND_NO_USE - 1);
							actionFl = startAtsWait(RAND_NO_USE[r_val], TXT_MARK_USE, 14, INV_USE_DEF);
							if (actionFl)
								ret = txtNr;
						}
					}

					_G(flags).MausTxt = false;
				} else {
					ret = -1;
				}

				if (dispFl && !actionFl) {
					const uint8 roomNum = _G(room)->_roomInfo->_roomNr;
					Common::StringArray desc = _G(atds)->getTextArray(roomNum, txtNr, ATS_DATA);

					if (desc.size() > 0) {
						ret = txtNr;
						_G(fontMgr)->setFont(_G(font8));
						calcTxtXy(&x, &y, desc);
						for (int16 i = 0; i < (int16)desc.size(); i++)
							printShadowed(x, y + i * 10, 255, 300, 0, _G(scr_width), desc[i].c_str());
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
						xy = (int16 *)_G(chewy)->image[_G(chewy_ph)[_G(moveState)[P_CHEWY].Phase * 8 + _G(moveState)[P_CHEWY].PhNr]];
						break;

					case P_HOWARD:
					case P_NICHELLE:
						if (_G(gameState)._personRoomNr[i] != _G(gameState)._personRoomNr[P_CHEWY])
							check = false;
						xy = (int16 *)_G(PersonTaf)[i]->image[_G(PersonSpr)[i][_G(moveState)[i].PhNr]];
						break;

					default:
						break;
					}
				} else
					xy = (int16 *)_G(spz_tinfo)->image[_G(spz_spr_nr)[_G(moveState)[i].PhNr]];
				if (check) {
					if (x + _G(gameState).scrollx >= _G(moveState)[i].Xypos[0] &&
					        x + _G(gameState).scrollx <= _G(moveState)[i].Xypos[0] + xy[0] + _G(moveState)[i].Xzoom &&
					        y + _G(gameState).scrolly >= _G(moveState)[i].Xypos[1] &&
					        y + _G(gameState).scrolly <= _G(moveState)[i].Xypos[1] + xy[1] + _G(moveState)[i].Yzoom) {
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
	int16 diaNr = -1;
	if (_G(flags).ShowAtsInvTxt && !_G(flags).InventMenu) {
		int16 p_nr = is_mouse_person(x, y);
		if (p_nr != -1) {
			if (!_G(gameState)._personHide[p_nr]) {
				_G(fontMgr)->setFont(_G(font8));
				char *str_ = ch_txt[p_nr];
				calcTxtXy(&x, &y, str_, 1);
				printShadowed(x, y, 255, 300, 0, _G(scr_width), str_);
				if (_G(mouseLeftClick)) {
					int16 def_nr = -1;
					if (!_G(cur)->usingInventoryCursor()) {
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
								diaNr = true;
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
						if (diaNr == -1) {
							if (txt_nr != 30000) {
								if (_G(menu_item) != CUR_WALK) {
									if (x + _G(gameState).scrollx > _G(moveState)[P_CHEWY].Xypos[0])
										setPersonSpr(P_RIGHT, P_CHEWY);
									else
										setPersonSpr(P_LEFT, P_CHEWY);
								}
								if (!startAtsWait(txt_nr, mode, 14, ATS_DATA))
									startAtsWait(def_nr, TXT_MARK_USE, 14, INV_USE_DEF);
							}
						}
					} else {
						if (_G(menu_item) != CUR_WALK) {
							if (x + _G(gameState).scrollx > _G(moveState)[P_CHEWY].Xypos[0])
								setPersonSpr(P_RIGHT, P_CHEWY);
							else
								setPersonSpr(P_LEFT, P_CHEWY);
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
	for (int16 i = _G(room)->_roomTimer._timerStart;
	     i < _G(room)->_roomTimer._timerStart + _G(room)->_roomTimer._timerMaxNr; i++) {
		if (_G(ani_timer)[i]._timeFlag)
			timer_action(i);
	}
}

void get_user_key(int16 mode) {
	_G(flags).StopAutoObj = true;
	mouseAction();
	_G(mouseLeftClick) = false;

	if (!_G(inv_disp_ok)) {
		switch (_G(in)->getSwitchCode()) {
		case Common::KEYCODE_F5:
		case Common::KEYCODE_SPACE:
		case Common::KEYCODE_ESCAPE:
			_G(maus_old_x) = g_events->_mousePos.x;
			_G(maus_old_y) = g_events->_mousePos.y;

			_G(tmp_menu_item) = _G(menu_item);
			_G(menu_item) = CUR_USE;
			Dialogs::Inventory::menu();
			_G(menu_display) = 0;
			_G(cur_display) = true;
			if (!_G(cur)->usingInventoryCursor()) {
				_G(menu_item) = _G(tmp_menu_item);
				cursorChoice(_G(menu_item));
				_G(cur)->setInventoryCursor(-1);
			} else {
				_G(menu_item) = CUR_USE;

				getDisplayCoord(&_G(gameState).DispZx, &_G(gameState).DispZy, _G(cur)->getInventoryCursor());
			}

			g_events->_kbInfo._keyCode = '\0';
			break;

		default:
			break;
		}
	}

	_G(flags).StopAutoObj = false;
}

void clear_prog_ani() {
	for (int16 i = 0; i < MAX_PROG_ANI; i++) {
		_G(spr_info)[i]._image = nullptr;
		_G(spr_info)[i]._zLevel = 255;
	}
}

void set_ani_screen() {
	if (_G(flags).AniUserAction)
		get_user_key(NO_SETUP);
	setupScreen(DO_SETUP);
}

void delInventory(int16 nr) {
	_G(obj)->delInventory(nr, &_G(room_blk));
	_G(cur)->setInventoryCursor(-1);
	_G(menu_item) = CUR_WALK;
	_G(cur)->setInventoryCursor(-1);
	cursorChoice(_G(menu_item));
	del_invent_slot(nr);
	_G(mouseLeftClick) = false;
}

bool isCurInventory(int16 nr) {
	return _G(cur)->getInventoryCursor() == nr;
}

void check_mouse_ausgang(int16 x, int16 y) {
	if (_G(menu_item) == CUR_WALK) {
		bool found = true;
		int16 nr = _G(obj)->is_exit(x, y);
		int16 attr = (nr >= 0 && nr < MAX_EXIT) ? _G(gameState).room_e_obj[nr].Attribut : 0;

		switch (attr) {
		case EXIT_LEFT:
			_G(cur_ausgang_flag) = EXIT_LEFT;
			cursorChoice(CUR_EXIT_LEFT);
			break;

		case EXIT_RIGHT:
			_G(cur_ausgang_flag) = EXIT_RIGHT;
			cursorChoice(CUR_EXIT_RIGHT);
			break;

		case EXIT_TOP:
			_G(cur_ausgang_flag) = EXIT_TOP;
			cursorChoice(CUR_EXIT_TOP);
			break;

		case EXIT_BOTTOM:
			_G(cur_ausgang_flag) = EXIT_BOTTOM;
			cursorChoice(CUR_EXIT_BOTTOM);
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
		_G(mouseLeftClick) = false;
		int16 nr = _G(obj)->is_exit(x, y);
		if (nr != -1) {
			_G(flags).ExitMov = true;
			if (autoMove(_G(gameState).room_e_obj[nr].AutoMov, P_CHEWY) == true) {
				_G(flags).ShowAtsInvTxt = false;
				_G(menu_item) = CUR_DISK;
				cursorChoice(CUR_DISK);
				setupScreen(DO_SETUP);
				_G(cur_hide_flag) = true;
				exit_room(nr);
				_G(gameState)._personRoomNr[P_CHEWY] = _G(gameState).room_e_obj[nr].Exit;
				_G(room)->loadRoom(&_G(room_blk), _G(gameState)._personRoomNr[P_CHEWY], &_G(gameState));
				setPersonPos(_G(Rdi)->AutoMov[_G(gameState).room_e_obj[nr].ExitMov]._x -
				               _G(spieler_mi)[_G(auto_p_nr)].HotMovX,
				               _G(Rdi)->AutoMov[_G(gameState).room_e_obj[nr].ExitMov]._y - _G(spieler_mi)[_G(auto_p_nr)].HotMovY
				               , P_CHEWY, -1);
				int16 *ScrXy = (int16 *)_G(ablage)[_G(room_blk).AkAblage];
				get_scroll_off(_G(moveState)[P_CHEWY].Xypos[0] + _G(spieler_mi)[P_CHEWY].HotX,
				               _G(moveState)[P_CHEWY].Xypos[1] + _G(spieler_mi)[P_CHEWY].HotY,
				               ScrXy[0], ScrXy[1],
				               &_G(gameState).scrollx, &_G(gameState).scrolly);

				const int16 paletteId = _G(barriers)->getBarrierId(_G(moveState)[P_CHEWY].Xypos[0] + _G(spieler_mi)[P_CHEWY].HotX,
				                                              _G(moveState)[P_CHEWY].Xypos[1] + _G(spieler_mi)[P_CHEWY].HotY);
				setShadowPalette(paletteId, false);
				setPersonSpr(_G(Rdi)->AutoMov[_G(gameState).room_e_obj[nr].ExitMov]._sprNr, P_CHEWY);
				_G(moveState)[P_CHEWY]._delayCount = 0;
				_G(fx_blend) = BLEND1;
				_G(auto_obj) = 0;
				_G(moveState)[P_CHEWY].Xzoom = 0;
				_G(moveState)[P_CHEWY].Yzoom = 0;
				_G(flags).ShowAtsInvTxt = true;
				_G(menu_item) = CUR_WALK;
				cursorChoice(_G(menu_item));
				_G(flags).ExitMov = false;
				enter_room(nr);
			}
			_G(flags).ExitMov = false;
			setupScreen(DO_SETUP);
		}
	}
}

void get_scroll_off(int16 x, int16 y, int16 pic_x, int16 pic_y, int16 *sc_x, int16 *sc_y) {
	*sc_x = 0;
	*sc_y = 0;

	if (x >= SCREEN_WIDTH) {
		*sc_x = MIN(x - (SCREEN_WIDTH / 2),
			pic_x - SCREEN_WIDTH);
	}

	if (y >= SCREEN_HEIGHT) {
		*sc_y = MIN(y - (SCREEN_HEIGHT / 2),
			pic_y - SCREEN_HEIGHT);
	}
}

void calc_scroll(int16 x, int16 y, int16 pic_x, int16 pic_y, int16 *sc_x, int16 *sc_y) {
	if (!_G(flags).NoScroll) {
		if (!_G(scroll_delay)) {

			if ((_G(gameState).ScrollxStep * _G(gameState).DelaySpeed) > CH_X_PIX)
				_G(scroll_delay) = CH_X_PIX / _G(gameState).ScrollxStep;

			if (x - *sc_x < SCROLL_LEFT) {
				if ((*sc_x - _G(gameState).ScrollxStep) > 0) {
					*sc_x -= _G(gameState).ScrollxStep;
				}
			} else if (x - *sc_x > SCROLL_RIGHT) {
				if ((*sc_x + _G(gameState).ScrollxStep) < pic_x - SCREEN_WIDTH) {
					*sc_x += _G(gameState).ScrollxStep;
				}
			}

			if (y - *sc_y < SCROLL_UP) {
				if ((*sc_y - _G(gameState).ScrollyStep) > 0) {
					*sc_y -= _G(gameState).ScrollyStep;
				}
			} else if (y - *sc_y > SCROLL_DOWN) {
				if ((*sc_y + _G(gameState).ScrollyStep) < pic_y - SCREEN_HEIGHT) {
					*sc_y += _G(gameState).ScrollyStep;
				}
			}
		} else
			--_G(scroll_delay);
	}
}

void auto_scroll(int16 scrx, int16 scry) {
	int16 tmpMouseClick = _G(mouseLeftClick);
	_G(mouseLeftClick) = false;
	_G(gameState).scrollx >>= 1;
	_G(gameState).scrollx <<= 1;
	_G(gameState).scrolly >>= 1;
	_G(gameState).scrolly <<= 1;
	bool endLoopFl = false;
	while (!endLoopFl) {
		if (scrx < _G(gameState).scrollx)
			_G(gameState).scrollx -= _G(gameState).ScrollxStep;
		else if (scrx > _G(gameState).scrollx)
			_G(gameState).scrollx += _G(gameState).ScrollxStep;
		if (scry < _G(gameState).scrolly)
			_G(gameState).scrolly -= _G(gameState).ScrollyStep;
		else if (scry > _G(gameState).scrolly)
			_G(gameState).scrolly += _G(gameState).ScrollyStep;
		if (scrx == _G(gameState).scrollx && scry == _G(gameState).scrolly)
			endLoopFl = true;
		setupScreen(DO_SETUP);
		SHOULD_QUIT_RETURN;
	}
	_G(mouseLeftClick) = tmpMouseClick;
}

void disable_timer() {
	_G(uhr)->disableTimer();
	_G(FrameSpeedTmp) = _G(FrameSpeed);
}

void enable_timer() {
	_G(uhr)->enableTimer();
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
		_G(spieler_mi)[_G(auto_p_nr)].XyzStart[0] = _G(moveState)[_G(auto_p_nr)].Xypos[0];
		_G(spieler_mi)[_G(auto_p_nr)].XyzStart[1] = _G(moveState)[_G(auto_p_nr)].Xypos[1];
		_G(spieler_mi)[_G(auto_p_nr)].XyzEnd[0] -= x_offset;
		_G(spieler_mi)[_G(auto_p_nr)].XyzEnd[1] -= y_offset;
		_G(mov)->get_mov_vector((int16 *)_G(spieler_mi)[_G(auto_p_nr)].XyzStart, (int16 *)_G(spieler_mi)[_G(auto_p_nr)].XyzEnd, _G(spieler_mi)[_G(auto_p_nr)].Vorschub, &_G(moveState)[_G(auto_p_nr)])
		;
		get_phase(&_G(moveState)[_G(auto_p_nr)], &_G(spieler_mi)[_G(auto_p_nr)]);
	}
}

void hide_person() {
	for (int16 i = 0; i < MAX_PERSON; i++) {
		if (!_G(gameState)._personHide[i]) {
			_G(gameState)._personHide[i] = true;
			_G(person_tmp_hide)[i] = true;
		} else
			_G(person_tmp_hide)[i] = false;
	}
}

void show_person() {
	for (int16 i = 0; i < MAX_PERSON; i++) {

		if (_G(person_tmp_hide)[i])
			_G(gameState)._personHide[i] = false;
	}
}

void save_person_rnr() {
	for (int16 i = 0; i < MAX_PERSON; i++)
		_G(person_tmp_room)[i] = _G(gameState)._personRoomNr[i];
	_G(flags).SavePersonRnr = true;
}

void set_person_rnr() {
	if (_G(flags).SavePersonRnr) {
		for (int16 i = 0; i < MAX_PERSON; i++)
			_G(gameState)._personRoomNr[i] = _G(person_tmp_room)[i];
		_G(flags).SavePersonRnr = false;
	}
}

bool is_chewy_busy() {
	bool ret = true;
	if (!_G(atds)->atsShown()) {
		if (_G(atds)->aadGetStatus() == -1) {
			if (_G(atds)->getDialogCloseupStatus() == -1) {
				if (!_G(mov)->auto_go_status()) {
					if (!_G(moveState)[P_CHEWY].Count) {
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

	_fontSurface.create(_dataWidth * _count, _dataHeight, Graphics::PixelFormat::createFormatCLUT8());

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

Graphics::Surface *ChewyFont::getLine(const Common::String &texts) {
	Graphics::Surface *line = new Graphics::Surface();
	if (texts.size() == 0)
		return line;

	Common::Rect subrect(0, 0, _dataWidth, _dataHeight);
	line->create(texts.size() * _deltaX, _dataHeight, Graphics::PixelFormat::createFormatCLUT8());
	line->fillRect(Common::Rect(line->w, line->h), 0xFF);

	for (uint i = 0; i < texts.size(); i++) {
		subrect.moveTo(((byte)texts[i] - _first) * _dataWidth, 0);
		line->copyRectToSurface(_fontSurface, i * (_deltaX - 2), 0, subrect);
	}

	return line;
}

Graphics::Surface *FontMgr::getLine(const Common::String &texts) {
	return _font->getLine(texts);
}

} // namespace Chewy
