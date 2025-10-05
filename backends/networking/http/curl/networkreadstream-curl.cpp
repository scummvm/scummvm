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

#define FORBIDDEN_SYMBOL_ALLOW_ALL
#define CURL_DISABLE_DEPRECATION

#include "backends/networking/basic/curl/cacert.h"
#include "backends/networking/http/curl/networkreadstream-curl.h"
#include "backends/networking/http/curl/connectionmanager-curl.h"
#include "base/version.h"
#include "common/debug.h"

namespace Networking {

NetworkReadStream *NetworkReadStream::make(const char *url, RequestHeaders *headersList, const Common::String &postFields, bool uploading, bool usingPatch, bool keepAlive, long keepAliveIdle, long keepAliveInterval) {
	return new NetworkReadStreamCurl(url, headersList, postFields, uploading, usingPatch, keepAlive, keepAliveIdle, keepAliveInterval);
}

NetworkReadStream *NetworkReadStream::make(const char *url, RequestHeaders *headersList, const Common::HashMap<Common::String, Common::String> &formFields, const Common::HashMap<Common::String, Common::Path> &formFiles, bool keepAlive, long keepAliveIdle, long keepAliveInterval) {
	return new NetworkReadStreamCurl(url, headersList, formFields, formFiles, keepAlive, keepAliveIdle, keepAliveInterval);
}

NetworkReadStream *NetworkReadStream::make(const char *url, RequestHeaders *headersList, const byte *buffer, uint32 bufferSize, bool uploading, bool usingPatch, bool post, bool keepAlive, long keepAliveIdle, long keepAliveInterval) {
	return new NetworkReadStreamCurl(url, headersList, buffer, bufferSize, uploading, usingPatch, post, keepAlive, keepAliveIdle, keepAliveInterval);
}

size_t NetworkReadStreamCurl::curlDataCallback(char *d, size_t n, size_t l, void *p) {
	NetworkReadStreamCurl *stream = (NetworkReadStreamCurl *)p;
	if (stream)
		return stream->_backingStream.write(d, n * l);
	return 0;
}

size_t NetworkReadStreamCurl::curlReadDataCallback(char *d, size_t n, size_t l, void *p) {
	NetworkReadStreamCurl *stream = (NetworkReadStreamCurl *)p;
	if (stream)
		return stream->fillWithSendingContents(d, n * l);
	return 0;
}

size_t NetworkReadStreamCurl::curlHeadersCallback(char *d, size_t n, size_t l, void *p) {
	NetworkReadStreamCurl *stream = (NetworkReadStreamCurl *)p;
	if (stream)
		return stream->addResponseHeaders(d, n * l);
	return 0;
}

static int curlProgressCallback(void *p, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
	NetworkReadStreamCurl *stream = (NetworkReadStreamCurl *)p;
	if (stream)
		stream->setProgress(dlnow, dltotal);
	return 0;
}

int NetworkReadStreamCurl::curlProgressCallbackOlder(void *p, double dltotal, double dlnow, double ultotal, double ulnow) {
	// for libcurl older than 7.32.0 (CURLOPT_PROGRESSFUNCTION)
	return curlProgressCallback(p, (curl_off_t)dltotal, (curl_off_t)dlnow, (curl_off_t)ultotal, (curl_off_t)ulnow);
}

void NetworkReadStreamCurl::resetStream() {
	_eos = _requestComplete = false;
	if (!_errorBuffer)
		_errorBuffer = (char *)calloc(CURL_ERROR_SIZE, 1);
	_sendingContentsBuffer = nullptr;
	_sendingContentsSize = _sendingContentsPos = 0;
	_progressDownloaded = _progressTotal = 0;
	_bufferCopy = nullptr;
	if (_headersSlist) {
		curl_slist_free_all(_headersSlist);
		_headersSlist = nullptr;
	}
}

void NetworkReadStreamCurl::initCurl(const char *url, RequestHeaders *headersList) {
	resetStream();

	_easy = curl_easy_init();
	curl_easy_setopt(_easy, CURLOPT_WRITEFUNCTION, curlDataCallback);
	curl_easy_setopt(_easy, CURLOPT_WRITEDATA, this); // so callback can call us
	curl_easy_setopt(_easy, CURLOPT_PRIVATE, this);   // so ConnectionManager can call us when request is complete
	curl_easy_setopt(_easy, CURLOPT_HEADER, 0L);
	curl_easy_setopt(_easy, CURLOPT_HEADERDATA, this);
	curl_easy_setopt(_easy, CURLOPT_HEADERFUNCTION, curlHeadersCallback);
	curl_easy_setopt(_easy, CURLOPT_URL, url);
	curl_easy_setopt(_easy, CURLOPT_ERRORBUFFER, _errorBuffer);
	curl_easy_setopt(_easy, CURLOPT_VERBOSE, 0L);
	curl_easy_setopt(_easy, CURLOPT_FOLLOWLOCATION, 1L); // probably it's OK to have it always on

	// Convert headers to curl_slist format
	_headersSlist = requestHeadersToSlist(headersList);
	curl_easy_setopt(_easy, CURLOPT_HTTPHEADER, _headersSlist);

	curl_easy_setopt(_easy, CURLOPT_USERAGENT, gScummVMFullVersion);
	curl_easy_setopt(_easy, CURLOPT_NOPROGRESS, 0L);
	curl_easy_setopt(_easy, CURLOPT_PROGRESSFUNCTION, curlProgressCallbackOlder);
	curl_easy_setopt(_easy, CURLOPT_PROGRESSDATA, this);

#if defined NINTENDO_SWITCH || defined PSP2
	curl_easy_setopt(_easy, CURLOPT_SSL_VERIFYPEER, 0);
#endif

	Common::String caCertPath = getCaCertPath();
	if (!caCertPath.empty()) {
		curl_easy_setopt(_easy, CURLOPT_CAINFO, caCertPath.c_str());
	}

#if LIBCURL_VERSION_NUM >= 0x072000
	// CURLOPT_XFERINFOFUNCTION introduced in libcurl 7.32.0
	// CURLOPT_PROGRESSFUNCTION is used as a backup plan in case older version is used
	curl_easy_setopt(_easy, CURLOPT_XFERINFOFUNCTION, curlProgressCallback);
	curl_easy_setopt(_easy, CURLOPT_XFERINFODATA, this);
#endif

#if LIBCURL_VERSION_NUM >= 0x071900
	// Added in libcurl 7.25.0
	if (_keepAlive) {
		curl_easy_setopt(_easy, CURLOPT_TCP_KEEPALIVE, 1L);
		curl_easy_setopt(_easy, CURLOPT_TCP_KEEPIDLE, _keepAliveIdle);
		curl_easy_setopt(_easy, CURLOPT_TCP_KEEPINTVL, _keepAliveInterval);
	}
#endif
}

bool NetworkReadStreamCurl::reuseCurl(const char *url, RequestHeaders *headersList) {
	if (!_keepAlive) {
		warning("NetworkReadStream: Can't reuse curl handle (was not setup as keep-alive)");
		return false;
	}

	resetStream();

	_headersSlist = requestHeadersToSlist(headersList);
	curl_easy_setopt(_easy, CURLOPT_URL, url);
	curl_easy_setopt(_easy, CURLOPT_HTTPHEADER, _headersSlist);
	curl_easy_setopt(_easy, CURLOPT_USERAGENT, gScummVMFullVersion); // in case headersList rewrites it

	return true;
}
void NetworkReadStreamCurl::setupBufferContents(const byte *buffer, uint32 bufferSize, bool uploading, bool usingPatch, bool post) {
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
			_bufferCopy = (byte *)malloc(bufferSize);
			memcpy(_bufferCopy, buffer, bufferSize);
			curl_easy_setopt(_easy, CURLOPT_POSTFIELDS, _bufferCopy);
#endif
		}
	}
	dynamic_cast<ConnectionManagerCurl &>(ConnMan).registerEasyHandle(_easy);
}

