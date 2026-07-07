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

#include "common/tokenizer.h"
#include "graphics/macgui/macmenu.h"

#include "graphics/mactoolbox/toolbox.h"

namespace Graphics {
namespace MacToolbox {

void Toolbox::AppendMenu(MenuHandle &theMenu, const Common::U32String &data) {
	if (!theMenu) {
		error("Toolbox::AppendMenu: Invalid menu handle");
		return;
	}

	debugC(5, kDebugLevelMacToolbox, "Toolbox::AppendMenu: menuID %d, data \"%s\"", theMenu->menuID, data.encode().c_str());

	if (_defaultMenu) {
		Graphics::MacMenuSubMenu *sub = _defaultMenu->getSubmenu(nullptr, theMenu->menuID-1);
		if (!sub) {
			debugC(0, kDebugLevelMacToolbox, "Toolbox::AppendMenu: menu ID %d not found, skipping", theMenu->menuID);
			return;
		}
		_defaultMenu->appendMenu(sub, data.encode(Common::kMacRoman), 0);
		Common::U32StringTokenizer tok(data, "\r;");
		while (!tok.empty()) {
			Common::U32String token = tok.nextToken();
			theMenu->menuData.push_back(token);
		}
	}
}

void Toolbox::CheckItem(MenuHandle &theMenu, uint16 item, bool checked) {
	if (!theMenu) {
		debugC(0, kDebugLevelMacToolbox, "Toolbox::CheckItem: empty menu handle");
		return;
	}
	if (theMenu->menuData.size() < (uint)(item)) {
		debugC(0, kDebugLevelMacToolbox, "Toolbox::CheckItem: item %d out of range for menu %d", item, theMenu->menuID);
		return;
	}
	if (_defaultMenu) {
		Graphics::MacMenuItem *sub = _defaultMenu->getMenuItem(theMenu->menuID-1);
		if (sub) {
			Graphics::MacMenuItem *it = _defaultMenu->getSubMenuItem(sub, item-1);
			if (it) {
				it->checked = checked;
			}
		}
	}
}

void Toolbox::ClearMenuBar() {
	if (_defaultMenu) {
		_defaultMenu->setVisible(false);
	}
}

uint16 Toolbox::CountMItems(MenuHandle &theMenu) {
	if (!theMenu) {
		error("Toolbox::CountMItems: Invalid menu handle");
		return 0;
	}
	return theMenu->menuData.size();
}

void Toolbox::DeleteMenu(uint16 menuID) {
	debugC(5, kDebugLevelMacToolbox, "Toolbox::DeleteMenu: menuID %d", menuID);
	if (_menu.contains(menuID)) {
		_menu.erase(menuID);
	}
	if (_defaultMenu) {
		_defaultMenu->clearSubMenu(menuID-1);
		_defaultMenu->removeMenuItem(nullptr, menuID-1);
		_defaultMenu->insertMenuItem(nullptr, "", menuID-1);
	}
}

void Toolbox::DisableItem(MenuHandle &theMenu, uint16 item) {
	if (!theMenu) {
		debugC(0, kDebugLevelMacToolbox, "Toolbox::DisableItem: empty menu handle");
		return;
	}
	if (theMenu->menuData.size() < (uint)(item)) {
		debugC(0, kDebugLevelMacToolbox, "Toolbox::DisableItem: item %d out of range for menu %d", item, theMenu->menuID);
		return;
	}
	if (_defaultMenu) {
		Graphics::MacMenuItem *sub = _defaultMenu->getMenuItem(theMenu->menuID-1);
		if (sub) {
			Graphics::MacMenuItem *it = _defaultMenu->getSubMenuItem(sub, item-1);
			if (it) {
				it->enabled = false;
			}
		}
	}
}

void Toolbox::DisposeMenu(MenuHandle &theMenu) {
	// only frees memory, DeleteMenu is responsible for removing it from the menu state
	theMenu = nullptr;
}

void Toolbox::DrawMenuBar() {
	if (_defaultMenu) {
		_defaultMenu->setVisible(true);
	}
}

void Toolbox::EnableItem(MenuHandle &theMenu, uint16 item) {
	if (!theMenu) {
		debugC(0, kDebugLevelMacToolbox, "Toolbox::EnableItem: empty menu handle");
		return;
	}
	if (theMenu->menuData.size() < (uint)(item)) {
		debugC(0, kDebugLevelMacToolbox, "Toolbox::EnableItem: item %d out of range for menu %d", item, theMenu->menuID);
		return;
	}
	if (_defaultMenu) {
		Graphics::MacMenuItem *sub = _defaultMenu->getMenuItem(theMenu->menuID-1);
		if (sub) {
			Graphics::MacMenuItem *it = _defaultMenu->getSubMenuItem(sub, item-1);
			if (it) {
				it->enabled = true;
			}
		}
	}
}


MenuHandle Toolbox::GetMHandle(uint16 menuID) {
	MenuHandle result;
	if (_menu.contains(menuID)) {
		result = _menu[menuID];
	}
	return result;
}

void Toolbox::HiliteMenu(uint16 menuID) {
	// FIXME: this is coupled to the mouse click code
	if (_defaultMenu) {
		if (menuID == 0) {
			for (int i = 0; i < _defaultMenu->numberOfMenus(); i++) {
				Graphics::MacMenuSubMenu *item = _defaultMenu->getSubmenu(nullptr, i);
				if (item) {
					item->highlight = false;
				}
			}
		} else {
			Graphics::MacMenuSubMenu *item = _defaultMenu->getSubmenu(nullptr, menuID - 1);
			if (item) {
				item->highlight = true;
			}
		}
		_defaultMenu->setDirty(true);
	}
	debugC(0, kDebugLevelMacToolbox, "STUB: Toolbox::HiliteMenu %d", menuID);
}

uint32 Toolbox::MenuKey(char ch) {
	uint32 result = 0;
	if (_defaultMenu) {
		int menuItem;
		int menuSubItem;
		_defaultMenu->getMenuShortCut((uint8)ch, menuItem, menuSubItem);
		if ((menuItem >= 0) && (menuSubItem >= 0)) {
			result |= ((menuItem + 1) & 0xffff) << 16;
			result |= ((menuSubItem + 1) & 0xffff);
			HiliteMenu(menuItem + 1);
		}
	}
	return result;
}

uint32 Toolbox::MenuSelect(const Common::Point &startPt) {
	uint32 result = 0;
	if (_defaultMenu) {
		if (_defaultMenu->mouseClick(startPt.x, startPt.y)) {
			while (_defaultMenu->_active) {
				Common::Event ev;
				while (g_system->getEventManager()->pollEvent(ev)) {
					_defaultMenu->processEvent(ev);
				}
				_updateScreen();
			}
			// must have released the mouse button to exit the menu
			_modifiers |= kModMouseButtonUp;
			int menuItem = _defaultMenu->getLastSelectedMenuItem();
			int menuSubItem = _defaultMenu->getLastSelectedSubmenuItem();
			if ((menuItem != -1) && (menuSubItem != -1)) {
				Graphics::MacMenuItem *sub = _defaultMenu->getMenuItem(menuItem);
				Graphics::MacMenuItem *item = _defaultMenu->getSubMenuItem(sub, menuSubItem);
				if (item->enabled) {
					result |= ((menuItem + 1) & 0xffff) << 16;
					result |= ((menuSubItem + 1) & 0xffff);
				}
			}
		}
	}
	// we've eaten a ton of events, fix the mouseUp modifier
	_modifiers |= kModMouseButtonUp;
	debugC(0, kDebugLevelMacToolbox, "Toolbox::MenuSelect: %08x", result);
	return result;
}

MenuHandle Toolbox::NewMenu(uint16 menuID, const Common::U32String &menuTitle) {
	MenuHandle handle(new MenuInfo());
	handle->menuID = menuID;
	handle->menuData.push_back(menuTitle);
	_menu[menuID] = handle;
	if (_defaultMenu) {
		for (int i = 0; i < (menuID - _defaultMenu->numberOfMenus()); i++) {
			_defaultMenu->addMenuItem(nullptr, Common::U32String());
		}
		_defaultMenu->clearSubMenu(menuID-1);
		Graphics::MacMenuItem *item = _defaultMenu->getMenuItem(menuID-1);
		if (!item) {
			debugC(0, kDebugLevelMacToolbox, "NewMenu: new menu didn't add for some reason??");
		} else {
			item->text = menuTitle.encode(Common::kMacRoman);
			item->unicode = false;
			_defaultMenu->addSubMenu(nullptr, menuID-1);
		}
	}
	return handle;
}

void Toolbox::SetItem(MenuHandle &theMenu, uint16 item, const Common::U32String &itemString) {
	if (!theMenu) {
		debugC(0, kDebugLevelMacToolbox, "Toolbox::SetItem: empty menu handle");
		return;
	}
	if (theMenu->menuData.size() < (uint)(item + 1)) {
		theMenu->menuData.resize(item + 1);
	}
	theMenu->menuData[item] = itemString;
	if (_defaultMenu) {
		Graphics::MacMenuSubMenu *submenu = _defaultMenu->getSubmenu(nullptr, theMenu->menuID-1);
		if (!submenu) {
			debugC(0, kDebugLevelMacToolbox, "Toolbox::SetItem: menu ID %d not found", theMenu->menuID);
			return;
		}
		if (item == 0) {
			Graphics::MacMenuItem *mi = _defaultMenu->getMenuItem(theMenu->menuID-1);
			mi->text = itemString.encode(Common::kMacRoman);
			mi->unicode = false;
		} else {
			_defaultMenu->removeMenuItem(submenu, item-1);
			_defaultMenu->insertMenuItem(submenu, itemString.encode(Common::kMacRoman), item-1);
		}
	}
}


} // End of namespace MacToolbox
} // End of namespace Graphics
