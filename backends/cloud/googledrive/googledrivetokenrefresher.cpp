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
#include "backends/cloud/googledrive/googledrivetokenrefresher.h"
#include "backends/cloud/googledrive/googledrivestorage.h"
#include "backends/networking/curl/networkreadstream.h"
#include "common/debug.h"
#include "common/json.h"

namespace Cloud {
namespace GoogleDrive {

GoogleDriveTokenRefresher::GoogleDriveTokenRefresher(GoogleDriveStorage *parent, Networking::JsonCallback callback, Networking::ErrorCallback ecb, const char *url):
	CurlJsonRequest(callback, ecb, url), _parentStorage(parent) {}

GoogleDriveTokenRefresher::~GoogleDriveTokenRefresher() {}

void GoogleDriveTokenRefresher::tokenRefreshed(Storage::BoolResponse response) {
	if (!response.value) {
		//failed to refresh token, notify user with NULL in original callback
		warning("GoogleDriveTokenRefresher: failed to refresh token");
		finishError(Networking::ErrorResponse(this, false, true, "GoogleDriveTokenRefresher::tokenRefreshed: failed to refresh token", -1));
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

void GoogleDriveTokenRefresher::finishJson(Common::JSONValue *json) {
	if (!json) {
		//that's probably not an error (200 OK)
		CurlJsonRequest::finishJson(nullptr);
		return;
	}

	if (jsonIsObject(json, "GoogleDriveTokenRefresher")) {
		Common::JSONObject result = json->asObject();
		long httpResponseCode = -1;
		if (result.contains("error") && jsonIsObject(result.getVal("error"), "GoogleDriveTokenRefresher")) {
			//new token needed => request token & then retry original request
			if (_stream) {
				httpResponseCode = _stream->httpResponseCode();
				debug(9, "GoogleDriveTokenRefresher: code = %ld", httpResponseCode);
			}

			Common::JSONObject error = result.getVal("error")->asObject();
			bool irrecoverable = true;

			uint32 code = 0xFFFFFFFF; // Invalid
			Common::String message;
			if (jsonContainsIntegerNumber(error, "code", "GoogleDriveTokenRefresher")) {
				code = error.getVal("code")->asIntegerNumber();
				debug(9, "GoogleDriveTokenRefresher: code = %u", code);
			}

			if (jsonContainsString(error, "message", "GoogleDriveTokenRefresher")) {
				message = error.getVal("message")->asString();
				debug(9, "GoogleDriveTokenRefresher: message = %s", message.c_str());
			}

			if (code == 401 || message == "Invalid Credentials")
				irrecoverable = false;

			if (irrecoverable) {
				finishError(Networking::ErrorResponse(this, false, true, json->stringify(true), httpResponseCode));
				delete json;
				return;
			}

			pause();
			delete json;
			_parentStorage->refreshAccessToken(new Common::Callback<GoogleDriveTokenRefresher, Storage::BoolResponse>(this, &GoogleDriveTokenRefresher::tokenRefreshed));
			return;
		}
	}

	//notify user of success
	CurlJsonRequest::finishJson(json);
}

void GoogleDriveTokenRefresher::setHeaders(Common::Array<Common::String> &headers) {
	_headers = headers;
	curl_slist_free_all(_headersList);
	_headersList = 0;
	for (uint32 i = 0; i < headers.size(); ++i)
		CurlJsonRequest::addHeader(headers[i]);
}

void GoogleDriveTokenRefresher::addHeader(Common::String header) {
	_headers.push_back(header);
	CurlJsonRequest::addHeader(header);
}

} // End of namespace GoogleDrive
} // End of namespace Cloud
