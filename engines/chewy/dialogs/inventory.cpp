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
#include "chewy/cursor.h"
#include "chewy/events.h"
#include "chewy/globals.h"
#include "chewy/main.h"
#include "chewy/menus.h"
#include "chewy/mouse.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Dialogs {

static const int16 ANI_INVENT_END[3] = { 7, 16, 24 };


int16 Inventory::inv_rand_x;
int16 Inventory::inv_rand_y;
int Inventory::keyVal;


void Inventory::plot_menu() {
	_G(out)->setPointer(_G(workptr));
	buildMenu(WIN_INVENTORY);

	for (int16 j = 0; j < 3; j++) {
		for (int16 i = 0; i < 5; i++)
			_G(out)->boxFill(WIN_INF_X + 14 + i * 54, WIN_INF_Y + 6 + 30 + j * 32,
				WIN_INF_X + 14 + i * 54 + 40, WIN_INF_Y + 6 + 30 + j * 32 + 24, 12);
	}

	int16 y;
	int16 k = _G(in)->findHotspot(_G(inventoryHotspots));
	if (k != -1) {
		if (k < 5)
			_G(out)->boxFill(_G(inventoryHotspots)[k].left, _G(inventoryHotspots)[k].top,
	  						 _G(inventoryHotspots)[k].right + 1, _G(inventoryHotspots)[k].bottom + 5, 41);
		else {
			int16 x = (g_events->_mousePos.x - (WIN_INF_X)) / 54;
			y = (g_events->_mousePos.y - (WIN_INF_Y + 4 + 30)) / 30;
			k = x + (y * 5);
			k += _G(gameState).InventY * 5;
			if (k < (_G(gameState).InventY + 3) * 5)
				_G(out)->boxFill(WIN_INF_X + 14 + x * 54, WIN_INF_Y + 6 + 30 + y * 32,
					WIN_INF_X + 14 + x * 54 + 40, WIN_INF_Y + 6 + 30 + y * 32 + 24, 41);
		}
	}

	if (inv_rand_x != -1) {
		_G(out)->boxFill(WIN_INF_X + 14 + inv_rand_x * 54, WIN_INF_Y + 6 + 30 + inv_rand_y * 32,
			WIN_INF_X + 14 + inv_rand_x * 54 + 40, WIN_INF_Y + 6 + 30 + inv_rand_y * 32 + 24, 41);
		_G(out)->box(WIN_INF_X + 14 + inv_rand_x * 54 - 1, WIN_INF_Y + 6 + 30 + inv_rand_y * 32 - 1,
			WIN_INF_X + 14 + inv_rand_x * 54 + 40 + 1, WIN_INF_Y + 6 + 30 + inv_rand_y * 32 + 24 + 1, 14);
	}

	for (int16 i = 0; i < 2; i++) {
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
		_G(out)->spriteSet(_G(curtaf)->image[_G(ani_count)[i]],
			WIN_INF_X + 8 + i * 32, WIN_INF_Y + 12 - y, _G(scr_width));
	}

	for (int16 i = 0; i < 2; i++) {
		_G(out)->spriteSet(_G(menutaf)->image[PFEIL_UP + i],
			WIN_INF_X + 200 + i * 40, WIN_INF_Y + 12, _G(scr_width));
	}

	y = WIN_INF_Y + 6 + 30;
	for (int16 j = 0; j < 3; j++) {
		for (int16 i = 0; i < 5; i++) {
			if (_G(gameState).InventSlot[(_G(gameState).InventY + j) * 5 + i] != -1) {
				int16 *xy = (int16 *)_G(inv_spr)[_G(gameState).InventSlot[(_G(gameState).InventY + j) * 5 + i]];
				int16 x1 = 40 - xy[0];
				x1 /= 2;
				int16 y1 = 24 - xy[1];
				y1 /= 2;
				_G(out)->spriteSet(_G(inv_spr)[_G(gameState).InventSlot[(_G(gameState).InventY + j) * 5 + i]],
					x1 + WIN_INF_X + 14 + i * 54,
					y1 + y + 32 * j, _G(scr_width));
			}
		}
	}
}

