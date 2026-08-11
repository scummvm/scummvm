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

#ifndef WII_FILESYSTEM_H
#define WII_FILESYSTEM_H

#include "backends/fs/abstract-fs.h"

/**
 * Implementation of the ScummVM file system API based on Wii.
 *
 * Parts of this class are documented in the base interface class, AbstractFSNode.
 */
class WiiFilesystemNode final : public AbstractFSNode {
protected:
	Common::String _displayName;
	Common::String _path;
	bool _exists, _isDirectory, _isReadable, _isWritable;

	virtual void initRootNode();
	virtual bool getDevopChildren(AbstractFSList &list, ListMode mode, bool hidden) const;
	virtual void setFlags(const struct stat *st);
	virtual void clearFlags();

public:
	/**
	 * Creates a WiiFilesystemNode with the root node as path.
	 */
	WiiFilesystemNode();

	/**
	 * Creates a WiiFilesystemNode for a given path.
	 *
	 * @param path Common::String with the path the new node should point to.
	 */
	WiiFilesystemNode(const Common::String &p);
	WiiFilesystemNode(const Common::String &p, const struct stat *st);

	bool exists() const override;
	Common::U32String getDisplayName() const override { return _displayName; }
	Common::String getName() const override { return _displayName; }
	Common::String getPath() const override { return _path; }
	bool isDirectory() const override { return _isDirectory; }
	bool isReadable() const override { return _isReadable; }
	bool isWritable() const override { return _isWritable; }

	AbstractFSNode *getChild(const Common::String &n) const override;
	bool getChildren(AbstractFSList &list, ListMode mode, bool hidden) const override;
	AbstractFSNode *getParent() const override;

	Common::SeekableReadStream *createReadStream() override;
	Common::SeekableWriteStream *createWriteStream(bool atomic) override;
	bool createDirectory() override;
};

#endif
