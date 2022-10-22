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
#include "backends/networking/enet/host.h"

namespace Networking {

Host::Host(ENetHost *host) {
	_host = host;
	_serverPeer = nullptr;
	_recentEvent = nullptr;
	_recentPacket = nullptr;
}

Host::Host(ENetHost *host, ENetPeer *serverPeer) {
	_host = host;
	_serverPeer = serverPeer;
	_recentEvent = nullptr;
	_recentPacket = nullptr;
}

Host::~Host() {
	if (_recentPacket)
		destroy_packet();
	enet_host_destroy(_host);
}

uint8 Host::service(int timeout) {
	ENetEvent event;
	enet_host_service(_host, &event, timeout);
	_recentEvent = &event;
	if (event.type == ENET_EVENT_TYPE_RECEIVE) {
		if (_recentPacket)
			destroy_packet();
		_recentPacket = event.packet;
	}
	return event.type;
}

Common::String Host::get_host() {
	if (!_recentEvent)
		return "";

	char _hostName[50];
	if (enet_address_get_host_ip(&_recentEvent->peer->address, _hostName, 50) == 0)
		return Common::String(_hostName);
	return "";
}

int Host::get_port() {
	if (!_recentEvent || !_recentEvent->peer)
		return 0;
	return _recentEvent->peer->address.port;
}

void Host::destroy_packet() {
	if (!_recentPacket)
		return;
	enet_packet_destroy(_recentPacket);
	_recentPacket = nullptr;
}
	
} // End of namespace Networking
