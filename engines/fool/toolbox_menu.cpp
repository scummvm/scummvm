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

#include "graphics/macgui/macmenu.h"
#include "graphics/macgui/mactext-canvas.h"
#include "graphics/macgui/mactext.h"
#include "graphics/managed_surface.h"
#include "graphics/pixelformat.h"

#include "fool/fool.h"
#include "fool/toolbox.h"
#include "fool/utils.h"

namespace Fool {

void Toolbox::ClearMenuBar() {
	if (_defaultMenu) {
		_defaultMenu->setVisible(false);
	}
}

void Toolbox::DeleteMenu(uint16 menuID) {
	if (_menu.contains(menuID)) {
		_menu.erase(menuID);
	}
	if (_defaultMenu) {
		_defaultMenu->clearSubMenu(menuID-1);
	}
}

void Toolbox::DrawMenuBar() {
	if (_defaultMenu) {
		_defaultMenu->setVisible(true);
	}
}

void Toolbox::DisposeMenu(MenuHandle &theMenu) {
	// only frees memory, DeleteMenu is responsible for removing it from the menu state
	theMenu = nullptr;
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
	warning("STUB: Toolbox::HiliteMenu");
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
			int menuItem = _defaultMenu->getLastSelectedMenuItem();
			int menuSubItem = _defaultMenu->getLastSelectedSubmenuItem();
			if ((menuItem != -1) && (menuSubItem != -1)) {
				result |= ((menuItem + 1) & 0xffff) << 16;
				result |= ((menuSubItem + 1) & 0xffff);
			}
		}
	}
	warning("Toolbox::MenuSelect: %08x", result);
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
		item->unicodeText = menuTitle;
		item->unicode = true;
		_defaultMenu->addSubMenu(nullptr, menuID-1);
	}
	return handle;
}

void Toolbox::SetItem(MenuHandle &theMenu, uint16 item, const Common::U32String &itemString) {
	if (!theMenu) {
		warning("Toolbox::SetItem: empty menu handle");
		return;
	}
	if (theMenu->menuData.size() < (uint)(item + 1)) {
		theMenu->menuData.resize(item + 1);
	}
	theMenu->menuData[item] = itemString;
	if (_defaultMenu) {
		Graphics::MacMenuSubMenu *submenu = _defaultMenu->getSubmenu(nullptr, theMenu->menuID-1);
		if (!submenu) {
			warning("Toolbox::SetItem: menu ID %d not found", theMenu->menuID);
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


} // namespace Fool
