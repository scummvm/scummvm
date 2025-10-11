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

#define FORBIDDEN_SYMBOL_EXCEPTION_asctime
#define FORBIDDEN_SYMBOL_EXCEPTION_clock
#define FORBIDDEN_SYMBOL_EXCEPTION_ctime
#define FORBIDDEN_SYMBOL_EXCEPTION_difftime
#define FORBIDDEN_SYMBOL_EXCEPTION_FILE
#define FORBIDDEN_SYMBOL_EXCEPTION_getdate
#define FORBIDDEN_SYMBOL_EXCEPTION_gmtime
#define FORBIDDEN_SYMBOL_EXCEPTION_localtime
#define FORBIDDEN_SYMBOL_EXCEPTION_mktime
#define FORBIDDEN_SYMBOL_EXCEPTION_strcpy
#define FORBIDDEN_SYMBOL_EXCEPTION_strdup
#define FORBIDDEN_SYMBOL_EXCEPTION_time
#include <emscripten.h>
#include <emscripten/fetch.h>

#include "backends/networking/http/emscripten/networkreadstream-emscripten.h"
#include "backends/networking/http/networkreadstream.h"
#include "base/version.h"
#include "common/debug.h"

namespace Networking {

NetworkReadStream *NetworkReadStream::make(const char *url, RequestHeaders *headersList, const Common::String &postFields, bool uploading, bool usingPatch, bool keepAlive, long keepAliveIdle, long keepAliveInterval) {
	return new NetworkReadStreamEmscripten(url, headersList, postFields, uploading, usingPatch, keepAlive, keepAliveIdle, keepAliveInterval);
}

NetworkReadStream *NetworkReadStream::make(const char *url, RequestHeaders *headersList, const Common::HashMap<Common::String, Common::String> &formFields, const Common::HashMap<Common::String, Common::Path> &formFiles, bool keepAlive, long keepAliveIdle, long keepAliveInterval) {
	return new NetworkReadStreamEmscripten(url, headersList, formFields, formFiles, keepAlive, keepAliveIdle, keepAliveInterval);
}

NetworkReadStream *NetworkReadStream::make(const char *url, RequestHeaders *headersList, const byte *buffer, uint32 bufferSize, bool uploading, bool usingPatch, bool post, bool keepAlive, long keepAliveIdle, long keepAliveInterval) {
	return new NetworkReadStreamEmscripten(url, headersList, buffer, bufferSize, uploading, usingPatch, post, keepAlive, keepAliveIdle, keepAliveInterval);
}

void NetworkReadStreamEmscripten::emscriptenOnReadyStateChange(emscripten_fetch_t *fetch) {
	if (fetch->readyState != 2)
		return;

	size_t headersLengthBytes = emscripten_fetch_get_response_headers_length(fetch) + 1;
	char *headerString = (char *)malloc(headersLengthBytes);

	assert(headerString);
	emscripten_fetch_get_response_headers(fetch, headerString, headersLengthBytes);
	NetworkReadStreamEmscripten *stream = (NetworkReadStreamEmscripten *)fetch->userData;
	stream->addResponseHeaders(headerString, headersLengthBytes);
	free(headerString);
}

void NetworkReadStreamEmscripten::emscriptenOnProgress(emscripten_fetch_t *fetch) {
	/*
	if (fetch->totalBytes) {
		debug(5,"Downloading %s.. %.2f percent complete.", fetch->url, fetch->dataOffset * 100.0 / fetch->totalBytes);
	} else {
		debug(5,"Downloading %s.. %lld bytes complete.", fetch->url, fetch->dataOffset + fetch->numBytes);
	}
	debug(5,"Downloading %s.. %.2f %s complete. HTTP readyState: %hu. HTTP status: %hu - "
			"HTTP statusText: %s. Received chunk [%llu, %llu]",
			fetch->url,
			fetch->totalBytes > 0 ? (fetch->dataOffset + fetch->numBytes) * 100.0 / fetch->totalBytes : (fetch->dataOffset + fetch->numBytes),
			fetch->totalBytes > 0 ? "percent" : " bytes",
			fetch->readyState,
			fetch->status,
			fetch->statusText,
			fetch->dataOffset,
			fetch->dataOffset + fetch->numBytes);
	*/
	NetworkReadStreamEmscripten *stream = (NetworkReadStreamEmscripten *)fetch->userData;
	if (stream) {
		stream->setProgress(fetch->dataOffset, fetch->totalBytes);
	}
}

void NetworkReadStreamEmscripten::emscriptenOnSuccess(emscripten_fetch_t *fetch) {
	NetworkReadStreamEmscripten *stream = (NetworkReadStreamEmscripten *)fetch->userData;
	stream->emscriptenDownloadFinished(true);
}

void NetworkReadStreamEmscripten::emscriptenOnError(emscripten_fetch_t *fetch) {
	NetworkReadStreamEmscripten *stream = (NetworkReadStreamEmscripten *)fetch->userData;
	stream->emscriptenDownloadFinished(false);
}

void NetworkReadStreamEmscripten::emscriptenDownloadFinished(bool success) {
	_requestComplete = true;
	if (_emscripten_fetch->numBytes > 0) {
		// TODO: This could be done continuously during emscriptenOnProgress?
		this->_backingStream.write(_emscripten_fetch->data, _emscripten_fetch->numBytes);
	}
	this->setProgress(_emscripten_fetch->numBytes, _emscripten_fetch->numBytes);

	if (success) {
		debug(5, "NetworkReadStreamEmscripten::emscriptenHandleDownload Finished downloading %llu bytes from URL %s. HTTP status code: %d", _emscripten_fetch->numBytes, _emscripten_fetch->url, _emscripten_fetch->status);
		_success = true; // TODO: actually pass the result code from emscripten_fetch
	} else {
		debug(5, "NetworkReadStreamEmscripten::emscriptenHandleDownload Downloading %s failed, HTTP failure status code: %d, status text: %s", _emscripten_fetch->url, _emscripten_fetch->status, _emscripten_fetch->statusText);

		// Make a copy of the error message since _emscripten_fetch might be cleaned up
		if (_emscripten_fetch && _emscripten_fetch->statusText) {
			_errorBuffer = strdup(_emscripten_fetch->statusText);
		} else {
			_errorBuffer = strdup("Unknown error");
		}
		warning("NetworkReadStreamEmscripten::finished %s - Request failed (%s)", _emscripten_fetch_url, getError());
	}
}

void NetworkReadStreamEmscripten::resetStream() {
	_eos = _requestComplete = false;
	_sendingContentsSize = _sendingContentsPos = 0;
	_progressDownloaded = _progressTotal = 0;
	_emscripten_fetch = nullptr;
	_emscripten_request_headers = nullptr;
	free(_errorBuffer);
	_errorBuffer = nullptr;
}

void NetworkReadStreamEmscripten::initEmscripten(const char *url, RequestHeaders *headersList) {

	resetStream();
	emscripten_fetch_attr_init(_emscripten_fetch_attr);

	// convert header list
	// first get size of list
	int size = 0;
	if (headersList) {
		size = headersList->size();
		debug(5, "_emscripten_request_headers count: %d", size);
	}
	_emscripten_request_headers = new char *[size * 2 + 1];
	_emscripten_request_headers[size * 2] = 0; // header array needs to be null-terminated.

	int i = 0;
	if (headersList) {
		for (const Common::String &header : *headersList) {
			// Find the colon separator
			uint colonPos = header.findFirstOf(':');
			if (colonPos == Common::String::npos) {
				warning("NetworkReadStreamEmscripten: Malformed header (no colon): %s", header.c_str());
				continue;
			}

			// Split into key and value parts
			Common::String key = header.substr(0, colonPos);
			Common::String value = header.substr(colonPos + 1);

			// Trim whitespace from key and value
			key.trim();
			value.trim();

			// Store key and value as separate strings
			_emscripten_request_headers[i++] = strdup(key.c_str());
			_emscripten_request_headers[i++] = strdup(value.c_str());
			debug(9, "_emscripten_request_headers key='%s' value='%s'", key.c_str(), value.c_str());
		}
	}

	_emscripten_fetch_attr->requestHeaders = _emscripten_request_headers;
	strcpy(_emscripten_fetch_attr->requestMethod, "GET"); // todo: move down to setup buffer contents
	_emscripten_fetch_attr->attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
	_emscripten_fetch_attr->onerror = emscriptenOnError;
	_emscripten_fetch_attr->onprogress = emscriptenOnProgress;
	_emscripten_fetch_attr->onreadystatechange = emscriptenOnReadyStateChange;
	_emscripten_fetch_attr->onsuccess = emscriptenOnSuccess;
	_emscripten_fetch_attr->userData = this;
}
void NetworkReadStreamEmscripten::setupBufferContents(const byte *buffer, uint32 bufferSize, bool uploading, bool usingPatch, bool post) {
	if (uploading) {
		strcpy(_emscripten_fetch_attr->requestMethod, "PUT");
		_emscripten_fetch_attr->requestDataSize = bufferSize;
		_emscripten_fetch_attr->requestData = (const char *)buffer;
	} else if (usingPatch) {
		strcpy(_emscripten_fetch_attr->requestMethod, "PATCH");
	} else {
		if (post || bufferSize != 0) {
			strcpy(_emscripten_fetch_attr->requestMethod, "POST");
			_emscripten_fetch_attr->requestDataSize = bufferSize;
			_emscripten_fetch_attr->requestData = (const char *)buffer;
		}
	}
	debug(5, "NetworkReadStreamEmscripten::setupBufferContents uploading %s usingPatch %s post %s ->method %s", uploading ? "true" : "false", usingPatch ? "true" : "false", post ? "true" : "false", _emscripten_fetch_attr->requestMethod);
	_emscripten_fetch = emscripten_fetch(_emscripten_fetch_attr, _emscripten_fetch_url);
}

void NetworkReadStreamEmscripten::setupFormMultipart(const Common::HashMap<Common::String, Common::String> &formFields, const Common::HashMap<Common::String, Common::Path> &formFiles) {
	// set POST multipart upload form fields/files
	error("NetworkReadStreamEmscripten::setupFormMultipart not implemented");
}

/** Send <postFields>, using POST by default. */
NetworkReadStreamEmscripten::NetworkReadStreamEmscripten(const char *url, RequestHeaders *headersList, const Common::String &postFields,
		bool uploading, bool usingPatch, bool keepAlive, long keepAliveIdle, long keepAliveInterval) :
		_emscripten_fetch_attr(new emscripten_fetch_attr_t()), _emscripten_fetch_url(url), _errorBuffer(nullptr),
		NetworkReadStream(keepAlive, keepAliveIdle, keepAliveInterval) {
	initEmscripten(url, headersList);
	setupBufferContents((const byte *)postFields.c_str(), postFields.size(), uploading, usingPatch, false);
}
/** Send <formFields>, <formFiles>, using POST multipart/form. */
NetworkReadStreamEmscripten::NetworkReadStreamEmscripten(const char *url, RequestHeaders *headersList, const Common::HashMap<Common::String,
		Common::String> &formFields, const Common::HashMap<Common::String, Common::Path> &formFiles, bool keepAlive, long keepAliveIdle,
		long keepAliveInterval) : _emscripten_fetch_attr(new emscripten_fetch_attr_t()), _emscripten_fetch_url(url), _errorBuffer(nullptr),
		NetworkReadStream(keepAlive, keepAliveIdle, keepAliveInterval) {
	initEmscripten(url, headersList);
	setupFormMultipart(formFields, formFiles);
}

/** Send <buffer>, using POST by default. */
NetworkReadStreamEmscripten::NetworkReadStreamEmscripten(const char *url, RequestHeaders *headersList, const byte *buffer, uint32 bufferSize,
		bool uploading, bool usingPatch, bool post, bool keepAlive, long keepAliveIdle, long keepAliveInterval) :
		_emscripten_fetch_attr(new emscripten_fetch_attr_t()), _emscripten_fetch_url(url), _errorBuffer(nullptr),
		NetworkReadStream(keepAlive, keepAliveIdle, keepAliveInterval) {
	initEmscripten(url, headersList);
	setupBufferContents(buffer, bufferSize, uploading, usingPatch, post);
}

NetworkReadStreamEmscripten::~NetworkReadStreamEmscripten() {
	if (_emscripten_fetch) {
		debug(5, "~NetworkReadStreamEmscripten: emscripten_fetch_close");
		emscripten_fetch_close(_emscripten_fetch);
	}

	// Free the headers array and its contents
	if (_emscripten_request_headers) {
		for (int i = 0; _emscripten_request_headers[i] != nullptr; ++i) {
			free(_emscripten_request_headers[i]); // Free each strdup'd string
		}
		delete[] _emscripten_request_headers;
	}
}

uint32 NetworkReadStreamEmscripten::read(void *dataPtr, uint32 dataSize) {
	uint32 actuallyRead = _backingStream.read(dataPtr, dataSize);

	// Only access _emscripten_fetch->url if _emscripten_fetch is valid
	// debug(5,"NetworkReadStreamEmscripten::read %u %s %s %s", actuallyRead, _eos ? "_eos" : "not _eos", _requestComplete ? "_requestComplete" : "_request not Complete", _emscripten_fetch ? _emscripten_fetch->url : "no-url");
	if (actuallyRead == 0) {
		if (_requestComplete)
			_eos = true;
		return 0;
	}

	return actuallyRead;
}

bool NetworkReadStreamEmscripten::hasError() const {
	return !_success;
}

const char *NetworkReadStreamEmscripten::getError() const {
	return _errorBuffer;
}

long NetworkReadStreamEmscripten::httpResponseCode() const {
	// return 200;
	unsigned short responseCode = 0;
	if (_emscripten_fetch)
		responseCode = _emscripten_fetch->status;
	debug(5, "NetworkReadStreamEmscripten::httpResponseCode %hu", responseCode);
	return responseCode;
}

Common::String NetworkReadStreamEmscripten::currentLocation() const {
	debug(5, "NetworkReadStreamEmscripten::currentLocation %s", _emscripten_fetch_url);
	return Common::String(_emscripten_fetch_url);
}

Common::HashMap<Common::String, Common::String> NetworkReadStreamEmscripten::responseHeadersMap() const {

	Common::HashMap<Common::String, Common::String> headers;

	const char *headerString = _responseHeaders.c_str();
	char **responseHeaders = emscripten_fetch_unpack_response_headers(headerString);
	assert(responseHeaders);

	int numHeaders = 0;
	for (; responseHeaders[numHeaders * 2]; ++numHeaders) {
		// Check both the header and its value are present.
		assert(responseHeaders[(numHeaders * 2) + 1]);
		headers[responseHeaders[numHeaders * 2]] = responseHeaders[(numHeaders * 2) + 1];
	}

	emscripten_fetch_free_unpacked_response_headers(responseHeaders);

	return headers;
}

} // namespace Networking
