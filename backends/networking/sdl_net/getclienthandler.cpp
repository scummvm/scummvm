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

#include "backends/networking/sdl_net/getclienthandler.h"
#include "common/textconsole.h"

namespace Networking {

GetClientHandler::GetClientHandler(Common::SeekableReadStream *stream): _responseCode(200), _headersPrepared(false), _stream(stream) {}

GetClientHandler::~GetClientHandler() { delete _stream; }

const char *GetClientHandler::responseMessage(long responseCode) {
	switch (responseCode) {
	case 200: return "OK";
	case 400: return "Bad Request";
	case 404: return "Not Found";
	case 500: return "Internal Server Error";
	}
	return "Unknown";
}

void GetClientHandler::prepareHeaders() {
	if (!_specialHeaders.contains("Content-Type"))
		setHeader("Content-Type", "text/html");

	if (!_specialHeaders.contains("Content-Length") && _stream)
		setHeader("Content-Length", Common::String::format("%u", _stream->size()));

	_headers = Common::String::format("HTTP/1.1 %d %s\r\n", _responseCode, responseMessage(_responseCode));
	for (Common::HashMap<Common::String, Common::String>::iterator i = _specialHeaders.begin(); i != _specialHeaders.end(); ++i)
		_headers += i->_key + ": " + i->_value + "\r\n";
	_headers += "\r\n";

	_headersPrepared = true;
}

void GetClientHandler::handle(Client *client) {
	if (!client) return;
	if (!_headersPrepared) prepareHeaders();
	
	const int kBufSize = 16 * 1024;
	char buf[kBufSize];
	uint32 readBytes;

	// send headers first
	if (_headers.size() > 0) {
		readBytes = _headers.size();
		if (readBytes > kBufSize) readBytes = kBufSize;
		memcpy(buf, _headers.c_str(), readBytes);
		_headers.erase(0, readBytes);
	} else {
		if (!_stream) {
			client->close();
			return;
		}

		readBytes = _stream->read(buf, kBufSize);
	}

	if (readBytes != 0)
		if (client->send(buf, readBytes) != readBytes) {
			warning("GetClientHandler: unable to send all bytes to the client");
			client->close();
			return;
		}

	// we're done here!
	if (_stream->eos()) client->close();
}

void GetClientHandler::setHeader(Common::String name, Common::String value) { _specialHeaders[name] = value; }
void GetClientHandler::setResponseCode(long code) { _responseCode = code; }

} // End of namespace Networking
