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
#include "common/keyboard.h"

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

struct MenuData {
	int menunum;
	const char *title;
	int action;
	byte shortcut;
	bool enabled;
} static const menuSubItems[] = {
	{ kMenuFile, "New",			kMenuActionNew, 0, false },
	{ kMenuFile, "Open...",		kMenuActionOpen, 0, false },
	{ kMenuFile, "Close",		kMenuActionClose, 0, true },
	{ kMenuFile, "Save",		kMenuActionSave, 0, false },
	{ kMenuFile, "Save as...",	kMenuActionSaveAs, 0, true },
	{ kMenuFile, "Revert",		kMenuActionRevert, 0, false },
	{ kMenuFile, "Quit",		kMenuActionQuit, 0, true },

	{ kMenuEdit, "Undo",		kMenuActionUndo, 'Z', false },
	{ kMenuEdit, NULL,			0, 0, false },
	{ kMenuEdit, "Cut",			kMenuActionCut, 'K', false },
	{ kMenuEdit, "Copy",		kMenuActionCopy, 'C', false },
	{ kMenuEdit, "Paste",		kMenuActionPaste, 'V', false },
	{ kMenuEdit, "Clear",		kMenuActionClear, 'B', false },

	{ 0, NULL,			0, 0, false }
};

Menu::Menu(int id, Gui *gui) : BaseMacWindow(id), _gui(gui) {
	_font = getMenuFont();

	MenuItem *about = new MenuItem(_gui->_builtInFonts ? "\xa9" : "\xf0"); // (c) Symbol as the most resembling apple
	_items.push_back(about);
	_items[0]->subitems.push_back(new MenuSubItem(_gui->_engine->_world->getAboutMenuItemName(), kMenuActionAbout));

	MenuItem *file = new MenuItem("File");
	_items.push_back(file);

	MenuItem *edit = new MenuItem("Edit");
	_items.push_back(edit);

	for (int i = 0; menuSubItems[i].menunum; i++) {
		const MenuData *m = &menuSubItems[i];

		_items[m->menunum]->subitems.push_back(new MenuSubItem(m->title, m->action, 0, m->shortcut, m->enabled));
	}

	_commands = new MenuItem(_gui->_engine->_world->_commandsMenuName.c_str());
	_items.push_back(_commands);
	regenCommandsMenu();

	_weapons = NULL;

	if (!_gui->_engine->_world->_weaponMenuDisabled) {
		_weapons = new MenuItem(_gui->_engine->_world->_weaponsMenuName.c_str());
		_items.push_back(_weapons);

		regenWeaponsMenu();
	}

	// Calculate menu dimensions
	int y = 1;
	int x = 18;

	for (uint i = 0; i < _items.size(); i++) {
		int w = _font->getStringWidth(_items[i]->name);

		if (_items[i]->bbox.bottom == 0) {
			_items[i]->bbox.left = x - kMenuLeftMargin;
			_items[i]->bbox.top = y;
			_items[i]->bbox.right = x + w + kMenuSpacing - kMenuLeftMargin;
			_items[i]->bbox.bottom = y + _font->getFontHeight() + (_gui->_builtInFonts ? 3 : 2);
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
	_tempSurface.create(_gui->_screen.w, _font->getFontHeight(), Graphics::PixelFormat::createFormatCLUT8());
}

Menu::~Menu() {
	for (uint i = 0; i < _items.size(); i++) {
		for (uint j = 0; j < _items[i]->subitems.size(); j++)
			delete _items[i]->subitems[j];
		delete _items[i];
	}
}

void Menu::regenCommandsMenu() {
	for (uint j = 0; j < _commands->subitems.size(); j++)
		delete _commands->subitems[j];

	_commands->subitems.clear();

	createCommandsMenu(_commands);
	calcMenuBounds(_commands);
}

void Menu::createCommandsMenu(MenuItem *menu) {
	Common::String string(_gui->_engine->_world->_commandsMenu);

	Common::String item;

	for (uint i = 0; i < string.size(); i++) {
		while(i < string.size() && string[i] != ';') // Read token
			item += string[i++];

		if (item == "(-") {
			menu->subitems.push_back(new MenuSubItem(NULL, 0));
		} else {
			bool enabled = true;
			int style = 0;
			char shortcut = 0;
			const char *shortPtr = strrchr(item.c_str(), '/');
			if (shortPtr != NULL) {
				if (strlen(shortPtr) >= 2) {
					shortcut = shortPtr[1];
					item.deleteChar(shortPtr - item.c_str());
					item.deleteChar(shortPtr - item.c_str());
				} else {
					error("Unexpected shortcut: '%s', item '%s' in menu '%s'", shortPtr, item.c_str(), string.c_str());
				}
			}

			while (item.size() >= 2 && item[item.size() - 2] == '<') {
				char c = item.lastChar();
				if (c == 'B') {
					style |= kFontStyleBold;
				} else if (c == 'I') {
					style |= kFontStyleItalic;
				} else if (c == 'U') {
					style |= kFontStyleUnderline;
				} else if (c == 'O') {
					style |= kFontStyleOutline;
				} else if (c == 'S') {
					style |= kFontStyleShadow;
				} else if (c == 'C') {
					style |= kFontStyleCondensed;
				} else if (c == 'E') {
					style |= kFontStyleExtended;
				}
				item.deleteLastChar();
				item.deleteLastChar();
			}

			Common::String tmpitem(item);
			tmpitem.trim();
			if (tmpitem[0] == '(') {
				enabled = false;

				for (uint j = 0; j < item.size(); j++)
					if (item[j] == '(') {
						item.deleteChar(j);
						break;
					}
			}

			menu->subitems.push_back(new MenuSubItem(item.c_str(), kMenuActionCommand, style, shortcut, enabled));
		}

		item.clear();
	}
}

void Menu::regenWeaponsMenu() {
	if (_gui->_engine->_world->_weaponMenuDisabled)
		return;

	for (uint j = 0; j < _weapons->subitems.size(); j++)
		delete _weapons->subitems[j];

	_weapons->subitems.clear();

	createWeaponsMenu(_weapons);
	calcMenuBounds(_weapons);
}

void Menu::createWeaponsMenu(MenuItem *menu) {
	Chr *player = _gui->_engine->_world->_player;
	ObjArray *weapons = player->getWeapons(true);

	for (uint i = 0; i < weapons->size(); i++) {
		Obj *obj = (*weapons)[i];
		if (obj->_type == Obj::REGULAR_WEAPON ||
			obj->_type == Obj::THROW_WEAPON ||
			obj->_type == Obj::MAGICAL_OBJECT) {
			Common::String command(obj->_operativeVerb);
			command += " ";
			command += obj->_name;

			menu->subitems.push_back(new MenuSubItem(command.c_str(), kMenuActionCommand, 0, 0, true));
		}
	}
	delete weapons;

	if (menu->subitems.empty())
		menu->subitems.push_back(new MenuSubItem("You have no weapons", 0, 0, 0, false));
}

const Graphics::Font *Menu::getMenuFont() {
	return _gui->getFont("Chicago-12", Graphics::FontManager::kBigGUIFont);
}

const char *Menu::getAcceleratorString(MenuSubItem *item, const char *prefix) {
	static char res[20];
	*res = 0;

	if (item->shortcut != 0)
		sprintf(res, "%s%c%c", prefix, (_gui->_builtInFonts ? '^' : '\x11'), item->shortcut);

	return res;
}

int Menu::calculateMenuWidth(MenuItem *menu) {
	int maxWidth = 0;
	for (uint i = 0; i < menu->subitems.size(); i++) {
		MenuSubItem *item = menu->subitems[i];
		if (!item->text.empty()) {
			Common::String text(item->text);
			Common::String acceleratorText(getAcceleratorString(item, "  "));
			if (!acceleratorText.empty()) {
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
	int x1 = menu->bbox.left - 1;
	int y1 = menu->bbox.bottom + 1;
	int x2 = x1 + maxWidth + kMenuDropdownPadding * 2 - 4;
	int y2 = y1 + menu->subitems.size() * kMenuDropdownItemHeight + 2;

	menu->subbbox.left = x1;
	menu->subbbox.top = y1;
	menu->subbbox.right = x2;
	menu->subbbox.bottom = y2;
}

bool Menu::draw(Graphics::ManagedSurface *g, bool forceRedraw) {
	Common::Rect r(_bbox);

	if (!_contentIsDirty)
		return false;

	_contentIsDirty = true;

	Design::drawFilledRoundRect(&_gui->_screen, r, kDesktopArc, kColorWhite, _gui->_patterns, kPatternSolid);
	r.top = 7;
	Design::drawFilledRect(&_gui->_screen, r, kColorWhite, _gui->_patterns, kPatternSolid);
	r.top = kMenuHeight - 1;
	Design::drawFilledRect(&_gui->_screen, r, kColorBlack, _gui->_patterns, kPatternSolid);

	for (uint i = 0; i < _items.size(); i++) {
		int color = kColorBlack;
		MenuItem *it = _items[i];

		if ((uint)_activeItem == i) {
			Common::Rect hbox = it->bbox;

			hbox.left -= 1;
			hbox.right += 2;

			Design::drawFilledRect(&_gui->_screen, hbox, kColorBlack, _gui->_patterns, kPatternSolid);
			color = kColorWhite;

			if (!it->subitems.empty())
				renderSubmenu(it);
		}

		_font->drawString(&_gui->_screen, it->name, it->bbox.left + kMenuLeftMargin, it->bbox.top + (_gui->_builtInFonts ? 2 : 1), it->bbox.width(), color);
	}

	g_system->copyRectToScreen(_gui->_screen.getPixels(), _gui->_screen.pitch, 0, 0, _gui->_screen.w, kMenuHeight);

	return true;
}

void Menu::renderSubmenu(MenuItem *menu) {
	Common::Rect *r = &menu->subbbox;

	if (r->width() == 0 || r->height() == 0)
		return;

	Design::drawFilledRect(&_gui->_screen, *r, kColorWhite, _gui->_patterns, kPatternSolid);
	Design::drawRect(&_gui->_screen, *r, 1, kColorBlack, _gui->_patterns, kPatternSolid);
	Design::drawVLine(&_gui->_screen, r->right + 1, r->top + 3, r->bottom + 1, 1, kColorBlack, _gui->_patterns, kPatternSolid);
	Design::drawHLine(&_gui->_screen, r->left + 3, r->right + 1, r->bottom + 1, 1, kColorBlack, _gui->_patterns, kPatternSolid);

	int x = r->left + kMenuDropdownPadding;
	int y = r->top + 1;
	for (uint i = 0; i < menu->subitems.size(); i++) {
		Common::String text(menu->subitems[i]->text);
		Common::String acceleratorText(getAcceleratorString(menu->subitems[i], ""));
		int accelX = r->right - 25;

		int color = kColorBlack;
		if (i == (uint)_activeSubItem && !text.empty() && menu->subitems[i]->enabled) {
			color = kColorWhite;
			Common::Rect trect(r->left, y - (_gui->_builtInFonts ? 1 : 0), r->right, y + _font->getFontHeight());

			Design::drawFilledRect(&_gui->_screen, trect, kColorBlack, _gui->_patterns, kPatternSolid);
		}

		if (!text.empty()) {
			Graphics::ManagedSurface *s = &_gui->_screen;
			int tx = x, ty = y;

			if (!menu->subitems[i]->enabled) {
				s = &_tempSurface;
				tx = 0;
				ty = 0;
				accelX -= x;

				_tempSurface.clear(kColorGreen);
			}

			_font->drawString(s, text, tx, ty, r->width(), color);

			if (!acceleratorText.empty())
				_font->drawString(s, acceleratorText, accelX, ty, r->width(), color);

			if (!menu->subitems[i]->enabled) {
				// I am lazy to extend drawString() with plotProc as a parameter, so
				// fake it here
				for (int ii = 0; ii < _tempSurface.h; ii++) {
					const byte *src = (const byte *)_tempSurface.getBasePtr(0, ii);
					byte *dst = (byte *)_gui->_screen.getBasePtr(x, y+ii);
					byte pat = _gui->_patterns[kPatternCheckers2 - 1][ii % 8];
					for (int j = 0; j < r->width(); j++) {
						if (*src != kColorGreen && (pat & (1 << (7 - (x + j) % 8))))
							*dst = *src;
						src++;
						dst++;
					}
				}
			}
		} else { // Delimiter
			Design::drawHLine(&_gui->_screen, r->left + 1, r->right - 1, y + kMenuDropdownItemHeight / 2, 1, kColorBlack, _gui->_patterns, kPatternStripes);
		}

		y += kMenuDropdownItemHeight;
	}

	g_system->copyRectToScreen(_gui->_screen.getBasePtr(r->left, r->top), _gui->_screen.pitch, r->left, r->top, r->width() + 3, r->height() + 3);
}

bool Menu::processEvent(Common::Event &event) {
	switch (event.type) {
	case Common::EVENT_KEYDOWN:
		return keyEvent(event);
	case Common::EVENT_LBUTTONDOWN:
		return mouseClick(event.mouse.x, event.mouse.y);
	case Common::EVENT_LBUTTONUP:
		return mouseRelease(event.mouse.x, event.mouse.y);
	case Common::EVENT_MOUSEMOVE:
		return mouseMove(event.mouse.x, event.mouse.y);
	default:
		return false;
	}
}

bool Menu::keyEvent(Common::Event &event) {
	if (event.type != Common::EVENT_KEYDOWN)
		return false;

	if (event.kbd.flags & (Common::KBD_ALT | Common::KBD_CTRL | Common::KBD_META)) {
		if (event.kbd.ascii >= 0x20 && event.kbd.ascii <= 0x7f) {
			return processMenuShortCut(event.kbd.flags, event.kbd.ascii);
		}
	}

	return false;
}

bool Menu::mouseClick(int x, int y) {
	if (_bbox.contains(x, y)) {
		if (!_menuActivated)
			_screenCopy.copyFrom(_gui->_screen);

		for (uint i = 0; i < _items.size(); i++)
			if (_items[i]->bbox.contains(x, y)) {
			  if ((uint)_activeItem == i)
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
	} else if (_menuActivated && _activeItem != -1) {
		_activeSubItem = -1;

		renderSubmenu(_items[_activeItem]);
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

		if (_activeItem != -1 && _activeSubItem != -1 && _items[_activeItem]->subitems[_activeSubItem]->enabled)
			executeCommand(_items[_activeItem]->subitems[_activeSubItem]);

		_activeItem = -1;
		_activeSubItem = -1;

		_gui->_wm.setFullRefresh(true);

		return true;
	}

	return false;
}

void Menu::executeCommand(MenuSubItem *subitem) {
	switch(subitem->action) {
	case kMenuActionAbout:
	case kMenuActionNew:
	case kMenuActionOpen:
	case kMenuActionClose:
	case kMenuActionSave:
	case kMenuActionSaveAs:
	case kMenuActionRevert:
	case kMenuActionQuit:

	case kMenuActionUndo:
		_gui->actionUndo();
		break;
	case kMenuActionCut:
		_gui->actionCut();
		break;
	case kMenuActionCopy:
		_gui->actionCopy();
		break;
	case kMenuActionPaste:
		_gui->actionPaste();
		break;
	case kMenuActionClear:
		_gui->actionClear();
		break;

	case kMenuActionCommand:
		_gui->_engine->processTurn(&subitem->text, NULL);
		break;

	default:
		warning("Unknown action: %d", subitem->action);

	}
}

bool Menu::processMenuShortCut(byte flags, uint16 ascii) {
	ascii = tolower(ascii);

	if (flags & (Common::KBD_CTRL | Common::KBD_META)) {
		for (uint i = 0; i < _items.size(); i++)
			for (uint j = 0; j < _items[i]->subitems.size(); j++)
				if (_items[i]->subitems[j]->enabled && tolower(_items[i]->subitems[j]->shortcut) == ascii) {
					executeCommand(_items[i]->subitems[j]);
					return true;
				}
	}

	return false;
}

void Menu::enableCommand(int menunum, int action, bool state) {
	for (uint i = 0; i < _items[menunum]->subitems.size(); i++)
		if (_items[menunum]->subitems[i]->action == action)
			_items[menunum]->subitems[i]->enabled = state;

	_contentIsDirty = true;
}

void Menu::disableAllMenus() {
	for (uint i = 1; i < _items.size(); i++) // Leave About menu on
		for (uint j = 0; j < _items[i]->subitems.size(); j++)
			_items[i]->subitems[j]->enabled = false;

	_contentIsDirty = true;
}

} // End of namespace Wage
