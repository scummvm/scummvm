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

#include "backends/cloud/dropbox/dropboxcreatedirectoryrequest.h"
#include "backends/cloud/storage.h"
#include "backends/networking/curl/connectionmanager.h"
#include "backends/networking/curl/curljsonrequest.h"
#include "backends/networking/curl/networkreadstream.h"
#include "common/json.h"

namespace Cloud {
namespace Dropbox {

#define DROPBOX_API_CREATE_FOLDER "https://api.dropboxapi.com/2/files/create_folder"

DropboxCreateDirectoryRequest::DropboxCreateDirectoryRequest(Common::String token, Common::String path, Storage::BoolCallback cb, Networking::ErrorCallback ecb):
	Networking::Request(nullptr, ecb), _token(token), _path(path), _boolCallback(cb),
	_workingRequest(nullptr), _ignoreCallback(false) {
	start();
}

DropboxCreateDirectoryRequest::~DropboxCreateDirectoryRequest() {
	_ignoreCallback = true;
	if (_workingRequest)
		_workingRequest->finish();
	delete _boolCallback;
}

void DropboxCreateDirectoryRequest::start() {
	_ignoreCallback = true;
	if (_workingRequest)
		_workingRequest->finish();
	_ignoreCallback = false;

	Networking::JsonCallback innerCallback = new Common::Callback<DropboxCreateDirectoryRequest, Networking::JsonResponse>(this, &DropboxCreateDirectoryRequest::responseCallback);
	Networking::ErrorCallback errorResponseCallback = new Common::Callback<DropboxCreateDirectoryRequest, Networking::ErrorResponse>(this, &DropboxCreateDirectoryRequest::errorCallback);
	Networking::CurlJsonRequest *request = new Networking::CurlJsonRequest(innerCallback, errorResponseCallback, DROPBOX_API_CREATE_FOLDER);
	request->addHeader("Authorization: Bearer " + _token);
	request->addHeader("Content-Type: application/json");

	Common::JSONObject jsonRequestParameters;
	jsonRequestParameters.setVal("path", new Common::JSONValue(_path));
	Common::JSONValue value(jsonRequestParameters);
	request->addPostField(Common::JSON::stringify(&value));

	_workingRequest = ConnMan.addRequest(request);
}

void DropboxCreateDirectoryRequest::responseCallback(Networking::JsonResponse response) {
	Common::JSONValue *json = response.value;
	_workingRequest = nullptr;
	if (_ignoreCallback) {
		delete json;
		return;
	}
	if (response.request) _date = response.request->date();

	Networking::ErrorResponse error(this, "DropboxCreateDirectoryRequest::responseCallback: unknown error");
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
		if (Networking::CurlJsonRequest::jsonContainsString(info, "error_summary", "DropboxCreateDirectoryRequest")) {
			Common::String summary = info.getVal("error_summary")->asString();
			if (summary.contains("path") && summary.contains("conflict") && summary.contains("folder")) {
				// existing directory - not an error for CreateDirectoryRequest
				finishCreation(false);
				delete json;
				return;
			}
		}
		error.response = json->stringify(true);
		finishError(error);
	}

	delete json;
}

void DropboxCreateDirectoryRequest::errorCallback(Networking::ErrorResponse error) {
	_workingRequest = nullptr;
	if (_ignoreCallback)
		return;
	if (error.request)
		_date = error.request->date();
	finishError(error);
}

void DropboxCreateDirectoryRequest::handle() {}

void DropboxCreateDirectoryRequest::restart() { start(); }

Common::String DropboxCreateDirectoryRequest::date() const { return _date; }

void DropboxCreateDirectoryRequest::finishCreation(bool success) {
	Request::finishSuccess();
	if (_boolCallback)
		(*_boolCallback)(Storage::BoolResponse(this, success));
}

} // End of namespace Dropbox
} // End of namespace Cloud
