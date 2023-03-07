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

#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "backends/networking/curl/socket.h"
#include "common/debug.h"
#include "common/system.h"
#include <curl/curl.h>

// Auxiliary function that waits on the socket.
// From https://github.com/curl/curl/blob/master/docs/examples/sendrecv.c
static int waitOnSocket(curl_socket_t sockfd, int for_recv, long timeout_ms) {
	struct timeval tv {};
	fd_set infd {}, outfd {}, errfd {};
	int res;

	tv.tv_sec = timeout_ms / 1000;
	tv.tv_usec = (timeout_ms % 1000) * 1000;

	FD_ZERO(&infd);
	FD_ZERO(&outfd);
	FD_ZERO(&errfd);

	FD_SET(sockfd, &errfd); /* always check for error */

	if(for_recv) {
		FD_SET(sockfd, &infd);
	} else {
		FD_SET(sockfd, &outfd);
	}

	/* select() returns the number of signalled sockets or -1 */
	res = select((int)sockfd + 1, &infd, &outfd, &errfd, &tv);
	return res;
}

namespace Networking {

CurlSocket::CurlSocket() {
	_easy = nullptr;
	_socket = 0;
}

CurlSocket::~CurlSocket() {
	// Always clean up.
	curl_easy_cleanup(_easy);
}

int CurlSocket::ready() {
	return waitOnSocket(_socket, 1, 0);
}

bool CurlSocket::connect(Common::String url) {
	_easy = curl_easy_init();
	if (_easy) {
		curl_easy_setopt(_easy, CURLOPT_URL, url.c_str());
		// Just connect to the host, do not do any transfers.
		curl_easy_setopt(_easy, CURLOPT_CONNECT_ONLY, 1L);

		// Uncomment this to disable SSL certificate verification
		// (e.g. self-signed certs).
		// curl_easy_setopt(_easy, CURLOPT_SSL_VERIFYPEER, 0L);

		CURLcode res = curl_easy_perform(_easy);
		if (res != CURLE_OK) {
			warning("libcurl: Failed to connect: %s", curl_easy_strerror(res));
			return false;
		}

		// Get the socket, we'll need it for waiting.
		// NB: Do not use CURL_AT_LEAST_VERSION(x,y,z) or CURL_VERSION_BITS(x,y,z) for version check
		//     These were only added around v7.45.0 release so break build with earlier libcurl versions
#if LIBCURL_VERSION_NUM >= 0x072d00 // 7.45.0
		// Use new CURLINFO_ACTIVESOCKET API for libcurl v7.45.0 or greater
		res = curl_easy_getinfo(_easy, CURLINFO_ACTIVESOCKET, &_socket);
		if (res == CURLE_OK) {
			return true;
		}
#else
		// Fallback on old deprecated CURLINFO_LASTSOCKET API for libcurl older than v7.45.0 (October 2015)
		long socket;
		res = curl_easy_getinfo(_easy, CURLINFO_LASTSOCKET, &socket);
		if (res == CURLE_OK) {
			// curl_socket_t is an int or a SOCKET (Win32) which is a UINT_PTR
			// A cast should be safe enough as long fits in it
			_socket = (curl_socket_t)socket;
			return true;
		}
#endif

		warning("libcurl: Failed to extract socket: %s", curl_easy_strerror(res));
		return false;
	}
	return false;
}

size_t CurlSocket::send(const char *data, int len) {
	if (!_socket)
		return -1;
    size_t nsent_total = 0, left = len;
    CURLcode res = CURLE_AGAIN;

	// Keep looping until the whole thing is sent, errors,
	// or times out.
	while (((left > 0) && (len > 0))) {
		size_t nsent = 0;
		uint32 tickCount = g_system->getMillis() + 5000;
		while (res == CURLE_AGAIN) {
			res = curl_easy_send(_easy, data + nsent_total, left - nsent_total, &nsent);
			if (g_system->getMillis() >= tickCount) {
				warning("libcurl: Took too long attempting to send data to socket");
				return nsent;
			}
		}
		if (res == CURLE_OK) {
			nsent_total += nsent;
			left -= nsent;
		} else if (res != CURLE_AGAIN) {
			warning("libcurl: Error when sending to socket: %s", curl_easy_strerror(res));
			return nsent_total;
		}
	}

	return nsent_total;
}

size_t CurlSocket::recv(void *data, int maxLen) {
	size_t nread = 0;
	CURLcode res = CURLE_AGAIN;
	uint32 tickCount = g_system->getMillis() + 5000;
	while (res == CURLE_AGAIN) {
		res = curl_easy_recv(_easy, data, maxLen, &nread);
		if (g_system->getMillis() >= tickCount) {
			warning("libcurl: Took too long attempting to read data from socket");
			return nread;
		}
	}
	if(res != CURLE_OK) {
		warning("libcurl Error on receiving data: %s\n", curl_easy_strerror(res));
		return nread;
	}

	debug(1, "libcurl: Received %lu bytes", (uint64)nread);
	return nread;
}

} // End of namespace Networking

