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

#include "common/system.h"
#include "common/stack.h"
#include "common/keyboard.h"
#include "common/macresman.h"
#include "common/formats/winexe_pe.h"
#include "common/unicode-bidi.h"
#include "common/timer.h"

#include "graphics/primitives.h"
#include "graphics/font.h"
#include "graphics/fonts/ttf.h"
#include "graphics/macgui/macfontmanager.h"
#include "graphics/macgui/macwindowmanager.h"
#include "graphics/macgui/macwindow.h"
#include "graphics/macgui/macmenu.h"

#define SCROLL_DELAY 100000

namespace Graphics {

enum {
	kMenuHeight = 20,
	kMenuLeftMargin = 7,
	kMenuSpacing = 13,
	kMenuPadding = 16,
	kMenuDropdownPadding = 14,
	kMenuDropdownItemHeight = 16,
	kMenuItemHeight = 20,
	kMenuWin95LeftDropdownPadding = 34,
	kMenuWin95RightDropdownPadding = 57,
	kMenuWin95DropdownItemHeight = 20
};

enum {
	kMenuHighLevel = -1
};

enum {
	kGrayed = 1,
	kInactive = 2,
	kPopUp = 16,
	kMenuBarBreak = 32,
	kMenuBreak = 64,
	kEndMenu = 128
};

struct MacMenuSubMenu;

MacMenuSubMenu::~MacMenuSubMenu() {
	for (uint i = 0; i < items.size(); i++)
		delete items[i];
}

void MacMenuSubMenu::enableAllItems() {
	for (uint i = 0; i < items.size(); ++i) {
		items[i]->enabled = true;
		if (items[i]->submenu) {
			items[i]->submenu->enableAllItems();
		}
	}
}

MacMenu::MacMenu(int id, const Common::Rect &bounds, MacWindowManager *wm)
		: BaseMacWindow(id, false, wm) {
	_loadedFont = NULL;

	_font = getMenuFont();

	_align = kTextAlignLeft;

	_type = MacWindowConstants::kWindowMenu;

	_screen.create(bounds.width(), bounds.height(), _wm->_pixelformat);

	_bbox.left = 0;
	_bbox.top = 0;
	_bbox.right = _screen.w;
	_bbox.bottom = kMenuHeight;

	_dimensionsDirty = true;

	if (_wm->_mode & kWMModeWin95 && !(_wm->_mode & kWMModeForceMacFontsInWin95)) {
		_menuDropdownItemHeight = kMenuWin95DropdownItemHeight;
		_menuLeftDropdownPadding = kMenuWin95LeftDropdownPadding;
		_menuRightDropdownPadding = kMenuWin95RightDropdownPadding;
	} else {
		_menuDropdownItemHeight = kMenuDropdownItemHeight;
		_menuLeftDropdownPadding = kMenuDropdownPadding;
		_menuRightDropdownPadding = kMenuDropdownPadding;
	}

	if (_wm->_mode & kWMModeAutohideMenu)
		_isVisible = false;
	else
		_isVisible = true;

	_activeItem = -1;
	_activeSubItem = -1;
	_lastActiveItem = -1;
	_lastActiveSubItem = -1;

	_ccallback = NULL;
	_unicodeccallback = NULL;
	_cdata = NULL;

	_scrollTimerActive = false;
	_scrollDirection = 0;

	_isModal = false;

	_tempSurface.create(_screen.w, _font->getFontHeight(), _wm->_pixelformat);
}

MacMenu::~MacMenu() {
	for (uint i = 0; i < _items.size(); i++)
		delete _items[i];

	delete _loadedFont;
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
	Common::U32String strData;
	uint16 wchar;
	while ((wchar = stream->readUint16LE()) != '\0') {
		strData += wchar;
	}
	return strData;
}

void MacMenu::setAlignment(Graphics::TextAlign align) {
	_align = align;
}

MacMenu *MacMenu::createMenuFromPEexe(Common::PEResources *exe, MacWindowManager *wm) {
	Common::SeekableReadStream *menuData = exe->getResource(Common::kWinMenu, 128);
	if (!menuData)
		return nullptr;

	menuData->readUint16LE(); // wVersion
	menuData->readUint16LE(); // cbHeaderSize

	MacMenu *menu = wm->addMenu();

	Common::Stack<MacMenuSubMenu *> menus;
	Common::Stack<bool> popups;

	int depth = 0;
	bool lastPopUp = false;
	while (depth >= 0) {
		uint16 flags = menuData->readUint16LE();
		if (flags & kPopUp) {
			if (depth == 0) {
				menu->addMenuItem(nullptr, readUnicodeString(menuData));
			} else {
				Common::U32String name = readUnicodeString(menuData);
				menu->addMenuItem(menus.top(), name, -1, 0, 0, !(flags & kGrayed));
			}

			MacMenuSubMenu *submenu = menu->addSubMenu(menus.size() ? menus.top() : nullptr);
			menus.push(submenu);
			popups.push(lastPopUp);

			lastPopUp = (flags & kEndMenu) != 0;

			depth++;
		} else {
			int action = menuData->readUint16LE();
			Common::U32String name = readUnicodeString(menuData);
			if (depth > 0) {
				menu->addMenuItem(menus.top(), name, action, 0, 0, !(flags & kGrayed));
			}
			if (flags & kEndMenu) {
				menus.pop();
				depth--;

				if (lastPopUp) {
					depth--;

					if (menus.size())
						menus.pop();
				}

				lastPopUp = popups.pop();
			}
		}
	}

	menu->processTabs();

	delete menuData;

	if (gDebugLevel > 5)
		menu->printMenu();

	if (wm->_mode & Graphics::kWMModeWin95)
		menu->enableAllMenus();

	return menu;
}

void MacMenu::printMenu(int level, MacMenuSubMenu *submenu) {
	if (!level) {
		for (uint i = 0; i < _items.size(); i++) {
			debug("0. %s --> %d", _items[i]->unicode ? convertFromU32String(_items[i]->unicodeText).c_str() : _items[i]->text.c_str(), _items[i]->action);

			if (_items[i]->submenu != nullptr)
				printMenu(level + 1, _items[i]->submenu);
		}
	} else {
		for (uint i = 0; i < submenu->items.size(); i++) {
			debugN("%d. ", level);
			for (int j = 0; j < level; j++)
				debugN("  ");

			debug("%s --> %d", submenu->items[i]->unicode ? convertFromU32String(submenu->items[i]->unicodeText).c_str() : submenu->items[i]->text.c_str(), submenu->items[i]->action);

			if (submenu->items[i]->submenu != nullptr)
				printMenu(level + 1, submenu->items[i]->submenu);
		}
	}
}

int MacMenu::numberOfMenus() {
	return _items.size();
}

MacMenuItem *MacMenu::getMenuItem(const Common::String &menuId) {
	MacMenuItem *menu = nullptr;

	for (uint i = 0; i < _items.size(); i++) {
		// TODO: support unicode text menu
		// didn't support unicode item finding yet
		if (!_items[i]->unicode) {
			if (_items[i]->text.equalsIgnoreCase(menuId)) {
				menu = _items[i];
				break;
			}
		}
	}
	return menu;
}

MacMenuItem *MacMenu::getMenuItem(int menuId) {
	MacMenuItem *menu = nullptr;

	if ((uint)menuId < _items.size())
		menu = _items[menuId];

	return menu;
}

MacMenuItem *MacMenu::getSubMenuItem(MacMenuItem *menu, const Common::String &itemId) {
	if (!menu) {
		warning("MacMenu::getSubMenuItem: can not find given menu");
		return nullptr;
	}

	if (!menu->submenu) {
		warning("MacMenu::getSubMenuItem: menu %s doesn't have submenu", menu->text.c_str());
		return nullptr;
	}

	for (uint i = 0; i < menu->submenu->items.size(); i++) {
		if (!menu->submenu->items[i]->unicode) {
			if (menu->submenu->items[i]->text.equalsIgnoreCase(itemId))
				return menu->submenu->items[i];
		}
	}

	warning("MacMenu::getSubMenuItem: menu %s doesn't have item with id %s", menu->text.c_str(), itemId.c_str());
	return nullptr;
}

MacMenuItem *MacMenu::getSubMenuItem(MacMenuItem *menu, int itemId) {
	if (!menu) {
		warning("MacMenu::getSubMenuItem: menuId out of bounds");
		return nullptr;
	}

	if (!menu->submenu) {
		warning("MacMenu::getSubMenuItem: menu %s doesn't have submenu", menu->text.c_str());
		return nullptr;
	}

	if ((uint)itemId < menu->submenu->items.size())
		return menu->submenu->items[itemId];

	warning("MacMenu::getSubMenuItem: itemId %d out of bounds in menu %s", itemId, menu->text.c_str());
	return nullptr;
}

int MacMenu::numberOfMenuItems(MacMenuItem *menu) {
	if (!menu) {
		warning("MacMenu::numberOfMenuItems: can not find menu");
		return 0;
	}

	if (!menu->submenu) {
		warning("MacMenu::numberOfMenuItems: menu with id %s has no submenu", menu->text.c_str());
		return 0;
	}

	return menu->submenu->items.size();
}

void MacMenu::addStaticMenus(const MacMenuData *data) {
	MacMenuItem *about = new MacMenuItem(_wm->_fontMan->hasBuiltInFonts() ? "\xa9" : "\xf0"); // (c) Symbol as the most resembling apple
	_items.push_back(about);

	for (int i = 0; data[i].menunum; i++) {
		const MacMenuData *m = &data[i];
		MacMenuSubMenu *submenu = getSubmenu(nullptr, m->menunum);

		if (m->menunum == kMenuHighLevel) {
			addMenuItem(nullptr, m->title);

			continue;
		}

		if (submenu == nullptr)
			submenu = addSubMenu(nullptr, m->menunum);

		addMenuItem(submenu, m->title, m->action, 0, m->shortcut, m->enabled);
	}

	calcDimensions();
}

MacMenuSubMenu *MacMenu::addSubMenu(MacMenuSubMenu *submenu, int index) {
	_dimensionsDirty = true;

	if (submenu == nullptr) {
		if (index == -1)
			index = _items.size() - 1;

		if (_items[index]->submenu != nullptr)
			warning("Overwritting submenu #%d", index);
		return (_items[index]->submenu = new MacMenuSubMenu());
	} else {
		if (index == -1)
			index = submenu->items.size() - 1;

		if (submenu->items[index]->submenu != nullptr)
			warning("Overwritting submenu #%d", index);
		return (submenu->items[index]->submenu = new MacMenuSubMenu());
	}
}

MacMenuSubMenu *MacMenu::getSubmenu(MacMenuSubMenu *submenu, int index) {
	if (submenu == nullptr) {
		if (index < 0 || (uint)index >= _items.size())
			return nullptr;
		return _items[index]->submenu;
	} else {
		if (index < 0 || (uint)index >= submenu->items.size())
			return nullptr;
		return submenu->items[index]->submenu;
	}
}

int MacMenu::addMenuItem(MacMenuSubMenu *submenu, const Common::String &text, int action, int style, char shortcut, bool enabled, bool checked) {
	_dimensionsDirty = true;

	if (submenu == nullptr) {
		MacMenuItem *i = new MacMenuItem(text);
		_items.push_back(i);

		return _items.size() - 1;
	}

	_dimensionsDirty = true;

	submenu->items.push_back(new MacMenuItem(text, action, style, shortcut, -1, enabled, checked));

	return submenu->items.size() - 1;
}

int MacMenu::addMenuItem(MacMenuSubMenu *submenu, const Common::U32String &text, int action, int style, char shortcut, bool enabled, bool checked) {
	_dimensionsDirty = true;

	Common::U32String amp("&");
	Common::U32String res;
	int shortcutPos = -1;

	for (uint i = 0; i < text.size(); i++)
		if (text[i] == amp[0]) {
			if (i < text.size() - 1) {
				if ((text[i + 1] & 0xff) != '&') {
					shortcut = text[i + 1] & 0xff;
					shortcutPos = i;
				} else {
					res += text[i];
				}
			}
		} else {
			res += text[i];
		}

	if (submenu == nullptr) {
		_items.push_back(new MacMenuItem(res, -1, 0, shortcut, shortcutPos));

		return _items.size() - 1;
	}

	submenu->items.push_back(new MacMenuItem(res, action, style, shortcut, shortcutPos, enabled, checked));

	return submenu->items.size() - 1;
}

void MacMenu::insertMenuItem(MacMenuSubMenu *submenu, const Common::String &text, uint pos, int action, int style, char shortcut, bool enabled, bool checked) {
	_dimensionsDirty = true;

	if (submenu == nullptr) {
		if (pos >= _items.size()) {
			_dimensionsDirty = false;
		} else {
			MacMenuItem *i = new MacMenuItem(text);
			_items.insert(_items.begin() + pos, i);
		}
		return;
	}

	if (pos >= submenu->items.size()) {
		_dimensionsDirty = false;
		return;
	}

	_dimensionsDirty = true;

	submenu->items.insert(submenu->items.begin() + pos, new MacMenuItem(text, action, style, shortcut, -1, enabled, checked));
}

void MacMenu::insertMenuItem(MacMenuSubMenu *submenu, const Common::U32String &text, uint pos, int action, int style, char shortcut, bool enabled, bool checked) {
	_dimensionsDirty = true;

	Common::U32String amp("&");
	Common::U32String res;
	int shortcutPos = -1;

	for (uint i = 0; i < text.size(); i++)
		if (text[i] == amp[0]) {
			if (i < text.size() - 1) {
				if ((text[i + 1] & 0xff) != '&') {
					shortcut = text[i + 1] & 0xff;
					shortcutPos = i;
				} else {
					res += text[i];
				}
			}
		} else {
			res += text[i];
		}

	if (submenu == nullptr) {
		if (pos >= _items.size()) {
			_dimensionsDirty = false;
		} else {
			_items.insert(_items.begin() + pos, new MacMenuItem(res, -1, 0, shortcut, shortcutPos));
		}
		return;
	}

	if (pos >= submenu->items.size()) {
		_dimensionsDirty = false;
		return;
	}

	submenu->items.insert(submenu->items.begin() + pos, new MacMenuItem(res, action, style, shortcut, shortcutPos, enabled, checked));
}

void MacMenu::removeMenuItem(MacMenuSubMenu *submenu, uint pos) {
	_dimensionsDirty = true;

	if (submenu == nullptr) {
		if (pos >= _items.size()) {
			_dimensionsDirty = false;
		} else {
			delete _items.remove_at(pos);
		}
		return;
	}

	if (pos >= submenu->items.size()) {
		_dimensionsDirty = false;
		return;
	}
	delete submenu->items.remove_at(pos);
}

void MacMenu::calcDimensions() {
	// Calculate menu dimensions
	int y = 1;
	int x = 18;

	for (uint i = 0; i < _items.size(); i++) {
		int w = _items[i]->unicode ? _font->getStringWidth(_items[i]->unicodeText) : _font->getStringWidth(_items[i]->text);

		if (_items[i]->bbox.bottom == 0) {
			_items[i]->bbox.left = x - kMenuLeftMargin;
			_items[i]->bbox.top = y;
			_items[i]->bbox.right = x + w + kMenuSpacing - kMenuLeftMargin;
			_items[i]->bbox.bottom = y + _font->getFontHeight() + (_wm->_fontMan->hasBuiltInFonts() ? 3 : 2);
		}

		calcSubMenuBounds(_items[i]->submenu, _items[i]->bbox.left - 1, _items[i]->bbox.bottom + 1);

		if (_align == kTextAlignRight) {
			int right = _items[i]->bbox.right;
			int width = right - _items[i]->bbox.left;
			_items[i]->bbox.left = _bbox.right + _bbox.left - right;
			_items[i]->bbox.right = _items[i]->bbox.left + width;
		}

		x += w + kMenuSpacing;
	}

	_dimensionsDirty = false;
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
		addMenuItem(nullptr, menuTitle);
		MacMenuSubMenu *submenu = addSubMenu(nullptr);
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

			addMenuItem(submenu, subMenuTitle, action++, style, key, initialState & 1);

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

void MacMenu::setCheckMark(MacMenuItem *menuItem, bool checkMark) {
	if (menuItem) {
		menuItem->checked = checkMark;
		_contentIsDirty = true;
	}
}

void MacMenu::setEnabled(MacMenuItem *menuItem, bool enabled) {
	if (menuItem) {
		menuItem->enabled = enabled;
		_contentIsDirty = true;
	}
}

void MacMenu::setName(MacMenuItem *menuItem, const Common::String &name) {
	if (menuItem) {
		menuItem->text = name;
		_contentIsDirty = true;
	}
}

void MacMenu::setAction(MacMenuItem *menuItem, int actionId) {
	if (menuItem) {
		menuItem->action = actionId;
	}
}

bool MacMenu::getCheckMark(MacMenuItem *menuItem) {
	return menuItem ? menuItem->checked : false;
}

bool MacMenu::getEnabled(MacMenuItem *menuItem) {
	return menuItem ? menuItem->enabled : false;
}

Common::String MacMenu::getName(MacMenuItem *menuItem) {
	return menuItem ? menuItem->text : Common::String();
}

int MacMenu::getAction(MacMenuItem *menuItem) {
	return menuItem ? menuItem->action : 0;
}

void MacMenu::clearSubMenu(int id) {
	MacMenuItem *menu = _items[id];

	if (menu->submenu == nullptr)
		return;

	for (uint j = 0; j < menu->submenu->items.size(); j++)
		delete menu->submenu->items[j];

	menu->submenu->items.clear();
}

void MacMenu::createSubMenuFromString(int id, const char *str, int commandId) {
	clearSubMenu(id);

	Common::String string(str);
	Common::String item;
	MacMenuSubMenu *submenu = getSubmenu(nullptr, id);

	if (submenu == nullptr)
		submenu = addSubMenu(nullptr, id);

	for (uint i = 0; i < string.size(); i++) {
		while (i < string.size() && (string[i] != ';' && string[i] != '\r')) // Read token, consume \r for popup menu (MacPopUp)
			item += string[i++];

		if (item.lastChar() == ']') { // we have command id
			item.deleteLastChar();

			const char *p = strrchr(item.c_str(), '[');

			p++;

			if (p == NULL) {
				error("MacMenu::createSubMenuFromString(): Malformed menu item: '%s', bad format for actionId", item.c_str());
			}

			commandId = atoi(p);

			item = Common::String(item.c_str(), p - 1);
		}

		if (item == "(-") {
			addMenuItem(submenu, NULL, 0);
		} else {
			bool enabled = true;
			bool checked = false;
			int style = 0;
			char shortcut = 0;
			const char *shortPtr = strrchr(item.c_str(), '/');
			if (shortPtr != NULL) {
				if (strlen(shortPtr) >= 2) {
					shortcut = shortPtr[1];
					item.deleteChar(shortPtr - item.c_str());
					item.deleteChar(shortPtr - item.c_str());
				} else {
					// if we only have one / without shortcut key, we skip it.
					item.deleteChar(shortPtr - item.c_str());
					// error("MacMenu::createSubMenuFromString(): Unexpected shortcut: '%s', item '%s' in menu '%s'", shortPtr, item.c_str(), string.c_str());
				}
			}

			while (item.size() >= 2 && item[item.size() - 2] == '<') {
				char c = item.lastChar();
				if (c == 'B') {
					style |= kMacFontBold;
				} else if (c == 'I') {
					style |= kMacFontItalic;
				} else if (c == 'U') {
					style |= kMacFontUnderline;
				} else if (c == 'O') {
					style |= kMacFontOutline;
				} else if (c == 'S') {
					style |= kMacFontShadow;
				} else if (c == 'C') {
					style |= kMacFontCondense;
				} else if (c == 'E') {
					style |= kMacFontExtend;
				}
				item.deleteLastChar();
				item.deleteLastChar();
			}

			Common::String tmpitem(item);
			tmpitem.trim();
			// is that any places locate a parenthese will disable the item, or only the first char and last char counts.
			if (tmpitem.size() > 0 && (tmpitem[0] == '(' || tmpitem.lastChar() == '(')) {
				enabled = false;

				for (uint j = 0; j < item.size(); j++)
					if (item[j] == '(') {
						item.deleteChar(j);
						break;
					}
			} else if (tmpitem.size() >= 2 && tmpitem[0] == '!' && (uint8)tmpitem[1] == 195) {
				// this is the !√ situation, we need to set item checked, 195 represent √ in director
				checked = true;
				item = item.substr(2, Common::String::npos);
			}

			addMenuItem(submenu, item, commandId, style, shortcut, enabled, checked);
		}

		item.clear();
	}
}

const Font *MacMenu::getMenuFont(int slant) {
#ifdef USE_FREETYPE2
	if (_wm->_mode & kWMModeWin95 && !(_wm->_mode & kWMModeForceMacFontsInWin95)) {
		if (!_loadedFont) {
			_loadedFont = Graphics::loadTTFFontFromArchive("ms_sans_serif.ttf", 16);

			if (_loadedFont)
				return _loadedFont;
		} else {
			return _loadedFont;
		}

		// If font was not loaded, fallback
	}
#endif

	return _wm->_fontMan->getFont(Graphics::MacFont(kMacFontChicago, 12, slant));
}

const Common::String MacMenu::getAcceleratorString(MacMenuItem *item, const char *prefix) {
	if (item->shortcut == 0)
		return Common::String();

	return Common::String::format("%s%c%c", prefix, (_wm->_fontMan->hasBuiltInFonts() ? '^' : '\x11'), item->shortcut);
}

void MacMenu::processTabs() {
	for (uint i = 0; i < _items.size(); i++)
		processSubmenuTabs(_items[i]->submenu);
}

void MacMenu::processSubmenuTabs(MacMenuSubMenu *submenu) {
	if (submenu == nullptr)
		return;

	for (uint i = 0; i < submenu->items.size(); i++) {
		MacMenuSubMenu *menu = submenu->items[i]->submenu;

		if (menu != nullptr)
			processSubmenuTabs(menu);
	}

	int maxWidth = 0;
	bool haveTabs = false;

	Common::U32String tabSymbol("\t");

	// First, we replace \t with two spaces, and thus, obtain
	// the widest string
	for (uint i = 0; i < submenu->items.size(); i++) {
		MacMenuItem *item = submenu->items[i];
		if (item->unicodeText.empty())
			continue;

		Common::U32String res(item->unicodeText);

		uint32 pos = item->unicodeText.find(tabSymbol);

		if (pos != Common::U32String::npos) {
			// Sanity check
			if (pos == 0 || pos >= item->unicodeText.size())
				error("Malformed menu: tab position");

			if (item->unicodeText.find(tabSymbol, pos + 1) != Common::U32String::npos)
				error("Malformed menu: extra tab");

			haveTabs = true;

			Common::U32String start = item->unicodeText.substr(0, pos);
			Common::U32String end = item->unicodeText.substr(pos + 1);

			res = start;
			res += Common::U32String("  ");
			res += end;
		}

		int width = _font->getStringWidth(res);
		if (width > maxWidth) {
			maxWidth = width;
		}
	}

	if (!haveTabs)
		return;

	// Now expand each tab to the relevant width
	// And yes, right edge is going to be uneven
	for (uint i = 0; i < submenu->items.size(); i++) {
		MacMenuItem *item = submenu->items[i];
		if (item->unicodeText.empty())
			continue;

		uint32 pos = item->unicodeText.find(tabSymbol);

		if (pos == Common::U32String::npos)
			continue;

		Common::U32String start = item->unicodeText.substr(0, pos);
		Common::U32String end = item->unicodeText.substr(pos + 1);
		Common::U32String res;
		Common::U32String spaces(" ");
		int width;

		do {
			res = start;
			res += spaces;
			res += end;

			width = _font->getStringWidth(res);

			spaces += spaces[0];
		} while (width < maxWidth);

		item->unicodeText = res;
	}
}

int MacMenu::calcSubMenuWidth(MacMenuSubMenu *submenu) {
	int maxWidth = 0;
	if (submenu == nullptr)
		return maxWidth;

	for (uint i = 0; i < submenu->items.size(); i++) {
		MacMenuItem *item = submenu->items[i];
		if (!item->text.empty()) {
			Common::String text(item->text);
			Common::String acceleratorText(getAcceleratorString(item, "  "));

			const Font *font = getMenuFont(item->style);
			int width = font->getStringWidth(text);

			if (!acceleratorText.empty()) {
				width += _font->getStringWidth(acceleratorText);
			}

			if (item->submenu != nullptr) { // If we're drawing triangle
				width += _font->getStringWidth("  ");
			}

			if (width > maxWidth) {
				maxWidth = width;
			}
		} else if (!item->unicodeText.empty()) {
			Common::U32String text(item->unicodeText);

			if (item->submenu != nullptr) // If we're drawing triangle
				text += Common::U32String("  ");

			int width = _font->getStringWidth(text);
			if (width > maxWidth) {
				maxWidth = width;
			}
		}
	}
	return maxWidth > 200 ? 200 : maxWidth;
}

void MacMenu::calcSubMenuBounds(MacMenuSubMenu *submenu, int x, int y) {
	if (submenu == nullptr)
		return;

	// TODO: cache maxWidth
	int maxWidth = calcSubMenuWidth(submenu);
	int x1 = x;
	int y1 = y;
	int x2 = x1 + maxWidth + _menuLeftDropdownPadding + _menuRightDropdownPadding - 4;
	int y2 = y1 + submenu->items.size() * _menuDropdownItemHeight + 2;
	y2 = MIN(y2, y1 + ((_screen.h - y1) / _menuDropdownItemHeight) * _menuDropdownItemHeight + 2);

	submenu->bbox.left = x1;
	submenu->bbox.top = y1;
	submenu->bbox.right = x2;
	submenu->bbox.bottom = y2;

	if (_align == kTextAlignRight) {
		submenu->bbox.left = _bbox.right + _bbox.left - x2;
		submenu->bbox.right = submenu->bbox.left + x2 - x1;
	}

	for (uint i = 0; i < submenu->items.size(); i++) {
		MacMenuSubMenu *menu = submenu->items[i]->submenu;

		if (menu != nullptr)
			calcSubMenuBounds(menu, x2 - 4, y1 + i * _menuDropdownItemHeight + 1);
	}
}

template <typename T>
static void drawPixelPlain(int x, int y, int color, void *data) {
	ManagedSurface *surface = (ManagedSurface *)data;

	if (x >= 0 && x < surface->w && y >= 0 && y < surface->h)
		*((T *)surface->getBasePtr(x, y)) = (T)color;
}

template <typename T>
static void drawFilledRoundRect(ManagedSurface *surface, Common::Rect &rect, int arc, int color) {
	drawRoundRect(rect, arc, color, true, drawPixelPlain<T>, surface);
}

template <typename T>
static void drawMenuPattern(ManagedSurface &srcSurf, ManagedSurface &destSurf, const byte *pattern, int x, int y, int width, uint32 colorKey) {
	// I am lazy to extend drawString() with plotProc as a parameter, so
	// fake it here

	for (int ii = 0; ii < srcSurf.h; ii++) {
		const T *src = (const T *)srcSurf.getBasePtr(0, ii);
		T *dst = (T *)destSurf.getBasePtr(x, y + ii);
		byte pat = pattern[ii % 8];
		for (int j = 0; j < width; j++) {
			if (*src != colorKey && (pat & (1 << (7 - (x + j) % 8))))
				*dst = *src;
			src++;
			dst++;
		}
	}
}

template <typename T>
static void drawMenuDelimiter(ManagedSurface &srcSurf, Common::Rect *r, int y, uint32 black, uint32 white) {
	bool flip = r->left & 2;
	T *ptr = (T *)srcSurf.getBasePtr(r->left + 1, y);
	for (int xx = r->left + 1; xx <= r->right - 1; xx++, ptr++) {
		*ptr = flip ? black : white;
		flip = !flip;
	}
}

static void underlineAccelerator(ManagedSurface *dst, const Font *font, const Common::UnicodeBiDiText &txt, int x, int y, int shortcutPos, uint32 color) {
	if (shortcutPos == -1)
		return;

	int visualPos = txt.getVisualPosition(shortcutPos);
	Common::U32String s(txt.visual);

	// Erase characters only if it is not end of the string
	if ((uint)(visualPos + 1) < s.size())
		s.erase(visualPos + 1);

	int pos2 = font->getStringWidth(s);
	s.deleteLastChar();
	int pos1 = font->getStringWidth(s);

	dst->hLine(x + pos1, y + font->getFontHeight(), x + pos2 - 1, color);
}

bool MacMenu::draw(ManagedSurface *g, bool forceRedraw) {
	Common::Rect r(_bbox);

	if (!_isVisible)
		return false;

	if (_dimensionsDirty)
		calcDimensions();

	if (!_contentIsDirty && !forceRedraw)
		return false;

	_contentIsDirty = false;

	_screen.clear(_wm->_colorGreen);

	bool shouldUseDesktopArc = !(_wm->_mode & kWMModeWin95) || (_wm->_mode & kWMModeForceMacBorder);

	// Fill in the corners with black
	_screen.fillRect(r, _wm->_colorBlack);

	if (_wm->_pixelformat.bytesPerPixel == 1) {
		drawFilledRoundRect<byte>(&_screen, r, shouldUseDesktopArc ? kDesktopArc : 0, _wm->_colorWhite);
	} else {
		drawFilledRoundRect<uint32>(&_screen, r, shouldUseDesktopArc ? kDesktopArc : 0, _wm->_colorWhite);
	}

	r.top = 7;
	_screen.fillRect(r, _wm->_colorWhite);
	r.top = kMenuHeight - 1;
	r.bottom++;
	_screen.fillRect(r, _wm->_colorGreen);
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

			if (_align == kTextAlignRight) {
				hbox.left -= 2;
				hbox.right -= 2;
			}

			_screen.fillRect(hbox, _wm->_colorBlack);
			color = _wm->_colorWhite;
		}

		int y = it->bbox.top + (_wm->_fontMan->hasBuiltInFonts() ? 2 : 1);
		int x = _align == kTextAlignRight ? -kMenuLeftMargin : kMenuLeftMargin;
		x += it->bbox.left;

		if (it->unicode) {
			int accOff = _align == kTextAlignRight ? it->bbox.width() - _font->getStringWidth(it->unicodeText) : 0;
			Common::UnicodeBiDiText utxt(it->unicodeText);

			_font->drawString(&_screen, utxt.visual, x, y, it->bbox.width(), color, _align, 0, true);
			underlineAccelerator(&_screen, _font, utxt, x + accOff, y, it->shortcutPos, color);
		} else {
			const Font *font = getMenuFont(it->style);

			font->drawString(&_screen, it->text, x, y, it->bbox.width(), color, Graphics::kTextAlignLeft, 0, true);
		}
	}

