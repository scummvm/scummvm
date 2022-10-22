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
#include "backends/networking/enet/host.h"
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
		warning("ENet: ENet library failed to initalize");
		return false;
	}
	debug(1, "ENet: Initalized.");
	_initialized = true;
	return true;
}

Host* ENet::create_host(Common::String address, int port, int numClients, int numChannels, int incBand, int outBand) {
	ENetAddress _address;
	ENetHost *_host;

	enet_address_set_host(&_address, address.c_str());
	_address.port = port;

	_host = enet_host_create(&_address, numClients, numChannels, incBand, outBand);
	if (_host == nullptr) {
		warning("ENet: An error occured when trying to create host with address %s:%d", address.c_str(), port);
		return nullptr;
	}

	return new Host(_host);
}

} // End of namespace Networking
