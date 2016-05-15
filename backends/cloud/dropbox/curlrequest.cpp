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
#include "backends/networking/curl/networkreadstream.h"
#include "common/debug.h"
#include <curl/curl.h>

namespace Cloud {
namespace Dropbox {

CurlRequest::CurlRequest(Callback cb, const char *url) : Request(cb), _firstTime(true), _stream(0) {	
	_url = url;
}

CurlRequest::~CurlRequest() {
	if (_stream) delete _stream;
}

bool CurlRequest::handle(Networking::ConnectionManager &manager) {
	if (_firstTime) {
		_stream = manager.makeRequest(_url);
		_firstTime = false;
	}

	if (_stream) {
		const int kBufSize = 10000;
		char buf[kBufSize+1];
		uint32 readBytes = _stream->read(buf, kBufSize);
		debug("%d", readBytes);
		//if(readBytes != 0) debug("%s", buf);

		if(_stream->eos()) {
			_callback(0);
			return true;
		}
	}

	return false;
}

} //end of namespace Dropbox
} //end of namespace Cloud