	if ((_wm->_mode & kWMModalMenuMode) || !_wm->_screen)
		g_system->copyRectToScreen(_screen.getBasePtr(_bbox.left, _bbox.top), _screen.pitch, _bbox.left, _bbox.top, _bbox.width(), _bbox.height());


	for (uint i = 0; i < _menustack.size(); i++) {
		renderSubmenu(_menustack[i], (i == _menustack.size() - 1));
	}

	if (g)
		g->transBlitFrom(_screen, _wm->_colorGreen);

	if (!(_wm->_mode & kWMModalMenuMode) && g)
		g_system->copyRectToScreen(g->getPixels(), g->pitch, 0, 0, g->w, g->h);

	return true;
}

void MacMenu::renderSubmenu(MacMenuSubMenu *menu, bool recursive) {
	Common::Rect *r = &menu->bbox;

	if (r->width() == 0 || r->height() == 0)
		return;

	_screen.fillRect(*r, _wm->_colorWhite);
	_screen.frameRect(*r, _wm->_colorBlack);
	_screen.vLine(r->right, r->top + 3, r->bottom + 1, _wm->_colorBlack);
	_screen.vLine(r->right + 1, r->top + 3, r->bottom + 1, _wm->_colorBlack);
	_screen.hLine(r->left + 3, r->bottom, r->right + 1, _wm->_colorBlack);
	_screen.hLine(r->left + 3, r->bottom + 1, r->right + 1, _wm->_colorBlack);

	int y = r->top + 1;
	int x = _align == kTextAlignRight ? -_menuRightDropdownPadding: _menuLeftDropdownPadding;
	x += r->left;

	int maxVis = menu->bbox.height() / _menuDropdownItemHeight;
	int numVis = menu->items.size() - menu->visStart - menu->visEnd + ABS(menu->scroll);
	numVis = MIN(numVis, maxVis);

	int ovTop = menu->visStart + menu->scroll; // Number of items overflowing from top
	int ovBot = menu->items.size() - ovTop - numVis; // Number of items overflowing from bottom

	for (int i = menu->visStart + menu->scroll; i < (int)menu->items.size(); i++) {
		if ((ovTop && i == menu->visStart + menu->scroll) ||
			(ovBot && i == numVis - 1 + menu->scroll + menu->visStart)) {
			int direction = (i == menu->visStart + menu->scroll) ? 1 : -1;

			int arrowX = _align == kTextAlignRight ? menu->bbox.right - _menuRightDropdownPadding : menu->bbox.left + _menuLeftDropdownPadding;
			int arrowY = direction == 1 ? menu->bbox.top + _menuDropdownItemHeight / 2 : menu->bbox.bottom - _menuDropdownItemHeight / 2;

			drawScrollArrow(arrowX, arrowY, direction);

			y += _menuDropdownItemHeight;
			continue;
		}

		Common::String text(menu->items[i]->text);
		Common::String acceleratorText(getAcceleratorString(menu->items[i], ""));

		Common::U32String unicodeText(menu->items[i]->unicodeText);

		int accOff = _align == kTextAlignRight ? r->width() - _font->getStringWidth(unicodeText) : 0;

		int shortcutPos = menu->items[i]->shortcutPos;

		int accelX = r->right - 25;
		int arrowX = r->right - 14;

		if (_align == kTextAlignRight) {
			accelX = r->left + 25;
			arrowX = r->left + 14;
		}

		int color = _wm->_colorBlack;
		if (i == menu->highlight && (!text.empty() || !unicodeText.empty()) && menu->items[i]->enabled) {
			color = _wm->_colorWhite;
			Common::Rect trect(r->left, y - (_wm->_fontMan->hasBuiltInFonts() ? 1 : 0), r->right, y + _font->getFontHeight());

			_screen.fillRect(trect, _wm->_colorBlack);
		}

		if (!text.empty() || !unicodeText.empty()) {
			ManagedSurface *s = &_screen;
			int tx = x, ty = y;

			if (!menu->items[i]->enabled) {
				s = &_tempSurface;
				tx = 0;
				ty = 0;
				accelX -= x;

				_tempSurface.clear(_wm->_colorGreen);
			}

			if (menu->items[i]->unicode) {
				Common::UnicodeBiDiText utxt(unicodeText);
				_font->drawString(s, utxt.visual, tx, ty, r->width(), color, _align, 0, true);
				underlineAccelerator(s, _font, utxt, tx + accOff, ty, shortcutPos, color);
			} else {
				const Font *font = getMenuFont(menu->items[i]->style);

				font->drawString(s, text, tx, ty, r->width(), color, Graphics::kTextAlignLeft, 0, true);
			}

			if (menu->items[i]->checked) {
				const Font *font = getMenuFont(menu->items[i]->style);
				int checkSymbol = _wm->_fontMan->hasBuiltInFonts() ? 0xD7 : 18;

				int padding = _align == kTextAlignRight ? -_menuRightDropdownPadding: _menuLeftDropdownPadding;
				int offset = padding - font->getCharWidth(checkSymbol);

				// calculating the padding and offset, we draw the √ at the center
				font->drawChar(s, checkSymbol, tx - padding + offset, ty, color);
			}

			if (!acceleratorText.empty() && shortcutPos == -1)
				_font->drawString(s, acceleratorText, accelX, ty, r->width(), color);

			if (menu->items[i]->submenu != nullptr)
				drawSubMenuArrow(s, arrowX, ty, color);

			if (!menu->items[i]->enabled) {
				if (_wm->_pixelformat.bytesPerPixel == 1) {
					drawMenuPattern<byte>(_tempSurface, _screen, _wm->getBuiltinPatterns()[kPatternCheckers2 - 1], x, y, r->width(), _wm->_colorGreen);
				} else {
					drawMenuPattern<uint32>(_tempSurface, _screen, _wm->getBuiltinPatterns()[kPatternCheckers2 - 1], x, y, r->width(), _wm->_colorGreen);
				}
			}

		} else { // Delimiter
			if (_wm->_pixelformat.bytesPerPixel == 1) {
				drawMenuDelimiter<byte>(_screen, r, y + _menuDropdownItemHeight / 2, _wm->_colorBlack, _wm->_colorWhite);
			} else {
				drawMenuDelimiter<uint32>(_screen, r, y + _menuDropdownItemHeight / 2, _wm->_colorBlack, _wm->_colorWhite);
			}
		}

		y += _menuDropdownItemHeight;
	}

	if (recursive && menu->highlight != -1 && menu->items[menu->highlight]->submenu != nullptr)
		renderSubmenu(menu->items[menu->highlight]->submenu, false);

	if (_wm->_mode & kWMModalMenuMode) {
		// TODO: Instead of cropping, reposition the submenu
		int w = r->width() + 2;
		if (r->left + w >= _screen.w)
			w = _screen.w - 1 - r->left;

		int h = r->height() + 2;
		if (r->top + h >= _screen.h)
			h = _screen.h - 1 - r->top;

		Graphics::ManagedSurface g;
		g.copyFrom(*_wm->_screenCopy);
		g.transBlitFrom(_screen, _wm->_colorGreen);
		g_system->copyRectToScreen(g.getBasePtr(r->left, r->top), g.pitch, r->left, r->top, w, h);
	}
}

