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

#include "backends/cloud/googledrive/googledriveresolveidrequest.h"
#include "backends/cloud/googledrive/googledrivestorage.h"
#include "backends/cloud/googledrive/googledrivetokenrefresher.h"
#include "backends/cloud/iso8601.h"
#include "backends/networking/curl/connectionmanager.h"
#include "backends/networking/curl/networkreadstream.h"
#include "common/json.h"

namespace Cloud {
namespace GoogleDrive {

GoogleDriveResolveIdRequest::GoogleDriveResolveIdRequest(GoogleDriveStorage *storage, Common::String path, Storage::UploadCallback cb, Networking::ErrorCallback ecb, bool recursive):
	Networking::Request(nullptr, ecb),
	_requestedPath(path), _storage(storage), _uploadCallback(cb),
	_workingRequest(nullptr), _ignoreCallback(false) {
	start();
}

GoogleDriveResolveIdRequest::~GoogleDriveResolveIdRequest() {
	_ignoreCallback = true;
	if (_workingRequest) _workingRequest->finish();
	delete _uploadCallback;
}

void GoogleDriveResolveIdRequest::start() {
	//cleanup
	_ignoreCallback = true;
	if (_workingRequest) _workingRequest->finish();
	_workingRequest = nullptr;
	_currentDirectory = "";
	_currentDirectoryId = "appDataFolder";
	_ignoreCallback = false;
	
	listNextDirectory(StorageFile("", 0, 0, false));
}

void GoogleDriveResolveIdRequest::listNextDirectory(StorageFile fileToReturn) {
	if (_currentDirectory == _requestedPath) {		
		finishFile(fileToReturn);
		return;
	}

	Storage::FileArrayCallback callback = new Common::Callback<GoogleDriveResolveIdRequest, Storage::FileArrayResponse>(this, &GoogleDriveResolveIdRequest::listedDirectoryCallback);
	Networking::ErrorCallback failureCallback = new Common::Callback<GoogleDriveResolveIdRequest, Networking::ErrorResponse>(this, &GoogleDriveResolveIdRequest::listedDirectoryErrorCallback);
	_workingRequest = _storage->listDirectoryById(_currentDirectoryId, callback, failureCallback);
}

void GoogleDriveResolveIdRequest::listedDirectoryCallback(Storage::FileArrayResponse response) {	
	_workingRequest = nullptr;
	if (_ignoreCallback) return;
	
	Common::String currentLevelName = _requestedPath;
	///debug("'%s'", currentLevelName.c_str());
	if (_currentDirectory.size()) currentLevelName.erase(0, _currentDirectory.size());
	if (currentLevelName.size() && (currentLevelName[0] == '/' || currentLevelName[0] == '\\')) currentLevelName.erase(0, 1);
	///debug("'%s'", currentLevelName.c_str());
	for (uint32 i = 0; i < currentLevelName.size(); ++i) {
		if (currentLevelName[i] == '/' || currentLevelName[i] == '\\') {
			currentLevelName.erase(i);
			///debug("'%s'", currentLevelName.c_str());
			break;
		}
	}

	///debug("so, searching for '%s' in '%s'", currentLevelName.c_str(), _currentDirectory.c_str());

	Common::Array<StorageFile> &files = response.value;
	bool found = false;
	for (uint32 i = 0; i < files.size(); ++i) {
		if (files[i].isDirectory() && files[i].name() == currentLevelName) {
			if (_currentDirectory != "") _currentDirectory += "/";
			_currentDirectory += files[i].name();
			_currentDirectoryId = files[i].path();
			///debug("found it! new directory and its id: '%s', '%s'", _currentDirectory.c_str(), _currentDirectoryId.c_str());
			listNextDirectory(files[i]);
			found = true;
			break;
		}
	}

	if (!found) {
		Common::String path = _currentDirectory;
		if (path != "") path += "/";
		path += currentLevelName;
		if (path == _requestedPath) finishError(Networking::ErrorResponse(this, false, true, "no such file found in its parent directory", 404));
		else finishError(Networking::ErrorResponse(this, false, true, "subdirectory not found", 400));
	}
}

void GoogleDriveResolveIdRequest::listedDirectoryErrorCallback(Networking::ErrorResponse error) {
	_workingRequest = nullptr;
	if (_ignoreCallback) return;
	finishError(error);
}

void GoogleDriveResolveIdRequest::handle() {}

void GoogleDriveResolveIdRequest::restart() { start(); }

void GoogleDriveResolveIdRequest::finishFile(StorageFile file) {
	Request::finishSuccess();
	if (_uploadCallback) (*_uploadCallback)(Storage::UploadResponse(this, file));
}

} // End of namespace GoogleDrive
} // End of namespace Cloud
