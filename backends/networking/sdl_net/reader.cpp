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
#include <common/debug.h>

namespace Networking {

Reader::Reader() {
	_state = RS_NONE;
	_bytesLeft = 0;

	_contentLength = 0;
	_availableBytes = 0;

	_window = nullptr;
	_windowUsed = 0;
	_windowSize = 0;

	_headers = "";
	_buffer = "";

	_content =
		"POST /upload HTTP/1.1\r\n" \
		"Host: 127.0.0.1:12345\r\n" \
		"User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; rv:47.0) Gecko/20100101 Firefox/47.0\r\n" \
		"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n" \
		"Accept-Language: ru,en-US;q=0.7,en;q=0.3\r\n" \
		"Accept-Encoding: gzip, deflate\r\n" \
		"Referer: http://127.0.0.1:12345/files\r\n" \
		"Connection: keep-alive\r\n" \
		"Content-Type: multipart/form-data; boundary=---------------------------93411339527546\r\n" \
		"Content-Length: 319\r\n" \
		"\r\n" \
		"-----------------------------93411339527546\r\n" \
		"Content-Disposition: form-data; name=\"path\"\r\n" \
		"\r\n" \
		"/root\r\n" \
		"-----------------------------93411339527546\r\n" \
		"Content-Disposition: form-data; name=\"upload_file\"; filename=\"irc.txt\"\r\n" \
		"Content-Type: text/plain\r\n" \
		"\r\n" \
		"shells.fnordserver.eu/1400\r\n" \
		"-----------------------------93411339527546--";
}

Reader::~Reader() {}

bool Reader::readResponse() {
	if (_state == RS_NONE) _state = RS_READING_HEADERS;

	while (true) {
		if (!bytesLeft()) return false;

		if (_state == RS_READING_HEADERS)
			if (!readHeaders())
				return false;

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
		//TODO: parse method, path, query, fragment

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
		//_fieldIsFile = true;
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
	_attachedFiles[_currentFieldName] = filename;
}

void Reader::handleValueContent(Common::String value) {
	debug("\nHANDLE CONTENT:\n>>%s<<", value.c_str());
	_fields[_currentFieldName] = value;
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
	_content.deleteChar(0);
	--_availableBytes;
	--_bytesLeft;
	return b;
}

uint32 Reader::bytesLeft() { return _bytesLeft; }

void Reader::setBytesLeft(uint32 b) { _bytesLeft = b; }

} // End of namespace Networking