void MacMenu::drawSubMenuArrow(ManagedSurface *dst, int x, int y, int color) {
	int height = 11;
	for (int col = 0; col < 6; col++) {
		if (_align != kTextAlignRight) {
			dst->vLine(x + col, y + col + 1, y + col + height + 1, color);
		} else {
			dst->vLine(x - col, y + col + 1, y + col + height + 1, color);
		}
		height -= 2;
	}
}

bool MacMenu::processEvent(Common::Event &event) {
	if (!_isVisible && event.type != Common::EVENT_KEYDOWN)
		return false;

	switch (event.type) {
	case Common::EVENT_QUIT:
		closeMenu();
		return false;
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
			return processMenuShortCut(event.kbd.ascii);
		}
	}

	return false;
}

bool MacMenu::checkIntersects(Common::Rect &rect) {
	if (_bbox.intersects(rect))
		return true;
	for (uint i = 0; i < _menustack.size(); i++)
		if (_menustack[i]->bbox.intersects(rect))
			return true;
	return false;
}

static void scrollCallback(void *data) {
	MacMenu *menu = (MacMenu *)data;
	MacMenuSubMenu *subMenu = menu->_menustack.back();

	int maxVis = subMenu->bbox.height() / menu->getDropdownItemHeight();
	int numVis = subMenu->items.size() - subMenu->visStart - subMenu->visEnd + ABS(subMenu->scroll);
	numVis = MIN(numVis, maxVis);

	int ovTop = subMenu->visStart + subMenu->scroll; // Number of items overflowing from top
	int ovBot = subMenu->items.size() - ovTop - numVis; // Number of items overflowing from bottom

	if (menu->getScrollDirection() == -1) {
		if (ovTop) {
			subMenu->scroll--;
			menu->renderSubmenu(subMenu);
		}
	} else {
		if (ovBot) {
			subMenu->scroll++;
			menu->renderSubmenu(subMenu);
		}
	}
}

