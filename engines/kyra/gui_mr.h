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

#ifndef KYRA_GUI_MR_H
#define KYRA_GUI_MR_H

#include "kyra/gui_v2.h"

namespace Kyra {

class KyraEngine_MR;
class Screen_MR;

class GUI_MR : public GUI_v2 {
friend class KyraEngine_MR;
public:
	GUI_MR(KyraEngine_MR *engine);

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

	uint8 defaultColor1() const { return 0xF0; }
	uint8 defaultColor2() const { return 0xD0; }

	KyraEngine_MR *_vm;
	Screen_MR *_screen;
};

} // end of namespace Kyra

#endif

