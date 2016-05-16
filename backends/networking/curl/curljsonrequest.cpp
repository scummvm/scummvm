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

#include "backends/networking/curl/curljsonrequest.h"
#include "backends/networking/curl/networkreadstream.h"
#include "common/debug.h"
#include "common/json.h"
#include <curl/curl.h>

namespace Networking {

CurlJsonRequest::CurlJsonRequest(Callback cb, const char *url) : Request(cb), _stream(0) {	
	_url = url;
}

CurlJsonRequest::~CurlJsonRequest() {
	if (_stream) delete _stream;
}

bool CurlJsonRequest::handle(ConnectionManager &manager) {
	if (!_stream) _stream = manager.makeRequest(_url);

	if (_stream) {
		const int kBufSize = 16*1024;
		char buf[kBufSize+1];
		uint32 readBytes = _stream->read(buf, kBufSize);
		if (readBytes != 0) _contents += Common::String(buf, readBytes);
		if (_stream->eos()) {
			//TODO: check that stream's CURL easy handle's HTTP response code is 200 OK
			debug("CurlJsonRequest: contents:\n%s", _contents.c_str());
			if (_callback) {
				Common::JSONValue *json = Common::JSON::parse(_contents.c_str()); //TODO: somehow fix JSON to work with UTF-8
				debug("CurlJsonRequest: JSONValue pointer = %p", json);
				_callback(json); //potential memory leak, free it in your callbacks!
			}
			return true;
		}
	}

	return false;
}

} //end of namespace Networking
