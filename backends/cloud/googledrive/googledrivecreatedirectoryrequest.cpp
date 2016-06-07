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

#include "backends/cloud/googledrive/googledrivecreatedirectoryrequest.h"
#include "backends/cloud/googledrive/googledrivestorage.h"
#include "common/debug.h"

namespace Cloud {
namespace GoogleDrive {

GoogleDriveCreateDirectoryRequest::GoogleDriveCreateDirectoryRequest(GoogleDriveStorage *storage, Common::String parentPath, Common::String directoryName, Storage::BoolCallback cb, Networking::ErrorCallback ecb):
	Networking::Request(nullptr, ecb),
	_requestedParentPath(parentPath), _requestedDirectoryName(directoryName), _storage(storage), _boolCallback(cb),
	_workingRequest(nullptr), _ignoreCallback(false) {
	start();
}

GoogleDriveCreateDirectoryRequest::~GoogleDriveCreateDirectoryRequest() {
	_ignoreCallback = true;
	if (_workingRequest) _workingRequest->finish();
	delete _boolCallback;
}

void GoogleDriveCreateDirectoryRequest::start() {
	//cleanup
	_ignoreCallback = true;
	if (_workingRequest) _workingRequest->finish();
	_workingRequest = nullptr;
	_ignoreCallback = false;

	//the only exception when we create parent folder - is when it's ScummVM/ base folder
	Common::String prefix = _requestedParentPath;
	if (prefix.size() > 7) prefix.erase(7);
	if (prefix.equalsIgnoreCase("ScummVM")) {
		Storage::BoolCallback callback = new Common::Callback<GoogleDriveCreateDirectoryRequest, Storage::BoolResponse>(this, &GoogleDriveCreateDirectoryRequest::createdBaseDirectoryCallback);
		Networking::ErrorCallback failureCallback = new Common::Callback<GoogleDriveCreateDirectoryRequest, Networking::ErrorResponse>(this, &GoogleDriveCreateDirectoryRequest::createdBaseDirectoryErrorCallback);
		_workingRequest = _storage->createDirectory("ScummVM", callback, failureCallback);
		return;
	}
	
	resolveId();
}

void GoogleDriveCreateDirectoryRequest::createdBaseDirectoryCallback(Storage::BoolResponse response) {
	_workingRequest = nullptr;
	if (_ignoreCallback) return;
	resolveId();
}

void GoogleDriveCreateDirectoryRequest::createdBaseDirectoryErrorCallback(Networking::ErrorResponse error) {
	_workingRequest = nullptr;
	if (_ignoreCallback) return;
	finishError(error);
}

void GoogleDriveCreateDirectoryRequest::resolveId() {
	//check whether such folder already exists
	Storage::UploadCallback innerCallback = new Common::Callback<GoogleDriveCreateDirectoryRequest, Storage::UploadResponse>(this, &GoogleDriveCreateDirectoryRequest::idResolvedCallback);
	Networking::ErrorCallback innerErrorCallback = new Common::Callback<GoogleDriveCreateDirectoryRequest, Networking::ErrorResponse>(this, &GoogleDriveCreateDirectoryRequest::idResolveFailedCallback);
	Common::String path = _requestedParentPath;
	if (_requestedParentPath != "") path += "/";
	path += _requestedDirectoryName;
	_workingRequest = _storage->resolveFileId(path, innerCallback, innerErrorCallback);
}

void GoogleDriveCreateDirectoryRequest::idResolvedCallback(Storage::UploadResponse response) {
	_workingRequest = nullptr;
	if (_ignoreCallback) return;

	//resolved => folder already exists
	finishSuccess(false);
}

void GoogleDriveCreateDirectoryRequest::idResolveFailedCallback(Networking::ErrorResponse error) {
	_workingRequest = nullptr;
	if (_ignoreCallback) return;
	
	//not resolved => folder not exists
	if (error.response.contains("no such file found in its parent directory")) {		
		//parent's id after the '\n'
		Common::String parentId = error.response;
		for (uint32 i = 0; i < parentId.size(); ++i)
			if (parentId[i] == '\n') {
				parentId.erase(0, i+1);
				break;
			}

		Storage::BoolCallback callback = new Common::Callback<GoogleDriveCreateDirectoryRequest, Storage::BoolResponse>(this, &GoogleDriveCreateDirectoryRequest::createdDirectoryCallback);
		Networking::ErrorCallback failureCallback = new Common::Callback<GoogleDriveCreateDirectoryRequest, Networking::ErrorResponse>(this, &GoogleDriveCreateDirectoryRequest::createdDirectoryErrorCallback);
		_workingRequest = _storage->createDirectoryWithParentId(parentId, _requestedDirectoryName, callback, failureCallback);
		return;
	}

	finishError(error);
}

void GoogleDriveCreateDirectoryRequest::createdDirectoryCallback(Storage::BoolResponse response) {
	_workingRequest = nullptr;
	if (_ignoreCallback) return;
	finishSuccess(response.value);
}

void GoogleDriveCreateDirectoryRequest::createdDirectoryErrorCallback(Networking::ErrorResponse error) {
	_workingRequest = nullptr;
	if (_ignoreCallback) return;
	finishError(error);
}

void GoogleDriveCreateDirectoryRequest::handle() {}

void GoogleDriveCreateDirectoryRequest::restart() { start(); }

void GoogleDriveCreateDirectoryRequest::finishSuccess(bool success) {
	Request::finishSuccess();
	if (_boolCallback) (*_boolCallback)(Storage::BoolResponse(this, success));
}

} // End of namespace GoogleDrive
} // End of namespace Cloud
