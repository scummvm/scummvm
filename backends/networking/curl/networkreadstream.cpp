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

#include <curl/curl.h>
#include "backends/networking/curl/networkreadstream.h"
#include "backends/networking/curl/connectionmanager.h"
#include "base/version.h"

namespace Networking {

size_t NetworkReadStream::curlDataCallback(char *d, size_t n, size_t l, void *p) {
	NetworkReadStream *stream = (NetworkReadStream *)p;
	if (stream)
		return stream->_backingStream.write(d, n * l);
	return 0;
}

size_t NetworkReadStream::curlReadDataCallback(char *d, size_t n, size_t l, void *p) {
	NetworkReadStream *stream = (NetworkReadStream *)p;
	if (stream)
		return stream->fillWithSendingContents(d, n * l);
	return 0;
}

size_t NetworkReadStream::curlHeadersCallback(char *d, size_t n, size_t l, void *p) {
	NetworkReadStream *stream = (NetworkReadStream *)p;
	if (stream)
		return stream->addResponseHeaders(d, n * l);
	return 0;
}

static int curlProgressCallback(void *p, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
	NetworkReadStream *stream = (NetworkReadStream *)p;
	if (stream)
		stream->setProgress(dlnow, dltotal);
	return 0;
}

int NetworkReadStream::curlProgressCallbackOlder(void *p, double dltotal, double dlnow, double ultotal, double ulnow) {
	// for libcurl older than 7.32.0 (CURLOPT_PROGRESSFUNCTION)
	return curlProgressCallback(p, (curl_off_t)dltotal, (curl_off_t)dlnow, (curl_off_t)ultotal, (curl_off_t)ulnow);
}

void NetworkReadStream::init(const char *url, curl_slist *headersList, const byte *buffer, uint32 bufferSize, bool uploading, bool usingPatch, bool post) {
	_eos = _requestComplete = false;
	_sendingContentsBuffer = nullptr;
	_sendingContentsSize = _sendingContentsPos = 0;
	_progressDownloaded = _progressTotal = 0;
	_bufferCopy = nullptr;

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
	curl_easy_setopt(_easy, CURLOPT_USERAGENT, gScummVMFullVersion);
	curl_easy_setopt(_easy, CURLOPT_NOPROGRESS, 0L);
	curl_easy_setopt(_easy, CURLOPT_PROGRESSFUNCTION, curlProgressCallbackOlder);
	curl_easy_setopt(_easy, CURLOPT_PROGRESSDATA, this);
#if LIBCURL_VERSION_NUM >= 0x072000
	// CURLOPT_XFERINFOFUNCTION introduced in libcurl 7.32.0
	// CURLOPT_PROGRESSFUNCTION is used as a backup plan in case older version is used
	curl_easy_setopt(_easy, CURLOPT_XFERINFOFUNCTION, curlProgressCallback);
	curl_easy_setopt(_easy, CURLOPT_XFERINFODATA, this);
#endif
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
#if LIBCURL_VERSION_NUM >= 0x071101
			// CURLOPT_COPYPOSTFIELDS available since curl 7.17.1
			curl_easy_setopt(_easy, CURLOPT_COPYPOSTFIELDS, buffer);
#else
			_bufferCopy = (byte*)malloc(bufferSize);
			memcpy(_bufferCopy, buffer, bufferSize);
			curl_easy_setopt(_easy, CURLOPT_POSTFIELDS, _bufferCopy);
#endif
		}
	}
	ConnMan.registerEasyHandle(_easy);
}

