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
	_recentHost = "";
	_recentPort = 0;
	_recentPacket = nullptr;
}

Host::Host(ENetHost *host, ENetPeer *serverPeer) {
	_host = host;
	_serverPeer = serverPeer;
	_recentHost = "";
	_recentPort = 0;
	_recentPacket = nullptr;
}

Host::~Host() {
	if (_recentPacket)
		destroyPacket();
	enet_host_destroy(_host);
}

uint8 Host::service(int timeout) {
	ENetEvent event;
	enet_host_service(_host, &event, timeout);

	if (event.type > ENET_EVENT_TYPE_NONE) {
		char hostName[50];
		if (enet_address_get_host_ip(&event.peer->address, hostName, 50) == 0)
			_recentHost = Common::String(hostName);
		_recentPort = event.peer->address.port;
	}

	if (event.type == ENET_EVENT_TYPE_RECEIVE) {
		if (_recentPacket)
			destroyPacket();
		_recentPacket = event.packet;
	}
	return event.type;
}

bool Host::connectPeer(Common::String address, int port, int timeout, int numChannels) {
	ENetAddress enetAddress;
	if (address == "255.255.255.255") {
		enetAddress.host = ENET_HOST_BROADCAST;
	} else {
		// NOTE: 0.0.0.0 returns ENET_HOST_ANY normally.
		enet_address_set_host(&enetAddress, address.c_str());
	}
	enetAddress.port = port;

	// Connect to server address
	ENetPeer *enetPeer = enet_host_connect(_host, &enetAddress, numChannels, 0);

	ENetEvent event;
	if (enet_host_service(_host, &event, timeout) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
		debug(1, "ENet: Connection to %s:%d succeeded.", address.c_str(), port);
		return true;
	}
	warning("ENet: Connection to %s:%d failed", address.c_str(), port);
	enet_peer_reset(enetPeer);
	return false;
}

void Host::disconnectPeer(int peerIndex) {
	// calling _later will ensure that all the queued packets are sent before disconnecting.
	enet_peer_disconnect_later(&_host->peers[peerIndex], 0);
	enet_host_flush(_host);

	if (_serverPeer) {
		// Allow 3 seconds for disconnection to succeed and drop incoming packets
		uint tickCount = 0;
		while(tickCount < 3000) {
			switch(service(0)) {
			case ENET_EVENT_TYPE_RECEIVE:
				destroyPacket();
				break;
			case ENET_EVENT_TYPE_DISCONNECT:
				// Disconnect succeeded.
				return;
			}
			g_system->delayMillis(5);
			tickCount += 1;
		}
	}
}

Common::String Host::getHost() {
	return _recentHost;
}

int Host::getPort() {
	return _recentPort;
}

int Host::getPeerIndexFromHost(Common::String host, int port) {
	for (int i = 0; i < (int)_host->peerCount; i++) {
		char hostName[50];
		if (enet_address_get_host_ip(&_host->peers[i].address, hostName, 50) == 0) {
			if (host == hostName && port == _host->peers[i].address.port) {
				return i;
			}
		}
	}
	return -1;
}

Common::String Host::getPacketData() {
	if (!_recentPacket)
		return "";
	return Common::String((const char*)_recentPacket->data, (uint32)_recentPacket->dataLength);
}

void Host::destroyPacket() {
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

bool Host::sendRawData(Common::String address, int port, const char *data) {
	ENetAddress enetAddress;
	if (address == "255.255.255.255") {
		enetAddress.host = ENET_HOST_BROADCAST;
	} else {
		// NOTE: 0.0.0.0 returns ENET_HOST_ANY normally.
		enet_address_set_host(&enetAddress, address.c_str());
	}
	enetAddress.port = port;

	ENetBuffer _buffer;
	_buffer.data = const_cast<char *>(data);
	_buffer.dataLength = strlen(data);

	int sentLength = enet_socket_send(_host->socket, &enetAddress, &_buffer, 1);
	if (sentLength < 0)
		return false;

	return true;

}

} // End of namespace Networking
