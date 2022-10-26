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

int Host::get_peer_index_from_host(Common::String host, int port) {
	for (int i = 0; i < (int)_host->peerCount; i++) {
		char _hostName[50];
		if (enet_address_get_host_ip(&_host->peers[i].address, _hostName, 50) == 0) {
			if (host == _hostName && port == _host->peers[i].address.port) {
				return i;
			}
		}
	}
	return -1;
}

Common::String Host::get_packet_data() {
	if (!_recentPacket)
		return "";
	return Common::String((const char*)_recentPacket->data, (uint32)_recentPacket->dataLength);
}

void Host::destroy_packet() {
	if (!_recentPacket)
		return;
	enet_packet_destroy(_recentPacket);
	_recentPacket = nullptr;
}

bool Host::send(const char *data, int peerIndex, int channel, bool reliable) {
	ENetPeer *peer;
	if (_serverPeer) {
		peer = _serverPeer;
	} else {
		if (peerIndex > (int)_host->peerCount) {
			warning("ENet: Peer index (%d) is too high", peerIndex);
			return false;
		}
		peer = &_host->peers[peerIndex];
	}

	ENetPacket *packet = enet_packet_create(const_cast<char *>(data), strlen(data), (reliable) ? ENET_PACKET_FLAG_RELIABLE : 0);
	enet_peer_send(peer, channel, packet);

	enet_host_flush(_host);
	return true;
}

bool Host::send_raw_data(Common::String address, int port, const char *data) {
	ENetAddress _address;
	if (address == "255.255.255.255") {
		_address.host = ENET_HOST_BROADCAST;
	} else {
		// NOTE: 0.0.0.0 returns ENET_HOST_ANY normally.
		enet_address_set_host(&_address, address.c_str());
	}
	_address.port = port;

	ENetBuffer _buffer;
	_buffer.data = const_cast<char *>(data);
	_buffer.dataLength = strlen(data);

	int sentLength = enet_socket_send(_host->socket, &_address, &_buffer, 1);
	if (sentLength < 0)
		return false;
	
	return true;

}
	
} // End of namespace Networking
