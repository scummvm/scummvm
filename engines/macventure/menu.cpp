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
*/
#include "macventure/menu.h"
#include "macventure/macventure.h"

#include "common/file.h"
#include "graphics/macgui/macwindowmanager.h"

namespace MacVenture {

Menu::Menu(MacVentureEngine *engine, Graphics::MacWindowManager *wm) {

	_engine = engine;
	_wm = wm;

	_menu = _wm->addMenu();

	if (!loadMenuData())
		error("GUI: Could not load menu data from %s", _engine->getGameFileName());
}

Menu::~Menu() {
	delete _
}

void Menu::draw() {

}

bool Menu::loadMenuData() {

}

} // End of namespace MacVenture