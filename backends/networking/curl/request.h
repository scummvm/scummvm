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

#ifndef BACKENDS_NETWORKING_CURL_REQUEST_H
#define BACKENDS_NETWORKING_CURL_REQUEST_H

#include "common/callback.h"
#include "common/scummsys.h"

namespace Networking {

class Request;

/**
 * Response<T> is a struct to be returned from Request
 * to user's callbacks. It's a type safe way to indicate
 * which "return value" Request has and user awaits.
 *
 * It just keeps a Request pointer together with
 * some T value (which might be a pointer, a reference
 * or a plain type (copied by value)).
 *
 * To make it more convenient, typedefs are used.
 * For example, Response<void *> is called DataResponse
 * and corresponding callback pointer is DataCallback.
 */

template<typename T> struct Response {
	Request *request;
	T value;

	Response(Request *rq, T v) : request(rq), value(v) {}
};

typedef Response<void *> DataReponse;
typedef Common::BaseCallback<DataReponse> *DataCallback;

/**
 * RequestState is used to indicate current Request state.
 * ConnectionManager uses it to decide what to do with the Request.
 *
 * PROCESSING state indicates that Request is working.
 * ConnectionManager calls handle() method of Requests in that state.
 *
 * PAUSED state indicates that Request is not working.
 * ConnectionManager keeps Requests in that state and doesn't call any methods of those.
 *
 * RETRY state indicates that Request must restart after a few seconds.
 * ConnectionManager calls handleRetry() method of Requests in that state.
 * Default handleRetry() implementation decreases _retryInSeconds value
 * until it reaches zero. When it does, Request's restart() method is called.
 *
 * FINISHED state indicates that Request did the work and might be deleted.
 * ConnectionManager deletes Requests in that state.
 * After this state is set, but before ConnectionManager deletes the Request,
 * Request calls user's callback. User can ask Request to change its state
 * by calling retry() or pause() methods and Request won't be deleted.
 */
enum RequestState {
	PROCESSING,
	PAUSED,
	RETRY,
	FINISHED
};

class Request {
protected:
	/**
	 * Callback, which should be called when Request is finished.
	 * That's the way Requests pass the result to the code which asked to create this request.
	 *
	 * @note some Requests use their own callbacks to return something but void *.
	 * @note callback must be called in finish() or similar method.
	 */
	DataCallback _callback;

	/**
	 * Request state, which is used by ConnectionManager to determine
	 * whether request might be deleted or it's still working.
	 *
	 * State might be changed from outside with finish(), pause() or
	 * retry() methods. Override these if you want to react to these
	 * changes correctly.
	 */
	RequestState _state;

	/** In RETRY state this indicates whether it's time to call restart(). */
	uint32 _retryInSeconds;

public:
	Request(DataCallback cb): _callback(cb), _state(PROCESSING), _retryInSeconds(0) {}
	virtual ~Request() { delete _callback; }

	/** Method, which does actual work. Depends on what this Request is doing. */
	virtual void handle() = 0;

	/** Method, which is called by ConnectionManager when Request's state is RETRY.	 */
	virtual void handleRetry() {
		if (_retryInSeconds > 0) --_retryInSeconds;
		else {
			_state = PROCESSING;
			restart();
		}
	}

	/** Method, which is used to restart the Request. */
	virtual void restart() = 0;

	/** Method, which is called to pause the Request. */
	virtual void pause() { _state = PAUSED; }

	/**
	 * Method, which is called to *interrupt* the Request.
	 * When it's called, Request must stop its work and
	 * call the callback to notify user of failure.
	 */
	virtual void finish() { _state = FINISHED; }

	/** Method, which is called to retry the Request. */
	virtual void retry(uint32 seconds) {		
		_state = RETRY;
		_retryInSeconds = seconds;
	}

	/** Returns Request's current state. */
	RequestState state() const { return _state; }
};

} // End of namespace Cloud

#endif