bool MacMenu::mouseClick(int x, int y) {
	if (!_isModal &&_bbox.contains(x, y)) {
		for (uint i = 0; i < _items.size(); i++) {
			if (_items[i]->bbox.contains(x, y)) {
				if ((uint)_activeItem == i)
					return true; // We catch the event but do not do anything

				if (_activeItem != -1) { // Restore background
					if (_items[_activeItem]->submenu != nullptr) {
						if (_wm->_mode & kWMModalMenuMode)
							g_system->copyRectToScreen(_wm->_screenCopy->getPixels(), _wm->_screenCopy->pitch, 0, 0, _wm->_screenCopy->w, _wm->_screenCopy->h);

						_menustack.pop_back(); // Drop previous submenu
						_contentIsDirty = true;
						_wm->setFullRefresh(true);
					}
				}

				_activeItem = i;
				_activeSubItem = -1;
				if (_items[_activeItem]->submenu != nullptr) {
					_menustack.push_back(_items[_activeItem]->submenu);
					_items[_activeItem]->submenu->highlight = -1;
					_contentIsDirty = true;
				}
			}
		}

		if (!_active)
			_wm->activateMenu();

		setActive(true);
		return true;
	}

	if (!_active)
		return false;

	if (_menustack.size() > 0 && _menustack.back()->bbox.contains(x, y)) {
		MacMenuSubMenu *menu = _menustack.back();
		int numSubItem = menu->ytoItem(y, _menuDropdownItemHeight);
		int selectedOption = numSubItem;
		numSubItem += menu->visStart + menu->scroll;
		numSubItem = MIN((uint)numSubItem, menu->items.size() - 1);

		if (numSubItem != _activeSubItem) {
			if (_wm->_mode & kWMModalMenuMode) {
				if (_activeSubItem == -1 || menu->items[_activeSubItem]->submenu != nullptr)
					g_system->copyRectToScreen(_wm->_screenCopy->getPixels(), _wm->_screenCopy->pitch, 0, 0, _wm->_screenCopy->w, _wm->_screenCopy->h);
			}

			int maxVis = menu->bbox.height() / _menuDropdownItemHeight;
			int numItemsVisible = menu->items.size() - menu->visStart - menu->visEnd + ABS(menu->scroll);
			numItemsVisible = MIN(numItemsVisible, maxVis);

			int ovTop = menu->visStart + menu->scroll;
			int ovBot = menu->items.size() - ovTop - numItemsVisible;

			// If the user clicks on the "Scroll Up Arrow" or the "Scroll Down Arrow" then ignore the selection
			if (!(ovTop && selectedOption == 0) && !(ovBot && numItemsVisible == maxVis && menu->ytoItem(y, _menuDropdownItemHeight) == numItemsVisible - 1)) {
				_activeSubItem = numSubItem;
				menu->highlight = _activeSubItem;
			}

			if (selectedOption == 0) {
				if (ovTop) {
					if (!_scrollTimerActive) {
						_scrollDirection = -1;
						_scrollTimerActive = true;
						g_system->getTimerManager()->installTimerProc(&scrollCallback, SCROLL_DELAY, this, "Scroll Up Handler");
					}
				}
			} else if (selectedOption == numItemsVisible - 1) {
				if (ovBot && numItemsVisible == maxVis) {
					if (!_scrollTimerActive) {
						_scrollDirection = 1;
						_scrollTimerActive = true;
						g_system->getTimerManager()->installTimerProc(&scrollCallback, SCROLL_DELAY, this, "Scroll Down Handler");
					}
				}
			} else {
				if (_scrollTimerActive) {
					_scrollTimerActive = false;
					g_system->getTimerManager()->removeTimerProc(&scrollCallback);
				}
			}

			_contentIsDirty = true;
			_wm->setFullRefresh(true);
		}

		return true;
	} else {
		if (_scrollTimerActive) {
			_scrollTimerActive = false;
			g_system->getTimerManager()->removeTimerProc(&scrollCallback);
		}
	}

	if (_activeSubItem != -1 && _menustack.back()->items[_activeSubItem]->submenu != nullptr) {
		if (_menustack.back()->items[_activeSubItem]->submenu->bbox.contains(x, y)) {
			_menustack.push_back(_menustack.back()->items[_activeSubItem]->submenu);

			_activeSubItem = 0;
			_contentIsDirty = true;
			_wm->setFullRefresh(true);

			_menustack.back()->highlight = 0;

			return true;
		}
	}

	if (_menustack.size() > 1) {
		if (_menustack[_menustack.size() - 2]->bbox.contains(x, y)) {
			_menustack.back()->highlight = -1; // Erase it for the closed popup

			if (_wm->_mode & kWMModalMenuMode) {
				int x1 = _menustack.back()->bbox.left;
				int y1 = _menustack.back()->bbox.top;
				uint w = _menustack.back()->bbox.width() + 2;
				uint h = _menustack.back()->bbox.height() + 2;

				if (x1 + (int)w > _wm->_screenCopy->w)
					w = _wm->_screenCopy->w - 1 - x1;
				if (y1 + (int)h > _wm->_screenCopy->h)
					h = _wm->_screenCopy->h - 1 - y1;

				g_system->copyRectToScreen(_wm->_screenCopy->getBasePtr(x1, y1), _wm->_screenCopy->pitch, x1, y1, w, h);
			}

			_menustack.pop_back();

			MacMenuSubMenu *menu = _menustack.back();

			_activeSubItem = menu->highlight = menu->ytoItem(y, _menuDropdownItemHeight);

			_contentIsDirty = true;
			_wm->setFullRefresh(true);

			return true;
		}
	}

	if (_activeItem != -1) {
		_activeSubItem = -1;

		return true;
	}

	return false;
}

