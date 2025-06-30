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

#ifndef PSP2_FILESYSTEM_H
#define PSP2_FILESYSTEM_H

#include "backends/fs/abstract-fs.h"

/**
 * Implementation of the ScummVM file system API based on the VitaSDK API.
 *
 * Parts of this class are documented in the base interface class, AbstractFSNode.
 */
class PSP2FilesystemNode : public AbstractFSNode {
protected:
	Common::String _displayName;
	Common::String _path;
	bool _isDirectory;
	bool _isValid;

	virtual AbstractFSNode *makeNode() const {
		return new PSP2FilesystemNode(_config);
	}
	AbstractFSNode *makeNode(const Common::String &path) const {
		return new PSP2FilesystemNode(path, _config, true);
	}

public:
	struct Config {
		Config() { }
		virtual ~Config() { }

		virtual bool getDrives(AbstractFSList &list, bool hidden) const = 0;
		virtual bool isDrive(const Common::String &path) const = 0;
	};

	/**
	 * Creates a PSP2FilesystemNode with the root node as path.
	 * 
	 * @param path Common::String with the path the new node should point to.
	 * @param config Config of the filesystem.
	 * @param verify true if the isValid and isDirectory flags should be verified during the construction.
	 */
	PSP2FilesystemNode(const Common::String &path, const Config &config, bool verify);
	PSP2FilesystemNode(const Config &config);

	virtual bool exists() const;
	virtual Common::U32String getDisplayName() const { return _displayName; }
	virtual Common::String getName() const { return _displayName; }
	virtual Common::String getPath() const { return _path; }
	virtual bool isDirectory() const { return _isDirectory; }
	virtual bool isReadable() const;
	virtual bool isWritable() const;

	virtual AbstractFSNode *getChild(const Common::String &n) const;
	virtual bool getChildren(AbstractFSList &list, ListMode mode, bool hidden) const;
	virtual AbstractFSNode *getParent() const;

	virtual Common::SeekableReadStream *createReadStream();
	virtual Common::SeekableWriteStream *createWriteStream(bool atomic);
	virtual bool createDirectory();

protected:
	const Config &_config;

private:
	bool _isPseudoRoot;
	bool isDrive(const Common::String &path) const;
};

#endif
