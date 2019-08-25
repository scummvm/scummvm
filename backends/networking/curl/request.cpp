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

#include "backends/networking/curl/request.h"

namespace Networking {

ErrorResponse::ErrorResponse(Request *rq, Common::String resp):
	request(rq), interrupted(false), failed(true), response(resp), httpResponseCode(-1) {}

ErrorResponse::ErrorResponse(Request *rq, bool interrupt, bool failure, Common::String resp, long httpCode):
	request(rq), interrupted(interrupt), failed(failure), response(resp), httpResponseCode(httpCode) {}

Request::Request(DataCallback cb, ErrorCallback ecb):
	_callback(cb), _errorCallback(ecb), _state(PROCESSING), _retryInSeconds(0) {}

Request::~Request() {
	delete _callback;
	delete _errorCallback;
}

void Request::handleRetry() {
	if (_retryInSeconds > 0) {
		--_retryInSeconds;
	} else {
		_state = PROCESSING;
		restart();
	}
}

void Request::pause() { _state = PAUSED; }

void Request::finish() {
	ErrorResponse error(this, true, false, "Request::finish() was called (i.e. interrupted)", -1);
	finishError(error);
}

void Request::retry(uint32 seconds) {
	_state = RETRY;
	_retryInSeconds = seconds;
}

RequestState Request::state() const { return _state; }

Common::String Request::date() const { return ""; }

void Request::finishError(ErrorResponse error) {
	_state = FINISHED;
	if (_errorCallback)
		(*_errorCallback)(error);
}

void Request::finishSuccess() { _state = FINISHED; }

} // End of namespace Networking
