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

GetClientHandler::GetClientHandler(Common::SeekableReadStream *stream):
	_responseCode(200), _headersPrepared(false),
	_stream(stream), _buffer(new byte[CLIENT_HANDLER_BUFFER_SIZE]) {}

GetClientHandler::~GetClientHandler() {
	delete _stream;
	delete[] _buffer;
}

const char *GetClientHandler::responseMessage(long responseCode) {
	switch (responseCode) {
	case 100: return "Continue";
	case 101: return "Switching Protocols";
	case 102: return "Processing";

	case 200: return "OK";
	case 201: return "Created";
	case 202: return "Accepted";
	case 203: return "Non-Authoritative Information";
	case 204: return "No Content";
	case 205: return "Reset Content";
	case 206: return "Partial Content";
	case 207: return "Multi-Status";
	case 226: return "IM Used";

	case 300: return "Multiple Choices";
	case 301: return "Moved Permanently";
	case 302: return "Moved Temporarily"; //case 302: return "Found";
	case 303: return "See Other";
	case 304: return "Not Modified";
	case 305: return "Use Proxy";
	case 306: return "RESERVED";
	case 307: return "Temporary Redirect";

	case 400: return "Bad Request";
	case 401: return "Unauthorized";
	case 402: return "Payment Required";
	case 403: return "Forbidden";
	case 404: return "Not Found";
	case 405: return "Method Not Allowed";
	case 406: return "Not Acceptable";
	case 407: return "Proxy Authentication Required";
	case 408: return "Request Timeout";
	case 409: return "Conflict";
	case 410: return "Gone";
	case 411: return "Length Required";
	case 412: return "Precondition Failed";
	case 413: return "Request Entity Too Large";
	case 414: return "Request-URI Too Large";
	case 415: return "Unsupported Media Type";
	case 416: return "Requested Range Not Satisfiable";
	case 417: return "Expectation Failed";
	case 422: return "Unprocessable Entity";
	case 423: return "Locked";
	case 424: return "Failed Dependency";
	case 425: return "Unordered Collection";
	case 426: return "Upgrade Required";
	case 428: return "Precondition Required";
	case 429: return "Too Many Requests";
	case 431: return "Request Header Fields Too Large";
	case 434: return "Requested Host Unavailable";
	case 449: return "Retry With";
	case 451: return "Unavailable For Legal Reasons";

	case 500: return "Internal Server Error";
	case 501: return "Not Implemented";
	case 502: return "Bad Gateway";
	case 503: return "Service Unavailable";
	case 504: return "Gateway Timeout";
	case 505: return "HTTP Version Not Supported";
	case 506: return "Variant Also Negotiates";
	case 507: return "Insufficient Storage";
	case 508: return "Loop Detected";
	case 509: return "Bandwidth Limit Exceeded";
	case 510: return "Not Extended";
	case 511: return "Network Authentication Required";
	}
	return "Unknown";
}

void GetClientHandler::prepareHeaders() {
	if (!_specialHeaders.contains("Content-Type"))
		setHeader("Content-Type", "text/html; charset=UTF-8");

	if (!_specialHeaders.contains("Content-Length") && _stream)
		setHeader("Content-Length", Common::String::format("%u", _stream->size()));

	_headers = Common::String::format("HTTP/1.1 %ld %s\r\n", _responseCode, responseMessage(_responseCode));
	for (Common::HashMap<Common::String, Common::String>::iterator i = _specialHeaders.begin(); i != _specialHeaders.end(); ++i)
		_headers += i->_key + ": " + i->_value + "\r\n";
	_headers += "\r\n";

	_headersPrepared = true;
}

void GetClientHandler::handle(Client *client) {
	if (!client)
		return;
	if (!_headersPrepared)
		prepareHeaders();

	uint32 readBytes;

	// send headers first
	if (_headers.size() > 0) {
		readBytes = _headers.size();
		if (readBytes > CLIENT_HANDLER_BUFFER_SIZE)
			readBytes = CLIENT_HANDLER_BUFFER_SIZE;
		memcpy(_buffer, _headers.c_str(), readBytes);
		_headers.erase(0, readBytes);
	} else {
		if (!_stream) {
			client->close();
			return;
		}

		readBytes = _stream->read(_buffer, CLIENT_HANDLER_BUFFER_SIZE);
	}

	if (readBytes != 0)
		if (client->send(_buffer, readBytes) != (int)readBytes) {
			warning("GetClientHandler: unable to send all bytes to the client");
			client->close();
			return;
		}

	// we're done here!
	if (_stream->eos())
		client->close();
}

void GetClientHandler::setHeader(Common::String name, Common::String value) { _specialHeaders[name] = value; }
void GetClientHandler::setResponseCode(long code) { _responseCode = code; }

} // End of namespace Networking
