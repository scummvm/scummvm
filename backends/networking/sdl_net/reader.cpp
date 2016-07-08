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
#include "common/stream.h"
#include "common/memstream.h"
#include "backends/fs/fs-factory.h"

// This define lets us use the system function remove() on Symbian, which
// is disabled by default due to a macro conflict.
// See backends/platform/symbian/src/portdefs.h .
#define SYMBIAN_USE_SYSTEM_REMOVE

#ifndef _WIN32_WCE
#include <errno.h>	// for removeFile()
#endif

namespace Networking {

Reader::Reader(): _randomSource("Networking::Reader") {
	_state = RS_NONE;
	_content = nullptr;
	_bytesLeft = 0;

	_window = nullptr;
	_windowUsed = 0;
	_windowSize = 0;

	_headers = "";
	_stream = nullptr;

	_contentLength = 0;
	_availableBytes = 0;
	_isFileField = false;
	_isBadRequest = false;
}

namespace {
bool removeFile(const char *filename) {
	// FIXME: remove does not exist on all systems. If your port fails to
	// compile because of this, please let us know (scummvm-devel).
	// There is a nicely portable workaround, too: Make this method overloadable.
	if (remove(filename) != 0) {
#ifndef _WIN32_WCE
		if (errno == EACCES)
			error("Reader: removeFile(): Search or write permission denied: %s", filename);

		if (errno == ENOENT)
			error("Reader: removeFile(): '%s' does not exist or path is invalid", filename);
#endif
		return false;
	} else {
		return true;
	}
}
}

Reader::~Reader() {
	cleanup();
}

Reader &Reader::operator=(Reader &r) {
	if (this == &r) return *this;
	cleanup();

	_state = r._state;
	_content = r._content;
	_bytesLeft = r._bytesLeft;
	r._state = RS_NONE;

	_window = r._window;
	_windowUsed = r._windowUsed;
	_windowSize = r._windowSize;
	r._window = nullptr;

	_headers = r._headers;
	_stream = r._stream;
	r._stream = nullptr;

	_headers = r._headers;
	_method = r._method;
	_path = r._path;
	_query = r._query;
	_anchor = r._anchor;
	_queryParameters = r._queryParameters;
	_attachedFiles = r._attachedFiles;
	r._attachedFiles.clear();
	_contentLength = r._contentLength;
	_boundary = r._boundary;
	_availableBytes = r._availableBytes;
	_currentFieldName = r._currentFieldName;
	_currentFileName = r._currentFileName;
	_currentTempFileName = r._currentTempFileName;
	_isFileField = r._isFileField;
	_isBadRequest = r._isBadRequest;

	return *this;
}

void Reader::cleanup() {
	//_content is not to be freed, it's not owned by Reader

	if (_window != nullptr) freeWindow();
	delete _stream;

	//delete temp files (by the time Reader is destucted those must be renamed or read)
	for (Common::HashMap<Common::String, Common::String>::iterator i = _attachedFiles.begin(); i != _attachedFiles.end(); ++i) {
		AbstractFSNode *node = g_system->getFilesystemFactory()->makeFileNodePath(i->_value);
		if (node->exists()) removeFile(node->getPath().c_str());
	}
}

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

namespace {
char generateRandomChar(Common::RandomSource &random) {
	int r = random.getRandomNumber(36);
	char c = '0' + r;
	if (r > 9) c = 'a' + r - 10;
	return c;
}

Common::String generateTempFileName(Common::String originalFilename, Common::RandomSource &random) {
	//generates "./<originalFilename>-<uniqueSequence>.scummtmp"
	//normalize <originalFilename>
	Common::String prefix = "./";
	for (uint32 i = 0; i < originalFilename.size(); ++i) {
		char c = originalFilename[i];
		if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9') || c == '.' || c == '_' || c == '-') {
			prefix += c;
		} else {
			prefix += '_';
		}
	}
	prefix += '-';

	//generate initial sequence
	Common::String uniqueSequence;
	for (uint32 i = 0; i < 5; ++i)
		uniqueSequence += generateRandomChar(random);

	//update sequence while generate path exists
	AbstractFSNode *node;
	Common::String path;
	do {
		uniqueSequence += generateRandomChar(random);
		path = prefix + uniqueSequence + ".scummtmp";
		node = g_system->getFilesystemFactory()->makeFileNodePath(path);
	} while (node->exists());

	return path;
}
}

bool Reader::readContent() {
	Common::String boundary = "--" + _boundary;
	if (_window == nullptr) {
		makeWindow(boundary.size());

		if (_stream) delete _stream;
		if (_isFileField) {
			//create temporary file
			_currentTempFileName = generateTempFileName(_currentFileName, _randomSource);
			AbstractFSNode *node = g_system->getFilesystemFactory()->makeFileNodePath(_currentTempFileName);
			_stream = node->createWriteStream();
			if (_stream == nullptr)
				error("Unable to open temp file to write into!");
		} else {
			_stream = new Common::MemoryReadWriteStream(DisposeAfterUse::YES);
		}
	}

	while (readOneByteInStream(_stream, boundary)) {
		if (!bytesLeft()) return false;
	}

	if (_isFileField) {
		if (_stream != nullptr) {
			_stream->flush();
			delete _stream;
			_stream = nullptr;
		} else {
			warning("No stream was created!");
		}
		handleFileContent(_currentTempFileName);
	} else {
		Common::MemoryReadWriteStream *dynamicStream = dynamic_cast<Common::MemoryReadWriteStream *>(_stream);
		if (dynamicStream != nullptr)
			if (dynamicStream->size() == 0)
				handleValueContent("");
			else
				handleValueContent(Common::String((char *)dynamicStream->getData(), dynamicStream->size()));
		else
			if (_stream != nullptr)
				warning("Stream somehow changed its type from MemoryReadWriteStream!");
			else
				warning("No stream was created!");
	}

	freeWindow();
	_state = RS_READING_HEADERS;
	return true;
}

void Reader::handleFileContent(Common::String filename) {
	debug("\nHANDLE FILE CONTENT:\nFILE >>%s<< SAVED INTO >>%s<<", _currentFileName.c_str(), filename.c_str());
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

bool Reader::readOneByteInStream(Common::WriteStream *stream, const Common::String &boundary) {
	byte b = readOne();
	_window[_windowUsed++] = b;
	if (_windowUsed < _windowSize) return true;

	//when window is filled, check whether that's the boundary
	if (Common::String((char *)_window, _windowSize) == boundary)
		return false;

	//if not, add the first byte of the window to the string
	stream->writeByte(_window[0]);
	for (uint32 i = 1; i < _windowSize; ++i)
		_window[i - 1] = _window[i];
	--_windowUsed;
	return true;
}

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
