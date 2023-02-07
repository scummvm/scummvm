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
#ifndef BACKENDS_NETWORKING_CURL_SOCKET_H
#define BACKENDS_NETWORKING_CURL_SOCKET_H

typedef void CURL;
#ifdef WIN32
// Including winsock2.h will result in errors, we have to define
// SOCKET ourselves.
#include <basetsd.h>
typedef UINT_PTR SOCKET;

typedef SOCKET curl_socket_t;
#else
typedef int curl_socket_t;
#endif

#include "common/str.h"

namespace Networking {

class CurlSocket {
public:
	CurlSocket();
	~CurlSocket();

	bool connect(Common::String url);

	int ready();

	size_t send(const char *data, int len);
	size_t recv(void *data, int maxLen);
private:
	CURL *_easy;
	curl_socket_t _socket;
};

} // End of namespace Networking



#endif