bool MacMenu::contains(int x, int y) {
	if (_bbox.contains(x, y))
		return true;
	for (uint i = 0; i < _menustack.size(); i++) {
		if (_menustack[i]->bbox.contains(x, y))
			return true;
	}
	if (_activeSubItem != -1 && _menustack.back()->items[_activeSubItem]->submenu != nullptr)
		if (_menustack.back()->items[_activeSubItem]->submenu->bbox.contains(x, y))
			return true;
	return false;
}

void MacMenu::drawScrollArrow(int arrowX, int arrowY, int direction) {
	int arrowHeight = getMenuFont()->getFontHeight() / 2;

	for (int j = 0; j <= arrowHeight / 2; j++)
		_screen.hLine(arrowX - j, arrowY + j * direction, arrowX + j, _wm->_colorBlack);
}

bool MacMenu::mouseMove(int x, int y) {
	if (_active) {
		if (mouseClick(x, y))
			return true;
	} else if ((_wm->_mode & kWMModeAutohideMenu) && !_bbox.contains(x, y)) {
		_isVisible = false;
		if (_wm->_mode & kWMModalMenuMode) {
			_wm->disableScreenCopy();
		}
	}

	return false;
}

bool MacMenu::checkCallback(bool unicode) {
	if (unicode) {
		if (_unicodeccallback == nullptr) {
			warning("MacMenu: no Unicode callback set");
			return false;
		}
	} else {
		if (_ccallback == nullptr) {
			warning("MacMenu: no callback set");
			return false;
		}
	}

	return true;
}

