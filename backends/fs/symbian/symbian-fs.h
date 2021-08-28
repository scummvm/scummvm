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

#ifndef SYMBIAN_FILESYSTEM_H
#define SYMBIAN_FILESYSTEM_H

#include "backends/fs/abstract-fs.h"

/**
 * Implementation of the ScummVM file system API based on POSIX.
 *
 * Parts of this class are documented in the base interface class, AbstractFSNode.
 */
class SymbianFilesystemNode final : public AbstractFSNode {
protected:
	Common::String _displayName;
	Common::String _path;
	bool _isDirectory;
	bool _isValid;
	bool _isPseudoRoot;
public:
	/**
	 * Creates a SymbianFilesystemNode with the root node as path.
	 *
	 * @param aIsRoot true if the node will be a pseudo root, false otherwise.
	 */
	SymbianFilesystemNode(bool aIsRoot);

	/**
	 * Creates a SymbianFilesystemNode for a given path.
	 *
	 * @param path Common::String with the path the new node should point to.
	 */
	SymbianFilesystemNode(const Common::String &path);

	virtual bool exists() const override;
	virtual Common::U32String getDisplayName() const override { return _displayName; }
	virtual Common::String getName() const override { return _displayName; }
	virtual Common::String getPath() const override { return _path; }
	virtual bool isDirectory() const override { return _isDirectory; }
	virtual bool isReadable() const override;
	virtual bool isWritable() const override;

	virtual AbstractFSNode *getChild(const Common::String &n) const override;
	virtual bool getChildren(AbstractFSList &list, ListMode mode, bool hidden) const override;
	virtual AbstractFSNode *getParent() const override;

	virtual Common::SeekableReadStream *createReadStream() override;
	virtual Common::SeekableWriteStream *createWriteStream() override;
	virtual bool createDirectory() override;
};

#endif
