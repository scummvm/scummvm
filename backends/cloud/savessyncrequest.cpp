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

#include "backends/cloud/savessyncrequest.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/system.h"
#include "common/savefile.h"

namespace Cloud {

SavesSyncRequest::SavesSyncRequest(Storage *storage, Storage::BoolCallback callback):
	Request(nullptr), _storage(storage), _boolCallback(callback),
	_workingRequest(nullptr), _ignoreCallback(false) {
	start();
}

SavesSyncRequest::~SavesSyncRequest() {
	_ignoreCallback = true;
	if (_workingRequest) _workingRequest->finish();
	delete _boolCallback;
}

void SavesSyncRequest::start() {
	//cleanup
	_ignoreCallback = true;
	if (_workingRequest) _workingRequest->finish();	
	_currentDownloadingFile = StorageFile();
	_currentUploadingFile = "";
	_filesToDownload.clear();
	_filesToUpload.clear();
	_localFilesTimestamps.clear();
	_ignoreCallback = false;

	//load timestamps
	loadTimestamps();

	//list saves directory
	_workingRequest = _storage->listDirectory("saves", new Common::Callback<SavesSyncRequest, Storage::ListDirectoryResponse>(this, &SavesSyncRequest::directoryListedCallback));
}

void SavesSyncRequest::directoryListedCallback(Storage::ListDirectoryResponse pair) {
	if (_ignoreCallback) return;

	ListDirectoryStatus status = pair.value;
	if (status.interrupted || status.failed) {
		finishBool(false);
		return;
	}

	const uint32 INVALID_TIMESTAMP = UINT_MAX;
	
	//determine which files to download and which files to upload
	Common::Array<StorageFile> &remoteFiles = status.files;
	for (uint32 i = 0; i < remoteFiles.size(); ++i) {
		StorageFile &file = remoteFiles[i];
		if (file.isDirectory()) continue;
		Common::String name = file.name();
		if (!_localFilesTimestamps.contains(name))
			_filesToDownload.push_back(file);
		else {
			if (_localFilesTimestamps[name] != INVALID_TIMESTAMP) {
				if (_localFilesTimestamps[name] == file.timestamp())
					continue;

				//we actually can have some files not only with timestamp < remote
				//but also with timestamp > remote (when we have been using ANOTHER CLOUD and then switched back)
				if (_localFilesTimestamps[name] < file.timestamp())
					_filesToDownload.push_back(file);
				else
					_filesToUpload.push_back(file.name());
			}
		}
	}

	//TODO: upload files which are added to local directory (not available on cloud), but have no timestamp

	//upload files with invalid timestamp (the ones we've added - means they might not have any remote version)
	for (Common::HashMap<Common::String, uint32>::iterator i = _localFilesTimestamps.begin(); i != _localFilesTimestamps.end(); ++i) {
		if (i->_value == INVALID_TIMESTAMP)
			_filesToUpload.push_back(i->_key);
	}

	//start downloading files
	downloadNextFile();
}

void SavesSyncRequest::downloadNextFile() {
	if (_filesToDownload.empty()) {
		uploadNextFile();
		return;
	}

	_currentDownloadingFile = _filesToDownload.back();
	_filesToDownload.pop_back();

	_workingRequest = _storage->download(_currentDownloadingFile.path(), "saves/" + _currentDownloadingFile.name(), //TODO: real saves folder here
		new Common::Callback<SavesSyncRequest, Storage::BoolResponse>(this, &SavesSyncRequest::fileDownloadedCallback)
	);
}

void SavesSyncRequest::fileDownloadedCallback(Storage::BoolResponse pair) {
	if (_ignoreCallback) return;

	//stop syncing if download failed
	if (!pair.value) {
		finish();
		return;
	}

	//update local timestamp for downloaded file
	_localFilesTimestamps[_currentDownloadingFile.name()] = _currentDownloadingFile.timestamp();

	//continue downloading files
	downloadNextFile();
}

void SavesSyncRequest::uploadNextFile() {
	if (_filesToUpload.empty()) {
		finishBool(true);
		return;
	}

	_currentUploadingFile = _filesToUpload.back();
	_filesToUpload.pop_back();
	
	_workingRequest = _storage->upload("saves/" + _currentUploadingFile, g_system->getSavefileManager()->openForLoading(_currentUploadingFile),
		new Common::Callback<SavesSyncRequest, Storage::UploadResponse>(this, &SavesSyncRequest::fileUploadedCallback)
	);
}

void SavesSyncRequest::fileUploadedCallback(Storage::UploadResponse pair) {
	if (_ignoreCallback) return;
	UploadStatus status = pair.value;

	//stop syncing if upload failed
	if (status.interrupted || status.failed) {
		finish();
		return;
	}

	//update local timestamp for the uploaded file
	_localFilesTimestamps[_currentUploadingFile] = status.file.timestamp();

	//continue uploading files
	uploadNextFile();
}

void SavesSyncRequest::handle() {}

void SavesSyncRequest::restart() { start(); }

void SavesSyncRequest::finish() { finishBool(false); }

void SavesSyncRequest::finishBool(bool success) {
	Request::finish();

	//save updated timestamps (even if Request failed, there would be only valid timestamps)
	saveTimestamps();

	if (_boolCallback) (*_boolCallback)(Storage::BoolResponse(this, success));
}

void SavesSyncRequest::loadTimestamps() {
	Common::File f;
	//TODO: real saves folder here
	if (!f.open("saves/timestamps"))
		error("SavesSyncRequest: failed to open 'saves/timestamps' file to load timestamps");
	
	while (!f.eos()) {
		//read filename into buffer (reading until the first ' ')
		Common::String buffer;
		while (!f.eos()) {
			byte b = f.readByte();
			if (b == ' ') break;
			buffer += (char)b;
		}
		
		//read timestamp info buffer (reading until ' ' or some line ending char)
		Common::String filename = buffer;
		bool lineEnded = false;
		buffer = "";
		while (!f.eos()) {
			byte b = f.readByte();
			if (b == ' ' || b == '\n' || b == '\r') {
				lineEnded = (b == '\n');
				break;
			}
			buffer += (char)b;
		}

		//parse timestamp
		uint timestamp = atol(buffer.c_str());
		_localFilesTimestamps[filename] = timestamp;

		//read until the end of the line
		if (!lineEnded) {
			while (!f.eos()) {
				byte b = f.readByte();
				if (b == '\n') break;
			}
		}
	}

	f.close();
}

void SavesSyncRequest::saveTimestamps() {
	Common::DumpFile f;
	//TODO: real saves folder here
	if (!f.open("saves/timestamps", true))
		error("SavesSyncRequest: failed to open 'saves/timestamps' file to save timestamps");
	Common::String data;
	for (Common::HashMap<Common::String, uint32>::iterator i = _localFilesTimestamps.begin(); i != _localFilesTimestamps.end(); ++i)
		data += i->_key + Common::String::format(" %u\n", i->_value);
	if (f.write(data.c_str(), data.size()) != data.size())
		error("SavesSyncRequest: failed to write timestamps data into 'saves/timestamps'");
	f.close();
}


} // End of namespace Cloud
