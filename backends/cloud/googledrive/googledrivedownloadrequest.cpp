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

#include "backends/cloud/googledrive/googledrivedownloadrequest.h"
#include "backends/cloud/googledrive/googledrivestorage.h"

namespace Cloud {
namespace GoogleDrive {

GoogleDriveDownloadRequest::GoogleDriveDownloadRequest(GoogleDriveStorage *storage, Common::String remotePath, Common::String localPath, Storage::BoolCallback cb, Networking::ErrorCallback ecb):
	Networking::Request(nullptr, ecb), _requestedFile(remotePath), _requestedLocalFile(localPath), _storage(storage), _boolCallback(cb),
	_workingRequest(nullptr), _ignoreCallback(false) {
	start();
}

GoogleDriveDownloadRequest::~GoogleDriveDownloadRequest() {
	_ignoreCallback = true;
	if (_workingRequest) _workingRequest->finish();
	delete _boolCallback;
}

void GoogleDriveDownloadRequest::start() {
	//cleanup
	_ignoreCallback = true;
	if (_workingRequest) _workingRequest->finish();
	_workingRequest = nullptr;
	_ignoreCallback = false;

	//find file's id
	Storage::UploadCallback innerCallback = new Common::Callback<GoogleDriveDownloadRequest, Storage::UploadResponse>(this, &GoogleDriveDownloadRequest::idResolvedCallback);
	Networking::ErrorCallback innerErrorCallback = new Common::Callback<GoogleDriveDownloadRequest, Networking::ErrorResponse>(this, &GoogleDriveDownloadRequest::idResolveFailedCallback);	
	_workingRequest = _storage->resolveFileId(_requestedFile, innerCallback, innerErrorCallback);
}

void GoogleDriveDownloadRequest::idResolvedCallback(Storage::UploadResponse response) {
	_workingRequest = nullptr;
	if (_ignoreCallback) return;

	Storage::BoolCallback innerCallback = new Common::Callback<GoogleDriveDownloadRequest, Storage::BoolResponse>(this, &GoogleDriveDownloadRequest::downloadCallback);
	Networking::ErrorCallback innerErrorCallback = new Common::Callback<GoogleDriveDownloadRequest, Networking::ErrorResponse>(this, &GoogleDriveDownloadRequest::downloadErrorCallback);
	_workingRequest = _storage->downloadById(response.value.id(), _requestedLocalFile, innerCallback, innerErrorCallback);
}

void GoogleDriveDownloadRequest::idResolveFailedCallback(Networking::ErrorResponse error) {
	_workingRequest = nullptr;
	if (_ignoreCallback) return;
	finishError(error);
}

void GoogleDriveDownloadRequest::downloadCallback(Storage::BoolResponse response) {
	_workingRequest = nullptr;
	if (_ignoreCallback) return;
	finishDownload(response.value);
}

void GoogleDriveDownloadRequest::downloadErrorCallback(Networking::ErrorResponse error) {
	_workingRequest = nullptr;
	if (_ignoreCallback) return;
	finishError(error);
}

void GoogleDriveDownloadRequest::handle() {}

void GoogleDriveDownloadRequest::restart() { start(); }

void GoogleDriveDownloadRequest::finishDownload(bool success) {
	Request::finishSuccess();
	if (_boolCallback) (*_boolCallback)(Storage::BoolResponse(this, success));
}

} // End of namespace GoogleDrive
} // End of namespace Cloud