void MacMenu::closeMenu() {
	setActive(false);

	if (_scrollTimerActive) {
		_scrollTimerActive = false;
		g_system->getTimerManager()->removeTimerProc(&scrollCallback);
	}

	if (_wm->_mode & kWMModeAutohideMenu)
		_isVisible = false;

	if (_wm->_mode & kWMModalMenuMode) {
		_wm->disableScreenCopy();
	}

	for (uint i = 0; i < _menustack.size(); i++) {
		_menustack[i]->highlight = -1;
	}

	_activeItem = -1;
	_activeSubItem = -1;
	_menustack.clear();

	_wm->setFullRefresh(true);
}

bool MacMenu::mouseRelease(int x, int y) {
	if (!_active)
		return false;

	bool haveCallBack = false;
	if (_activeItem != -1 && _activeSubItem != -1 && _menustack.back()->items[_activeSubItem]->enabled) {
		// no action if item has submenu
		if (_menustack.back()->items[_activeSubItem]->submenu) {
			return false;
		}
		if (_menustack.back()->items[_activeSubItem]->unicode) {
			if (checkCallback(true)) {
				(*_unicodeccallback)(_menustack.back()->items[_activeSubItem]->action,
									 _menustack.back()->items[_activeSubItem]->unicodeText, _cdata);
				haveCallBack = true;
			}
		} else {
			if (checkCallback()) {
				(*_ccallback)(_menustack.back()->items[_activeSubItem]->action,
							  _menustack.back()->items[_activeSubItem]->text, _cdata);
				haveCallBack = true;
			}
		}
	}

	// Set last active items and subitems before leaving!
	_lastActiveItem = _activeItem;
	_lastActiveSubItem = _activeSubItem;

	// if the mode is not win95, or the click position is outside of the menu, then we close it
	if (!(_wm->_mode & kWMModeWin95) || !contains(x, y) || haveCallBack)
		closeMenu();
	return true;
}

