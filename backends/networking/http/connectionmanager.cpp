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

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "backends/networking/http/connectionmanager.h"
#include "backends/networking/http/networkreadstream.h"
#include "common/debug.h"
#include "common/fs.h"
#include "common/system.h"
#include "common/timer.h"

#if defined(USE_LIBCURL)
#include <curl/curl.h>
#endif
#if defined(EMSCRIPTEN)
#include <emscripten.h>
#endif
#if defined(ANDROID_BACKEND)
#include "backends/platform/android/jni-android.h"
#endif

namespace Common {

DECLARE_SINGLETON(Networking::ConnectionManager);

}

namespace Networking {

ConnectionManager::ConnectionManager(): _multi(nullptr), _timerStarted(false), _frame(0) {
#ifdef USE_LIBCURL
	curl_global_init(CURL_GLOBAL_ALL);
	_multi = curl_multi_init();
#endif
}

ConnectionManager::~ConnectionManager() {
	stopTimer();

	//terminate all added requests which haven't been processed yet
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

	//terminate all requests
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

	//cleanup
#ifdef USE_LIBCURL
	curl_multi_cleanup(_multi);
	curl_global_cleanup();
	_multi = nullptr;
#endif
	_handleMutex.unlock();
}

#ifdef USE_LIBCURL
void ConnectionManager::registerEasyHandle(CURL *easy) const {
	curl_multi_add_handle(_multi, easy);
}
#endif

Request *ConnectionManager::addRequest(Request *request, RequestCallback callback) {
	_addedRequestsMutex.lock();
	_addedRequests.push_back(RequestWithCallback(request, callback));
	if (!_timerStarted)
		startTimer();
	_addedRequestsMutex.unlock();
	return request;
}

Common::String ConnectionManager::urlEncode(const Common::String &s) const {
	Common::String result = "";
	debug(5, "ConnectionManager::urlEncode(%s)", s.c_str());
#ifdef USE_LIBCURL
	if (!_multi)
		return "";
#if LIBCURL_VERSION_NUM >= 0x070F04
	char *output = curl_easy_escape(_multi, s.c_str(), s.size());
#else
	char *output = curl_escape(s.c_str(), s.size());
#endif
	if (output) {
		result = output;
		curl_free(output);
	}
#elif defined(EMSCRIPTEN)
	const char *input = s.c_str();
	result = (char*) EM_ASM_PTR({
		var jsString = encodeURIComponent(UTF8ToString($0));
		var size = lengthBytesUTF8(jsString) + 1;
		var ret = Module._malloc(size);
		stringToUTF8Array(jsString, HEAP8, ret, size);
		return ret;
	}, input);
#endif
	debug(5, "ConnectionManager::urlEncode(%s) = %s", s.c_str(), result.c_str());
		
	return result;
}

uint32 ConnectionManager::getCloudRequestsPeriodInMicroseconds() {
	return TIMER_INTERVAL * CLOUD_PERIOD;
}

Common::String ConnectionManager::getCaCertPath() {
#if defined(ANDROID_BACKEND)
	// cacert path must exist on filesystem and be reachable by standard open syscall
	// Lets use ScummVM internal directory
	Common::String assetsPath = JNI::getScummVMAssetsPath();
	return assetsPath + "/cacert.pem";
#elif defined(DATA_PATH)
	static enum {
		kNotInitialized,
		kFileNotFound,
		kFileExists
	} state = kNotInitialized;

	if (state == kNotInitialized) {
		Common::FSNode node(DATA_PATH"/cacert.pem");
		state = node.exists() ? kFileExists : kFileNotFound;
	}

	if (state == kFileExists) {
		return DATA_PATH"/cacert.pem";
	} else {
		return "";
	}
#else
	return "";
#endif
}

//private goes here:

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
	//lock mutex here (in case another handle() would be called before this one ends)
	_handleMutex.lock();
	++_frame;
	if (_frame % CLOUD_PERIOD == 0)
		interateRequests();
	if (_frame % CURL_PERIOD == 0)
		processTransfers();

	if (_requests.empty() && !hasAddedRequests())
		stopTimer();
	_handleMutex.unlock();
}

void ConnectionManager::interateRequests() {
	//add new requests
	_addedRequestsMutex.lock();
	for (auto &addedRequest : _addedRequests) {
		_requests.push_back(addedRequest);
	}
	_addedRequests.clear();
	_addedRequestsMutex.unlock();

	//call handle() of all running requests (so they can do their work)
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
				(*i->onDeleteCallback)(i->request); //that's not a mistake (we're passing an address and that method knows there is no object anymore)
				delete i->onDeleteCallback;
			}
			_requests.erase(i);
			continue;
		}

		++i;
	}
}

void ConnectionManager::processTransfers() {
#ifdef USE_LIBCURL
	if (!_multi) return;

	//check libcurl's transfers and notify requests of messages from queue (transfer completion or failure)
	int transfersRunning;
	curl_multi_perform(_multi, &transfersRunning);

	int messagesInQueue;
	CURLMsg *curlMsg;
	while ((curlMsg = curl_multi_info_read(_multi, &messagesInQueue))) {
		if (curlMsg->msg == CURLMSG_DONE) {
			CURL *easyHandle = curlMsg->easy_handle;

			NetworkReadStream *stream = nullptr;
			curl_easy_getinfo(easyHandle, CURLINFO_PRIVATE, &stream);

			if (stream)
				stream->finished(curlMsg->data.result);

			curl_multi_remove_handle(_multi, easyHandle);
		} else {
			warning("Unknown libcurl message type %d", curlMsg->msg);
		}
	}
#endif 
}

} // End of namespace Cloud
