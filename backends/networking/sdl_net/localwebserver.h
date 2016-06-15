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

#ifndef BACKENDS_NETWORKING_SDL_NET_LOCALWEBSERVER_H
#define BACKENDS_NETWORKING_SDL_NET_LOCALWEBSERVER_H

#include "common/singleton.h"
#include "common/scummsys.h"

typedef struct _SDLNet_SocketSet *SDLNet_SocketSet;
typedef struct _TCPsocket *TCPsocket;

namespace Networking {

class LocalWebserver : public Common::Singleton<LocalWebserver> {
	static const uint32 FRAMES_PER_SECOND = 20;
	static const uint32 TIMER_INTERVAL = 1000000 / FRAMES_PER_SECOND;
	static const uint32 SERVER_PORT = 12345;
	static const uint32 MAX_CONNECTIONS = 10;

	friend void localWebserverTimer(void *); //calls handle()

	SDLNet_SocketSet _set;
	TCPsocket _serverSocket;
	TCPsocket _clientSocket[MAX_CONNECTIONS];
	int _clients;
	bool _timerStarted;

	void startTimer(int interval = TIMER_INTERVAL);
	void stopTimer();
	void handle();

public:
	LocalWebserver();
	virtual ~LocalWebserver();

	void start();
	void stop();
};

/** Shortcut for accessing the local webserver. */
#define LocalServer		Networking::LocalWebserver::instance()

} // End of namespace Networking

#endif