bool MacMenu::processMenuShortCut(uint16 ascii) {
	ascii = tolower(ascii);

	for (uint i = 0; i < _items.size(); i++) {
		if (_items[i]->submenu != nullptr) {
			for (uint j = 0; j < _items[i]->submenu->items.size(); j++) {
				if (_items[i]->submenu->items[j]->enabled && tolower(_items[i]->submenu->items[j]->shortcut) == ascii) {
					if (_items[i]->submenu->items[j]->unicode) {
						if (checkCallback(true))
							(*_unicodeccallback)(_items[i]->submenu->items[j]->action, _items[i]->submenu->items[j]->unicodeText, _cdata);
					} else {
						if (checkCallback())
							(*_ccallback)(_items[i]->submenu->items[j]->action, _items[i]->submenu->items[j]->text, _cdata);
					}
					return true;
				}
			}
		}
	}

	return false;
}

void MacMenu::enableCommand(int menunum, int action, bool state) {
	if (!_items[menunum]->submenu)
		return;

	for (uint i = 0; i < _items[menunum]->submenu->items.size(); i++)
		if (_items[menunum]->submenu->items[i]->action == action)
			_items[menunum]->submenu->items[i]->enabled = state;

	_contentIsDirty = true;
}

void MacMenu::enableCommand(const char *menuitem, const char *menuaction, bool state) {
	uint menunum = 0;

	while (menunum < _items.size()) {
		assert(!_items[menunum]->unicode);
		if (_items[menunum]->text.equalsIgnoreCase(menuitem))
			break;
		else
			menunum++;
	}
	if (menunum == _items.size())
		return;

	if (_items[menunum]->submenu != nullptr)
		for (uint i = 0; i < _items[menunum]->submenu->items.size(); i++) {
			assert(!_items[menunum]->submenu->items[i]->unicode);
			if (_items[menunum]->submenu->items[i]->text.equalsIgnoreCase(menuaction))
				_items[menunum]->submenu->items[i]->enabled = state;
		}

	_contentIsDirty = true;
}