void Inventory::menu() {
	keyVal = 0;
	_G(flags).InventMenu = true;
	const int16 oldDispFlag = _G(gameState).DispFlag;
	_G(gameState).DispFlag = false;
	const uint16 oldAutoAniPlay = _G(flags).AutoAniPlay;
	_G(flags).AutoAniPlay = true;
	_G(flags).StopAutoObj = true;
	_G(menu_display) = 0;
	_G(cur)->move(152, 92);
	g_events->_mousePos.x = 152;
	g_events->_mousePos.y = 92;

	_G(invent_cur_mode) = CUR_USE;
	if (_G(gameState).AkInvent != -1) {
		cursorChoice(CUR_AK_INVENT);

	} else {
		_G(invent_cur_mode) = CUR_USE;
		cursorChoice(CUR_USE);
	}

	int16 menu_flag1 = MENU_DISPLAY;
	int16 taste_flag = 28;
	g_events->_kbInfo._keyCode = '\0';
	bool mouseFl = true;

	for (int16 i = 0; i < 3; i++) {
		_G(ani_invent_delay)[i][0] = 30000;
		_G(ani_count)[i] = _G(ani_invent_anf)[i];
	}

	int16 ret_look = -1;
	bool menuFirstFl = false;
	_G(show_invent_menu) = 1;

	while (_G(show_invent_menu) == 1 && !SHOULD_QUIT) {
		if (!_G(minfo).button)
			mouseFl = false;
		if (_G(minfo).button == 1 || g_events->_kbInfo._keyCode == Common::KEYCODE_RETURN || keyVal) {
			if (!mouseFl) {
				mouseFl = true;
				g_events->_kbInfo._keyCode = '\0';

				int16 k = _G(in)->findHotspot(_G(inventoryHotspots));
				if (keyVal == Common::KEYCODE_F1)
					k = 0;
				else if (keyVal == Common::KEYCODE_F2)
					k = 1;
				else if (keyVal == Common::KEYCODE_RETURN)
					k = 5;

				keyVal = 0;

				switch (k) {
				case 0:
					_G(invent_cur_mode) = CUR_USE;
					_G(menu_item) = CUR_USE;
					if (_G(gameState).AkInvent == -1) {
						cursorChoice(CUR_USE);
					} else {
						cursorChoice(CUR_AK_INVENT);
					}
					break;

				case 1:
					if (_G(gameState).AkInvent != -1) {
						inv_rand_x = -1;
						inv_rand_y = -1;
						ret_look = look(_G(gameState).AkInvent, INV_ATS_MODE, -1);

						taste_flag = Common::KEYCODE_ESCAPE;
					} else {
						_G(invent_cur_mode) = CUR_LOOK;
						_G(menu_item) = CUR_LOOK;
						cursorChoice(CUR_LOOK);
					}
					break;

				case 3:
					_G(in)->_hotkey = Common::KEYCODE_PAGEUP;
					break;

				case 4:
					_G(in)->_hotkey = Common::KEYCODE_PAGEDOWN;
					break;

				case 5:
					inv_rand_x = (g_events->_mousePos.x - (WIN_INF_X)) / 54;
					inv_rand_y = (g_events->_mousePos.y - (WIN_INF_Y + 4 + 30)) / 30;
					k = inv_rand_x + (inv_rand_y * 5);
					k += _G(gameState).InventY * 5;
					if (_G(invent_cur_mode) == CUR_USE) {
						if (_G(gameState).AkInvent == -1) {
							if (_G(gameState).InventSlot[k] != -1 && calc_use_invent(_G(gameState).InventSlot[k]) == false) {
								_G(menu_item) = CUR_USE;
								_G(gameState).AkInvent = _G(gameState).InventSlot[k];
								cursorChoice(CUR_AK_INVENT);
								del_invent_slot(_G(gameState).InventSlot[k]);
							}
						} else if (_G(gameState).InventSlot[k] != -1)
							evaluateObj(_G(gameState).InventSlot[k], INVENTORY_NORMAL);
						else {
							_G(gameState).InventSlot[k] = _G(gameState).AkInvent;
							_G(obj)->sort();
							_G(gameState).AkInvent = -1;
							_G(menu_item) = _G(invent_cur_mode);
							cursorChoice(_G(invent_cur_mode));
						}
					} else if (_G(invent_cur_mode) == CUR_LOOK && _G(gameState).InventSlot[k] != -1 && calc_use_invent(_G(gameState).InventSlot[k]) == false) {
						_G(gameState).AkInvent = _G(gameState).InventSlot[k];
						ret_look = look(_G(gameState).InventSlot[k], INV_ATS_MODE, -1);
						_G(gameState).AkInvent = -1;
						cursorChoice(_G(invent_cur_mode));
						taste_flag = Common::KEYCODE_ESCAPE;
					}
					break;

				default:
					break;
				}
			}
		} else if (_G(minfo).button == 2 || g_events->_kbInfo._keyCode == Common::KEYCODE_ESCAPE) {
			if (!mouseFl) {
				// Set virtual key
				_G(in)->_hotkey = Common::KEYCODE_ESCAPE;
				mouseFl = true;
			}
		}

		if (ret_look == 0) {
			_G(invent_cur_mode) = CUR_USE;
			_G(menu_item) = CUR_USE;
			if (_G(gameState).AkInvent == -1)
				cursorChoice(CUR_USE);
			else
				cursorChoice(CUR_AK_INVENT);
		} else if (ret_look == 5) {
			taste_flag = false;
			mouseFl = false;
			_G(minfo).button = 1;
			keyVal = Common::KEYCODE_RETURN;
		}

		ret_look = -1;
		int16 abfrage = _G(in)->getSwitchCode();
		// The original hid the cursor here

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
				if (!menuFirstFl) {
					_G(cur)->show_cur();
					while (_G(in)->getSwitchCode() == Common::KEYCODE_ESCAPE) {
						setupScreen(NO_SETUP);
						inv_rand_x = -1;
						inv_rand_y = -1;
						plot_menu();
						_G(cur)->plot_cur();
						_G(out)->copyToScreen();
					}
				} else {
					if (menu_flag1 != MENU_DISPLAY) {
						menu_flag1 = MENU_HIDE;
						_G(show_invent_menu) = false;
					}
				}
				break;

			case Common::KEYCODE_RIGHT:
				if (g_events->_mousePos.x < 320 - _G(gameState)._curWidth)
					g_events->_mousePos.x += 3;
				break;

			case Common::KEYCODE_LEFT:
				if (g_events->_mousePos.x > 2)
					g_events->_mousePos.x -= 3;
				break;

			case Common::KEYCODE_UP:
				if (g_events->_mousePos.y > 2)
					g_events->_mousePos.y -= 3;
				break;

			case Common::KEYCODE_DOWN:
				if (g_events->_mousePos.y < 197 - _G(gameState)._curHeight)
					g_events->_mousePos.y += 3;
				break;

			case Common::KEYCODE_PAGEUP:
				if (_G(gameState).InventY > 0)
					--_G(gameState).InventY;
				g_events->_kbInfo._keyCode = '\0';
				break;

			case Common::KEYCODE_PAGEDOWN:
				if (_G(gameState).InventY < (MAX_MOV_OBJ / 5) - 3)
					++_G(gameState).InventY;
				g_events->_kbInfo._keyCode = '\0';
				break;

			default:
				break;
			}
			menuFirstFl = true;
		}

		if (_G(show_invent_menu) != 2) {
			setupScreen(NO_SETUP);
			if (menu_flag1 != MENU_HIDE) {
				inv_rand_x = -1;
				inv_rand_y = -1;
				plot_menu();
			}
			if (menu_flag1 == false)
				_G(cur)->plot_cur();
			_G(out)->setPointer(nullptr);
			if (menu_flag1 == MENU_DISPLAY) {
				_G(fx)->blende1(_G(workptr), nullptr, 0, 300);
			} else if (menu_flag1 == MENU_HIDE)
				_G(fx)->blende1(_G(workptr), nullptr, 1, 300);
			menu_flag1 = false;
			_G(out)->copyToScreen();
		} else {
			showCur();
		}
	}

	_G(cur)->move(_G(maus_old_x), _G(maus_old_y));
	g_events->_mousePos.x = _G(maus_old_x);
	g_events->_mousePos.y = _G(maus_old_y);
	_G(minfo).button = 0;

	while (_G(in)->getSwitchCode() == Common::KEYCODE_ESCAPE && !SHOULD_QUIT) {
		setupScreen(NO_SETUP);
		_G(cur)->plot_cur();
		_G(out)->copyToScreen();
	}

	_G(flags).InventMenu = false;
	_G(flags).AutoAniPlay = oldAutoAniPlay;
	_G(gameState).DispFlag = oldDispFlag;
	_G(menu_display) = _G(tmp_menu);
	_G(flags).StopAutoObj = false;
}

