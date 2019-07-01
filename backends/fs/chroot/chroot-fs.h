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

#ifndef BACKENDS_FS_CHROOT_CHROOT_FS_H
#define BACKENDS_FS_CHROOT_CHROOT_FS_H

#include "backends/fs/posix/posix-fs.h"

class ChRootFilesystemNode : public AbstractFSNode {
	Common::String _root;
	POSIXFilesystemNode *_realNode;

	ChRootFilesystemNode(const Common::String &root, POSIXFilesystemNode *);

public:
	ChRootFilesystemNode(const Common::String &root, const Common::String &path);
	virtual ~ChRootFilesystemNode();

	virtual bool exists() const;
	virtual Common::String getDisplayName() const;
	virtual Common::String getName() const;
	virtual Common::String getPath() const;
	virtual bool isDirectory() const;
	virtual bool isReadable() const;
	virtual bool isWritable() const;

	virtual AbstractFSNode *getChild(const Common::String &n) const;
	virtual bool getChildren(AbstractFSList &list, ListMode mode, bool hidden) const;
	virtual AbstractFSNode *getParent() const;

	virtual Common::SeekableReadStream *createReadStream();
	virtual Common::WriteStream *createWriteStream();
	virtual bool createDirectory();

private:
	static Common::String addPathComponent(const Common::String &path, const Common::String &component);
};

#endif /* BACKENDS_FS_CHROOT_CHROOT_FS_H */