void NetworkReadStreamCurl::setupFormMultipart(const Common::HashMap<Common::String, Common::String> &formFields, const Common::HashMap<Common::String, Common::Path> &formFiles) {
	struct curl_httppost *formpost = nullptr;
	struct curl_httppost *lastptr = nullptr;

	for (Common::HashMap<Common::String, Common::String>::iterator i = formFields.begin(); i != formFields.end(); ++i) {
		CURLFORMcode code = curl_formadd(
			&formpost,
			&lastptr,
			CURLFORM_COPYNAME, i->_key.c_str(),
			CURLFORM_COPYCONTENTS, i->_value.c_str(),
			CURLFORM_END);

		if (code != CURL_FORMADD_OK)
			warning("NetworkReadStreamCurl: field curl_formadd('%s') failed", i->_key.c_str());
	}

	for (Common::HashMap<Common::String, Common::Path>::iterator i = formFiles.begin(); i != formFiles.end(); ++i) {
		CURLFORMcode code = curl_formadd(
			&formpost,
			&lastptr,
			CURLFORM_COPYNAME, i->_key.c_str(),
			CURLFORM_FILE, i->_value.toString(Common::Path::kNativeSeparator).c_str(),
			CURLFORM_END);

		if (code != CURL_FORMADD_OK)
			warning("NetworkReadStreamCurl: file curl_formadd('%s') failed", i->_key.c_str());
	}

	curl_easy_setopt(_easy, CURLOPT_HTTPPOST, formpost);
	dynamic_cast<ConnectionManagerCurl &>(ConnMan).registerEasyHandle(_easy);
}

