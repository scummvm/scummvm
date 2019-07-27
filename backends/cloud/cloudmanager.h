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

#ifndef CLOUD_CLOUDMANAGER_H
#define CLOUD_CLOUDMANAGER_H

#include "backends/cloud/storage.h"
#include "backends/cloud/cloudicon.h"

#include "common/array.h"
#include "common/singleton.h"
#include "common/str-array.h"
#include "common/events.h"

namespace GUI {

class CommandReceiver;

}

namespace Cloud {

// The actual indexes in CloudManager's array
enum StorageID {
	kStorageNoneId = 0,
	kStorageDropboxId = 1,
	kStorageOneDriveId = 2,
	kStorageGoogleDriveId = 3,
	kStorageBoxId = 4,

	kStorageTotal
};

class CloudManager : public Common::Singleton<CloudManager>, public Common::EventSource {
	static const char *const kStoragePrefix;

	struct StorageConfig {
		Common::String name, username;
		uint64 usedBytes;
		Common::String lastSyncDate;
	};

	Common::Array<StorageConfig> _storages;
	uint _currentStorageIndex;
	Storage *_activeStorage;
	Common::Array<Storage *> _storagesToRemove;

	CloudIcon _icon;

	void loadStorage();

	Common::String getStorageConfigName(uint32 index) const;

	/** Frees memory used by storages which failed to connect. */
	void freeStorages();

	/** Calls the error callback with a special "no storage connected" message. */
	void passNoStorageConnected(Networking::ErrorCallback errorCallback) const;

	/**
	 * Common::EventSource interface
	 *
	 * The cloud manager registers itself as an event source even if does not
	 * actually produce events as a mean to be polled periodically by the GUI
	 * or engine code.
	 *
	 * The periodical polling is used to update the OSD icon indicating
	 * background sync activity.
	 */
	virtual bool pollEvent(Common::Event &event) override;

public:
	CloudManager();
	virtual ~CloudManager();

	/**
	 * Loads all information from configs and creates current Storage instance.
	 *
	 * @note It's called once on startup in scummvm_main().
	 */
	void init();

	/**
	 * Saves all information into configuration file.
	 */
	void save();

	/**
	 * Replace active Storage.
	 * @note this method automatically saves the changes with ConfMan.
	 *
	 * @param   storage Cloud::Storage to replace active storage with.
	 * @param   index   one of Cloud::StorageID enum values to indicate what storage type is replaced.
	 */
	void replaceStorage(Storage *storage, uint32 index);

	/** Adds storage in the list of storages to remove later. */
	void removeStorage(Storage *storage);

	/**
	 * Returns active Storage, which could be used to interact
	 *  with cloud storage.
	 *
	 * @return  active Cloud::Storage or null, if there is no active Storage.
	 */
	Cloud::Storage *getCurrentStorage() const;

	/**
	 * Return active Storage's index.
	 *
	 * @return  active Storage's index.
	 */
	uint32 getStorageIndex() const;

	/**
	 * Return Storages names as list.
	 *
	 * @return  a list of Storages names.
	 */
	Common::StringArray listStorages() const;

	/**
	 * Changes the storage to the one with given index.
	 *
	 * @param   new Storage's index.
	 */
	bool switchStorage(uint32 index);

	/**
	 * Return username used by Storage.
	 *
	 * @param   Storage's index.
	 * @returns username or "" if index is invalid (no such Storage).
	 */
	Common::String getStorageUsername(uint32 index);

	/**
	 * Return space used by Storage.
	 *
	 * @param   Storage's index.
	 * @returns used space in bytes or 0 if index is invalid (no such Storage).
	 */
	uint64 getStorageUsedSpace(uint32 index);

	/**
	 * Return Storage's last sync date.
	 *
	 * @param   Storage's index.
	 * @returns last sync date or "" if index is invalid (no such Storage).
	            It also returns "" if there never was any sync
	            or if storage is syncing right now.
	 */
	Common::String getStorageLastSync(uint32 index);

	/**
	 * Set Storage's username.
	 * Automatically saves changes to the config.
	 *
	 * @param   index   Storage's index.
	 * @param   name    username to set
	 */
	void setStorageUsername(uint32 index, Common::String name);

