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

#include "graphics/macgui/mactext-canvas.h"
#include "graphics/macgui/mactext.h"
#include "graphics/managed_surface.h"
#include "graphics/pixelformat.h"

#include "fool/fool.h"
#include "fool/toolbox.h"
#include "fool/utils.h"

namespace Fool {

void Toolbox::ClearMenuBar() {
	warning("STUB: Toolbox::ClearMenuBar");
}

void Toolbox::DeleteMenu(uint16 menuID) {
	warning("STUB: Toolbox::DeleteMenu");
}

void Toolbox::DrawMenuBar() {
	warning("STUB: Toolbox::DrawMenuBar");
}

void Toolbox::DisposeMenu(MenuHandle &theMenu) {
	warning("STUB: Toolbox::DisposeMenu");
}

MenuHandle Toolbox::GetMHandle(uint16 menuID) {
	warning("STUB: Toolbox::GetMHandle");
	return MenuHandle();
}

void Toolbox::HiliteMenu(uint16 menuID) {
	warning("STUB: Toolbox::HiliteMenu");
}

uint32 Toolbox::MenuSelect(const Common::Point &startPt) {
	warning("STUB: Toolbox::MenuSelect");
	return 0;
}


} // namespace Fool
