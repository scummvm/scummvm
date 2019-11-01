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
#include "common/str.h"

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

/**
 * ErrorResponse is a struct to be returned from Request
 * to user's failure callbacks.
 *
 * It keeps a Request pointer together with some useful
 * information fields, which would explain why failure
 * callback was called.
 *
 * <interrupted> flag is set when Request was interrupted,
 * i.e. finished by user with finish() call.
 *
 * <failed> flag is set when Request has failed because of
 * some error (bad server response, for example).
 *
 * <response> contains server's original response.
 *
 * <httpResponseCode> contains server's HTTP response code.
 */

struct ErrorResponse {
	Request *request;
	bool interrupted;
	bool failed;
	Common::String response;
	long httpResponseCode;

	ErrorResponse(Request *rq, Common::String resp);
	ErrorResponse(Request *rq, bool interrupt, bool failure, Common::String resp, long httpCode);
};

typedef Response<void *> DataResponse;
typedef Common::BaseCallback<DataResponse> *DataCallback;
typedef Common::BaseCallback<ErrorResponse> *ErrorCallback;

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
 *
 * Request get a success and failure callbacks. Request must call one
 * (and only one!) of these callbacks when it sets FINISHED state.
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
	* Callback, which should be called when Request is failed/interrupted.
	* That's the way Requests pass error information to the code which asked to create this request.
	* @note callback must be called in finish() or similar method.
	*/
	ErrorCallback _errorCallback;

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

	/** Sets FINISHED state and calls the _errorCallback with given error. */
	virtual void finishError(ErrorResponse error);

	/** Sets FINISHED state. Implementations might extend it if needed. */
	virtual void finishSuccess();

public:
	Request(DataCallback cb, ErrorCallback ecb);
	virtual ~Request();

	/** Method, which does actual work. Depends on what this Request is doing. */
	virtual void handle() = 0;

	/** Method, which is called by ConnectionManager when Request's state is RETRY.  */
	virtual void handleRetry();

	/** Method, which is used to restart the Request. */
	virtual void restart() = 0;

	/** Method, which is called to pause the Request. */
	virtual void pause();

	/**
	 * Method, which is called to *interrupt* the Request.
	 * When it's called, Request must stop its work and
	 * call the failure callback to notify user.
	 */
	virtual void finish();

	/** Method, which is called to retry the Request. */
	virtual void retry(uint32 seconds);

	/** Returns Request's current state. */
	RequestState state() const;

	/**
	 * Return date this Request received from server.
	 * It could be extracted from "Date" header,
	 * which is kept in NetworkReadStream.
	 *
	 * @note not all Requests do that, so "" is returned
	 * to indicate the date is unknown. That's also true
	 * if no server response available or no "Date" header
	 * was passed.
	 *
	 * @returns date from "Date" response header.
	 */
	virtual Common::String date() const;
};

} // End of namespace Networking

#endif
