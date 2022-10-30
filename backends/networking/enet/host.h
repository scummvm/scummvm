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

#ifndef BACKENDS_NETWORKING_ENET_HOST_H
#define BACKENDS_NETWORKING_ENET_HOST_H

typedef struct _ENetHost ENetHost;
typedef struct _ENetPeer ENetPeer;

typedef struct _ENetEvent ENetEvent;
typedef struct _ENetPacket ENetPacket;


// Event types
#define ENET_EVENT_TYPE_NONE 0
#define ENET_EVENT_TYPE_CONNECT 1
#define ENET_EVENT_TYPE_DISCONNECT 2
#define ENET_EVENT_TYPE_RECEIVE 3

#include "common/system.h"
#include "common/str.h"
#include "common/debug.h"

namespace Networking {

class Host {
public:
	Host(ENetHost *host);
	Host(ENetHost *host, ENetPeer *serverPeer);
	~Host();

	uint8 service(int timeout = 0);

	void disconnectPeer(int peerIndex);

	bool send(const char *data, int peerIndex, int channel = 0, bool reliable = true);
	bool sendRawData(Common::String address, int port, const char *data);

	Common::String getHost();
	int getPort();

	int getPeerIndexFromHost(Common::String host, int port);

	Common::String getPacketData();
	void destroyPacket();
private:
	ENetHost *_host;
	ENetPeer *_serverPeer; // Only used for clients.
	ENetEvent *_recentEvent;
	ENetPacket *_recentPacket;


};
	
} // End of namespace Networking

#endif
