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

#include "backends/cloud/curl/networkreadstream.h"
#include "common/debug.h"
#include <curl/curl.h>

namespace Cloud {

static size_t curlDataCallback(char *d, size_t n, size_t l, void *p) {	
	NetworkReadStream *stream = (NetworkReadStream *)p;
	if (stream) return stream->dataCallback(d, n, l);
	return 0;
}

NetworkReadStream::NetworkReadStream(const char *url): _easy(0), _eos(false), _requestComplete(false) {
	_easy = curl_easy_init();
	curl_easy_setopt(_easy, CURLOPT_WRITEFUNCTION, curlDataCallback);
	curl_easy_setopt(_easy, CURLOPT_WRITEDATA, this); //so callback can call us
	curl_easy_setopt(_easy, CURLOPT_PRIVATE, this); //so ConnectionManager can call us when request is complete
	curl_easy_setopt(_easy, CURLOPT_HEADER, 0L);
	curl_easy_setopt(_easy, CURLOPT_URL, url);
	curl_easy_setopt(_easy, CURLOPT_VERBOSE, 0L);
}

NetworkReadStream::~NetworkReadStream() {
	curl_easy_cleanup(_easy);
}

bool NetworkReadStream::eos() const {
	return _eos;
}

uint32 NetworkReadStream::read(void *dataPtr, uint32 dataSize) {
	uint32 available = _bytes.size();

	if (available == 0) {
		if (_requestComplete) _eos = true;
		return 0;
	}

	char *data = (char *)dataPtr;
	uint32 actuallyRead = (dataSize < available ? dataSize : available);	
	for (uint32 i = 0; i < actuallyRead; ++i) data[i] = _bytes[i];
	data[actuallyRead] = 0;
	_bytes.erase(0, actuallyRead);
	return actuallyRead;
}

void NetworkReadStream::done() {
	_requestComplete = true;
}

size_t NetworkReadStream::dataCallback(char *d, size_t n, size_t l) {
	//TODO: return CURL_WRITEFUNC_PAUSE if _bytes is too long
	//TODO: remember https://curl.haxx.se/libcurl/c/curl_easy_pause.html (Memory Use / compressed data case)
	//TODO: if using pause, don't forget to unpause it somehow from read() up there
	_bytes += Common::String(d, n*l);
	return n*l;
}

} //end of namespace Cloud
