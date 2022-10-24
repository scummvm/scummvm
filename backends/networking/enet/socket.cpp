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
#include "backends/networking/enet/socket.h"
#include "common/debug.h"

namespace Networking {

Socket::Socket(ENetSocket socket) {
	_socket = socket;
	_recentData = Common::String();
	_recentHost = Common::String();
	_recentPort = 0;
}
Socket::~Socket() {
	enet_socket_destroy(_socket);
}

bool Socket::send(Common::String address, int port, const char *data) {
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

	int sentLength = enet_socket_send(_socket, &_address, &_buffer, 1);
	if (sentLength < 0)
		return false;
	
	return true;
}

bool Socket::receive() {
	ENetBuffer _buffer;

	char data[4096]; // ENET_PROTOCOL_MAXIMUM_MTU
	_buffer.data = data;
	_buffer.dataLength = sizeof(data);

	ENetAddress _address;

	int receivedLength = enet_socket_receive(_socket, &_address, &_buffer, 1);
	if (receivedLength < 0) {
		warning("ENet: An error has occured when receiving data from socket");
		return false;
	}

	if (receivedLength == 0)
		return false;
	
	_recentData = Common::String((const char*)data, receivedLength);

	char _hostName[15];
	if (enet_address_get_host_ip(&_address, _hostName, sizeof(_hostName)) == 0)
		_recentHost = _hostName;
	else
		_recentHost = "";
	_recentPort = _address.port;

	return true;
}

Common::String Socket::get_data() {
	return _recentData;
}

Common::String Socket::get_host() {
	return _recentHost;
}

int Socket::get_port() {
	return _recentPort;
}

} // End of namespace Networking
