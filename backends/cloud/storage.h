/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef BACKENDS_CLOUD_STORAGE_H
#define BACKENDS_CLOUD_STORAGE_H

#include "backends/cloud/storagefile.h"
#include "backends/cloud/storageinfo.h"
#include "backends/networking/curl/request.h"
#include "backends/networking/curl/curlrequest.h"
#include "common/array.h"
#include "common/callback.h"
#include "common/mutex.h"
#include "common/path.h"
#include "common/stream.h"
#include "common/str.h"

namespace GUI {

class CommandReceiver;

}

namespace Cloud {

class SavesSyncRequest;
class FolderDownloadRequest;

class Storage {
public:
	typedef Networking::Response<const Common::Array<StorageFile> &> FileArrayResponse;
	typedef Networking::Response<const StorageInfo &> StorageInfoResponse;
	typedef Networking::Response<bool> BoolResponse;
	typedef Networking::Response<const StorageFile &> UploadResponse;
	typedef Networking::Response<const Common::Array<StorageFile> &> ListDirectoryResponse;

	typedef Common::BaseCallback<const FileArrayResponse &> *FileArrayCallback;
	typedef Common::BaseCallback<const StorageInfoResponse &> *StorageInfoCallback;
	typedef Common::BaseCallback<const BoolResponse &> *BoolCallback;
	typedef Common::BaseCallback<const UploadResponse &> *UploadCallback;
	typedef Common::BaseCallback<const ListDirectoryResponse &> *ListDirectoryCallback;

protected:
	/** Keeps track of running requests. */
	uint32 _runningRequestsCount;
	Common::Mutex _runningRequestsMutex;

	/** SavesSyncRequest-related */
	SavesSyncRequest *_savesSyncRequest;
	bool _syncRestartRequestsed;

	/** FolderDownloadRequest-related */
	FolderDownloadRequest *_downloadFolderRequest;

	/** Whether user manually enabled the Storage. */
	bool _isEnabled;

	/** Returns default error callback (printErrorResponse). */
	virtual Networking::ErrorCallback getErrorPrintingCallback();

	/** Prints ErrorResponse contents with debug(). */
	virtual void printErrorResponse(const Networking::ErrorResponse &error);

	/**
	 * Adds request to the ConnMan, but also increases _runningRequestsCount.
	 * This method should be used by Storage implementations instead of
	 * direct ConnMan.addRequest() call.
	 *
	 * @return the same Request pointer, just as a shortcut
	 */
	virtual Networking::Request *addRequest(Networking::Request *request);

	/**
	 * Decreases _runningRequestCount. It's called from ConnMan automatically.
	 * Passed pointer is dangling, but one can use the address to determine
	 * some special Requests (which addresses were remembered somewhere).
	 */
	virtual void requestFinishedCallback(Networking::Request *invalidRequestPointer);

public:
	Storage();
	virtual ~Storage();

	/**
	 * Storage methods, which are used by CloudManager to save
	 * storage in configuration file.
	 */

	/**
	 * Save storage data using ConfMan.
	 * @param keyPrefix all saved keys must start with this prefix.
	 * @note every Storage must write keyPrefix + "type" key
	 *       with common value (e.g. "Dropbox").
	 */
	virtual void saveConfig(const Common::String  &keyPrefix) = 0;

	/**
	* Return unique storage name.
	* @returns  some unique storage name (for example, "Dropbox (user@example.com)")
	*/
	virtual Common::String name() const = 0;

	/**
	 * Return whether Storage has been manually enabled by user.
	 */
	bool isEnabled() const;

	/**
	 * Set _isEnabled to true.
	 */
	void enable();

	/**
	 * Public Cloud API comes down there.
	 *
	 * All Cloud API methods return Networking::Request *, which
	 * might be used to control request. All methods also accept
	 * a callback, which is called, when request is complete.
	 */

	/** Returns ListDirectoryResponse with list of files. */
	virtual Networking::Request *listDirectory(const Common::String &path, ListDirectoryCallback callback, Networking::ErrorCallback errorCallback, bool recursive = false) = 0;

	/** Returns StorageFile with info about uploaded file. */
	virtual Networking::Request *upload(const Common::String &path, Common::SeekableReadStream *contents, UploadCallback callback, Networking::ErrorCallback errorCallback) = 0;
	virtual Networking::Request *upload(const Common::String &remotePath, const Common::Path &localPath, UploadCallback callback, Networking::ErrorCallback errorCallback);

	/** Returns whether Storage supports upload(ReadStream). */
	virtual bool uploadStreamSupported();

