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

Client::Client() : _state(INVALID), _set(nullptr), _socket(nullptr), _handler(nullptr) {}

Client::Client(SDLNet_SocketSet set, TCPsocket socket) : _state(INVALID), _set(nullptr), _socket(nullptr), _handler(nullptr) {
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
		close();
		return;
	}
	_headers += Common::String(buffer, bytes);
	checkIfHeadersEnded();
	checkIfBadRequest();
}

void Client::checkIfHeadersEnded() {
	const char *cstr = _headers.c_str();
	const char *position = strstr(cstr, "\r\n\r\n");
	if (position) _state = READ_HEADERS;
}

void Client::checkIfBadRequest() {
	uint32 headersSize = _headers.size();
	bool bad = false;

	const uint32 SUSPICIOUS_HEADERS_SIZE = 128 * 1024;
	if (headersSize > SUSPICIOUS_HEADERS_SIZE) bad = true;

	if (!bad) {
		if (headersSize > 0) {
			const char *cstr = _headers.c_str();
			const char *position = strstr(cstr, "\r\n");
			if (position) { //we have at least one line - and we want the first one
				//"<METHOD> <path> HTTP/<VERSION>\r\n"
				Common::String method, path, http, buf;
				uint32 length = position - cstr;
				if (headersSize > length) headersSize = length;
				for (uint32 i = 0; i < headersSize; ++i) {
					if (_headers[i] != ' ') buf += _headers[i];
					if (_headers[i] == ' ' || i == headersSize - 1) {
						if (method == "") method = buf;
						else if (path == "") path = buf;
						else if (http == "") http = buf;
						else {
							bad = true;
							break;
						}
						buf = "";
					}
				}

				//check that method is supported
				if (method != "GET" && method != "PUT" && method != "POST") bad = true;

				//check that HTTP/<VERSION> is OK
				if (!http.hasPrefix("HTTP/")) bad = true;

				_method = method;
				parsePathQueryAndAnchor(path);
			}
		}
	}

	if (bad) _state = BAD_REQUEST;
}

void Client::parsePathQueryAndAnchor(Common::String path) {
	//<path>[?query][#anchor]
	bool readingPath = true;
	bool readingQuery = false;
	_path = "";
	_query = "";
	_anchor = "";
	for (uint32 i = 0; i < path.size(); ++i) {
		if (readingPath) {
			if (path[i] == '?') {
				readingPath = false;
				readingQuery = true;
			} else _path += path[i];
		} else if(readingQuery) {
			if (path[i] == '#') {
				readingQuery = false;
			} else _query += path[i];
		} else _anchor += path[i];
	}
}

void Client::setHandler(ClientHandler *handler) {	
	if (_handler) delete _handler;
	_state = BEING_HANDLED;
	_handler = handler;
}

void Client::handle() {
	if (_state != BEING_HANDLED) warning("handle() called in a wrong Client's state");
	if (!_handler) warning("Client doesn't have handler to be handled by");
	if (_handler) _handler->handle(this);
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


ClientState Client::state() const { return _state; }

Common::String Client::headers() const { return _headers; }

Common::String Client::method() const { return _method; }

Common::String Client::path() const { return _path; }

Common::String Client::query() const { return _query; }

Common::String Client::queryParameter(Common::String name) const {
	// this approach is a bit slower than searching for the <name>
	// yet I believe it to be the right one, because we probably can have "<name>=" in the value of other key
	Common::String key = "";
	Common::String value = "";
	bool readingKey = true;
	for (uint32 i = 0; i < _query.size(); ++i) {
		if (readingKey) {
			if (_query[i] == '=') {
				readingKey = false;
				value = "";
			} else key += _query[i];
		} else {
			if (_query[i] == '&') {
				if (key == name) return value;
				readingKey = true;
				key = "";
			} else value += _query[i];
		}
	}
	return "";
}

Common::String Client::anchor() const { return _anchor; }

bool Client::socketIsReady() { return SDLNet_SocketReady(_socket); }

int Client::recv(void *data, int maxlen) { return SDLNet_TCP_Recv(_socket, data, maxlen); }

int Client::send(void *data, int len) { return SDLNet_TCP_Send(_socket, data, len); }

} // End of namespace Networking
