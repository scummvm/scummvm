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

#include "backends/networking/http/connectionmanager.h"
#include "backends/networking/http/networkreadstream.h"
#include "common/debug.h"
#include "common/system.h"
#include "common/timer.h"

namespace Common {

DECLARE_SINGLETON(Networking::ConnectionManager);

/* The makeInstance function is defined in the platform specific source file */

} // namespace Common

namespace Networking {
ConnectionManager::ConnectionManager() : _timerStarted(false), _frame(0) {}

ConnectionManager::~ConnectionManager() {
	stopTimer();

	// terminate all added requests which haven't been processed yet
	_addedRequestsMutex.lock();
	for (auto &curRequest : _addedRequests) {
		Request *request = curRequest.request;
		RequestCallback callback = curRequest.onDeleteCallback;
		if (request)
			request->finish();
		delete request;
		if (callback) {
			(*callback)(request);
			delete callback;
		}
	}
	_addedRequests.clear();
	_addedRequestsMutex.unlock();

	// terminate all requests
	_handleMutex.lock();
	for (auto &curRequest : _requests) {
		Request *request = curRequest.request;
		RequestCallback callback = curRequest.onDeleteCallback;
		if (request)
			request->finish();
		delete request;
		if (callback) {
			(*callback)(request);
			delete callback;
		}
	}
	_requests.clear();

	_handleMutex.unlock();
}

Request *ConnectionManager::addRequest(Request *request, RequestCallback callback) {
	_addedRequestsMutex.lock();
	_addedRequests.push_back(RequestWithCallback(request, callback));
	if (!_timerStarted)
		startTimer();
	_addedRequestsMutex.unlock();
	return request;
}

uint32 ConnectionManager::getCloudRequestsPeriodInMicroseconds() {
	return TIMER_INTERVAL * ITERATION_PERIOD;
}

// private goes here:

void connectionsThread(void *ignored) {
	ConnMan.handle();
}

void ConnectionManager::startTimer(int interval) {
	Common::TimerManager *manager = g_system->getTimerManager();
	if (manager->installTimerProc(connectionsThread, interval, nullptr, "Networking::ConnectionManager's Timer")) {
		_timerStarted = true;
	} else {
		warning("Failed to install Networking::ConnectionManager's timer");
	}
}

void ConnectionManager::stopTimer() {
	debug(9, "timer stopped");
	Common::TimerManager *manager = g_system->getTimerManager();
	manager->removeTimerProc(connectionsThread);
	_timerStarted = false;
}

bool ConnectionManager::hasAddedRequests() {
	_addedRequestsMutex.lock();
	bool hasNewRequests = !_addedRequests.empty();
	_addedRequestsMutex.unlock();
	return hasNewRequests;
}

void ConnectionManager::handle() {
	// lock mutex here (in case another handle() would be called before this one ends)
	_handleMutex.lock();
	++_frame;
	if (_frame % ITERATION_PERIOD == 0)
		iterateRequests();
	if (_frame % PROCESSING_PERIOD == 0)
		processTransfers();

	if (_requests.empty() && !hasAddedRequests())
		stopTimer();
	_handleMutex.unlock();
}

void ConnectionManager::iterateRequests() {
	// add new requests
	_addedRequestsMutex.lock();
	for (auto &addedRequest : _addedRequests) {
		_requests.push_back(addedRequest);
	}
	_addedRequests.clear();
	_addedRequestsMutex.unlock();

	// call handle() of all running requests (so they can do their work)
	if (_frame % DEBUG_PRINT_PERIOD == 0)
		debug(9, "handling %d request(s)", _requests.size());
	for (Common::Array<RequestWithCallback>::iterator i = _requests.begin(); i != _requests.end();) {
		Request *request = i->request;
		if (request) {
			if (request->state() == PROCESSING)
				request->handle();
			else if (request->state() == RETRY)
				request->handleRetry();
		}

		if (!request || request->state() == FINISHED) {
			delete (i->request);
			if (i->onDeleteCallback) {
				(*i->onDeleteCallback)(i->request); // that's not a mistake (we're passing an address and that method knows there is no object anymore)
				delete i->onDeleteCallback;
			}
			_requests.erase(i);
			continue;
		}

		++i;
	}
}

} // End of namespace Networking
