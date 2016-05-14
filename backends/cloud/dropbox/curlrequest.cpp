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

#include "backends/cloud/dropbox/curlrequest.h"
#include "common/debug.h"
#include <curl/curl.h>

namespace Cloud {
namespace Dropbox {

static size_t curlDataCallback(char *d, size_t n, size_t l, void *p) {
	debug("%p got %d more bytes", p, n * l);
	return n * l;
}

CurlRequest::CurlRequest(Callback cb, char *url) : Request(cb), _firstTime(true) {
	_curlm = curl_multi_init();
	_url = url;
}

CurlRequest::~CurlRequest() {
	curl_multi_cleanup(_curlm);
}

bool CurlRequest::handle() {
	if (_firstTime) {
		CURL *eh = curl_easy_init();
		curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, curlDataCallback);
		curl_easy_setopt(eh, CURLOPT_WRITEDATA, this);
		curl_easy_setopt(eh, CURLOPT_HEADER, 0L);
		curl_easy_setopt(eh, CURLOPT_URL, _url);
		curl_easy_setopt(eh, CURLOPT_VERBOSE, 0L);
		curl_multi_add_handle(_curlm, eh);

		_firstTime = false;
	}

	int U;
	curl_multi_perform(_curlm, &U);

	int Q;
	CURLMsg *_curlMsg;
	while ((_curlMsg = curl_multi_info_read(_curlm, &Q))) {
		if (_curlMsg->msg == CURLMSG_DONE) {			
			CURL *e = _curlMsg->easy_handle;
			debug("R: %d - %s\n", _curlMsg->data.result, curl_easy_strerror(_curlMsg->data.result));
			curl_multi_remove_handle(_curlm, e);
			curl_easy_cleanup(e);

			_callback(0);
			return true;
		} else {
			debug("E: CURLMsg (%d)\n", _curlMsg->msg);
		}
	}

	return false;
}

} //end of namespace Dropbox
} //end of namespace Cloud
