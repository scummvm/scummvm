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

#include "backends/cloud/dropbox/dropboxtokenrefresher.h"
#include "backends/cloud/dropbox/dropboxstorage.h"
#include "backends/networking/http/networkreadstream.h"
#include "common/debug.h"
#include "common/formats/json.h"

namespace Cloud {
namespace Dropbox {

DropboxTokenRefresher::DropboxTokenRefresher(DropboxStorage *parent, Networking::JsonCallback callback, Networking::ErrorCallback ecb, const char *url):
	HttpJsonRequest(callback, ecb, url), _parentStorage(parent) {}

DropboxTokenRefresher::~DropboxTokenRefresher() {}

void DropboxTokenRefresher::tokenRefreshed(const Storage::BoolResponse &response) {
	if (!response.value) {
		//failed to refresh token, notify user with NULL in original callback
		warning("DropboxTokenRefresher: failed to refresh token");
		finishError(Networking::ErrorResponse(this, false, true, "DropboxTokenRefresher::tokenRefreshed: failed to refresh token", -1));
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

void DropboxTokenRefresher::finishJson(const Common::JSONValue *json) {
	if (!json) {
		//that's probably not an error (200 OK)
		HttpJsonRequest::finishJson(nullptr);
		return;
	}

	if (jsonIsObject(json, "DropboxTokenRefresher")) {
		Common::JSONObject result = json->asObject();

		if (result.contains("error") || result.contains("error_summary")) {
			long httpCode = -1;
			if (_stream) {
				httpCode = _stream->httpResponseCode();
				debug(9, "DropboxTokenRefresher: code %ld", httpCode);
			}

			bool irrecoverable = true;
			if (jsonContainsString(result, "error_summary", "DropboxTokenRefresher")) {
				if (result.getVal("error_summary")->asString().contains("expired_access_token")) {
					irrecoverable = false;
				}
			}

			if (irrecoverable) {
				finishError(Networking::ErrorResponse(this, false, true, json->stringify(true), httpCode));
				delete json;
				return;
			}

			pause();
			delete json;
			_parentStorage->refreshAccessToken(new Common::Callback<DropboxTokenRefresher, const Storage::BoolResponse &>(this, &DropboxTokenRefresher::tokenRefreshed));
			return;
		}
	}

	//notify user of success
	HttpJsonRequest::finishJson(json);
}

void DropboxTokenRefresher::finishError(const Networking::ErrorResponse &error, Networking::RequestState state) {
	if (error.httpResponseCode == 401) {
		pause();
		_parentStorage->refreshAccessToken(new Common::Callback<DropboxTokenRefresher, const Storage::BoolResponse &>(this, &DropboxTokenRefresher::tokenRefreshed));
		return;
	}

	Request::finishError(error);
}

} // End of namespace Dropbox
} // End of namespace Cloud
