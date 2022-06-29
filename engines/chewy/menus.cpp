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

#include "chewy/cursor.h"
#include "chewy/defines.h"
#include "chewy/detail.h"
#include "chewy/events.h"
#include "chewy/globals.h"
#include "chewy/main.h"
#include "chewy/mcga_graphics.h"
#include "chewy/menus.h"
#include "chewy/mouse.h"

namespace Chewy {

void plotMainMenu() {
	static const int IMAGES[] = { 7, 8, 9, 10, 12, 11 };
	int16 zoomX, zoomY;

	if (_G(menu_item) != _G(tmp_menu)) {
		_G(m_flip) = 0;
		_G(tmp_menu) = _G(menu_item);
	}

	// Don't allow the mouse to warp past the right edge, as this messes
	// up the menu screen
	if (g_events->_mousePos.x > 266)
		g_events->warpMouse(Common::Point(266, g_events->_mousePos.y));

	const int16 x = MAX(g_events->_mousePos.x - 32, 0);
	_G(menu_item) = (x / (MOUSE_MENU_MAX_X / 5));

	int16 *correction = (int16 *)_G(menutaf)->correction;

	for (int16 i = MENU_START_SPRITE; i < MAX_MENU_SPRITE; i++) {
		int deltaX = 0;

		if (i <= 8) {
			zoomX = 0;
			zoomY = 0;
		} else {
			zoomX = -3;
			zoomY = -3;

			if (i == 11)
				deltaX = 40;
			else if (i == 12)
				deltaX = -40;
		}

		_G(out)->scale_set(_G(menutaf)->image[i],
			MENU_X + deltaX + correction[i * 2],
		    _G(gameState).MainMenuY + correction[i * 2 + 1],
			zoomX, zoomY, 0);
	}

	zoomX = 16;
	zoomY = 16;
	++_G(m_flip);
	if (_G(m_flip) < 12 * (_G(gameState).DelaySpeed + 1)) {
		int deltaX = 0;
		if (_G(menu_item) == CUR_SAVE)
			deltaX = -40;
		else if (_G(menu_item) == CUR_INVENT)
			deltaX = 40;

		int img = IMAGES[_G(menu_item)];
		_G(out)->scale_set(_G(menutaf)->image[img],
		    MENU_X + deltaX + correction[img * 2] - 5,
		    _G(gameState).MainMenuY + correction[img * 2 + 1] - 10,
			zoomX, zoomY, 0);
	} else {
		if (_G(m_flip) > 15 * (_G(gameState).DelaySpeed + 1))
			_G(m_flip) = 0;
	}
}

void calcTxtXy(int16 *x, int16 *y, char *txtAdr, int16 txtCount) {
	int16 len = 0;
	for (int16 i = 0; i < txtCount; i++) {
		int16 tmpLen = strlen(_G(txt)->strPos(txtAdr, i));
		if (tmpLen > len)
			len = tmpLen;
	}
	len = len * _G(fontMgr)->getFont()->getDataWidth();
	int16 pixLen = len / 2;
	*x = *x - pixLen + 12;
	if (*x > (SCREEN_WIDTH - len))
		*x = SCREEN_WIDTH - len;
	else if (*x < 0)
		*x = 0;
	*y = *y - (10 * txtCount);
	if (*y < 0)
		*y = 0;
}

void calcTxtXy(int16 *x, int16 *y, Common::StringArray &textList) {
	int16 len = 0;
	for (int16 i = 0; i < (int16)textList.size(); i++) {
		if ((int16)textList[i].size() > len)
			len = textList[i].size();
	}
	len = len * _G(fontMgr)->getFont()->getDataWidth();
	int16 pixLen = len / 2;
	*x = *x - pixLen + 12;
	if (*x > (SCREEN_WIDTH - len))
		*x = SCREEN_WIDTH - len;
	else if (*x < 0)
		*x = 0;
	*y = *y - (10 * textList.size());
	if (*y < 0)
		*y = 0;
}

void getDisplayCoord(int16 *x, int16 *y, int16 nr) {
	int16 *xy = (int16 *)_G(inv_spr)[nr];
	int16 x1 = 48 - xy[0];
	x1 /= 2;
	*x = x1;
	int16 y1 = 48 - xy[1];
	y1 /= 2;
	*y = y1;
}

void buildMenu(int16 x, int16 y, int16 xNr, int16 yNr, int16 col, int16 mode) {
#define BUILDING_MENU_CORNER_LO 15
#define BUILDING_MENU_CORNER_RO 16
#define BUILDING_MENU_CORNER_LU 17
#define BUILDING_MENU_CORNER_RU 18
#define BUILDING_MENU_BELOW 19
#define BUILDING_MENU_SIDE_L 20
#define BUILDING_MENU_SIDE_R 21
#define BUILDING_MENU_ABOVE 22
#define BUILDING_MENU_ABOVE_L 23
#define BUILDING_MENU_ABOVE_R 24
#define BUILDING_MENU_ABOVE_M 25
	int16 i, j;
	int16 xy[4][2] = {{0, 0}, {0, 0}, {0, 0}, {0, 0}};
	uint8 spriteCornerNr[4] = { BUILDING_MENU_CORNER_LO, BUILDING_MENU_CORNER_RO, BUILDING_MENU_CORNER_LU, BUILDING_MENU_CORNER_RU };

	xy[0][0] = x;
	xy[0][1] = y;
	xy[1][0] = x + 16 * (xNr - 1);
	xy[1][1] = y;
	xy[2][0] = x;
	xy[2][1] = y + 16 * (yNr - 1);
	xy[3][0] = x + 16 * (xNr - 1);
	xy[3][1] = y + 16 * (yNr - 1);

	for (i = 0; i < 4; i++)
		_G(out)->spriteSet(_G(menutaf)->image[(int16)spriteCornerNr[i]],
		                 xy[i][0], xy[i][1], _G(scr_width));

	int16 s_nr = BUILDING_MENU_SIDE_L;
	for (j = 0; j < 2; j++) {
		y = xy[j][1] + 16;
		for (i = 0; i < yNr - 2; i++) {
			_G(out)->spriteSet(_G(menutaf)->image[s_nr], xy[j][0], y + i * 16, _G(scr_width));
		}
		++s_nr;
	}

	s_nr = BUILDING_MENU_ABOVE;
	for (j = 0; j < 2; j++) {
		x = xy[j * 2][0] + 16;
		if ((!mode) || (mode == 1 && j == 1)) {
			for (i = 0; i < xNr - 2; i++) {
				_G(out)->spriteSet(_G(menutaf)->image[s_nr], x + i * 16, xy[j * 2][1], _G(scr_width));
			}
		}
		s_nr -= 3;
	}

	if (mode) {
		int16 leer = 0;
		int16 center;
		i = xNr - 7;
		if (i == 0)
			center = 1;
		else if (i < 6)
			center = i + 1;
		else if (i < 8) {
			leer = 1;
			center = i - 1;
		} else {
			leer = 2;
			center = i - 3;
		}

		x = xy[0][0] + 16;
		for (j = 0; j < 2; j++) {
			for (i = 0; i < leer; i++)
				_G(out)->spriteSet(_G(menutaf)->image[BUILDING_MENU_ABOVE], x + i * 16, xy[0][1], _G(scr_width));
			x = xy[1][0] - leer * 16;
		}
		_G(out)->spriteSet(_G(menutaf)->image[BUILDING_MENU_ABOVE_L], xy[0][0] + 16 + leer * 16, xy[0][1], _G(scr_width));

		x = xy[0][0] + 16 + leer * 16 + 32;
		for (i = 0; i < center; i++)
			_G(out)->spriteSet(_G(menutaf)->image[BUILDING_MENU_ABOVE_M], x + i * 16, xy[0][1], _G(scr_width));
		_G(out)->spriteSet(_G(menutaf)->image[BUILDING_MENU_ABOVE_R], x + i * 16, xy[0][1], _G(scr_width));
	}

	_G(out)->boxFill(xy[0][0] + 16, xy[0][1] + 16, xy[0][0] + 16 + (xNr - 2) * 16, xy[0][1] + 16 + (yNr - 2) * 16, col);
}

void autoMenu(int16 *x, int16 *y, int16 lineNr, int16 height, char *text, int16 mode) {
	int16 tmp;

	int16 x_pix = 0;
	for (int16 i = 0; i < lineNr; i++) {
		tmp = strlen(_G(txt)->strPos(text, i));
		if (x_pix < tmp)
			x_pix = tmp;
	}
	x_pix *= _G(font8)->getDataWidth();
	x_pix += 12;
	tmp = x_pix;
	if (x_pix % 16)
		tmp += 16;
	int16 tmp1 = lineNr * height + 6 + 11;
	if (tmp1 % 16)
		tmp1 += 16;
	*x -= ((tmp / 16) * 16) / 2;
	*y -= ((tmp1 / 16) * 16) / 2;
	buildMenu(*x, *y, tmp / 16, tmp1 / 16, 60, mode);
	*x += 6 + (((tmp / 16) * 16) - x_pix) / 2;
	*y += 5 + ((((tmp1 / 16) * 16) - 4) - (lineNr * height)) / 2;
}

#define ADS_WIN 0,153,20,3,60,1

void handleDialogCloseupMenu() {
	int16 curYStart;
	int16 col;

	if (_G(flags).DialogCloseup) {
		_G(flags).ShowAtsInvTxt = false;
		_G(flags).MainInput = false;
		if (_G(ads_item_nr) > 4)
			curYStart = 190;
		else
			curYStart = 190 - (4 - _G(ads_item_nr)) * 10;
		int16 curY = g_events->_mousePos.y;
		if (curY < 160 || curY > curYStart + 10)
			curY = 255;
		else
			curY = (curYStart + 8 - curY) / 10;

		if (_G(atds)->aadGetStatus() == -1 && _G(ads_push) == false &&
		        _G(flags).NoDiaBox == false) {
			_G(cur_display) = true;

			buildMenu(ADS_WIN);
			_G(fontMgr)->setFont(_G(font6));
			if (_G(ads_item_nr) > 4)
				curYStart = 190;
			else
				curYStart = 190 - (4 - _G(ads_item_nr)) * 10;
			for (int16 i = 0; i < _G(ads_item_nr) && i < 4; i++) {
				if (curY == i)
					col = 255;
				else
					col = 14;
				_G(out)->printxy(4, curYStart - i * 10, col, 300, 0, _G(dialogCloseupItemPtr)[i]);
			}
		}

		switch (_G(in)->getSwitchCode()) {
		case 255:
		case Common::KEYCODE_RETURN:
			if (curY < _G(ads_item_nr) && curY >= 0 && _G(ads_push) == false) {
				_G(cur_display) = false;
				_G(ads_push) = true;
				g_events->_mousePos.y = 159;
				DialogCloseupNextBlock *an_blk = _G(atds)->dialogCloseupItemChoice(_G(ads_dia_nr), _G(ads_blk_nr), curY);
				if (an_blk->_blkNr == -1) {
					selectDialogOption(_G(ads_dia_nr), _G(ads_blk_nr), an_blk->_endNr);
					endDialogCloseup(_G(ads_dia_nr), _G(ads_blk_nr), an_blk->_endNr);
					stopDialogCloseupDialog();
				} else {
					an_blk = _G(atds)->calcNextDialogCloseupBlock(_G(ads_dia_nr), _G(ads_blk_nr), curY);
					selectDialogOption(_G(ads_dia_nr), _G(ads_blk_nr), an_blk->_endNr);
					_G(ads_blk_nr) = an_blk->_blkNr;
					_G(dialogCloseupItemPtr) = _G(atds)->dialogCloseupItemPtr(_G(ads_dia_nr), _G(ads_blk_nr), &_G(ads_item_nr));
				}
				_G(det)->stop_detail(_G(talk_start_ani));
				_G(det)->showStaticSpr(_G(talk_hide_static));
				_G(talk_start_ani) = -1;
				_G(talk_hide_static) = -1;
			}
			break;

		default:
			_G(ads_push) = false;
			break;
		}
	}
}

void stopDialogCloseupDialog() {
	aadWait(-1);
	_G(gameState).DispFlag = _G(ads_tmp_dsp);
	_G(cur_display) = true;
	_G(flags).ShowAtsInvTxt = true;
	_G(flags).MainInput = true;
	_G(flags).DialogCloseup = false;
	_G(mouseLeftClick) = false;
	_G(atds)->stopDialogCloseup();
	if (_G(minfo).button)
		_G(flags).mainMouseFlag = 1;
}

void cur_2_inventory() {
	if (_G(cur)->usingInventoryCursor()) {
		invent_2_slot(_G(cur)->getInventoryCursor());
		_G(cur)->setInventoryCursor(-1);
		_G(menu_item) = CUR_WALK;
		cursorChoice(_G(menu_item));
	}
	_G(cur)->setInventoryCursor(-1);
}

void inventory_2_cur(int16 nr) {
	if (!_G(cur)->usingInventoryCursor() && _G(obj)->checkInventory(nr)) {
		del_invent_slot(nr);
		_G(menu_item) = CUR_USE;
		_G(cur)->setInventoryCursor(nr);
		getDisplayCoord(&_G(gameState).DispZx, &_G(gameState).DispZy, _G(cur)->getInventoryCursor());
	}
}

void new_invent_2_cur(int16 inv_nr) {
	cur_2_inventory();
	_G(obj)->addInventory(inv_nr, &_G(room_blk));
	inventory_2_cur(inv_nr);
}

void invent_2_slot(int16 nr) {
	for (int16 i = 0; i < MAX_MOV_OBJ; i++) {
		if (_G(gameState).InventSlot[i] == -1) {
			_G(gameState).InventSlot[i] = nr;
			break;
		}
	}
	_G(obj)->addInventory(nr, &_G(room_blk));
}

int16 del_invent_slot(int16 nr) {
	int16 ok = -1;
	for (int16 i = 0; i < MAX_MOV_OBJ; i++) {
		if (_G(gameState).InventSlot[i] == nr) {
			_G(gameState).InventSlot[i] = -1;
			if (ok == -1)
				ok = i;
		}
	}

	return ok;
}

void remove_inventory(int16 nr) {
	if (nr == _G(cur)->getInventoryCursor()) {
		delInventory(nr);
	} else {
		_G(obj)->delInventory(nr, &_G(room_blk));
		del_invent_slot(nr);
	}
}

} // namespace Chewy
