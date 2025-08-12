/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef BACKENDS_NETWORKING_HTTP_CONNECTIONMANAGER_H
#define BACKENDS_NETWORKING_HTTP_CONNECTIONMANAGER_H

#include "backends/networking/http/request.h"
#include "common/hashmap.h"
#include "common/mutex.h"
#include "common/singleton.h"
#include "common/str.h"

namespace Networking {

class ConnectionManager : public Common::Singleton<Networking::ConnectionManager> {
	static const uint32 FRAMES_PER_SECOND = 100;
	static const uint32 TIMER_INTERVAL = 1000000 / FRAMES_PER_SECOND;
	static const uint32 ITERATION_PERIOD = 1;                       // every frame
	static const uint32 PROCESSING_PERIOD = 1;                        // every frame
	static const uint32 DEBUG_PRINT_PERIOD = FRAMES_PER_SECOND; // once per second

	friend void connectionsThread(void *); // calls handle()

	typedef Common::BaseCallback<Request *> *RequestCallback;

	/**
	 * RequestWithCallback is used by ConnectionManager to
	 * storage the Request and a callback which should be
	 * called on Request delete.
	 *
	 * Usually one won't need to pass such callback, but
	 * in some cases you'd like to know whether Request is
	 * still running.
	 *
	 * For example, Cloud::Storage is keeping track of how
	 * many Requests are running, and thus it needs to know
	 * that Request was destroyed to decrease its counter.
	 *
	 * onDeleteCallback is called with *invalid* pointer.
	 * ConnectionManager deletes Request first and then passes
	 * the pointer to the callback. One may use the address
	 * to find it in own HashMap or Array and remove it.
	 * So, again, this pointer is for information only. One
	 * cannot use it.
	 */
	struct RequestWithCallback {
		Request *request;
		RequestCallback onDeleteCallback;

		RequestWithCallback(Request *rq = nullptr, RequestCallback cb = nullptr) : request(rq), onDeleteCallback(cb) {}
	};

	bool _timerStarted;
	Common::Array<RequestWithCallback> _requests, _addedRequests;
	Common::Mutex _handleMutex, _addedRequestsMutex;
	uint32 _frame;

	void startTimer(int interval = TIMER_INTERVAL);
	void stopTimer();
	void handle();
	void iterateRequests();
	virtual void processTransfers() = 0;
	bool hasAddedRequests();

public:
	ConnectionManager();
	~ConnectionManager();

	/**
	 * Use this method to add new Request into manager's queue.
	 * Manager will periodically call handle() method of these
	 * Requests until they set their state to FINISHED.
	 *
	 * If Request's state is RETRY, handleRetry() is called instead.
	 *
	 * The passed callback would be called after Request is deleted.
	 *
	 * @note This method starts the timer if it's not started yet.
	 *
	 * @return the same Request pointer, just as a shortcut
	 */
	Request *addRequest(Request *request, RequestCallback callback = nullptr);

	static uint32 getCloudRequestsPeriodInMicroseconds();
};

/** Shortcut for accessing the connection manager. */
#define ConnMan Networking::ConnectionManager::instance()

} // End of namespace Networking

namespace Common {
template<>
Networking::ConnectionManager *Singleton<Networking::ConnectionManager>::makeInstance();
} // End of namespace Common

#endif
