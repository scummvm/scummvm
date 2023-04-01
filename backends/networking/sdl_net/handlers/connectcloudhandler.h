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

#ifndef BACKENDS_NETWORKING_SDL_NET_CONNECTCLOUDHANDLER_H
#define BACKENDS_NETWORKING_SDL_NET_CONNECTCLOUDHANDLER_H

#include "backends/networking/sdl_net/handlers/basehandler.h"
#include "backends/networking/sdl_net/client.h"
#include "backends/networking/curl/request.h"
#include "common/memstream.h"

namespace Networking {

class ConnectCloudHandler: public BaseHandler {
	void handleError(Client &client, Common::String message) const;
	void setJsonResponseHandler(Client &client, Common::String type, Common::String message) const;

	Networking::ErrorCallback _storageConnectionCallback;

public:
	ConnectCloudHandler();
	virtual ~ConnectCloudHandler();

	virtual void handle(Client &client);
	virtual bool minimalModeSupported() { return true; }

	void setStorageConnectionCallback(Networking::ErrorCallback cb) { _storageConnectionCallback = cb; }
	void storageConnected(const Networking::ErrorResponse& response) const;
};

class ConnectCloudClientHandler : public ClientHandler {
	const ConnectCloudHandler *_cloudHandler;
	Common::MemoryWriteStreamDynamic _clientContent;
	Client *_client;
	static const int32 SUSPICIOUS_CONTENT_SIZE = 640 * 1024; // 640K ought to be enough for anybody

	void respond(Client &client, Common::String response, long responseCode = 200) const;
	void respondWithJson(Client &client, bool error, Common::String message, long responseCode = 200) const;
	void handleError(Client &client, Common::String message, long responseCode) const;
	void handleSuccess(Client &client, Common::String message) const;
	void storageConnectionCallback(Networking::ErrorResponse response);

public:
	ConnectCloudClientHandler(const ConnectCloudHandler* cloudHandler);
	virtual ~ConnectCloudClientHandler();

	virtual void handle(Client *client);
};

} // End of namespace Networking

#endif
