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

#include "backends/cloud/onedrive/onedrivelistdirectoryrequest.h"
#include "backends/cloud/onedrive/onedrivestorage.h"
#include "backends/cloud/onedrive/onedrivetokenrefresher.h"
#include "backends/cloud/iso8601.h"
#include "backends/networking/curl/connectionmanager.h"
#include "backends/networking/curl/networkreadstream.h"
#include "common/json.h"

namespace Cloud {
namespace OneDrive {

#define ONEDRIVE_API_SPECIAL_APPROOT_CHILDREN "https://graph.microsoft.com/v1.0/drive/special/approot:/%s:/children"
#define ONEDRIVE_API_SPECIAL_APPROOT_CHILDREN_ROOT_ITSELF "https://graph.microsoft.com/v1.0/drive/special/approot/children"

OneDriveListDirectoryRequest::OneDriveListDirectoryRequest(OneDriveStorage *storage, Common::String path, Storage::ListDirectoryCallback cb, Networking::ErrorCallback ecb, bool recursive):
	Networking::Request(nullptr, ecb),
	_requestedPath(path), _requestedRecursive(recursive), _storage(storage), _listDirectoryCallback(cb),
	_workingRequest(nullptr), _ignoreCallback(false) {
	start();
}

OneDriveListDirectoryRequest::~OneDriveListDirectoryRequest() {
	_ignoreCallback = true;
	if (_workingRequest)
		_workingRequest->finish();
	delete _listDirectoryCallback;
}

void OneDriveListDirectoryRequest::start() {
	//cleanup
	_ignoreCallback = true;
	if (_workingRequest)
		_workingRequest->finish();
	_workingRequest = nullptr;
	_files.clear();
	_directoriesQueue.clear();
	_currentDirectory = "";
	_ignoreCallback = false;

	_directoriesQueue.push_back(_requestedPath);
	listNextDirectory();
}

void OneDriveListDirectoryRequest::listNextDirectory() {
	if (_directoriesQueue.empty()) {
		finishListing(_files);
		return;
	}

	_currentDirectory = _directoriesQueue.back();
	_directoriesQueue.pop_back();

	if (_currentDirectory != "" && _currentDirectory.lastChar() != '/' && _currentDirectory.lastChar() != '\\')
		_currentDirectory += '/';

	Common::String dir = _currentDirectory;
	dir.deleteLastChar();
	Common::String url = Common::String::format(ONEDRIVE_API_SPECIAL_APPROOT_CHILDREN, ConnMan.urlEncode(dir).c_str());
	if (dir == "") url = Common::String(ONEDRIVE_API_SPECIAL_APPROOT_CHILDREN_ROOT_ITSELF);
	makeRequest(url);
}

void OneDriveListDirectoryRequest::makeRequest(Common::String url) {
	Networking::JsonCallback callback = new Common::Callback<OneDriveListDirectoryRequest, Networking::JsonResponse>(this, &OneDriveListDirectoryRequest::listedDirectoryCallback);
	Networking::ErrorCallback failureCallback = new Common::Callback<OneDriveListDirectoryRequest, Networking::ErrorResponse>(this, &OneDriveListDirectoryRequest::listedDirectoryErrorCallback);
	Networking::CurlJsonRequest *request = new OneDriveTokenRefresher(_storage, callback, failureCallback, url.c_str());
	request->addHeader("Authorization: bearer " + _storage->accessToken());
	_workingRequest = ConnMan.addRequest(request);
}

void OneDriveListDirectoryRequest::listedDirectoryCallback(Networking::JsonResponse response) {
	_workingRequest = nullptr;
	Common::JSONValue *json = response.value;

	if (_ignoreCallback) {
		delete json;
		return;
	}

	if (response.request)
		_date = response.request->date();

	Networking::ErrorResponse error(this, "OneDriveListDirectoryRequest::listedDirectoryCallback: unknown error");
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

	Common::JSONObject object = json->asObject();

	//check that ALL keys exist AND HAVE RIGHT TYPE to avoid segfaults
	if (!Networking::CurlJsonRequest::jsonContainsArray(object, "value", "OneDriveListDirectoryRequest")) {
		error.response = "\"value\" not found or that's not an array!";
		finishError(error);
		delete json;
		return;
	}

	Common::JSONArray items = object.getVal("value")->asArray();
	for (uint32 i = 0; i < items.size(); ++i) {
		if (!Networking::CurlJsonRequest::jsonIsObject(items[i], "OneDriveListDirectoryRequest")) continue;

		Common::JSONObject item = items[i]->asObject();

		if (!Networking::CurlJsonRequest::jsonContainsAttribute(item, "folder", "OneDriveListDirectoryRequest", true)) continue;
		if (!Networking::CurlJsonRequest::jsonContainsString(item, "name", "OneDriveListDirectoryRequest")) continue;
		if (!Networking::CurlJsonRequest::jsonContainsIntegerNumber(item, "size", "OneDriveListDirectoryRequest")) continue;
		if (!Networking::CurlJsonRequest::jsonContainsString(item, "lastModifiedDateTime", "OneDriveListDirectoryRequest")) continue;

		Common::String path = _currentDirectory + item.getVal("name")->asString();
		bool isDirectory = item.contains("folder");
		uint32 size = item.getVal("size")->asIntegerNumber();
		uint32 timestamp = ISO8601::convertToTimestamp(item.getVal("lastModifiedDateTime")->asString());

		StorageFile file(path, size, timestamp, isDirectory);
		_files.push_back(file);
		if (_requestedRecursive && file.isDirectory()) {
			_directoriesQueue.push_back(file.path());
		}
	}

	bool hasMore = object.contains("@odata.nextLink");
	if (hasMore) {
		if (!Networking::CurlJsonRequest::jsonContainsString(object, "@odata.nextLink", "OneDriveListDirectoryRequest")) {
			error.response = "\"@odata.nextLink\" is not a string!";
			finishError(error);
			delete json;
			return;
		}

		makeRequest(object.getVal("@odata.nextLink")->asString());
	} else {
		listNextDirectory();
	}

	delete json;
}

void OneDriveListDirectoryRequest::listedDirectoryErrorCallback(Networking::ErrorResponse error) {
	_workingRequest = nullptr;
	if (_ignoreCallback)
		return;
	if (error.request)
		_date = error.request->date();
	finishError(error);
}

void OneDriveListDirectoryRequest::handle() {}

void OneDriveListDirectoryRequest::restart() { start(); }

Common::String OneDriveListDirectoryRequest::date() const { return _date; }

void OneDriveListDirectoryRequest::finishListing(Common::Array<StorageFile> &files) {
	Request::finishSuccess();
	if (_listDirectoryCallback)
		(*_listDirectoryCallback)(Storage::ListDirectoryResponse(this, files));
}

} // End of namespace OneDrive
} // End of namespace Cloud
