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

#ifndef BACKENDS_NETWORKING_ENET_SOCKET_H
#define BACKENDS_NETWORKING_ENET_SOCKET_H

#ifdef WIN32
// Including winsock2.h will result in errors, we have to define
// SOCKET ourselves.
#include <basetsd.h>
typedef UINT_PTR SOCKET;

typedef SOCKET ENetSocket;
#else
typedef int ENetSocket;
#endif

#include "common/str.h"

namespace Networking {

class Socket {
public:
	/**
	 * A representation of a raw UDP socket.
	 * @param socket Contains the socket itself.
	 */
	Socket(ENetSocket socket);
	~Socket();

	/**
	 * Send data to the specified address and port.
	 * @param address Address to send data to.
	 * @param port Port number to send data to.
	 * @param data The data which will be sent.
	 * @retval true on successful.
	 * @retval false on failure.
	 */
	bool send(Common::String address, int port, const char *data);
	/**
	 * Checks for received data.
	 * @retval true if received data.
	 * @retval false otherwise.
	 */
	bool receive();

	/**
	 * Get the data received from socket.
	 * @return String containing received data.
	 * @note receive() must be called and returned true to get actual data.
	 */
	Common::String getData();

	/**
	 * Get the host name of received data.
	 * @return The host name
	 * @note receive() must be called and returned true to get host name.
	 */
	Common::String getHost();
	/**
	 * Get the port number of received data.
	 * @return The port number
	 * @note receive() must be called and returned true to get port.
	 */
	int getPort();
private:
	/**
	 * Representation of the UDP socket.
	 */
	ENetSocket _socket;
	/**
	 * String containing the recent data received, 
	 * @see getData()
	 */
	Common::String _recentData;
	/**
	 * String containing the last received host.
	 * @see getHost()
	*/
	Common::String _recentHost;
	/**
	 * The last last received port number
	 * @see getPort()
	*/
	int _recentPort;
};

} // End of namespace Networking

#endif
