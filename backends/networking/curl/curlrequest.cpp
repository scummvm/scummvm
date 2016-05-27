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

#include "backends/networking/curl/curlrequest.h"
#include "backends/networking/curl/connectionmanager.h"
#include "backends/networking/curl/networkreadstream.h"
#include "common/textconsole.h"
#include <curl/curl.h>

namespace Networking {

CurlRequest::CurlRequest(DataCallback cb, Common::String url):
	Request(cb), _url(url), _stream(0), _headersList(0) {}

CurlRequest::~CurlRequest() { delete _stream; }

void CurlRequest::handle() {
	if (!_stream) _stream = new NetworkReadStream(_url.c_str(), _headersList, _postFields);	

	if (_stream && _stream->eos()) {
		if (_stream->httpResponseCode() != 200)
			warning("HTTP response code is not 200 OK (it's %ld)", _stream->httpResponseCode());
		finish();
	}
}

void CurlRequest::restart() {
	if (_stream) delete _stream;
	_stream = 0;
	//with no stream available next handle() will create another one
}

void CurlRequest::setHeaders(Common::Array<Common::String> &headers) {
	curl_slist_free_all(_headersList);
	_headersList = 0;
	for (uint32 i = 0; i < headers.size(); ++i)
		addHeader(headers[i]);
}

void CurlRequest::addHeader(Common::String header) {
	_headersList = curl_slist_append(_headersList, header.c_str());
}

void CurlRequest::addPostField(Common::String keyValuePair) {
	if (_postFields == "")
		_postFields = keyValuePair;
	else
		_postFields += "&" + keyValuePair;
}

NetworkReadStreamResponse CurlRequest::execute() {
	if (!_stream) {
		_stream = new NetworkReadStream(_url.c_str(), _headersList, _postFields);
		ConnMan.addRequest(this);
	}

	return NetworkReadStreamResponse(this, _stream);
}

} //end of namespace Networking