NetworkReadStreamCurl::NetworkReadStreamCurl(const char *url, RequestHeaders *headersList, const Common::String &postFields, bool uploading, bool usingPatch, bool keepAlive, long keepAliveIdle, long keepAliveInterval)
	: NetworkReadStream(keepAlive, keepAliveIdle, keepAliveInterval),
	_errorBuffer(nullptr), _headersSlist(nullptr) {
	initCurl(url, headersList);
	setupBufferContents((const byte *)postFields.c_str(), postFields.size(), uploading, usingPatch, false);
}

NetworkReadStreamCurl::NetworkReadStreamCurl(const char *url, RequestHeaders *headersList, const Common::HashMap<Common::String, Common::String> &formFields, const Common::HashMap<Common::String, Common::Path> &formFiles, bool keepAlive, long keepAliveIdle, long keepAliveInterval)
	: NetworkReadStream(keepAlive, keepAliveIdle, keepAliveInterval),
	_errorBuffer(nullptr), _headersSlist(nullptr) {
	initCurl(url, headersList);
	setupFormMultipart(formFields, formFiles);
}

NetworkReadStreamCurl::NetworkReadStreamCurl(const char *url, RequestHeaders *headersList, const byte *buffer, uint32 bufferSize, bool uploading, bool usingPatch, bool post, bool keepAlive, long keepAliveIdle, long keepAliveInterval)
	: NetworkReadStream(keepAlive, keepAliveIdle, keepAliveInterval),
	_errorBuffer(nullptr), _headersSlist(nullptr) {
	initCurl(url, headersList);
	setupBufferContents(buffer, bufferSize, uploading, usingPatch, post);
}

curl_slist *NetworkReadStreamCurl::requestHeadersToSlist(const RequestHeaders *headersList) {
	curl_slist *slist = nullptr;
	if (headersList) {
		for (const Common::String &header : *headersList) {
			slist = curl_slist_append(slist, header.c_str());
		}
	}
	return slist;
}

bool NetworkReadStreamCurl::reuse(const char *url, RequestHeaders *headersList, const Common::String &postFields, bool uploading, bool usingPatch) {
	if (!reuseCurl(url, headersList))
		return false;

	_backingStream = Common::MemoryReadWriteStream(DisposeAfterUse::YES);
	setupBufferContents((const byte *)postFields.c_str(), postFields.size(), uploading, usingPatch, false);
	return true;
}

bool NetworkReadStreamCurl::reuse(const char *url, RequestHeaders *headersList, const Common::HashMap<Common::String, Common::String> &formFields, const Common::HashMap<Common::String, Common::Path> &formFiles) {
	if (!reuseCurl(url, headersList))
		return false;

	_backingStream = Common::MemoryReadWriteStream(DisposeAfterUse::YES);
	setupFormMultipart(formFields, formFiles);
	return true;
}

