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

#ifndef BACKENDS_NETWORKING_SDL_NET_READER_H
#define BACKENDS_NETWORKING_SDL_NET_READER_H

#include "common/scummsys.h"
#include "common/str.h"
#include "common/hashmap.h"
#include "common/hash-str.h"

namespace Networking {

enum ReaderState {
	RS_NONE,
	RS_READING_HEADERS,
	RS_READING_CONTENT
};

class Reader {
	///Common::String _headers;
	///Common::String _method, _path, _query, _anchor;
	ReaderState _state;
	Common::String _content;
	uint32 _bytesLeft;

	Common::String _boundary;
	uint32 _contentLength;
	uint32 _availableBytes;

	Common::String _headers;
	Common::String _buffer;

	Common::String _currentFieldName;
	Common::HashMap<Common::String, Common::String> _fields;
	Common::HashMap<Common::String, Common::String> _attachedFiles;

	byte *_window;
	uint32 _windowUsed, _windowSize;

	bool readHeaders(); //true when ended reading
	bool readContent(); //true when ended reading
	void handleHeaders(Common::String headers);
	void handleFileContent(Common::String filename);
	void handleValueContent(Common::String value);

	void makeWindow(uint32 size);
	void freeWindow();
	///bool Reader::readOneByteInStream(stream);
	bool Reader::readOneByteInString(Common::String &buffer, const Common::String &boundary);

	byte readOne();
	uint32 bytesLeft();

public:
	Reader();
	~Reader();

	bool readResponse(); //true when ended reading
	void setBytesLeft(uint32 b);
};

} // End of namespace Networking

#endif
