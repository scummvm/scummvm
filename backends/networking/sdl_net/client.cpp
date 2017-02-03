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
#include "backends/networking/sdl_net/localwebserver.h"
#include "common/memstream.h"
#include <SDL_net.h>

namespace Networking {

Client::Client():
	_state(INVALID), _set(nullptr), _socket(nullptr), _handler(nullptr),
	_previousHandler(nullptr), _stream(nullptr), _buffer(new byte[CLIENT_BUFFER_SIZE]) {}

Client::Client(SDLNet_SocketSet set, TCPsocket socket):
	_state(INVALID), _set(nullptr), _socket(nullptr), _handler(nullptr),
	_previousHandler(nullptr), _stream(nullptr), _buffer(new byte[CLIENT_BUFFER_SIZE]) {
	open(set, socket);
}

Client::~Client() {
	close();
	delete[] _buffer;
}

void Client::open(SDLNet_SocketSet set, TCPsocket socket) {
	if (_state != INVALID)
		close();
	_state = READING_HEADERS;
	_socket = socket;
	_set = set;
	Reader cleanReader;
	_reader = cleanReader;
	if (_handler)
		delete _handler;
	_handler = nullptr;
	if (_previousHandler)
		delete _previousHandler;
	_previousHandler = nullptr;
	_stream = new Common::MemoryReadWriteStream(DisposeAfterUse::YES);
	if (set) {
		int numused = SDLNet_TCP_AddSocket(set, socket);
		if (numused == -1) {
			error("Client: SDLNet_AddSocket: %s\n", SDLNet_GetError());
		}
	}
}

bool Client::readMoreIfNeeded() {
	if (_stream == nullptr)
		return false; //nothing to read into
	if (_stream->size() - _stream->pos() > 0)
		return true; //not needed, some data left in the stream
	if (!_socket)
		return false;
	if (!SDLNet_SocketReady(_socket))
		return false;

	int bytes = SDLNet_TCP_Recv(_socket, _buffer, CLIENT_BUFFER_SIZE);
	if (bytes <= 0) {
		warning("Client::readMoreIfNeeded: recv fail");
		close();
		return false;
	}

	if (_stream->write(_buffer, bytes) != (uint32)bytes) {
		warning("Client::readMoreIfNeeded: failed to write() into MemoryReadWriteStream");
		close();
		return false;
	}

	return true;
}

void Client::readHeaders() {
	if (!readMoreIfNeeded())
		return;
	_reader.setContent(_stream);
	if (_reader.readFirstHeaders())
		_state = (_reader.badRequest() ? BAD_REQUEST : READ_HEADERS);
}

bool Client::readContent(Common::WriteStream *stream) {
	if (!readMoreIfNeeded())
		return false;
	_reader.setContent(_stream);
	return _reader.readFirstContent(stream);
}

bool Client::readBlockHeaders(Common::WriteStream *stream) {
	if (!readMoreIfNeeded())
		return false;
	_reader.setContent(_stream);
	return _reader.readBlockHeaders(stream);
}

bool Client::readBlockContent(Common::WriteStream *stream) {
	if (!readMoreIfNeeded())
		return false;
	_reader.setContent(_stream);
	return _reader.readBlockContent(stream);
}

void Client::setHandler(ClientHandler *handler) {
	if (_handler) {
		if (_previousHandler)
			delete _previousHandler;
		_previousHandler = _handler; //can't just delete it, as setHandler() could've been called by handler itself
	}
	_state = BEING_HANDLED;
	_handler = handler;
}

void Client::handle() {
	if (_state != BEING_HANDLED)
		warning("handle() called in a wrong Client's state");
	if (!_handler)
		warning("Client doesn't have handler to be handled by");
	if (_handler)
		_handler->handle(this);
}

void Client::close() {
	if (_set) {
		if (_socket) {
			int numused = SDLNet_TCP_DelSocket(_set, _socket);
			if (numused == -1)
				error("Client: SDLNet_DelSocket: %s\n", SDLNet_GetError());
		}
		_set = nullptr;
	}

	if (_socket) {
		SDLNet_TCP_Close(_socket);
		_socket = nullptr;
	}

	if (_stream) {
		delete _stream;
		_stream = nullptr;
	}

	_state = INVALID;
}


ClientState Client::state() const { return _state; }

Common::String Client::headers() const { return _reader.headers(); }

Common::String Client::method() const { return _reader.method(); }

Common::String Client::path() const { return _reader.path(); }

Common::String Client::query() const { return _reader.query(); }

Common::String Client::queryParameter(Common::String name) const { return _reader.queryParameter(name); }

Common::String Client::anchor() const { return _reader.anchor(); }

bool Client::noMoreContent() const { return _reader.noMoreContent(); }

bool Client::socketIsReady() { return SDLNet_SocketReady(_socket); }

int Client::recv(void *data, int maxlen) { return SDLNet_TCP_Recv(_socket, data, maxlen); }

int Client::send(void *data, int len) { return SDLNet_TCP_Send(_socket, data, len); }

} // End of namespace Networking
