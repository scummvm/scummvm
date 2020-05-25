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

#include "ultima/ultima4/controllers/read_dir_controller.h"
#include "ultima/ultima4/map/direction.h"
#include "ultima/ultima4/meta_engine.h"

namespace Ultima {
namespace Ultima4 {


ReadDirController::ReadDirController() : WaitableController<Direction>(DIR_NONE) {
}

void ReadDirController::setActive() {
	// Special mode for inputing directions
	MetaEngine::setKeybindingMode(KBMODE_DIRECTION);
}

void ReadDirController::keybinder(KeybindingAction action) {
	switch (action) {
	case KEYBIND_UP:
		_value = DIR_NORTH;
		break;
	case KEYBIND_DOWN:
		_value = DIR_SOUTH;
		break;
	case KEYBIND_LEFT:
		_value = DIR_WEST;
		break;
	case KEYBIND_RIGHT:
		_value = DIR_EAST;
		break;
	case KEYBIND_ESCAPE:
		_value = DIR_NONE;
		doneWaiting();
		break;
	default:
		return;
	}

	doneWaiting();
}

} // End of namespace Ultima4
} // End of namespace Ultima
