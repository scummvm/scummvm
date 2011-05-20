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
 */

#ifndef BADA_FILESYSTEM_H
#define BADA_FILESYSTEM_H

#include "common/scummsys.h"
#include "backends/fs/abstract-fs.h"

#include <FBaseString.h>
#include <FIoDirectory.h>
#include <FIoFile.h>

using namespace Osp::Io;

/**
 * Implementation of the ScummVM file system API based on BADA.
 *
 * Parts of this class are documented in the base interface class, AbstractFSNode.
 */
class BADAFilesystemNode : public AbstractFSNode {
public:
	/**
	 * Creates a BADAFilesystemNode for a given path.
	 *
	 * @param path the path the new node should point to.
	 */
	BADAFilesystemNode(const Common::String &path);

	Common::String getDisplayName() const { return displayName; }
	Common::String getName() const { return displayName; }
	Common::String getPath() const { return path; }

	bool exists() const;
	bool isDirectory() const { return isDir; }
	bool isReadable() const;
	bool isWritable() const;

	AbstractFSNode *getChild(const Common::String &n) const;
	bool getChildren(AbstractFSList &list, ListMode mode, bool hidden) const;
	AbstractFSNode *getParent() const;

	Common::SeekableReadStream *createReadStream();
	Common::WriteStream *createWriteStream();

protected:
	/**
	 * Plain constructor, for internal use only (hence protected).
	 */
	BADAFilesystemNode() : isDir(false), isValid(false) {}

  AbstractFSNode *makeNode(const Common::String &path) const {
		return new BADAFilesystemNode(path);
	}

	Common::String displayName;
	Common::String path;
	bool isDir;
	bool isValid;
  FileAttributes attr;
};

#endif
