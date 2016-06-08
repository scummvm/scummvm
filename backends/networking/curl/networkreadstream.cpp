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

#include "backends/networking/curl/networkreadstream.h"
#include "backends/networking/curl/connectionmanager.h"
#include "common/debug.h"
#include <curl/curl.h>

namespace Networking {

static size_t curlDataCallback(char *d, size_t n, size_t l, void *p) {	
	NetworkReadStream *stream = (NetworkReadStream *)p;
	if (stream) return stream->write(d, n*l);
	return 0;
}

static size_t curlReadDataCallback(char *d, size_t n, size_t l, void *p) {	
	NetworkReadStream *stream = (NetworkReadStream *)p;
	if (stream) return stream->fillWithSendingContents(d, n*l);
	return 0;
}

static size_t curlHeadersCallback(char *d, size_t n, size_t l, void *p) {
	NetworkReadStream *stream = (NetworkReadStream *)p;
	if (stream) return stream->addResponseHeaders(d, n*l);
	return 0;
}

NetworkReadStream::NetworkReadStream(const char *url, curl_slist *headersList, Common::String postFields, bool uploading, bool usingPatch):
	NetworkReadStream(url, headersList, (byte *)postFields.c_str(), postFields.size(), uploading, usingPatch, false) {}

NetworkReadStream::NetworkReadStream(const char *url, curl_slist *headersList, byte *buffer, uint32 bufferSize, bool uploading, bool usingPatch, bool post):
	_easy(0), _eos(false), _requestComplete(false), _sendingContentsBuffer(nullptr), _sendingContentsSize(0), _sendingContentsPos(0) {
	_easy = curl_easy_init();
	curl_easy_setopt(_easy, CURLOPT_WRITEFUNCTION, curlDataCallback);
	curl_easy_setopt(_easy, CURLOPT_WRITEDATA, this); //so callback can call us
	curl_easy_setopt(_easy, CURLOPT_PRIVATE, this); //so ConnectionManager can call us when request is complete
	curl_easy_setopt(_easy, CURLOPT_HEADER, 0L);
	curl_easy_setopt(_easy, CURLOPT_HEADERDATA, this);
	curl_easy_setopt(_easy, CURLOPT_HEADERFUNCTION, curlHeadersCallback);
	curl_easy_setopt(_easy, CURLOPT_URL, url);
	curl_easy_setopt(_easy, CURLOPT_VERBOSE, 0L);
	curl_easy_setopt(_easy, CURLOPT_FOLLOWLOCATION, 1L); //probably it's OK to have it always on
	curl_easy_setopt(_easy, CURLOPT_HTTPHEADER, headersList);	
	if (uploading) {
		curl_easy_setopt(_easy, CURLOPT_UPLOAD, 1L);
		curl_easy_setopt(_easy, CURLOPT_READDATA, this);
		curl_easy_setopt(_easy, CURLOPT_READFUNCTION, curlReadDataCallback);
		_sendingContentsBuffer = buffer;
		_sendingContentsSize = bufferSize;
	} else if (usingPatch) {		
		curl_easy_setopt(_easy, CURLOPT_CUSTOMREQUEST, "PATCH");
	} else {
		if (post || bufferSize != 0) {
			curl_easy_setopt(_easy, CURLOPT_POSTFIELDSIZE, bufferSize);
			curl_easy_setopt(_easy, CURLOPT_COPYPOSTFIELDS, buffer);
		}
	}
	ConnMan.registerEasyHandle(_easy);
}

NetworkReadStream::~NetworkReadStream() {
	if (_easy)
		curl_easy_cleanup(_easy);
}

bool NetworkReadStream::eos() const {
	return _eos;
}

uint32 NetworkReadStream::read(void *dataPtr, uint32 dataSize) {
	uint32 actuallyRead = MemoryReadWriteStream::read(dataPtr, dataSize);

	if (actuallyRead == 0) {
		if (_requestComplete) _eos = true;
		return 0;
	}

	return actuallyRead;
}

void NetworkReadStream::finished() {
	_requestComplete = true;
}

long NetworkReadStream::httpResponseCode() const {
	long responseCode = -1;
	if (_easy)
		curl_easy_getinfo(_easy, CURLINFO_RESPONSE_CODE, &responseCode);
	return responseCode;
}

Common::String NetworkReadStream::currentLocation() const {
	Common::String result = "";
	if (_easy) {
		char *pointer;
		curl_easy_getinfo(_easy, CURLINFO_EFFECTIVE_URL, &pointer);
		result = Common::String(pointer);
	}
	return result;
}

Common::String NetworkReadStream::responseHeaders() const {
	return _responseHeaders;
}

uint32 NetworkReadStream::fillWithSendingContents(char *bufferToFill, uint32 maxSize) {
	uint32 size = _sendingContentsSize - _sendingContentsPos;
	if (size > maxSize) size = maxSize;
	for (uint32 i = 0; i < size; ++i) {
		bufferToFill[i] = _sendingContentsBuffer[_sendingContentsPos + i];
	}
	_sendingContentsPos += size;
	return size;
}

uint32 NetworkReadStream::addResponseHeaders(char *buffer, uint32 size) {
	_responseHeaders += Common::String(buffer, size);
	return size;
}

} // End of namespace Cloud
