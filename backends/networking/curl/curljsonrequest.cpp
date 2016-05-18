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
#include "backends/networking/curl/connectionmanager.h"
#include "backends/networking/curl/networkreadstream.h"
#include "common/debug.h"
#include "common/json.h"
#include <curl/curl.h>

namespace Networking {

CurlJsonRequest::CurlJsonRequest(Callback cb, const char *url) : Request(cb), _stream(0), _headersList(0), _contentsStream(DisposeAfterUse::YES) {
	_url = url;
}

CurlJsonRequest::~CurlJsonRequest() {
	if (_stream) delete _stream;
}

char *CurlJsonRequest::getPreparedContents() {
	//write one more byte in the end
	byte zero[1] = { 0 };
	_contentsStream.write(zero, 1);

	//replace all "bad" bytes with '.' character
	byte *result = _contentsStream.getData();
	uint32 size = _contentsStream.size();
	for (uint32 i = 0; i < size; ++i)
		if (result[i] < 0x20 || result[i] > 0x7f)
			result[i] = '.';

	//make it zero-terminated string
	result[size - 1] = '\0';

	return (char *)result;
}

bool CurlJsonRequest::handle() {
	if (!_stream) _stream = new NetworkReadStream(_url, _headersList, _postFields);

	if (_stream) {
		const int kBufSize = 16*1024;
		char buf[kBufSize+1];
		uint32 readBytes = _stream->read(buf, kBufSize);
		if (readBytes != 0)
			if (_contentsStream.write(buf, readBytes) != readBytes)
				warning("MemoryWriteStreamDynamic was unable to write all the bytes");

		if (_stream->eos()) {			
			if (_stream->httpResponseCode() != 200)
				warning("HTTP response code is not 200 OK");

			if (_callback) {
				char *contents = getPreparedContents();				
				Common::JSONValue *json = Common::JSON::parse(contents);				
				_callback(json); //potential memory leak, free it in your callbacks!
			}
			return true;
		}
	}

	return false;
}

void CurlJsonRequest::addHeader(Common::String header) {
	_headersList = curl_slist_append(_headersList, header.c_str());
}

void CurlJsonRequest::addPostField(Common::String keyValuePair) {
	if (_postFields == "")
		_postFields = keyValuePair;
	else
		_postFields += "&" + keyValuePair;
}


} //end of namespace Networking
