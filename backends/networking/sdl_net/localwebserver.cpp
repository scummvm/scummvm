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
#include "common/str.h"
#include "common/system.h"
#include "common/timer.h"
#include "common/textconsole.h"
#include <SDL/SDL_net.h>

namespace Common {

DECLARE_SINGLETON(Networking::LocalWebserver);

}

namespace Networking {

LocalWebserver::LocalWebserver(): _set(nullptr), _serverSocket(nullptr), _timerStarted(false), _clients(0) {}

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
	if (_timerStarted) return;
	startTimer();

	// Create a listening TCP socket
	IPaddress ip;	
	if (SDLNet_ResolveHost(&ip, NULL, SERVER_PORT) == -1) {
		error("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
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
}

void LocalWebserver::stop() {
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
}

void LocalWebserver::handle() {
	int numready = SDLNet_CheckSockets(_set, 0);
	if (numready == -1) {
		error("SDLNet_CheckSockets: %s\n", SDLNet_GetError());
	} else if (numready) acceptClient();

	for (uint32 i = 0; i < MAX_CONNECTIONS; ++i)
		handleClient(i);
}

void LocalWebserver::handleClient(uint32 i) {	
	switch (_client[i].state()) {
	case INVALID: return;
	case READING_HEADERS: _client[i].readHeaders(); break;
	case READ_HEADERS: //decide what to do next with that client
		//if GET, check whether we know a handler for such URL
		//if PUT, check whether we know a handler for that URL
		//if no handler, answer with default BAD REQUEST
		warning("headers %s", _client[i].headers().c_str());
		_client[i].close();
		break;
	case BAD_REQUEST:
		//TODO: answer with BAD REQUEST
		_client[i].close();
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
	
	_client[_clients++].open(_set, client);
}

} // End of namespace Networking
