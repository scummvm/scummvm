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

#include "backends/networking/sdl_net/client.h"
#include "backends/networking/sdl_net/handlers/basehandler.h"
#include "backends/networking/sdl_net/handlers/createdirectoryhandler.h"
#include "backends/networking/sdl_net/handlers/downloadfilehandler.h"
#include "backends/networking/sdl_net/handlers/filesajaxpagehandler.h"
#include "backends/networking/sdl_net/handlers/filespagehandler.h"
#include "backends/networking/sdl_net/handlers/indexpagehandler.h"
#include "backends/networking/sdl_net/handlers/listajaxhandler.h"
#include "backends/networking/sdl_net/handlers/resourcehandler.h"
#include "backends/networking/sdl_net/handlers/uploadfilehandler.h"
#include "common/hash-str.h"
#include "common/mutex.h"
#include "common/singleton.h"
#include "common/scummsys.h"

namespace Common {
class SeekableReadStream;
}

typedef struct _SDLNet_SocketSet *SDLNet_SocketSet;
typedef struct _TCPsocket *TCPsocket;

namespace Networking {

#define NETWORKING_LOCALWEBSERVER_ENABLE_PORT_OVERRIDE

class LocalWebserver : public Common::Singleton<LocalWebserver> {
	static const uint32 FRAMES_PER_SECOND = 20;
	static const uint32 TIMER_INTERVAL = 1000000 / FRAMES_PER_SECOND;
	static const uint32 MAX_CONNECTIONS = 10;

	friend void localWebserverTimer(void *); //calls handle()

	SDLNet_SocketSet _set;
	TCPsocket _serverSocket;
	Client _client[MAX_CONNECTIONS];
	uint32 _clients;
	bool _timerStarted, _stopOnIdle, _minimalMode;
	Common::HashMap<Common::String, BaseHandler*> _pathHandlers;
	BaseHandler *_defaultHandler;
	IndexPageHandler _indexPageHandler;
	FilesPageHandler _filesPageHandler;
	CreateDirectoryHandler _createDirectoryHandler;
	DownloadFileHandler _downloadFileHandler;
	UploadFileHandler _uploadFileHandler;
	ListAjaxHandler _listAjaxHandler;
	FilesAjaxPageHandler _filesAjaxPageHandler;
	ResourceHandler _resourceHandler;
	uint32 _idlingFrames;
	Common::Mutex _handleMutex;
	Common::String _address;
	uint32 _serverPort;

	void startTimer(int interval = TIMER_INTERVAL);
	void stopTimer();
	void handle();
	void handleClient(uint32 i);
	void acceptClient();
	void resolveAddress(void *ipAddress);
	void addPathHandler(Common::String path, BaseHandler *handler);

public:
	static const uint32 DEFAULT_SERVER_PORT = 12345;

	LocalWebserver();
	virtual ~LocalWebserver();

	void start(bool useMinimalMode = false);
	void stop();
	void stopOnIdle();

	Common::String getAddress();
	IndexPageHandler &indexPageHandler();
	bool isRunning();
	static uint32 getPort();

	static void setClientGetHandler(Client &client, Common::String response, long code = 200, const char *mimeType = nullptr);
	static void setClientGetHandler(Client &client, Common::SeekableReadStream *responseStream, long code = 200, const char *mimeType = nullptr);
	static void setClientRedirectHandler(Client &client, Common::String response, Common::String location, const char *mimeType = nullptr);
	static void setClientRedirectHandler(Client &client, Common::SeekableReadStream *responseStream, Common::String location, const char *mimeType = nullptr);
	static Common::String urlDecode(Common::String value);
	static Common::String urlEncodeQueryParameterValue(Common::String value);
};

/** Shortcut for accessing the local webserver. */
#define LocalServer     Networking::LocalWebserver::instance()

} // End of namespace Networking

#endif
