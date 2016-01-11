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
 * MIT License:
 *
 * Copyright (c) 2009 Alexei Svitkine, Eugene Sandulenko
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "common/system.h"

#include "wage/wage.h"
#include "wage/entities.h"
#include "wage/design.h"
#include "wage/gui.h"
#include "wage/menu.h"
#include "wage/world.h"

namespace Wage {

struct MenuSubItem {
	Common::String text;
	int style;
	char shortcut;
	bool enabled;

	MenuSubItem(Common::String &t, int s, char sh) : text(t), style(s), shortcut(sh), enabled(true) {}
};

struct MenuItem {
	Common::String name;
	Common::Array<MenuSubItem *> subitems;

	MenuItem(const char *n) : name(n) {}
};

static byte fillPattern[8] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

Menu::Menu(Gui *gui) : _gui(gui) {
	MenuItem *about = new MenuItem(_gui->_builtInFonts ? "\xa9" : "\xf0"); // (c) Symbol as the most resembling apple
	_items.push_back(about);

	MenuItem *file = new MenuItem("File");
	_items.push_back(file);

	MenuItem *edit = new MenuItem("Edit");
	_items.push_back(edit);

	MenuItem *commands = new MenuItem("Commands");
	_items.push_back(commands);

	if (!_gui->_engine->_world->_weaponMenuDisabled) {
		MenuItem *weapons = new MenuItem("Weapons");
		_items.push_back(weapons);
	}
}

const Graphics::Font *Menu::getMenuFont() {
	return _gui->getFont("Chicago-12", Graphics::FontManager::kBigGUIFont);
}

void Menu::render() {
	Common::Rect r(0, 0, _gui->_screen.w - 1, kMenuHeight - 1);
	Patterns p;
	p.push_back(fillPattern);

	Design::drawFilledRoundRect(&_gui->_screen, r, kDesktopArc, kColorWhite, p, 1);
	r.top = 7;
	Design::drawFilledRect(&_gui->_screen, r, kColorWhite, p, 1);
	r.top = kMenuHeight - 1;
	Design::drawFilledRect(&_gui->_screen, r, kColorBlack, p, 1);

	const Graphics::Font *font = getMenuFont();
	int y = _gui->_builtInFonts ? 3 : 2;
	int x = 18;

	for (int i = 0; i < _items.size(); i++) {
		int w = font->getStringWidth(_items[i]->name);
		font->drawString(&_gui->_screen, _items[i]->name, x, y, w, kColorBlack);

		x += w + 13;
	}

	g_system->copyRectToScreen(_gui->_screen.getPixels(), _gui->_screen.pitch, 0, 0, _gui->_screen.w, kMenuHeight);
}

} // End of namespace Wage
