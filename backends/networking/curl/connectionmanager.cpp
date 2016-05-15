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

#include "backends/networking/curl/connectionmanager.h"
#include "backends/networking/curl/networkreadstream.h"
#include "common/debug.h"
#include <curl/curl.h>

namespace Networking {

ConnectionManager::ConnectionManager(): _multi(0) {
	curl_global_init(CURL_GLOBAL_ALL);
	_multi = curl_multi_init();
}

ConnectionManager::~ConnectionManager() {
	curl_multi_cleanup(_multi);
	curl_global_cleanup();
}

NetworkReadStream *ConnectionManager::makeRequest(const char *url) {
	NetworkReadStream *stream = new NetworkReadStream(url);
	curl_multi_add_handle(_multi, stream->getEasyHandle());
	return stream;
}

void ConnectionManager::handle() {
	int U;
	curl_multi_perform(_multi, &U);

	int Q;
	CURLMsg *curlMsg;
	while ((curlMsg = curl_multi_info_read(_multi, &Q))) {
		if (curlMsg->msg == CURLMSG_DONE) {
			CURL *e = curlMsg->easy_handle;

			NetworkReadStream *stream;
			curl_easy_getinfo(e, CURLINFO_PRIVATE, &stream);
			if (stream) stream->done();			

			debug("ConnectionManager: SUCCESS (%d - %s)", curlMsg->data.result, curl_easy_strerror(curlMsg->data.result));
			curl_multi_remove_handle(_multi, e);
		} else {
			debug("ConnectionManager: FAILURE (CURLMsg (%d))", curlMsg->msg);
			//TODO: notify stream on this case also
		}
	}
}

} //end of namespace Cloud
