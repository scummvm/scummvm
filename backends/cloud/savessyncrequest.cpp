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
#include "backends/cloud/cloudmanager.h"
#include "backends/networking/curl/curljsonrequest.h"
#include "backends/saves/default/default-saves.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/json.h"
#include "common/savefile.h"
#include "common/system.h"
#include "gui/saveload-dialog.h"

namespace Cloud {

SavesSyncRequest::SavesSyncRequest(Storage *storage, Storage::BoolCallback callback, Networking::ErrorCallback ecb):
	Request(nullptr, ecb), CommandSender(nullptr), _storage(storage), _boolCallback(callback),
	_workingRequest(nullptr), _ignoreCallback(false) {
	start();
}

SavesSyncRequest::~SavesSyncRequest() {
	_ignoreCallback = true;
	if (_workingRequest)
		_workingRequest->finish();
	delete _boolCallback;
}

void SavesSyncRequest::start() {
	//cleanup
	_ignoreCallback = true;
	if (_workingRequest)
		_workingRequest->finish();
	_currentDownloadingFile = StorageFile();
	_currentUploadingFile = "";
	_filesToDownload.clear();
	_filesToUpload.clear();
	_localFilesTimestamps.clear();
	_totalFilesToHandle = 0;
	_ignoreCallback = false;

	//load timestamps
	_localFilesTimestamps = DefaultSaveFileManager::loadTimestamps();

	//list saves directory
	Common::String dir = _storage->savesDirectoryPath();
	if (dir.lastChar() == '/')
		dir.deleteLastChar();
	_workingRequest = _storage->listDirectory(
		dir,
		new Common::Callback<SavesSyncRequest, Storage::ListDirectoryResponse>(this, &SavesSyncRequest::directoryListedCallback),
		new Common::Callback<SavesSyncRequest, Networking::ErrorResponse>(this, &SavesSyncRequest::directoryListedErrorCallback)
	);
	if (!_workingRequest) finishError(Networking::ErrorResponse(this, "SavesSyncRequest::start: Storage couldn't create Request to list directory"));
}

void SavesSyncRequest::directoryListedCallback(Storage::ListDirectoryResponse response) {
	_workingRequest = nullptr;
	if (_ignoreCallback)
		return;

	if (response.request) _date = response.request->date();

	Common::HashMap<Common::String, bool> localFileNotAvailableInCloud;
	for (Common::HashMap<Common::String, uint32>::iterator i = _localFilesTimestamps.begin(); i != _localFilesTimestamps.end(); ++i) {
		localFileNotAvailableInCloud[i->_key] = true;
	}

	//determine which files to download and which files to upload
	Common::Array<StorageFile> &remoteFiles = response.value;
	uint64 totalSize = 0;
	debug(9, "SavesSyncRequest decisions:");
	for (uint32 i = 0; i < remoteFiles.size(); ++i) {
		StorageFile &file = remoteFiles[i];
		if (file.isDirectory())
			continue;
		totalSize += file.size();
		if (file.name() == DefaultSaveFileManager::TIMESTAMPS_FILENAME || !CloudMan.canSyncFilename(file.name()))
			continue;

		Common::String name = file.name();
		if (!_localFilesTimestamps.contains(name)) {
			_filesToDownload.push_back(file);
			debug(9, "- downloading file %s, because it is not present on local", name.c_str());
		} else {
			localFileNotAvailableInCloud[name] = false;

			if (_localFilesTimestamps[name] == file.timestamp())
				continue;

			//we actually can have some files not only with timestamp < remote
			//but also with timestamp > remote (when we have been using ANOTHER CLOUD and then switched back)
			if (_localFilesTimestamps[name] > file.timestamp() || _localFilesTimestamps[name] == DefaultSaveFileManager::INVALID_TIMESTAMP)
				_filesToUpload.push_back(file.name());
			else
				_filesToDownload.push_back(file);

			if (_localFilesTimestamps[name] == DefaultSaveFileManager::INVALID_TIMESTAMP)
				debug(9, "- uploading file %s, because it is has invalid timestamp", name.c_str());
			else if (_localFilesTimestamps[name] > file.timestamp())
				debug(9, "- uploading file %s, because it is %d seconds newer than remote\n\tlocal = %d; \tremote = %d", name.c_str(), _localFilesTimestamps[name] - file.timestamp(), _localFilesTimestamps[name], file.timestamp());
			else
				debug(9, "- downloading file %s, because it is %d seconds older than remote\n\tlocal = %d; \tremote = %d", name.c_str(), file.timestamp() - _localFilesTimestamps[name], _localFilesTimestamps[name], file.timestamp());
		}
	}

	CloudMan.setStorageUsedSpace(CloudMan.getStorageIndex(), totalSize);

	//upload files which are unavailable in cloud
	for (Common::HashMap<Common::String, bool>::iterator i = localFileNotAvailableInCloud.begin(); i != localFileNotAvailableInCloud.end(); ++i) {
		if (i->_key == DefaultSaveFileManager::TIMESTAMPS_FILENAME || !CloudMan.canSyncFilename(i->_key))
			continue;
		if (i->_value) {
			_filesToUpload.push_back(i->_key);
			debug(9, "- uploading file %s, because it is not present on remote", i->_key.c_str());
		}
	}

	debug(9, "\nSavesSyncRequest: ");
	if (_filesToDownload.size() > 0) {
		debug(9, "download files:");
		for (uint32 i = 0; i < _filesToDownload.size(); ++i) {
			debug(9, " %s", _filesToDownload[i].name().c_str());
		}
		debug(9, "%s", "");
	} else {
		debug(9, "nothing to download");
	}
	debug(9, "SavesSyncRequest: ");
	if (_filesToUpload.size() > 0) {
		debug(9, "upload files:");
		for (uint32 i = 0; i < _filesToUpload.size(); ++i) {
			debug(9, " %s", _filesToUpload[i].c_str());
		}
	} else {
		debug(9, "nothing to upload");
	}
	_totalFilesToHandle = _filesToDownload.size() + _filesToUpload.size();

	//start downloading files
	if (!_filesToDownload.empty()) {
		downloadNextFile();
	} else {
		uploadNextFile();
	}
}

void SavesSyncRequest::directoryListedErrorCallback(Networking::ErrorResponse error) {
	_workingRequest = nullptr;
	if (_ignoreCallback)
		return;

	if (error.failed) debug(9, "%s", error.response.c_str());

	bool irrecoverable = error.interrupted || error.failed;
	if (error.failed) {
		Common::JSONValue *value = Common::JSON::parse(error.response.c_str());

		// somehow OneDrive returns JSON with '.' in unexpected places, try fixing it
		if (!value) {
			Common::String fixedResponse = error.response;
			for (uint32 i = 0; i < fixedResponse.size(); ++i) {
				if (fixedResponse[i] == '.')
					fixedResponse.replace(i, 1, " ");
			}
			value = Common::JSON::parse(fixedResponse.c_str());
		}

		if (value) {
			if (value->isObject()) {
				Common::JSONObject object = value->asObject();

				//Dropbox-related error:
				if (object.contains("error_summary") && object.getVal("error_summary")->isString()) {
					Common::String summary = object.getVal("error_summary")->asString();
					if (summary.contains("not_found")) {
						irrecoverable = false;
					}
				}

				//OneDrive-related error:
				if (object.contains("error") && object.getVal("error")->isObject()) {
					Common::JSONObject errorNode = object.getVal("error")->asObject();
					if (Networking::CurlJsonRequest::jsonContainsString(errorNode, "code", "SavesSyncRequest")) {
						Common::String code = errorNode.getVal("code")->asString();
						if (code == "itemNotFound") {
							irrecoverable = false;
						}
					}
				}
			}
			delete value;
		}

		//Google Drive, Box and OneDrive-related ScummVM-based error
		if (error.response.contains("subdirectory not found")) {
			irrecoverable = false; //base "/ScummVM/" folder not found
		} else if (error.response.contains("no such file found in its parent directory")) {
			irrecoverable = false; //"Saves" folder within "/ScummVM/" not found
		} else if (error.response.contains("itemNotFound") && error.response.contains("Item does not exist")) {
			irrecoverable = false; //"saves" folder within application folder is not found
		}
	}

	if (irrecoverable) {
		finishError(error);
		return;
	}

	//we're lucky - user just lacks his "/cloud/" folder - let's create one
	Common::String dir = _storage->savesDirectoryPath();
	if (dir.lastChar() == '/')
		dir.deleteLastChar();
	debug(9, "\nSavesSyncRequest: creating %s", dir.c_str());
	_workingRequest = _storage->createDirectory(
		dir,
		new Common::Callback<SavesSyncRequest, Storage::BoolResponse>(this, &SavesSyncRequest::directoryCreatedCallback),
		new Common::Callback<SavesSyncRequest, Networking::ErrorResponse>(this, &SavesSyncRequest::directoryCreatedErrorCallback)
	);
	if (!_workingRequest)
		finishError(Networking::ErrorResponse(this, "SavesSyncRequest::directoryListedErrorCallback: Storage couldn't create Request to create remote directory"));
}

void SavesSyncRequest::directoryCreatedCallback(Storage::BoolResponse response) {
	_workingRequest = nullptr;
	if (_ignoreCallback)
		return;

	//stop syncing if failed to create saves directory
	if (!response.value) {
		finishError(Networking::ErrorResponse(this, false, true, "SavesSyncRequest::directoryCreatedCallback: failed to create remote directory", -1));
		return;
	}

	//continue with empty files list
	Common::Array<StorageFile> files;
	directoryListedCallback(Storage::ListDirectoryResponse(response.request, files));
}

void SavesSyncRequest::directoryCreatedErrorCallback(Networking::ErrorResponse error) {
	_workingRequest = nullptr;
	if (_ignoreCallback)
		return;

	//stop syncing if failed to create saves directory
	finishError(error);
}

void SavesSyncRequest::downloadNextFile() {
	if (_filesToDownload.empty()) {
		_currentDownloadingFile = StorageFile("", 0, 0, false); //so getFilesToDownload() would return an empty array
		sendCommand(GUI::kSavesSyncEndedCmd, 0);
		uploadNextFile();
		return;
	}

	_currentDownloadingFile = _filesToDownload.back();
	_filesToDownload.pop_back();

	sendCommand(GUI::kSavesSyncProgressCmd, (int)(getDownloadingProgress() * 100));

	debug(9, "\nSavesSyncRequest: downloading %s (%d %%)", _currentDownloadingFile.name().c_str(), (int)(getProgress() * 100));
	_workingRequest = _storage->downloadById(
		_currentDownloadingFile.id(),
		DefaultSaveFileManager::concatWithSavesPath(_currentDownloadingFile.name()),
		new Common::Callback<SavesSyncRequest, Storage::BoolResponse>(this, &SavesSyncRequest::fileDownloadedCallback),
		new Common::Callback<SavesSyncRequest, Networking::ErrorResponse>(this, &SavesSyncRequest::fileDownloadedErrorCallback)
	);
	if (!_workingRequest)
		finishError(Networking::ErrorResponse(this, "SavesSyncRequest::downloadNextFile: Storage couldn't create Request to download a file"));
}

void SavesSyncRequest::fileDownloadedCallback(Storage::BoolResponse response) {
	_workingRequest = nullptr;
	if (_ignoreCallback)
		return;

	//stop syncing if download failed
	if (!response.value) {
		//delete the incomplete file
		g_system->getSavefileManager()->removeSavefile(_currentDownloadingFile.name());
		finishError(Networking::ErrorResponse(this, false, true, "SavesSyncRequest::fileDownloadedCallback: failed to download a file", -1));
		return;
	}

	//update local timestamp for downloaded file
	_localFilesTimestamps = DefaultSaveFileManager::loadTimestamps();
	_localFilesTimestamps[_currentDownloadingFile.name()] = _currentDownloadingFile.timestamp();
	DefaultSaveFileManager::saveTimestamps(_localFilesTimestamps);

	//continue downloading files
	downloadNextFile();
}

void SavesSyncRequest::fileDownloadedErrorCallback(Networking::ErrorResponse error) {
	_workingRequest = nullptr;
	if (_ignoreCallback)
		return;

	//stop syncing if download failed
	finishError(error);
}

void SavesSyncRequest::uploadNextFile() {
	if (_filesToUpload.empty()) {
		finishSync(true);
		return;
	}

	_currentUploadingFile = _filesToUpload.back();
	_filesToUpload.pop_back();

	debug(9, "\nSavesSyncRequest: uploading %s (%d %%)", _currentUploadingFile.c_str(), (int)(getProgress() * 100));
	if (_storage->uploadStreamSupported()) {
		_workingRequest = _storage->upload(
			_storage->savesDirectoryPath() + _currentUploadingFile,
			g_system->getSavefileManager()->openRawFile(_currentUploadingFile),
			new Common::Callback<SavesSyncRequest, Storage::UploadResponse>(this, &SavesSyncRequest::fileUploadedCallback),
			new Common::Callback<SavesSyncRequest, Networking::ErrorResponse>(this, &SavesSyncRequest::fileUploadedErrorCallback)
		);
	} else {
		_workingRequest = _storage->upload(
			_storage->savesDirectoryPath() + _currentUploadingFile,
			DefaultSaveFileManager::concatWithSavesPath(_currentUploadingFile),
			new Common::Callback<SavesSyncRequest, Storage::UploadResponse>(this, &SavesSyncRequest::fileUploadedCallback),
			new Common::Callback<SavesSyncRequest, Networking::ErrorResponse>(this, &SavesSyncRequest::fileUploadedErrorCallback)
		);
	}
	if (!_workingRequest) finishError(Networking::ErrorResponse(this, "SavesSyncRequest::uploadNextFile: Storage couldn't create Request to upload a file"));
}

void SavesSyncRequest::fileUploadedCallback(Storage::UploadResponse response) {
	_workingRequest = nullptr;
	if (_ignoreCallback)
		return;

	//update local timestamp for the uploaded file
	_localFilesTimestamps = DefaultSaveFileManager::loadTimestamps();
	_localFilesTimestamps[_currentUploadingFile] = response.value.timestamp();
	DefaultSaveFileManager::saveTimestamps(_localFilesTimestamps);

	//continue uploading files
	uploadNextFile();
}

void SavesSyncRequest::fileUploadedErrorCallback(Networking::ErrorResponse error) {
	_workingRequest = nullptr;
	if (_ignoreCallback)
		return;

	//stop syncing if upload failed
	finishError(error);
}

void SavesSyncRequest::handle() {}

void SavesSyncRequest::restart() { start(); }

double SavesSyncRequest::getDownloadingProgress() const {
	if (_totalFilesToHandle == 0) {
		if (_state == Networking::FINISHED)
			return 1; //nothing to upload and download => Request ends soon
		return 0; //directory not listed yet
	}

	if (_totalFilesToHandle == _filesToUpload.size())
		return 1; //nothing to download => download complete

	uint32 totalFilesToDownload = _totalFilesToHandle - _filesToUpload.size();
	uint32 filesLeftToDownload = _filesToDownload.size() + (_currentDownloadingFile.name() != "" ? 1 : 0);
	return (double)(totalFilesToDownload - filesLeftToDownload) / (double)(totalFilesToDownload);
}

double SavesSyncRequest::getProgress() const {
	if (_totalFilesToHandle == 0) {
		if (_state == Networking::FINISHED)
			return 1; //nothing to upload and download => Request ends soon
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
	debug(9, "SavesSync::finishError");
	//if we were downloading a file - remember the name
	//and make the Request close() it, so we can delete it
	Common::String name = _currentDownloadingFile.name();
	if (_workingRequest) {
		_ignoreCallback = true;
		_workingRequest->finish();
		_workingRequest = nullptr;
		_ignoreCallback = false;
	}
	//unlock all the files by making getFilesToDownload() return empty array
	_currentDownloadingFile = StorageFile();
	_filesToDownload.clear();
	//delete the incomplete file
	if (name != "")
		g_system->getSavefileManager()->removeSavefile(name);
	Request::finishError(error);
}

void SavesSyncRequest::finishSync(bool success) {
	Request::finishSuccess();

	//update last successful sync date
	CloudMan.setStorageLastSync(CloudMan.getStorageIndex(), _date);

	if (_boolCallback)
		(*_boolCallback)(Storage::BoolResponse(this, success));
}

} // End of namespace Cloud
