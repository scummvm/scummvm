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

#include "backends/networking/http/networkreadstream.h"
#include "base/version.h"
#include "common/debug.h"

#define CURLE_OK 0x0  // the only CURLcode value used/checked in ScummVM

namespace Networking {

void NetworkReadStream::emscriptenOnReadyStateChange(emscripten_fetch_t *fetch) {
	if (fetch->readyState != 2)
		return;

	size_t headersLengthBytes = emscripten_fetch_get_response_headers_length(fetch) + 1;
	char *headerString = (char *)malloc(headersLengthBytes);

	assert(headerString);
	emscripten_fetch_get_response_headers(fetch, headerString, headersLengthBytes);
	NetworkReadStream *stream = (NetworkReadStream *)fetch->userData;
	stream->addResponseHeaders(headerString, headersLengthBytes);
	free(headerString);
}

void NetworkReadStream::emscriptenOnProgress(emscripten_fetch_t *fetch) {
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
	NetworkReadStream *stream = (NetworkReadStream *)fetch->userData;
	if (stream) {
		stream->setProgress(fetch->dataOffset, fetch->totalBytes);
	}
}

void NetworkReadStream::emscriptenOnSuccess(emscripten_fetch_t *fetch) {
	NetworkReadStream *stream = (NetworkReadStream *)fetch->userData;
	stream->emscriptenDownloadFinished(true);
}

void NetworkReadStream::emscriptenOnError(emscripten_fetch_t *fetch) {
	NetworkReadStream *stream = (NetworkReadStream *)fetch->userData;
	stream->emscriptenDownloadFinished(false);
}

void NetworkReadStream::emscriptenDownloadFinished(bool success) {

	if (success) {
		debug(5, "NetworkReadStream::emscriptenHandleDownload Finished downloading %llu bytes from URL %s. HTTP status code: %d", _emscripten_fetch->numBytes, _emscripten_fetch->url, _emscripten_fetch->status);
	} else {
		debug(5, "NetworkReadStream::emscriptenHandleDownload Downloading %s failed, HTTP failure status code: %d, status text: %s", _emscripten_fetch->url, _emscripten_fetch->status, _emscripten_fetch->statusText);
	}
	if (_emscripten_fetch->numBytes > 0) {
		// TODO: This could be done continuously during emscriptenOnProgress?
		this->_backingStream.write(_emscripten_fetch->data, _emscripten_fetch->numBytes); 
	}
	this->setProgress(_emscripten_fetch->numBytes, _emscripten_fetch->numBytes);
	if (success) {
		this->finished(CURLE_OK); // TODO: actually pass the result code from emscripten_fetch

	} else {
		this->finished(-1);
	}
}

void NetworkReadStream::resetStream() {
	_eos = _requestComplete = false;
	_sendingContentsSize = _sendingContentsPos = 0;
	_progressDownloaded = _progressTotal = 0;
	_bufferCopy = nullptr;
	_emscripten_fetch = nullptr;
	_emscripten_request_headers = nullptr;
}

void NetworkReadStream::initEmscripten(const char *url, RequestHeaders *headersList) {

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
				warning("NetworkReadStream: Malformed header (no colon): %s", header.c_str());
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
void NetworkReadStream::setupBufferContents(const byte *buffer, uint32 bufferSize, bool uploading, bool usingPatch, bool post) {
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
	debug(5, "NetworkReadStream::setupBufferContents uploading %s usingPatch %s post %s ->method %s", uploading ? "true" : "false", usingPatch ? "true" : "false", post ? "true" : "false", _emscripten_fetch_attr->requestMethod);
	_emscripten_fetch = emscripten_fetch(_emscripten_fetch_attr, _emscripten_fetch_url);
}

void NetworkReadStream::setupFormMultipart(const Common::HashMap<Common::String, Common::String> &formFields, const Common::HashMap<Common::String, Common::Path> &formFiles) {
	// set POST multipart upload form fields/files
	error("NetworkReadStream::setupFormMultipart not implemented");
}

/** Send <postFields>, using POST by default. */
NetworkReadStream::NetworkReadStream(const char *url, RequestHeaders *headersList, const Common::String &postFields, bool uploading, bool usingPatch, bool keepAlive, long keepAliveIdle, long keepAliveInterval) : _emscripten_fetch_attr(new emscripten_fetch_attr_t()), _emscripten_fetch_url(url), _backingStream(DisposeAfterUse::YES), _keepAlive(keepAlive), _errorBuffer(nullptr), _errorCode(CURLE_OK) {
	initEmscripten(url, headersList);
	setupBufferContents((const byte *)postFields.c_str(), postFields.size(), uploading, usingPatch, false);
}
/** Send <formFields>, <formFiles>, using POST multipart/form. */
NetworkReadStream::NetworkReadStream(const char *url, RequestHeaders *headersList, const Common::HashMap<Common::String, Common::String> &formFields, const Common::HashMap<Common::String, Common::Path> &formFiles, bool keepAlive, long keepAliveIdle, long keepAliveInterval) : _emscripten_fetch_attr(new emscripten_fetch_attr_t()), _emscripten_fetch_url(url), _backingStream(DisposeAfterUse::YES), _keepAlive(keepAlive), _errorBuffer(nullptr), _errorCode(CURLE_OK) {
	initEmscripten(url, headersList);
	setupFormMultipart(formFields, formFiles);
}

/** Send <buffer>, using POST by default. */
NetworkReadStream::NetworkReadStream(const char *url, RequestHeaders *headersList, const byte *buffer, uint32 bufferSize, bool uploading, bool usingPatch, bool post, bool keepAlive, long keepAliveIdle, long keepAliveInterval) : _emscripten_fetch_attr(new emscripten_fetch_attr_t()), _emscripten_fetch_url(url), _backingStream(DisposeAfterUse::YES), _keepAlive(keepAlive), _errorBuffer(nullptr), _errorCode(CURLE_OK) {
	initEmscripten(url, headersList);
	setupBufferContents(buffer, bufferSize, uploading, usingPatch, post);
}

bool NetworkReadStream::reuse(const char *url, RequestHeaders *headersList, const Common::String &postFields, bool uploading, bool usingPatch) {
	return false; // Not implemented for Emscripten
}

bool NetworkReadStream::reuse(const char *url, RequestHeaders *headersList, const Common::HashMap<Common::String, Common::String> &formFields, const Common::HashMap<Common::String, Common::Path> &formFiles) {
	return false; // Not implemented for Emscripten
}

bool NetworkReadStream::reuse(const char *url, RequestHeaders *headersList, const byte *buffer, uint32 bufferSize, bool uploading, bool usingPatch, bool post) {
	return false; // Not implemented for Emscripten
}

NetworkReadStream::~NetworkReadStream() {
	if (_emscripten_fetch) {
		debug(5, "~NetworkReadStream: emscripten_fetch_close");
		emscripten_fetch_close(_emscripten_fetch);
	}
	
	// Free the headers array and its contents
	if (_emscripten_request_headers) {
		for (int i = 0; _emscripten_request_headers[i] != nullptr; ++i) {
			free(_emscripten_request_headers[i]);  // Free each strdup'd string
		}
		delete[] _emscripten_request_headers;
	}
	
	free(_bufferCopy);
	free(_errorBuffer);
}

bool NetworkReadStream::eos() const {
	return _eos;
}

uint32 NetworkReadStream::read(void *dataPtr, uint32 dataSize) {
	uint32 actuallyRead = _backingStream.read(dataPtr, dataSize);

	// Only access _emscripten_fetch->url if _emscripten_fetch is valid
	// debug(5,"NetworkReadStream::read %u %s %s %s", actuallyRead, _eos ? "_eos" : "not _eos", _requestComplete ? "_requestComplete" : "_request not Complete", _emscripten_fetch ? _emscripten_fetch->url : "no-url");
	if (actuallyRead == 0) {
		if (_requestComplete)
			_eos = true;
		return 0;
	}

	return actuallyRead;
}

void NetworkReadStream::finished(uint32 errorCode) {
	_requestComplete = true;
	_errorCode = errorCode;

	if (_errorCode == CURLE_OK) {
		debug(9, "NetworkReadStream::finished %s - Request succeeded", _emscripten_fetch_url);
	} else {
		// Make a copy of the error message since _emscripten_fetch might be cleaned up
		if (_emscripten_fetch && _emscripten_fetch->statusText) {
			_errorBuffer = strdup(_emscripten_fetch->statusText);
		} else {
			_errorBuffer = strdup("Unknown error");
		}
		warning("NetworkReadStream::finished %s - Request failed (%d - %s)", _emscripten_fetch_url, _errorCode, getError());
	}
}

bool NetworkReadStream::hasError() const {
	return _errorCode != CURLE_OK;
}

const char *NetworkReadStream::getError() const {
	return _errorBuffer;
}

long NetworkReadStream::httpResponseCode() const {
	// return 200;
	unsigned short responseCode = 0;
	if (_emscripten_fetch)
		responseCode = _emscripten_fetch->status;
	debug(5, "NetworkReadStream::httpResponseCode %hu %hu ", _emscripten_fetch->status, responseCode);
	return responseCode;
}

Common::String NetworkReadStream::currentLocation() const {
	debug(5, "NetworkReadStream::currentLocation %s", _emscripten_fetch_url);
	return Common::String(_emscripten_fetch_url);
}

Common::String NetworkReadStream::responseHeaders() const {
	return _responseHeaders;
}

Common::HashMap<Common::String, Common::String> NetworkReadStream::responseHeadersMap() const {

	Common::HashMap<Common::String, Common::String> headers;

	const char *headerString = responseHeaders().c_str();
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

} // namespace Networking