int16 Inventory::look(int16 invent_nr, int16 mode, int16 ats_nr) {
	int16 lineCount = 0;
	int16 xoff = 0;
	int16 yoff = 0;
	int16 visibleCount = 0;
	Common::String itemName;
	Common::StringArray itemDesc;
	char c[2] = { 0 };
	int16 ret = -1;
	bool endLoop = false;
	int16 startLine = 0;
	bool mouseFl = true;
	bool firstTime = true;

	if (mode == INV_ATS_MODE) {
		itemName = _G(atds)->getTextEntry(invent_nr, TXT_MARK_NAME, INV_ATS_DATA);
		itemDesc = _G(atds)->getTextArray(invent_nr, TXT_MARK_LOOK, INV_ATS_DATA);
		lineCount = itemDesc.size();
		xoff = itemName.size();
		xoff *= _G(font8)->getDataWidth();
		xoff = (254 - xoff) / 2;
		visibleCount = 2;
		yoff = 10;
	} else if (mode == INV_USE_ATS_MODE) {
		visibleCount = 3;
		yoff = 0;

		if (ats_nr >= 15000)
			itemDesc = _G(atds)->getTextArray(0, ats_nr - 15000, INV_USE_DEF, -1);
		else
			itemDesc = _G(atds)->getTextArray(0, ats_nr, INV_USE_DATA, -1);

		lineCount = itemDesc.size();
		if (itemDesc.size() == 0)
			endLoop = true;
	} else {
		endLoop = true;
	}

	const int16 speechId = _G(atds)->getLastSpeechId();

	while (!endLoop) {
		int16 rect = _G(in)->findHotspot(_G(inventoryHotspots));

		if (_G(minfo).button) {
			if (_G(minfo).button == 2) {
				if (!mouseFl)
					g_events->_kbInfo._scanCode = Common::KEYCODE_ESCAPE;
			} else if (_G(minfo).button == 1) {
				if (!mouseFl) {
					switch (rect) {
					case 0:
						endLoop = true;
						ret = 0;
						break;

					case 1:
						endLoop = true;
						ret = 1;
						break;

					case 3:
					case 6:
						g_events->_kbInfo._scanCode = Common::KEYCODE_UP;
						break;

					case 4:
					case 7:
						g_events->_kbInfo._scanCode = Common::KEYCODE_DOWN;
						break;

					case 5:
						ret = 5;
						endLoop = true;
						break;

					default:
						break;
					}
				}
			}

			mouseFl = true;
		} else {
			mouseFl = false;
		}

		switch (g_events->_kbInfo._scanCode) {
		case Common::KEYCODE_F1:
		case Common::KEYCODE_F2:
			_G(in)->_hotkey = g_events->_kbInfo._scanCode;
			break;

		case Common::KEYCODE_ESCAPE:
			endLoop = true;
			break;

		case Common::KEYCODE_UP:
			if (startLine > 0)
				--startLine;
			break;

		case Common::KEYCODE_DOWN:
			if (startLine < lineCount - visibleCount)
				++startLine;
			break;

		default:
			break;
		}

		g_events->_kbInfo._scanCode = Common::KEYCODE_INVALID;
		setupScreen(NO_SETUP);
		plot_menu();

		if (mode == INV_ATS_MODE) {
			_G(fontMgr)->setFont(_G(font8));
			_G(out)->printxy(WIN_LOOK_X + xoff, WIN_LOOK_Y, 255, 300,
							 _G(scr_width), itemName.c_str());
		}

		_G(fontMgr)->setFont(_G(font6));

		if (lineCount > visibleCount) {
			if (startLine > 0) {
				if (rect == 6)
					_G(out)->boxFill(WIN_INF_X + 262, WIN_INF_Y + 136, WIN_INF_X + 272,
						WIN_INF_Y + 136 + 14, 41);
				c[0] = 24;
				_G(out)->printxy(WIN_LOOK_X + 250, WIN_LOOK_Y + 4, 14, 300,
					_G(scr_width), c);
			}

			if (startLine < lineCount - visibleCount) {
				if (rect == 7)
					_G(out)->boxFill(WIN_INF_X + 262, WIN_INF_Y + 156, WIN_INF_X + 272,
						WIN_INF_Y + 156 + 14, 41);
				c[0] = 25;
				_G(out)->printxy(WIN_LOOK_X + 250, WIN_LOOK_Y + 24, 14, 300, _G(scr_width), c);
			}
		}

		int16 k = 0;

		if (itemDesc.size() > 0) {
			for (int16 i = startLine; i < lineCount && i < startLine + visibleCount; i++) {
				_G(out)->printxy(WIN_LOOK_X, WIN_LOOK_Y + yoff + k * 10, 14, 300,
								 _G(scr_width), itemDesc[i].c_str());
				++k;
			}

			if (g_engine->_sound->speechEnabled() && speechId >= 0 && firstTime) {
				g_engine->_sound->playSpeech(speechId, false);
				firstTime = false;
			}
		}

		_G(cur)->plot_cur();
		_G(out)->copyToScreen();
		SHOULD_QUIT_RETURN0;
	}

	while (_G(in)->getSwitchCode() == Common::KEYCODE_ESCAPE) {
		setupScreen(NO_SETUP);
		plot_menu();
		_G(cur)->plot_cur();
		_G(out)->copyToScreen();
		SHOULD_QUIT_RETURN0;
	}

	return ret;
}

