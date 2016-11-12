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

#include "backends/networking/sdl_net/reader.h"
#include "common/str.h"

namespace Common {
class MemoryReadWriteStream;
}

typedef struct _SDLNet_SocketSet *SDLNet_SocketSet;
typedef struct _TCPsocket *TCPsocket;

namespace Networking {

enum ClientState {
	INVALID,
	READING_HEADERS,
	READ_HEADERS,
	BAD_REQUEST,
	BEING_HANDLED
};

class Client;

#define CLIENT_BUFFER_SIZE 1 * 1024 * 1024

class ClientHandler {
public:
	virtual ~ClientHandler() {};
	virtual void handle(Client *client) = 0;
};

/**
 * Client class represents one client's HTTP request
 * to the LocalWebserver.
 *
 * While in READING_HEADERS state, it's kept in LocalWebserver.
 * Client must read the headers and decide whether it's
 * READ_HEADERS (could be handled) or BAD_REQUEST (failed).
 *
 * If it's READ_HEADERS, LocalWebserver searches for a corresponding
 * BaseHandler. These classes use the information from headers -
 * like method, path, GET parameters - to build the response
 * for this client's request. When they do, they call setHandler()
 * and pass a special ClientHandler. Client becomes BEING_HANDLED.
 *
 * While in that state, LocalWebserver calls Client's handle() and
 * it's passed to ClientHandler. The latter does the job: it commands
 * Client to read or write bytes with its socket or calls
 * readContent() methods, so Client reads the request through Reader.
 */

class Client {
	ClientState _state;
	SDLNet_SocketSet _set;
	TCPsocket _socket;
	Reader _reader;
	ClientHandler *_handler, *_previousHandler;
	Common::MemoryReadWriteStream *_stream;
	byte *_buffer;

	bool readMoreIfNeeded();

public:
	Client();
	Client(SDLNet_SocketSet set, TCPsocket socket);
	virtual ~Client();

	void open(SDLNet_SocketSet set, TCPsocket socket);
	void readHeaders();
	bool readContent(Common::WriteStream *stream);
	bool readBlockHeaders(Common::WriteStream *stream);
	bool readBlockContent(Common::WriteStream *stream);
	void setHandler(ClientHandler *handler);
	void handle();
	void close();

	ClientState state() const;
	Common::String headers() const;
	Common::String method() const;
	Common::String path() const;
	Common::String query() const;
	Common::String queryParameter(Common::String name) const;
	Common::String anchor() const;

	bool noMoreContent() const;

	/**
	 * Return SDLNet_SocketReady(_socket).
	 *
	 * It's "ready" when it has something
	 * to read (recv()). You can send()
	 * when this is false.
	 */
	bool socketIsReady();
	int recv(void *data, int maxlen);
	int send(void *data, int len);
};

} // End of namespace Networking

#endif
