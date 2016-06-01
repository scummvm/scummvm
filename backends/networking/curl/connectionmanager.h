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

typedef void CURL;
typedef void CURLM;
struct curl_slist;

namespace Networking {

class NetworkReadStream;

class ConnectionManager : public Common::Singleton<ConnectionManager> {
	friend void connectionsThread(void *); //calls handle()

	typedef Common::BaseCallback<Request *> *RequestCallback;

	struct RequestWithCallback { //I'm completely out of ideas
		Request *request;
		RequestCallback callback;
		
		RequestWithCallback(Request *rq = nullptr, RequestCallback cb = nullptr): request(rq), callback(cb) {}
	};

	CURLM *_multi;	
	bool _timerStarted;
	Common::Array<RequestWithCallback> _requests;
	
	void startTimer(int interval = 1000000); //1 second is the default interval
	void stopTimer();
	void handle();
	void interateRequests();
	void processTransfers();

public:
	ConnectionManager();
	virtual ~ConnectionManager();

	/**
	 * All libcurl transfers are going through this ConnectionManager.
	 * So, if you want to start any libcurl transfer, you must create
	 * an easy handle and register it using this method.
	 */
	void registerEasyHandle(CURL *easy);

	/**
	 * Use this method to add new Request into manager's queue.
	 * Manager will periodically call handle() method of these
	 * Requests until they set their state to FINISHED.
	 *
	 * If Request's state is RETRY, handleRetry() is called instead.
	 *
	 * @note This method starts the timer if it's not started yet.
	 *
	 * @return the same Request pointer, just as a shortcut
	 */
	Request *addRequest(Request *request, RequestCallback callback = nullptr);
};

/** Shortcut for accessing the connection manager. */
#define ConnMan		Networking::ConnectionManager::instance()

} // End of namespace Networking

#endif
