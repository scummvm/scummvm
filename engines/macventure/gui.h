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

#ifndef MACVENTURE_GUI_H
#define MACVENTURE_GUI_H

#include "graphics/macgui/macwindowmanager.h"

#include "macventure/macventure.h"

namespace MacVenture {

using namespace Graphics::MacGUIConstants;

class Gui {	

public:
	Gui(); 
	~Gui();

	void draw();

private: // Attributes

	Graphics::ManagedSurface _screen;
	Graphics::MacWindowManager _wm;

private: // Methods

	void initGUI();
	void loadBorder(Graphics::MacWindow * target, Common::String filename, bool active);

};

} // End of namespace MacVenture

#endif
