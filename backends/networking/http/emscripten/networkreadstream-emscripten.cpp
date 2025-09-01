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

void NetworkReadStreamEmscripten::resetStream() {
	_eos = false;
	_sendingContentsSize = _sendingContentsPos = 0;
	_readPos = 0;
	_fetchId = 0;
}

void NetworkReadStreamEmscripten::initFetch() {
	static bool initialized = false;
	if (!initialized) {
		NetworkReadStreamEmscripten_init();
		initialized = true;
	}
}

char **NetworkReadStreamEmscripten::buildHeadersArray(const RequestHeaders *headersList) {
	if (!headersList || headersList->empty())
		return nullptr;

	const int maxEntries = headersList->size() * 2;
	char **headers = new char *[maxEntries + 1];

	int idx = 0;
	for (const Common::String &header : *headersList) {
		uint colonPos = header.findFirstOf(':');
		if (colonPos == Common::String::npos) {
			warning("NetworkReadStreamEmscripten: Malformed header (no colon): %s", header.c_str());
			continue;
		}

		Common::String key = header.substr(0, colonPos);
		Common::String value = header.substr(colonPos + 1);
		key.trim();
		value.trim();
		headers[idx++] = scumm_strdup(key.c_str());
		headers[idx++] = scumm_strdup(value.c_str());
		debug(5, "Header: '%s' = '%s'", key.c_str(), value.c_str());
	}

	headers[idx] = nullptr;

	return headers;
}

void NetworkReadStreamEmscripten::cleanupStringArray(char **array) {
	if (!array)
		return;

	for (int i = 0; array[i] != nullptr; ++i) {
		free(array[i]);
	}

	delete[] array;
}

char **NetworkReadStreamEmscripten::buildFormFieldsArray(const Common::HashMap<Common::String, Common::String> &formFields) {
	if (formFields.empty())
		return nullptr;

	// Array contains alternating name/value pairs, plus null terminator
	int count = formFields.size() * 2;
	char **fields = new char *[count + 1];

	int idx = 0;
	for (Common::HashMap<Common::String, Common::String>::const_iterator i = formFields.begin(); i != formFields.end(); ++i) {
		fields[idx++] = scumm_strdup(i->_key.c_str());
		fields[idx++] = scumm_strdup(i->_value.c_str());
	}
	fields[idx] = nullptr;

	return fields;
}

char **NetworkReadStreamEmscripten::buildFormFilesArray(const Common::HashMap<Common::String, Common::Path> &formFiles) {
	if (formFiles.empty())
		return nullptr;

	// Array contains alternating name/path pairs, plus null terminator
	int count = formFiles.size() * 2;
	char **files = new char *[count + 1];

	int idx = 0;
	for (Common::HashMap<Common::String, Common::Path>::const_iterator i = formFiles.begin(); i != formFiles.end(); ++i) {
		files[idx++] = scumm_strdup(i->_key.c_str());
		files[idx++] = scumm_strdup(i->_value.toString(Common::Path::kNativeSeparator).c_str());
	}
	files[idx] = nullptr;

	return files;
}

double NetworkReadStreamEmscripten::getProgress() const {
	uint64 numBytes = NetworkReadStreamEmscripten_getNumBytes(_fetchId);
	uint64 totalBytes = NetworkReadStreamEmscripten_totalBytes(_fetchId);
	if (numBytes == 0 || totalBytes == 0) {
		return 0.0; // avoid division by zero or infinite if either is zero
	}
	debug(5, "NetworkReadStreamEmscripten::getProgress - Progress: %llu / %llu for %s", numBytes, totalBytes, _url.c_str());
	return (double)numBytes / (double)totalBytes;
}

/** Send <postFields>, using POST by default. */
NetworkReadStreamEmscripten::NetworkReadStreamEmscripten(const char *url, RequestHeaders *headersList, const Common::String &postFields,
	bool uploading, bool usingPatch, bool keepAlive, long keepAliveIdle, long keepAliveInterval) :
	NetworkReadStream(keepAlive, keepAliveIdle, keepAliveInterval),
	_fetchId(0), _url(url), _headersList(headersList), _readPos(0) {

	initFetch();

	// Determine HTTP method
	const char *method = "GET";
	if (uploading) {
		method = "PUT";
	} else if (usingPatch) {
		method = "PATCH";
	} else if (postFields.size() != 0) {
		method = "POST";
	}

	// Build headers
	char **headers = buildHeadersArray(_headersList);

	debug(5, "Starting fetch: %s %s (body size: %u)", method, _url.c_str(), postFields.size());

	// Start the fetch
	_fetchId = NetworkReadStreamEmscripten_fetch(method, _url.c_str(), headers, postFields.c_str(), postFields.size());

	// Clean up headers
	cleanupStringArray(headers);
}

