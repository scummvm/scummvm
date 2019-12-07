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

#ifndef BACKENDS_NETWORKING_CURL_CONNECTIONMANAGER_H
#define BACKENDS_NETWORKING_CURL_CONNECTIONMANAGER_H

#include "backends/networking/curl/request.h"
#include "common/str.h"
#include "common/singleton.h"
#include "common/hashmap.h"
#include "common/mutex.h"

typedef void CURL;
typedef void CURLM;
struct curl_slist;

namespace Networking {

class NetworkReadStream;

class ConnectionManager : public Common::Singleton<ConnectionManager> {
	static const uint32 FRAMES_PER_SECOND = 25;
	static const uint32 TIMER_INTERVAL = 1000000 / FRAMES_PER_SECOND;
	static const uint32 CLOUD_PERIOD = 1; //every frame
	static const uint32 CURL_PERIOD = 1; //every frame
	static const uint32 DEBUG_PRINT_PERIOD = FRAMES_PER_SECOND; // once per second

	friend void connectionsThread(void *); //calls handle()

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

		RequestWithCallback(Request *rq = nullptr, RequestCallback cb = nullptr): request(rq), onDeleteCallback(cb) {}
	};

	CURLM *_multi;
	bool _timerStarted;
	Common::Array<RequestWithCallback> _requests, _addedRequests;
	Common::Mutex _handleMutex, _addedRequestsMutex;
	uint32 _frame;

	void startTimer(int interval = TIMER_INTERVAL);
	void stopTimer();
	void handle();
	void interateRequests();
	void processTransfers();
	bool hasAddedRequests();

public:
	ConnectionManager();
	virtual ~ConnectionManager();

	/**
	 * All libcurl transfers are going through this ConnectionManager.
	 * So, if you want to start any libcurl transfer, you must create
	 * an easy handle and register it using this method.
	 */
	void registerEasyHandle(CURL *easy) const;

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

	/** Return URL-encoded version of given string. */
	Common::String urlEncode(Common::String s) const;

	static uint32 getCloudRequestsPeriodInMicroseconds();

	/** Return the path to the CA certificates bundle. */
	static const char *getCaCertPath();
};

/** Shortcut for accessing the connection manager. */
#define ConnMan     Networking::ConnectionManager::instance()

} // End of namespace Networking

#endif
