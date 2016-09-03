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

#include "backends/cloud/id/idlistdirectoryrequest.h"
#include "backends/cloud/id/idstorage.h"

namespace Cloud {
namespace Id {

IdListDirectoryRequest::IdListDirectoryRequest(IdStorage *storage, Common::String path, Storage::ListDirectoryCallback cb, Networking::ErrorCallback ecb, bool recursive):
	Networking::Request(nullptr, ecb),
	_requestedPath(path), _requestedRecursive(recursive), _storage(storage), _listDirectoryCallback(cb),
	_workingRequest(nullptr), _ignoreCallback(false) {
	start();
}

IdListDirectoryRequest::~IdListDirectoryRequest() {
	_ignoreCallback = true;
	if (_workingRequest)
		_workingRequest->finish();
	delete _listDirectoryCallback;
}

void IdListDirectoryRequest::start() {
	//cleanup
	_ignoreCallback = true;
	if (_workingRequest)
		_workingRequest->finish();
	_workingRequest = nullptr;
	_files.clear();
	_directoriesQueue.clear();
	_currentDirectory = StorageFile();
	_ignoreCallback = false;

	//find out that directory's id
	Storage::UploadCallback innerCallback = new Common::Callback<IdListDirectoryRequest, Storage::UploadResponse>(this, &IdListDirectoryRequest::idResolvedCallback);
	Networking::ErrorCallback innerErrorCallback = new Common::Callback<IdListDirectoryRequest, Networking::ErrorResponse>(this, &IdListDirectoryRequest::idResolveErrorCallback);
	_workingRequest = _storage->resolveFileId(_requestedPath, innerCallback, innerErrorCallback);
}

void IdListDirectoryRequest::idResolvedCallback(Storage::UploadResponse response) {
	_workingRequest = nullptr;
	if (_ignoreCallback)
		return;
	if (response.request)
		_date = response.request->date();

	StorageFile directory = response.value;
	directory.setPath(_requestedPath);
	_directoriesQueue.push_back(directory);
	listNextDirectory();
}

void IdListDirectoryRequest::idResolveErrorCallback(Networking::ErrorResponse error) {
	_workingRequest = nullptr;
	if (_ignoreCallback)
		return;
	if (error.request)
		_date = error.request->date();
	finishError(error);
}

void IdListDirectoryRequest::listNextDirectory() {
	if (_directoriesQueue.empty()) {
		finishListing(_files);
		return;
	}

	_currentDirectory = _directoriesQueue.back();
	_directoriesQueue.pop_back();

	Storage::FileArrayCallback callback = new Common::Callback<IdListDirectoryRequest, Storage::FileArrayResponse>(this, &IdListDirectoryRequest::listedDirectoryCallback);
	Networking::ErrorCallback failureCallback = new Common::Callback<IdListDirectoryRequest, Networking::ErrorResponse>(this, &IdListDirectoryRequest::listedDirectoryErrorCallback);
	_workingRequest = _storage->listDirectoryById(_currentDirectory.id(), callback, failureCallback);
}

void IdListDirectoryRequest::listedDirectoryCallback(Storage::FileArrayResponse response) {
	_workingRequest = nullptr;
	if (_ignoreCallback)
		return;
	if (response.request)
		_date = response.request->date();

	for (uint32 i = 0; i < response.value.size(); ++i) {
		StorageFile &file = response.value[i];
		Common::String path = _currentDirectory.path();
		if (path.size() && path.lastChar() != '/' && path.lastChar() != '\\')
			path += '/';
		path += file.name();
		file.setPath(path);
		_files.push_back(file);
		if (_requestedRecursive && file.isDirectory()) {
			_directoriesQueue.push_back(file);
		}
	}

	listNextDirectory();
}

void IdListDirectoryRequest::listedDirectoryErrorCallback(Networking::ErrorResponse error) {
	_workingRequest = nullptr;
	if (_ignoreCallback)
		return;
	if (error.request)
		_date = error.request->date();
	finishError(error);
}

void IdListDirectoryRequest::handle() {}

void IdListDirectoryRequest::restart() { start(); }

Common::String IdListDirectoryRequest::date() const { return _date; }

void IdListDirectoryRequest::finishListing(Common::Array<StorageFile> &files) {
	Request::finishSuccess();
	if (_listDirectoryCallback)
		(*_listDirectoryCallback)(Storage::ListDirectoryResponse(this, files));
}

} // End of namespace Id
} // End of namespace Cloud
