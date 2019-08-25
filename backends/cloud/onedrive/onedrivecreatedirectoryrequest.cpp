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

#include "backends/cloud/onedrive/onedrivecreatedirectoryrequest.h"
#include "backends/cloud/onedrive/onedrivestorage.h"
#include "backends/cloud/onedrive/onedrivetokenrefresher.h"
#include "backends/networking/curl/connectionmanager.h"
#include "backends/networking/curl/curljsonrequest.h"
#include "backends/networking/curl/networkreadstream.h"
#include "common/json.h"

namespace Cloud {
namespace OneDrive {

#define ONEDRIVE_API_SPECIAL_APPROOT "https://graph.microsoft.com/v1.0/drive/special/approot"

OneDriveCreateDirectoryRequest::OneDriveCreateDirectoryRequest(OneDriveStorage *storage, Common::String path, Storage::BoolCallback cb, Networking::ErrorCallback ecb):
	Networking::Request(nullptr, ecb), _storage(storage), _path(path), _boolCallback(cb),
	_workingRequest(nullptr), _ignoreCallback(false) {
	start();
}

OneDriveCreateDirectoryRequest::~OneDriveCreateDirectoryRequest() {
	_ignoreCallback = true;
	if (_workingRequest)
		_workingRequest->finish();
	delete _boolCallback;
}

void OneDriveCreateDirectoryRequest::start() {
	_ignoreCallback = true;
	if (_workingRequest)
		_workingRequest->finish();
	_ignoreCallback = false;

	Common::String name = _path, parent = _path;
	if (name.size() != 0) {
		uint32 i = name.size() - 1;
		while (true) {
			parent.deleteLastChar();
			if (name[i] == '/' || name[i] == '\\') {
				name.erase(0, i + 1);
				break;
			}
			if (i == 0)
				break;
			--i;
		}
	}

	Common::String url = ONEDRIVE_API_SPECIAL_APPROOT;
	if (parent != "")
		url += ":/" + ConnMan.urlEncode(parent) + ":";
	url += "/children";
	Networking::JsonCallback innerCallback = new Common::Callback<OneDriveCreateDirectoryRequest, Networking::JsonResponse>(this, &OneDriveCreateDirectoryRequest::responseCallback);
	Networking::ErrorCallback errorResponseCallback = new Common::Callback<OneDriveCreateDirectoryRequest, Networking::ErrorResponse>(this, &OneDriveCreateDirectoryRequest::errorCallback);
	Networking::CurlJsonRequest *request = new OneDriveTokenRefresher(_storage, innerCallback, errorResponseCallback, url.c_str());
	request->addHeader("Authorization: Bearer " + _storage->accessToken());
	request->addHeader("Content-Type: application/json");

	Common::JSONObject jsonRequestParameters;
	jsonRequestParameters.setVal("name", new Common::JSONValue(name));
	jsonRequestParameters.setVal("folder", new Common::JSONValue(Common::JSONObject()));
	Common::JSONValue value(jsonRequestParameters);
	request->addPostField(Common::JSON::stringify(&value));

	_workingRequest = ConnMan.addRequest(request);
}

void OneDriveCreateDirectoryRequest::responseCallback(Networking::JsonResponse response) {
	Common::JSONValue *json = response.value;
	_workingRequest = nullptr;
	if (_ignoreCallback) {
		delete json;
		return;
	}
	if (response.request)
		_date = response.request->date();

	Networking::ErrorResponse error(this, "OneDriveCreateDirectoryRequest::responseCallback: unknown error");
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

	Common::JSONObject info = json->asObject();
	if (info.contains("id")) {
		finishCreation(true);
	} else {
		error.response = json->stringify(true);
		finishError(error);
	}

	delete json;
}

void OneDriveCreateDirectoryRequest::errorCallback(Networking::ErrorResponse error) {
	_workingRequest = nullptr;
	if (_ignoreCallback)
		return;
	if (error.request)
		_date = error.request->date();
	finishError(error);
}

void OneDriveCreateDirectoryRequest::handle() {}

void OneDriveCreateDirectoryRequest::restart() { start(); }

Common::String OneDriveCreateDirectoryRequest::date() const { return _date; }

void OneDriveCreateDirectoryRequest::finishCreation(bool success) {
	Request::finishSuccess();
	if (_boolCallback)
		(*_boolCallback)(Storage::BoolResponse(this, success));
}

} // End of namespace OneDrive
} // End of namespace Cloud
