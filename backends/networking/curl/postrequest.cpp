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

#include "backends/networking/curl/postrequest.h"
#include "backends/networking/curl/connectionmanager.h"
#include "backends/networking/curl/curljsonrequest.h"
#include "backends/networking/curl/networkreadstream.h"
#include "common/json.h"

namespace Networking {

PostRequest::PostRequest(Common::String url, byte *postData, int postLen, Networking::JSONValueCallback cb, Networking::ErrorCallback ecb):
	Networking::Request(nullptr, ecb), _url(url), _jsonCallback(cb),
	_workingRequest(nullptr), _ignoreCallback(false), _postData(postData), _postLen(postLen) {
	start();
}

PostRequest::~PostRequest() {
	_ignoreCallback = true;
	if (_workingRequest)
		_workingRequest->finish();
	delete _jsonCallback;
}

void PostRequest::start() {
	_ignoreCallback = true;
	if (_workingRequest)
		_workingRequest->finish();
	_ignoreCallback = false;

	Networking::JsonCallback innerCallback = new Common::Callback<PostRequest, Networking::JsonResponse>(this, &PostRequest::responseCallback);
	Networking::ErrorCallback errorResponseCallback = new Common::Callback<PostRequest, Networking::ErrorResponse>(this, &PostRequest::errorCallback);
	Networking::CurlJsonRequest *request = new Networking::CurlJsonRequest(innerCallback, errorResponseCallback, _url);
	request->addHeader("Content-Type: application/json");

	request->setBuffer(_postData, _postLen);

	_workingRequest = ConnMan.addRequest(request);
}

void PostRequest::responseCallback(Networking::JsonResponse response) {
	Common::JSONValue *json = response.value;
	_workingRequest = nullptr;
	if (_ignoreCallback) {
		delete json;
		return;
	}
	if (response.request) _date = response.request->date();

	Networking::ErrorResponse error(this, "PostRequest::responseCallback: unknown error");
	Networking::CurlJsonRequest *rq = (Networking::CurlJsonRequest *)response.request;
	if (rq && rq->getNetworkReadStream())
		error.httpResponseCode = rq->getNetworkReadStream()->httpResponseCode();

	if (json == nullptr) {
		error.response = "Failed to parse JSON, null passed!";
		finishError(error);
		return;
	}

	if (!json->isObject()) {
		error.response = "Passed JSON is not an object!";
		finishError(error);
		delete json;
		return;
	}

	finishSuccess();

	if (_jsonCallback)
		(*_jsonCallback)(json);

	delete json;
}

void PostRequest::errorCallback(Networking::ErrorResponse error) {
	_workingRequest = nullptr;
	if (_ignoreCallback)
		return;
	if (error.request)
		_date = error.request->date();
	finishError(error);
}

void PostRequest::handle() {}

void PostRequest::restart() { start(); }

Common::String PostRequest::date() const { return _date; }

} // End of namespace Networking
