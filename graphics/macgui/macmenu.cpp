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
#include "common/macresman.h"

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
	kGrayed = 1,
	kInactive = 2,
	kPopUp = 16,
	kMenuBarBreak = 32,
	kMenuBreak = 64,
	kEndMenu = 128
};

struct MacMenuSubItem {
	Common::String text;
	Common::U32String unicodeText;
	bool unicode;
	int action;
	int style;
	char shortcut;
	bool enabled;
	Common::Rect bbox;

	MacMenuSubItem(const Common::String &t, int a, int s = 0, char sh = 0, bool e = true) : text(t), unicode(false), action(a), style(s), shortcut(sh), enabled(e) {}
	MacMenuSubItem(const Common::U32String &t, int a, int s = 0, char sh = 0, bool e = true) : unicodeText(t), unicode(true), action(a), style(s), shortcut(sh), enabled(e) {}
};

typedef Common::Array<MacMenuSubItem *> SubItemArray;

struct MacMenuItem {
	Common::String name;
	Common::U32String unicodeName;
	bool unicode;
	SubItemArray subitems;
	Common::Rect bbox;
	Common::Rect subbbox;

	MacMenuItem(const Common::String &n) : name(n), unicode(false) {}
	MacMenuItem(const Common::U32String &n) : unicodeName(n), unicode(true) {}
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

	if (_wm->_mode & kWMModeAutohideMenu)
		_isVisible = false;
	else
		_isVisible = true;

	_activeItem = -1;
	_activeSubItem = -1;

