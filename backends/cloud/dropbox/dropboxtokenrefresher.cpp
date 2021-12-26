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

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include <curl/curl.h>
#include "backends/cloud/dropbox/dropboxtokenrefresher.h"
#include "backends/cloud/dropbox/dropboxstorage.h"
#include "backends/networking/curl/networkreadstream.h"
#include "common/debug.h"
#include "common/json.h"

namespace Cloud {
namespace Dropbox {

DropboxTokenRefresher::DropboxTokenRefresher(DropboxStorage *parent, Networking::JsonCallback callback, Networking::ErrorCallback ecb, const char *url):
	CurlJsonRequest(callback, ecb, url), _parentStorage(parent) {}

DropboxTokenRefresher::~DropboxTokenRefresher() {}

void DropboxTokenRefresher::tokenRefreshed(Storage::BoolResponse response) {
	if (!response.value) {
		//failed to refresh token, notify user with NULL in original callback
		warning("DropboxTokenRefresher: failed to refresh token");
		finishError(Networking::ErrorResponse(this, false, true, "DropboxTokenRefresher::tokenRefreshed: failed to refresh token", -1));
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

void DropboxTokenRefresher::finishJson(Common::JSONValue *json) {
	if (!json) {
		//that's probably not an error (200 OK)
		CurlJsonRequest::finishJson(nullptr);
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
			_parentStorage->refreshAccessToken(new Common::Callback<DropboxTokenRefresher, Storage::BoolResponse>(this, &DropboxTokenRefresher::tokenRefreshed));
			return;
		}
	}

	//notify user of success
	CurlJsonRequest::finishJson(json);
}

void DropboxTokenRefresher::finishError(Networking::ErrorResponse error, Networking::RequestState state) {
	if (error.httpResponseCode == 401) {
		pause();
		_parentStorage->refreshAccessToken(new Common::Callback<DropboxTokenRefresher, Storage::BoolResponse>(this, &DropboxTokenRefresher::tokenRefreshed));
		return;
	}

	Request::finishError(error);
}

void DropboxTokenRefresher::setHeaders(Common::Array<Common::String> &headers) {
	_headers = headers;
	curl_slist_free_all(_headersList);
	_headersList = nullptr;
	for (uint32 i = 0; i < headers.size(); ++i)
		CurlJsonRequest::addHeader(headers[i]);
}

void DropboxTokenRefresher::addHeader(Common::String header) {
	_headers.push_back(header);
	CurlJsonRequest::addHeader(header);
}

} // End of namespace Dropbox
} // End of namespace Cloud
