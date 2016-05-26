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

enum RequestState {
	PROCESSING,
	PAUSED,
	RETRY,
	FINISHED
};

struct RequestInfo {
	int32 id;
	Request *request;
	RequestState state;
	void *data;
	uint32 retryInSeconds;

	RequestInfo() : id(-1), request(0), state(FINISHED), data(0), retryInSeconds(0) {}
	RequestInfo(int32 rqId, Request *rq) : id(rqId), request(rq), state(PROCESSING), data(0), retryInSeconds(0) {}
};

class ConnectionManager : public Common::Singleton<ConnectionManager> {
	friend void connectionsThread(void *); //calls handle()

	CURLM *_multi;	
	bool _timerStarted;
	Common::HashMap<int32, RequestInfo> _requests;
	int32 _nextId;
	
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
	* Requests until they return true.
	*
	* @note This method starts the timer if it's not started yet.
	*
	* @return generated Request's id, which might be used to get its status
	*/
	int32 addRequest(Request *request);	

	RequestInfo &getRequestInfo(int32 id);
};

/** Shortcut for accessing the connection manager. */
#define ConnMan		Networking::ConnectionManager::instance()

} //end of namespace Networking

#endif