	_ccallback = NULL;
	_unicodeccallback = NULL;
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

static Common::U32String readUnicodeString(Common::SeekableReadStream *stream) {
	Common::Array<uint32> strData;
	uint16 wchar;
	while ((wchar = stream->readUint16LE()) != '\0') {
		strData.push_back(wchar);
	}
	return strData.empty() ? Common::U32String() : Common::U32String(strData.data(), strData.size());
}


MacMenu *MacMenu::createMenuFromPEexe(Common::PEResources &exe, MacWindowManager *wm) {
	Common::SeekableReadStream *menuData = exe.getResource(Common::kWinMenu, 128);
	if (!menuData)
		return nullptr;

	menuData->readUint16LE(); // wVersion
	menuData->readUint16LE(); // cbHeaderSize

	MacMenu *menu = wm->addMenu();

	int depth = 0;
	int curMenuItemId = 0;
	int action = 0;
	bool lastPopUp = false;
	bool lastPopUpCopy = false; // no more than 2 level menu for now
	while (depth >= 0) {
		uint16 flags = menuData->readUint16LE();
		if (flags & kPopUp) {
			if (depth == 0) {
				menu->addMenuItem(readUnicodeString(menuData));
			} else {
				// TODO
				// for now skip
				readUnicodeString(menuData);
			}
			if (lastPopUp) {
				lastPopUpCopy = lastPopUp;
			}

			lastPopUp = (flags & kEndMenu) != 0;
			depth++;
		} else {
			menuData->readUint16LE(); // menu id
			Common::U32String name = readUnicodeString(menuData);
			if (depth == 1) {
				menu->addMenuSubItem(curMenuItemId, name, action);
			}
			if (!name.empty()) {
				action++;
			}
			if (flags & kEndMenu) {
				if (lastPopUp)
					depth -= 2;
				else
					depth--;

				if (depth == 0)
					curMenuItemId++;

				lastPopUp = lastPopUpCopy;
				lastPopUpCopy = false;
			}
		}
	}
	delete menuData;
	return menu;
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

	calcDimensions();
}

int MacMenu::addMenuItem(const Common::String &name) {
	MacMenuItem *i = new MacMenuItem(name);
	_items.push_back(i);

	return _items.size() - 1;
}

int MacMenu::addMenuItem(const Common::U32String &name) {
	MacMenuItem *i = new MacMenuItem(name);
	_items.push_back(i);

	return _items.size() - 1;
}

void MacMenu::addMenuSubItem(int id, const Common::String &text, int action, int style, char shortcut, bool enabled) {
	_items[id]->subitems.push_back(new MacMenuSubItem(text, action, style, shortcut, enabled));

	calcMenuBounds(_items[id]);
}

void MacMenu::addMenuSubItem(int id, const Common::U32String &text, int action, int style, char shortcut, bool enabled) {
	_items[id]->subitems.push_back(new MacMenuSubItem(text, action, style, shortcut, enabled));

	calcMenuBounds(_items[id]);
}

void MacMenu::calcDimensions() {
	// Calculate menu dimensions
	int y = 1;
	int x = 18;

	for (uint i = 0; i < _items.size(); i++) {
		int w = _items[i]->unicode ? _font->getStringWidth(_items[i]->unicodeName) : _font->getStringWidth(_items[i]->name);

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

void MacMenu::loadMenuResource(Common::MacResManager *resFork, uint16 id) {
	Common::SeekableReadStream *res = resFork->getResource(MKTAG('M', 'E', 'N', 'U'), id);
	assert(res);

	uint16 menuID = res->readUint16BE();
	/* uint16 width = */ res->readUint16BE();
	/* uint16 height = */ res->readUint16BE();
	/* uint16 resourceID = */ res->readUint16BE();
	/* uint16 placeholder = */ res->readUint16BE();
	uint32 initialState = res->readUint32BE();
	Common::String menuTitle = res->readPascalString();

	if (!menuTitle.empty()) {
		int menu = addMenuItem(menuTitle);
		initialState >>= 1;

		// Read submenu items
		int action = menuID << 16;
		while (true) {
			Common::String subMenuTitle = res->readPascalString();
			if (subMenuTitle.empty())
				break;

			/* uint8 icon = */ res->readByte();
			uint8 key = res->readByte();
			/* uint8 mark = */ res->readByte();
			uint8 style = res->readByte();

			addMenuSubItem(menu, subMenuTitle, action++, style, key, initialState & 1);
			initialState >>= 1;
		}
	}

	delete res;
}

void MacMenu::loadMenuBarResource(Common::MacResManager *resFork, uint16 id) {
	Common::SeekableReadStream *res = resFork->getResource(MKTAG('M', 'B', 'A', 'R'), id);
	assert(res);

	uint16 count = res->readUint16BE();
	for (int i = 0; i < count; i++) {
		loadMenuResource(resFork, res->readUint16BE());
	}
}

void MacMenu::clearSubMenu(int id) {
	MacMenuItem *menu = _items[id];

	for (uint j = 0; j < menu->subitems.size(); j++)
		delete menu->subitems[j];

	menu->subitems.clear();
}

void MacMenu::createSubMenuFromString(int id, const char *str, int commandId) {
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

			menu->subitems.push_back(new MacMenuSubItem(item, commandId, style, shortcut, enabled));
		}

		item.clear();
	}

	calcMenuBounds(menu);
}

const Font *MacMenu::getMenuFont() {
	return _wm->_fontMan->getFont(Graphics::MacFont(kMacFontChicago, 12));
}

const Common::String MacMenu::getAcceleratorString(MacMenuSubItem *item, const char *prefix) {
	if (item->shortcut == 0)
		return Common::String();

	return Common::String::format("%s%c%c", prefix, (_wm->_fontMan->hasBuiltInFonts() ? '^' : '\x11'), item->shortcut);
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
		} else if (!item->unicodeText.empty()) {
			// add accelerator
			int width = _font->getStringWidth(item->unicodeText);
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

	if (!_isVisible)
		return false;

	if (!_contentIsDirty && !forceRedraw)
		return false;

	_contentIsDirty = false;

	_screen.clear(kColorGreen);

	drawFilledRoundRect(&_screen, r, kDesktopArc, _wm->_colorWhite);
	r.top = 7;
	_screen.fillRect(r, _wm->_colorWhite);
	r.top = kMenuHeight - 1;
	r.bottom++;
	_screen.fillRect(r, kColorGreen);
	r.bottom--;
	_screen.fillRect(r, _wm->_colorBlack);

	for (uint i = 0; i < _items.size(); i++) {
		int color = _wm->_colorBlack;
		MacMenuItem *it = _items[i];

		if ((uint)_activeItem == i) {
			Common::Rect hbox = it->bbox;

			hbox.left -= 1;
			hbox.right += 3;
			hbox.bottom += 1;

			_screen.fillRect(hbox, _wm->_colorBlack);
			color = _wm->_colorWhite;

			if (!it->subitems.empty())
				renderSubmenu(it);
		}

		if (it->unicode) {
			_font->drawString(&_screen, it->unicodeName, it->bbox.left + kMenuLeftMargin,
							  it->bbox.top + (_wm->_fontMan->hasBuiltInFonts() ? 2 : 1), it->bbox.width(), color);
		} else {
			_font->drawString(&_screen, it->name, it->bbox.left + kMenuLeftMargin,
							  it->bbox.top + (_wm->_fontMan->hasBuiltInFonts() ? 2 : 1), it->bbox.width(), color);
		}
	}

	g->transBlitFrom(_screen, kColorGreen);

	g_system->copyRectToScreen(g->getPixels(), g->pitch, 0, 0, g->w, g->h);

	return true;
}

void MacMenu::renderSubmenu(MacMenuItem *menu) {
	Common::Rect *r = &menu->subbbox;

	if (r->width() == 0 || r->height() == 0)
		return;

	_screen.fillRect(*r, _wm->_colorWhite);
	_screen.frameRect(*r, _wm->_colorBlack);
	_screen.vLine(r->right, r->top + 3, r->bottom + 1, _wm->_colorBlack);
	_screen.vLine(r->right + 1, r->top + 3, r->bottom + 1, _wm->_colorBlack);
	_screen.hLine(r->left + 3, r->bottom, r->right + 1, _wm->_colorBlack);
	_screen.hLine(r->left + 3, r->bottom + 1, r->right + 1, _wm->_colorBlack);

	int x = r->left + kMenuDropdownPadding;
	int y = r->top + 1;
	for (uint i = 0; i < menu->subitems.size(); i++) {
		Common::String text(menu->subitems[i]->text);
		Common::String acceleratorText(getAcceleratorString(menu->subitems[i], ""));

		Common::U32String unicodeText(menu->subitems[i]->unicodeText);
		// add unicode accelerator

		int accelX = r->right - 25;

		int color = _wm->_colorBlack;
		if (i == (uint)_activeSubItem && (!text.empty() || !unicodeText.empty()) && menu->subitems[i]->enabled) {
			color = _wm->_colorWhite;
			Common::Rect trect(r->left, y - (_wm->_fontMan->hasBuiltInFonts() ? 1 : 0), r->right, y + _font->getFontHeight());

			_screen.fillRect(trect, _wm->_colorBlack);
		}

		if (!text.empty() || !unicodeText.empty()) {
			ManagedSurface *s = &_screen;
			int tx = x, ty = y;

			if (!menu->subitems[i]->enabled) {
				s = &_tempSurface;
				tx = 0;
				ty = 0;
				accelX -= x;

				_tempSurface.clear(kColorGreen);
			}

			if (menu->subitems[i]->unicode)
				_font->drawString(s, unicodeText, tx, ty, r->width(), color);
			else
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
				*ptr = flip ? _wm->_colorBlack : _wm->_colorWhite;
				flip = !flip;
			}
		}

		y += kMenuDropdownItemHeight;
	}

