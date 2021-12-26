/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * of the License, or(at your option) any later version.
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

#ifndef AGS_PLUGINS_AGS_SOCK_H
#define AGS_PLUGINS_AGS_SOCK_H

#include "ags/plugins/ags_plugin.h"

namespace AGS3 {
namespace Plugins {
namespace AGSSock {

class AGSSock : public PluginBase {
	SCRIPT_HASH(AGSSock)
private:
	Common::String _text;

	/**
	 * Creates a new data container with specified size
	 * (and what character to fill it with)
	 */
	void SockData_Create(ScriptMethodParams &params);

	/**
	 * Creates a new data container of zero size
	 */
	void SockData_CreateEmpty(ScriptMethodParams &params);

	/**
	 * Creates a data container from a string
	 */
	void SockData_CreateFromString(ScriptMethodParams &params);

	/**
	 * Gets the size property
	 */
	void SockData_get_Size(ScriptMethodParams &params);

	/**
	 * Sets the size property
	 */
	void SockData_set_Size(ScriptMethodParams &params);

	/**
	 * Gets the chars array
	 */
	void SockData_geti_Chars(ScriptMethodParams &params);

	/**
	 * Sets the chars array
	 */
	void SockData_seti_Chars(ScriptMethodParams &params);

	/**
	 * Makes and returns a string from the data object.
	 * (Warning: anything after a null character will be truncated)
	 */
	void SockData_AsString(ScriptMethodParams &params);

	/**
	 * Removes all the data from a socket data object,
	 * reducing it's size to zero
	 */
	void SockData_Clear(ScriptMethodParams &params);

	/**
	 * Creates an empty socket address. (advanced: set type
	 * to IPv6 if you're using IPv6)
	 */
	void SockAddr_Create(ScriptMethodParams &params);

	/**
	 * Creates a socket address from a string.
	 * (for example: "https://www.scummvm.org/"
	 */
	void SockAddr_CreateFromString(ScriptMethodParams &params);

	/**
	 * Creates a socket address from raw data. (advanced)
	 */
	void SockAddr_CreateFromData(ScriptMethodParams &params);

	/**
	 * Creates a socket address from an IP-address.
	 * (for example: "127.0.0.1")
	 */
	void SockAddr_CreateIP(ScriptMethodParams &params);

	/**
	 * Creates a socket address from an IPv6-address.
	 * (for example: "::1")
	 */
	void SockAddr_CreateIPv6(ScriptMethodParams &params);

	/**
	 * Gets the value of the port property
	 */
	void SockAddr_get_Port(ScriptMethodParams &params);

	/**
	 * Sets the value of the port property
	 */
	void SockAddr_set_Port(ScriptMethodParams &params);

	/**
	 * Gets the value of the address property
	 */
	void SockAddr_get_Address(ScriptMethodParams &params);

	/**
	 * Sets the value of the address property
	 */
	void SockAddr_set_Address(ScriptMethodParams &params);

	/**
	 * Gets the value of the IP property
	 */
	void SockAddr_get_IP(ScriptMethodParams &params);

	/**
	 * Sets the value of the IP property
	 */
	void SockAddr_set_IP(ScriptMethodParams &params);

	/**
	 * Returns a SockData object that contains the raw data
	 * of the socket address. (advanced)
	 */
	void SockAddr_GetData(ScriptMethodParams &params);

	/**
	 * Creates a socket for the specified protocol. (advanced)
	 */
	void Socket_Create(ScriptMethodParams &params);

	/**
	 * Creates a UDP socket. (unrealiable, connectionless, message based)
	 */
	void Socket_CreateUDP(ScriptMethodParams &params);

	/**
	 * Creates a TCP socket. (reliable, connection based, streaming)
	 */
	void Socket_CreateTCP(ScriptMethodParams &params);

	/**
	 * Creates a UDP socket for IPv6. (when in doubt use CreateUDP)
	 */
	void Socket_CreateUDPv6(ScriptMethodParams &params);

	/**
	 * Creates a TCP socket for IPv6. (when in doubt use CreateTCP)
	 */
	void Socket_CreateTCPv6(ScriptMethodParams &params);

	/**
	 * Gets the value of the Tag property
	 */
	void Socket_get_Tag(ScriptMethodParams &params);

	/**
	 * Sets the value of the Tag property
	 */
	void Socket_set_Tag(ScriptMethodParams &params);

	/**
	 * Gets the value of the Local property
	 */
	void Socket_get_Local(ScriptMethodParams &params);

	/**
	 * Gets the value of the Remote property
	 */
	void Socket_get_Remote(ScriptMethodParams &params);

	/**
	 * Gets the value of the Value property
	 */
	void Socket_get_Valid(ScriptMethodParams &params);

	/**
	 * Returns the last error observed from this socket
	 * as an human readable string
	 */
	void Socket_ErrorString(ScriptMethodParams &params);

	/**
	 * Binds the socket to a local address. (generally used
	 * before listening)
	 */
	void Socket_Bind(ScriptMethodParams &params);

	/**
	 * Makes a socket listen for incoming connection requests.
	 * (TCP only) Backlog specifies how many requests can be
	 * queued. (optional)
	 */
	void Socket_Listen(ScriptMethodParams &params);

	/**
	 * Makes a socket connect to a remote host. (for UDP it
	 * will simply bind to a remote address) Defaults to sync
	 * which makes it wait; see the manual for async use.
	 */
	void Socket_Connect(ScriptMethodParams &params);

	/**
	 * Accepts a connection request and returns the resulting
	 * socket when successful. (TCP only)
	 */
	void Socket_Accept(ScriptMethodParams &params);

	/**
	 * Closes the socket. (you can still receive until socket
	 * is marked invalid
	 */
	void Socket_Close(ScriptMethodParams &params);

	/**
	 * Sends a string to the remote host. Returns whether
	 * successful. (no error means: try again later)
	 */
	void Socket_Send(ScriptMethodParams &params);

	/**
	 * Sends a string to the specified remote host. (UDP only)
	 */
	void Socket_SendTo(ScriptMethodParams &params);

	/**
	 * Receives a string from the remote host. (no error
	 * means: try again later)
	 */
	void Socket_Recv(ScriptMethodParams &params);

	/**
	 * Receives a string from an unspecified host. The given
	 * address object will contain the remote address. (UDP only)
	 */
	void Socket_RecvFrom(ScriptMethodParams &params);

	/**
	 * Sends raw data to the remote host. Returns whether
	 * successful. (no error means: try again later
	 */
	void Socket_SendData(ScriptMethodParams &params);

	/**
	 * Sends raw data to the specified remote host. (UDP only)
	 */
	void Socket_SendDataTo(ScriptMethodParams &params);

	/**
	 * Receives raw data from the remote host. (no error
	 * means: try again later
	 */
	void Socket_RecvData(ScriptMethodParams &params);

	/**
	 * Receives raw data from an unspecified host. The given
	 * address object will contain the remote address. (UDP only)
	 */
	void Socket_RecvDataFrom(ScriptMethodParams &params);

	/**
	 * Gets a socket option. (advanced)
	 */
	void Socket_GetOption(ScriptMethodParams &params);

	/**
	 * Sets a socket option. (advanced)
	 */
	void Socket_SetOption(ScriptMethodParams &params);

public:
	AGSSock() : PluginBase() {}
	virtual ~AGSSock() {}

	const char *AGS_GetPluginName() override;
	void AGS_EngineStartup(IAGSEngine *engine) override;

};

} // namespace AGSSock
} // namespace Plugins
} // namespace AGS3

#endif
