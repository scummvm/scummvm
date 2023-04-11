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

#include "backends/networking/sdl_net/reader.h"
#include "backends/fs/fs-factory.h"
#include "backends/networking/sdl_net/localwebserver.h"
#include "common/memstream.h"
#include "common/stream.h"

namespace Networking {

Reader::Reader() {
	_state = RS_NONE;
	_mode = RM_HTTP_GENERIC;
	_content = nullptr;
	_bytesLeft = 0;

	_window = nullptr;
	_windowUsed = 0;
	_windowSize = 0;
	_windowReadPosition = 0;
	_windowWritePosition = 0;
	_windowHash = 0;

	_headersStream = nullptr;
	_firstBlock = true;

	_contentLength = 0;
	_availableBytes = 0;
	_isBadRequest = false;
	_allContentRead = false;
}

Reader::~Reader() {
	cleanup();
}

Reader &Reader::operator=(Reader &r) {
	if (this == &r)
		return *this;
	cleanup();

	_state = r._state;
	_content = r._content;
	_bytesLeft = r._bytesLeft;
	r._state = RS_NONE;

	_window = r._window;
	_windowUsed = r._windowUsed;
	_windowSize = r._windowSize;
	_windowReadPosition = r._windowReadPosition;
	_windowWritePosition = r._windowWritePosition;
	_windowHash = r._windowHash;
	r._window = nullptr;

	_headersStream = r._headersStream;
	r._headersStream = nullptr;

	_headers = r._headers;
	_method = r._method;
	_path = r._path;
	_query = r._query;
	_anchor = r._anchor;
	_queryParameters = r._queryParameters;
	_contentLength = r._contentLength;
	_boundary = r._boundary;
	_availableBytes = r._availableBytes;
	_firstBlock = r._firstBlock;
	_isBadRequest = r._isBadRequest;
	_allContentRead = r._allContentRead;

	return *this;
}

void Reader::cleanup() {
	//_content is not to be freed, it's not owned by Reader

	if (_headersStream != nullptr)
		delete _headersStream;

	if (_window != nullptr)
		freeWindow();
}

namespace {
uint32 calculateHash(const Common::String& boundary) {
	uint32 result = 0;
	for (uint32 i = 0; i < boundary.size(); ++i)
		result ^= boundary[i];
	return result;
}
}

bool Reader::readAndHandleFirstHeaders() {
	Common::String boundary = "\r\n\r\n";
	uint32 boundaryHash = calculateHash(boundary);
	if (_window == nullptr) {
		makeWindow(boundary.size());
	}
	if (_headersStream == nullptr) {
		_headersStream = new Common::MemoryReadWriteStream(DisposeAfterUse::YES);
	}

	while (readOneByteInStream(_headersStream, boundary, boundaryHash)) {
		if (_headersStream->size() > SUSPICIOUS_HEADERS_SIZE) {
			_isBadRequest = true;
			return true;
		}
		if (!bytesLeft())
			return false;
	}
	handleFirstHeaders(_headersStream);

	freeWindow();
	_state = RS_READING_CONTENT;
	return true;
}

bool Reader::readBlockHeadersIntoStream(Common::WriteStream *stream) {
	Common::String boundary = "\r\n\r\n";
	uint32 boundaryHash = calculateHash(boundary);
	if (_window == nullptr) makeWindow(boundary.size());

	while (readOneByteInStream(stream, boundary, boundaryHash)) {
		if (!bytesLeft())
			return false;
	}
	if (stream) stream->flush();

	freeWindow();
	_state = RS_READING_CONTENT;
	return true;
}

namespace {
void readFromThatUntilLineEnd(const char *cstr, Common::String needle, Common::String &result) {
	const char *position = strstr(cstr, needle.c_str());

	if (position) {
		char c;
		for (const char *i = position + needle.size(); c = *i, c != 0; ++i) {
			if (c == '\n' || c == '\r')
				break;
			result += c;
		}
	}
}
}

void Reader::handleFirstHeaders(Common::MemoryReadWriteStream *headersStream) {
	if (!_boundary.empty()) {
		warning("Reader: handleFirstHeaders() called when first headers were already handled");
		return;
	}

	//parse method, path, query, fragment
	_headers = readEverythingFromMemoryStream(headersStream);
	parseFirstLine(_headers);

	//find boundary
	_boundary = "";
	readFromThatUntilLineEnd(_headers.c_str(), "boundary=", _boundary);

	//find content length
	Common::String contentLength = "";
	readFromThatUntilLineEnd(_headers.c_str(), "Content-Length: ", contentLength);
	_contentLength = contentLength.asUint64();
	_availableBytes = _contentLength;
}

void Reader::parseFirstLine(const Common::String &headersToParse) {
	uint32 headersSize = headersToParse.size();
	bool bad = false;

	if (headersSize > 0) {
		const char *cstr = headersToParse.c_str();
		const char *position = strstr(cstr, "\r\n");
		if (position) { //we have at least one line - and we want the first one
			//"<METHOD> <path> HTTP/<VERSION>\r\n"
			Common::String methodParsed, pathParsed, http, buf;
			uint32 length = position - cstr;
			if (headersSize > length)
				headersSize = length;
			for (uint32 i = 0; i < headersSize; ++i) {
				if (headersToParse[i] != ' ')
					buf += headersToParse[i];
				if (headersToParse[i] == ' ' || i == headersSize - 1) {
					if (methodParsed == "") {
						methodParsed = buf;
					} else if (pathParsed == "") {
						pathParsed = buf;
					} else if (http == "") {
						http = buf;
					} else {
						bad = true;
						break;
					}
					buf = "";
				}
			}

			//check that method is supported
			if (methodParsed != "GET" && methodParsed != "PUT" && methodParsed != "POST" && methodParsed != "OPTIONS")
				bad = true;

			//check that HTTP/<VERSION> is OK
			if (!http.hasPrefix("HTTP/"))
				bad = true;

			_method = methodParsed;
			parsePathQueryAndAnchor(pathParsed);
		}
	}

	if (bad) _isBadRequest = true;
}

void Reader::parsePathQueryAndAnchor(Common::String pathToParse) {
	//<path>[?query][#anchor]
	bool readingPath = true;
	bool readingQuery = false;
	_path = "";
	_query = "";
	_anchor = "";
	for (uint32 i = 0; i < pathToParse.size(); ++i) {
		if (readingPath) {
			if (pathToParse[i] == '?') {
				readingPath = false;
				readingQuery = true;
			} else {
				_path += pathToParse[i];
			}
		} else if (readingQuery) {
			if (pathToParse[i] == '#') {
				readingQuery = false;
			} else {
				_query += pathToParse[i];
			}
		} else {
			_anchor += pathToParse[i];
		}
	}

	parseQueryParameters();
}

void Reader::parseQueryParameters() {
	Common::String key = "";
	Common::String value = "";
	bool readingKey = true;
	for (uint32 i = 0; i < _query.size(); ++i) {
		if (readingKey) {
			if (_query[i] == '=') {
				readingKey = false;
				value = "";
			} else {
				key += _query[i];
			}
		} else {
			if (_query[i] == '&') {
				if (_queryParameters.contains(key))
					warning("Reader: query parameter \"%s\" is already set!", key.c_str());
				else
					_queryParameters[key] = LocalWebserver::urlDecode(value);
				readingKey = true;
				key = "";
			} else {
				value += _query[i];
			}
		}
	}

	if (!key.empty()) {
		if (_queryParameters.contains(key))
			warning("Reader: query parameter \"%s\" is already set!", key.c_str());
		else
			_queryParameters[key] = LocalWebserver::urlDecode(value);
	}
}

bool Reader::readContentIntoStream(Common::WriteStream *stream) {
	Common::String boundary = "--" + _boundary;
	if (!_firstBlock)
		boundary = "\r\n" + boundary;
	if (_boundary.empty())
		boundary = "\r\n";
	if (_window == nullptr)
		makeWindow(boundary.size());

	uint32 boundaryHash = calculateHash(boundary);
	while (readOneByteInStream(stream, boundary, boundaryHash)) {
		if (!bytesLeft())
			return false;
	}

	_firstBlock = false;
	if (stream)
		stream->flush();

	freeWindow();
	_state = RS_READING_HEADERS;
	return true;
}

void Reader::makeWindow(uint32 size) {
	freeWindow();

	_window = new byte[size];
	_windowUsed = 0;
	_windowSize = size;
	_windowReadPosition = 0;
	_windowWritePosition = 0;
	_windowHash = 0;
}

void Reader::freeWindow() {
	delete[] _window;
	_window = nullptr;
	_windowUsed = _windowSize = 0;
	_windowReadPosition = _windowWritePosition = 0;
	_windowHash = 0;
}

namespace {
bool windowEqualsString(const byte *window, uint32 windowStart, uint32 windowSize, const Common::String &boundary) {
	if (boundary.size() != windowSize)
		return false;

	for (uint32 i = 0; i < windowSize; ++i) {
		if (window[(windowStart + i) % windowSize] != boundary[i])
			return false;
	}

	return true;
}
}

bool Reader::readOneByteInStream(Common::WriteStream *stream, const Common::String &boundary, const uint32 boundaryHash) {
	byte b = readOne();
	++_windowUsed;
	_window[_windowWritePosition] = b;
	_windowWritePosition = (_windowWritePosition + 1) % _windowSize;
	_windowHash ^= b;
	if (_windowUsed < _windowSize)
		return true;

	//when window is filled, check whether that's the boundary
	if (_windowHash == boundaryHash && windowEqualsString(_window, _windowReadPosition, _windowSize, boundary))
		return false;

	//if not, add the first byte of the window to the string
	if (stream)
		stream->writeByte(_window[_windowReadPosition]);
	_windowHash ^= _window[_windowReadPosition];
	_windowReadPosition = (_windowReadPosition + 1) % _windowSize;
	--_windowUsed;
	return true;
}

byte Reader::readOne() {
	byte b = 0;
	_content->read(&b, 1);
	--_availableBytes;
	--_bytesLeft;
	return b;
}

/// public

bool Reader::readFirstHeaders() {
	if (_state == RS_NONE)
		_state = RS_READING_HEADERS;

	if (!bytesLeft())
		return false;

	if (_state == RS_READING_HEADERS)
		return readAndHandleFirstHeaders();

	warning("Reader::readFirstHeaders(): bad state");
	return false;
}

bool Reader::readContent(Common::WriteStream* stream) {
	if (_mode != RM_HTTP_GENERIC) {
		warning("Reader::readContent(): bad mode");
		return false;
	}

	if (_state != RS_READING_CONTENT) {
		warning("Reader::readContent(): bad state");
		return false;
	}

	if (!bytesLeft())
		return false;

	byte buffer[1024];
	while (_availableBytes > 0) {
		uint32 bytesRead = _content->read(&buffer, 1024);
		_availableBytes -= bytesRead;
		_bytesLeft -= bytesRead;

		if (stream)
			if (stream->write(buffer, bytesRead) != bytesRead) {
				warning("Reader::readContent(): failed to write buffer to stream");
				return false;
			}

		if (_availableBytes == 0)
			_allContentRead = true;

		if (!bytesLeft())
			break;
	}

	if (stream)
		stream->flush();

	return _allContentRead;
}

bool Reader::readFirstContent(Common::WriteStream *stream) {
	if (_mode != RM_POST_FORM_MULTIPART) {
		warning("Reader::readFirstContent(): bad mode");
		return false;
	}

	if (_state != RS_READING_CONTENT) {
		warning("Reader::readFirstContent(): bad state");
		return false;
	}

	// no difference, actually
	return readBlockContent(stream);
}

bool Reader::readBlockHeaders(Common::WriteStream *stream) {
	if (_mode != RM_POST_FORM_MULTIPART) {
		warning("Reader::readBlockHeaders(): bad mode");
		return false;
	}

	if (_state != RS_READING_HEADERS) {
		warning("Reader::readBlockHeaders(): bad state");
		return false;
	}

	if (!bytesLeft())
		return false;

	return readBlockHeadersIntoStream(stream);
}

bool Reader::readBlockContent(Common::WriteStream *stream) {
	if (_mode != RM_POST_FORM_MULTIPART) {
		warning("Reader::readBlockContent(): bad mode");
		return false;
	}

	if (_state != RS_READING_CONTENT) {
		warning("Reader::readBlockContent(): bad state");
		return false;
	}

	if (!bytesLeft())
		return false;

	if (!readContentIntoStream(stream))
		return false;

	if (_availableBytes >= 2) {
		Common::String bts;
		bts += readOne();
		bts += readOne();
		if (bts == "--")
			_allContentRead = true;
		else if (bts != "\r\n")
			warning("Reader: strange bytes: \"%s\"", bts.c_str());
	} else {
		warning("Reader: strange ending");
		_allContentRead = true;
	}

	return true;
}

uint32 Reader::bytesLeft() const { return _bytesLeft; }

void Reader::setMode(ReaderMode mode) { _mode = mode; }

void Reader::setContent(Common::MemoryReadWriteStream *stream) {
	_content = stream;
	_bytesLeft = stream->size() - stream->pos();
}

bool Reader::badRequest() const { return _isBadRequest; }

bool Reader::noMoreContent() const { return _allContentRead; }

Common::String Reader::headers() const { return _headers; }

Common::String Reader::method() const { return _method; }

Common::String Reader::path() const { return _path; }

Common::String Reader::query() const { return _query; }

Common::String Reader::queryParameter(Common::String name) const { return _queryParameters.getValOrDefault(name); }

Common::String Reader::anchor() const { return _anchor; }

Common::String Reader::readEverythingFromMemoryStream(Common::MemoryReadWriteStream *stream) {
	Common::String result;
	char buf[1024];
	uint32 readBytes;
	while (true) {
		readBytes = stream->read(buf, 1024);
		if (readBytes == 0)
			break;
		result += Common::String(buf, readBytes);
	}
	return result;
}

} // End of namespace Networking