/** Send <formFields>, <formFiles>, using POST multipart/form. */
NetworkReadStreamEmscripten::NetworkReadStreamEmscripten(const char *url, RequestHeaders *headersList, const Common::HashMap<Common::String, Common::String> &formFields,
	const Common::HashMap<Common::String, Common::Path> &formFiles, bool keepAlive, long keepAliveIdle, long keepAliveInterval) :
	NetworkReadStream(keepAlive, keepAliveIdle, keepAliveInterval) ,
	_fetchId(0), _url(url), _headersList(headersList), _readPos(0){

	initFetch();

	// Build all three arrays
	char **formFieldsArray = buildFormFieldsArray(formFields);
	char **formFilesArray = buildFormFilesArray(formFiles);
	char **headers = buildHeadersArray(_headersList);

	debug(5, "Starting FormData fetch: POST %s with %d fields and %d files", _url.c_str(), formFields.size(), formFiles.size());

	// Call NetworkReadStreamEmscripten_fetch with FormData parameters (method is always POST for FormData)
	_fetchId = NetworkReadStreamEmscripten_fetch("POST", _url.c_str(), headers, nullptr, 0, // no regular request body
												 formFieldsArray, formFilesArray);

	// Clean up allocated strings
	cleanupStringArray(formFieldsArray);
	cleanupStringArray(formFilesArray);
	cleanupStringArray(headers);
}
/** Send <buffer>, using POST by default. */
NetworkReadStreamEmscripten::NetworkReadStreamEmscripten(const char *url, RequestHeaders *headersList, const byte *buffer, uint32 bufferSize,
	bool uploading, bool usingPatch, bool post, bool keepAlive, long keepAliveIdle, long keepAliveInterval) : 
	NetworkReadStream(keepAlive, keepAliveIdle, keepAliveInterval),
	_fetchId(0), _url(url), _headersList(headersList), _readPos(0) {

	initFetch();

	// Determine HTTP method
	const char *method = "GET";
	if (uploading) {
		method = "PUT";
	} else if (usingPatch) {
		method = "PATCH";
	} else if (post || bufferSize != 0) {
		method = "POST";
	}

	// Build headers
	char **headers = buildHeadersArray(_headersList);

	debug(5, "Starting fetch: %s %s (buffer size: %u)", method, _url.c_str(), bufferSize);

	// Start the fetch
	_fetchId = NetworkReadStreamEmscripten_fetch(method, _url.c_str(), headers, (const char *)buffer, bufferSize);

	// Clean up headers
	cleanupStringArray(headers);
}

NetworkReadStreamEmscripten::~NetworkReadStreamEmscripten() {
	debug(5, "NetworkReadStreamEmscripten::~NetworkReadStreamEmscripten %s", _url.c_str());
	if (_fetchId) {
		debug(5, "~NetworkReadStreamEmscripten: NetworkReadStreamEmscripten_close");
		NetworkReadStreamEmscripten_close(_fetchId);
		_fetchId = 0;
	}
}

uint32 NetworkReadStreamEmscripten::read(void *dataPtr, uint32 dataSize) {
	if (!_fetchId || _eos || dataSize == 0) {
		warning("NetworkReadStreamEmscripten::read - Invalid state");
		return 0;
	}

	// Get direct pointer to JS buffer
	char *jsBuffer = NetworkReadStreamEmscripten_getDataPtr(_fetchId);
	if (!jsBuffer) {
		return 0;
	}

	// Get current number of bytes available
	uint32 numBytes = NetworkReadStreamEmscripten_getNumBytes(_fetchId);

	// Calculate how many bytes we can actually read
	uint32 availableBytes = numBytes - _readPos;
	uint32 bytesToRead = (dataSize < availableBytes) ? dataSize : availableBytes;
	uint32 totalBytes = NetworkReadStreamEmscripten_totalBytes(_fetchId);
	debug(5, "NetworkReadStreamEmscripten::read - Progress: %u / %u for %s, currently at %u Trying to read %u bytes", numBytes, totalBytes, _url.c_str(), _readPos, bytesToRead);

	if (bytesToRead == 0) {
		// Check if transfer is complete
		if (NetworkReadStreamEmscripten_completed(_fetchId))
			_eos = true;
		return 0;
	}

	// Copy data directly from JS buffer
	memcpy(dataPtr, jsBuffer + _readPos, bytesToRead);
	_readPos += bytesToRead;

	return bytesToRead;
}

bool NetworkReadStreamEmscripten::hasError() const {
	return NetworkReadStreamEmscripten_hasError(_fetchId) && NetworkReadStreamEmscripten_completed(_fetchId);
}

const char *NetworkReadStreamEmscripten::getError() const {
	if (!hasError() || _fetchId == 0) {
		return "No error";
	}

	return NetworkReadStreamEmscripten_getErr(_fetchId);
}

bool NetworkReadStreamEmscripten::eos() const {
	return _eos;
}

long NetworkReadStreamEmscripten::httpResponseCode() const {
	return _fetchId ? NetworkReadStreamEmscripten_status(_fetchId) : 0;
}

Common::String NetworkReadStreamEmscripten::currentLocation() const {
	return Common::String(_url);
}

Common::HashMap<Common::String, Common::String> NetworkReadStreamEmscripten::responseHeadersMap() const {

	Common::HashMap<Common::String, Common::String> headers;

	if (!_fetchId)
		return headers;

	char **responseHeaders = NetworkReadStreamEmscripten_responseHeadersArray(_fetchId);
	if (!responseHeaders)
		return headers;

	for (int i = 0; responseHeaders[i * 2]; ++i) {
		headers[responseHeaders[i * 2]] = responseHeaders[(i * 2) + 1];
	}

	// Note: No need to free responseHeaders - it's managed by JavaScript side
	return headers;
}

} // namespace Networking
