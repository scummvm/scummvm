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

#include "backends/networking/sdl_net/reader.h"
#include "backends/networking/sdl_net/localwebserver.h"
#include "common/debug.h"

namespace Networking {

Reader::Reader() {
	_state = RS_NONE;
	_content = nullptr;
	_bytesLeft = 0;

	_window = nullptr;
	_windowUsed = 0;
	_windowSize = 0;

	_headers = "";
	_buffer = "";

	_contentLength = 0;
	_availableBytes = 0;
	_isFileField = false;
	_isBadRequest = false;
}

Reader::~Reader() {}

bool Reader::readRequest() {
	if (_state == RS_NONE) _state = RS_READING_HEADERS;

	while (true) {
		if (!bytesLeft()) return false;

		if (_state == RS_READING_HEADERS)
			if (!readHeaders())
				return false;

		if (_boundary.empty()) return true; //not POST multipart

		if (!bytesLeft()) return false;

		if (_state == RS_READING_CONTENT)
			if (!readContent())
				return false;

		if (_availableBytes >= 2) {
			Common::String bts;
			bts += readOne();
			bts += readOne();
			if (bts == "--") break;
			if (bts == "\r\n") continue;
			warning("strange bytes: \"%s\"", bts);
		} else {
			warning("strange ending");
			break;
		}
	}	
	if (_availableBytes > 0) debug("STRANGE END: %llu bytes left", _availableBytes);
	else debug("END");

	return true;
}

bool Reader::readHeaders() {
	Common::String boundary = "\r\n\r\n";
	if (_window == nullptr) {
		makeWindow(boundary.size());
		_headers = "";
	}

	while (readOneByteInString(_headers, boundary)) {
		if (!bytesLeft()) return false;
	}
	handleHeaders(_headers);

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
			if (c == '\n' || c == '\r') break;
			result += c;
		}
	}
}
}

void Reader::handleHeaders(Common::String headers) {
	debug("\nHANDLE HEADERS:\n>>%s<<", headers.c_str());
	if (_boundary.empty()) {
		//parse method, path, query, fragment
		parseFirstLine(headers);

		//find boundary
		_boundary = "";
		readFromThatUntilLineEnd(headers.c_str(), "boundary=", _boundary);

		//find content length
		Common::String contentLength = "";
		readFromThatUntilLineEnd(headers.c_str(), "Content-Length: ", contentLength);
		_contentLength = contentLength.asUint64();
		_availableBytes = _contentLength;
		debug("BOUNDARY: %s", _boundary.c_str());
		debug("LENGTH: %llu", _contentLength);
	} else {
		//find field name
		_currentFieldName = "";
		readFromThatUntilLineEnd(headers.c_str(), "name=\"", _currentFieldName);
		for (uint32 i = 0; i < _currentFieldName.size(); ++i)
			if (_currentFieldName[i] == '\"') {
				_currentFieldName.erase(i);
				break;
			}
		debug("FIELD NAME: >>%s<<", _currentFieldName.c_str());

		//find out field type
		_currentFileName = "";
		readFromThatUntilLineEnd(headers.c_str(), "filename=\"", _currentFileName);
		for (uint32 i = 0; i < _currentFileName.size(); ++i)
			if (_currentFileName[i] == '\"') {
				_currentFileName.erase(i);
				break;
			}

		if (!_currentFileName.empty()) {
			_isFileField = true;
			_queryParameters[_currentFieldName] = _currentFileName;
			debug("FILE NAME: >>%s<<", _currentFileName.c_str());
		} else {
			_isFileField = false;
		}
	}
}

void Reader::parseFirstLine(const Common::String &headers) {
	uint32 headersSize = headers.size();
	bool bad = false;

	const uint32 SUSPICIOUS_HEADERS_SIZE = 128 * 1024;
	if (headersSize > SUSPICIOUS_HEADERS_SIZE) bad = true;

	if (!bad) {
		if (headersSize > 0) {
			const char *cstr = headers.c_str();
			const char *position = strstr(cstr, "\r\n");
			if (position) { //we have at least one line - and we want the first one
							//"<METHOD> <path> HTTP/<VERSION>\r\n"
				Common::String method, path, http, buf;
				uint32 length = position - cstr;
				if (headersSize > length) headersSize = length;
				for (uint32 i = 0; i < headersSize; ++i) {
					if (headers[i] != ' ') buf += headers[i];
					if (headers[i] == ' ' || i == headersSize - 1) {
						if (method == "") method = buf;
						else if (path == "") path = buf;
						else if (http == "") http = buf;
						else {
							bad = true;
							break;
						}
						buf = "";
					}
				}

				//check that method is supported
				if (method != "GET" && method != "PUT" && method != "POST") bad = true;

				//check that HTTP/<VERSION> is OK
				if (!http.hasPrefix("HTTP/")) bad = true;

				_method = method;
				parsePathQueryAndAnchor(path);
			}
		}
	}

	if (bad) _isBadRequest = true;
}

