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
#include "common/debug.h"

namespace Cloud {

FolderDownloadRequest::FolderDownloadRequest(Storage *storage, Storage::FileArrayCallback callback, Common::String remoteDirectoryPath, Common::String localDirectoryPath, bool recursive):
	Request(nullptr), _storage(storage), _fileArrayCallback(callback),
	_remoteDirectoryPath(remoteDirectoryPath), _localDirectoryPath(localDirectoryPath), _recursive(recursive),
	_workingRequest(nullptr), _ignoreCallback(false) {
	start();
}

FolderDownloadRequest::~FolderDownloadRequest() {
	_ignoreCallback = true;
	if (_workingRequest) _workingRequest->finish();
	delete _fileArrayCallback;
}


void FolderDownloadRequest::start() {
	//cleanup
	_ignoreCallback = true;
	if (_workingRequest) _workingRequest->finish();	
	_currentFile = StorageFile();
	_files.clear();
	_failedFiles.clear();
	_ignoreCallback = false;

	//list directory first
	_workingRequest = _storage->listDirectory(
		_remoteDirectoryPath,
		new Common::Callback<FolderDownloadRequest, Storage::FileArrayResponse>(this, &FolderDownloadRequest::directoryListedCallback),
		_recursive
	);
}

void FolderDownloadRequest::directoryListedCallback(Storage::FileArrayResponse pair) {
	if (_ignoreCallback) return;
	//TODO: somehow ListDirectory requests must indicate that file array is incomplete
	_files = pair.value;
	downloadNextFile();
}

void FolderDownloadRequest::fileDownloadedCallback(Storage::BoolResponse pair) {
	if (_ignoreCallback) return;
	if (!pair.value) _failedFiles.push_back(_currentFile);
	downloadNextFile();
}

void FolderDownloadRequest::downloadNextFile() {
	do {
		if (_files.empty()) {
			finishFiles(_failedFiles);
			return;
		}
	
		_currentFile = _files.back();
		_files.pop_back();
	} while (_currentFile.isDirectory()); //TODO: may be create these directories (in case those are empty)

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
	_workingRequest = _storage->download(
		remotePath, localPath,
		new Common::Callback<FolderDownloadRequest, Storage::BoolResponse>(this, &FolderDownloadRequest::fileDownloadedCallback)		
	);
}

void FolderDownloadRequest::finish() {
	//TODO: somehow indicate that request was interrupted
	Common::Array<StorageFile> files;
	finishFiles(files);
}

void FolderDownloadRequest::finishFiles(Common::Array<StorageFile> &files) {
	Request::finish();
	if (_fileArrayCallback) (*_fileArrayCallback)(Storage::FileArrayResponse(this, files));
}

} // End of namespace Cloud
