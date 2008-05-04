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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef KYRA_GUI_V2_H
#define KYRA_GUI_V2_H

#include "kyra/gui.h"

namespace Kyra {

#define GUI_V2_BUTTON(button, a, b, c, d, e, f, h, i, j, k, l, m, n, o, p, q, r, s, t) \
	button.nextButton = 0; \
	button.index = a; \
	button.unk6 = b; \
	button.unk8 = c; \
	button.data0Val1 = d; \
	button.data1Val1 = e; \
	button.data2Val1 = f; \
	button.flags = h; \
	button.data0ShapePtr = button.data1ShapePtr = button.data2ShapePtr = 0; \
	button.dimTableIndex = i; \
	button.x = j; \
	button.y = k; \
	button.width = l; \
	button.height = m; \
	button.data0Val2 = n; \
	button.data0Val3 = o; \
	button.data1Val2 = p; \
	button.data1Val3 = q; \
	button.data2Val2 = r; \
	button.data2Val3 = s; \
	button.flags2 = t;

#define GUI_V2_MENU(menu, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q) \
	menu.x = a; \
	menu.y = b; \
	menu.width = c; \
	menu.height = d; \
	menu.bkgdColor = e; \
	menu.color1 = f; \
	menu.color2 = g; \
	menu.menuNameId = h; \
	menu.textColor = i; \
	menu.titleX = j; \
	menu.titleY = k; \
	menu.highlightedItem = l; \
	menu.numberOfItems = m; \
	menu.scrollUpButtonX = n; \
	menu.scrollUpButtonY = o; \
	menu.scrollDownButtonX = p; \
	menu.scrollDownButtonY = q

#define GUI_V2_MENU_ITEM(item, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q) \
	item.enabled = a; \
	item.itemId = b; \
	item.x = c; \
	item.y = d; \
	item.width = e; \
	item.height = f; \
	item.textColor = g; \
	item.highlightColor = h; \
	item.titleX = i; \
	item.bkgdColor = j; \
	item.color1 = k; \
	item.color2 = l; \
	item.saveSlot = m; \
	item.labelId = n; \
	item.labelX = o; \
	item.labelY = p; \
	item.unk1F = q

class KyraEngine_v2;
class Screen_v2;

class GUI_v2 : public GUI {
public:
	GUI_v2(KyraEngine_v2 *vm);

	Button *addButtonToList(Button *list, Button *newButton);

	void processButton(Button *button);
	int processButtonList(Button *button, uint16 inputFlag);

protected:
	void updateButton(Button *button);

	KyraEngine_v2 *_vm;
	Screen_v2 *_screen;

	bool _buttonListChanged;
	Button *_backUpButtonList;
	Button *_unknownButtonList;
};

} // end of namespace Kyra

#endif

