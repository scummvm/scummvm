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
#include "backends/networking/enet/socket.h"
#include "common/debug.h"

namespace Networking {

ENet::ENet() {
	_initialized = false;
}

ENet::~ENet() {
	if (_initialized) {
		// Deinitialize the library.
		debug(1, "ENet: Deinitializing.");
		enet_deinitialize();
	}
}

bool ENet::initialize() {
	if (ENet::_initialized) {
		return true;
	}

	if (enet_initialize() != 0) {
		warning("ENet: ENet library failed to initialize");
		return false;
	}
	debug(1, "ENet: Initialized.");
	_initialized = true;
	return true;
}

Host *ENet::createHost(Common::String address, int port, int numClients, int numChannels, int incBand, int outBand) {
	ENetAddress enetAddress;
	// NOTE: 0.0.0.0 returns ENET_HOST_ANY normally.
	enet_address_set_host(&enetAddress, address.c_str());
	enetAddress.port = port;

	ENetHost *_host = enet_host_create(&enetAddress, numClients, numChannels, incBand, outBand);
	if (_host == nullptr) {
		warning("ENet: An error occured when trying to create host with address %s:%d", address.c_str(), port);
		return nullptr;
	}

	return new Host(_host);
}

Host *ENet::connectToHost(Common::String hostAddress, int hostPort, Common::String address, int port, int timeout, int numChannels, int incBand, int outBand) {
	ENetAddress enetHostAddress;
	// NOTE: 0.0.0.0 returns ENET_HOST_ANY normally.
	enet_address_set_host(&enetHostAddress, hostAddress.c_str());
	enetHostAddress.port = hostPort;

	// NOTE: Number of channels must match with the server's.
	ENetHost *enetHost = enet_host_create(&enetHostAddress, 1, numChannels, incBand, outBand);
	if (enetHost == nullptr) {
		warning("ENet: An error occured when trying to create client host");
		return nullptr;
	}

	ENetAddress enetAddress;
	if (address == "255.255.255.255") {
		enetAddress.host = ENET_HOST_BROADCAST;
	} else {
		// NOTE: 0.0.0.0 returns ENET_HOST_ANY normally.
		enet_address_set_host(&enetAddress, address.c_str());
	}
	enetAddress.port = port;

	// Connect to server address
	ENetPeer *enetPeer = enet_host_connect(enetHost, &enetAddress, numChannels, 0);

	ENetEvent event;
	if (enet_host_service(enetHost, &event, timeout) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
		debug(1, "ENet: Connection to %s:%d succeeded.", address.c_str(), port);
		return new Host(enetHost, enetPeer);
	}
	warning("ENet: Connection to %s:%d failed", address.c_str(), port);
	enet_peer_reset(enetPeer);
	enet_host_destroy(enetHost);
	return nullptr;
}

Host *ENet::connectToHost(Common::String address, int port, int timeout, int numChannels, int incBand, int outBand) {
	return connectToHost("0.0.0.0", 0, address, port, timeout, numChannels, incBand, outBand);
}

Socket *ENet::createSocket(Common::String address, int port) {
	ENetAddress enetAddress;
	if (address == "255.255.255.255") {
		enetAddress.host = ENET_HOST_BROADCAST;
	} else {
		// NOTE: 0.0.0.0 returns ENET_HOST_ANY normally.
		enet_address_set_host(&enetAddress, address.c_str());
	}
	enetAddress.port = port;

	ENetSocket enetSocket = enet_socket_create(ENET_SOCKET_TYPE_DATAGRAM);
	if (enetSocket == ENET_SOCKET_NULL) {
		warning("ENet: Unable to create socket");
		return nullptr;
	}
	if (enet_socket_bind(enetSocket, &enetAddress) < 0) {
		warning("ENet: Unable to bind socket to address %s:%d", address.c_str(), port);
		enet_socket_destroy(enetSocket);
		return nullptr;
	}

	enet_socket_set_option (enetSocket, ENET_SOCKOPT_NONBLOCK, 1);
    enet_socket_set_option (enetSocket, ENET_SOCKOPT_BROADCAST, 1);
	enet_socket_set_option (enetSocket, ENET_SOCKOPT_RCVBUF, ENET_HOST_RECEIVE_BUFFER_SIZE);
    enet_socket_set_option (enetSocket, ENET_SOCKOPT_SNDBUF, ENET_HOST_SEND_BUFFER_SIZE);

	return new Socket(enetSocket);
}

} // End of namespace Networking
