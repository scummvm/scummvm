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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/system.h"
#include "common/keyboard.h"

#include "graphics/primitives.h"
#include "graphics/font.h"
#include "graphics/macgui/macfontmanager.h"
#include "graphics/macgui/macwindowmanager.h"
#include "graphics/macgui/macwindow.h"
#include "graphics/macgui/macmenu.h"

namespace Graphics {

enum {
	kMenuHeight = 20,
	kMenuLeftMargin = 7,
	kMenuSpacing = 13,
	kMenuPadding = 16,
	kMenuDropdownPadding = 14,
	kMenuDropdownItemHeight = 16,
	kMenuItemHeight = 20
};

enum {
	kMenuHighLevel = -1
};

enum {
	kFontStyleBold = 1,
	kFontStyleItalic = 2,
	kFontStyleUnderline = 4,
	kFontStyleOutline = 8,
	kFontStyleShadow = 16,
	kFontStyleCondensed = 32,
	kFontStyleExtended = 64
};

enum {
	kMenuActionCommand
};


struct MacMenuSubItem {
	Common::String text;
	int action;
	int style;
	char shortcut;
	bool enabled;
	Common::Rect bbox;

	MacMenuSubItem(const char *t, int a, int s = 0, char sh = 0, bool e = true) : text(t), action(a), style(s), shortcut(sh), enabled(e) {}
};

typedef Common::Array<MacMenuSubItem *> SubItemArray;

struct MacMenuItem {
	Common::String name;
	SubItemArray subitems;
	Common::Rect bbox;
	Common::Rect subbbox;

