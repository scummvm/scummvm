/* ScummVM - Graphic Adventure Engine
*
* ScummVM is the legal property of its developers, whose names
* are too numerous to list here. Please refer to the COPYRIGHT
* file distributed with this source distribution.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
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

#ifndef BACKENDS_NETWORKING_SDL_NET_CLIENT_H
#define BACKENDS_NETWORKING_SDL_NET_CLIENT_H

#include "common/scummsys.h"
#include "common/str.h"

typedef struct _SDLNet_SocketSet *SDLNet_SocketSet;
typedef struct _TCPsocket *TCPsocket;

namespace Networking {

enum ClientState {
	INVALID,
	READING_HEADERS,
	READ_HEADERS,
	BAD_REQUEST
};

class Client {
	ClientState _state;
	SDLNet_SocketSet _set;
	TCPsocket _socket;
	Common::String _headers;

	void checkIfHeadersEnded();
	void checkIfBadRequest();

public:
	Client();
	Client(SDLNet_SocketSet set, TCPsocket socket);
	virtual ~Client();

	void open(SDLNet_SocketSet set, TCPsocket socket);
	void readHeaders();
	void close();

	ClientState state();
	Common::String headers();
};

} // End of namespace Networking

#endif
