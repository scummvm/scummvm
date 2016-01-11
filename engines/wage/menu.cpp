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
	int action;
	int style;
	char shortcut;
	bool enabled;
	Common::Rect bbox;

	MenuSubItem(const char *t, int a, int s = 0, char sh = 0, bool e = true) : text(t), action(a), style(s), shortcut(sh), enabled(e) {}
};

struct MenuItem {
	Common::String name;
	Common::Array<MenuSubItem *> subitems;
	Common::Rect bbox;

	MenuItem(const char *n) : name(n) {}
};

static byte fillPattern[8] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

enum {
	kMenuActionAbout,
	kMenuActionNew,
	kMenuActionOpen,
	kMenuActionClose,
	kMenuActionSave,
	kMenuActionSaveAs,
	kMenuActionRevert,
	kMenuActionQuit,

	kMenuActionUndo,
	kMenuActionCut,
	kMenuActionCopy,
	kMenuActionPaste,
	kMenuActionClear
};

struct MenuData {
	int menunum;
	const char *title;
	int action;
	byte shortcut;
} menuSubItems[] = {
	{ 1, "New",			kMenuActionNew, 0 },
	{ 1, "Open...",		kMenuActionOpen, 0 },
	{ 1, "Close",		kMenuActionClose, 0 },
	{ 1, "Save",		kMenuActionSave, 0 },
	{ 1, "Save as...",	kMenuActionSaveAs, 0 },
	{ 1, "Revert",		kMenuActionRevert, 0 },
	{ 1, "Quit",		kMenuActionQuit, 0 },

	{ 2, "Undo",		kMenuActionUndo, 'Z' },
	{ 2, NULL,			0, 0 },
	{ 2, "Cut",			kMenuActionCut, 'K' },
	{ 2, "Copy",		kMenuActionCopy, 'C' },
	{ 2, "Paste",		kMenuActionPaste, 'V' },
	{ 2, "Clear",		kMenuActionClear, 'B' },

	{ 0, NULL,			0, 0 }
};

Menu::Menu(Gui *gui) : _gui(gui) {
	MenuItem *about = new MenuItem(_gui->_builtInFonts ? "\xa9" : "\xf0"); // (c) Symbol as the most resembling apple
	_items.push_back(about);
	_items[0]->subitems.push_back(new MenuSubItem(_gui->_engine->_world->getAboutMenuItemName(), kMenuActionAbout));

	MenuItem *file = new MenuItem("File");
	_items.push_back(file);

	MenuItem *edit = new MenuItem("Edit");
	_items.push_back(edit);

	for (int i = 0; menuSubItems[i].menunum; i++) {
		MenuData *m = &menuSubItems[i];

		_items[m->menunum]->subitems.push_back(new MenuSubItem(m->title, m->action, m->shortcut));
	}

	MenuItem *commands = new MenuItem("Commands");
	_items.push_back(commands);

	assert(_gui->_engine);
	assert(_gui->_engine->_world);

	if (!_gui->_engine->_world->_weaponMenuDisabled) {
		MenuItem *weapons = new MenuItem("Weapons");
		_items.push_back(weapons);
	}

	_bbox.left = 0;
	_bbox.top = 0;
	_bbox.right = _gui->_screen.w - 1;
	_bbox.bottom = kMenuHeight - 1;

	_menuActivated = false;
	_activeItem = -1;
	_activeSubItem = -1;
}

Menu::~Menu() {
	for (int i = 0; i < _items.size(); i++) {
		for (int j = 0; j < _items[i]->subitems.size(); j++)
			delete _items[i]->subitems[j];
		delete _items[i];
	}
}

const Graphics::Font *Menu::getMenuFont() {
	return _gui->getFont("Chicago-12", Graphics::FontManager::kBigGUIFont);
}

void Menu::render() {
	Common::Rect r(_bbox);
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
		int color = kColorBlack;

		if (_activeItem == i) {
			Design::drawFilledRect(&_gui->_screen, _items[i]->bbox, kColorBlack, p, 1);
			color = kColorWhite;
		}

		font->drawString(&_gui->_screen, _items[i]->name, x, y, w, color);

		if (_items[i]->bbox.bottom == 0) {
			_items[i]->bbox.left = x;
			_items[i]->bbox.top = y;
			_items[i]->bbox.right = x + w;
			_items[i]->bbox.bottom = y + font->getFontHeight();
		}

		x += w + 13;
	}

	g_system->copyRectToScreen(_gui->_screen.getPixels(), _gui->_screen.pitch, 0, 0, _gui->_screen.w, kMenuHeight);
}

bool Menu::mouseClick(int x, int y) {
	if (_bbox.contains(x, y)) {
		for (int i = 0; i < _items.size(); i++)
			if (_items[i]->bbox.contains(x, y)) {
				if (_activeItem == i)
					return false;

				_activeItem = i;
				_activeSubItem = -1;
				_menuActivated = true;

				return true;
			}
	}

	return false;
}

bool Menu::mouseMove(int x, int y) {
	if (_menuActivated)
		if (mouseClick(x, y))
			return true;

	return false;
}

bool Menu::mouseRelease(int x, int y) {
	if (_menuActivated) {
		_menuActivated = false;
		_activeItem = -1;

		return true;
	}

	return false;
}

} // End of namespace Wage
