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

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "backends/networking/sdl_net/client.h"
#include "common/textconsole.h"
#include <SDL/SDL_net.h>

namespace Networking {

Client::Client(): _state(INVALID), _set(nullptr), _socket(nullptr) {}

Client::Client(SDLNet_SocketSet set, TCPsocket socket): _state(INVALID), _set(nullptr), _socket(nullptr) {
	open(set, socket);
}

Client::~Client() {
	close();
}

void Client::open(SDLNet_SocketSet set, TCPsocket socket) {	
	if (_state != INVALID) close();
	_state = READING_HEADERS;
	_socket = socket;
	_set = set;
	if (set) {
		int numused = SDLNet_TCP_AddSocket(set, socket);
		if (numused == -1) {
			error("SDLNet_AddSocket: %s\n", SDLNet_GetError());
		}
	}
}

void Client::readHeaders() {
	if (!_socket) return;
	if (!SDLNet_SocketReady(_socket)) return;

	const uint32 BUFFER_SIZE = 16 * 1024;
	char buffer[BUFFER_SIZE];	
	int bytes = SDLNet_TCP_Recv(_socket, buffer, BUFFER_SIZE);	
	if (bytes <= 0) {
		warning("Client::readHeaders recv fail");		
		_state = INVALID;
		return;
	}	
	_headers += Common::String(buffer, bytes);	
	checkIfHeadersEnded();
}

void Client::checkIfHeadersEnded() {
	const char *cstr = _headers.c_str();
	const char *position = strstr(cstr, "\r\n\r\n");
	if (position) _state = READ_HEADERS;
}

void Client::close() {
	if (_set) {
		if (_socket) {
			int numused = SDLNet_TCP_DelSocket(_set, _socket);
			if (numused == -1)
				error("SDLNet_DelSocket: %s\n", SDLNet_GetError());
		}
		_set = nullptr;
	}

	if (_socket) {
		SDLNet_TCP_Close(_socket);
		_socket = nullptr;
	}

	_state = INVALID;
}


ClientState Client::state() { return _state; }

Common::String Client::headers() { return _headers; }

} // End of namespace Networking
