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

typedef Common::Array<MenuSubItem *> SubItemArray;

struct MenuItem {
	Common::String name;
	SubItemArray subitems;
	Common::Rect bbox;
	Common::Rect subbbox;

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
	_patterns.push_back(fillPattern);

	MenuItem *about = new MenuItem(_gui->_builtInFonts ? "\xa9" : "\xf0"); // (c) Symbol as the most resembling apple
	_items.push_back(about);
	_items[0]->subitems.push_back(new MenuSubItem(_gui->_engine->_world->getAboutMenuItemName(), kMenuActionAbout));

	MenuItem *file = new MenuItem("File");
	_items.push_back(file);

	MenuItem *edit = new MenuItem("Edit");
	_items.push_back(edit);

	for (int i = 0; menuSubItems[i].menunum; i++) {
		MenuData *m = &menuSubItems[i];

		_items[m->menunum]->subitems.push_back(new MenuSubItem(m->title, m->action, 0, m->shortcut));
	}

	MenuItem *commands = new MenuItem("Commands");
	_items.push_back(commands);

	assert(_gui->_engine);
	assert(_gui->_engine->_world);

	if (!_gui->_engine->_world->_weaponMenuDisabled) {
		MenuItem *weapons = new MenuItem("Weapons");
		_items.push_back(weapons);
	}

	// Calculate menu dimensions
	_font = getMenuFont();
	int y = _gui->_builtInFonts ? 3 : 2;
	int x = 18;

	for (int i = 0; i < _items.size(); i++) {
		int w = _font->getStringWidth(_items[i]->name);

		if (_items[i]->bbox.bottom == 0) {
			_items[i]->bbox.left = x - kMenuLeftMargin;
			_items[i]->bbox.top = y;
			_items[i]->bbox.right = x + w + kMenuSpacing - kMenuLeftMargin;
			_items[i]->bbox.bottom = y + _font->getFontHeight();
		}

		calcMenuBounds(_items[i]);

		x += w + kMenuSpacing;
	}

	_bbox.left = 0;
	_bbox.top = 0;
	_bbox.right = _gui->_screen.w - 1;
	_bbox.bottom = kMenuHeight - 1;

	_menuActivated = false;
	_activeItem = -1;
	_activeSubItem = -1;