	/** Returns pointer to Networking::NetworkReadStream. */
	virtual Networking::Request *streamFile(const Common::String &path, Networking::NetworkReadStreamCallback callback, Networking::ErrorCallback errorCallback);
	virtual Networking::Request *streamFileById(const Common::String &id, Networking::NetworkReadStreamCallback callback, Networking::ErrorCallback errorCallback) = 0;

	/** Calls the callback when finished. */
	virtual Networking::Request *download(const Common::String &remotePath, const Common::Path &localPath, BoolCallback callback, Networking::ErrorCallback errorCallback);
	virtual Networking::Request *downloadById(const Common::String &remoteId, const Common::Path &localPath, BoolCallback callback, Networking::ErrorCallback errorCallback);

	/** Returns Common::Array<StorageFile> with list of files, which were not downloaded. */
	virtual Networking::Request *downloadFolder(const Common::String &remotePath, const Common::Path &localPath, FileArrayCallback callback, Networking::ErrorCallback errorCallback, bool recursive = false);

	/** Calls the callback when finished. */
	virtual SavesSyncRequest *syncSaves(BoolCallback callback, Networking::ErrorCallback errorCallback);

	/** Calls the callback when finished. */
	virtual Networking::Request *createDirectory(const Common::String &path, BoolCallback callback, Networking::ErrorCallback errorCallback) = 0;

	/**
	 * Returns the StorageInfo struct via <callback>.
	 * Calls the <errorCallback> if failed to get information.
	 *
	 * @note on success Storage should also call
	 *       CloudMan.setStorageUsername().
	 */
	virtual Networking::Request *info(StorageInfoCallback callback, Networking::ErrorCallback errorCallback) = 0;

	/** Returns storage's saves directory path with the trailing slash. */
	virtual Common::String savesDirectoryPath() = 0;

	/** Returns whether there are any requests running. */
	virtual bool isWorking();

	///// SavesSyncRequest-related /////

	/** Returns whether there is a SavesSyncRequest running. */
	virtual bool isSyncing();

	/** Returns a number in [0, 1] range which represents current sync downloading progress (1 = complete). */
	virtual double getSyncDownloadingProgress();

	struct SyncDownloadingInfo {
		uint64 bytesDownloaded = 0, bytesToDownload = 0;
		uint64 filesDownloaded = 0, filesToDownload = 0;
		bool inProgress = false;
	};

	/** Fills a struct with numbers about current sync downloading progress. */
	virtual void getSyncDownloadingInfo(SyncDownloadingInfo &info);

	/** Returns a number in [0, 1] range which represents current sync progress (1 = complete). */
	virtual double getSyncProgress();

	/** Returns an array of saves names which are not yet synced (thus cannot be used). */
	virtual Common::Array<Common::String> getSyncingFiles();

	/** Cancels running sync. */
	virtual void cancelSync();

protected:
	/** Finishes the sync. Shows an OSD message. */
	virtual void savesSyncDefaultCallback(const BoolResponse &response);

	/** Finishes the sync. Shows an OSD message. */
	virtual void savesSyncDefaultErrorCallback(const Networking::ErrorResponse &error);

public:
	///// DownloadFolderRequest-related /////

	/** Starts a folder download. */
	virtual bool startDownload(const Common::String &remotePath, const Common::Path &localPath);

	/** Cancels running download. */
	virtual void cancelDownload();

	/** Sets FolderDownloadRequest's target to given CommandReceiver. */
	virtual void setDownloadTarget(GUI::CommandReceiver *target);

	/** Returns whether there is a FolderDownloadRequest running. */
	virtual bool isDownloading();

	/** Returns a number in [0, 1] range which represents current download progress (1 = complete). */
	virtual double getDownloadingProgress();

	/** Returns a number of bytes that is downloaded in current download progress. */
	virtual uint64 getDownloadBytesNumber();

	/** Returns a total number of bytes to be downloaded in current download progress. */
	virtual uint64 getDownloadTotalBytesNumber();

	/** Returns download speed of current download progress. */
	virtual uint64 getDownloadSpeed();

	/** Returns remote directory path. */
	virtual Common::String getDownloadRemoteDirectory();

	/** Returns local directory path. */
	virtual Common::Path getDownloadLocalDirectory();

protected:
	/** Finishes the download. Shows an OSD message. */
	virtual void directoryDownloadedCallback(const FileArrayResponse &response);

	/** Finishes the download. Shows an OSD message. */
	virtual void directoryDownloadedErrorCallback(const Networking::ErrorResponse &error);
};

} // End of namespace Cloud

#endif
