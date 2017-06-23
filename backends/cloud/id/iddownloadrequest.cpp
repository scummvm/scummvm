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

#include "backends/cloud/id/iddownloadrequest.h"
#include "backends/cloud/id/idstorage.h"
#include "backends/cloud/downloadrequest.h"

namespace Cloud {
namespace Id {

IdDownloadRequest::IdDownloadRequest(IdStorage *storage, Common::String remotePath, Common::String localPath, Storage::BoolCallback cb, Networking::ErrorCallback ecb):
	Networking::Request(nullptr, ecb), _requestedFile(remotePath), _requestedLocalFile(localPath), _storage(storage), _boolCallback(cb),
	_workingRequest(nullptr), _ignoreCallback(false) {
	start();
}

IdDownloadRequest::~IdDownloadRequest() {
	_ignoreCallback = true;
	if (_workingRequest)
		_workingRequest->finish();
	delete _boolCallback;
}

void IdDownloadRequest::start() {
	//cleanup
	_ignoreCallback = true;
	if (_workingRequest)
		_workingRequest->finish();
	_workingRequest = nullptr;
	_ignoreCallback = false;

	//find file's id
	Storage::UploadCallback innerCallback = new Common::Callback<IdDownloadRequest, Storage::UploadResponse>(this, &IdDownloadRequest::idResolvedCallback);
	Networking::ErrorCallback innerErrorCallback = new Common::Callback<IdDownloadRequest, Networking::ErrorResponse>(this, &IdDownloadRequest::idResolveFailedCallback);
	_workingRequest = _storage->resolveFileId(_requestedFile, innerCallback, innerErrorCallback);
}

void IdDownloadRequest::idResolvedCallback(Storage::UploadResponse response) {
	_workingRequest = nullptr;
	if (_ignoreCallback)
		return;

	Storage::BoolCallback innerCallback = new Common::Callback<IdDownloadRequest, Storage::BoolResponse>(this, &IdDownloadRequest::downloadCallback);
	Networking::ErrorCallback innerErrorCallback = new Common::Callback<IdDownloadRequest, Networking::ErrorResponse>(this, &IdDownloadRequest::downloadErrorCallback);
	_workingRequest = _storage->downloadById(response.value.id(), _requestedLocalFile, innerCallback, innerErrorCallback);
}

void IdDownloadRequest::idResolveFailedCallback(Networking::ErrorResponse error) {
	_workingRequest = nullptr;
	if (_ignoreCallback)
		return;
	finishError(error);
}

void IdDownloadRequest::downloadCallback(Storage::BoolResponse response) {
	_workingRequest = nullptr;
	if (_ignoreCallback)
		return;
	finishDownload(response.value);
}

void IdDownloadRequest::downloadErrorCallback(Networking::ErrorResponse error) {
	_workingRequest = nullptr;
	if (_ignoreCallback)
		return;
	finishError(error);
}

void IdDownloadRequest::handle() {}

void IdDownloadRequest::restart() { start(); }

void IdDownloadRequest::finishDownload(bool success) {
	Request::finishSuccess();
	if (_boolCallback)
		(*_boolCallback)(Storage::BoolResponse(this, success));
}

double IdDownloadRequest::getProgress() const {
	DownloadRequest *downloadRequest = dynamic_cast<DownloadRequest *>(_workingRequest);
	if (downloadRequest == nullptr)
		return 0; // resolving id still

	// id resolve is 10 % and download is the other 90 %
	return 0.1 + 0.9 * downloadRequest->getProgress(); // downloading
}

} // End of namespace Id
} // End of namespace Cloud
