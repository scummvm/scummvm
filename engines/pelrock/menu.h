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
#ifndef PELROCK_MENU_H
#define PELROCK_MENU_H

#include "graphics/screen.h"

#include "pelrock/events.h"
#include "pelrock/resources.h"

namespace Pelrock {

class MenuManager {
public:
    MenuManager(Graphics::Screen *screen, PelrockEventManager *events, ResourceManager *res);
    ~MenuManager();
    void menuLoop();
    void loadMenu();
	byte _mainMenuPalette[768] = {0};
private:
	void checkMouseClick(int x, int y);
    void loadInventoryDescriptions();
    void loadMenuTexts();
    void tearDown();
    Graphics::Screen *_screen = nullptr;
    PelrockEventManager *_events = nullptr;
    ResourceManager *_res = nullptr;
	byte *_mainMenu = nullptr;
    byte *_compositeBuffer = nullptr;
    Common::Array<Common::StringArray> _menuTexts;
	// Temporary
	int _selectedInvIndex = 0;
	int _curInventoryPage = 0;
	Common::StringArray _menuText;
	Common::Array<Common::StringArray> _inventoryDescriptions;
};

} // End of namespace Pelrock
#endif // PELROCK_MENU_H
