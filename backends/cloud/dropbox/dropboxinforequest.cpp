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

#include "backends/cloud/dropbox/dropboxinforequest.h"
#include "backends/cloud/cloudmanager.h"
#include "backends/cloud/storage.h"
#include "backends/networking/curl/connectionmanager.h"
#include "backends/networking/curl/curljsonrequest.h"
#include "backends/networking/curl/networkreadstream.h"
#include "common/json.h"

namespace Cloud {
namespace Dropbox {

#define DROPBOX_API_GET_CURRENT_ACCOUNT "https://api.dropboxapi.com/2/users/get_current_account"
#define DROPBOX_API_GET_SPACE_USAGE "https://api.dropboxapi.com/2/users/get_space_usage"

DropboxInfoRequest::DropboxInfoRequest(Common::String token, Storage::StorageInfoCallback cb, Networking::ErrorCallback ecb):
	Networking::Request(nullptr, ecb), _token(token), _infoCallback(cb),
	_workingRequest(nullptr), _ignoreCallback(false) {
	start();
}

DropboxInfoRequest::~DropboxInfoRequest() {
	_ignoreCallback = true;
	if (_workingRequest)
		_workingRequest->finish();
	delete _infoCallback;
}

void DropboxInfoRequest::start() {
	_ignoreCallback = true;
	if (_workingRequest)
		_workingRequest->finish();
	_ignoreCallback = false;

	Networking::JsonCallback innerCallback = new Common::Callback<DropboxInfoRequest, Networking::JsonResponse>(this, &DropboxInfoRequest::userResponseCallback);
	Networking::ErrorCallback errorResponseCallback = new Common::Callback<DropboxInfoRequest, Networking::ErrorResponse>(this, &DropboxInfoRequest::errorCallback);
	Networking::CurlJsonRequest *request = new Networking::CurlJsonRequest(innerCallback, errorResponseCallback, DROPBOX_API_GET_CURRENT_ACCOUNT);
	request->addHeader("Authorization: Bearer " + _token);
	request->addHeader("Content-Type: application/json");
	request->addPostField("null"); //use POST

	_workingRequest = ConnMan.addRequest(request);
}

void DropboxInfoRequest::userResponseCallback(Networking::JsonResponse response) {
	Common::JSONValue *json = response.value;
	_workingRequest = nullptr;
	if (_ignoreCallback) {
		delete json;
		return;
	}

	Networking::ErrorResponse error(this, "DropboxInfoRequest::userResponseCallback: unknown error");
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

	//Dropbox documentation states there are no errors for this API method
	Common::JSONObject info = json->asObject();
	if (Networking::CurlJsonRequest::jsonContainsAttribute(info, "name", "DropboxInfoRequest") &&
		Networking::CurlJsonRequest::jsonIsObject(info.getVal("name"), "DropboxInfoRequest")) {
		Common::JSONObject nameInfo = info.getVal("name")->asObject();
		if (Networking::CurlJsonRequest::jsonContainsString(nameInfo, "display_name", "DropboxInfoRequest")) {
			_name = nameInfo.getVal("display_name")->asString();
		}
	}
	if (Networking::CurlJsonRequest::jsonContainsString(info, "account_id", "DropboxInfoRequest")) {
		_uid = info.getVal("account_id")->asString();
	}
	if (Networking::CurlJsonRequest::jsonContainsString(info, "email", "DropboxInfoRequest")) {
		_email = info.getVal("email")->asString();
	}
	CloudMan.setStorageUsername(kStorageDropboxId, _email);
	delete json;

	Networking::JsonCallback innerCallback = new Common::Callback<DropboxInfoRequest, Networking::JsonResponse>(this, &DropboxInfoRequest::quotaResponseCallback);
	Networking::ErrorCallback errorResponseCallback = new Common::Callback<DropboxInfoRequest, Networking::ErrorResponse>(this, &DropboxInfoRequest::errorCallback);
	Networking::CurlJsonRequest *request = new Networking::CurlJsonRequest(innerCallback, errorResponseCallback, DROPBOX_API_GET_SPACE_USAGE);
	request->addHeader("Authorization: Bearer " + _token);
	request->addHeader("Content-Type: application/json");
	request->addPostField("null"); //use POST

	_workingRequest = ConnMan.addRequest(request);
}

void DropboxInfoRequest::quotaResponseCallback(Networking::JsonResponse response) {
	Common::JSONValue *json = response.value;
	_workingRequest = nullptr;
	if (_ignoreCallback) {
		delete json;
		return;
	}

	Networking::ErrorResponse error(this, "DropboxInfoRequest::quotaResponseCallback: unknown error");
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

	//Dropbox documentation states there are no errors for this API method
	Common::JSONObject info = json->asObject();

	if (!Networking::CurlJsonRequest::jsonContainsIntegerNumber(info, "used", "DropboxInfoRequest")) {
		error.response = "Passed JSON misses 'used' attribute!";
		finishError(error);
		delete json;
		return;
	}

	uint64 used = info.getVal("used")->asIntegerNumber(), allocated = 0;

	if (Networking::CurlJsonRequest::jsonContainsAttribute(info, "allocation", "DropboxInfoRequest") &&
		Networking::CurlJsonRequest::jsonIsObject(info.getVal("allocation"), "DropboxInfoRequest")) {
		Common::JSONObject allocation = info.getVal("allocation")->asObject();
		if (!Networking::CurlJsonRequest::jsonContainsIntegerNumber(allocation, "allocated", "DropboxInfoRequest")) {
			error.response = "Passed JSON misses 'allocation/allocated' attribute!";
			finishError(error);
			delete json;
			return;
		}

		allocated = allocation.getVal("allocated")->asIntegerNumber();
	}

	finishInfo(StorageInfo(_uid, _name, _email, used, allocated));
	delete json;
}

void DropboxInfoRequest::errorCallback(Networking::ErrorResponse error) {
	_workingRequest = nullptr;
	if (_ignoreCallback) return;
	finishError(error);
}

void DropboxInfoRequest::handle() {}

void DropboxInfoRequest::restart() { start(); }

void DropboxInfoRequest::finishInfo(StorageInfo info) {
	Request::finishSuccess();
	if (_infoCallback)
		(*_infoCallback)(Storage::StorageInfoResponse(this, info));
}

} // End of namespace Dropbox
} // End of namespace Cloud
