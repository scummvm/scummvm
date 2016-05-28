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
#include "common/json.h"

namespace Cloud {
namespace OneDrive {

OneDriveListDirectoryRequest::OneDriveListDirectoryRequest(OneDriveStorage *storage, Common::String path, Storage::FileArrayCallback cb, bool recursive):
	Networking::Request(0),
	_requestedPath(path), _requestedRecursive(recursive), _storage(storage), _filesCallback(cb),
	_workingRequest(nullptr), _ignoreCallback(false) {
	start();
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
	listNextDirectory();
}

void OneDriveListDirectoryRequest::listNextDirectory() {
	if (_directoriesQueue.empty()) {
		finishFiles(_files);
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
	Common::JSONValue *json = pair.value;

	if (_ignoreCallback) {
		delete json;
		return;
	}

	if (!json) {
		finish();
		return;
	}

	Common::JSONObject response = json->asObject();		
	
	//TODO: check that all keys exist to avoid segfaults

	Common::JSONArray items = response.getVal("value")->asArray();
	for (uint32 i = 0; i < items.size(); ++i) {
		Common::JSONObject item = items[i]->asObject();	

		Common::String path = _currentDirectory + item.getVal("name")->asString();
		bool isDirectory = item.contains("folder");
		uint32 size = 0, timestamp = 0;		
		//if (!isDirectory) {
		size = item.getVal("size")->asNumber();
		timestamp = ISO8601::convertToTimestamp(item.getVal("lastModifiedDateTime")->asString());
		//}

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
		listNextDirectory();
	}

	delete json;
}

void OneDriveListDirectoryRequest::finish() {
	//TODO: indicate it's interrupted
	Common::Array<StorageFile> files;
	finishFiles(files);
}

void OneDriveListDirectoryRequest::finishFiles(Common::Array<StorageFile> &files) {
	Request::finish();
	if (_filesCallback) (*_filesCallback)(Storage::FileArrayResponse(this, files));
}

} // End of namespace OneDrive
} // End of namespace Cloud
