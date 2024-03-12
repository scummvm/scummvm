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

#include "m4/gui/gui_item.h"
#include "m4/platform/keys.h"
#include "m4/graphics/gr_draw.h"
#include "m4/graphics/gr_line.h"
#include "m4/graphics/gr_pal.h"
#include "m4/mem/mem.h"
#include "m4/vars.h"

namespace M4 {

#define _REDRAW_ALL 2
#define _REDRAW_ACTIVE 4
#define _INIT_LISTBOX 8

#define _LB_BLACK	0x00
#define _LB_LTGREY	0x07
#define _LB_DKGREY	0x08
#define _LB_WHITE	0x0f
#define _LB_BROWN	0x07

#define STR_ITEM "gui item"
#define STR_LIST "gui list item"
#define STR_PROMPT "prompt"

// Interface sprites
#define scrollUpWidth		11
#define scrollUpHeight		12
static const byte scrollUpData[] = {
	_LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY,
	_LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY,  _LB_WHITE, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY,
	_LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY,  _LB_WHITE,  _LB_BLACK,  _LB_WHITE, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY,
	_LB_LTGREY, _LB_LTGREY, _LB_LTGREY,  _LB_WHITE,  _LB_BLACK,  _LB_BROWN,  _LB_BROWN,  _LB_WHITE, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY,
	_LB_LTGREY, _LB_LTGREY,  _LB_WHITE,  _LB_BLACK,  _LB_BROWN,  _LB_BROWN,  _LB_BROWN,  _LB_BROWN,  _LB_WHITE, _LB_LTGREY, _LB_LTGREY,
	_LB_LTGREY,  _LB_WHITE,  _LB_BLACK,  _LB_BROWN,  _LB_BROWN,  _LB_BROWN,  _LB_BROWN,  _LB_BROWN, _LB_DKGREY,  _LB_WHITE, _LB_LTGREY,
	_LB_LTGREY,  _LB_WHITE,  _LB_WHITE,  _LB_WHITE,  _LB_BROWN,  _LB_BROWN, _LB_DKGREY,  _LB_WHITE,  _LB_WHITE,  _LB_WHITE, _LB_LTGREY,
	_LB_LTGREY, _LB_LTGREY, _LB_LTGREY,  _LB_WHITE,  _LB_BROWN,  _LB_BROWN, _LB_DKGREY,  _LB_WHITE, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY,
	_LB_LTGREY, _LB_LTGREY, _LB_LTGREY,  _LB_WHITE,  _LB_BROWN,  _LB_BROWN, _LB_DKGREY,  _LB_WHITE, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY,
	_LB_LTGREY, _LB_LTGREY, _LB_LTGREY,  _LB_WHITE, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY,  _LB_WHITE, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY,
	_LB_LTGREY, _LB_LTGREY, _LB_LTGREY,  _LB_WHITE,  _LB_WHITE,  _LB_WHITE,  _LB_WHITE,  _LB_WHITE, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY,
	_LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY
};
static const Buffer scrollUpBuff = { scrollUpWidth, scrollUpHeight, const_cast<uint8 *>(&scrollUpData[0]), 0, 0 };

static const byte scrollUpPressedData[] = {
	_LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY,
	_LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY,  _LB_WHITE, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY,
	_LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY,  _LB_WHITE, _LB_DKGREY,  _LB_WHITE, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY,
	_LB_DKGREY, _LB_DKGREY, _LB_DKGREY,  _LB_WHITE, _LB_DKGREY,  _LB_BROWN,  _LB_BROWN,  _LB_WHITE, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY,
	_LB_DKGREY, _LB_DKGREY,  _LB_WHITE, _LB_DKGREY,  _LB_BROWN,  _LB_BROWN,  _LB_BROWN,  _LB_BROWN,  _LB_WHITE, _LB_DKGREY, _LB_DKGREY,
	_LB_DKGREY,  _LB_WHITE, _LB_DKGREY,  _LB_BROWN,  _LB_BROWN,  _LB_BROWN,  _LB_BROWN, _LB_LTGREY,  _LB_BLACK,  _LB_WHITE, _LB_DKGREY,
	_LB_DKGREY,  _LB_WHITE,  _LB_WHITE,  _LB_WHITE,  _LB_BROWN,  _LB_BROWN, _LB_LTGREY,  _LB_WHITE,  _LB_WHITE,  _LB_WHITE, _LB_DKGREY,
	_LB_DKGREY, _LB_DKGREY, _LB_DKGREY,  _LB_WHITE,  _LB_BROWN,  _LB_BROWN, _LB_LTGREY,  _LB_WHITE, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY,
	_LB_DKGREY, _LB_DKGREY, _LB_DKGREY,  _LB_WHITE,  _LB_BROWN,  _LB_BROWN,  _LB_BLACK,  _LB_WHITE, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY,
	_LB_DKGREY, _LB_DKGREY, _LB_DKGREY,  _LB_WHITE, _LB_LTGREY,  _LB_BLACK,  _LB_BLACK,  _LB_WHITE, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY,
	_LB_DKGREY, _LB_DKGREY, _LB_DKGREY,  _LB_WHITE,  _LB_WHITE,  _LB_WHITE,  _LB_WHITE,  _LB_WHITE, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY,
	_LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY,
};
static const Buffer scrollUpPressedBuff = { scrollUpWidth, scrollUpHeight, const_cast<uint8 *>(&scrollUpPressedData[0]), 0, 0 };

#define scrollDownWidth		11
#define scrollDownHeight	12
static const byte scrollDownData[] = {
	_LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY,
	_LB_LTGREY, _LB_LTGREY, _LB_LTGREY,  _LB_WHITE,  _LB_WHITE,  _LB_WHITE,  _LB_WHITE,  _LB_WHITE, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY,
	_LB_LTGREY, _LB_LTGREY, _LB_LTGREY,  _LB_WHITE,  _LB_BLACK,  _LB_BLACK,  _LB_BLACK,  _LB_WHITE, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY,
	_LB_LTGREY, _LB_LTGREY, _LB_LTGREY,  _LB_WHITE,  _LB_BLACK,  _LB_BROWN,  _LB_BROWN,  _LB_WHITE, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY,
	_LB_LTGREY, _LB_LTGREY, _LB_LTGREY,  _LB_WHITE,  _LB_BLACK,  _LB_BROWN,  _LB_BROWN,  _LB_WHITE, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY,
	_LB_LTGREY,  _LB_WHITE,  _LB_WHITE,  _LB_WHITE,  _LB_BLACK,  _LB_BROWN,  _LB_BROWN,  _LB_WHITE,  _LB_WHITE,  _LB_WHITE, _LB_LTGREY,
	_LB_LTGREY,  _LB_WHITE,  _LB_BLACK,  _LB_BLACK,  _LB_BROWN,  _LB_BROWN,  _LB_BROWN,  _LB_BROWN, _LB_DKGREY,  _LB_WHITE, _LB_LTGREY,
	_LB_LTGREY, _LB_LTGREY,  _LB_WHITE,  _LB_BROWN,  _LB_BROWN,  _LB_BROWN,  _LB_BROWN, _LB_DKGREY,  _LB_WHITE, _LB_LTGREY, _LB_LTGREY,
	_LB_LTGREY, _LB_LTGREY, _LB_LTGREY,  _LB_WHITE,  _LB_BROWN,  _LB_BROWN, _LB_DKGREY,  _LB_WHITE, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY,
	_LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY,  _LB_WHITE, _LB_DKGREY,  _LB_WHITE, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY,
	_LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY,  _LB_WHITE, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY,
	_LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY
};
static const Buffer scrollDownBuff = { scrollDownWidth, scrollDownHeight, const_cast<uint8 *>(&scrollDownData[0]), 0, 0 };

static const byte scrollDownPressedData[] = {
	_LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY,
	_LB_DKGREY, _LB_DKGREY, _LB_DKGREY,  _LB_WHITE,  _LB_WHITE,  _LB_WHITE,  _LB_WHITE,  _LB_WHITE, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY,
	_LB_DKGREY, _LB_DKGREY, _LB_DKGREY,  _LB_WHITE, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY,  _LB_WHITE, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY,
	_LB_DKGREY, _LB_DKGREY, _LB_DKGREY,  _LB_WHITE, _LB_DKGREY,  _LB_BROWN,  _LB_BROWN,  _LB_WHITE, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY,
	_LB_DKGREY, _LB_DKGREY, _LB_DKGREY,  _LB_WHITE, _LB_DKGREY,  _LB_BROWN,  _LB_BROWN,  _LB_WHITE, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY,
	_LB_DKGREY,  _LB_WHITE,  _LB_WHITE,  _LB_WHITE, _LB_DKGREY,  _LB_BROWN,  _LB_BROWN,  _LB_WHITE,  _LB_WHITE,  _LB_WHITE, _LB_DKGREY,
	_LB_DKGREY,  _LB_WHITE, _LB_DKGREY, _LB_DKGREY,  _LB_BROWN,  _LB_BROWN,  _LB_BROWN,  _LB_BROWN,  _LB_BLACK,  _LB_WHITE, _LB_DKGREY,
	_LB_DKGREY, _LB_DKGREY,  _LB_WHITE,  _LB_BROWN,  _LB_BROWN,  _LB_BROWN,  _LB_BROWN,  _LB_BLACK,  _LB_WHITE, _LB_DKGREY, _LB_DKGREY,
	_LB_DKGREY, _LB_DKGREY, _LB_DKGREY,  _LB_WHITE,  _LB_BROWN,  _LB_BROWN,  _LB_BLACK,  _LB_WHITE, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY,
	_LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY,  _LB_WHITE,  _LB_BLACK,  _LB_WHITE, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY,
	_LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY,  _LB_WHITE, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY,
	_LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY
};
static const Buffer scrollDownPressedBuff = { scrollDownWidth, scrollDownHeight, const_cast<uint8 *>(&scrollDownPressedData[0]), 0, 0 };

#define thumbWidth			11
#define thumbHeight			11
static const byte thumbData[] = {
	_LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY,
	_LB_LTGREY, _LB_LTGREY, _LB_LTGREY,  _LB_BLACK,  _LB_BLACK,  _LB_BLACK,  _LB_BLACK,  _LB_BLACK, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY,
	_LB_LTGREY, _LB_LTGREY,  _LB_BLACK,  _LB_BROWN,  _LB_BROWN,  _LB_BROWN,  _LB_BROWN,  _LB_BROWN,  _LB_WHITE, _LB_LTGREY, _LB_LTGREY,
	_LB_LTGREY, _LB_LTGREY,  _LB_BLACK, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY,  _LB_WHITE, _LB_LTGREY, _LB_LTGREY,
	_LB_LTGREY, _LB_LTGREY,  _LB_BLACK,  _LB_BROWN,  _LB_BROWN,  _LB_BROWN,  _LB_BROWN,  _LB_BROWN,  _LB_WHITE, _LB_LTGREY, _LB_LTGREY,
	_LB_LTGREY, _LB_LTGREY,  _LB_BLACK, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY,  _LB_WHITE, _LB_LTGREY, _LB_LTGREY,
	_LB_LTGREY, _LB_LTGREY,  _LB_BLACK,  _LB_BROWN,  _LB_BROWN,  _LB_BROWN,  _LB_BROWN,  _LB_BROWN,  _LB_WHITE, _LB_LTGREY, _LB_LTGREY,
	_LB_LTGREY, _LB_LTGREY,  _LB_BLACK, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY,  _LB_WHITE, _LB_LTGREY, _LB_LTGREY,
	_LB_LTGREY, _LB_LTGREY,  _LB_BLACK,  _LB_BROWN,  _LB_BROWN,  _LB_BROWN,  _LB_BROWN,  _LB_BROWN,  _LB_WHITE, _LB_LTGREY, _LB_LTGREY,
	_LB_LTGREY, _LB_LTGREY, _LB_LTGREY,  _LB_WHITE,  _LB_WHITE,  _LB_WHITE,  _LB_WHITE,  _LB_WHITE,  _LB_WHITE, _LB_LTGREY, _LB_LTGREY,
	_LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY, _LB_LTGREY
};
static const Buffer thumbBuff = { thumbWidth, thumbHeight, const_cast<uint8 *>(&thumbData[0]), 0, 0 };

static const byte thumbPressedData[] = {
	_LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY,
	_LB_DKGREY, _LB_DKGREY, _LB_DKGREY,  _LB_WHITE,  _LB_WHITE,  _LB_WHITE,  _LB_WHITE,  _LB_WHITE, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY,
	_LB_DKGREY, _LB_DKGREY,  _LB_WHITE,  _LB_BROWN,  _LB_BROWN,  _LB_BROWN,  _LB_BROWN,  _LB_BROWN,  _LB_BLACK, _LB_DKGREY, _LB_DKGREY,
	_LB_DKGREY, _LB_DKGREY,  _LB_WHITE, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY,  _LB_BLACK, _LB_DKGREY, _LB_DKGREY,
	_LB_DKGREY, _LB_DKGREY,  _LB_WHITE,  _LB_BROWN,  _LB_BROWN,  _LB_BROWN,  _LB_BROWN,  _LB_BROWN,  _LB_BLACK, _LB_DKGREY, _LB_DKGREY,
	_LB_DKGREY, _LB_DKGREY,  _LB_WHITE, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY,  _LB_BLACK, _LB_DKGREY, _LB_DKGREY,
	_LB_DKGREY, _LB_DKGREY,  _LB_WHITE,  _LB_BROWN,  _LB_BROWN,  _LB_BROWN,  _LB_BROWN,  _LB_BROWN,  _LB_BLACK, _LB_DKGREY, _LB_DKGREY,
	_LB_DKGREY, _LB_DKGREY,  _LB_WHITE, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY,  _LB_BLACK, _LB_DKGREY, _LB_DKGREY,
	_LB_DKGREY, _LB_DKGREY,  _LB_WHITE,  _LB_BROWN,  _LB_BROWN,  _LB_BROWN,  _LB_BROWN,  _LB_BROWN,  _LB_BLACK, _LB_DKGREY, _LB_DKGREY,
	_LB_DKGREY, _LB_DKGREY, _LB_DKGREY,  _LB_BLACK,  _LB_BLACK,  _LB_BLACK,  _LB_BLACK,  _LB_BLACK,  _LB_BLACK, _LB_DKGREY, _LB_DKGREY,
	_LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY, _LB_DKGREY
};
static const Buffer thumbPressedBuff = { thumbWidth, thumbHeight, const_cast<uint8 *>(&thumbPressedData[0]), 0, 0 };

#define minThumbY (scrollUpHeight + _G(items).buttonHeight)
#define maxThumbY (scrollDownHeight + thumbHeight + (_G(items).buttonHeight<<1))
#define thumbYRange (scrollUpHeight + scrollUpHeight + thumbHeight + (_G(items).buttonHeight<<2) + _G(items).buttonHeight - 1)

static void Item_Clear_origPrompt();

bool sizeofGUIelement_border(int16 el_type, int32 *w, int32 *h) {
	if ((!w) || (!h))
		return false;

	if ((el_type == MESSAGE) || (el_type == PICTURE)) {
		*w = 0;
		*h = 0;
	} else {
		*w = 3;
		*h = 3;
	}

	return true;
}

bool sizeofGUIelement_interior(ButtonDrawRec *bdr, M4Rect *myRect) {
	if ((!myRect) || (!bdr))
		return false;

	if ((bdr->el_type == MESSAGE) || (bdr->el_type == PICTURE)) {
		myRect->x1 = bdr->x1;
		myRect->y1 = bdr->y1;
		myRect->x2 = bdr->x2;
		myRect->y2 = bdr->y2;
	} else if ((bdr->el_type == TEXTFIELD) || (bdr->el_type == LISTBOX) || (!bdr->pressed)) {
		myRect->x1 = bdr->x1 + 2;
		myRect->y1 = bdr->y1 + 2;
		myRect->x2 = bdr->x2 - 1;
		myRect->y2 = bdr->y2 - 1;
	} else {
		myRect->x1 = bdr->x1 + 1;
		myRect->y1 = bdr->y1 + 1;
		myRect->x2 = bdr->x2 - 2;
		myRect->y2 = bdr->y2 - 2;
	}

	return true;
}

bool drawGUIelement(ButtonDrawRec *bdr, M4Rect *myRect) {
	int32 topLeftColor, interiorColor, bottomRightColor, bottomLeftPix, topRightPix;
	int32 x1 = bdr->x1, y1 = bdr->y1, x2 = bdr->x2, y2 = bdr->y2;
	bool pressed = bdr->pressed;

	if (!bdr)
		return false;

	if (bdr->el_type == MESSAGE) {
		sizeofGUIelement_interior(bdr, myRect);
		return true;
	}

	if ((bdr->el_type == TEXTFIELD) || (bdr->el_type == LISTBOX)) {
		if (!pressed) {
			topLeftColor = __DKGRAY;
			bottomRightColor = __WHITE;
			interiorColor = __LTGRAY;
		} else {
			topLeftColor = __BLACK;
			bottomRightColor = __LTGRAY;
			interiorColor = __DKGRAY;
		}
		bottomLeftPix = topLeftColor;
		topRightPix = bottomRightColor;
	} else {
		if (!pressed) {
			topLeftColor = __WHITE;
			interiorColor = __LTGRAY;
			bottomRightColor = __DKGRAY;
			bottomLeftPix = topLeftColor;
			topRightPix = bottomRightColor;
		} else {
			topLeftColor = __BLACK;
			interiorColor = __DKGRAY;
			bottomRightColor = __WHITE;
			bottomLeftPix = bottomRightColor;
			topRightPix = topLeftColor;
		}
	}
	if (bdr->fillMe == FILL_INTERIOR) {
		gr_color_set(interiorColor);
		gr_buffer_rect_fill(bdr->scrBuf, x1 + 1, y1 + 1, x2 - x1 - 1, y2 - y1 - 1);
	}

	gr_color_set(topLeftColor);
	gr_hline(bdr->scrBuf, x1, x2 - 1, y1);									// Top inner line
	gr_vline(bdr->scrBuf, x1, y1 + 1, y2 - 1);								// Left inner edge

	gr_color_set(bottomRightColor);
	gr_hline(bdr->scrBuf, x1 + 1, x2, y2);									// Bottom outer edge
	gr_vline(bdr->scrBuf, x2, y1 + 1, y2 - 1);								// Right outer edge

	buffer_put_pixel(bdr->scrBuf, x1, y2, (byte)bottomLeftPix);				// Bottom left pixel
	buffer_put_pixel(bdr->scrBuf, x2, y1, (byte)topRightPix);				// Top right pixel

	sizeofGUIelement_interior(bdr, myRect);
	return true;
}

bool InitItems() {
	_G(items).origPrompt = nullptr;
	_G(items).undoPrompt = nullptr;
	_G(items).currTextField = nullptr;
	_G(items).clipBoard[0] = '\0';
	_G(items).clipBoard[99] = '\0';
	if (!sizeofGUIelement_border(BUTTON, &_G(items).buttonWidth, &_G(items).buttonHeight)) return false;
	return true;
}

Item *Item_create(Item *parent, enum ItemType type, int32 tag, M4CALLBACK cb) {
	Item *temp;
	if ((temp = (Item *)mem_alloc(sizeof(Item), STR_ITEM)) == nullptr)
		return nullptr;

	temp->callback = cb;
	temp->type = type;
	temp->prompt = nullptr;

	temp->tag = tag;
	temp->x = temp->y = temp->w = temp->h = 0;

	temp->myList = nullptr;

	// add to end of list
	if (parent) {
		while (parent->next)
			parent = parent->next;
		parent->next = temp;
		temp->prev = parent;
	} else temp->prev = nullptr;
	temp->next = nullptr;
	return temp;
}

void Item_destroy(Item *myItem) {
	ListItem *myList, *tempListItem;
	myList = myItem->myList;
	tempListItem = myList;
	while (tempListItem) {
		myList = myList->next;
		mem_free((void *)tempListItem);
		tempListItem = myList;
	}
	if (myItem->prompt) mem_free(myItem->prompt);
	mem_free((void *)myItem);
}

void Item_empty_list(Item *myItem) {
	ListItem *myList, *tempListItem;
	myList = myItem->myList;
	tempListItem = myList;
	while (tempListItem) {
		myList = myList->next;
		mem_free((void *)tempListItem);
		tempListItem = myList;
	}
	myItem->myList = nullptr;
	myItem->currItem = nullptr;
	myItem->viewTop = nullptr;
	myItem->viewBottom = nullptr;
	myItem->myListCount = 0;
	myItem->viewIndex = 0;
	myItem->thumbY = minThumbY;
}

static int32 item_string_width(char *myStr, int32 spacing) {
	char *tempPtr, *tempPtr2, *tempPtr3, highlightChar[2];
	int32 highlightNum;
	int32 strWidth, column;

	highlightChar[0] = '~';
	highlightChar[1] = '\0';
	if (!myStr) return 0;
	column = 0;
	if ((tempPtr2 = strrchr(myStr, '^')) != nullptr) {
		*tempPtr2 = '\0';
		if ((tempPtr = strrchr(myStr, '^')) == nullptr) return 0;
		tempPtr++;
		column = (int32)atoi(tempPtr);
		*tempPtr2 = '^';
		tempPtr2++;
	} else tempPtr2 = myStr;
	tempPtr = strchr(tempPtr2, '~');
	highlightNum = 0;
	while (tempPtr) {
		highlightNum++;
		tempPtr3 = tempPtr + 1;
		tempPtr = strchr(tempPtr3, '~');
	}
	strWidth = column + gr_font_string_width(tempPtr2, spacing) - (gr_font_string_width(highlightChar, spacing) * highlightNum);
	return strWidth;
}

static int32 item_string_write(Buffer *target, char *myStr, int32 x, int32 y, int32 w, int32 spacing, int32 color, int32 highlight) {
	char *tempPtr, *strPtr, *nextStrPtr;
	char *highlightPtr;
	char tempStr[2];
	int32 currX, column;

	if (!target) return false;
	if ((tempPtr = strchr(myStr, '^')) == nullptr) {
		if ((tempPtr = strchr(myStr, '~')) == nullptr) {
			gr_font_set_color((char)color);
			gr_font_write(target, myStr, x, y, w, spacing);
			return true;
		}
	}
	tempStr[1] = '\0';
	column = 0;
	strPtr = myStr;
	while (strPtr) {
		column = 0;
		if (*strPtr == '^') {
			if ((tempPtr = strchr((char *)(strPtr + 1), '^')) == nullptr) return false;
			*tempPtr = '\0';
			strPtr++;
			column = (int32)atoi(strPtr);
			*tempPtr = '^';
			strPtr = tempPtr + 1;
		}
		currX = x + column;
		nextStrPtr = strchr(strPtr, '^');
		if (nextStrPtr) *nextStrPtr = '\0';
		highlightPtr = strchr(strPtr, '~');
		while (highlightPtr) {
			if (highlightPtr != strPtr) {
				*highlightPtr = '\0';
				gr_font_set_color((char)color);
				gr_font_write(target, strPtr, currX, y, 0, spacing);
				currX += gr_font_string_width(strPtr, spacing);
				*highlightPtr = '~';
			}
			if (*(highlightPtr + 1) == '\0') return true;
			tempStr[0] = *(highlightPtr + 1);
			gr_font_set_color((char)highlight);
			gr_font_write(target, tempStr, currX, y, 0, spacing);
			currX += gr_font_string_width(tempStr, spacing);
			strPtr = highlightPtr + 2;
			highlightPtr = strchr(strPtr, '~');
		}
		if (*strPtr != '\0') {
			gr_font_set_color((char)color);
			gr_font_write(target, strPtr, currX, y, 0, spacing);
		}
		if (nextStrPtr) *nextStrPtr = '^';
		strPtr = nextStrPtr;
	}
	return true;
}

static void CorrectItemWidthHeight(Item *item, int32 fontHeight) {
	int32 tempWidth, tempHeight, minWidth, minHeight;

	if (!item)
		return;

	switch (item->type) {
	case LISTBOX:
		minHeight = scrollUpHeight + thumbHeight + scrollDownHeight + _G(items).buttonHeight * 3;	//scrollup + thumb + scrolldown heights + (newHeight for each)
		if (!sizeofGUIelement_border(LISTBOX, &tempWidth, &tempHeight)) return;
		minWidth = scrollUpWidth + thumbWidth + scrollDownWidth + _G(items).buttonWidth * 3 + (tempWidth + 1);	//same as above + width of listbox
		if (item->h < minHeight) item->h = minHeight;
		if (item->w < minWidth) item->w = minWidth;
		break;
	case MESSAGE:
		if (!sizeofGUIelement_border(MESSAGE, &tempWidth, &tempHeight)) return;
		item->w = item_string_width(item->prompt, 1) + tempWidth;
		item->h = fontHeight + tempHeight;
		break;
	case TEXTFIELD:
		if (!sizeofGUIelement_border(TEXTFIELD, &tempWidth, &tempHeight)) return;
		item->h = fontHeight + tempHeight + 1;
		break;
	case BUTTON:
	case REPEAT_BUTTON:
		if (!sizeofGUIelement_border(BUTTON, &tempWidth, &tempHeight)) return;
		item->w = item_string_width(item->prompt, 1) + tempWidth;
		item->h = fontHeight + tempHeight + 1;
		break;
	case PICTURE:
	default:
		break;
	}
}

Item *ItemAdd(Item *itemList, int32 x, int32 y, int32 w, int32 h, const char *prompt, int32 tag, enum ItemType type, M4CALLBACK cb, int32 promptMax) {
	Item *item;
	int32				fontHeight, listboxWidth, listboxHeight;
	Font *myFont;

	if ((item = Item_create(itemList, type, tag, cb)) == nullptr) return nullptr;
	myFont = gr_font_get();
	fontHeight = gr_font_get_height();
	item->myFont = myFont;
	item->x = x;
	item->y = y;
	item->w = w;
	item->h = h;
	item->status = ITEM_NORMAL;
	item->callback = cb;

	switch (type) {
	case LISTBOX:
		item->prompt = nullptr;
		item->myListCount = 0;
		item->viewIndex = 0;
		item->thumbY = minThumbY;
		item->myList = nullptr;
		item->currItem = nullptr;
		item->viewTop = nullptr;
		item->viewBottom = nullptr;
		break;
	case PICTURE:
		item->myFont = nullptr;
		item->prompt = nullptr;
		// FIXME: Refactor out const_cast
		item->aux = const_cast<char *>(prompt);
		break;
	case TEXTFIELD:
		if ((int)strlen(prompt) > (promptMax + 1))
			item->promptMax = strlen(prompt) + 1;
		else
			item->promptMax = promptMax + 1;

		item->prompt = (char *)mem_alloc(item->promptMax, STR_PROMPT);
		Common::strcpy_s(item->prompt, 256, prompt);
		item->aux = &(item->prompt[strlen(item->prompt)]);
		item->aux2 = item->aux;
		break;
	case MESSAGE:
	case BUTTON:
	case REPEAT_BUTTON:
		item->prompt = mem_strdup(prompt);
		break;

	default:
		break;
	}

	CorrectItemWidthHeight(item, fontHeight);
	if (type == LISTBOX) {
		if (!sizeofGUIelement_border(LISTBOX, &listboxWidth, &listboxHeight)) {
			Item_destroy(item);
			return nullptr;
		}
		item->listView = (item->h - listboxHeight - 2) / fontHeight;
	}
	return item;
}

Item *ItemFind(Item *itemList, int32 tag) {
	if (!itemList)
		return nullptr;
	if (tag > 0) {
		while (itemList && (itemList->tag != tag)) itemList = itemList->next;
	} else {
		itemList = nullptr;
	}

	return itemList;
}

bool Item_SetViewBottom(Item *i) {
	int32 count;
	ListItem *myListItem;
	bool found = false;

	if ((!i) || (!i->viewTop)) return false;
	myListItem = i->viewTop;
	for (count = 1; (count < i->listView) && myListItem->next; count++) {
		if (myListItem == i->currItem) found = true;
		myListItem = myListItem->next;
	}
	if (myListItem == i->currItem) found = true;
	if (count == i->listView) i->viewBottom = myListItem;
	else i->viewBottom = nullptr;
	return found;
}

static void CalculateViewIndex(Item *myItem) {
	ListItem *myListItem;
	int32 i;
	if ((!myItem) || (!myItem->viewTop)) return;
	i = 0;
	myListItem = myItem->myList;
	while (myListItem != myItem->viewTop) {
		i++;
		myListItem = myListItem->next;
	}
	myItem->viewIndex = i;
	if (myItem->myListCount > myItem->listView) {
		if (myItem->viewBottom && myItem->viewBottom->next) {
			myItem->thumbY = minThumbY + (((myItem->h - thumbYRange) * myItem->viewIndex) / (myItem->myListCount - myItem->listView));
		} else myItem->thumbY = myItem->h - maxThumbY;
	} else myItem->thumbY = minThumbY;
}

static void SetViewIndex(Item *myItem) {
	ListItem *myListItem;
	int32 i;

	if ((!myItem) || (!myItem->myList)) return;
	if (myItem->thumbY == myItem->h - maxThumbY) {
		myItem->viewIndex = myItem->myListCount - myItem->listView;
	} else {
		myItem->viewIndex = ((myItem->thumbY - minThumbY) * (myItem->myListCount - myItem->listView)) / (myItem->h - thumbYRange);
	}
	myListItem = myItem->myList;
	for (i = 0; i < myItem->viewIndex; i++) {
		myListItem = myListItem->next;
	}
	myItem->viewTop = myListItem;
	Item_SetViewBottom(myItem);
}

bool ListItemExists(Item *myItem, char *prompt, int32 listTag) {
	ListItem *myListItems;
	if (!myItem) return false;
	myListItems = myItem->myList;
	if (prompt) {
		while (myListItems && strcmp(myListItems->prompt, prompt)) {
			myListItems = myListItems->next;
		}
	} else {
		while (myListItems && (myListItems->tag != listTag)) {
			myListItems = myListItems->next;
		}
	}
	if (myListItems) return true;
	return false;
}

bool ListItemAdd(Item *myItem, char *prompt, int32 listTag, int32 addMode, ListItem *changedItem) {
	ListItem *myList;
	ListItem *newListItem;

	if (!myItem) return false;
	if (changedItem) newListItem = changedItem;
	else {
		if ((newListItem = (ListItem *)mem_alloc(sizeof(ListItem), STR_LIST)) == nullptr) return false;
		Common::strlcpy(newListItem->prompt, prompt, 80);
		newListItem->tag = listTag;
	}
	//Add it into the list in the correct place...
	myList = myItem->myList;
	if (!myList) {
		newListItem->prev = nullptr;
		newListItem->next = nullptr;
		myItem->myList = newListItem;
		myItem->currItem = newListItem;
		myItem->viewTop = newListItem;
		myItem->viewIndex = 0;
		myItem->thumbY = minThumbY;
	} else {
		switch (addMode) {
		case LIST_SEQUN:
			while (myList->next) myList = myList->next;
			myList->next = newListItem;
			newListItem->prev = myList;
			newListItem->next = nullptr;
			break;
		case LIST_ALPH:
			if (strcmp(newListItem->prompt, myList->prompt) <= 0) {		//add to front
				newListItem->prev = nullptr;
				newListItem->next = myList;
				myList->prev = newListItem;
				myItem->myList = newListItem;
				myItem->currItem = newListItem;
				myItem->viewTop = newListItem;
			} else {
				while (myList->next &&
					(strcmp(newListItem->prompt, myList->next->prompt) > 0)) {
					myList = myList->next;
				}
				if (myList->next) {
					newListItem->next = myList->next;
					newListItem->prev = myList;
					myList->next->prev = newListItem;
					myList->next = newListItem;
				} else {
					newListItem->next = nullptr;
					newListItem->prev = myList;
					myList->next = newListItem;
				}
			}
			break;
		case LIST_BY_TAG:
		default:
			if (newListItem->tag <= myList->tag) {		//add to front
				newListItem->prev = nullptr;
				newListItem->next = myList;
				myList->prev = newListItem;
				myItem->myList = newListItem;
				myItem->currItem = newListItem;
				myItem->viewTop = newListItem;
			} else {
				while (myList->next && (newListItem->tag > myList->next->tag)) {
					myList = myList->next;
				}
				if (myList->next) {
					newListItem->next = myList->next;
					newListItem->prev = myList;
					myList->next->prev = newListItem;
					myList->next = newListItem;
				} else {
					newListItem->next = nullptr;
					newListItem->prev = myList;
					myList->next = newListItem;
				}
			}
			break;
		}
		if (changedItem) {
			myItem->currItem = myList;
			myItem->viewTop = myList;
		}
	}
	myItem->myListCount++;
	if (!Item_SetViewBottom(myItem)) ViewCurrListItem(myItem);
	else CalculateViewIndex(myItem);
	return true;
}

bool ListItemDelete(Item *myItem, ListItem *myListItem, int32 listTag) {
	ListItem *myList;

	if (!myItem) return false;
	if (!myListItem) {
		myList = myItem->myList;
		while (myList && (myList->tag != listTag)) myList = myList->next;
	} else myList = myListItem;
	if (!myList) return false;
	if (myList == myItem->myList) {	//first in the list...
		myItem->myList = myItem->myList->next;
		if (myItem->myList) myItem->myList->prev = nullptr;
	} else {
		myList->prev->next = myList->next;
		if (myList->next) myList->next->prev = myList->prev;
	}
	if (myList == myItem->currItem) {
		if (myList->next) myItem->currItem = myList->next;
		else myItem->currItem = myList->prev;
	}
	if (myList == myItem->viewTop) {
		if (myItem->viewTop->prev) myItem->viewTop = myItem->viewTop->prev;
		else myItem->viewTop = myItem->viewTop->next;
	}
	myItem->myListCount--;
	Item_SetViewBottom(myItem);
	if (!myItem->viewBottom) ViewCurrListItem(myItem);
	else CalculateViewIndex(myItem);
	mem_free((void *)myList);
	return true;
}

bool ListItemChange(Item *myItem, ListItem *myListItem, int32 listTag,
	char *newPrompt, int32 newTag, int32 changeMode) {
	ListItem *myList;
	int32			oldTag;

	if (!myItem) return false;
	if (!myListItem) {
		myList = myItem->myList;
		while (myList && (myList->tag != listTag)) myList = myList->next;
	} else myList = myListItem;
	if (!myList) return false;
	if ((!strcmp(myList->prompt, newPrompt)) && (myList->tag == newTag)) return false;
	Common::strcpy_s(myList->prompt, newPrompt);
	oldTag = myList->tag;
	myList->tag = newTag;
	if (((changeMode == LIST_BY_TAG) && (oldTag != newTag)) || (changeMode == LIST_ALPH)) {
		if (myList == myItem->myList) {	//first in the list...
			myItem->myList = myItem->myList->next;
			if (myItem->myList) myItem->myList->prev = nullptr;
		} else {
			myList->prev->next = myList->next;
			if (myList->next) myList->next->prev = myList->prev;
		}
		ListItemAdd(myItem, nullptr, 0, changeMode, myList);
	}
	return true;
}

bool GetNextListItem(Item *myItem) {
	ListItem *nextItem;
	if (myItem->currItem) {
		nextItem = myItem->currItem->next;
		if (!nextItem) return false;
		if (myItem->currItem == myItem->viewBottom) {
			myItem->viewBottom = nextItem;
			myItem->viewTop = myItem->viewTop->next;
			myItem->viewIndex++;
			if (myItem->viewBottom && myItem->viewBottom->next) {
				myItem->thumbY = minThumbY + (((myItem->h - thumbYRange) * myItem->viewIndex) / (myItem->myListCount - myItem->listView));
			} else myItem->thumbY = myItem->h - maxThumbY;
		}
		myItem->currItem = nextItem;
		return true;
	}
	return false;
}

bool GetNextPageList(Item *myItem) {
	int32 i;
	bool changed = false;

	if (myItem->currItem && myItem->viewBottom) {
		for (i = 0; i < (myItem->listView - 1); i++) {
			if (myItem->viewBottom->next) {
				if (myItem->currItem == myItem->viewTop) myItem->currItem = myItem->currItem->next;
				myItem->viewTop = myItem->viewTop->next;
				myItem->viewBottom = myItem->viewBottom->next;
				myItem->viewIndex++;
				changed = true;
			} else break;
		}
		if (myItem->viewBottom && myItem->viewBottom->next) {
			myItem->thumbY = minThumbY + (((myItem->h - thumbYRange) * myItem->viewIndex) / (myItem->myListCount - myItem->listView));
		} else myItem->thumbY = myItem->h - maxThumbY;
	}
	return changed;
}

bool GetPrevListItem(Item *myItem) {
	ListItem *prevItem;
	if (myItem->currItem) {
		prevItem = myItem->currItem->prev;
		if (!prevItem) return false;
		if (myItem->currItem == myItem->viewTop) {
			myItem->viewTop = prevItem;
			myItem->viewBottom = myItem->viewBottom->prev;
			myItem->viewIndex--;
			myItem->thumbY = minThumbY + (((myItem->h - thumbYRange) * myItem->viewIndex) / (myItem->myListCount - myItem->listView));
		}
		myItem->currItem = prevItem;
		return true;
	}
	return false;
}

bool GetPrevPageList(Item *myItem) {
	int32 i;
	bool changed = false;

	if (myItem->currItem && myItem->viewBottom) {
		for (i = 0; i < (myItem->listView - 1); i++) {
			if (myItem->viewTop->prev) {
				if (myItem->currItem == myItem->viewBottom) myItem->currItem = myItem->currItem->prev;
				myItem->viewTop = myItem->viewTop->prev;
				myItem->viewBottom = myItem->viewBottom->prev;
				myItem->viewIndex--;
				changed = true;
			} else break;
		}
		myItem->thumbY = minThumbY + (((myItem->h - thumbYRange) * myItem->viewIndex) / (myItem->myListCount - myItem->listView));
	}
	return changed;
}

void ViewCurrListItem(Item *myItem) {
	int32 i, j;
	ListItem *tempItem;
	bool breakFlag = false;

	if (!myItem->currItem) {
		myItem->viewTop = nullptr;
		myItem->viewBottom = nullptr;
		return;
	}
	myItem->viewTop = myItem->currItem;
	tempItem = myItem->currItem;
	i = myItem->listView - 2;
	while (tempItem->next && (i > 0)) {
		tempItem = tempItem->next;
		i--;
	}
	if (i > 0) {
		j = 0;
		while ((j <= i) && (!breakFlag)) {
			if (myItem->viewTop->prev) {
				myItem->viewTop = myItem->viewTop->prev;
				j++;
			} else breakFlag = true;
		}
		if (j > i) myItem->viewBottom = tempItem;
		else myItem->viewBottom = nullptr;
	} else if (!tempItem->next) {
		if (myItem->viewTop->prev) {
			myItem->viewTop = myItem->viewTop->prev;
			myItem->viewBottom = tempItem;
		} else myItem->viewBottom = nullptr;
	} else myItem->viewBottom = tempItem->next;
	CalculateViewIndex(myItem);
}

ListItem *ListItemFind(Item *myItem, int32 searchMode, char *searchStr, int32 parm1) {
	ListItem *myList;
	int32 i;
	if (!myItem)
		return nullptr;

	myList = myItem->myList;
	if (searchMode == LIST_BY_TAG) {
		while (myList && (myList->tag != parm1)) myList = myList->next;
	} else if (searchMode == LIST_ALPH) {
		while (myList && scumm_strnicmp(myList->prompt, searchStr, strlen(searchStr)))
			myList = myList->next;
	} else if (searchMode == LIST_SEQUN) {
		i = 0;
		while (myList && (i < parm1)) {
			myList = myList->next;
			i++;
		}
	} else return nullptr;
	return myList;
}

bool ListItemSearch(Item *myItem, int32 searchMode, char *searchStr, int32 parm1) {
	ListItem *myList;
	myList = ListItemFind(myItem, searchMode, searchStr, parm1);
	if (!myList) {
		myItem->currItem = myItem->myList;
		myItem->viewTop = myItem->myList;
		Item_SetViewBottom(myItem);
		CalculateViewIndex(myItem);
		return false;
	} else {
		myItem->currItem = myList;
		if (!Item_SetViewBottom(myItem)) ViewCurrListItem(myItem);
		else CalculateViewIndex(myItem);
		return true;
	}
}

bool DoubleClickOnListBox(Item *myItem, int32 xOffset, int32 yOffset) {
	int32 listboxContentX2, itemOffset, fontHeight;
	ListItem *myListItem;
	M4Rect interiorRect;
	ButtonDrawRec	bdr;
	Font *currFont;

	if (!myItem)
		return false;
	currFont = gr_font_get();
	if (currFont != myItem->myFont)
		gr_font_set(myItem->myFont);
	fontHeight = gr_font_get_height();
	if (currFont != myItem->myFont)
		gr_font_set(currFont);

	if (myItem->myListCount > myItem->listView) {
		listboxContentX2 = myItem->w - scrollUpWidth - _G(items).buttonWidth - 1;
	} else listboxContentX2 = myItem->w - 1;

	bdr.el_type = LISTBOX;
	bdr.pressed = true;	//since this procedure will only be called myItem is the default item
	bdr.x1 = 0; bdr.y1 = 0; bdr.x2 = listboxContentX2; bdr.y2 = myItem->h - 1;
	if (!sizeofGUIelement_interior(&bdr, &interiorRect))
		return false;

	xOffset -= interiorRect.x1;
	yOffset -= interiorRect.y1;
	listboxContentX2 = interiorRect.x2 - interiorRect.x1;
	if ((xOffset < 0) || (xOffset > listboxContentX2))
		return false;
	if ((yOffset < 0) || (yOffset > fontHeight *myItem->listView))
		return false;
	itemOffset = 0;
	myListItem = myItem->viewTop;
	while (myListItem && (itemOffset + fontHeight <= yOffset)) {
		myListItem = myListItem->next;
		itemOffset += fontHeight;
	}
	if (!myListItem)
		return false;
	myItem->currItem = myListItem;
	return true;
}

bool ClickOnListBox(Item *myItem, int32 xOffset, int32 yOffset, int32 scrollType) {
	int32 fontHeight, itemOffset, boxWidth;
	ListItem *myListItem;
	bool scrollable = false;
	bool changed = false;
	int32 newThumbY, i;
	static int32 thumbOffset;
	M4Rect interiorRect;
	ButtonDrawRec bdr;
	Font *currFont;

	if (!myItem) return false;

	if (myItem->myListCount > myItem->listView) {
		scrollable = true;
		boxWidth = myItem->w - (scrollUpHeight + _G(items).buttonHeight + 1);

		if (myItem->status & THUMB_PRESSED) {
			newThumbY = yOffset - thumbOffset;
			if (newThumbY < minThumbY) newThumbY = minThumbY;
			else if (newThumbY > (myItem->h - maxThumbY)) newThumbY = myItem->h - maxThumbY;
			myItem->thumbY = newThumbY;
			SetViewIndex(myItem);
			return true;
		} else if (scrollType && (!(myItem->status & BOX_PRESSED)) && (xOffset >= (myItem->w - scrollUpWidth - _G(items).buttonWidth)) && (xOffset < myItem->w)) {
			if ((yOffset >= 0) && (yOffset <= (scrollUpHeight + _G(items).buttonHeight - 1))) {
				if (((myItem->status & AREA_PRESSED) == 0) || (myItem->status & SU_PRESSED)) {
					myItem->status = SU_PRESSED;
					if (myItem->viewTop->prev) {
						myItem->viewTop = myItem->viewTop->prev;
						myItem->viewBottom = myItem->viewBottom->prev;
						myItem->viewIndex--;
						myItem->thumbY = minThumbY + (((myItem->h - thumbYRange) * myItem->viewIndex) / (myItem->myListCount - myItem->listView));
						changed = true;
					} else changed = false;
				} else changed = false;
			} else if ((yOffset >= (scrollUpHeight + _G(items).buttonHeight)) && (yOffset < myItem->thumbY)) {
				if ((scrollType & PAGEABLE) && (
						(myItem->status & AREA_PRESSED) == 0 || (myItem->status & PU_PRESSED))) {
					myItem->status = PU_PRESSED;
					for (i = 0; i < myItem->listView - 1; i++) {
						if (myItem->viewTop->prev) {
							myItem->viewTop = myItem->viewTop->prev;
							myItem->viewBottom = myItem->viewBottom->prev;
							myItem->viewIndex--;
							changed = true;
						} else break;
					}
					myItem->thumbY = minThumbY + (((myItem->h - thumbYRange) * myItem->viewIndex) / (myItem->myListCount - myItem->listView));
				} else changed = false;
			} else if ((yOffset >= (myItem->h - (scrollDownHeight + _G(items).buttonHeight))) && (yOffset < myItem->h)) {
				if (((myItem->status & AREA_PRESSED) == 0) || (myItem->status & SD_PRESSED)) {
					myItem->status = SD_PRESSED;
					if (myItem->viewBottom->next) {
						myItem->viewTop = myItem->viewTop->next;
						myItem->viewBottom = myItem->viewBottom->next;
						myItem->viewIndex++;
						if (myItem->viewBottom && myItem->viewBottom->next) {
							myItem->thumbY = minThumbY + (((myItem->h - thumbYRange) * myItem->viewIndex) / (myItem->myListCount - myItem->listView));
						} else myItem->thumbY = myItem->h - maxThumbY;
						changed = true;
					} else changed = false;
				} else changed = false;
			} else if ((scrollType & PAGEABLE) && (yOffset > (myItem->thumbY + thumbHeight + _G(items).buttonHeight)) && (yOffset < (myItem->h - scrollDownHeight - _G(items).buttonHeight))) {
				if (((myItem->status & AREA_PRESSED) == 0) || (myItem->status & PD_PRESSED)) {
					myItem->status = PD_PRESSED;
					for (i = 0; i < myItem->listView - 1; i++) {
						if (myItem->viewBottom->next) {
							myItem->viewTop = myItem->viewTop->next;
							myItem->viewBottom = myItem->viewBottom->next;
							myItem->viewIndex++;
							changed = true;
						} else break;
					}
					if (myItem->viewBottom && myItem->viewBottom->next) {
						myItem->thumbY = minThumbY + (((myItem->h - thumbYRange) * myItem->viewIndex) / (myItem->myListCount - myItem->listView));
					} else myItem->thumbY = myItem->h - maxThumbY;
				} else changed = false;
			} else if (((myItem->status & AREA_PRESSED) == 0) && (yOffset >= (myItem->thumbY)) && (yOffset < (myItem->thumbY + thumbHeight + _G(items).buttonHeight))) {
				myItem->status = THUMB_PRESSED;
				thumbOffset = yOffset - myItem->thumbY;
			} else changed = false;
			return changed;
		}
	} else boxWidth = myItem->w - 1;
	if ((xOffset < 0) || (xOffset > boxWidth)) return false;
	if (((myItem->status & AREA_PRESSED) == 0) || (myItem->status & BOX_PRESSED)) {
		myItem->status = (BOX_PRESSED | ITEM_PRESSED);
		currFont = gr_font_get();
		if (myItem->myFont != currFont)
			gr_font_set(myItem->myFont);
		fontHeight = gr_font_get_height();
		if (myItem->myFont != currFont)
			gr_font_set(currFont);

		bdr.el_type = LISTBOX;
		bdr.pressed = true;	//since this procedure will only be called myItem is the default item
		bdr.x1 = 0; bdr.y1 = 0; bdr.x2 = myItem->w - 1; bdr.y2 = myItem->h - 1;
		if (!sizeofGUIelement_interior(&bdr, &interiorRect)) return false;

		yOffset -= interiorRect.y1;
		if (yOffset < 0) {
			if (scrollType && scrollable && myItem->viewTop->prev) {
				myItem->viewTop = myItem->viewTop->prev;
				myItem->viewBottom = myItem->viewBottom->prev;
				myItem->viewIndex--;
				myItem->thumbY = minThumbY + (((myItem->h - thumbYRange) * myItem->viewIndex) / (myItem->myListCount - myItem->listView));
			}
			myListItem = myItem->viewTop;
		} else if (yOffset >= fontHeight * myItem->listView) {
			if (scrollType && scrollable && myItem->viewBottom->next) {
				myItem->viewTop = myItem->viewTop->next;
				myItem->viewBottom = myItem->viewBottom->next;
				myItem->viewIndex++;
				if (myItem->viewBottom && myItem->viewBottom->next) {
					myItem->thumbY = minThumbY + (((myItem->h - thumbYRange) * myItem->viewIndex) / (myItem->myListCount - myItem->listView));
				} else myItem->thumbY = myItem->h - maxThumbY;
			}
			myListItem = myItem->viewBottom;
		} else {
			itemOffset = 0;
			myListItem = myItem->viewTop;
			while (myListItem && (itemOffset + fontHeight <= yOffset)) {
				myListItem = myListItem->next;
				itemOffset += fontHeight;
			}
		}
		if ((!myListItem) || (myListItem == myItem->currItem))
			return false;
		myItem->currItem = myListItem;
		return true;
	}
	return false;
}

bool ResetDefaultListBox(Item *myItem) {
	bool changed;
	if (!myItem) return false;
	if ((myItem->status & AREA_PRESSED) == 0) changed = false;
	else if (myItem->status & BOX_PRESSED) changed = false;
	else changed = true;
	myItem->status = ITEM_NORMAL;
	return changed;
}

bool Item_change_prompt(Item *myItem, const char *newPrompt) {
	int32 fontHeight;
	Font *currFont;
	if (!strcmp(myItem->prompt, newPrompt)) return false;
	if (myItem->type == TEXTFIELD) {
		if ((int)strlen(newPrompt) >= myItem->promptMax)
			myItem->promptMax = strlen(newPrompt) + 1;
		Common::strcpy_s(myItem->prompt, 256, newPrompt);
		myItem->aux = &(myItem->prompt[strlen(myItem->prompt)]);
		myItem->aux2 = myItem->aux;
		Item_Clear_origPrompt();
	} else if (myItem->type == LISTBOX) {
		return false;
	} else {
		if (strlen(myItem->prompt) < strlen(newPrompt)) {
			mem_free(myItem->prompt);
			myItem->prompt = mem_strdup(newPrompt);
		} else Common::strcpy_s(myItem->prompt, 256, newPrompt);
	}
	currFont = gr_font_get();
	if (myItem->myFont != currFont)
		gr_font_set(myItem->myFont);
	fontHeight = gr_font_get_height();
	CorrectItemWidthHeight(myItem, fontHeight);
	if (myItem->myFont != currFont)
		gr_font_set(currFont);
	return true;
}

static void Item_Clear_origPrompt() {
	if (_G(items).origPrompt) {
		mem_free(_G(items).origPrompt);
		_G(items).origPrompt = nullptr;
	}
	if (_G(items).undoPrompt) {
		mem_free(_G(items).undoPrompt);
		_G(items).undoPrompt = nullptr;
	}
}

Item *Item_RestoreTextField(void) {
	if (!_G(items).origPrompt) return nullptr;
	Common::strcpy_s(_G(items).currTextField->prompt, 256, _G(items).origPrompt);
	_G(items).currTextField->aux = &(_G(items).currTextField->prompt[strlen(_G(items).currTextField->prompt)]);
	_G(items).currTextField->aux2 = _G(items).currTextField->aux;
	mem_free(_G(items).origPrompt);
	_G(items).origPrompt = nullptr;
	if (_G(items).undoPrompt) {
		mem_free(_G(items).undoPrompt);
		_G(items).undoPrompt = nullptr;
	}
	return _G(items).currTextField;
}

Item *Item_CheckTextField(void) {
	Item *myItem = nullptr;
	if (_G(items).origPrompt) {
		if (strcmp(_G(items).currTextField->prompt, _G(items).origPrompt)) myItem = _G(items).currTextField;
		Item_Clear_origPrompt();
	}
	return myItem;
}

void Item_SaveTextField(Item *myItem) {
	_G(items).origPrompt = mem_strdup(myItem->prompt);
	_G(items).currTextField = myItem;
}

static void Item_SaveTextFieldChange(Item *myItem, bool majorChange) {
	if (_G(items).undoPrompt && (!majorChange)) return;
	if (_G(items).undoPrompt) mem_free(_G(items).undoPrompt);
	_G(items).undoPrompt = mem_strdup(myItem->prompt);
	_G(items).undoAux = _G(items).undoPrompt + (myItem->aux - myItem->prompt);
	_G(items).undoAux2 = _G(items).undoPrompt + (myItem->aux2 - myItem->prompt);
}

static bool Item_UndoTextFieldChange(void) {
	char *tempBuf, *tempAux, *tempAux2;
	if (_G(items).undoPrompt) {
		tempBuf = mem_strdup(_G(items).currTextField->prompt);
		tempAux = tempBuf + (_G(items).currTextField->aux - _G(items).currTextField->prompt);
		tempAux2 = tempBuf + (_G(items).currTextField->aux2 - _G(items).currTextField->prompt);
		Common::strcpy_s(_G(items).currTextField->prompt, 256, _G(items).undoPrompt);
		_G(items).currTextField->aux = _G(items).currTextField->prompt + (_G(items).undoAux - _G(items).undoPrompt);
		_G(items).currTextField->aux2 = _G(items).currTextField->prompt + (_G(items).undoAux2 - _G(items).undoPrompt);
		mem_free(_G(items).undoPrompt);
		_G(items).undoPrompt = tempBuf;
		_G(items).undoAux = tempAux;
		_G(items).undoAux2 = tempAux2;
		return true;
	}
	return false;
}

void SetTextBlockBegin(Item *myItem, int32 relXPos) {
	char myChar, *scan;
	bool finished = false;
	M4Rect interiorRect;
	ButtonDrawRec bdr;

	bdr.el_type = TEXTFIELD;
	bdr.pressed = true;	//since this procedure will only be called myItem is the default item
	bdr.x1 = 0; bdr.y1 = 0; bdr.x2 = myItem->w - 1; bdr.y2 = myItem->h - 1;
	if (!sizeofGUIelement_interior(&bdr, &interiorRect)) return;
	relXPos -= interiorRect.x1;
	if (relXPos <= 0) myItem->aux = myItem->prompt;
	else if (relXPos >= item_string_width(myItem->prompt, 1)) {
		myItem->aux = &(myItem->prompt[strlen(myItem->prompt)]);
	} else {
		scan = &(myItem->prompt[1]);
		finished = false;
		while (!finished) {
			myChar = *scan;
			*scan = '\0';
			if (item_string_width(myItem->prompt, 1) > relXPos) {
				myItem->aux = scan - 1;
				finished = true;
			}
			*scan++ = myChar;
		}
	}
	myItem->aux2 = myItem->aux;
}

void SetTextBlockEnd(Item *myItem, int32 relXPos) {
	char myChar, *scan;
	bool finished = false;
	M4Rect interiorRect;
	ButtonDrawRec bdr;

	bdr.el_type = TEXTFIELD;
	bdr.pressed = true;	//since this procedure will only be called myItem is the default item
	bdr.x1 = 0; bdr.y1 = 0; bdr.x2 = myItem->w - 1; bdr.y2 = myItem->h - 1;
	if (!sizeofGUIelement_interior(&bdr, &interiorRect)) return;
	relXPos -= interiorRect.x1;
	if (relXPos <= 0) myItem->aux2 = myItem->prompt;
	else if (relXPos >= item_string_width(myItem->prompt, 1)) {
		myItem->aux2 = &(myItem->prompt[strlen(myItem->prompt)]);
	} else {
		scan = &(myItem->prompt[1]);
		finished = false;
		while (!finished) {
			myChar = *scan;
			*scan = '\0';
			if (item_string_width(myItem->prompt, 1) > relXPos) {
				myItem->aux2 = scan - 1;
				finished = true;
			}
			*scan++ = myChar;
		}
	}
}

static int32 CopyTextBlock(Item *myItem) {
	int32 numOfCopiedChars = 0;
	const char *beginBlock, *endBlock;

	if (myItem->aux != myItem->aux2) {
		if (myItem->aux < myItem->aux2) {
			beginBlock = myItem->aux;
			endBlock = myItem->aux2;
		} else {
			beginBlock = myItem->aux2;
			endBlock = myItem->aux;
		}
		numOfCopiedChars = endBlock - beginBlock;
		Common::strlcpy(_G(items).clipBoard, beginBlock, 100);
		if (endBlock - beginBlock <= 99) {
			_G(items).clipBoard[endBlock - beginBlock] = '\0';
		} else numOfCopiedChars = 99;
	}
	return numOfCopiedChars;
}

static int32 DeleteTextBlock(Item *myItem) {
	int32 numOfDeletedChars = 0;
	char tempBuf[80];
	char *beginBlock, *endBlock;

	if (myItem->aux != myItem->aux2) {
		Item_SaveTextFieldChange(myItem, true);
		if (myItem->aux < myItem->aux2) {
			beginBlock = myItem->aux;
			endBlock = myItem->aux2;
		} else {
			beginBlock = myItem->aux2;
			endBlock = myItem->aux;
		}
		numOfDeletedChars = endBlock - beginBlock;
		Common::strcpy_s(tempBuf, endBlock);
		Common::strcpy_s(beginBlock, 80, tempBuf);
		myItem->aux = beginBlock;
		myItem->aux2 = beginBlock;
	}
	return numOfDeletedChars;
}

bool Item_TextEdit(Item *myItem, int32 parm1) {
	char tempBuf[80];
	bool absorbed = true;
	switch (parm1) {
	case KEY_LEFT:
		if (myItem->aux < myItem->aux2) myItem->aux2 = myItem->aux;
		else if (myItem->aux > myItem->aux2) myItem->aux = myItem->aux2;
		else if (myItem->aux != myItem->prompt) {
			myItem->aux--;
			myItem->aux2--;
		}
		break;
	case KEY_RIGHT:
		if (myItem->aux < myItem->aux2) myItem->aux = myItem->aux2;
		else if (myItem->aux > myItem->aux2) myItem->aux2 = myItem->aux;
		else if (*(myItem->aux) != '\0') {
			myItem->aux++;
			myItem->aux2++;
		}
		break;
	case KEY_ALT_LEFT:
		if (myItem->aux2 != myItem->prompt) {
			myItem->aux2--;
		}
		break;
	case KEY_ALT_RIGHT:
		if (*(myItem->aux2) != '\0') {
			myItem->aux2++;
		}
		break;
	case KEY_ALT_UP:
		if (myItem->aux2 != myItem->prompt) {
			myItem->aux2 = myItem->prompt;
		}
		break;
	case KEY_ALT_DOWN:
		if (*(myItem->aux2) != '\0') {
			myItem->aux2 = &(myItem->prompt[strlen(myItem->prompt)]);
		}
		break;
	case KEY_HOME:
		myItem->aux = myItem->prompt;
		myItem->aux2 = myItem->aux;
		break;
	case KEY_END:
		myItem->aux = &(myItem->prompt[strlen(myItem->prompt)]);
		myItem->aux2 = myItem->aux;
		break;
	case KEY_DELETE:
		if (!_G(items).origPrompt) Item_SaveTextField(myItem);
		if ((DeleteTextBlock(myItem) <= 0) && (*(myItem->aux) != '\0')) {
			Item_SaveTextFieldChange(myItem, false);
			if (*(myItem->aux + 1) == '\0') {
				*(myItem->aux) = '\0';
			} else {
				Common::strcpy_s(tempBuf, myItem->aux + 1);
				Common::strcpy_s(myItem->aux, 80, tempBuf);
			}
			myItem->aux2 = myItem->aux;
		}
		break;
	case KEY_BACKSP:
		if (!_G(items).origPrompt) Item_SaveTextField(myItem);
		if ((DeleteTextBlock(myItem) <= 0) && (myItem->aux != myItem->prompt)) {
			Item_SaveTextFieldChange(myItem, false);
			if (*(myItem->aux) == '\0') {
				myItem->aux--;
				*(myItem->aux) = '\0';
			} else {
				Common::strcpy_s(tempBuf, myItem->aux);
				myItem->aux--;
				Common::strcpy_s(myItem->aux, 80, tempBuf);
			}
			myItem->aux2 = myItem->aux;
		}
		break;
	case KEY_ALT_X:
		if (!_G(items).origPrompt) Item_SaveTextField(myItem);
		if (CopyTextBlock(myItem) > 0) {
			DeleteTextBlock(myItem);
		}
		break;
	case KEY_ALT_C:
		CopyTextBlock(myItem);
		break;
	case KEY_ALT_V:
		if (!_G(items).origPrompt) Item_SaveTextField(myItem);
		if ((int)strlen(_G(items).clipBoard) && ((int)strlen(myItem->prompt) - abs(myItem->aux2 - myItem->aux) +
			(int)strlen(_G(items).clipBoard) < myItem->promptMax)) {
			if (DeleteTextBlock(myItem) <= 0) Item_SaveTextFieldChange(myItem, true);
			Common::strcpy_s(tempBuf, myItem->aux);
			Common::strcpy_s(myItem->aux, 100, _G(items).clipBoard);
			Common::strcat_s(myItem->prompt, 80, tempBuf);
			myItem->aux = myItem->aux + strlen(_G(items).clipBoard);
			myItem->aux2 = myItem->aux;
		}
		break;
	case KEY_ALT_Z:
		Item_UndoTextFieldChange();
		break;
	default:
		if (parm1 >= 32 && parm1 <= 127) {									// Visible key character...
			if (!_G(items).origPrompt) Item_SaveTextField(myItem);			// back up original...
			DeleteTextBlock(myItem);										// if a block was highlighted...
			if ((int)strlen(myItem->prompt) < (myItem->promptMax - 1)) {	// room in the string...
				Item_SaveTextFieldChange(myItem, false);					// save a minor change...
				if (*(myItem->aux) == '\0') {								// add to the end of the string...
					*(myItem->aux++) = (char)parm1;
					*(myItem->aux) = '\0';
				} else {
					Common::strcpy_s(tempBuf, myItem->aux);
					*(myItem->aux++) = (char)parm1;
					Common::strcpy_s(myItem->aux, 80, tempBuf);
				}
				myItem->aux2 = myItem->aux;
			}
		} else absorbed = false;
		break;
	}
	return absorbed;
}

bool Item_show(Item *i, void *bdrDialog, Buffer *scrBuf, int32 itemType) {
	int32 x1, y1, x2, y2;
	int32 listboxContentX2, fontHeight, viewCount, temp;
	char myChar, *beginBlock, *endBlock;
	ListItem *myList;
	Buffer pictBuff;
	const Buffer *tempBuff;
	ButtonDrawRec bdr;
	M4Rect interiorRect;
	Font *currFont;

	if (!i)
		return false;

	x1 = i->x;
	y1 = i->y;
	x2 = x1 + i->w - 1;
	y2 = y1 + i->h - 1;
	if ((x1 < 0) || (y1 < 0) || (x2 >= scrBuf->w) || (y2 >= scrBuf->h))
		return false;

	currFont = gr_font_get();
	if (i->myFont != currFont)
		gr_font_set(i->myFont);
	fontHeight = gr_font_get_height();

	gr_color_set(__LTGRAY);
	gr_buffer_rect_fill(scrBuf, x1, y1, i->w, i->h);

	bdr.el_type = i->type;
	bdr.dialog = bdrDialog;
	bdr.scrBuf = scrBuf;
	bdr.fillMe = FILL_INTERIOR;
	bdr.x1 = x1;
	bdr.y1 = y1;
	bdr.x2 = x2;
	bdr.y2 = y2;
	if (((i->type == TEXTFIELD) && (itemType == ITEM_DEFAULT)) || (i->status & ITEM_PRESSED)) {
		bdr.pressed = true;
	} else {
		bdr.pressed = false;
	}
	if ((i->type != LISTBOX) && (i->type != PICTURE)) {
		if (!drawGUIelement(&bdr, &interiorRect)) return false;
		x1 = interiorRect.x1;
		y1 = interiorRect.y1;
		x2 = interiorRect.x2;
		y2 = interiorRect.y2;
	}

	switch (i->type) {
	case MESSAGE:
		item_string_write(scrBuf, i->prompt, x1, y1, 0, 1, __BLACK, __BLUE);
		break;
	case PICTURE:
		pictBuff.w = i->w;
		pictBuff.h = i->h;
		pictBuff.stride = i->w;
		pictBuff.data = (uint8 *)(i->aux);
		gr_buffer_rect_copy_2(&pictBuff, scrBuf, 0, 0, x1, y1, pictBuff.w, pictBuff.h);
		break;
	case TEXTFIELD:
		if (itemType == ITEM_DEFAULT) {
			gr_font_set_color(__GREEN);
			gr_font_write(scrBuf, i->prompt, x1, y1, 0, 1);
			if (i->aux == i->aux2) {
				myChar = *(i->aux);
				*(i->aux) = '\0';
				temp = x1 + gr_font_string_width(i->prompt, 1);
				*(i->aux) = myChar;
				gr_color_set(__RED);
				gr_vline(scrBuf, temp, y1, y2);
			} else {
				if (i->aux < i->aux2) {
					beginBlock = i->aux;
					endBlock = i->aux2;
				} else {
					beginBlock = i->aux2;
					endBlock = i->aux;
				}
				myChar = *beginBlock;
				*beginBlock = '\0';
				temp = x1 + gr_font_string_width(i->prompt, 1);
				*beginBlock = myChar;
				myChar = *endBlock;
				*endBlock = '\0';
				gr_color_set(__RED);
				gr_buffer_rect_fill(scrBuf, temp, y1, gr_font_string_width(beginBlock, 1) + 1, fontHeight + 1);
				gr_font_set_color(__WHITE);
				gr_font_write(scrBuf, beginBlock, temp, y1, 0, 1);
				*endBlock = myChar;
			}
		} else {
			gr_font_set_color(__BLACK);
			gr_font_write(scrBuf, i->prompt, x1, y1, 0, 1);
		}
		break;

	case BUTTON:
	case REPEAT_BUTTON:
		gr_color_set(__DKGRAY);
		if (i->status & ITEM_PRESSED) {
			item_string_write(scrBuf, i->prompt, x1, y1, 0, 1, __RED, __BLUE);
		} else {
			item_string_write(scrBuf, i->prompt, x1, y1, 0, 1, __BLACK, __BLUE);
		}
		if (itemType == ITEM_RETURN) {
			bdr.fillMe = false; bdr.el_type = BUTTON; bdr.pressed = false;
			bdr.x1 = i->x - _G(items).buttonWidth;
			bdr.y1 = i->y - _G(items).buttonHeight;
			bdr.x2 = i->x + i->w + _G(items).buttonWidth - 1;
			bdr.y2 = i->y + i->h + _G(items).buttonHeight - 1;
			drawGUIelement(&bdr, nullptr);
		}
		break;

	case LISTBOX:
		if (i->myListCount > i->listView) {
			listboxContentX2 = x2 - (_G(items).buttonWidth + scrollUpWidth);

			//DRAW IN THE PIPES BEHIND THE THUMB
			temp = y1 + i->thumbY;
			bdr.pressed = false;
			bdr.fillMe = true;
			bdr.el_type = TEXTFIELD;
			bdr.x1 = listboxContentX2 + 4; bdr.y1 = y1 + (scrollUpHeight >> 1); bdr.x2 = x2 - 4; bdr.y2 = temp + (thumbHeight >> 1);
			drawGUIelement(&bdr, nullptr);
			bdr.x1 = listboxContentX2 + 4; bdr.y1 = temp + (thumbHeight >> 1); bdr.x2 = x2 - 4; bdr.y2 = y2 - (scrollDownHeight >> 1);
			drawGUIelement(&bdr, nullptr);
			bdr.el_type = BUTTON;

			if (i->status & THUMB_PRESSED) {
				bdr.pressed = true;
				tempBuff = &thumbPressedBuff;
			} else {
				bdr.pressed = false;
				tempBuff = &thumbBuff;
			}
			bdr.x1 = listboxContentX2 + 1; bdr.y1 = temp; bdr.x2 = x2; bdr.y2 = temp + thumbHeight + _G(items).buttonHeight - 1;
			drawGUIelement(&bdr, &interiorRect);
			gr_buffer_rect_copy_2(tempBuff, scrBuf, 0, 0, (short)interiorRect.x1, interiorRect.y1, thumbWidth, thumbHeight);

			if (i->status & SU_PRESSED) {
				bdr.pressed = true;
				tempBuff = &scrollUpPressedBuff;
			} else {
				bdr.pressed = false;
				tempBuff = &scrollUpBuff;
			}
			bdr.x1 = listboxContentX2 + 1; bdr.y1 = y1; bdr.x2 = x2; bdr.y2 = y1 + scrollUpHeight + _G(items).buttonHeight - 1;
			drawGUIelement(&bdr, &interiorRect);
			gr_buffer_rect_copy_2(tempBuff, scrBuf, 0, 0, (short)interiorRect.x1, interiorRect.y1, scrollUpWidth, scrollUpHeight);

			if (i->status & SD_PRESSED) {
				bdr.pressed = true;
				tempBuff = &scrollDownPressedBuff;
			} else {
				bdr.pressed = false;
				tempBuff = &scrollDownBuff;
			}
			bdr.x1 = listboxContentX2 + 1; bdr.y1 = y2 - (scrollDownHeight + _G(items).buttonHeight) + 1; bdr.x2 = x2; bdr.y2 = y2;
			drawGUIelement(&bdr, &interiorRect);
			gr_buffer_rect_copy_2(tempBuff, scrBuf, 0, 0, (short)interiorRect.x1, interiorRect.y1, scrollDownWidth, scrollDownHeight);
		} else {
			listboxContentX2 = x2 + 1;
		}
		if (itemType == ITEM_DEFAULT) bdr.pressed = true;
		else bdr.pressed = false;
		bdr.el_type = LISTBOX;
		bdr.x1 = x1; bdr.y1 = y1; bdr.x2 = listboxContentX2 - 1; bdr.y2 = y2;
		drawGUIelement(&bdr, &interiorRect);
		x1 = interiorRect.x1;
		y1 = interiorRect.y1;
		myList = i->viewTop;
		viewCount = 0;
		while (myList && (viewCount < i->listView)) {
			if (myList == i->currItem) {
				item_string_write(scrBuf, myList->prompt, x1, y1 + (fontHeight * (viewCount)), 0, 1, __RED, __BLUE);
			} else if (itemType == ITEM_DEFAULT) {
				item_string_write(scrBuf, myList->prompt, x1, y1 + (fontHeight * (viewCount)), 0, 1, __WHITE, __BLUE);
			} else {
				item_string_write(scrBuf, myList->prompt, x1, y1 + (fontHeight * (viewCount)), 0, 1, __BLACK, __BLUE);
			}
			viewCount++;
			myList = myList->next;
		}
		break;

	default:
		break;
	}

	if (i->myFont != currFont)
		gr_font_set(currFont);

	return true;
}

Item *Item_set_pressed(Item *itemList, Item *myItem, int32 tag) {
	if (!myItem) myItem = ItemFind(itemList, tag);
	if (myItem) myItem->status = (myItem->status & AREA_PRESSED) + ITEM_PRESSED;
	else return nullptr;
	return myItem;
}

Item *Item_set_unpressed(Item *itemList, Item *myItem, int32 tag) {
	if (!myItem) myItem = ItemFind(itemList, tag);
	if (myItem) myItem->status = ITEM_NORMAL;
	else return nullptr;
	return myItem;
}

Item *Item_set_default(Item *itemList, Item *currDefault, int32 tag) {
	Item *myItem = nullptr;
	if (tag > 0) {
		myItem = itemList;
		while (myItem && (myItem->tag != tag)) myItem = myItem->next;
	}
	if (myItem != currDefault) {
		if (currDefault) {
			currDefault->status = ITEM_NORMAL;
		}
		if (myItem && (myItem->type == TEXTFIELD)) {
			myItem->aux = myItem->prompt;
			myItem->aux2 = &(myItem->prompt[strlen(myItem->prompt)]);
		} else if (myItem && (myItem->type != LISTBOX)) myItem = nullptr;
	}
	if (myItem) myItem->status = (myItem->status & AREA_PRESSED) + ITEM_PRESSED;
	return myItem;
}

Item *Item_set_next_default(Item *currDefault, Item *itemList) {
	Item *nextDefault;
	if (currDefault) {
		nextDefault = currDefault->next;
		while (nextDefault && (nextDefault->type != LISTBOX) && (nextDefault->type != TEXTFIELD)) {
			nextDefault = nextDefault->next;
		}
		if (!nextDefault) nextDefault = itemList;
		while (nextDefault && (nextDefault->type != LISTBOX) && (nextDefault->type != TEXTFIELD)) {
			nextDefault = nextDefault->next;
		}
	} else {
		nextDefault = itemList;
		while (nextDefault && (nextDefault->type != LISTBOX) && (nextDefault->type != TEXTFIELD)) {
			nextDefault = nextDefault->next;
		}
	}
	if (nextDefault != currDefault) {
		if (currDefault) {
			currDefault->status = ITEM_NORMAL;
		}
		if (nextDefault->type == TEXTFIELD) {
			nextDefault->aux = nextDefault->prompt;
			nextDefault->aux2 = &(nextDefault->prompt[strlen(nextDefault->prompt)]);
		}
		nextDefault->status = ITEM_PRESSED;
	}
	return nextDefault;
}

Item *Item_set_prev_default(Item *currDefault, Item *listBottom) {
	Item *prevDefault;
	if (currDefault) {
		prevDefault = currDefault->prev;
		while (prevDefault && (prevDefault->type != LISTBOX) && (prevDefault->type != TEXTFIELD)) {
			prevDefault = prevDefault->prev;
		}
		if (!prevDefault) prevDefault = listBottom;
		while (prevDefault && (prevDefault->type != LISTBOX) && (prevDefault->type != TEXTFIELD)) {
			prevDefault = prevDefault->prev;
		}
	} else {
		prevDefault = listBottom;
		while (prevDefault && (prevDefault->type != LISTBOX) && (prevDefault->type != TEXTFIELD)) {
			prevDefault = prevDefault->prev;
		}
	}
	if (prevDefault != currDefault) {
		if (currDefault) {
			currDefault->status = ITEM_NORMAL;
		}
		if (prevDefault->type == TEXTFIELD) {
			prevDefault->aux = prevDefault->prompt;
			prevDefault->aux2 = &(prevDefault->prompt[strlen(prevDefault->prompt)]);
		}
		prevDefault->status = ITEM_PRESSED;
	}
	return prevDefault;
}

} // End of namespace M4
