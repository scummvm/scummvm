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

#ifndef BACKENDS_NETWORKING_ENET_ENET_H
#define BACKENDS_NETWORKING_ENET_ENET_H

#include "common/str.h"

namespace Networking {

class Host;
class Socket;

class ENet {
public:
	/**
	 * The main object that allows for ENet host and socket creation.
	 */
	ENet();
	~ENet();

	/**
	 * Initializes the ENet library.  Must be called first before any other functions.
	 * @return true if successful, false on failure.
	 */
	bool initialize();
	/**
	 * Creates a new ENet Host instance for listening for/connecting to peers.
	 * @param address the address at which other peers may connect to this host.  "0.0.0.0" may be to used to use the default host.
	 * @param port a port number this host will use.
	 * @param numClients the maximum number of peers that should be allocated for the host.
	 * @param numChannels the maximum number of channels allowed; if 0, then this is equivalent to ENET_PROTOCOL_MAXIMUM_CHANNEL_COUNT
	 * @param incBand downstream bandwidth of the host in bytes/second; if 0, ENet will assume unlimited bandwidth.
	 * @param outBand upstream bandwidth of the host in bytes/second; if 0, ENet will assume unlimited bandwidth.
	 * @retval Networking::Host object on success
	 * @retval nullptr on failure
	 * @see Networking::Host
	 */
	Host *createHost(Common::String address, int port, int numClients, int numChannels = 1, int incBand = 0, int outBand = 0);
	/**
	 * Creates a new ENet Host instance, and attempts to connect to the assigned address and port.
	 * @param hostAddress the address this host will use to connect to this peer.  "0.0.0.0" may be to used to use the default host.
	 * @param hostPort a port number this host will use.  If not used, the host will use an allocated port given by the operating system.
	 * @param address the address of the peer that will attempt to connect to.
	 * @param port the port number of the peer that will attempt to connect to.
	 * @param timeout specifies the connection timeout in milliseconds, 5 full seconds by default.  Will fail if the given time has passed.
	 * @param numChannels the maximum number of channels allowed; if 0, then this is equivalent to ENET_PROTOCOL_MAXIMUM_CHANNEL_COUNT. This must match with the connecting peer.
	 * @param incBand downstream bandwidth of the host in bytes/second; if 0, ENet will assume unlimited bandwidth. This must match with the connecting peer.
	 * @param outBand upstream bandwidth of the host in bytes/second; if 0, ENet will assume unlimited bandwidth. This must match with the connecting peer.
	 * @retval Networking::Host object on success
	 * @retval nullptr on failure
	 * @see Networking::Host
	 */
	Host *connectToHost(Common::String hostAddress, int hostPort, Common::String address, int port, int timeout = 5000, int numChannels = 1, int incBand = 0, int outBand = 0);
	Host *connectToHost(Common::String address, int port, int timeout = 5000, int numChannels = 1, int incBand = 0, int outBand = 0);
	/**
	 * Creates a Networking::Socket instance which is a representation of a raw UDP socket.
	 * Useful for and sending and receiving data that is outside the ENet library protocol.
	 * @param address the address this socket will send to and/or receive data from.
	 * @param port the port number this socket will send to and/or receive data from.
	 * @retval Networking::Socket object on success
	 * @retval nullptr on failure
	 * @see Networking::Socket
	 */
	Socket *createSocket(Common::String address, int port);
private:
	/** 
	 * Indicates if the ENet library has successfully initialized or not.
	 * @see initialize()
	 */
	bool _initialized;
};

} // End of namespace Networking


#endif
