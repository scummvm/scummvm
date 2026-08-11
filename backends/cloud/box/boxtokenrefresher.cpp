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

#include "backends/cloud/box/boxtokenrefresher.h"
#include "backends/cloud/box/boxstorage.h"
#include "backends/networking/http/networkreadstream.h"
#include "common/debug.h"
#include "common/formats/json.h"

namespace Cloud {
namespace Box {

BoxTokenRefresher::BoxTokenRefresher(BoxStorage *parent, Networking::JsonCallback callback, Networking::ErrorCallback ecb, const char *url):
	HttpJsonRequest(callback, ecb, url), _parentStorage(parent) {}

BoxTokenRefresher::~BoxTokenRefresher() {}

void BoxTokenRefresher::tokenRefreshed(const Storage::BoolResponse &response) {
	if (!response.value) {
		//failed to refresh token, notify user with NULL in original callback
		warning("BoxTokenRefresher: failed to refresh token");
		finishError(Networking::ErrorResponse(this, false, true, "BoxTokenRefresher::tokenRefreshed: failed to refresh token", -1));
		return;
	}

	//update headers: first change header with token, then pass those to request
	for (uint32 i = 0; i < _headersList.size(); ++i) {
		if (_headersList[i].contains("Authorization")) {
			_headersList[i] = "Authorization: Bearer " + _parentStorage->accessToken();
		}
	}

	//successfully received refreshed token, can restart the original request now
	retry(0);
}

void BoxTokenRefresher::finishJson(const Common::JSONValue *json) {
	if (!json) {
		//that's probably not an error (200 OK)
		HttpJsonRequest::finishJson(nullptr);
		return;
	}

	if (jsonIsObject(json, "BoxTokenRefresher")) {
		Common::JSONObject result = json->asObject();
		if (result.contains("type") && result.getVal("type")->isString() && result.getVal("type")->asString() == "error") {
			//new token needed => request token & then retry original request
			long httpCode = -1;
			if (_stream) {
				httpCode = _stream->httpResponseCode();
				debug(9, "BoxTokenRefresher: code %ld", httpCode);
			}

			bool irrecoverable = true;

			Common::String code, message;
			if (jsonContainsString(result, "code", "BoxTokenRefresher")) {
				code = result.getVal("code")->asString();
				debug(9, "BoxTokenRefresher: code = %s", code.c_str());
			}

			if (jsonContainsString(result, "message", "BoxTokenRefresher")) {
				message = result.getVal("message")->asString();
				debug(9, "BoxTokenRefresher: message = %s", message.c_str());
			}

			//TODO: decide when token refreshment will help
			//for now refreshment is used only when HTTP 401 is passed in finishError()
			//if (code == "unauthenticated") irrecoverable = false;

			if (irrecoverable) {
				finishError(Networking::ErrorResponse(this, false, true, json->stringify(true), httpCode));
				delete json;
				return;
			}

			pause();
			delete json;
			_parentStorage->refreshAccessToken(new Common::Callback<BoxTokenRefresher, const Storage::BoolResponse &>(this, &BoxTokenRefresher::tokenRefreshed));
			return;
		}
	}

	//notify user of success
	HttpJsonRequest::finishJson(json);
}

void BoxTokenRefresher::finishError(const Networking::ErrorResponse &error, Networking::RequestState state) {
	if (error.httpResponseCode == 401) { // invalid_token
		pause();
		_parentStorage->refreshAccessToken(new Common::Callback<BoxTokenRefresher, const Storage::BoolResponse &>(this, &BoxTokenRefresher::tokenRefreshed));
		return;
	}

	// there are also 400 == invalid_request and 403 == insufficient_scope
	// but TokenRefresher is there to refresh token when it's invalid only

	Request::finishError(error);
}

} // End of namespace Box
} // End of namespace Cloud
