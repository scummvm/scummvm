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

OneDriveListDirectoryRequest::OneDriveListDirectoryRequest(OneDriveStorage *storage, Common::String path, Storage::ListDirectoryCallback cb, bool recursive):
	Networking::Request(0),
	_requestedPath(path), _requestedRecursive(recursive), _storage(storage), _listDirectoryCallback(cb),
	_workingRequest(nullptr), _ignoreCallback(false) {
	start();
}

OneDriveListDirectoryRequest::~OneDriveListDirectoryRequest() {
	_ignoreCallback = true;
	if (_workingRequest) _workingRequest->finish();
	delete _listDirectoryCallback;
}

void OneDriveListDirectoryRequest::start() {
	//cleanup
	_ignoreCallback = true;
	if (_workingRequest) _workingRequest->finish();
	_workingRequest = nullptr;
	_files.clear();
	_directoriesQueue.clear();
	_currentDirectory = "";
	_ignoreCallback = false;

	_directoriesQueue.push_back(_requestedPath);
	listNextDirectory(_files);
}

void OneDriveListDirectoryRequest::listNextDirectory(ListDirectoryStatus status) {
	if (_directoriesQueue.empty()) {
		finishStatus(status);
		return;
	}

	_currentDirectory = _directoriesQueue.back();
	_directoriesQueue.pop_back();

	if (_currentDirectory != "" && _currentDirectory.lastChar() != '/' && _currentDirectory.lastChar() != '\\')
		_currentDirectory += '/';

	Common::String url = "https://api.onedrive.com/v1.0/drive/special/approot:/" + _currentDirectory;
	url.deleteLastChar();
	url += ":/children";
	makeRequest(url);
}

void OneDriveListDirectoryRequest::makeRequest(Common::String url) {	
	Networking::JsonCallback callback = new Common::Callback<OneDriveListDirectoryRequest, Networking::JsonResponse>(this, &OneDriveListDirectoryRequest::listedDirectoryCallback);
	Networking::CurlJsonRequest *request = new OneDriveTokenRefresher(_storage, callback, url.c_str());
	request->addHeader("Authorization: Bearer " + _storage->accessToken());
	_workingRequest = ConnMan.addRequest(request);
}

void OneDriveListDirectoryRequest::listedDirectoryCallback(Networking::JsonResponse pair) {
	_workingRequest = nullptr;
	Common::JSONValue *json = pair.value;

	if (_ignoreCallback) {
		delete json;
		return;
	}

	ListDirectoryStatus status(_files);
	Networking::CurlJsonRequest *rq = (Networking::CurlJsonRequest *)pair.request;
	if (rq && rq->getNetworkReadStream())
		status.httpResponseCode = rq->getNetworkReadStream()->httpResponseCode();

	if (!json) {
		status.failed = true;
		finishStatus(status);
		return;
	}

	Common::JSONObject response = json->asObject();		
	
	//TODO: check that ALL keys exist AND HAVE RIGHT TYPE to avoid segfaults

	Common::JSONArray items = response.getVal("value")->asArray();
	for (uint32 i = 0; i < items.size(); ++i) {
		Common::JSONObject item = items[i]->asObject();	

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

	bool hasMore = response.contains("@odata.nextLink");
	if (hasMore) {
		makeRequest(response.getVal("@odata.nextLink")->asString());
	} else {
		listNextDirectory(status);
	}

	delete json;
}

void OneDriveListDirectoryRequest::finish() {	
	Common::Array<StorageFile> files;
	ListDirectoryStatus status(files);
	status.interrupted = true;
	finishStatus(status);
}

void OneDriveListDirectoryRequest::finishStatus(ListDirectoryStatus status) {
	Request::finish();
	if (_listDirectoryCallback) (*_listDirectoryCallback)(Storage::ListDirectoryResponse(this, status));
}

} // End of namespace OneDrive
} // End of namespace Cloud
