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

#include "backends/networking/sdl_net/handlers/connectcloudhandler.h"
#include "backends/fs/fs-factory.h"
#include "backends/cloud/cloudmanager.h"
#include "backends/networking/curl/curljsonrequest.h"
#include "backends/networking/sdl_net/getclienthandler.h"
#include "backends/networking/sdl_net/handlerutils.h"
#include "backends/networking/sdl_net/localwebserver.h"
#include "backends/networking/sdl_net/reader.h"
#include "common/formats/json.h"
#include "common/memstream.h"
#include "common/translation.h"
#include "common/callback.h"

namespace Networking {

ConnectCloudHandler::ConnectCloudHandler() : _storageConnectionCallback(nullptr) {}

ConnectCloudHandler::~ConnectCloudHandler() {}

void ConnectCloudHandler::handle(Client &client) {
	client.setHandler(new ConnectCloudClientHandler(this));
}

void ConnectCloudHandler::storageConnected(const Networking::ErrorResponse &response) const {
	if (_storageConnectionCallback)
		(*_storageConnectionCallback)(response);
}

//

ConnectCloudClientHandler::ConnectCloudClientHandler(const ConnectCloudHandler *cloudHandler):
	_cloudHandler(cloudHandler), _clientContent(DisposeAfterUse::YES), _client(nullptr) {}

ConnectCloudClientHandler::~ConnectCloudClientHandler() {}

void ConnectCloudClientHandler::respond(Client &client, Common::String response, long responseCode) const {
	Common::SeekableReadStream *responseStream = HandlerUtils::makeResponseStreamFromString(response);
	GetClientHandler *responseHandler = new GetClientHandler(responseStream);
	responseHandler->setResponseCode(responseCode);	
	responseHandler->setHeader("Access-Control-Allow-Origin", "https://cloud.scummvm.org");
	responseHandler->setHeader("Access-Control-Allow-Methods", "POST");
	responseHandler->setHeader("Access-Control-Allow-Headers", "Content-Type");

	client.setHandler(responseHandler);
}

void ConnectCloudClientHandler::respondWithJson(Client &client, bool error, Common::String message, long responseCode) const {
	Common::JSONObject response;
	response.setVal("error", new Common::JSONValue(error));
	response.setVal("message", new Common::JSONValue(message));

	Common::JSONValue json = response;
	respond(client, json.stringify(true), responseCode);
}

void ConnectCloudClientHandler::handleError(Client &client, Common::String message, long responseCode) const {
	respondWithJson(client, true, message, responseCode);
}

void ConnectCloudClientHandler::handleSuccess(Client &client, Common::String message) const {
	respondWithJson(client, false, message);
}

/// public

void ConnectCloudClientHandler::handle(Client *client) {
	if (client == nullptr) {
		warning("ConnectCloudClientHandler::handle(): empty client pointer");
		return;
	}

	_client = client;

	if (client->method() == "OPTIONS") {
		respond(*client, "", 200);
		return;
	}

	if (client->method() != "POST") {
		handleError(*client, "Method Not Allowed", 405);
		return;
	}

	if (_clientContent.size() > SUSPICIOUS_CONTENT_SIZE) {
		handleError(*client, "Bad Request", 400);
		return;
	}

	if (!client->readContent(&_clientContent))
		return;

	char *contents = Common::JSON::getPreparedContents(_clientContent);
	Common::JSONValue *json = Common::JSON::parse(contents);
	if (json == nullptr) {
		handleError(*client, "Not Acceptable", 406);
		return;
	}

	Networking::ErrorCallback callback = new Common::Callback<ConnectCloudClientHandler, Networking::ErrorResponse>(this, &ConnectCloudClientHandler::storageConnectionCallback);
	Networking::JsonResponse jsonResponse(nullptr, json);
	if (!CloudMan.connectStorage(jsonResponse, callback)) { // JSON doesn't have "storage" in it or it was invalid
		delete callback;
		handleError(*client, "Not Acceptable", 406);
	}
}

void ConnectCloudClientHandler::storageConnectionCallback(Networking::ErrorResponse response) {
	if (response.failed || response.interrupted) {
		Common::String message = "Failed to connect storage.";
		if (response.failed) {
			message += " Context: ";
			message += response.response.c_str();
		}

		handleError(*_client, message, 200);
	} else {
		handleSuccess(*_client, "Storage connected.");
	}

	_cloudHandler->storageConnected(response);
}

} // End of namespace Networking
