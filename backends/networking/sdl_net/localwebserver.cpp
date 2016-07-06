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

#include "backends/networking/sdl_net/localwebserver.h"
#include "backends/networking/sdl_net/getclienthandler.h"
#include "common/memstream.h"
#include "common/str.h"
#include "common/system.h"
#include "common/timer.h"
#include "common/textconsole.h"
#include <SDL/SDL_net.h>

namespace Common {
class MemoryReadWriteStream;

DECLARE_SINGLETON(Networking::LocalWebserver);

}

namespace Networking {

LocalWebserver::LocalWebserver(): _set(nullptr), _serverSocket(nullptr), _timerStarted(false),
	_stopOnIdle(false), _clients(0), _idlingFrames(0) {
	addPathHandler("/", _indexPageHandler.getHandler());
	addPathHandler("/files", _filesPageHandler.getHandler());
	_defaultHandler = _resourceHandler.getHandler();
}

LocalWebserver::~LocalWebserver() {
	stop();
}

void localWebserverTimer(void *ignored) {
	LocalServer.handle();
}

void LocalWebserver::startTimer(int interval) {
	Common::TimerManager *manager = g_system->getTimerManager();
	if (manager->installTimerProc(localWebserverTimer, interval, 0, "Networking::LocalWebserver's Timer")) {
		_timerStarted = true;
	} else {
		warning("Failed to install Networking::LocalWebserver's timer");
	}
}

void LocalWebserver::stopTimer() {	
	Common::TimerManager *manager = g_system->getTimerManager();
	manager->removeTimerProc(localWebserverTimer);
	_timerStarted = false;
}

void LocalWebserver::start() {
	_handleMutex.lock();
	_stopOnIdle = false;
	if (_timerStarted) {
		_handleMutex.unlock();
		return;
	}
	startTimer();

	// Create a listening TCP socket
	IPaddress ip;	
	if (SDLNet_ResolveHost(&ip, NULL, SERVER_PORT) == -1) {
		error("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
	}

	_address = Common::String::format("http://127.0.0.1:%u/ (unresolved)", SERVER_PORT);

	const char *name = SDLNet_ResolveIP(&ip);
	if (name == NULL) {
		warning("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
	} else {
		IPaddress localIp;
		if (SDLNet_ResolveHost(&localIp, name, SERVER_PORT) == -1) {
			warning("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
		} else {
			_address = Common::String::format(
				"http://%u.%u.%u.%u:%u/",
				localIp.host & 0xFF, (localIp.host >> 8) & 0xFF, (localIp.host >> 16) & 0xFF, (localIp.host >> 24) & 0xFF,
				SERVER_PORT
			);
		}
	}

	_serverSocket = SDLNet_TCP_Open(&ip);
	if (!_serverSocket) {
		error("SDLNet_TCP_Open: %s\n", SDLNet_GetError());		
	}

	// Create a socket set
	_set = SDLNet_AllocSocketSet(MAX_CONNECTIONS + 1); //one more for our server socket
	if (!_set) {
		error("SDLNet_AllocSocketSet: %s\n", SDLNet_GetError());		
	}
	
	int numused = SDLNet_TCP_AddSocket(_set, _serverSocket);
	if (numused == -1) {
		error("SDLNet_AddSocket: %s\n", SDLNet_GetError());
	}
	_handleMutex.unlock();
}

void LocalWebserver::stop() {
	_handleMutex.lock();
	if (_timerStarted) stopTimer();

	if (_serverSocket) {
		SDLNet_TCP_Close(_serverSocket);
		_serverSocket = nullptr;
	}

	for (uint32 i = 0; i < MAX_CONNECTIONS; ++i)
		_client[i].close();

	_clients = 0;

	if (_set) {
		SDLNet_FreeSocketSet(_set);
		_set = nullptr;
	}
	_handleMutex.unlock();
}

void LocalWebserver::stopOnIdle() { _stopOnIdle = true; }

void LocalWebserver::addPathHandler(Common::String path, ClientHandlerCallback handler) {
	if (_pathHandlers.contains(path)) warning("LocalWebserver::addPathHandler: path already had a handler");
	_pathHandlers[path] = handler;
}

void LocalWebserver::removePathHandler(Common::String path) {
	if (!_pathHandlers.contains(path)) warning("LocalWebserver::removePathHandler: no handler known for this path");
	_pathHandlers.erase(path);
}

Common::String LocalWebserver::getAddress() { return _address;  }

IndexPageHandler &LocalWebserver::indexPageHandler() { return _indexPageHandler; }

bool LocalWebserver::isRunning() {
	bool result = false;
	_handleMutex.lock();
	result = _timerStarted;
	_handleMutex.unlock();
	return result;
}

void LocalWebserver::handle() {
	_handleMutex.lock();
	int numready = SDLNet_CheckSockets(_set, 0);
	if (numready == -1) {
		error("SDLNet_CheckSockets: %s\n", SDLNet_GetError());
	} else if (numready) acceptClient();

	for (uint32 i = 0; i < MAX_CONNECTIONS; ++i)
		handleClient(i);

	_clients = 0;
	for (uint32 i = 0; i < MAX_CONNECTIONS; ++i)
		if (_client[i].state() != INVALID)
			++_clients;

	if (_clients == 0) ++_idlingFrames;
	else _idlingFrames = 0;
	
	if (_idlingFrames > FRAMES_PER_SECOND && _stopOnIdle) {
		_handleMutex.unlock();
		stop();
		return;
	}

	_handleMutex.unlock();
}

void LocalWebserver::handleClient(uint32 i) {	
	switch (_client[i].state()) {
	case INVALID: return;
	case READING_HEADERS: _client[i].readHeaders(); break;
	case READ_HEADERS: //decide what to do next with that client
		//if GET, check whether we know a handler for such URL
		//if PUT, check whether we know a handler for that URL
		if (_pathHandlers.contains(_client[i].path()))
			(*_pathHandlers[_client[i].path()])(_client[i]);
		else if (_defaultHandler)
			(*_defaultHandler)(_client[i]); //try default handler
		
		if (_client[i].state() == BEING_HANDLED || _client[i].state() == INVALID) break;

		//if no handler, answer with default BAD REQUEST
		//fallthrough
	case BAD_REQUEST:
		setClientGetHandler(_client[i], "<html><head><title>ScummVM - Bad Request</title></head><body>BAD REQUEST</body></html>", 400);
		break;
	case BEING_HANDLED:
		_client[i].handle();
		break;
	}
}

void LocalWebserver::acceptClient() {
	if (!SDLNet_SocketReady(_serverSocket)) return;

	TCPsocket client = SDLNet_TCP_Accept(_serverSocket);
	if (!client) return;

	if (_clients == MAX_CONNECTIONS) { //drop the connection
		SDLNet_TCP_Close(client);
		return;
	}

	++_clients;
	for (uint32 i = 0; i < MAX_CONNECTIONS; ++i)
		if (_client[i].state() == INVALID) {
			_client[i].open(_set, client);
			break;
		}
}

void LocalWebserver::setClientGetHandler(Client &client, Common::String response, long code, const char *mimeType) {
	byte *data = new byte[response.size()];
	memcpy(data, response.c_str(), response.size());
	Common::MemoryReadStream *stream = new Common::MemoryReadStream(data, response.size(), DisposeAfterUse::YES);
	setClientGetHandler(client, stream, code, mimeType);
}

void LocalWebserver::setClientGetHandler(Client &client, Common::SeekableReadStream *responseStream, long code, const char *mimeType) {
	GetClientHandler *handler = new GetClientHandler(responseStream);
	handler->setResponseCode(code);
	if (mimeType) handler->setHeader("Content-Type", mimeType);
	client.setHandler(handler);
}

const char *LocalWebserver::determineMimeType(Common::String &filename) {
	// text
	if (filename.hasSuffix(".html")) return "text/html";
	if (filename.hasSuffix(".css")) return "text/css";
	if (filename.hasSuffix(".txt")) return "text/plain";
	if (filename.hasSuffix(".js")) return "application/javascript";

	// images
	if (filename.hasSuffix(".jpeg") || filename.hasSuffix(".jpg") || filename.hasSuffix(".jpe")) return "image/jpeg";
	if (filename.hasSuffix(".gif")) return "image/gif";
	if (filename.hasSuffix(".png")) return "image/png";
	if (filename.hasSuffix(".svg")) return "image/svg+xml";
	if (filename.hasSuffix(".tiff")) return "image/tiff";
	if (filename.hasSuffix(".ico")) return "image/vnd.microsoft.icon";
	if (filename.hasSuffix(".wbmp")) return "image/vnd.wap.wbmp";

	if (filename.hasSuffix(".zip")) return "application/zip";
	return "application/octet-stream";
}

} // End of namespace Networking
