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

#include "config.h"
#include "common/scummsys.h"
#include "common/stream.h"
//#include "common/str.h"
#include "backends/fs/abstract-fs.h"

#include <FBaseString.h>
#include <FBaseUtilStringUtil.h>
#include <FIoDirectory.h>
#include <FIoFile.h>

using namespace Osp::Io;
using namespace Osp::Base;
using namespace Osp::Base::Utility;

/**
 * Implementation of the ScummVM file system API based on BADA.
 *
 * Parts of this class are documented in the base interface class, AbstractFSNode.
 */
class BadaFilesystemNode : public AbstractFSNode {
public:
	/**
	 * Creates a BadaFilesystemNode for a given path.
	 *
	 * @param path the path the new node should point to.
	 */
	BadaFilesystemNode(const Common::String &path);

	Common::String getDisplayName() const { return displayName; }
	Common::String getName() const { return displayName; }
	Common::String getPath() const { return path; }

	bool exists() const;
	bool isDirectory() const;
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
	BadaFilesystemNode() : isValid(false) {}

  AbstractFSNode *makeNode(const Common::String &path) const {
		return new BadaFilesystemNode(path);
	}

	Common::String displayName;
	Common::String path;
  String unicodePath;
	bool isValid;
  FileAttributes attr;
};

#endif
