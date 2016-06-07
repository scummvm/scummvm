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

#include "backends/cloud/googledrive/googledrivelistdirectoryrequest.h"
#include "backends/cloud/googledrive/googledrivestorage.h"

namespace Cloud {
namespace GoogleDrive {

GoogleDriveListDirectoryRequest::GoogleDriveListDirectoryRequest(GoogleDriveStorage *storage, Common::String path, Storage::ListDirectoryCallback cb, Networking::ErrorCallback ecb, bool recursive):
	Networking::Request(nullptr, ecb),
	_requestedPath(path), _requestedRecursive(recursive), _storage(storage), _listDirectoryCallback(cb),
	_workingRequest(nullptr), _ignoreCallback(false) {
	start();
}

GoogleDriveListDirectoryRequest::~GoogleDriveListDirectoryRequest() {
	_ignoreCallback = true;
	if (_workingRequest) _workingRequest->finish();
	delete _listDirectoryCallback;
}

void GoogleDriveListDirectoryRequest::start() {
	//cleanup
	_ignoreCallback = true;
	if (_workingRequest) _workingRequest->finish();
	_workingRequest = nullptr;
	_files.clear();
	_directoriesQueue.clear();
	_currentDirectory = "";
	_ignoreCallback = false;

	//find out that directory's id
	Storage::UploadCallback innerCallback = new Common::Callback<GoogleDriveListDirectoryRequest, Storage::UploadResponse>(this, &GoogleDriveListDirectoryRequest::idResolvedCallback);
	Networking::ErrorCallback innerErrorCallback = new Common::Callback<GoogleDriveListDirectoryRequest, Networking::ErrorResponse>(this, &GoogleDriveListDirectoryRequest::idResolveErrorCallback);
	_workingRequest = _storage->resolveFileId(_requestedPath, innerCallback, innerErrorCallback);
}

void GoogleDriveListDirectoryRequest::idResolvedCallback(Storage::UploadResponse response) {
	_workingRequest = nullptr;
	if (_ignoreCallback) return;

	_directoriesQueue.push_back(response.value.path());
	listNextDirectory();
}

void GoogleDriveListDirectoryRequest::idResolveErrorCallback(Networking::ErrorResponse error) {
	_workingRequest = nullptr;
	if (_ignoreCallback) return;
	finishError(error);
}

void GoogleDriveListDirectoryRequest::listNextDirectory() {
	if (_directoriesQueue.empty()) {
		finishSuccess(_files);
		return;
	}

	_currentDirectory = _directoriesQueue.back();
	_directoriesQueue.pop_back();

	Storage::FileArrayCallback callback = new Common::Callback<GoogleDriveListDirectoryRequest, Storage::FileArrayResponse>(this, &GoogleDriveListDirectoryRequest::listedDirectoryCallback);
	Networking::ErrorCallback failureCallback = new Common::Callback<GoogleDriveListDirectoryRequest, Networking::ErrorResponse>(this, &GoogleDriveListDirectoryRequest::listedDirectoryErrorCallback);	
	_workingRequest = _storage->listDirectoryById(_currentDirectory, callback, failureCallback);
}

void GoogleDriveListDirectoryRequest::listedDirectoryCallback(Storage::FileArrayResponse response) {
	_workingRequest = nullptr;
	if (_ignoreCallback) return;

	for (uint32 i = 0; i < response.value.size(); ++i) {
		StorageFile &file = response.value[i];
		_files.push_back(file);
		if (_requestedRecursive && file.isDirectory()) {
			_directoriesQueue.push_back(file.path());
		}
	}

	listNextDirectory();
}

void GoogleDriveListDirectoryRequest::listedDirectoryErrorCallback(Networking::ErrorResponse error) {
	_workingRequest = nullptr;
	if (_ignoreCallback) return;
	finishError(error);
}

void GoogleDriveListDirectoryRequest::handle() {}

void GoogleDriveListDirectoryRequest::restart() { start(); }

void GoogleDriveListDirectoryRequest::finishSuccess(Common::Array<StorageFile> &files) {
	Request::finishSuccess();
	if (_listDirectoryCallback) (*_listDirectoryCallback)(Storage::ListDirectoryResponse(this, files));
}

} // End of namespace GoogleDrive
} // End of namespace Cloud
