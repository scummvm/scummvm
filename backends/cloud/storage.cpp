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

#include "backends/cloud/storage.h"
#include "backends/cloud/downloadrequest.h"
#include "backends/cloud/folderdownloadrequest.h"
#include "backends/cloud/savessyncrequest.h"
#include "backends/networking/curl/connectionmanager.h"
#include "common/debug.h"
#include "common/file.h"

namespace Cloud {

Storage::Storage(): _runningRequestsCount(0), _savesSyncRequest(nullptr) {}

Storage::~Storage() {}

Networking::ErrorCallback Storage::getErrorPrintingCallback() {
	return new Common::Callback<Storage, Networking::ErrorResponse>(this, &Storage::printErrorResponse);
}

void Storage::printErrorResponse(Networking::ErrorResponse error) {
	debug("error response (%s, %ld):", (error.failed ? "failed" : "interrupted"), error.httpResponseCode);
	debug("%s", error.response.c_str());
}

Networking::Request *Storage::addRequest(Networking::Request *request) {
	_runningRequestsMutex.lock();
	++_runningRequestsCount;
	if (_runningRequestsCount == 1) debug("Storage is working now");
	_runningRequestsMutex.unlock();
	return ConnMan.addRequest(request, new Common::Callback<Storage, Networking::Request *>(this, &Storage::requestFinishedCallback));
}

void Storage::requestFinishedCallback(Networking::Request *invalidRequestPointer) {
	_runningRequestsMutex.lock();
	if (invalidRequestPointer == _savesSyncRequest)
		_savesSyncRequest = nullptr;
	--_runningRequestsCount;
	if (_runningRequestsCount == 0) debug("Storage is not working now");
	_runningRequestsMutex.unlock();
}

Networking::Request *Storage::upload(Common::String remotePath, Common::String localPath, UploadCallback callback, Networking::ErrorCallback errorCallback) {
	if (!errorCallback) errorCallback = getErrorPrintingCallback();

	Common::File *f = new Common::File();
	if (!f->open(localPath)) {
		warning("Storage: unable to open file to upload from");
		if (errorCallback) (*errorCallback)(Networking::ErrorResponse(nullptr, false, true, "", -1));
		delete errorCallback;
		delete callback;
		delete f;
		return nullptr;
	}

	return upload(remotePath, f, callback, errorCallback);
}

Networking::Request *Storage::streamFile(Common::String path, Networking::NetworkReadStreamCallback callback, Networking::ErrorCallback errorCallback) {
	//most Storages use paths instead of ids, so this should work
	return streamFile(path, callback, errorCallback);
}

Networking::Request *Storage::download(Common::String remotePath, Common::String localPath, BoolCallback callback, Networking::ErrorCallback errorCallback) {
	//most Storages use paths instead of ids, so this should work
	return downloadById(remotePath, localPath, callback, errorCallback);
}

Networking::Request *Storage::downloadById(Common::String remoteId, Common::String localPath, BoolCallback callback, Networking::ErrorCallback errorCallback) {
	if (!errorCallback) errorCallback = getErrorPrintingCallback();

	Common::DumpFile *f = new Common::DumpFile();
	if (!f->open(localPath, true)) {
		warning("Storage: unable to open file to download into");
		if (errorCallback) (*errorCallback)(Networking::ErrorResponse(nullptr, false, true, "", -1));
		delete errorCallback;
		delete callback;
		delete f;
		return nullptr;
	}

	return addRequest(new DownloadRequest(this, callback, errorCallback, remoteId, f));
}

Networking::Request *Storage::downloadFolder(Common::String remotePath, Common::String localPath, FileArrayCallback callback, Networking::ErrorCallback errorCallback, bool recursive) {
	if (!errorCallback) errorCallback = getErrorPrintingCallback();
	return addRequest(new FolderDownloadRequest(this, callback, errorCallback, remotePath, localPath, recursive));
}

SavesSyncRequest *Storage::syncSaves(BoolCallback callback, Networking::ErrorCallback errorCallback) {
	_runningRequestsMutex.lock();
	if (_savesSyncRequest) {
		warning("Storage::syncSaves: there is a sync in progress already");
		_runningRequestsMutex.unlock();
		return _savesSyncRequest;
	}
	if (!errorCallback) errorCallback = getErrorPrintingCallback();
	_savesSyncRequest = new SavesSyncRequest(this, callback, errorCallback);
	_runningRequestsMutex.unlock();	
	return (SavesSyncRequest *)addRequest(_savesSyncRequest); //who knows what that ConnMan could return in the future
}

bool Storage::isWorking() {
	_runningRequestsMutex.lock();
	bool working = _runningRequestsCount > 0;
	_runningRequestsMutex.unlock();
	return working;
}

bool Storage::isSyncing() {
	_runningRequestsMutex.lock();
	bool syncing = _savesSyncRequest != nullptr;
	_runningRequestsMutex.unlock();
	return syncing;
}

double Storage::getSyncDownloadingProgress() {
	double result = 1;
	_runningRequestsMutex.lock();
	if (_savesSyncRequest)
		result = _savesSyncRequest->getDownloadingProgress();
	_runningRequestsMutex.unlock();
	return result;
}

double Storage::getSyncProgress() {
	double result = 1;
	_runningRequestsMutex.lock();
	if (_savesSyncRequest)
		result = _savesSyncRequest->getProgress();
	_runningRequestsMutex.unlock();
	return result;
}

Common::Array<Common::String> Storage::getSyncingFiles() {
	Common::Array<Common::String> result;
	_runningRequestsMutex.lock();
	if (_savesSyncRequest)
		result = _savesSyncRequest->getFilesToDownload();
	_runningRequestsMutex.unlock();
	return result;
}

void Storage::cancelSync() {
	_runningRequestsMutex.lock();
	if (_savesSyncRequest)
		_savesSyncRequest->finish();
	_runningRequestsMutex.unlock();
}

void Storage::setSyncTarget(GUI::CommandReceiver *target) {
	_runningRequestsMutex.lock();
	if (_savesSyncRequest)
		_savesSyncRequest->setTarget(target);
	_runningRequestsMutex.unlock();
}

} // End of namespace Cloud