	/**
	 * Set Storage's used space field.
	 * Automatically saves changes to the config.
	 *
	 * @param   index   Storage's index.
	 * @param   used    value to set
	 */
	void setStorageUsedSpace(uint32 index, uint64 used);

	/**
	 * Set Storage's last sync date.
	 * Automatically saves changes to the config.
	 *
	 * @param   index   Storage's index.
	 * @param   date    date to set
	 */
	void setStorageLastSync(uint32 index, Common::String date);

	/**
	 * Replace Storage which has given index with a
	 * storage created with given code.
	 *
	 * @param   index   Storage's index
	 * @param   code    OAuth2 code received from user
	 * @param	cb		callback to notify of success or error
	 */
	void connectStorage(uint32 index, Common::String code, Networking::ErrorCallback cb = nullptr);

	/**
	 * Remove Storage with a given index from config.
	 *
	 * @param   index   Storage's index
	 */
	void disconnectStorage(uint32 index);

	/** Returns ListDirectoryResponse with list of files. */
	Networking::Request *listDirectory(Common::String path, Storage::ListDirectoryCallback callback, Networking::ErrorCallback errorCallback, bool recursive = false);

	/** Returns Common::Array<StorageFile> with list of files, which were not downloaded. */
	Networking::Request *downloadFolder(Common::String remotePath, Common::String localPath, Storage::FileArrayCallback callback, Networking::ErrorCallback errorCallback, bool recursive = false);

	/** Return the StorageInfo struct. */
	Networking::Request *info(Storage::StorageInfoCallback callback, Networking::ErrorCallback errorCallback);

	/** Returns storage's saves directory path with the trailing slash. */
	Common::String savesDirectoryPath();

	/** Returns whether given filename could be uploaded to or downloaded from storage. */
	bool canSyncFilename(const Common::String &filename) const;

	/** Returns whether current Storage is manually enabled by user (or false, if there is no active Storage). */
	bool isStorageEnabled() const;

	/** Sets Storage::_isEnabled to true and updates the config. */
	void enableStorage();

	/**
	 * Starts saves syncing process in currently active storage if there is any.
	 */
	SavesSyncRequest *syncSaves(Cloud::Storage::BoolCallback callback = nullptr, Networking::ErrorCallback errorCallback = nullptr);

	/** Returns whether there are any requests running. */
	bool isWorking() const;

	///// SavesSyncRequest-related /////

	/** Returns whether there is a SavesSyncRequest running. */
	bool isSyncing() const;

	/** Returns a number in [0, 1] range which represents current sync downloading progress (1 = complete). */
	double getSyncDownloadingProgress() const;

	/** Returns a number in [0, 1] range which represents current sync progress (1 = complete). */
	double getSyncProgress() const;

	/** Returns an array of saves names which are not yet synced (thus cannot be used). */
	Common::Array<Common::String> getSyncingFiles() const;

	/** Cancels running sync. */
	void cancelSync() const;

	/** Sets SavesSyncRequest's target to given CommandReceiver. */
	void setSyncTarget(GUI::CommandReceiver *target) const;

	/** Shows a "cloud disabled" icon for three seconds. */
	void showCloudDisabledIcon();

	///// DownloadFolderRequest-related /////

	/** Starts a folder download. */
	bool startDownload(Common::String remotePath, Common::String localPath) const;

	/** Cancels running download. */
	void cancelDownload() const;

	/** Sets FolderDownloadRequest's target to given CommandReceiver. */
	void setDownloadTarget(GUI::CommandReceiver *target) const;

	/** Returns whether there is a FolderDownloadRequest running. */
	bool isDownloading() const;

	/** Returns a number in [0, 1] range which represents current download progress (1 = complete). */
	double getDownloadingProgress() const;

	/** Returns a number of bytes that is downloaded in current download progress. */
	uint64 getDownloadBytesNumber() const;

	/** Returns a total number of bytes to be downloaded in current download progress. */
	uint64 getDownloadTotalBytesNumber() const;

	/** Returns download speed of current download progress. */
	uint64 getDownloadSpeed() const;

	/** Returns remote directory path. */
	Common::String getDownloadRemoteDirectory() const;

	/** Returns local directory path. */
	Common::String getDownloadLocalDirectory() const;
};

/** Shortcut for accessing the connection manager. */
#define CloudMan        Cloud::CloudManager::instance()

} // End of namespace Cloud

#endif
