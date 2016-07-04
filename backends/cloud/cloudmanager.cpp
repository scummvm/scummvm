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

#include "backends/cloud/cloudmanager.h"
#include "backends/cloud/dropbox/dropboxstorage.h"
#include "backends/cloud/onedrive/onedrivestorage.h"
#include "backends/cloud/googledrive/googledrivestorage.h"
#include "common/debug.h"
#include "common/translation.h"
#include "common/config-manager.h"
#include "common/str.h"

namespace Common {

DECLARE_SINGLETON(Cloud::CloudManager);

}

namespace Cloud {

const char *const CloudManager::kStoragePrefix = "storage_";

CloudManager::CloudManager() : _currentStorageIndex(0), _activeStorage(nullptr) {}

CloudManager::~CloudManager() {
	//TODO: do we have to save storages on manager destruction?	
	delete _activeStorage;
}

Common::String CloudManager::getStorageConfigName(uint32 index) const {
	switch (index) {
	case kStorageNoneId: return "<none>";
	case kStorageDropboxId: return "Dropbox";
	case kStorageOneDriveId: return "OneDrive";
	case kStorageGoogleDriveId: return "GoogleDrive";
	}
	assert(false); // Unhandled StorageID value
	return "";
}

void CloudManager::loadStorage() {
	switch (_currentStorageIndex) {
	case kStorageDropboxId:
		_activeStorage = Dropbox::DropboxStorage::loadFromConfig(kStoragePrefix + getStorageConfigName(_currentStorageIndex) + "_");
		break;
	case kStorageOneDriveId:
		_activeStorage = OneDrive::OneDriveStorage::loadFromConfig(kStoragePrefix + getStorageConfigName(_currentStorageIndex) + "_");
		break;
	case kStorageGoogleDriveId:
		_activeStorage = GoogleDrive::GoogleDriveStorage::loadFromConfig(kStoragePrefix + getStorageConfigName(_currentStorageIndex) + "_");
		break;
	default:
		_activeStorage = nullptr;
	}

	if (!_activeStorage) {
		_currentStorageIndex = kStorageNoneId;
	}
}

void CloudManager::init() {
	//init configs structs
	for (uint32 i = 0; i < kStorageTotal; ++i) {
		Common::String name = getStorageConfigName(i);
		StorageConfig config;
		config.name = _(name);
		config.username = "";
		config.lastSyncDate = "";
		config.usedBytes = 0;
		if (ConfMan.hasKey(kStoragePrefix + name + "_username", ConfMan.kCloudDomain))
			config.username = ConfMan.get(kStoragePrefix + name + "_username", ConfMan.kCloudDomain);
		if (ConfMan.hasKey(kStoragePrefix + name + "_lastSync", ConfMan.kCloudDomain))
			config.lastSyncDate = ConfMan.get(kStoragePrefix + name + "_lastSync", ConfMan.kCloudDomain);
		if (ConfMan.hasKey(kStoragePrefix + name + "_usedBytes", ConfMan.kCloudDomain))
			config.usedBytes = ConfMan.get(kStoragePrefix + name + "_usedBytes", ConfMan.kCloudDomain).asUint64();
		_storages.push_back(config);
	}

	//load an active storage if there is any
	_currentStorageIndex = kStorageNoneId;
	if (ConfMan.hasKey("current_storage", ConfMan.kCloudDomain))
		_currentStorageIndex = ConfMan.getInt("current_storage", ConfMan.kCloudDomain);

	loadStorage();
}

void CloudManager::save() {
	for (uint32 i = 0; i < _storages.size(); ++i) {
		if (i == kStorageNoneId) continue;
		Common::String name = getStorageConfigName(i);
		ConfMan.set(kStoragePrefix + name + "_username", _storages[i].username, ConfMan.kCloudDomain);
		ConfMan.set(kStoragePrefix + name + "_lastSync", _storages[i].lastSyncDate, ConfMan.kCloudDomain);
		ConfMan.set(kStoragePrefix + name + "_usedBytes", Common::String::format("%llu", _storages[i].usedBytes), ConfMan.kCloudDomain);
	}

	ConfMan.set("current_storage", Common::String::format("%u", _currentStorageIndex), ConfMan.kCloudDomain);
	if (_activeStorage)
		_activeStorage->saveConfig(kStoragePrefix + getStorageConfigName(_currentStorageIndex) + "_");
	ConfMan.flushToDisk();
}

void CloudManager::replaceStorage(Storage *storage, uint32 index) {
	if (!storage) error("CloudManager::replaceStorage: NULL storage passed");
	if (index >= kStorageTotal) error("CloudManager::replaceStorage: invalid index passed");
	delete _activeStorage;
	_activeStorage = storage;
	_currentStorageIndex = index;
	save();

	//do what should be done on first Storage connect
	if (_activeStorage) {
		_activeStorage->info(nullptr, nullptr); //automatically calls setStorageUsername()
		_activeStorage->syncSaves(nullptr, nullptr);
	}
}

Storage *CloudManager::getCurrentStorage() const {
	return _activeStorage;
}

uint32 CloudManager::getStorageIndex() const {
	return _currentStorageIndex;
}

Common::StringArray CloudManager::listStorages() const {
	Common::StringArray result;
	for (uint32 i = 0; i < _storages.size(); ++i) {
		result.push_back(_storages[i].name);
	}
	return result;
}

bool CloudManager::switchStorage(uint32 index) {
	if (index >= _storages.size()) {
		warning("CloudManager::switchStorage: invalid index passed");
		return false;
	}

	Storage *storage = getCurrentStorage();
	if (storage && storage->isWorking()) {
		warning("CloudManager::switchStorage: another storage is working now");
		return false;
	}

	_currentStorageIndex = index;
	loadStorage();
	save();
	return true;
}

Common::String CloudManager::getStorageUsername(uint32 index) {
	if (index >= _storages.size()) return "";
	return _storages[index].username;
}

uint64 CloudManager::getStorageUsedSpace(uint32 index) {
	if (index >= _storages.size()) return 0;
	return _storages[index].usedBytes;
}

Common::String CloudManager::getStorageLastSync(uint32 index) {
	if (index >= _storages.size()) return "";
	if (index == _currentStorageIndex && isSyncing()) return "";
	return _storages[index].lastSyncDate;
}

void CloudManager::setStorageUsername(uint32 index, Common::String name) {
	if (index >= _storages.size()) return;
	_storages[index].username = name;
	save();
}

void CloudManager::setStorageUsedSpace(uint32 index, uint64 used) {
	if (index >= _storages.size()) return;
	_storages[index].usedBytes = used;
	save();
}

void CloudManager::setStorageLastSync(uint32 index, Common::String date) {
	if (index >= _storages.size()) return;
	_storages[index].lastSyncDate = date;
	save();
}

void CloudManager::connectStorage(uint32 index, Common::String code) {
	Storage *storage = nullptr;
	switch (index) {
	case kStorageDropboxId: storage = new Dropbox::DropboxStorage(code); break;
	case kStorageOneDriveId: storage = new OneDrive::OneDriveStorage(code); break;
	case kStorageGoogleDriveId: storage = new GoogleDrive::GoogleDriveStorage(code); break;
	}
	//these would automatically request replaceStorage() when they receive the token
}

void CloudManager::printBool(Storage::BoolResponse response) const {
	debug("bool = %s", (response.value ? "true" : "false"));
}

Networking::Request *CloudManager::listDirectory(Common::String path, Storage::ListDirectoryCallback callback, Networking::ErrorCallback errorCallback, bool recursive) {
	Storage *storage = getCurrentStorage();
	if (storage) return storage->listDirectory(path, callback, errorCallback, recursive);
	else {
		delete callback;
		delete errorCallback;
		//TODO: should we call errorCallback?
	}
	return nullptr;
}

Networking::Request *CloudManager::downloadFolder(Common::String remotePath, Common::String localPath, Storage::FileArrayCallback callback, Networking::ErrorCallback errorCallback, bool recursive) {
	Storage *storage = getCurrentStorage();
	if (storage) return storage->downloadFolder(remotePath, localPath, callback, errorCallback, recursive);
	else {
		delete callback;
		delete errorCallback;
		//TODO: should we call errorCallback?
	}
	return nullptr;
}

Networking::Request *CloudManager::info(Storage::StorageInfoCallback callback, Networking::ErrorCallback errorCallback) {
	Storage *storage = getCurrentStorage();
	if (storage) storage->info(callback, errorCallback);
	else {
		delete callback;
		delete errorCallback;
		//TODO: should we call errorCallback?
	}
	return nullptr;
}

Common::String CloudManager::savesDirectoryPath() {
	Storage *storage = getCurrentStorage();
	if (storage) return storage->savesDirectoryPath();
	return "";
}

SavesSyncRequest *CloudManager::syncSaves(Storage::BoolCallback callback, Networking::ErrorCallback errorCallback) {
	Storage *storage = getCurrentStorage();
	if (storage) {
		setStorageLastSync(_currentStorageIndex, "???"); //TODO get the date
		return storage->syncSaves(callback, errorCallback);
	}
	return nullptr;
}

void CloudManager::testFeature() {
	//Storage *storage = getCurrentStorage();
	//if (storage) storage->info(nullptr, nullptr);
}

bool CloudManager::isWorking() {
	Storage *storage = getCurrentStorage();
	if (storage) return storage->isWorking();
	return false;
}

bool CloudManager::isSyncing() {
	Storage *storage = getCurrentStorage();
	if (storage) return storage->isSyncing();
	return false;
}

double CloudManager::getSyncDownloadingProgress() {
	Storage *storage = getCurrentStorage();
	if (storage) return storage->getSyncDownloadingProgress();
	return 1;
}

double CloudManager::getSyncProgress() {
	Storage *storage = getCurrentStorage();
	if (storage) return storage->getSyncProgress();
	return 1;
}

Common::Array<Common::String> CloudManager::getSyncingFiles() {
	Storage *storage = getCurrentStorage();
	if (storage) return storage->getSyncingFiles();
	return Common::Array<Common::String>();
}

void CloudManager::cancelSync() {
	Storage *storage = getCurrentStorage();
	if (storage) storage->cancelSync();
}

void CloudManager::setSyncTarget(GUI::CommandReceiver *target) {
	Storage *storage = getCurrentStorage();
	if (storage) storage->setSyncTarget(target);
}

} // End of namespace Cloud
