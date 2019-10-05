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

#include "common/str.h"
#include "common/hashmap.h"
#include "common/hash-str.h"

namespace Common {
class MemoryReadWriteStream;
class WriteStream;
}

namespace Networking {

enum ReaderState {
	RS_NONE,
	RS_READING_HEADERS,
	RS_READING_CONTENT
};

/**
 * This is a helper class for Client.
 *
 * It parses HTTP request and finds headers
 * and content. It also supports POST form/multipart.
 *
 * One might pass the request even byte by byte,
 * Reader will always be able to continue from the
 * state it stopped on.
 *
 * Main headers/content must be read with
 * readFirstHeaders() and readFirstContent() methods.
 * Further headers/content blocks (POST form/multipart)
 * must be read with readBlockHeaders() and readBlockContent().
 *
 * Main headers and parsed URL components could be accessed
 * with special methods after reading.
 *
 * To use the object, call setContent() and then one of those
 * reading methods. It would return whether reading is over
 * or not. If reading is over, content stream still could
 * contain bytes to read with other methods.
 *
 * If reading is not over, Reader awaits you to call the
 * same reading method when you'd get more content.
 *
 * If it's over, you should check whether Reader awaits
 * more content with noMoreContent() and call the other
 * reading method, if it is. When headers are read, one
 * must read contents, and vice versa.
 */

class Reader {
	ReaderState _state;
	Common::MemoryReadWriteStream *_content;
	uint32 _bytesLeft;

	byte *_window;
	uint32 _windowUsed, _windowSize;

	Common::MemoryReadWriteStream *_headersStream;

	Common::String _headers;
	Common::String _method, _path, _query, _anchor;
	Common::HashMap<Common::String, Common::String> _queryParameters;
	uint32 _contentLength;
	Common::String _boundary;
	uint32 _availableBytes;
	bool _firstBlock;
	bool _isBadRequest;
	bool _allContentRead;

	void cleanup();

	bool readAndHandleFirstHeaders(); //true when ended reading
	bool readBlockHeadersIntoStream(Common::WriteStream *stream); //true when ended reading
	bool readContentIntoStream(Common::WriteStream *stream); //true when ended reading

	void handleFirstHeaders(Common::MemoryReadWriteStream *headers);
	void parseFirstLine(const Common::String &headers);
	void parsePathQueryAndAnchor(Common::String pathToParse);
	void parseQueryParameters();

	void makeWindow(uint32 size);
	void freeWindow();
	bool readOneByteInStream(Common::WriteStream *stream, const Common::String &boundary);

	byte readOne();
	uint32 bytesLeft() const;

public:
	static const int32 SUSPICIOUS_HEADERS_SIZE = 1024 * 1024; // 1 MB is really a lot

	Reader();
	~Reader();

	Reader &operator=(Reader &r);

	bool readFirstHeaders(); //true when ended reading
	bool readFirstContent(Common::WriteStream *stream); //true when ended reading
	bool readBlockHeaders(Common::WriteStream *stream); //true when ended reading
	bool readBlockContent(Common::WriteStream *stream); //true when ended reading

	void setContent(Common::MemoryReadWriteStream *stream);

	bool badRequest() const;
	bool noMoreContent() const;

	Common::String headers() const;
	Common::String method() const;
	Common::String path() const;
	Common::String query() const;
	Common::String queryParameter(Common::String name) const;
	Common::String anchor() const;

	static Common::String readEverythingFromMemoryStream(Common::MemoryReadWriteStream *stream);
};

} // End of namespace Networking

#endif
