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

#ifndef BACKENDS_CLOUD_BOX_BOXSTORAGE_H
#define BACKENDS_CLOUD_BOX_BOXSTORAGE_H

#include "backends/cloud/id/idstorage.h"
#include "backends/networking/curl/curljsonrequest.h"

namespace Cloud {
namespace Box {

class BoxStorage: public Id::IdStorage {
	/** This private constructor is called from loadFromConfig(). */
	BoxStorage(Common::String token, Common::String refreshToken, bool enabled);

	/** Constructs StorageInfo based on JSON response from cloud. */
	void infoInnerCallback(StorageInfoCallback outerCallback, Networking::JsonResponse json);

	void createDirectoryInnerCallback(BoolCallback outerCallback, Networking::JsonResponse response);

protected:
	/**
	 * @return "box"
	 */
	virtual Common::String cloudProvider();

	/**
	 * @return kStorageBoxId
	 */
	virtual uint32 storageIndex();

	virtual bool needsRefreshToken();

	virtual bool canReuseRefreshToken();

public:
	/** This constructor uses OAuth code flow to get tokens. */
	BoxStorage(Common::String code, Networking::ErrorCallback cb);
	virtual ~BoxStorage();

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

	/**
	* Return unique storage name.
	* @returns  some unique storage name (for example, "Dropbox (user@example.com)")
	*/
	virtual Common::String name() const;

	/** Public Cloud API comes down there. */

	virtual Networking::Request *listDirectoryById(Common::String id, ListDirectoryCallback callback, Networking::ErrorCallback errorCallback);
	virtual Networking::Request *createDirectoryWithParentId(Common::String parentId, Common::String directoryName, BoolCallback callback, Networking::ErrorCallback errorCallback);

	/** Returns UploadStatus struct with info about uploaded file. */
	virtual Networking::Request *upload(Common::String remotePath, Common::String localPath, UploadCallback callback, Networking::ErrorCallback errorCallback);
	virtual Networking::Request *upload(Common::String path, Common::SeekableReadStream *contents, UploadCallback callback, Networking::ErrorCallback errorCallback);

	/** Returns whether Storage supports upload(ReadStream). */
	virtual bool uploadStreamSupported();

	/** Returns pointer to Networking::NetworkReadStream. */
	virtual Networking::Request *streamFileById(Common::String path, Networking::NetworkReadStreamCallback callback, Networking::ErrorCallback errorCallback);

	/** Returns the StorageInfo struct. */
	virtual Networking::Request *info(StorageInfoCallback callback, Networking::ErrorCallback errorCallback);

	/** Returns storage's saves directory path with the trailing slash. */
	virtual Common::String savesDirectoryPath();

	/**
	 * Load token and user id from configs and return BoxStorage for those.
	 * @return pointer to the newly created BoxStorage or 0 if some problem occured.
	 */
	static BoxStorage *loadFromConfig(Common::String keyPrefix);

	/**
	 * Remove all BoxStorage-related data from config.
	 */
	static void removeFromConfig(Common::String keyPrefix);

	virtual Common::String getRootDirectoryId();

	Common::String accessToken() const { return _token; }
};

} // End of namespace Box
} // End of namespace Cloud

#endif
