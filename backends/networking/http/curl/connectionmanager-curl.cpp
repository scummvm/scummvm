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

#include "backends/networking/http/curl/connectionmanager-curl.h"
#include "backends/networking/http/curl/networkreadstream-curl.h"
#include "common/debug.h"
#include "common/system.h"
#include "common/timer.h"

namespace Common {

template<>
Networking::ConnectionManager *Singleton<Networking::ConnectionManager>::makeInstance() {
	return new Networking::ConnectionManagerCurl();
}

} // namespace Common

namespace Networking {
/* Workaround a MSVC bug from MSVC 2015
 * The compiler considers this template specialization as inline.
 * If this TU doesn't use the function, it is then discarded.
 */
#if defined(_MSC_VER) && (_MSC_VER >= 1900)
void dummyFunction() {
	ConnMan;
}
#endif

ConnectionManagerCurl::ConnectionManagerCurl() : ConnectionManager(), _multi(nullptr) {
	curl_global_init(CURL_GLOBAL_ALL);
	_multi = curl_multi_init();
}

ConnectionManagerCurl::~ConnectionManagerCurl() {
	// cleanup
	curl_multi_cleanup(_multi);
	curl_global_cleanup();
	_multi = nullptr;
}

void ConnectionManagerCurl::registerEasyHandle(CURL *easy) const {
	curl_multi_add_handle(_multi, easy);
}

// private goes here:
void ConnectionManagerCurl::processTransfers() {
	if (!_multi)
		return;

	// check libcurl's transfers and notify requests of messages from queue (transfer completion or failure)
	int transfersRunning;
	curl_multi_perform(_multi, &transfersRunning);

	int messagesInQueue;
	CURLMsg *curlMsg;
	while ((curlMsg = curl_multi_info_read(_multi, &messagesInQueue))) {
		if (curlMsg->msg == CURLMSG_DONE) {
			CURL *easyHandle = curlMsg->easy_handle;

			NetworkReadStreamCurl *stream = nullptr;
			curl_easy_getinfo(easyHandle, CURLINFO_PRIVATE, &stream);

			if (stream)
				stream->finished(curlMsg->data.result);

			curl_multi_remove_handle(_multi, easyHandle);
		} else {
			warning("Unknown libcurl message type %d", curlMsg->msg);
		}
	}
}

} // End of namespace Networking
