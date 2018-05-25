/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/stark/ui/menu/saveloadmenu.h"

namespace Stark {

SaveLoadMenuScreen::SaveLoadMenuScreen(Gfx::Driver *gfx, Cursor *cursor) :
		StaticLocationScreen(gfx, cursor, "LoadSaveLocation", Screen::kScreenSaveLoadMenu) {
}

SaveLoadMenuScreen::~SaveLoadMenuScreen() {
}

void SaveLoadMenuScreen::open() {
	StaticLocationScreen::open();

	_widgets.push_back(new StaticLocationWidget(
			"loadsavebg",
			nullptr,
			nullptr));
	
	_widgets.push_back(new StaticLocationWidget(
			"Cancel",
			nullptr,
			nullptr));
	
	_widgets.push_back(new StaticLocationWidget(
			"back to index",
			nullptr,
			nullptr));
	
	_widgets.push_back(new StaticLocationWidget(
			"SaveText",
			nullptr,
			nullptr));
	
	_widgets.push_back(new StaticLocationWidget(
			"LoadText",
			nullptr,
			nullptr));
	
	_widgets.push_back(new StaticLocationWidget(
			"Back",
			nullptr,
			nullptr));
	
	_widgets.push_back(new StaticLocationWidget(
			"Next",
			nullptr,
			nullptr));
	
}

} // End of namespace Stark