void Inventory::look_screen(int16 txt_mode, int16 txt_nr) {
	int16 ok;
	int16 m_mode = 0;

	if (!_G(flags).AtsAction) {
		if (txt_nr != -1) {
			switch (txt_mode) {
			case INVENTORY_NORMAL:
			case INVENTORY_STATIC:
				ok = true;
				switch (_G(menu_item)) {
				case CUR_LOOK:
					m_mode = TXT_MARK_LOOK;
					break;

				case CUR_USE:
				case CUR_USER:
				case CUR_HOWARD:
				case CUR_NICHELLE:
					m_mode = TXT_MARK_USE;
					if (_G(gameState).inv_cur)
						ok = false;
					break;

				case CUR_WALK:
					m_mode = TXT_MARK_WALK;
					break;

				case CUR_TALK:
					m_mode = TXT_MARK_TALK;
					break;

				default:
					break;
				}

				if (_G(atds)->getControlBit(txt_nr, ATS_ACTION_BIT)) {
					atsAction(txt_nr, m_mode, ATS_ACTION_VOR);
				}
				if (ok) {
					startAtsWait(txt_nr, m_mode, 14, ATS_DATA);
				}

				if (_G(atds)->getControlBit(txt_nr, ATS_ACTION_BIT))
					atsAction(txt_nr, m_mode, ATS_ACTION_NACH);
				if (_G(menu_item) == CUR_USE)
					_G(flags).StaticUseTxt = true;
				break;

			default:
				break;
			}
		}
	}
}

