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

#include <curl/curl.h>
#include "backends/cloud/box/boxtokenrefresher.h"
#include "backends/cloud/box/boxstorage.h"
#include "backends/networking/curl/networkreadstream.h"
#include "common/debug.h"
#include "common/json.h"

namespace Cloud {
namespace Box {

BoxTokenRefresher::BoxTokenRefresher(BoxStorage *parent, Networking::JsonCallback callback, Networking::ErrorCallback ecb, const char *url):
	CurlJsonRequest(callback, ecb, url), _parentStorage(parent) {}

BoxTokenRefresher::~BoxTokenRefresher() {}

void BoxTokenRefresher::tokenRefreshed(Storage::BoolResponse response) {
	if (!response.value) {
		//failed to refresh token, notify user with NULL in original callback
		warning("BoxTokenRefresher: failed to refresh token");
		finishError(Networking::ErrorResponse(this, false, true, "BoxTokenRefresher::tokenRefreshed: failed to refresh token", -1));
		return;
	}

	//update headers: first change header with token, then pass those to request
	for (uint32 i = 0; i < _headers.size(); ++i) {
		if (_headers[i].contains("Authorization")) {
			_headers[i] = "Authorization: Bearer " + _parentStorage->accessToken();
		}
	}
	setHeaders(_headers);

	//successfully received refreshed token, can restart the original request now
	retry(0);
}

void BoxTokenRefresher::finishJson(Common::JSONValue *json) {
	if (!json) {
		//that's probably not an error (200 OK)
		CurlJsonRequest::finishJson(nullptr);
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
			_parentStorage->refreshAccessToken(new Common::Callback<BoxTokenRefresher, Storage::BoolResponse>(this, &BoxTokenRefresher::tokenRefreshed));
			return;
		}
	}

	//notify user of success
	CurlJsonRequest::finishJson(json);
}

void BoxTokenRefresher::finishError(Networking::ErrorResponse error) {
	if (error.httpResponseCode == 401) { // invalid_token
		pause();
		_parentStorage->refreshAccessToken(new Common::Callback<BoxTokenRefresher, Storage::BoolResponse>(this, &BoxTokenRefresher::tokenRefreshed));
		return;
	}

	// there are also 400 == invalid_request and 403 == insufficient_scope
	// but TokenRefresher is there to refresh token when it's invalid only

	Request::finishError(error);
}

void BoxTokenRefresher::setHeaders(Common::Array<Common::String> &headers) {
	_headers = headers;
	curl_slist_free_all(_headersList);
	_headersList = 0;
	for (uint32 i = 0; i < headers.size(); ++i)
		CurlJsonRequest::addHeader(headers[i]);
}

void BoxTokenRefresher::addHeader(Common::String header) {
	_headers.push_back(header);
	CurlJsonRequest::addHeader(header);
}

} // End of namespace Box
} // End of namespace Cloud
