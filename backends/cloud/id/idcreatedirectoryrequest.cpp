/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "backends/cloud/id/idcreatedirectoryrequest.h"
#include "backends/cloud/id/idstorage.h"
#include "common/debug.h"

namespace Cloud {
namespace Id {

IdCreateDirectoryRequest::IdCreateDirectoryRequest(IdStorage *storage, const Common::String &parentPath, const Common::String &directoryName, Storage::BoolCallback cb, Networking::ErrorCallback ecb):
	Networking::Request(nullptr, ecb),
	_requestedParentPath(parentPath), _requestedDirectoryName(directoryName), _storage(storage), _boolCallback(cb),
	_workingRequest(nullptr), _ignoreCallback(false) {
	start();
}

IdCreateDirectoryRequest::~IdCreateDirectoryRequest() {
	_ignoreCallback = true;
	if (_workingRequest)
		_workingRequest->finish();
	delete _boolCallback;
}

void IdCreateDirectoryRequest::start() {
	//cleanup
	_ignoreCallback = true;
	if (_workingRequest)
		_workingRequest->finish();
	_workingRequest = nullptr;
	_ignoreCallback = false;

	//the only exception when we create parent folder - is when it's ScummVM/ base folder
	Common::String prefix = _requestedParentPath;
	if (prefix.size() > 7)
		prefix.erase(7);
	if (prefix.equalsIgnoreCase("ScummVM")) {
		Storage::BoolCallback callback = new Common::Callback<IdCreateDirectoryRequest, const Storage::BoolResponse &>(this, &IdCreateDirectoryRequest::createdBaseDirectoryCallback);
		Networking::ErrorCallback failureCallback = new Common::Callback<IdCreateDirectoryRequest, const Networking::ErrorResponse &>(this, &IdCreateDirectoryRequest::createdBaseDirectoryErrorCallback);
		_workingRequest = _storage->createDirectory("ScummVM", callback, failureCallback);
		return;
	}

	resolveId();
}

void IdCreateDirectoryRequest::createdBaseDirectoryCallback(const Storage::BoolResponse &response) {
	_workingRequest = nullptr;
	if (_ignoreCallback)
		return;
	if (response.request)
		_date = response.request->date();
	resolveId();
}

void IdCreateDirectoryRequest::createdBaseDirectoryErrorCallback(const Networking::ErrorResponse &error) {
	_workingRequest = nullptr;
	if (_ignoreCallback)
		return;
	if (error.request)
		_date = error.request->date();
	finishError(error);
}

void IdCreateDirectoryRequest::resolveId() {
	//check whether such folder already exists
	Storage::UploadCallback innerCallback = new Common::Callback<IdCreateDirectoryRequest, const Storage::UploadResponse &>(this, &IdCreateDirectoryRequest::idResolvedCallback);
	Networking::ErrorCallback innerErrorCallback = new Common::Callback<IdCreateDirectoryRequest, const Networking::ErrorResponse &>(this, &IdCreateDirectoryRequest::idResolveFailedCallback);
	Common::String path = _requestedParentPath;
	if (_requestedParentPath != "")
		path += "/";
	path += _requestedDirectoryName;
	_workingRequest = _storage->resolveFileId(path, innerCallback, innerErrorCallback);
}

void IdCreateDirectoryRequest::idResolvedCallback(const Storage::UploadResponse &response) {
	_workingRequest = nullptr;
	if (_ignoreCallback)
		return;
	if (response.request)
		_date = response.request->date();

	//resolved => folder already exists
	finishCreation(false);
}

void IdCreateDirectoryRequest::idResolveFailedCallback(const Networking::ErrorResponse &error) {
	_workingRequest = nullptr;
	if (_ignoreCallback)
		return;
	if (error.request)
		_date = error.request->date();

	//not resolved => folder not exists
	if (error.response.contains("no such file found in its parent directory")) {
		//parent's id after the '\n'
		Common::String parentId = error.response;
		for (uint32 i = 0; i < parentId.size(); ++i)
			if (parentId[i] == '\n') {
				parentId.erase(0, i + 1);
				break;
			}

		Storage::BoolCallback callback = new Common::Callback<IdCreateDirectoryRequest, const Storage::BoolResponse &>(this, &IdCreateDirectoryRequest::createdDirectoryCallback);
		Networking::ErrorCallback failureCallback = new Common::Callback<IdCreateDirectoryRequest, const Networking::ErrorResponse &>(this, &IdCreateDirectoryRequest::createdDirectoryErrorCallback);
		_workingRequest = _storage->createDirectoryWithParentId(parentId, _requestedDirectoryName, callback, failureCallback);
		return;
	}

	finishError(error);
}

void IdCreateDirectoryRequest::createdDirectoryCallback(const Storage::BoolResponse &response) {
	_workingRequest = nullptr;
	if (_ignoreCallback)
		return;
	if (response.request)
		_date = response.request->date();
	finishCreation(response.value);
}

void IdCreateDirectoryRequest::createdDirectoryErrorCallback(const Networking::ErrorResponse &error) {
	_workingRequest = nullptr;
	if (_ignoreCallback)
		return;
	if (error.request)
		_date = error.request->date();
	finishError(error);
}

void IdCreateDirectoryRequest::handle() {}

void IdCreateDirectoryRequest::restart() { start(); }

Common::String IdCreateDirectoryRequest::date() const { return _date; }

void IdCreateDirectoryRequest::finishCreation(bool success) {
	Request::finishSuccess();
	if (_boolCallback)
		(*_boolCallback)(Storage::BoolResponse(this, success));
}

} // End of namespace Id
} // End of namespace Cloud
