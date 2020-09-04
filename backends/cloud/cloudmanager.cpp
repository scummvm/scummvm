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
#include "backends/cloud/box/boxstorage.h"
#include "backends/cloud/dropbox/dropboxstorage.h"
#include "backends/cloud/onedrive/onedrivestorage.h"
#include "backends/cloud/googledrive/googledrivestorage.h"
#include "common/translation.h"
#include "common/config-manager.h"
#include "common/str.h"
#ifdef USE_SDL_NET
#include "backends/networking/sdl_net/localwebserver.h"
#endif

namespace Common {

DECLARE_SINGLETON(Cloud::CloudManager);

}

namespace Cloud {

const char *const CloudManager::kStoragePrefix = "storage_";

CloudManager::CloudManager() : _currentStorageIndex(0), _activeStorage(nullptr) {}

CloudManager::~CloudManager() {
	g_system->getEventManager()->getEventDispatcher()->unregisterSource(this);

	delete _activeStorage;
	freeStorages();
}

Common::String CloudManager::getStorageConfigName(uint32 index) const {
	switch (index) {
	case kStorageNoneId: return "<none>";
	case kStorageDropboxId: return "Dropbox";
	case kStorageOneDriveId: return "OneDrive";
	case kStorageGoogleDriveId: return "GoogleDrive";
	case kStorageBoxId: return "Box";
	default:
		break;
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
	case kStorageBoxId:
		_activeStorage = Box::BoxStorage::loadFromConfig(kStoragePrefix + getStorageConfigName(_currentStorageIndex) + "_");
		break;
	default:
		_activeStorage = nullptr;
		break;
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
		config.name = name;
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

	g_system->getEventManager()->getEventDispatcher()->registerSource(this, false);
}

void CloudManager::save() {
	for (uint32 i = 0; i < _storages.size(); ++i) {
		if (i == kStorageNoneId)
			continue;
		Common::String name = getStorageConfigName(i);
		ConfMan.set(kStoragePrefix + name + "_username", _storages[i].username, ConfMan.kCloudDomain);
		ConfMan.set(kStoragePrefix + name + "_lastSync", _storages[i].lastSyncDate, ConfMan.kCloudDomain);
		ConfMan.set(kStoragePrefix + name + "_usedBytes", Common::String::format("%lu", _storages[i].usedBytes), ConfMan.kCloudDomain);
	}

	ConfMan.set("current_storage", Common::String::format("%u", _currentStorageIndex), ConfMan.kCloudDomain);
	if (_activeStorage)
		_activeStorage->saveConfig(kStoragePrefix + getStorageConfigName(_currentStorageIndex) + "_");
	ConfMan.flushToDisk();
}

void CloudManager::replaceStorage(Storage *storage, uint32 index) {
	freeStorages();
	if (!storage)
		error("CloudManager::replaceStorage: NULL storage passed");
	if (index >= kStorageTotal)
		error("CloudManager::replaceStorage: invalid index passed");
	if (_activeStorage != nullptr && _activeStorage->isWorking()) {
		warning("CloudManager::replaceStorage: replacing Storage while the other is working");
		if (_activeStorage->isDownloading())
			_activeStorage->cancelDownload();
		if (_activeStorage->isSyncing())
			_activeStorage->cancelSync();
		removeStorage(_activeStorage);
	} else {
		delete _activeStorage;
	}
	_activeStorage = storage;
	_currentStorageIndex = index;
	if (_storages[index].username == "") {
		// options' Cloud tab believes Storage is connected once it has non-empty username
		_storages[index].username = Common::convertFromU32String(_("<syncing...>"));
		_storages[index].lastSyncDate = Common::convertFromU32String(_("<right now>"));
		_storages[index].usedBytes = 0;
	}
	save();

	//do what should be done on first Storage connect
	if (_activeStorage) {
		_activeStorage->info(nullptr, nullptr); //automatically calls setStorageUsername()
	}
}

void CloudManager::removeStorage(Storage *storage) {
	// can't just delete it as it's mostly likely the one who calls the method
	// it would be freed on freeStorages() call (on next Storage connect or replace)
	_storagesToRemove.push_back(storage);
}

void CloudManager::freeStorages() {
	for (uint32 i = 0; i < _storagesToRemove.size(); ++i)
		delete _storagesToRemove[i];
	_storagesToRemove.clear();
}

void CloudManager::passNoStorageConnected(Networking::ErrorCallback errorCallback) const {
	if (errorCallback == nullptr)
		return;
	(*errorCallback)(Networking::ErrorResponse(nullptr, false, true, "No Storage connected!", -1));
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
	if (index >= _storages.size())
		return "";
	return _storages[index].username;
}

uint64 CloudManager::getStorageUsedSpace(uint32 index) {
	if (index >= _storages.size())
		return 0;
	return _storages[index].usedBytes;
}

Common::String CloudManager::getStorageLastSync(uint32 index) {
	if (index >= _storages.size())
		return "";
	if (index == _currentStorageIndex && isSyncing())
		return "";
	return _storages[index].lastSyncDate;
}

void CloudManager::setStorageUsername(uint32 index, Common::String name) {
	if (index >= _storages.size())
		return;
	_storages[index].username = name;
	save();
}

void CloudManager::setStorageUsedSpace(uint32 index, uint64 used) {
	if (index >= _storages.size())
		return;
	_storages[index].usedBytes = used;
	save();
}

void CloudManager::setStorageLastSync(uint32 index, Common::String date) {
	if (index >= _storages.size())
		return;
	_storages[index].lastSyncDate = date;
	save();
}

void CloudManager::connectStorage(uint32 index, Common::String code, Networking::ErrorCallback cb) {
	freeStorages();

	switch (index) {
	case kStorageDropboxId:
		new Dropbox::DropboxStorage(code, cb);
		break;
	case kStorageOneDriveId:
		new OneDrive::OneDriveStorage(code, cb);
		break;
	case kStorageGoogleDriveId:
		new GoogleDrive::GoogleDriveStorage(code, cb);
		break;
	case kStorageBoxId:
		new Box::BoxStorage(code, cb);
		break;
	default:
		break;
	}
	// in these constructors Storages request token using the passed code
	// when the token is received, they call replaceStorage()
	// or removeStorage(), if some error occurred
	// thus, no memory leak happens
}

void CloudManager::disconnectStorage(uint32 index) {
	if (index >= kStorageTotal)
		error("CloudManager::disconnectStorage: invalid index passed");

	Common::String name = getStorageConfigName(index);
	switch (index) {
	case kStorageDropboxId:
		Dropbox::DropboxStorage::removeFromConfig(kStoragePrefix + name + "_");
		break;
	case kStorageOneDriveId:
		OneDrive::OneDriveStorage::removeFromConfig(kStoragePrefix + name + "_");
		break;
	case kStorageGoogleDriveId:
		GoogleDrive::GoogleDriveStorage::removeFromConfig(kStoragePrefix + name + "_");
		break;
	case kStorageBoxId:
		Box::BoxStorage::removeFromConfig(kStoragePrefix + name + "_");
		break;
	default:
		break;
	}

	switchStorage(kStorageNoneId);

	ConfMan.removeKey(kStoragePrefix + name + "_username", ConfMan.kCloudDomain);
	ConfMan.removeKey(kStoragePrefix + name + "_lastSync", ConfMan.kCloudDomain);
	ConfMan.removeKey(kStoragePrefix + name + "_usedBytes", ConfMan.kCloudDomain);

	StorageConfig config;
	config.name = name;
	config.username = "";
	config.lastSyncDate = "";
	config.usedBytes = 0;

	_storages[index] = config;
}


Networking::Request *CloudManager::listDirectory(Common::String path, Storage::ListDirectoryCallback callback, Networking::ErrorCallback errorCallback, bool recursive) {
	Storage *storage = getCurrentStorage();
	if (storage) {
		return storage->listDirectory(path, callback, errorCallback, recursive);
	} else {
		passNoStorageConnected(errorCallback);
		delete callback;
		delete errorCallback;
	}
	return nullptr;
}

Networking::Request *CloudManager::downloadFolder(Common::String remotePath, Common::String localPath, Storage::FileArrayCallback callback, Networking::ErrorCallback errorCallback, bool recursive) {
	Storage *storage = getCurrentStorage();
	if (storage) {
		return storage->downloadFolder(remotePath, localPath, callback, errorCallback, recursive);
	} else {
		passNoStorageConnected(errorCallback);
		delete callback;
		delete errorCallback;
	}
	return nullptr;
}

Networking::Request *CloudManager::info(Storage::StorageInfoCallback callback, Networking::ErrorCallback errorCallback) {
	Storage *storage = getCurrentStorage();
	if (storage) {
		return storage->info(callback, errorCallback);
	} else {
		passNoStorageConnected(errorCallback);
		delete callback;
		delete errorCallback;
	}
	return nullptr;
}

Common::String CloudManager::savesDirectoryPath() {
	Storage *storage = getCurrentStorage();
	if (storage)
		return storage->savesDirectoryPath();
	return "";
}

bool CloudManager::canSyncFilename(const Common::String &filename) const {
	if (filename == "" || filename[0] == '.')
		return false;

	return true;
}

bool CloudManager::isStorageEnabled() const {
	Storage *storage = getCurrentStorage();
	if (storage)
		return storage->isEnabled();
	return false;
}

void CloudManager::enableStorage() {
	Storage *storage = getCurrentStorage();
	if (storage) {
		storage->enable();
		save();
	}
}

SavesSyncRequest *CloudManager::syncSaves(Storage::BoolCallback callback, Networking::ErrorCallback errorCallback) {
	Storage *storage = getCurrentStorage();
	if (storage) {
		setStorageLastSync(_currentStorageIndex, "???"); //TODO get the date
		return storage->syncSaves(callback, errorCallback);
	} else {
		passNoStorageConnected(errorCallback);
		delete callback;
		delete errorCallback;
	}
	return nullptr;
}

bool CloudManager::isWorking() const {
	Storage *storage = getCurrentStorage();
	if (storage)
		return storage->isWorking();
	return false;
}

///// SavesSyncRequest-related /////

bool CloudManager::isSyncing() const {
	Storage *storage = getCurrentStorage();
	if (storage)
		return storage->isSyncing();
	return false;
}

double CloudManager::getSyncDownloadingProgress() const {
	Storage *storage = getCurrentStorage();
	if (storage)
		return storage->getSyncDownloadingProgress();
	return 1;
}

double CloudManager::getSyncProgress() const {
	Storage *storage = getCurrentStorage();
	if (storage)
		return storage->getSyncProgress();
	return 1;
}

Common::Array<Common::String> CloudManager::getSyncingFiles() const {
	Storage *storage = getCurrentStorage();
	if (storage)
		return storage->getSyncingFiles();
	return Common::Array<Common::String>();
}

void CloudManager::cancelSync() const {
	Storage *storage = getCurrentStorage();
	if (storage)
		storage->cancelSync();
}

void CloudManager::setSyncTarget(GUI::CommandReceiver *target) const {
	Storage *storage = getCurrentStorage();
	if (storage)
		storage->setSyncTarget(target);
}

void CloudManager::showCloudDisabledIcon() {
	_icon.show(CloudIcon::kDisabled, 3000);
}

///// DownloadFolderRequest-related /////

bool CloudManager::startDownload(Common::String remotePath, Common::String localPath) const {
	Storage *storage = getCurrentStorage();
	if (storage)
		return storage->startDownload(remotePath, localPath);
	return false;
}

void CloudManager::cancelDownload() const {
	Storage *storage = getCurrentStorage();
	if (storage)
		storage->cancelDownload();
}

void CloudManager::setDownloadTarget(GUI::CommandReceiver *target) const {
	Storage *storage = getCurrentStorage();
	if (storage)
		storage->setDownloadTarget(target);
}

bool CloudManager::isDownloading() const {
	Storage *storage = getCurrentStorage();
	if (storage)
		return storage->isDownloading();
	return false;
}

double CloudManager::getDownloadingProgress() const {
	Storage *storage = getCurrentStorage();
	if (storage)
		return storage->getDownloadingProgress();
	return 1;
}

uint64 CloudManager::getDownloadBytesNumber() const {
	Storage *storage = getCurrentStorage();
	if (storage)
		return storage->getDownloadBytesNumber();
	return 0;
}

uint64 CloudManager::getDownloadTotalBytesNumber() const {
	Storage *storage = getCurrentStorage();
	if (storage)
		return storage->getDownloadTotalBytesNumber();
	return 0;
}

uint64 CloudManager::getDownloadSpeed() const {
	Storage *storage = getCurrentStorage();
	if (storage)
		return storage->getDownloadSpeed();
	return 0;
}

Common::String CloudManager::getDownloadRemoteDirectory() const {
	Storage *storage = getCurrentStorage();
	if (storage)
		return storage->getDownloadRemoteDirectory();
	return "";
}

Common::String CloudManager::getDownloadLocalDirectory() const {
	Storage *storage = getCurrentStorage();
	if (storage)
		return storage->getDownloadLocalDirectory();
	return "";
}

bool CloudManager::pollEvent(Common::Event &event) {
	if (_icon.needsUpdate()) {
		if (_icon.getShownType() != CloudIcon::kDisabled) {
			if (isWorking()) {
				_icon.show(CloudIcon::kSyncing);
			} else {
				_icon.show(CloudIcon::kNone);
			}
		}

		_icon.update();
	}

	return false;
}

} // End of namespace Cloud
