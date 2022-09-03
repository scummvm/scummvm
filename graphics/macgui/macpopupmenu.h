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

#ifndef GRAPHICS_MACGUI_MACPOPUPMENU_H
#define GRAPHICS_MACGUI_MACPOPUPMENU_H

#include "graphics/macgui/macmenu.h"
#include "graphics/macgui/macwindowmanager.h"

namespace Common {
class U32String;
class MacResManager;
class PEResources;
}

namespace Graphics {

class MacMenu;

class MacPopUp : public MacMenu {
public:
	MacPopUp(int id, const Common::Rect &bounds, MacWindowManager *wm) : MacMenu(id, bounds, wm) {}
};

} // End of namespace Graphics
#endif