bool NetworkReadStreamCurl::reuse(const char *url, RequestHeaders *headersList, const byte *buffer, uint32 bufferSize, bool uploading, bool usingPatch, bool post) {
	if (!reuseCurl(url, headersList))
		return false;

	_backingStream = Common::MemoryReadWriteStream(DisposeAfterUse::YES);
	setupBufferContents(buffer, bufferSize, uploading, usingPatch, post);
	return true;
}

NetworkReadStreamCurl::~NetworkReadStreamCurl() {
	if (_easy)
		curl_easy_cleanup(_easy);
	free(_bufferCopy);
	free(_errorBuffer);
	if (_headersSlist) {
		curl_slist_free_all(_headersSlist);
	}
}

void NetworkReadStreamCurl::finished(CURLcode errorCode) {
	_requestComplete = true;

	char *url = nullptr;
	curl_easy_getinfo(_easy, CURLINFO_EFFECTIVE_URL, &url);

	_errorCode = errorCode;

	if (_errorCode == CURLE_OK) {
		debug(9, "NetworkReadStreamCurl: %s - Request succeeded", url);
	} else {
		warning("NetworkReadStreamCurl: %s - Request failed (%d - %s)", url, _errorCode, getError());
	}
}

bool NetworkReadStreamCurl::hasError() const {
	return _errorCode != CURLE_OK;
}

const char *NetworkReadStreamCurl::getError() const {
	return strlen(_errorBuffer) ? _errorBuffer : curl_easy_strerror(_errorCode);
}

long NetworkReadStreamCurl::httpResponseCode() const {
	long responseCode = -1;
	if (_easy)
		curl_easy_getinfo(_easy, CURLINFO_RESPONSE_CODE, &responseCode);
	return responseCode;
}

Common::String NetworkReadStreamCurl::currentLocation() const {
	Common::String result = "";
	if (_easy) {
		char *pointer;
		curl_easy_getinfo(_easy, CURLINFO_EFFECTIVE_URL, &pointer);
		result = Common::String(pointer);
	}
	return result;
}

Common::HashMap<Common::String, Common::String> NetworkReadStreamCurl::responseHeadersMap() const {
	// HTTP headers are described at RFC 2616: https://datatracker.ietf.org/doc/html/rfc2616#section-4.2
	// this implementation tries to follow it, but for simplicity it does not support multi-line header values

	Common::HashMap<Common::String, Common::String> headers;
	Common::String headerName, headerValue, trailingWhitespace;
	char c;
	bool readingName = true;

	for (uint i = 0; i < _responseHeaders.size(); ++i) {
		c = _responseHeaders[i];

		if (readingName) {
			if (c == ' ' || c == '\r' || c == '\n' || c == '\t') {
				// header names should not contain any whitespace, this is invalid
				// ignore what's been before
				headerName = "";
				continue;
			}
			if (c == ':') {
				if (!headerName.empty()) {
					readingName = false;
				}
				continue;
			}
			headerName += c;
			continue;
		}

		// reading value:
		if (c == ' ' || c == '\t') {
			if (headerValue.empty()) {
				// skip leading whitespace
				continue;
			} else {
				// accumulate trailing whitespace
				trailingWhitespace += c;
				continue;
			}
		}

		if (c == '\r' || c == '\n') {
			// not sure if RFC allows empty values, we'll ignore such
			if (!headerName.empty() && !headerValue.empty()) {
				// add header value
				// RFC allows header with the same name to be sent multiple times
				// and requires it to be equivalent of just listing all header values separated with comma
				// so if header already was met, we'll add new value to the old one
				headerName.toLowercase();
				if (headers.contains(headerName)) {
					headers[headerName] += "," + headerValue;
				} else {
					headers[headerName] = headerValue;
				}
			}

			headerName = "";
			headerValue = "";
			trailingWhitespace = "";
			readingName = true;
			continue;
		}

		// if we meet non-whitespace character, turns out those "trailing" whitespace characters were not so trailing
		headerValue += trailingWhitespace;
		trailingWhitespace = "";
		headerValue += c;
	}

	return headers;
}

} // End of namespace Networking
