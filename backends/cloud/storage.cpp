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
#include <common/translation.h>
#include "common/osd_message_queue.h"

namespace Cloud {

Storage::Storage():
	_runningRequestsCount(0), _savesSyncRequest(nullptr), _syncRestartRequestsed(false),
	_downloadFolderRequest(nullptr), _isEnabled(false) {}

Storage::~Storage() {}

bool Storage::isEnabled() const {
	return _isEnabled;
}

void Storage::enable() {
	_isEnabled = true;
}

Networking::ErrorCallback Storage::getErrorPrintingCallback() {
	return new Common::Callback<Storage, Networking::ErrorResponse>(this, &Storage::printErrorResponse);
}

void Storage::printErrorResponse(Networking::ErrorResponse error) {
	debug(9, "Storage: error response (%s, %ld):", (error.failed ? "failed" : "interrupted"), error.httpResponseCode);
	debug(9, "%s", error.response.c_str());
}

Networking::Request *Storage::addRequest(Networking::Request *request) {
	_runningRequestsMutex.lock();
	++_runningRequestsCount;
	if (_runningRequestsCount == 1)
		debug(9, "Storage is working now");
	_runningRequestsMutex.unlock();
	return ConnMan.addRequest(request, new Common::Callback<Storage, Networking::Request *>(this, &Storage::requestFinishedCallback));
}

void Storage::requestFinishedCallback(Networking::Request *invalidRequestPointer) {
	bool restartSync = false;

	_runningRequestsMutex.lock();
	if (invalidRequestPointer == _savesSyncRequest)
		_savesSyncRequest = nullptr;
	--_runningRequestsCount;
	if (_syncRestartRequestsed)
		restartSync = true;
	if (_runningRequestsCount == 0 && !restartSync)
		debug(9, "Storage is not working now");
	_runningRequestsMutex.unlock();

	if (restartSync)
		syncSaves(nullptr, nullptr);
}

Networking::Request *Storage::upload(Common::String remotePath, Common::String localPath, UploadCallback callback, Networking::ErrorCallback errorCallback) {
	if (!errorCallback) errorCallback = getErrorPrintingCallback();

	Common::File *f = new Common::File();
	if (!f->open(localPath)) {
		warning("Storage: unable to open file to upload from");
		if (errorCallback)
			(*errorCallback)(Networking::ErrorResponse(nullptr, false, true, "", -1));
		delete errorCallback;
		delete callback;
		delete f;
		return nullptr;
	}

	return upload(remotePath, f, callback, errorCallback);
}

bool Storage::uploadStreamSupported() {
	return true;
}

Networking::Request *Storage::streamFile(Common::String path, Networking::NetworkReadStreamCallback callback, Networking::ErrorCallback errorCallback) {
	//most Storages use paths instead of ids, so this should work
	return streamFileById(path, callback, errorCallback);
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
	if (!_isEnabled) {
		warning("Storage::downloadFolder: cannot be run while Storage is disabled");
		if (errorCallback)
			(*errorCallback)(Networking::ErrorResponse(nullptr, false, true, "Storage is disabled.", -1));
		return nullptr;
	}
	if (!errorCallback)
		errorCallback = getErrorPrintingCallback();
	return addRequest(new FolderDownloadRequest(this, callback, errorCallback, remotePath, localPath, recursive));
}

SavesSyncRequest *Storage::syncSaves(BoolCallback callback, Networking::ErrorCallback errorCallback) {
	_runningRequestsMutex.lock();
	if (!_isEnabled) {
		warning("Storage::syncSaves: cannot be run while Storage is disabled");
		if (errorCallback)
			(*errorCallback)(Networking::ErrorResponse(nullptr, false, true, "Storage is disabled.", -1));
		_runningRequestsMutex.unlock();
		return nullptr;
	}
	if (_savesSyncRequest) {
		warning("Storage::syncSaves: there is a sync in progress already");
		_syncRestartRequestsed = true;
		_runningRequestsMutex.unlock();
		return _savesSyncRequest;
	}
	if (!callback)
		callback = new Common::Callback<Storage, BoolResponse>(this, &Storage::savesSyncDefaultCallback);
	if (!errorCallback)
		errorCallback = new Common::Callback<Storage, Networking::ErrorResponse>(this, &Storage::savesSyncDefaultErrorCallback);
	_savesSyncRequest = new SavesSyncRequest(this, callback, errorCallback);
	_syncRestartRequestsed = false;
	_runningRequestsMutex.unlock();
	return (SavesSyncRequest *)addRequest(_savesSyncRequest); //who knows what that ConnMan could return in the future
}

bool Storage::isWorking() {
	_runningRequestsMutex.lock();
	bool working = _runningRequestsCount > 0;
	_runningRequestsMutex.unlock();
	return working;
}

///// SavesSyncRequest-related /////

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

void Storage::savesSyncDefaultCallback(BoolResponse response) {
	_runningRequestsMutex.lock();
	_savesSyncRequest = nullptr;
	_runningRequestsMutex.unlock();

	if (!response.value)
		warning("SavesSyncRequest called success callback with `false` argument");
}

void Storage::savesSyncDefaultErrorCallback(Networking::ErrorResponse error) {
	_runningRequestsMutex.lock();
	_savesSyncRequest = nullptr;
	_runningRequestsMutex.unlock();

	printErrorResponse(error);

	if (error.interrupted)
		Common::OSDMessageQueue::instance().addMessage(_("Saved games sync was cancelled."));
	else
		Common::OSDMessageQueue::instance().addMessage(_("Saved games sync failed.\nCheck your Internet connection."));
}

///// DownloadFolderRequest-related /////

bool Storage::startDownload(Common::String remotePath, Common::String localPath) {
	_runningRequestsMutex.lock();
	if (_downloadFolderRequest) {
		warning("Storage::startDownload: there is a download in progress already");
		_runningRequestsMutex.unlock();
		return false;
	}
	_downloadFolderRequest = (FolderDownloadRequest *)downloadFolder(
		remotePath, localPath,
		new Common::Callback<Storage, FileArrayResponse>(this, &Storage::directoryDownloadedCallback),
		new Common::Callback<Storage, Networking::ErrorResponse>(this, &Storage::directoryDownloadedErrorCallback),
		true
	);
	_runningRequestsMutex.unlock();
	return true;
}

void Storage::cancelDownload() {
	_runningRequestsMutex.lock();
	if (_downloadFolderRequest)
		_downloadFolderRequest->finish();
	_runningRequestsMutex.unlock();
}

void Storage::setDownloadTarget(GUI::CommandReceiver *target) {
	_runningRequestsMutex.lock();
	if (_downloadFolderRequest)
		_downloadFolderRequest->setTarget(target);
	_runningRequestsMutex.unlock();
}

bool Storage::isDownloading() {
	_runningRequestsMutex.lock();
	bool syncing = _downloadFolderRequest != nullptr;
	_runningRequestsMutex.unlock();
	return syncing;
}

double Storage::getDownloadingProgress() {
	double result = 1;
	_runningRequestsMutex.lock();
	if (_downloadFolderRequest)
		result = _downloadFolderRequest->getProgress();
	_runningRequestsMutex.unlock();
	return result;
}

uint64 Storage::getDownloadBytesNumber() {
	uint64 result = 0;
	_runningRequestsMutex.lock();
	if (_downloadFolderRequest)
		result = _downloadFolderRequest->getDownloadedBytes();
	_runningRequestsMutex.unlock();
	return result;
}

uint64 Storage::getDownloadTotalBytesNumber() {
	uint64 result = 0;
	_runningRequestsMutex.lock();
	if (_downloadFolderRequest)
		result = _downloadFolderRequest->getTotalBytesToDownload();
	_runningRequestsMutex.unlock();
	return result;
}

uint64 Storage::getDownloadSpeed() {
	uint64 result = 0;
	_runningRequestsMutex.lock();
	if (_downloadFolderRequest)
		result = _downloadFolderRequest->getDownloadSpeed();
	_runningRequestsMutex.unlock();
	return result;
}

Common::String Storage::getDownloadRemoteDirectory() {
	Common::String result = "";
	_runningRequestsMutex.lock();
	if (_downloadFolderRequest)
		result = _downloadFolderRequest->getRemotePath();
	_runningRequestsMutex.unlock();
	return result;
}

Common::String Storage::getDownloadLocalDirectory() {
	Common::String result = "";
	_runningRequestsMutex.lock();
	if (_downloadFolderRequest)
		result = _downloadFolderRequest->getLocalPath();
	_runningRequestsMutex.unlock();
	return result;
}

void Storage::directoryDownloadedCallback(FileArrayResponse response) {
	_runningRequestsMutex.lock();
	_downloadFolderRequest = nullptr;
	_runningRequestsMutex.unlock();

	Common::U32String message;
	if (response.value.size()) {
		message = Common::U32String::format(_("Download complete.\nFailed to download %u files."), response.value.size());
	} else {
		message = _("Download complete.");
	}
	Common::OSDMessageQueue::instance().addMessage(message);
}

void Storage::directoryDownloadedErrorCallback(Networking::ErrorResponse error) {
	_runningRequestsMutex.lock();
	_downloadFolderRequest = nullptr;
	_runningRequestsMutex.unlock();

	Common::OSDMessageQueue::instance().addMessage(_("Download failed."));
}

} // End of namespace Cloud
