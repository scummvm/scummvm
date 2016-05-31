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

#ifndef BACKENDS_CLOUD_DROPBOX_STORAGE_H
#define BACKENDS_CLOUD_DROPBOX_STORAGE_H

#include "backends/cloud/storage.h"
#include "common/callback.h"
#include "backends/networking/curl/curljsonrequest.h"

namespace Cloud {
namespace Dropbox {

class DropboxStorage: public Cloud::Storage {
	static char *KEY, *SECRET;

	static void loadKeyAndSecret();

	Common::String _token, _uid;

	/** This private constructor is called from loadFromConfig(). */
	DropboxStorage(Common::String token, Common::String uid);

	static void getAccessToken(Common::String code);

	/** Constructs StorageInfo based on JSON response from cloud. */
	void infoInnerCallback(StorageInfoCallback outerCallback, Networking::JsonResponse json);

	void printFiles(FileArrayResponse response);
	void printBool(BoolResponse response);
	void printStorageFile(UploadResponse response);

public:	
	virtual ~DropboxStorage();

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
	virtual void saveConfig(Common::String keyPrefix);

	/** Public Cloud API comes down there. */

	/** Returns ListDirectoryStatus struct with list of files. */
	virtual Networking::Request *listDirectory(Common::String path, ListDirectoryCallback callback, Networking::ErrorCallback errorCallback, bool recursive = false);
	
	/** Returns UploadStatus struct with info about uploaded file. */
	virtual Networking::Request *upload(Common::String path, Common::SeekableReadStream *contents, UploadCallback callback, Networking::ErrorCallback errorCallback);	

	/** Returns pointer to Networking::NetworkReadStream. */
	virtual Networking::Request *streamFile(Common::String path, Networking::NetworkReadStreamCallback callback, Networking::ErrorCallback errorCallback);

	/** Calls the callback when finished. */
	virtual Networking::Request *download(Common::String remotePath, Common::String localPath, BoolCallback callback, Networking::ErrorCallback errorCallback);

	/** Returns Common::Array<StorageFile> with list of files, which were not downloaded. */
	virtual Networking::Request *downloadFolder(Common::String remotePath, Common::String localPath, FileArrayCallback callback, Networking::ErrorCallback errorCallback, bool recursive = false);

	/** Calls the callback when finished. */
	virtual Networking::Request *remove(Common::String path, BoolCallback callback, Networking::ErrorCallback errorCallback) { return nullptr; } //TODO

	/** Calls the callback when finished. */
	virtual Networking::Request *createDirectory(Common::String path, BoolCallback callback, Networking::ErrorCallback errorCallback) { return nullptr; } //TODO

	/** Calls the callback when finished. */
	virtual Networking::Request *touch(Common::String path, BoolCallback callback, Networking::ErrorCallback errorCallback) { return nullptr; } //TODO

	/** Returns the StorageInfo struct. */
	virtual Networking::Request *info(StorageInfoCallback callback, Networking::ErrorCallback errorCallback);

	/** This method is passed into info(). (Temporary) */
	void infoMethodCallback(StorageInfoResponse response);

	/** Returns storage's saves directory path with the trailing slash. */
	virtual Common::String savesDirectoryPath();

	/** Returns whether saves sync process is running. */
	virtual bool isSyncing() { return false; } //TODO

	/** Returns whether there are any requests running. */
	virtual bool isWorking() { return false; } //TODO

	/**
	 * Load token and user id from configs and return DropboxStorage for those.	
	 * @return pointer to the newly created DropboxStorage or 0 if some problem occured.
	 */
	static DropboxStorage *loadFromConfig(Common::String keyPrefix);

	/**
	 * Returns Dropbox auth link.
	 */
	static Common::String getAuthLink();

	/**
	 * Show message with Dropbox auth instructions. (Temporary)
	 */
	static void authThroughConsole();
};

} // End of namespace Dropbox
} // End of namespace Cloud

#endif
