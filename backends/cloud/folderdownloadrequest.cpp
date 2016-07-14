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

#include "backends/cloud/folderdownloadrequest.h"
#include "backends/cloud/downloadrequest.h"
#include "backends/cloud/id/iddownloadrequest.h"
#include "common/debug.h"
#include "gui/downloaddialog.h"

namespace Cloud {

FolderDownloadRequest::FolderDownloadRequest(Storage *storage, Storage::FileArrayCallback callback, Networking::ErrorCallback ecb, Common::String remoteDirectoryPath, Common::String localDirectoryPath, bool recursive):
	Request(nullptr, ecb), CommandSender(nullptr), _storage(storage), _fileArrayCallback(callback),
	_remoteDirectoryPath(remoteDirectoryPath), _localDirectoryPath(localDirectoryPath), _recursive(recursive),
	_workingRequest(nullptr), _ignoreCallback(false), _totalFiles(0) {
	start();
}

FolderDownloadRequest::~FolderDownloadRequest() {
	sendCommand(GUI::kDownloadEndedCmd, 0);
	_ignoreCallback = true;
	if (_workingRequest) _workingRequest->finish();
	delete _fileArrayCallback;
}

void FolderDownloadRequest::start() {
	//cleanup
	_ignoreCallback = true;
	if (_workingRequest) _workingRequest->finish();	
	_currentFile = StorageFile();
	_pendingFiles.clear();
	_failedFiles.clear();
	_ignoreCallback = false;
	_totalFiles = 0;

	//list directory first
	_workingRequest = _storage->listDirectory(
		_remoteDirectoryPath,
		new Common::Callback<FolderDownloadRequest, Storage::ListDirectoryResponse>(this, &FolderDownloadRequest::directoryListedCallback),
		new Common::Callback<FolderDownloadRequest, Networking::ErrorResponse>(this, &FolderDownloadRequest::directoryListedErrorCallback),
		_recursive
	);
}

void FolderDownloadRequest::directoryListedCallback(Storage::ListDirectoryResponse response) {
	_workingRequest = nullptr;
	if (_ignoreCallback) return;
	_pendingFiles = response.value;

	// remove all directories
	for (Common::Array<StorageFile>::iterator i = _pendingFiles.begin(); i != _pendingFiles.end(); )
		if (i->isDirectory())
			_pendingFiles.erase(i);
		else
			++i;

	_totalFiles = _pendingFiles.size();
	downloadNextFile();
}

void FolderDownloadRequest::directoryListedErrorCallback(Networking::ErrorResponse error) {
	_workingRequest = nullptr;
	if (_ignoreCallback) return;
	finishError(error);
}

void FolderDownloadRequest::fileDownloadedCallback(Storage::BoolResponse response) {
	_workingRequest = nullptr;
	if (_ignoreCallback) return;
	if (!response.value) _failedFiles.push_back(_currentFile);
	downloadNextFile();
}

void FolderDownloadRequest::fileDownloadedErrorCallback(Networking::ErrorResponse error) {
	_workingRequest = nullptr;
	if (_ignoreCallback) return;
	fileDownloadedCallback(Storage::BoolResponse(error.request, false));
}

void FolderDownloadRequest::downloadNextFile() {
	do {
		if (_pendingFiles.empty()) {
			sendCommand(GUI::kDownloadEndedCmd, 0);
			finishDownload(_failedFiles);
			return;
		}
	
		_currentFile = _pendingFiles.back();
		_pendingFiles.pop_back();
	} while (_currentFile.isDirectory()); //TODO: may be create these directories (in case those are empty)

	sendCommand(GUI::kDownloadProgressCmd, (int)(getProgress() * 100));

	Common::String remotePath = _currentFile.path();
	Common::String localPath = remotePath;
	if (_remoteDirectoryPath == "" || remotePath.hasPrefix(_remoteDirectoryPath)) {
		localPath.erase(0, _remoteDirectoryPath.size());
		if (_remoteDirectoryPath != "" && (_remoteDirectoryPath.lastChar() != '/' && _remoteDirectoryPath.lastChar() != '\\'))
			localPath.erase(0, 1);
	} else {
		warning("Can't process the following paths:");
		warning("remote directory: %s", _remoteDirectoryPath.c_str());
		warning("remote file under that directory: %s", remotePath.c_str());		
	}
	if (_localDirectoryPath != "") {
		if (_localDirectoryPath.lastChar() == '/' || _localDirectoryPath.lastChar() == '\\')
			localPath = _localDirectoryPath + localPath;
		else
			localPath = _localDirectoryPath + "/" + localPath;
	}
	debug("%s -> %s", remotePath.c_str(), localPath.c_str());
	_workingRequest = _storage->downloadById(
		_currentFile.id(), localPath,
		new Common::Callback<FolderDownloadRequest, Storage::BoolResponse>(this, &FolderDownloadRequest::fileDownloadedCallback),
		new Common::Callback<FolderDownloadRequest, Networking::ErrorResponse>(this, &FolderDownloadRequest::fileDownloadedErrorCallback)
	);
}

void FolderDownloadRequest::handle() {}

void FolderDownloadRequest::restart() { start(); }

void FolderDownloadRequest::finishDownload(Common::Array<StorageFile> &files) {
	Request::finishSuccess();
	if (_fileArrayCallback) (*_fileArrayCallback)(Storage::FileArrayResponse(this, files));
}

double FolderDownloadRequest::getProgress() const {
	if (_totalFiles == 0) return 0;

	double currentFileProgress = 0;
	DownloadRequest *downloadRequest = dynamic_cast<DownloadRequest *>(_workingRequest);
	if (downloadRequest != nullptr) currentFileProgress = downloadRequest->getProgress();
	else {
		Id::IdDownloadRequest *idDownloadRequest = dynamic_cast<Id::IdDownloadRequest *>(_workingRequest);
		if (idDownloadRequest != nullptr) currentFileProgress = idDownloadRequest->getProgress();
	}

	uint32 uploadedFiles = _totalFiles - _pendingFiles.size() - 1; // -1 because currently downloaded file is already removed from _pendingFiles
	return (double)(uploadedFiles + currentFileProgress) / (double)(_totalFiles);
}

} // End of namespace Cloud