bool Inventory::calc_use_invent(int16 invNr) {
	bool retVal = false;

	if (_G(menu_item) == CUR_LOOK) {
		switch (invNr) {
		case ZEITUNG_INV:
			Rooms::Room44::look_news();
			break;

		case CUTMAG_INV:
			_G(show_invent_menu) = 2;
			retVal = true;
			Rooms::Room58::look_cut_mag(58);
			break;

		case SPARK_INV:
			_G(show_invent_menu) = 2;
			retVal = true;
			save_person_rnr();
			Rooms::Room58::look_cut_mag(60);
			break;

		case DIARY_INV:
			showDiary();
			retVal = true;
			break;

		default:
			break;
		}
	} else if (_G(menu_item) == CUR_USE && invNr == NOTEBOOK_INV) {
		int16 id = del_invent_slot(NOTEBOOK_INV);
		_G(gameState).InventSlot[id] = NOTEBOOK_OPEN_INV;
		_G(obj)->changeInventory(NOTEBOOK_INV, NOTEBOOK_OPEN_INV, &_G(room_blk));
		retVal = true;
	}

	return retVal;
}

void Inventory::showDiary() {
	int16 scrollx = _G(gameState).scrollx,
		scrolly = _G(gameState).scrolly;
	_G(gameState).scrollx = 0;
	_G(gameState).scrolly = 0;

	_G(room)->load_tgp(DIARY_START, &_G(room_blk), GBOOK_TGP, false, GBOOK);
	_G(out)->setPointer(_G(workptr));
	_G(out)->map_spr2screen(_G(ablage)[_G(room_blk).AkAblage], _G(gameState).scrollx, _G(gameState).scrolly);
	_G(out)->copyToScreen();
	_G(room)->set_ak_pal(&_G(room_blk));
	_G(out)->setPointer(nullptr);
	_G(fx)->blende1(_G(workptr), _G(pal), 0, 0);

	while (_G(in)->getSwitchCode() != Common::KEYCODE_ESCAPE) {
		g_events->update();
		SHOULD_QUIT_RETURN;
	}
	while (_G(in)->getSwitchCode() != Common::KEYCODE_INVALID) {
		g_events->update();
		SHOULD_QUIT_RETURN;
	}

	_G(room)->load_tgp(_G(gameState)._personRoomNr[P_CHEWY], &_G(room_blk), EPISODE1_TGP, true, EPISODE1);
	_G(gameState).scrollx = scrollx;
	_G(gameState).scrolly = scrolly;
	setupScreen(NO_SETUP);
	plot_menu();
	_G(out)->setPointer(nullptr);
	_G(room)->set_ak_pal(&_G(room_blk));
	_G(fx)->blende1(_G(workptr), _G(pal), 0, 0);
}

} // namespace Dialogs
} // namespace Chewy
