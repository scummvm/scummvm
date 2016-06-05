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
#include "common/json.h"
#include "common/savefile.h"
#include "common/system.h"

namespace Cloud {

const char *SavesSyncRequest::TIMESTAMPS_FILENAME = "timestamps";

enum {
	kSavesSyncProgressCmd = 'SSPR',
	kSavesSyncEndedCmd = 'SSEN'
};

SavesSyncRequest::SavesSyncRequest(Storage *storage, Storage::BoolCallback callback, Networking::ErrorCallback ecb):
	Request(nullptr, ecb), CommandSender(nullptr), _storage(storage), _boolCallback(callback),
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
	_totalFilesToHandle = 0;
	_ignoreCallback = false;

	//load timestamps
	loadTimestamps();

	//list saves directory
	Common::String dir = _storage->savesDirectoryPath();
	if (dir.lastChar() == '/') dir.deleteLastChar();
	_workingRequest = _storage->listDirectory(
		dir,
		new Common::Callback<SavesSyncRequest, Storage::ListDirectoryResponse>(this, &SavesSyncRequest::directoryListedCallback),
		new Common::Callback<SavesSyncRequest, Networking::ErrorResponse>(this, &SavesSyncRequest::directoryListedErrorCallback)
	);
	if (!_workingRequest) finishError(Networking::ErrorResponse(this));
}

