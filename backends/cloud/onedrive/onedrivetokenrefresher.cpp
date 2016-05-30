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

#include "backends/cloud/onedrive/onedrivetokenrefresher.h"
#include "backends/cloud/onedrive/onedrivestorage.h"
#include "backends/networking/curl/networkreadstream.h"
#include "common/debug.h"
#include "common/json.h"
#include <curl/curl.h>

namespace Cloud {
namespace OneDrive {

OneDriveTokenRefresher::OneDriveTokenRefresher(OneDriveStorage *parent, Networking::JsonCallback callback, const char *url):
	CurlJsonRequest(callback, url), _parentStorage(parent) {}

OneDriveTokenRefresher::~OneDriveTokenRefresher() {}

void OneDriveTokenRefresher::tokenRefreshed(Storage::BoolResponse pair) {
	if (!pair.value) {
		//failed to refresh token, notify user with NULL in original callback
		warning("OneDriveTokenRefresher: failed to refresh token");
		finish();
		return;
	}

	//update headers: first change header with token, then pass those to request
	for (uint32 i = 0; i < _headers.size(); ++i) {
		if (_headers[i].contains("Authorization")) {
			_headers[i] = "Authorization: bearer " + _parentStorage->accessToken();
		}
	}
	setHeaders(_headers);

	//successfully received refreshed token, can restart the original request now	
	retry(0);
}

void OneDriveTokenRefresher::finishJson(Common::JSONValue *json) {	
	if (!json) {
		//notify user of failure
		warning("OneDriveTokenRefresher: got NULL instead of JSON");
		CurlJsonRequest::finishJson(nullptr);
		return;
	}

	Common::JSONObject result = json->asObject();
	if (result.contains("error")) {
		//new token needed => request token & then retry original request		
		if (_stream) {
			debug("code %ld", _stream->httpResponseCode());
		}

		Common::JSONObject error = result.getVal("error")->asObject();
		bool irrecoverable = true;

		if (error.contains("code")) {
			Common::String code = error.getVal("code")->asString();
			debug("code = %s", code.c_str());
			//if (code == "itemNotFound") irrecoverable = true;
		}

		if (error.contains("message")) {
			Common::String message = error.getVal("message")->asString();
			debug("message = %s", message.c_str());
		}

		if (irrecoverable) {
			CurlJsonRequest::finishJson(nullptr);
			return;
		}

		pause();		
		delete json;
		_parentStorage->getAccessToken(new Common::Callback<OneDriveTokenRefresher, Storage::BoolResponse>(this, &OneDriveTokenRefresher::tokenRefreshed));
		return;
	}

	//notify user of success
	CurlJsonRequest::finishJson(json);
}

void OneDriveTokenRefresher::setHeaders(Common::Array<Common::String> &headers) {	
	_headers = headers;
	curl_slist_free_all(_headersList);
	_headersList = 0;
	for (uint32 i = 0; i < headers.size(); ++i)
		CurlJsonRequest::addHeader(headers[i]);
}


} // End of namespace OneDrive
} // End of namespace Cloud
