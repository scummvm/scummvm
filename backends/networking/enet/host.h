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
	/**
	 * A representation of ENetHost.
	 * @param host A pointer to ENetHost given by the createHost function.
	 * @see Networking::ENet::createHost
	*/
	Host(ENetHost *host);
	/**
	 * A representation of ENetHost, connected to a server peer.
	 * @param host A pointer to ENetHost given by the connnectToHost function.
	 * @param serverPeer a pointer to a connected peer given by the connnectToHost function.
	 * @see Networking::ENet::connectToHost
	 */
	Host(ENetHost *host, ENetPeer *serverPeer);
	~Host();

	/**
	 * Services the host which receives or sends pending messages,
	 * intended to be called at the start of a game loop.
	 * @param timeout number of milliseconds that ENet should wait for events.
	 * @retval > 0 if an event occurred within the specified time limit.
	 * @retval 0 if no event occurred.
	 * @retval < 0 on failure.
	 */
	uint8 service(int timeout = 0);

	/**
	 * Connected to a foreign peer.
	 * @param address the address of the peer that will attempt to connect to.
	 * @param port the port number of the peer that will attempt to connect to.
	 * @param timeout specifies the connection timeout in milliseconds, 5 full seconds by default.  Will fail if the given time has passed.
	 * @param numChannels the maximum number of channels allowed; if 0, then this is equivalent to ENET_PROTOCOL_MAXIMUM_CHANNEL_COUNT. This must match with the connecting peer.
	 * @retval true on successful.
	 * @retval false on failure.
	 */
	bool connectPeer(Common::String address, int port, int timeout = 5000, int numChannels = 1);
	/**
	 * Disconnects a specific peer from the host.
	 * @param peerIndex Index of a peer to disconnect.
	 * @note A peer index can be retrieved from the getPeerIndexFromHost function.
	 */
	void disconnectPeer(int peerIndex);

	/**
	 * Creates a packet and sends to a peer.
	 * @param data Contents of the packet's data; the packet's data will remain uninitialized if data is NULL.
	 * @param peerIndex Index of a peer to send the packet to.
	 * @param channel channel on which to send; 0 by default.
	 * @param reliable Indicates that the packet must be received by the target peer and resend attempts will be made until the packet is delivered.
	 * @retval true on successful.
	 * @retval false on failure.
	 * @note Currently, if this object is created by the connectToHost function, data will always get sent to _serverPeer regardless of index given.
	 */
	bool send(const char *data, int peerIndex, int channel = 0, bool reliable = true);
	/**
	 * Sends raw data to an address outside the ENet protocol using its UDP socket directly.
	 * @param address Address to send data to.
	 * @param port Port number to send data to.
	 * @param data The data which will be sent.
	 * @retval true on successful.
	 * @retval false on failure.
	 * @note This sends data as a raw connection-less UDP socket, the same functionaility as a Networking::Socket object, but retains the address and port this host object is using.
	 * @note Useful for hole-punching a peer, so it can connect to it.
	 */
	bool sendRawData(Common::String address, int port, const char *data);

	/**
	 * Gets the host name of a peer that have recently connected, disconnected or received packet from.
	 * @return String containing the host name.
	 * @note service() must be called and returned > 0 for this function to work.
	 */
	Common::String getHost();
	/**
	 * Gets the port number of a peer that have recently connected, disconnected or received packet from.
	 * @return A port number.
	 * @note service() must be called and returned > 0 for this function to work.
	 */
	int getPort();

	/**
	 * Gets an index from a connected peer.
	 * @param host A peer's host name
	 * @param port A peer's port number.
	 * @retval >= 0 containing a peer index if successfully found.
	 * @retval -1 if not found.
	 */
	int getPeerIndexFromHost(Common::String host, int port);

	/**
	 * Gets the data from the most-recently received packet.
	 * @return String containing the packet's data.
	 * @note service() must be called and returned ENET_EVENT_TYPE_RECEIVE (3) for this function to work.
	 */
	Common::String getPacketData();
	/**
	 * Deallocate the packet, must be called upon receiving and finished using the packet's data.
	 */
	void destroyPacket();
private:

	/**
	 * Pointer to ENetHost this object represents.
	 * @see Networking::ENet::createHost
	 * @see Networking::ENet::connectToHost
	 */
	ENetHost *_host;
	/**
	 * A representing server peer connected by the connectToHost function.
	 * @see Networking::ENet::connectToHost
	 */
	ENetPeer *_serverPeer;
	/**
	 * String containing the recent host name connected, disconnected or received from.
	 * @see getHost()
	*/
	Common::String _recentHost;
	/**
	 * String containing the recent host name connected, disconnected or received from.
	 * @see getPort()
	*/
	int _recentPort;
	/**
	 * String containing the recent host name connected, disconnected or received from.
	 * @see getPacketData()
	*/
	ENetPacket *_recentPacket;
};

} // End of namespace Networking

#endif
