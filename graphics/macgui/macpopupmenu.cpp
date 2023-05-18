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

#include "graphics/macgui/macpopupmenu.h"
#include "graphics/macgui/macwindowmanager.h"

namespace Graphics {

MacPopUp::MacPopUp(int id, const Common::Rect &bounds, MacWindowManager *wm, const char *string) : MacMenu(id, bounds, wm) {
	_menuItemId = addMenuItem(nullptr, "");
	createSubMenuFromString(0, string, 0);
	wm->addMenu(id, this);
	_menuId = id;
}

bool MacPopUp::draw(ManagedSurface *g, bool forceRedraw) {

	if (!_isVisible)
		return false;

	if (_dimensionsDirty)
		calcSubMenuBounds(_items[0]->submenu, _mouseX, _mouseY);

	if (!_contentIsDirty && !forceRedraw)
		return false;
	_contentIsDirty = false;

	_screen.clear(_wm->_colorGreen);
	renderSubmenu(_items[0]->submenu, false);

	if (g)
		g->transBlitFrom(_screen, _wm->_colorGreen);

	if (!(_wm->_mode & kWMModalMenuMode) && g)
		g_system->copyRectToScreen(g->getPixels(), g->pitch, 0, 0, g->w, g->h);

	return true;
}

uint32 MacPopUp::drawAndSelectMenu(int x, int y, int item) {
	_mouseX = x;
	_mouseY = y;

	// If menu is not active, then activate it!
	if (!_active)
		_wm->activateMenu();
	setActive(true);

	_contentIsDirty = true; // Set true to force refresh menu open changes

	// Push our submenu to stack
	_menustack.clear();
	_menustack.push_back(_items[0]->submenu);

	// Display menu and update according to events
	this->draw(_wm->_screen);
	eventLoop();

	// Close menu
	closeMenu();

	int activeSubItem = getLastSelectedSubmenuItem();
	if (activeSubItem == -1)
		return item;

	// Return one indexed item!
	return activeSubItem + 1;
}

Common::String MacPopUp::getItemText(int item) {
	// Convert 1-indexed item to 0 indexed
	item = item - 1;
	MacMenuItem *menu = getMenuItem(_menuItemId);
	MacMenuItem *submenu = getSubMenuItem(menu, item);
	return getName(submenu);
}
} // end of namespace Graphics