	MacMenuItem(const char *n) : name(n) {}
};

MacMenu::MacMenu(int id, const Common::Rect &bounds, MacWindowManager *wm)
		: BaseMacWindow(id, false, wm) {
	_font = getMenuFont();

	_screen.create(bounds.width(), bounds.height(), PixelFormat::createFormatCLUT8());

	_bbox.left = 0;
	_bbox.top = 0;
	_bbox.right = _screen.w;
	_bbox.bottom = kMenuHeight;

	_menuActivated = false;
	_activeItem = -1;
	_activeSubItem = -1;

	_ccallback = NULL;
	_cdata = NULL;

	_tempSurface.create(_screen.w, _font->getFontHeight(), PixelFormat::createFormatCLUT8());
}

MacMenu::~MacMenu() {
	for (uint i = 0; i < _items.size(); i++) {
		for (uint j = 0; j < _items[i]->subitems.size(); j++)
			delete _items[i]->subitems[j];
		delete _items[i];
	}
}

Common::StringArray *MacMenu::readMenuFromResource(Common::SeekableReadStream *res) {
	res->skip(10);
	int enableFlags = res->readUint32BE();
	Common::String menuName = res->readPascalString();
	Common::String menuItem = res->readPascalString();
	int menuItemNumber = 1;
	Common::String menu;
	byte itemData[4];

	while (!menuItem.empty()) {
		if (!menu.empty()) {
			menu += ';';
		}
		if ((enableFlags & (1 << menuItemNumber)) == 0) {
			menu += '(';
		}
		menu += menuItem;
		res->read(itemData, 4);
		static const char styles[] = {'B', 'I', 'U', 'O', 'S', 'C', 'E', 0};
		for (int i = 0; styles[i] != 0; i++) {
			if ((itemData[3] & (1 << i)) != 0) {
				menu += '<';
				menu += styles[i];
			}
		}
		if (itemData[1] != 0) {
			menu += '/';
			menu += (char)itemData[1];
		}
		menuItem = res->readPascalString();
		menuItemNumber++;
	}

	Common::StringArray *result = new Common::StringArray;
	result->push_back(menuName);
	result->push_back(menu);

	debug(4, "menuName: %s", menuName.c_str());
	debug(4, "menu: %s", menu.c_str());

	return result;
}

void MacMenu::addStaticMenus(const MacMenuData *data) {
	MacMenuItem *about = new MacMenuItem(_wm->_fontMan->hasBuiltInFonts() ? "\xa9" : "\xf0"); // (c) Symbol as the most resembling apple
	_items.push_back(about);

	for (int i = 0; data[i].menunum; i++) {
		const MacMenuData *m = &data[i];

		if (m->menunum == kMenuHighLevel) {
			MacMenuItem *item = new MacMenuItem(m->title);
			_items.push_back(item);

			continue;
		}

		_items[m->menunum]->subitems.push_back(new MacMenuSubItem(m->title, m->action, 0, m->shortcut, m->enabled));
	}
}

int MacMenu::addMenuItem(const char *name) {
	MacMenuItem *i = new MacMenuItem(name);
	_items.push_back(i);

	return _items.size() - 1;
}

void MacMenu::addMenuSubItem(int id, const char *text, int action, int style, char shortcut, bool enabled) {
	_items[id]->subitems.push_back(new MacMenuSubItem(text, action, style, shortcut, enabled));

	calcMenuBounds(_items[id]);
}

void MacMenu::calcDimensions() {
	// Calculate menu dimensions
	int y = 1;
	int x = 18;

	for (uint i = 0; i < _items.size(); i++) {
		int w = _font->getStringWidth(_items[i]->name);

		if (_items[i]->bbox.bottom == 0) {
			_items[i]->bbox.left = x - kMenuLeftMargin;
			_items[i]->bbox.top = y;
			_items[i]->bbox.right = x + w + kMenuSpacing - kMenuLeftMargin;
			_items[i]->bbox.bottom = y + _font->getFontHeight() + (_wm->_fontMan->hasBuiltInFonts() ? 3 : 2);
		}

		calcMenuBounds(_items[i]);

		x += w + kMenuSpacing;
	}
}

void MacMenu::clearSubMenu(int id) {
	MacMenuItem *menu = _items[id];

	for (uint j = 0; j < menu->subitems.size(); j++)
		delete menu->subitems[j];

	menu->subitems.clear();
}

void MacMenu::createSubMenuFromString(int id, const char *str) {
	clearSubMenu(id);

	MacMenuItem *menu = _items[id];
	Common::String string(str);

	Common::String item;

	for (uint i = 0; i < string.size(); i++) {
		while(i < string.size() && string[i] != ';') // Read token
			item += string[i++];

		if (item == "(-") {
			menu->subitems.push_back(new MacMenuSubItem(NULL, 0));
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
			if (tmpitem.size() > 0 && tmpitem[0] == '(') {
				enabled = false;

				for (uint j = 0; j < item.size(); j++)
					if (item[j] == '(') {
						item.deleteChar(j);
						break;
					}
			}

			menu->subitems.push_back(new MacMenuSubItem(item.c_str(), kMenuActionCommand, style, shortcut, enabled));
		}

		item.clear();
	}

	calcMenuBounds(menu);
}

const Font *MacMenu::getMenuFont() {
	return _wm->_fontMan->getFont(Graphics::MacFont(kMacFontChicago, 12));
}

const char *MacMenu::getAcceleratorString(MacMenuSubItem *item, const char *prefix) {
	static char res[20];
	*res = 0;

	if (item->shortcut != 0)
		sprintf(res, "%s%c%c", prefix, (_wm->_fontMan->hasBuiltInFonts() ? '^' : '\x11'), item->shortcut);

	return res;
}

int MacMenu::calculateMenuWidth(MacMenuItem *menu) {
	int maxWidth = 0;
	for (uint i = 0; i < menu->subitems.size(); i++) {
		MacMenuSubItem *item = menu->subitems[i];
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

void MacMenu::calcMenuBounds(MacMenuItem *menu) {
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

static void drawPixelPlain(int x, int y, int color, void *data) {
	ManagedSurface *surface = (ManagedSurface *)data;

	if (x >= 0 && x < surface->w && y >= 0 && y < surface->h)
		*((byte *)surface->getBasePtr(x, y)) = (byte)color;
}

static void drawFilledRoundRect(ManagedSurface *surface, Common::Rect &rect, int arc, int color) {
	drawRoundRect(rect, arc, color, true, drawPixelPlain, surface);
}

bool MacMenu::draw(ManagedSurface *g, bool forceRedraw) {
	Common::Rect r(_bbox);

	if (!_contentIsDirty && !forceRedraw)
		return false;

	_contentIsDirty = false;

	_screen.clear(kColorGreen);

	drawFilledRoundRect(&_screen, r, kDesktopArc, kColorWhite);
	r.top = 7;
	_screen.fillRect(r, kColorWhite);
	r.top = kMenuHeight - 1;
	r.bottom++;
	_screen.fillRect(r, kColorGreen);
	r.bottom--;
	_screen.fillRect(r, kColorBlack);

	for (uint i = 0; i < _items.size(); i++) {
		int color = kColorBlack;
		MacMenuItem *it = _items[i];

		if ((uint)_activeItem == i) {
			Common::Rect hbox = it->bbox;

			hbox.left -= 1;
			hbox.right += 3;
			hbox.bottom += 1;

			_screen.fillRect(hbox, kColorBlack);
			color = kColorWhite;

			if (!it->subitems.empty())
				renderSubmenu(it);
		}

		_font->drawString(&_screen, it->name, it->bbox.left + kMenuLeftMargin, it->bbox.top + (_wm->_fontMan->hasBuiltInFonts() ? 2 : 1), it->bbox.width(), color);
	}

	g->transBlitFrom(_screen, kColorGreen);

	g_system->copyRectToScreen(g->getPixels(), g->pitch, 0, 0, g->w, g->h);

	return true;
}

void MacMenu::renderSubmenu(MacMenuItem *menu) {
	Common::Rect *r = &menu->subbbox;

	if (r->width() == 0 || r->height() == 0)
		return;

	_screen.fillRect(*r, kColorWhite);
	_screen.frameRect(*r, kColorBlack);
	_screen.vLine(r->right, r->top + 3, r->bottom + 1, kColorBlack);
	_screen.vLine(r->right + 1, r->top + 3, r->bottom + 1, kColorBlack);
	_screen.hLine(r->left + 3, r->bottom, r->right + 1, kColorBlack);
	_screen.hLine(r->left + 3, r->bottom + 1, r->right + 1, kColorBlack);

	int x = r->left + kMenuDropdownPadding;
	int y = r->top + 1;
	for (uint i = 0; i < menu->subitems.size(); i++) {
		Common::String text(menu->subitems[i]->text);
		Common::String acceleratorText(getAcceleratorString(menu->subitems[i], ""));
		int accelX = r->right - 25;

		int color = kColorBlack;
		if (i == (uint)_activeSubItem && !text.empty() && menu->subitems[i]->enabled) {
			color = kColorWhite;
			Common::Rect trect(r->left, y - (_wm->_fontMan->hasBuiltInFonts() ? 1 : 0), r->right, y + _font->getFontHeight());

			_screen.fillRect(trect, kColorBlack);
		}

		if (!text.empty()) {
			ManagedSurface *s = &_screen;
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
					byte *dst = (byte *)_screen.getBasePtr(x, y+ii);
					byte pat = _wm->getPatterns()[kPatternCheckers2 - 1][ii % 8];
					for (int j = 0; j < r->width(); j++) {
						if (*src != kColorGreen && (pat & (1 << (7 - (x + j) % 8))))
							*dst = *src;
						src++;
						dst++;
					}
				}
			}
		} else { // Delimiter
			bool flip = r->left & 2;
			byte *ptr = (byte *)_screen.getBasePtr(r->left + 1, y + kMenuDropdownItemHeight / 2);
			for (int xx = r->left + 1; xx <= r->right - 1; xx++, ptr++) {
				*ptr = flip ? kColorBlack : kColorWhite;
				flip = !flip;
			}
		}

		y += kMenuDropdownItemHeight;
	}

	_contentIsDirty = true;
	//g_system->copyRectToScreen(_screen.getBasePtr(r->left, r->top), _screen.pitch, r->left, r->top, r->width() + 2, r->height() + 2);
}

bool MacMenu::processEvent(Common::Event &event) {
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

bool MacMenu::keyEvent(Common::Event &event) {
	if (event.type != Common::EVENT_KEYDOWN)
		return false;

	if (event.kbd.flags & (Common::KBD_ALT | Common::KBD_CTRL | Common::KBD_META)) {
		if (event.kbd.ascii >= 0x20 && event.kbd.ascii <= 0x7f) {
			return processMenuShortCut(event.kbd.flags, event.kbd.ascii);
		}
	}

	return false;
}

bool MacMenu::mouseClick(int x, int y) {
	if (_bbox.contains(x, y)) {
		for (uint i = 0; i < _items.size(); i++)
			if (_items[i]->bbox.contains(x, y)) {
			  if ((uint)_activeItem == i)
					return false;

				if (_activeItem != -1) { // Restore background
					Common::Rect r(_items[_activeItem]->subbbox);
					r.right += 3;
					r.bottom += 3;

					_wm->setFullRefresh(true);
				}

				_activeItem = i;
				_activeSubItem = -1;
				_menuActivated = true;

				_contentIsDirty = true;

				return true;
			}
	} else if (_menuActivated && _items[_activeItem]->subbbox.contains(x, y)) {
		MacMenuItem *it = _items[_activeItem];
		int numSubItem = (y - it->subbbox.top) / kMenuDropdownItemHeight;

		if (numSubItem != _activeSubItem) {
			_activeSubItem = numSubItem;

			renderSubmenu(_items[_activeItem]);
			_contentIsDirty = true;
		}
	} else if (_menuActivated && _activeItem != -1) {
		_activeSubItem = -1;

		renderSubmenu(_items[_activeItem]);
		_contentIsDirty = true;
	}

	return false;
}

bool MacMenu::mouseMove(int x, int y) {
	if (_menuActivated)
		if (mouseClick(x, y))
			return true;

	return false;
}

bool MacMenu::mouseRelease(int x, int y) {
	if (_menuActivated) {
		_menuActivated = false;

		if (_activeItem != -1 && _activeSubItem != -1 && _items[_activeItem]->subitems[_activeSubItem]->enabled)
			(*_ccallback)(_items[_activeItem]->subitems[_activeSubItem]->action,
					_items[_activeItem]->subitems[_activeSubItem]->text, _cdata);

		_activeItem = -1;
		_activeSubItem = -1;

		_wm->setFullRefresh(true);

		return true;
	}

	return false;
}

bool MacMenu::processMenuShortCut(byte flags, uint16 ascii) {
	ascii = tolower(ascii);

	if (flags & (Common::KBD_CTRL | Common::KBD_META)) {
		for (uint i = 0; i < _items.size(); i++)
			for (uint j = 0; j < _items[i]->subitems.size(); j++)
				if (_items[i]->subitems[j]->enabled && tolower(_items[i]->subitems[j]->shortcut) == ascii) {
					(*_ccallback)(_items[i]->subitems[j]->action, _items[i]->subitems[j]->text, _cdata);
					return true;
				}
	}

	return false;
}

void MacMenu::enableCommand(int menunum, int action, bool state) {
	for (uint i = 0; i < _items[menunum]->subitems.size(); i++)
		if (_items[menunum]->subitems[i]->action == action)
			_items[menunum]->subitems[i]->enabled = state;

	_contentIsDirty = true;
}

void MacMenu::disableAllMenus() {
	for (uint i = 1; i < _items.size(); i++) // Leave About menu on
		for (uint j = 0; j < _items[i]->subitems.size(); j++)
			_items[i]->subitems[j]->enabled = false;

	_contentIsDirty = true;
}

} // End of namespace Wage