void Reader::parsePathQueryAndAnchor(Common::String path) {
	//<path>[?query][#anchor]
	bool readingPath = true;
	bool readingQuery = false;
	_path = "";
	_query = "";
	_anchor = "";
	for (uint32 i = 0; i < path.size(); ++i) {
		if (readingPath) {
			if (path[i] == '?') {
				readingPath = false;
				readingQuery = true;
			} else _path += path[i];
		} else if (readingQuery) {
			if (path[i] == '#') {
				readingQuery = false;
			} else _query += path[i];
		} else _anchor += path[i];
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
			} else key += _query[i];
		} else {
			if (_query[i] == '&') {				
				if (_queryParameters.contains(key)) warning("Query parameter \"%s\" is already set!");
				else _queryParameters[key] = LocalWebserver::urlDecode(value);
				readingKey = true;
				key = "";
			} else value += _query[i];
		}
	}

	if (!key.empty()) {
		if (_queryParameters.contains(key)) warning("Query parameter \"%s\" is already set!");
		else _queryParameters[key] = LocalWebserver::urlDecode(value);
	}
}

bool Reader::readContent() {
	Common::String boundary = "--" + _boundary;
	if (_window == nullptr) {
		makeWindow(boundary.size());
		_buffer = "";
	}

	/*
	if (_fieldIsFile) {
		//create temporary file
		tempFileName = generateTempFileName();
		stream = openFileStream(tempFileName);
		//read till "--" + _boundary
		while (readOneByteInStream(stream));
		handleFileContent(tempFileName);
	} else {
	*/
		while (readOneByteInString(_buffer, boundary)) {		
			if (!bytesLeft()) return false;
		}
		handleValueContent(_buffer);
	//}

	freeWindow();
	_state = RS_READING_HEADERS;
	return true;
}

void Reader::handleFileContent(Common::String filename) {
	_attachedFiles[_currentFileName] = filename;
}

void Reader::handleValueContent(Common::String value) {
	debug("\nHANDLE CONTENT:\n>>%s<<", value.c_str());
	_queryParameters[_currentFieldName] = value;
}

void Reader::makeWindow(uint32 size) {
	freeWindow();

	_window = new byte[size];
	_windowUsed = 0;
	_windowSize = size;
}

void Reader::freeWindow() {
	delete[] _window;
	_window = nullptr;
	_windowUsed = _windowSize = 0;
}

/*
bool Reader::readOneByteInStream(stream) {
	b = read(1);
	_window[_windowUsed++] = b;
	if (_windowUsed < _windowSize) return true;

	//when window is filled, check whether that's the boundary
	if (_window == "--" + _boundary)
		return false;

	//if not, write the first byte of the window to the stream
	stream.write(_window[0]);
	for (uint32 i = 1; i < _windowSize; ++i)
		_window[i - 1] = _window[i];
	--_windowUsed;
	return true;
}
*/

bool Reader::readOneByteInString(Common::String &buffer, const Common::String &boundary) {
	byte b = readOne();
	_window[_windowUsed++] = b;
	if (_windowUsed < _windowSize) return true;

	//when window is filled, check whether that's the boundary
	if (Common::String((char *)_window, _windowSize) == boundary)
		return false;

	//if not, add the first byte of the window to the string
	buffer += _window[0];
	for (uint32 i = 1; i < _windowSize; ++i)
		_window[i - 1] = _window[i];
	--_windowUsed;
	return true;
}

byte Reader::readOne() {
	byte b = _content[0];
	++_content;
	--_availableBytes;
	--_bytesLeft;
	return b;
}

uint32 Reader::bytesLeft() { return _bytesLeft; }

void Reader::setContent(byte *buffer, uint32 size) {
	_content = buffer;
	_bytesLeft = size;
}

bool Reader::badRequest() { return _isBadRequest; }

Common::String Reader::method() const { return _method; }

Common::String Reader::path() const { return _path; }

Common::String Reader::query() const { return _query; }

Common::String Reader::queryParameter(Common::String name) const { return _queryParameters[name]; }

Common::String Reader::anchor() const { return _anchor; }

} // End of namespace Networking
