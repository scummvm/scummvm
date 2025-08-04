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

#ifndef CLOUD_FILESYSTEM_H
#define CLOUD_FILESYSTEM_H

#include "backends/fs/abstract-fs.h"

#ifdef USE_CLOUD
#include "backends/cloud/cloudmanager.h"
#include "backends/cloud/storage.h"
#include "backends/cloud/storagefile.h"
#include "backends/networking/http/connectionmanager.h"
#include "backends/networking/http/httpjsonrequest.h"
#include "backends/networking/http/request.h"
#endif

#include "backends/fs/posix/posix-fs.h"

/**
 * Implementation of the ScummVM file system API based on POSIX.
 *
 * Parts of this class are documented in the base interface class, AbstractFSNode.
 */
class CloudFilesystemNode : public AbstractFSNode {
#define CLOUD_FS_PATH "/cloud"
protected:
	static Common::HashMap<Common::String, AbstractFSList> _cloudFolders;
	Common::String _displayName;
	Common::String _path;
	Common::String _storageFileId;
	bool _isDirectory;
	bool _isValid;

	/**
	 * Plain constructor, for internal use only (hence protected).
	 */
	CloudFilesystemNode() : _isDirectory(false), _isValid(false), _storageFileId(nullptr) {}

	virtual AbstractFSNode *makeNode(const Common::String &path) const {
		return new CloudFilesystemNode(path);
	}

	/**
	 * Callbacks for network calls (file download and directory listing)
	 */
	void directoryListedCallback(const Cloud::Storage::ListDirectoryResponse &response);
	void directoryListedErrorCallback(const Networking::ErrorResponse &error);
	void fileDownloadedCallback(const Cloud::Storage::BoolResponse &response);
	void fileDownloadedErrorCallback(const Networking::ErrorResponse &error);

public:
	/**
	 * Creates a CloudFilesystemNode for a given path.
	 *
	 * @param path the path the new node should point to.
	 */
	CloudFilesystemNode(const Common::String &path);

	bool exists() const override;
	Common::U32String getDisplayName() const override { return _displayName; }
	Common::String getName() const override { return _displayName; }
	Common::String getPath() const override { return _path; }
	bool isDirectory() const override { return _isDirectory; }
	bool isReadable() const override;
	bool isWritable() const override;

	AbstractFSNode *getChild(const Common::String &n) const override;
	bool getChildren(AbstractFSList &list, ListMode mode, bool hidden) const override;
	AbstractFSNode *getParent() const override;

	Common::SeekableReadStream *createReadStream() override;
	Common::SeekableWriteStream *createWriteStream(bool atomic) override;
	bool createDirectory() override;
};

#endif