void SavesSyncRequest::directoryListedCallback(Storage::ListDirectoryResponse response) {
	_workingRequest = nullptr;
	if (_ignoreCallback) return;

	Common::HashMap<Common::String, bool> localFileNotAvailableInCloud;
	for (Common::HashMap<Common::String, uint32>::iterator i = _localFilesTimestamps.begin(); i != _localFilesTimestamps.end(); ++i) {		
		localFileNotAvailableInCloud[i->_key] = true;
	}

	//determine which files to download and which files to upload
	Common::Array<StorageFile> &remoteFiles = response.value;
	for (uint32 i = 0; i < remoteFiles.size(); ++i) {
		StorageFile &file = remoteFiles[i];
		if (file.isDirectory()) continue;
		if (file.name() == TIMESTAMPS_FILENAME) continue;

		Common::String name = file.name();
		if (!_localFilesTimestamps.contains(name))
			_filesToDownload.push_back(file);
		else {
			localFileNotAvailableInCloud[name] = false;

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

	//upload files which are unavailable in cloud
	for (Common::HashMap<Common::String, bool>::iterator i = localFileNotAvailableInCloud.begin(); i != localFileNotAvailableInCloud.end(); ++i) {
		if (i->_key == TIMESTAMPS_FILENAME) continue;
		if (i->_value) _filesToUpload.push_back(i->_key);
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
	_totalFilesToHandle = _filesToDownload.size() + _filesToUpload.size();
	///////

	//start downloading files
	downloadNextFile();
}

void SavesSyncRequest::directoryListedErrorCallback(Networking::ErrorResponse error) {
	_workingRequest = nullptr;
	if (_ignoreCallback) return;

	bool irrecoverable = error.interrupted || error.failed;
	if (error.failed) {
		Common::JSONValue *value = Common::JSON::parse(error.response.c_str());
		if (value) {
			if (value->isObject()) {
				Common::JSONObject object = value->asObject();

				//Dropbox-related error:
				if (object.contains("error_summary")) {
					Common::String summary = object.getVal("error_summary")->asString();
					if (summary.contains("not_found")) {						
						irrecoverable = false;
					}
				}

				//OneDrive-related error:
				if (object.contains("error") && object.getVal("error")->isObject()) {
					Common::JSONObject errorNode = object.getVal("error")->asObject();
					if (errorNode.contains("code") && errorNode.contains("message")) {
						Common::String code = errorNode.getVal("code")->asString();
						if (code == "itemNotFound") {
							irrecoverable = false;
						}
					}
				}
			}
			delete value;
		}
	}

	if (irrecoverable) {
		finishError(error);
		return;
	}

	//we're lucky - user just lacks his "/cloud/" folder - let's create one
	Common::String dir = _storage->savesDirectoryPath();
	if (dir.lastChar() == '/') dir.deleteLastChar();
	debug("creating %s", dir.c_str());
	_workingRequest = _storage->createDirectory(dir,
		new Common::Callback<SavesSyncRequest, Storage::BoolResponse>(this, &SavesSyncRequest::directoryCreatedCallback),
		new Common::Callback<SavesSyncRequest, Networking::ErrorResponse>(this, &SavesSyncRequest::directoryCreatedErrorCallback)
	);
	if (!_workingRequest) finishError(Networking::ErrorResponse(this));
}

void SavesSyncRequest::directoryCreatedCallback(Storage::BoolResponse response) {
	_workingRequest = nullptr;
	if (_ignoreCallback) return;

	//stop syncing if failed to create saves directory
	if (!response.value) {
		finishError(Networking::ErrorResponse(this, false, true, "", -1));
		return;
	}

	//continue with empty files list
	Common::Array<StorageFile> files;
	directoryListedCallback(Storage::ListDirectoryResponse(response.request, files));
}

void SavesSyncRequest::directoryCreatedErrorCallback(Networking::ErrorResponse error) {
	_workingRequest = nullptr;
	if (_ignoreCallback) return;

	//stop syncing if failed to create saves directory
	finishError(error);
}

void SavesSyncRequest::downloadNextFile() {
	if (_filesToDownload.empty()) {
		_currentDownloadingFile = StorageFile("", 0, 0, false); //so getFilesToDownload() would return an empty array
		sendCommand(kSavesSyncEndedCmd, 0);
		uploadNextFile();
		return;
	}

	_currentDownloadingFile = _filesToDownload.back();
	_filesToDownload.pop_back();

	sendCommand(kSavesSyncProgressCmd, (int)(getDownloadingProgress() * 100));

	///////
	debug("downloading %s (%d %%)", _currentDownloadingFile.name().c_str(), (int)(getProgress() * 100));
	///////
	_workingRequest = _storage->download(_currentDownloadingFile.path(), concatWithSavesPath(_currentDownloadingFile.name()),
		new Common::Callback<SavesSyncRequest, Storage::BoolResponse>(this, &SavesSyncRequest::fileDownloadedCallback),
		new Common::Callback<SavesSyncRequest, Networking::ErrorResponse>(this, &SavesSyncRequest::fileDownloadedErrorCallback)
	);
	if (!_workingRequest) finishError(Networking::ErrorResponse(this));
}

void SavesSyncRequest::fileDownloadedCallback(Storage::BoolResponse response) {
	_workingRequest = nullptr;
	if (_ignoreCallback) return;

	//stop syncing if download failed
	if (!response.value) {
		finishError(Networking::ErrorResponse(this, false, true, "", -1));
		return;
	}

	//update local timestamp for downloaded file
	_localFilesTimestamps[_currentDownloadingFile.name()] = _currentDownloadingFile.timestamp();

	//continue downloading files
	downloadNextFile();
}

void SavesSyncRequest::fileDownloadedErrorCallback(Networking::ErrorResponse error) {
	_workingRequest = nullptr;
	if (_ignoreCallback) return;

	//stop syncing if download failed
	finishError(error);	
}

void SavesSyncRequest::uploadNextFile() {
	if (_filesToUpload.empty()) {
		finishSuccess(true);
		return;
	}

	_currentUploadingFile = _filesToUpload.back();
	_filesToUpload.pop_back();
	
	///////
	debug("uploading %s (%d %%)", _currentUploadingFile.c_str(), (int)(getProgress()*100));
	///////
	_workingRequest = _storage->upload(_storage->savesDirectoryPath() + _currentUploadingFile, g_system->getSavefileManager()->openRawFile(_currentUploadingFile),
		new Common::Callback<SavesSyncRequest, Storage::UploadResponse>(this, &SavesSyncRequest::fileUploadedCallback),
		new Common::Callback<SavesSyncRequest, Networking::ErrorResponse>(this, &SavesSyncRequest::fileUploadedErrorCallback)
	);
	if (!_workingRequest) finishError(Networking::ErrorResponse(this));
}

void SavesSyncRequest::fileUploadedCallback(Storage::UploadResponse response) {
	_workingRequest = nullptr;
	if (_ignoreCallback) return;
	
	//update local timestamp for the uploaded file
	_localFilesTimestamps[_currentUploadingFile] = response.value.timestamp();

	//continue uploading files
	uploadNextFile();
}

void SavesSyncRequest::fileUploadedErrorCallback(Networking::ErrorResponse error) {
	_workingRequest = nullptr;
	if (_ignoreCallback) return;

	//stop syncing if upload failed
	finishError(error);
}

void SavesSyncRequest::handle() {}

void SavesSyncRequest::restart() { start(); }

double SavesSyncRequest::getDownloadingProgress() {
	if (_totalFilesToHandle == 0) {
		if (_state == Networking::FINISHED) return 1; //nothing to upload and download => Request ends soon
		return 0; //directory not listed yet
	}

	if (_totalFilesToHandle == _filesToUpload.size()) return 1; //nothing to download => download complete

	uint32 totalFilesToDownload = _totalFilesToHandle - _filesToUpload.size();
	uint32 filesLeftToDownload = _filesToDownload.size() + (_currentDownloadingFile.name() != "" ? 1 : 0);
	return (double)(totalFilesToDownload - filesLeftToDownload) / (double)(totalFilesToDownload);
}

double SavesSyncRequest::getProgress() {
	if (_totalFilesToHandle == 0) {
		if (_state == Networking::FINISHED) return 1; //nothing to upload and download => Request ends soon
		return 0; //directory not listed yet
	}

	return (double)(_totalFilesToHandle - _filesToDownload.size() - _filesToUpload.size()) / (double)(_totalFilesToHandle);
}

Common::Array<Common::String> SavesSyncRequest::getFilesToDownload() {
	Common::Array<Common::String> result;
	for (uint32 i = 0; i < _filesToDownload.size(); ++i)
		result.push_back(_filesToDownload[i].name());
	if (_currentDownloadingFile.name() != "")
		result.push_back(_currentDownloadingFile.name());
	return result;
}

void SavesSyncRequest::finishError(Networking::ErrorResponse error) {
	debug("SavesSync::finishError");

	//save updated timestamps (even if Request failed, there would be only valid timestamps)
	saveTimestamps();

	Request::finishError(error);
}

void SavesSyncRequest::finishSuccess(bool success) {
	Request::finishSuccess();

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
