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

#include "backends/cloud/id/idresolveidrequest.h"
#include "backends/cloud/id/idstorage.h"

namespace Cloud {
namespace Id {

IdResolveIdRequest::IdResolveIdRequest(IdStorage *storage, Common::String path, Storage::UploadCallback cb, Networking::ErrorCallback ecb, bool recursive):
	Networking::Request(nullptr, ecb),
	_requestedPath(path), _storage(storage), _uploadCallback(cb),
	_workingRequest(nullptr), _ignoreCallback(false) {
	start();
}

IdResolveIdRequest::~IdResolveIdRequest() {
	_ignoreCallback = true;
	if (_workingRequest)
		_workingRequest->finish();
	delete _uploadCallback;
}

void IdResolveIdRequest::start() {
	//cleanup
	_ignoreCallback = true;
	if (_workingRequest)
		_workingRequest->finish();
	_workingRequest = nullptr;
	_currentDirectory = "";
	_currentDirectoryId = _storage->getRootDirectoryId();
	_ignoreCallback = false;

	listNextDirectory(StorageFile(_currentDirectoryId, 0, 0, true));
}

void IdResolveIdRequest::listNextDirectory(StorageFile fileToReturn) {
	if (_currentDirectory.equalsIgnoreCase(_requestedPath)) {
		finishFile(fileToReturn);
		return;
	}

	Storage::FileArrayCallback callback = new Common::Callback<IdResolveIdRequest, Storage::FileArrayResponse>(this, &IdResolveIdRequest::listedDirectoryCallback);
	Networking::ErrorCallback failureCallback = new Common::Callback<IdResolveIdRequest, Networking::ErrorResponse>(this, &IdResolveIdRequest::listedDirectoryErrorCallback);
	_workingRequest = _storage->listDirectoryById(_currentDirectoryId, callback, failureCallback);
}

void IdResolveIdRequest::listedDirectoryCallback(Storage::FileArrayResponse response) {
	_workingRequest = nullptr;
	if (_ignoreCallback)
		return;

	Common::String currentLevelName = _requestedPath;
	///debug(9, "'%s'", currentLevelName.c_str());
	if (_currentDirectory.size())
		currentLevelName.erase(0, _currentDirectory.size());
	if (currentLevelName.size() && (currentLevelName[0] == '/' || currentLevelName[0] == '\\'))
		currentLevelName.erase(0, 1);
	///debug(9, "'%s'", currentLevelName.c_str());
	for (uint32 i = 0; i < currentLevelName.size(); ++i) {
		if (currentLevelName[i] == '/' || currentLevelName[i] == '\\') {
			currentLevelName.erase(i);
			///debug(9, "'%s'", currentLevelName.c_str());
			break;
		}
	}

	Common::String path = _currentDirectory;
	if (path != "")
		path += "/";
	path += currentLevelName;
	bool lastLevel = (path.equalsIgnoreCase(_requestedPath));

	///debug(9, "IdResolveIdRequest: searching for '%s' in '%s'", currentLevelName.c_str(), _currentDirectory.c_str());

	Common::Array<StorageFile> &files = response.value;
	bool found = false;
	for (uint32 i = 0; i < files.size(); ++i) {
		if ((files[i].isDirectory() || lastLevel) && files[i].name().equalsIgnoreCase(currentLevelName)) {
			if (_currentDirectory != "")
				_currentDirectory += "/";
			_currentDirectory += files[i].name();
			_currentDirectoryId = files[i].id();
			///debug(9, "IdResolveIdRequest: found it! new directory and its id: '%s', '%s'", _currentDirectory.c_str(), _currentDirectoryId.c_str());
			listNextDirectory(files[i]);
			found = true;
			break;
		}
	}

	if (!found) {
		if (lastLevel)
			finishError(Networking::ErrorResponse(this, false, true, Common::String("no such file found in its parent directory\n") + _currentDirectoryId, 404));
		else
			finishError(Networking::ErrorResponse(this, false, true, "subdirectory not found", 400));
	}
}

void IdResolveIdRequest::listedDirectoryErrorCallback(Networking::ErrorResponse error) {
	_workingRequest = nullptr;
	if (_ignoreCallback)
		return;
	finishError(error);
}

void IdResolveIdRequest::handle() {}

void IdResolveIdRequest::restart() { start(); }

void IdResolveIdRequest::finishFile(StorageFile file) {
	Request::finishSuccess();
	if (_uploadCallback)
		(*_uploadCallback)(Storage::UploadResponse(this, file));
}

} // End of namespace Id
} // End of namespace Cloud
