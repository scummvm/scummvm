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

#ifndef BACKENDS_CLOUD_ID_IDSTORAGE_H
#define BACKENDS_CLOUD_ID_IDSTORAGE_H

#include "backends/cloud/basestorage.h"
#include "backends/networking/curl/curljsonrequest.h"

/*
 * Id::IdStorage is a special base class, which is created
 * to simplify adding new storages which use ids instead of
 * paths in their API.
 *
 * Some Requests are already implemented, and Storage based
 * on IdStorage needs to override/implement a few basic things.
 *
 * For example, ListDirectoryRequest and ResolveIdRequests are
 * based on listDirectoryById() and getRootDirectoryId() methods.
 * Implementing these you'll get id resolving and directory
 * listing by path.
 */

namespace Cloud {
namespace Id {

class IdStorage: public Cloud::BaseStorage {
protected:
	void printFiles(FileArrayResponse response);
	void printBool(BoolResponse response);
	void printFile(UploadResponse response);

	ListDirectoryCallback getPrintFilesCallback();

public:
	IdStorage();
	IdStorage(Common::String token, Common::String refreshToken, bool enabled);
	virtual ~IdStorage();

	/** Public Cloud API comes down there. */

	/** Returns StorageFile with the resolved file's id. */
	virtual Networking::Request *resolveFileId(Common::String path, UploadCallback callback, Networking::ErrorCallback errorCallback);

	/** Returns ListDirectoryStatus struct with list of files. */
	virtual Networking::Request *listDirectory(Common::String path, ListDirectoryCallback callback, Networking::ErrorCallback errorCallback, bool recursive = false);
	virtual Networking::Request *listDirectoryById(Common::String id, ListDirectoryCallback callback, Networking::ErrorCallback errorCallback) = 0;

	/** Calls the callback when finished. */
	virtual Networking::Request *createDirectory(Common::String path, BoolCallback callback, Networking::ErrorCallback errorCallback);
	virtual Networking::Request *createDirectoryWithParentId(Common::String parentId, Common::String name, BoolCallback callback, Networking::ErrorCallback errorCallback) = 0;

	/** Returns pointer to Networking::NetworkReadStream. */
	virtual Networking::Request *streamFile(Common::String path, Networking::NetworkReadStreamCallback callback, Networking::ErrorCallback errorCallback);
	virtual Networking::Request *streamFileById(Common::String id, Networking::NetworkReadStreamCallback callback, Networking::ErrorCallback errorCallback) = 0;

	/** Calls the callback when finished. */
	virtual Networking::Request *download(Common::String remotePath, Common::String localPath, BoolCallback callback, Networking::ErrorCallback errorCallback);

	virtual Common::String getRootDirectoryId() = 0;
};

} // End of namespace Id
} // End of namespace Cloud

#endif
