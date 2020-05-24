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

#include "ultima/ultima4/controllers/read_int_controller.h"

namespace Ultima {
namespace Ultima4 {

ReadIntController::ReadIntController(int maxlen, int screenX, int screenY) :
		ReadStringController(maxlen, screenX, screenY, "0123456789 \n\r\010") {}

int ReadIntController::get(int maxlen, int screenX, int screenY, EventHandler *eh) {
	if (!eh)
		eh = eventHandler;

	ReadIntController ctrl(maxlen, screenX, screenY);
	eh->pushController(&ctrl);
	ctrl.waitFor();
	return ctrl.getInt();
}

int ReadIntController::getInt() const {
	return static_cast<int>(strtol(_value.c_str(), nullptr, 10));
}

} // End of namespace Ultima4
} // End of namespace Ultima
