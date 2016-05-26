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

#ifndef BACKENDS_CLOUD_STORAGE_H
#define BACKENDS_CLOUD_STORAGE_H

#include "common/array.h"
#include "common/stream.h"
#include "common/str.h"
#include "common/callback.h"
#include "backends/cloud/storagefile.h"
#include "backends/cloud/storageinfo.h"
#include "backends/networking/curl/networkreadstream.h"
#include <backends/networking/curl/request.h>

namespace Cloud {

class Storage {
public:
	typedef Networking::RequestIdPair<Common::Array<StorageFile>&> RequestFileArrayPair;
	typedef Networking::RequestIdPair<Networking::NetworkReadStream *> RequestReadStreamPair;
	typedef Networking::RequestIdPair<StorageInfo> RequestStorageInfoPair;
	typedef Networking::RequestIdPair<bool> RequestBoolPair;	

	typedef Common::BaseCallback<RequestFileArrayPair> *FileArrayCallback;
	typedef Common::BaseCallback<RequestReadStreamPair> *ReadStreamCallback;
	typedef Common::BaseCallback<RequestStorageInfoPair> *StorageInfoCallback;
	typedef Common::BaseCallback<RequestBoolPair> *BoolCallback;

	Storage() {}
	virtual ~Storage() {}

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

	virtual void saveConfig(Common::String keyPrefix) = 0;

	/**
	 * Public Cloud API comes down there.
	 *
	 * All Cloud API methods return int32 request id, which might be used to access
	 * request through ConnectionManager. All methods also accept a callback, which
	 * would be called, when request is complete.
	 */

	/** Returns Common::Array<StorageFile>. */
	virtual int32 listDirectory(Common::String path, FileArrayCallback callback, bool recursive = false) = 0;

	/** Calls the callback when finished. */
	virtual int32 upload(Common::String path, Common::ReadStream *contents, BoolCallback callback) = 0;

	/** Returns pointer to Networking::NetworkReadStream. */
	virtual int32 streamFile(Common::String path, ReadStreamCallback callback) = 0;

	/** Calls the callback when finished. */
	virtual int32 download(Common::String remotePath, Common::String localPath, BoolCallback callback) = 0;

	/** Calls the callback when finished. */
	virtual int32 remove(Common::String path, BoolCallback callback) = 0;

	/** Calls the callback when finished. */
	virtual int32 syncSaves(BoolCallback callback) = 0;

	/** Calls the callback when finished. */
	virtual int32 createDirectory(Common::String path, BoolCallback callback) = 0;

	/** Calls the callback when finished. */
	virtual int32 touch(Common::String path, BoolCallback callback) = 0;

	/** Returns the StorageInfo struct. */
	virtual int32 info(StorageInfoCallback callback) = 0;

	/** Returns whether saves sync process is running. */
	virtual bool isSyncing() = 0;

	/** Returns whether there are any requests running. */
	virtual bool isWorking() = 0;
};

} //end of namespace Cloud

#endif
