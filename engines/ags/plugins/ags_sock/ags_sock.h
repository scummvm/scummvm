/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or(at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef AGS_PLUGINS_AGS_SOCK_H
#define AGS_PLUGINS_AGS_SOCK_H

#include "ags/plugins/plugin_base.h"

namespace AGS3 {
namespace Plugins {
namespace AGSSock {

class AGSSock : public PluginBase {
private:
	static IAGSEngine *_engine;
	static Common::String *_text;
private:
	static const char *AGS_GetPluginName();
	static void AGS_EngineStartup(IAGSEngine *engine);

	/**
	 * Creates a new data container with specified size
	 * (and what character to fill it with)
	 */
	static void SockData_Create(ScriptMethodParams &params);

	/**
	 * Creates a new data container of zero size
	 */
	static void SockData_CreateEmpty(ScriptMethodParams &params);

	/**
	 * Creates a data container from a string
	 */
	static void SockData_CreateFromString(ScriptMethodParams &params);

	/**
	 * Gets the size property
	 */
	static void SockData_get_Size(ScriptMethodParams &params);

	/**
	 * Sets the size property
	 */
	static void SockData_set_Size(ScriptMethodParams &params);

	/**
	 * Gets the chars array
	 */
	static void SockData_geti_Chars(ScriptMethodParams &params);

	/**
	 * Sets the chars array
	 */
	static void SockData_seti_Chars(ScriptMethodParams &params);

	/**
	 * Makes and returns a string from the data object.
	 * (Warning: anything after a null character will be truncated)
	 */
	static void SockData_AsString(ScriptMethodParams &params);

	/**
	 * Removes all the data from a socket data object,
	 * reducing it's size to zero
	 */
	static void SockData_Clear(ScriptMethodParams &params);

	/**
	 * Creates an empty socket address. (advanced: set type
	 * to IPv6 if you're using IPv6)
	 */
	static void SockAddr_Create(ScriptMethodParams &params);

	/**
	 * Creates a socket address from a string.
	 * (for example: "https://www.scummvm.org/"
	 */
	static void SockAddr_CreateFromString(ScriptMethodParams &params);

	/**
	 * Creates a socket address from raw data. (advanced)
	 */
	static void SockAddr_CreateFromData(ScriptMethodParams &params);

	/**
	 * Creates a socket address from an IP-address.
	 * (for example: "127.0.0.1")
	 */
	static void SockAddr_CreateIP(ScriptMethodParams &params);

	/**
	 * Creates a socket address from an IPv6-address.
	 * (for example: "::1")
	 */
	static void SockAddr_CreateIPv6(ScriptMethodParams &params);

	/**
	 * Gets the value of the port property
	 */
	static void SockAddr_get_Port(ScriptMethodParams &params);

	/**
	 * Sets the value of the port property
	 */
	static void SockAddr_set_Port(ScriptMethodParams &params);

	/**
	 * Gets the value of the address property
	 */
	static void SockAddr_get_Address(ScriptMethodParams &params);

	/**
	 * Sets the value of the address property
	 */
	static void SockAddr_set_Address(ScriptMethodParams &params);

	/**
	 * Gets the value of the IP property
	 */
	static void SockAddr_get_IP(ScriptMethodParams &params);

	/**
	 * Sets the value of the IP property
	 */
	static void SockAddr_set_IP(ScriptMethodParams &params);

	/**
	 * Returns a SockData object that contains the raw data
	 * of the socket address. (advanced)
	 */
	static void SockAddr_GetData(ScriptMethodParams &params);

	/**
	 * Creates a socket for the specified protocol. (advanced)
	 */
	static void Socket_Create(ScriptMethodParams &params);

	/**
	 * Creates a UDP socket. (unrealiable, connectionless, message based)
	 */
	static void Socket_CreateUDP(ScriptMethodParams &params);

	/**
	 * Creates a TCP socket. (reliable, connection based, streaming)
	 */
	static void Socket_CreateTCP(ScriptMethodParams &params);

	/**
	 * Creates a UDP socket for IPv6. (when in doubt use CreateUDP)
	 */
	static void Socket_CreateUDPv6(ScriptMethodParams &params);

	/**
	 * Creates a TCP socket for IPv6. (when in doubt use CreateTCP)
	 */
	static void Socket_CreateTCPv6(ScriptMethodParams &params);

	/**
	 * Gets the value of the Tag property
	 */
	static void Socket_get_Tag(ScriptMethodParams &params);

	/**
	 * Sets the value of the Tag property
	 */
	static void Socket_set_Tag(ScriptMethodParams &params);

	/**
	 * Gets the value of the Local property
	 */
	static void Socket_get_Local(ScriptMethodParams &params);

	/**
	 * Gets the value of the Remote property
	 */
	static void Socket_get_Remote(ScriptMethodParams &params);

	/**
	 * Gets the value of the Value property
	 */
	static void Socket_get_Valid(ScriptMethodParams &params);

	/**
	 * Returns the last error observed from this socket
	 * as an human readable string
	 */
	static void Socket_ErrorString(ScriptMethodParams &params);

	/**
	 * Binds the socket to a local address. (generally used
	 * before listening)
	 */
	static void Socket_Bind(ScriptMethodParams &params);

	/**
	 * Makes a socket listen for incoming connection requests.
	 * (TCP only) Backlog specifies how many requests can be
	 * queued. (optional)
	 */
	static void Socket_Listen(ScriptMethodParams &params);

	/**
	 * Makes a socket connect to a remote host. (for UDP it
	 * will simply bind to a remote address) Defaults to sync
	 * which makes it wait; see the manual for async use.
	 */
	static void Socket_Connect(ScriptMethodParams &params);

	/**
	 * Accepts a connection request and returns the resulting
	 * socket when successful. (TCP only)
	 */
	static void Socket_Accept(ScriptMethodParams &params);

	/**
	 * Closes the socket. (you can still receive until socket
	 * is marked invalid
	 */
	static void Socket_Close(ScriptMethodParams &params);

	/**
	 * Sends a string to the remote host. Returns whether
	 * successful. (no error means: try again later)
	 */
	static void Socket_Send(ScriptMethodParams &params);

	/**
	 * Sends a string to the specified remote host. (UDP only)
	 */
	static void Socket_SendTo(ScriptMethodParams &params);

	/**
	 * Receives a string from the remote host. (no error
	 * means: try again later)
	 */
	static void Socket_Recv(ScriptMethodParams &params);

	/**
	 * Receives a string from an unspecified host. The given
	 * address object will contain the remote address. (UDP only)
	 */
	static void Socket_RecvFrom(ScriptMethodParams &params);

	/**
	 * Sends raw data to the remote host. Returns whether
	 * successful. (no error means: try again later
	 */
	static void Socket_SendData(ScriptMethodParams &params);

	/**
	 * Sends raw data to the specified remote host. (UDP only)
	 */
	static void Socket_SendDataTo(ScriptMethodParams &params);

	/**
	 * Receives raw data from the remote host. (no error
	 * means: try again later
	 */
	static void Socket_RecvData(ScriptMethodParams &params);

	/**
	 * Receives raw data from an unspecified host. The given
	 * address object will contain the remote address. (UDP only)
	 */
	static void Socket_RecvDataFrom(ScriptMethodParams &params);

	/**
	 * Gets a socket option. (advanced)
	 */
	static void Socket_GetOption(ScriptMethodParams &params);

	/**
	 * Sets a socket option. (advanced)
	 */
	static void Socket_SetOption(ScriptMethodParams &params);

public:
	AGSSock();
};

} // namespace AGSSock
} // namespace Plugins
} // namespace AGS3

#endif
