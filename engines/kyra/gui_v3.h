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

#ifndef KYRA_GUI_V3_H
#define KYRA_GUI_V3_H

#include "kyra/gui.h"

namespace Kyra {

#define GUI_V3_BUTTON(button, a, b, c, d, e, f, h, i, j, k, l, m, n, o, p, q, r, s, t) \
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

class KyraEngine_v3;
class Screen_v3;

class GUI_v3 : public GUI {
friend class KyraEngine_v3;
public:
	GUI_v3(KyraEngine_v3 *engine);

	Button *addButtonToList(Button *list, Button *newButton);

	void processButton(Button *button);
	int processButtonList(Button *button, uint16 inputFlag);

	void flagButtonEnable(Button *button);
	void flagButtonDisable(Button *button);
private:
	const char *getMenuTitle(const Menu &menu) { return 0; }
	const char *getMenuItemTitle(const MenuItem &menuItem) { return 0; }
	const char *getMenuItemLabel(const MenuItem &menuItem) { return 0; }

	Button *getButtonListData() { return 0; }

	Button *getScrollUpButton() { return 0; }
	Button *getScrollDownButton() { return 0; }

	Button::Callback getScrollUpButtonHandler() const { return Button::Callback(); }
	Button::Callback getScrollDownButtonHandler() const { return Button::Callback(); }

	uint8 defaultColor1() const { return 0xCF; }
	uint8 defaultColor2() const { return 0xF8; }

	KyraEngine_v3 *_vm;
	Screen_v3 *_screen;

	bool _buttonListChanged;
	Button *_backUpButtonList;
	Button *_unknownButtonList;
};

} // end of namespace Kyra

#endif