	_screenCopy.create(_gui->_screen.w, _gui->_screen.h, Graphics::PixelFormat::createFormatCLUT8());
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

const char *Menu::getAcceleratorString(MenuSubItem *item) {
	static char res[20];
	*res = 0;

	if (item->shortcut != 0)
		sprintf(res, "      \x11%c", item->shortcut);

	return res;
}

int Menu::calculateMenuWidth(MenuItem *menu) {
	int maxWidth = 0;
	for (int i = 0; i < menu->subitems.size(); i++) {
		MenuSubItem *item = menu->subitems[i];
		if (item->text.size()) {
			Common::String text(item->text);
			Common::String acceleratorText(getAcceleratorString(item));
			if (acceleratorText.size()) {
				text += acceleratorText;
			}

			int width = _font->getStringWidth(text);
			if (width > maxWidth) {
				maxWidth = width;
			}
		}
	}
	return maxWidth;
}

void Menu::calcMenuBounds(MenuItem *menu) {
	// TODO: cache maxWidth
	int maxWidth = calculateMenuWidth(menu);
	int x1 = menu->bbox.left;
	int y1 = menu->bbox.bottom;
	int x2 = x1 + maxWidth + kMenuDropdownPadding * 2;
	int y2 = y1 + menu->subitems.size() * kMenuDropdownItemHeight - 3;

	menu->subbbox.left = x1;
	menu->subbbox.top = y1;
	menu->subbbox.right = x2;
	menu->subbbox.bottom = y2;
}

void Menu::render() {
	Common::Rect r(_bbox);

	Design::drawFilledRoundRect(&_gui->_screen, r, kDesktopArc, kColorWhite, _patterns, 1);
	r.top = 7;
	Design::drawFilledRect(&_gui->_screen, r, kColorWhite, _patterns, 1);
	r.top = kMenuHeight - 1;
	Design::drawFilledRect(&_gui->_screen, r, kColorBlack, _patterns, 1);

	for (int i = 0; i < _items.size(); i++) {
		int color = kColorBlack;
		MenuItem *it = _items[i];

		if (_activeItem == i) {
			Design::drawFilledRect(&_gui->_screen, it->bbox, kColorBlack, _patterns, 1);
			color = kColorWhite;

			if (it->subitems.size())
				renderSubmenu(it);
		}

		_font->drawString(&_gui->_screen, it->name, it->bbox.left + kMenuLeftMargin, it->bbox.top, it->bbox.width(), color);
	}

	g_system->copyRectToScreen(_gui->_screen.getPixels(), _gui->_screen.pitch, 0, 0, _gui->_screen.w, kMenuHeight);
}

void Menu::renderSubmenu(MenuItem *menu) {
	Common::Rect *r = &menu->subbbox;

	if (r->width() == 0 || r->height() == 0)
		return;

	Design::drawFilledRect(&_gui->_screen, *r, kColorWhite, _patterns, 1);
	Design::drawRect(&_gui->_screen, *r, 1, kColorBlack, _patterns, 1);
	Design::drawVLine(&_gui->_screen, r->right + 1, r->top + 2, r->bottom + 2, 1, kColorBlack, _patterns, 1);
	Design::drawVLine(&_gui->_screen, r->right + 2, r->top + 2, r->bottom + 2, 1, kColorBlack, _patterns, 1);
	Design::drawHLine(&_gui->_screen, r->left + 3, r->right + 2, r->bottom + 1, 1, kColorBlack, _patterns, 1);
	Design::drawHLine(&_gui->_screen, r->left + 3, r->right + 2, r->bottom + 2, 1, kColorBlack, _patterns, 1);

	int x = r->left + kMenuDropdownPadding;
	int y = r->top;
	for (int i = 0; i < menu->subitems.size(); i++) {
		Common::String text(menu->subitems[i]->text);
		Common::String acceleratorText(getAcceleratorString(menu->subitems[i]));
		if (acceleratorText.size()) {
			text += acceleratorText;
		}

		int color = kColorBlack;
		if (i == _activeSubItem && menu->subitems[_activeSubItem]->text.size()) {
			color = kColorWhite;
			Common::Rect trect(r->left, y, r->right, y + kMenuDropdownItemHeight);

			Design::drawFilledRect(&_gui->_screen, trect, kColorBlack, _patterns, 1);
		}
		_font->drawString(&_gui->_screen, text, x, y, r->width(), color);

		y += kMenuDropdownItemHeight;
	}

	g_system->copyRectToScreen(_gui->_screen.getBasePtr(r->left, r->top), _gui->_screen.pitch, r->left, r->top, r->width() + 3, r->height() + 3);
}

bool Menu::mouseClick(int x, int y) {
	if (_bbox.contains(x, y)) {
		if (!_menuActivated)
			_screenCopy.copyFrom(_gui->_screen);

		for (int i = 0; i < _items.size(); i++)
			if (_items[i]->bbox.contains(x, y)) {
				if (_activeItem == i)
					return false;

				if (_activeItem != -1) { // Restore background
					Common::Rect r(_items[_activeItem]->subbbox);
					r.right += 3;
					r.bottom += 3;

					_gui->_screen.copyRectToSurface(_screenCopy, r.left, r.top, r);
					g_system->copyRectToScreen(_gui->_screen.getBasePtr(r.left, r.top), _gui->_screen.pitch, r.left, r.top, r.width() + 1, r.height() + 1);
				}

				_activeItem = i;
				_activeSubItem = -1;
				_menuActivated = true;

				return true;
			}
	} else if (_menuActivated && _items[_activeItem]->subbbox.contains(x, y)) {
		MenuItem *it = _items[_activeItem];
		int numSubItem = (y - it->subbbox.top) / kMenuDropdownItemHeight;

		if (numSubItem != _activeSubItem) {
			_activeSubItem = numSubItem;

			renderSubmenu(_items[_activeItem]);
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
