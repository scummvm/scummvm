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
	void printFiles(const FileArrayResponse &response);
	void printBool(const BoolResponse &response);
	void printFile(const UploadResponse &response);

	ListDirectoryCallback getPrintFilesCallback();

public:
	IdStorage();
	IdStorage(const Common::String &token, const Common::String &refreshToken, bool enabled);
	~IdStorage() override;

	/** Public Cloud API comes down there. */

	/** Returns StorageFile with the resolved file's id. */
	virtual Networking::Request *resolveFileId(const Common::String &path, UploadCallback callback, Networking::ErrorCallback errorCallback);

	/** Returns ListDirectoryStatus struct with list of files. */
	Networking::Request *listDirectory(const Common::String &path, ListDirectoryCallback callback, Networking::ErrorCallback errorCallback, bool recursive = false) override;
	virtual Networking::Request *listDirectoryById(const Common::String &id, ListDirectoryCallback callback, Networking::ErrorCallback errorCallback) = 0;

	/** Calls the callback when finished. */
	Networking::Request *createDirectory(const Common::String &path, BoolCallback callback, Networking::ErrorCallback errorCallback) override;
	virtual Networking::Request *createDirectoryWithParentId(const Common::String &parentId, const Common::String &name, BoolCallback callback, Networking::ErrorCallback errorCallback) = 0;

	/** Returns pointer to Networking::NetworkReadStream. */
	Networking::Request *streamFile(const Common::String &path, Networking::NetworkReadStreamCallback callback, Networking::ErrorCallback errorCallback) override;
	virtual Networking::Request *streamFileById(const Common::String &id, Networking::NetworkReadStreamCallback callback, Networking::ErrorCallback errorCallback) override = 0;

	/** Calls the callback when finished. */
	Networking::Request *download(const Common::String &remotePath, const Common::Path &localPath, BoolCallback callback, Networking::ErrorCallback errorCallback) override;

	virtual Common::String getRootDirectoryId() = 0;
};

} // End of namespace Id
} // End of namespace Cloud

#endif
