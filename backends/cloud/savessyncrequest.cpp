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
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/system.h"
#include <common/json.h>

namespace Cloud {

const char *SavesSyncRequest::TIMESTAMPS_FILENAME = "timestamps";

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
	_workingRequest = _storage->listDirectory("/saves", new Common::Callback<SavesSyncRequest, Storage::ListDirectoryResponse>(this, &SavesSyncRequest::directoryListedCallback));
}

void SavesSyncRequest::directoryListedCallback(Storage::ListDirectoryResponse pair) {
	_workingRequest = nullptr;
	if (_ignoreCallback) return;

	ListDirectoryStatus status = pair.value;
	bool irrecoverable = status.interrupted || status.failed;
	if (status.failed) {
		Common::JSONValue *value = Common::JSON::parse(status.response.c_str());
		if (value) {
			if (value->isObject()) {
				Common::JSONObject object = value->asObject();
				//Dropbox-related error:
				if (object.contains("error_summary")) {
					Common::String summary = object.getVal("error_summary")->asString();
					if (summary.contains("not_found")) {
						//oh how lucky we are! It's just user don't have /cloud/ folder yet!
						irrecoverable = false;
					}
				}
			}
			delete value;
		}
	}

	if (irrecoverable) {
		finishBool(false);
		return;
	}
	
	//determine which files to download and which files to upload
	Common::Array<StorageFile> &remoteFiles = status.files;
	for (uint32 i = 0; i < remoteFiles.size(); ++i) {
		StorageFile &file = remoteFiles[i];
		if (file.isDirectory()) continue;
		if (file.name() == TIMESTAMPS_FILENAME) continue;
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

	//upload files with invalid timestamp (the ones we've added - means they might not have any remote version)
	for (Common::HashMap<Common::String, uint32>::iterator i = _localFilesTimestamps.begin(); i != _localFilesTimestamps.end(); ++i) {
		if (i->_key == TIMESTAMPS_FILENAME) continue;
		if (i->_value == INVALID_TIMESTAMP)
			_filesToUpload.push_back(i->_key);
	}

	///////
	debug("\ndownload files:");
	for (uint32 i = 0; i < _filesToDownload.size(); ++i) {
		debug("%s", _filesToDownload[i].name().c_str());
	}
	debug("\nupload files:");
	for (uint32 i = 0; i < _filesToUpload.size(); ++i) {
		debug("%s", _filesToUpload[i].c_str());
	}
	///////

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

	///////
	debug("downloading %s", _currentDownloadingFile.name().c_str());
	///////
	_workingRequest = _storage->download(_currentDownloadingFile.path(), concatWithSavesPath(_currentDownloadingFile.name()),
		new Common::Callback<SavesSyncRequest, Storage::BoolResponse>(this, &SavesSyncRequest::fileDownloadedCallback)
	);
}

void SavesSyncRequest::fileDownloadedCallback(Storage::BoolResponse pair) {
	_workingRequest = nullptr;
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
	
	///////
	debug("uploading %s", _currentUploadingFile.c_str());
	///////
	_workingRequest = _storage->upload("/saves/" + _currentUploadingFile, g_system->getSavefileManager()->openRawFile(_currentUploadingFile),
		new Common::Callback<SavesSyncRequest, Storage::UploadResponse>(this, &SavesSyncRequest::fileUploadedCallback)
	);
}

void SavesSyncRequest::fileUploadedCallback(Storage::UploadResponse pair) {
	_workingRequest = nullptr;
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
	//start with listing all the files in saves/ directory and setting invalid timestamp to them
	Common::StringArray localFiles = g_system->getSavefileManager()->listSavefiles("*");
	for (uint32 i = 0; i < localFiles.size(); ++i)
		_localFilesTimestamps[localFiles[i]] = INVALID_TIMESTAMP;

	//now actually load timestamps from file
	Common::InSaveFile *file = g_system->getSavefileManager()->openRawFile(TIMESTAMPS_FILENAME);
	if (!file) {
		warning("SavesSyncRequest: failed to open '%s' file to load timestamps", TIMESTAMPS_FILENAME);
		return;
	}

	
	while (!file->eos()) {
		//read filename into buffer (reading until the first ' ')
		Common::String buffer;
		while (!file->eos()) {
			byte b = file->readByte();
			if (b == ' ') break;
			buffer += (char)b;
		}
		
		//read timestamp info buffer (reading until ' ' or some line ending char)
		Common::String filename = buffer;
		bool lineEnded = false;
		buffer = "";
		while (!file->eos()) {
			byte b = file->readByte();
			if (b == ' ' || b == '\n' || b == '\r') {
				lineEnded = (b == '\n');
				break;
			}
			buffer += (char)b;
		}

		//parse timestamp
		uint timestamp = atol(buffer.c_str());
		if (buffer == "" || timestamp == 0) break;
		_localFilesTimestamps[filename] = timestamp;

		//read until the end of the line
		if (!lineEnded) {
			while (!file->eos()) {
				byte b = file->readByte();
				if (b == '\n') break;
			}
		}
	}
	
	delete file;
}

void SavesSyncRequest::saveTimestamps() {
	Common::DumpFile f;	
	Common::String filename = concatWithSavesPath(TIMESTAMPS_FILENAME);
	if (!f.open(filename, true)) {
		warning("SavesSyncRequest: failed to open '%s' file to save timestamps", filename.c_str());
		return;
	}
	
	for (Common::HashMap<Common::String, uint32>::iterator i = _localFilesTimestamps.begin(); i != _localFilesTimestamps.end(); ++i) {
		Common::String data = i->_key + Common::String::format(" %u\n", i->_value);
		if (f.write(data.c_str(), data.size()) != data.size()) {
			warning("SavesSyncRequest: failed to write timestamps data into '%s'", filename.c_str());
			return;
		}
	}

	f.close();
}

Common::String SavesSyncRequest::concatWithSavesPath(Common::String name) {
	Common::String path = ConfMan.get("savepath");
	if (path.size() > 0 && (path.lastChar() == '/' || path.lastChar() == '\\'))
		return path + name;

	//simple heuristic to determine which path separator to use
	int backslashes = 0;
	for (uint32 i = 0; i < path.size(); ++i)
		if (path[i] == '/') --backslashes;
		else if (path[i] == '\\') ++backslashes;

	if (backslashes) return path + '\\' + name;
	return path + '/' + name;
}

} // End of namespace Cloud