void MacMenu::enableCommand(const Common::U32String &menuitem, const Common::U32String &menuaction, bool state) {
	uint menunum = 0;

	while (menunum < _items.size()) {
		assert(_items[menunum]->unicode);
		if (_items[menunum]->unicodeText.equals(menuitem))
			break;
		else
			menunum++;
	}
	if (menunum == _items.size())
		return;

	if (_items[menunum]->submenu != nullptr)
		for (uint i = 0; i < _items[menunum]->submenu->items.size(); i++) {
			assert(_items[menunum]->submenu->items[i]->unicode);
			if (_items[menunum]->submenu->items[i]->unicodeText.equals(menuaction))
				_items[menunum]->submenu->items[i]->enabled = state;
		}

	_contentIsDirty = true;
}

void MacMenu::enableAllMenus() {
	for (uint i = 0; i < _items.size(); i++)
		if (_items[i]->submenu != nullptr)
			_items[i]->submenu->enableAllItems();

	_contentIsDirty = true;
}

void MacMenu::disableAllMenus() {
	for (uint i = 1; i < _items.size(); i++) // Leave About menu on
		if (_items[i]->submenu != nullptr)
			for (uint j = 0; j < _items[i]->submenu->items.size(); j++)
				_items[i]->submenu->items[j]->enabled = false;

	_contentIsDirty = true;
}

void MacMenu::eventLoop() {
	_contentIsDirty = true;

	while (_active) {
		Common::Event event;

		while (g_system->getEventManager()->pollEvent(event)) {
			processEvent(event);

			draw(_wm->_screen);
		}

		if (_active) {
			g_system->updateScreen();
			g_system->delayMillis(10);
		}
	}
}

} // End of namespace Graphics
