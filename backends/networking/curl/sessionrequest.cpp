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

#include <curl/curl.h>
#include "backends/networking/curl/connectionmanager.h"
#include "backends/networking/curl/networkreadstream.h"
#include "backends/networking/curl/sessionrequest.h"
#include "common/debug.h"
#include "common/json.h"

namespace Networking {

SessionRequest::SessionRequest(Common::String url, DataCallback cb, ErrorCallback ecb):
	CurlRequest(cb, ecb, url), _contentsStream(DisposeAfterUse::YES),
	_buffer(new byte[CURL_SESSION_REQUEST_BUFFER_SIZE]), _text(nullptr),
	_started(false), _complete(false), _success(false) {}

SessionRequest::~SessionRequest() {
	delete[] _buffer;
}

char *SessionRequest::getPreparedContents() {
	//write one more byte in the end
	byte zero[1] = {0};
	_contentsStream.write(zero, 1);

	//replace all "bad" bytes with '.' character
	byte *result = _contentsStream.getData();
	uint32 size = _contentsStream.size();

	//make it zero-terminated string
	result[size - 1] = '\0';

	return (char *)result;
}

void SessionRequest::finishError(ErrorResponse error) {
	_complete = true;
	_success = false;
	CurlRequest::finishError(error);
}

void SessionRequest::finishSuccess() {
	_state = PAUSED;
	_complete = true;
	_success = true;

	if (_callback)
		(*_callback)(DataResponse(this, text()));
}

void SessionRequest::start() {
	if (_started) {
		warning("Can't start() SessionRequest as it is already started");
	} else {
		_started = true;
		ConnMan.addRequest(this);
	}
}

void SessionRequest::handle() {
	if (!_stream) _stream = makeStream();

	if (_stream) {
		uint32 readBytes = _stream->read(_buffer, CURL_SESSION_REQUEST_BUFFER_SIZE);
		if (readBytes != 0)
			if (_contentsStream.write(_buffer, readBytes) != readBytes)
				warning("SessionRequest: unable to write all the bytes into MemoryWriteStreamDynamic");

		if (_stream->eos()) {			
			finishSuccess();
		}
	}
}

void SessionRequest::restart() {
	if (_stream)
		delete _stream;
	_stream = nullptr;
	_contentsStream = Common::MemoryWriteStreamDynamic(DisposeAfterUse::YES);
	_text = nullptr;
	_complete = false;
	_success = false;
	//with no stream available next handle() will create another one
}

void SessionRequest::close() {
	_state = FINISHED;
}

void SessionRequest::startAndWait() {
	start();
	wait();
}

bool SessionRequest::complete() {
	return _complete;
}

bool SessionRequest::success() {
	return _success;
}

char *SessionRequest::text() {
	if (_text == nullptr)
		_text = getPreparedContents();
	return _text;
}

Common::JSONValue *SessionRequest::json() {
	return Common::JSON::parse(text());
}

} // End of namespace Networking
