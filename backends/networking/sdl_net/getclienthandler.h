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

#ifndef BACKENDS_NETWORKING_SDL_NET_GETCLIENTHANDLER_H
#define BACKENDS_NETWORKING_SDL_NET_GETCLIENTHANDLER_H

#include "backends/networking/sdl_net/client.h"
#include "common/hashmap.h"
#include "common/stream.h"
#include "common/hash-str.h"

namespace Networking {

#define CLIENT_HANDLER_BUFFER_SIZE 1 * 1024 * 1024

class GetClientHandler: public ClientHandler {
	Common::HashMap<Common::String, Common::String> _specialHeaders;
	long _responseCode;
	bool _headersPrepared;
	Common::String _headers;
	Common::SeekableReadStream *_stream;
	byte *_buffer;

	static const char *responseMessage(long responseCode);
	void prepareHeaders();

public:
	GetClientHandler(Common::SeekableReadStream *stream);
	virtual ~GetClientHandler();

	virtual void handle(Client *client);
	void setHeader(Common::String name, Common::String value);
	void setResponseCode(long code);
};

} // End of namespace Networking

#endif