	_contentIsDirty = true;
	//g_system->copyRectToScreen(_screen.getBasePtr(r->left, r->top), _screen.pitch, r->left, r->top, r->width() + 2, r->height() + 2);
}

bool MacMenu::processEvent(Common::Event &event) {
	if (!_isVisible)
		return false;

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
	if (_menuActivated) {
		if (mouseClick(x, y))
			return true;
	} else if ((_wm->_mode & kWMModeAutohideMenu) && !_bbox.contains(x, y)) {
		_isVisible = false;
		if (_wm->_mode & kWMModalMenuMode) {
			_wm->pauseEngine(false);
			*_wm->_screen = *_wm->_screenCopy; // restore screen
			g_system->copyRectToScreen(_wm->_screenCopy->getBasePtr(0, 0), _wm->_screenCopy->pitch, 0, 0, _wm->_screenCopy->w, _wm->_screenCopy->h);
		}
	}

	return false;
}

bool MacMenu::mouseRelease(int x, int y) {
	if (_menuActivated) {
		_menuActivated = false;
		if (_wm->_mode & kWMModeAutohideMenu)
			_isVisible = false;

		if (_wm->_mode & kWMModalMenuMode) {
			_wm->pauseEngine(false);
			*_wm->_screen = *_wm->_screenCopy; // restore screen
			g_system->copyRectToScreen(_wm->_screenCopy->getBasePtr(0, 0), _wm->_screenCopy->pitch, 0, 0, _wm->_screenCopy->w, _wm->_screenCopy->h);
		}

		if (_activeItem != -1 && _activeSubItem != -1 && _items[_activeItem]->subitems[_activeSubItem]->enabled) {
			if (_items[_activeItem]->subitems[_activeSubItem]->unicode) {
				(*_unicodeccallback)(_items[_activeItem]->subitems[_activeSubItem]->action,
							  _items[_activeItem]->subitems[_activeSubItem]->unicodeText, _cdata);
			} else {
				(*_ccallback)(_items[_activeItem]->subitems[_activeSubItem]->action,
							  _items[_activeItem]->subitems[_activeSubItem]->text, _cdata);
			}
		}

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
					if (_items[i]->subitems[j]->unicode) {
						(*_unicodeccallback)(_items[i]->subitems[j]->action, _items[i]->subitems[j]->unicodeText, _cdata);
					} else {
						(*_ccallback)(_items[i]->subitems[j]->action, _items[i]->subitems[j]->text, _cdata);
					}
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

void MacMenu::enableCommand(const char *menuitem, const char *menuaction, bool state) {
	uint menunum = 0;

	while (menunum < _items.size()) {
		assert(!_items[menunum]->unicode);
		if (_items[menunum]->name.equalsIgnoreCase(menuitem))
			break;
		else
			menunum++;
	}
	if (menunum == _items.size())
		return;

	for (uint i = 0; i < _items[menunum]->subitems.size(); i++) {
		assert(!_items[menunum]->subitems[i]->unicode);
		if (_items[menunum]->subitems[i]->text.equalsIgnoreCase(menuaction))
			_items[menunum]->subitems[i]->enabled = state;
	}

	_contentIsDirty = true;
}

void MacMenu::enableCommand(const Common::U32String &menuitem, const Common::U32String &menuaction, bool state) {
	uint menunum = 0;

	while (menunum < _items.size()) {
		assert(_items[menunum]->unicode);
		if (_items[menunum]->unicodeName.equals(menuitem))
			break;
		else
			menunum++;
	}
	if (menunum == _items.size())
		return;

	for (uint i = 0; i < _items[menunum]->subitems.size(); i++) {
		assert(_items[menunum]->subitems[i]->unicode);
		if (_items[menunum]->subitems[i]->unicodeText.equals(menuaction))
			_items[menunum]->subitems[i]->enabled = state;
	}

	_contentIsDirty = true;
}

void MacMenu::disableAllMenus() {
	for (uint i = 1; i < _items.size(); i++) // Leave About menu on
		for (uint j = 0; j < _items[i]->subitems.size(); j++)
			_items[i]->subitems[j]->enabled = false;

	_contentIsDirty = true;
}

} // End of namespace Wage
