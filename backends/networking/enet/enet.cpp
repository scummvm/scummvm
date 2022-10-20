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

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include <enet/enet.h>
#include "backends/networking/enet/enet.h"
#include "common/debug.h"

namespace Networking {

ENet::ENet() {
	_initialized = false;
}

ENet::~ENet() {
	if (_initialized) {
		// Deinitialize the library.
		debug(1, "ENet: Deinitalizing.");
		enet_deinitialize();
	}
}

bool ENet::initalize() {
	if (ENet::_initialized) {
		return true;
	}

	if (enet_initialize() != 0) {
		warning("ENet: ENet library failed to initalize.");
		return false;
	}
	debug(1, "ENet: Initalized.");
	_initialized = true;
	return true;
}

} // End of namespace Networking
