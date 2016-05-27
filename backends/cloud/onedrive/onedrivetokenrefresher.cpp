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
#include "backends/networking/curl/connectionmanager.h"
#include "backends/networking/curl/curljsonrequest.h"
#include "backends/networking/curl/networkreadstream.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/json.h"
#include <curl/curl.h>

namespace Cloud {
namespace OneDrive {

OneDriveTokenRefresher::OneDriveTokenRefresher(OneDriveStorage *parent, Networking::JsonCallback callback, const char *url):
	CurlJsonRequest(0, url),
	_parentStorage(parent),
	_innerRequest(
		new CurlJsonRequest(
			new Common::Callback<OneDriveTokenRefresher, Networking::JsonResponse>(this, &OneDriveTokenRefresher::innerRequestCallback),
			url
		)
	), _jsonCallback(callback), _retryRequest(nullptr), _started(false) {}

OneDriveTokenRefresher::~OneDriveTokenRefresher() {}

void OneDriveTokenRefresher::innerRequestCallback(Networking::JsonResponse pair) {
	if (!pair.value) {
		//notify user of failure
		warning("OneDriveTokenRefresher: got NULL instead of JSON");
		finish();
		return;
	}

	Common::JSONObject result = pair.value->asObject();
	if (result.contains("error")) {
		//new token needed => request token & then retry original request		
		CurlJsonRequest *streamRequest = (CurlJsonRequest *)pair.request;
		if (streamRequest) {
			const Networking::NetworkReadStream *stream = streamRequest->getNetworkReadStream();
			if (stream) {
				debug("code %ld", stream->httpResponseCode());
			}
		}

		Common::JSONObject error = result.getVal("error")->asObject();
		debug("code = %s", error.getVal("code")->asString().c_str());
		debug("message = %s", error.getVal("message")->asString().c_str());
		if (pair.request) pair.request->pause();
		_retryRequest = pair.request;
		delete pair.value;
		_parentStorage->getAccessToken(new Common::Callback<OneDriveTokenRefresher, Storage::BoolResponse>(this, &OneDriveTokenRefresher::tokenRefreshed));
		return;
	}

	//notify user of success	
	finishJson(pair.value);
}

void OneDriveTokenRefresher::tokenRefreshed(Storage::BoolResponse pair) {
	if (!pair.value) {
		//failed to refresh token, notify user with NULL in original callback
		warning("OneDriveTokenRefresher: failed to refresh token");
		finish();
		return;
	}

	//successfully received refreshed token, can restart the original request now	
	if (_retryRequest) _retryRequest->retry(1);

	//update headers: first change header with token, then pass those to request
	for (uint32 i = 0; i < _headers.size(); ++i) {
		if (_headers[i].contains("Authorization: bearer ")) {			
			_headers[i] = "Authorization: bearer " + _parentStorage->accessToken();
		}
	}
	CurlJsonRequest *retryRequest = (CurlJsonRequest *)_retryRequest;
	if (retryRequest) retryRequest->setHeaders(_headers);
}

void OneDriveTokenRefresher::handle() {
	if (!_started) {
		for (uint32 i = 0; i < _headers.size(); ++i)
			_innerRequest->addHeader(_headers[i]);
		_started = true;
		ConnMan.addRequest(_innerRequest);
	}
}

void OneDriveTokenRefresher::restart() {
	//can't restart as all headers were passed to _innerRequest which is probably dead now
	warning("OneDriveTokenRefresher: cannot be restarted");
	finish();
}

void OneDriveTokenRefresher::finish() {
	finishJson(0);
}

void OneDriveTokenRefresher::finishJson(Common::JSONValue *json) {
	Request::finish();
	if (_jsonCallback) (*_jsonCallback)(Networking::JsonResponse(this, json));
}

Networking::NetworkReadStreamResponse OneDriveTokenRefresher::execute() {
	if (!_started) {
		for (uint32 i = 0; i < _headers.size(); ++i)
			_innerRequest->addHeader(_headers[i]);
		_started = true;
	} else {
		warning("OneDriveTokenRefresher: inner Request is already started");
	}
	return _innerRequest->execute();
}

} //end of namespace OneDrive
} //end of namespace Cloud
