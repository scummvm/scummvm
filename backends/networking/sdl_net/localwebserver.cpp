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
#include "common/translation.h"
#include <SDL_net.h>
#include <common/config-manager.h>

#ifdef POSIX
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>

#ifndef SIOCGIFCONF
#include <sys/sockio.h>
#endif

#ifndef _SIZEOF_ADDR_IFREQ
#define _SIZEOF_ADDR_IFREQ sizeof
#endif

#define LSSDP_BUFFER_LEN 2048
#endif

namespace Common {
class MemoryReadWriteStream;

DECLARE_SINGLETON(Networking::LocalWebserver);

}

namespace Networking {

LocalWebserver::LocalWebserver(): _set(nullptr), _serverSocket(nullptr), _timerStarted(false),
	_stopOnIdle(false), _minimalMode(false), _clients(0), _idlingFrames(0), _serverPort(DEFAULT_SERVER_PORT) {
	addPathHandler("/", &_indexPageHandler);
	addPathHandler("/files", &_filesPageHandler);
	addPathHandler("/create", &_createDirectoryHandler);
	addPathHandler("/download", &_downloadFileHandler);
	addPathHandler("/upload", &_uploadFileHandler);
	addPathHandler("/list", &_listAjaxHandler);
	addPathHandler("/filesAJAX", &_filesAjaxPageHandler);
	_defaultHandler = &_resourceHandler;
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

void LocalWebserver::start(bool useMinimalMode) {
	_handleMutex.lock();
	_serverPort = getPort();
	_stopOnIdle = false;
	if (_timerStarted) {
		_handleMutex.unlock();
		return;
	}
	_minimalMode = useMinimalMode;
	startTimer();

	// Create a listening TCP socket
	IPaddress ip;
	if (SDLNet_ResolveHost(&ip, NULL, _serverPort) == -1) {
		error("LocalWebserver: SDLNet_ResolveHost: %s\n", SDLNet_GetError());
	}

	resolveAddress(&ip);

	_serverSocket = SDLNet_TCP_Open(&ip);
	if (!_serverSocket) {
		warning("LocalWebserver: SDLNet_TCP_Open: %s", SDLNet_GetError());
		stopTimer();
		g_system->displayMessageOnOSD(_("Failed to start local webserver.\nCheck whether selected port is not used by another application and try again."));
		_handleMutex.unlock();
		return;
	}

	// Create a socket set
	_set = SDLNet_AllocSocketSet(MAX_CONNECTIONS + 1); //one more for our server socket
	if (!_set) {
		error("LocalWebserver: SDLNet_AllocSocketSet: %s\n", SDLNet_GetError());
	}

	int numused = SDLNet_TCP_AddSocket(_set, _serverSocket);
	if (numused == -1) {
		error("LocalWebserver: SDLNet_AddSocket: %s\n", SDLNet_GetError());
	}
	_handleMutex.unlock();
}

void LocalWebserver::stop() {
	_handleMutex.lock();
	if (_timerStarted)
		stopTimer();

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

void LocalWebserver::addPathHandler(Common::String path, BaseHandler *handler) {
	if (_pathHandlers.contains(path))
		warning("LocalWebserver::addPathHandler: path already had a handler");
	_pathHandlers[path] = handler;
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

uint32 LocalWebserver::getPort() {
#ifdef NETWORKING_LOCALWEBSERVER_ENABLE_PORT_OVERRIDE
	if (ConfMan.hasKey("local_server_port"))
		return ConfMan.getInt("local_server_port");
#endif
	return DEFAULT_SERVER_PORT;
}

void LocalWebserver::handle() {
	_handleMutex.lock();
	int numready = SDLNet_CheckSockets(_set, 0);
	if (numready == -1) {
		error("LocalWebserver: SDLNet_CheckSockets: %s\n", SDLNet_GetError());
	} else if (numready) {
		acceptClient();
	}

	for (uint32 i = 0; i < MAX_CONNECTIONS; ++i)
		handleClient(i);

	_clients = 0;
	for (uint32 i = 0; i < MAX_CONNECTIONS; ++i)
		if (_client[i].state() != INVALID)
			++_clients;

	if (_clients == 0)
		++_idlingFrames;
	else
		_idlingFrames = 0;

	if (_idlingFrames > FRAMES_PER_SECOND && _stopOnIdle) {
		_handleMutex.unlock();
		stop();
		return;
	}

	_handleMutex.unlock();
}

void LocalWebserver::handleClient(uint32 i) {
	switch (_client[i].state()) {
	case INVALID:
		return;
	case READING_HEADERS:
		_client[i].readHeaders();
		break;
	case READ_HEADERS: {
		// decide what to do next with that client
		// check whether we know a handler for such URL
		BaseHandler *handler = nullptr;
		if (_pathHandlers.contains(_client[i].path())) {
			handler = _pathHandlers[_client[i].path()];
		} else {
			// try default handler
			handler = _defaultHandler;
		}

		// if server's in "minimal mode", only handlers which support it are used
		if (handler && (!_minimalMode || handler->minimalModeSupported()))
			handler->handle(_client[i]);

		if (_client[i].state() == BEING_HANDLED || _client[i].state() == INVALID)
			break;

		// if no handler, answer with default BAD REQUEST
	}
	// fall through

	case BAD_REQUEST:
		setClientGetHandler(_client[i], "<html><head><title>ScummVM - Bad Request</title></head><body>BAD REQUEST</body></html>", 400);
		break;
	case BEING_HANDLED:
		_client[i].handle();
		break;
	}
}

void LocalWebserver::acceptClient() {
	if (!SDLNet_SocketReady(_serverSocket))
		return;

	TCPsocket client = SDLNet_TCP_Accept(_serverSocket);
	if (!client)
		return;

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

void LocalWebserver::resolveAddress(void *ipAddress) {
	IPaddress *ip = (IPaddress *)ipAddress;

	// not resolved
	_address = Common::String::format("http://127.0.0.1:%u/ (unresolved)", _serverPort);

	// default way (might work everywhere, surely works on Windows)
	const char *name = SDLNet_ResolveIP(ip);
	if (name == NULL) {
		warning("LocalWebserver: SDLNet_ResolveIP: %s", SDLNet_GetError());
	} else {
		IPaddress localIp;
		if (SDLNet_ResolveHost(&localIp, name, _serverPort) == -1) {
			warning("LocalWebserver: SDLNet_ResolveHost: %s", SDLNet_GetError());
		} else {
			_address = Common::String::format(
				"http://%u.%u.%u.%u:%u/",
				localIp.host & 0xFF, (localIp.host >> 8) & 0xFF, (localIp.host >> 16) & 0xFF, (localIp.host >> 24) & 0xFF,
				_serverPort
			);
		}
	}

	// check that our trick worked
	if (_address.contains("/127.0.0.1:") || _address.contains("localhost") || _address.contains("/0.0.0.0:"))
		warning("LocalWebserver: Failed to resolve IP with the default way");
	else
		return;

	// if not - try platform-specific
#ifdef POSIX
	void *tmpAddrPtr = NULL;

	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		warning("LocalWebserver: failed to create socket: %s (%d)", strerror(errno), errno);
	} else {
		// get ifconfig
		char buffer[LSSDP_BUFFER_LEN] = {};
		struct ifconf ifc;
		ifc.ifc_len = sizeof(buffer);
		ifc.ifc_buf = (caddr_t) buffer;

		if (ioctl(fd, SIOCGIFCONF, &ifc) < 0) {
		    warning("LocalWebserver: ioctl SIOCGIFCONF failed: %s (%d)", strerror(errno), errno);
		} else {
			struct ifreq *i;
			for (size_t index = 0; index < (size_t)ifc.ifc_len; index += _SIZEOF_ADDR_IFREQ(*i)) {
				i = (struct ifreq *)(buffer + index);

				Common::String addr;

				// IPv4
				if (i->ifr_addr.sa_family == AF_INET) {
					tmpAddrPtr = &((struct sockaddr_in *)&i->ifr_addr)->sin_addr;
					char addressBuffer[INET_ADDRSTRLEN];
					inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
					debug(9, "%s IP Address %s", i->ifr_name, addressBuffer);
					addr = addressBuffer;
				}

				// IPv6
				/*
				if (i->ifr_addr.sa_family == AF_INET6) {
					tmpAddrPtr = &((struct sockaddr_in6 *)&i->ifr_addr)->sin6_addr;
					char addressBuffer[INET6_ADDRSTRLEN];
					inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
					debug(9, "%s IP Address %s", i->ifr_name, addressBuffer);
					addr = addressBuffer;
				}
				*/

				if (addr.empty())
					continue;

				// ignored IPv4 addresses
				if (addr.equals("127.0.0.1") || addr.equals("0.0.0.0") || addr.equals("localhost"))
					continue;

				// ignored IPv6 addresses
				/*
				if (addr.equals("::1"))
					continue;
				*/

				// use the address found
				_address = "http://" + addr + Common::String::format(":%u/", _serverPort);
			}
		}

		// close socket
		if (close(fd) != 0) {
			warning("LocalWebserver: failed to close socket [fd %d]: %s (%d)", fd, strerror(errno), errno);
		}
	}
#endif
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
	if (mimeType)
		handler->setHeader("Content-Type", mimeType);
	client.setHandler(handler);
}

void LocalWebserver::setClientRedirectHandler(Client &client, Common::String response, Common::String location, const char *mimeType) {
	byte *data = new byte[response.size()];
	memcpy(data, response.c_str(), response.size());
	Common::MemoryReadStream *stream = new Common::MemoryReadStream(data, response.size(), DisposeAfterUse::YES);
	setClientRedirectHandler(client, stream, location, mimeType);
}

void LocalWebserver::setClientRedirectHandler(Client &client, Common::SeekableReadStream *responseStream, Common::String location, const char *mimeType) {
	GetClientHandler *handler = new GetClientHandler(responseStream);
	handler->setResponseCode(302); //redirect
	handler->setHeader("Location", location);
	if (mimeType)
		handler->setHeader("Content-Type", mimeType);
	client.setHandler(handler);
}

namespace {
int hexDigit(char c) {
	if ('0' <= c && c <= '9') return c - '0';
	if ('A' <= c && c <= 'F') return c - 'A' + 10;
	if ('a' <= c && c <= 'f') return c - 'a' + 10;
	return -1;
}
}

Common::String LocalWebserver::urlDecode(Common::String value) {
	Common::String result = "";
	uint32 size = value.size();
	for (uint32 i = 0; i < size; ++i) {
		if (value[i] == '+') {
			result += ' ';
			continue;
		}

		if (value[i] == '%' && i + 2 < size) {
			int d1 = hexDigit(value[i + 1]);
			int d2 = hexDigit(value[i + 2]);
			if (0 <= d1 && d1 < 16 && 0 <= d2 && d2 < 16) {
				result += (char)(d1 * 16 + d2);
				i = i + 2;
				continue;
			}
		}

		result += value[i];
	}
	return result;
}

namespace {
bool isQueryUnreserved(char c) {
	return (
		('0' <= c && c <= '9') ||
		('A' <= c && c <= 'Z') ||
		('a' <= c && c <= 'z') ||
		c == '-' || c == '_' || c == '.' || c == '!' ||
		c == '~' || c == '*' || c == '\'' || c == '(' || c == ')'
	);
}
}

Common::String LocalWebserver::urlEncodeQueryParameterValue(Common::String value) {
	//OK chars = alphanum | "-" | "_" | "." | "!" | "~" | "*" | "'" | "(" | ")"
	//reserved for query are ";", "/", "?", ":", "@", "&", "=", "+", ","
	//that means these must be encoded too or otherwise they could malform the query
	Common::String result = "";
	char hexChar[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	for (uint32 i = 0; i < value.size(); ++i) {
		char c = value[i];
		if (isQueryUnreserved(c))
			result += c;
		else {
			result += '%';
			result += hexChar[(c >> 4) & 0xF];
			result += hexChar[c & 0xF];
		}
	}
	return result;
}

} // End of namespace Networking