void NetworkReadStream::init(const char *url, curl_slist *headersList, Common::HashMap<Common::String, Common::String> formFields, Common::HashMap<Common::String, Common::String> formFiles) {
	_eos = _requestComplete = false;
	_sendingContentsBuffer = nullptr;
	_sendingContentsSize = _sendingContentsPos = 0;
	_progressDownloaded = _progressTotal = 0;
	_bufferCopy = nullptr;

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
	curl_easy_setopt(_easy, CURLOPT_USERAGENT, gScummVMFullVersion);
	curl_easy_setopt(_easy, CURLOPT_NOPROGRESS, 0L);
	curl_easy_setopt(_easy, CURLOPT_PROGRESSFUNCTION, curlProgressCallbackOlder);
	curl_easy_setopt(_easy, CURLOPT_PROGRESSDATA, this);
#if LIBCURL_VERSION_NUM >= 0x072000
	// CURLOPT_XFERINFOFUNCTION introduced in libcurl 7.32.0
	// CURLOPT_PROGRESSFUNCTION is used as a backup plan in case older version is used
	curl_easy_setopt(_easy, CURLOPT_XFERINFOFUNCTION, curlProgressCallback);
	curl_easy_setopt(_easy, CURLOPT_XFERINFODATA, this);
#endif

	// set POST multipart upload form fields/files
	struct curl_httppost *formpost = nullptr;
	struct curl_httppost *lastptr = nullptr;

	for (Common::HashMap<Common::String, Common::String>::iterator i = formFields.begin(); i != formFields.end(); ++i) {
		CURLFORMcode code = curl_formadd(
			&formpost,
			&lastptr,
			CURLFORM_COPYNAME, i->_key.c_str(),
			CURLFORM_COPYCONTENTS, i->_value.c_str(),
			CURLFORM_END
		);

		if (code != CURL_FORMADD_OK)
			warning("NetworkReadStream: field curl_formadd('%s') failed", i->_key.c_str());
	}

	for (Common::HashMap<Common::String, Common::String>::iterator i = formFiles.begin(); i != formFiles.end(); ++i) {
		CURLFORMcode code = curl_formadd(
			&formpost,
			&lastptr,
			CURLFORM_COPYNAME, i->_key.c_str(),
			CURLFORM_FILE, i->_value.c_str(),
			CURLFORM_END
		);

		if (code != CURL_FORMADD_OK)
			warning("NetworkReadStream: file curl_formadd('%s') failed", i->_key.c_str());
	}

	curl_easy_setopt(_easy, CURLOPT_HTTPPOST, formpost);

	ConnMan.registerEasyHandle(_easy);
}

NetworkReadStream::NetworkReadStream(const char *url, curl_slist *headersList, Common::String postFields, bool uploading, bool usingPatch) :
		_backingStream(DisposeAfterUse::YES) {
	init(url, headersList, (const byte *)postFields.c_str(), postFields.size(), uploading, usingPatch, false);
}

NetworkReadStream::NetworkReadStream(const char *url, curl_slist *headersList, Common::HashMap<Common::String, Common::String> formFields, Common::HashMap<Common::String, Common::String> formFiles) :
		_backingStream(DisposeAfterUse::YES) {
	init(url, headersList, formFields, formFiles);
}

NetworkReadStream::NetworkReadStream(const char *url, curl_slist *headersList, const byte *buffer, uint32 bufferSize, bool uploading, bool usingPatch, bool post) :
		_backingStream(DisposeAfterUse::YES) {
	init(url, headersList, buffer, bufferSize, uploading, usingPatch, post);
}

NetworkReadStream::~NetworkReadStream() {
	if (_easy)
		curl_easy_cleanup(_easy);
	free(_bufferCopy);
}

bool NetworkReadStream::eos() const {
	return _eos;
}

uint32 NetworkReadStream::read(void *dataPtr, uint32 dataSize) {
	uint32 actuallyRead = _backingStream.read(dataPtr, dataSize);

	if (actuallyRead == 0) {
		if (_requestComplete)
			_eos = true;
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
	uint32 sendSize = _sendingContentsSize - _sendingContentsPos;
	if (sendSize > maxSize)
		sendSize = maxSize;
	for (uint32 i = 0; i < sendSize; ++i) {
		bufferToFill[i] = _sendingContentsBuffer[_sendingContentsPos + i];
	}
	_sendingContentsPos += sendSize;
	return sendSize;
}

uint32 NetworkReadStream::addResponseHeaders(char *buffer, uint32 bufferSize) {
	_responseHeaders += Common::String(buffer, bufferSize);
	return bufferSize;
}

double NetworkReadStream::getProgress() const {
	if (_progressTotal < 1)
		return 0;
	return (double)_progressDownloaded / (double)_progressTotal;
}

void NetworkReadStream::setProgress(uint64 downloaded, uint64 total) {
	_progressDownloaded = downloaded;
	_progressTotal = total;
}

} // End of namespace Cloud
