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

#ifndef POSIX_DRIVES_FILESYSTEM_H
#define POSIX_DRIVES_FILESYSTEM_H

#include "backends/fs/posix/posix-fs.h"

/**
 * POSIX file system node where the top-level directory is a hardcoded
 * list of drives.
 */
class DrivePOSIXFilesystemNode : public POSIXFilesystemNode {
protected:
	AbstractFSNode *makeNode(const Common::String &path) const override {
		return new DrivePOSIXFilesystemNode(path, _drives);
	}

public:
	typedef Common::Array<Common::String> DrivesArray;

	DrivePOSIXFilesystemNode(const Common::String &path, const DrivesArray &drives);
	DrivePOSIXFilesystemNode(const DrivesArray &drives);

	// POSIXFilesystemNode API
	AbstractFSNode *getChild(const Common::String &n) const override;
	bool getChildren(AbstractFSList &list, ListMode mode, bool hidden) const override;
	AbstractFSNode *getParent() const override;

private:
	bool _isPseudoRoot;
	const DrivesArray &_drives;

	DrivePOSIXFilesystemNode *getChildWithKnownType(const Common::String &n, bool isDirectory) const;
	bool isDrive(const Common::String &path) const;
};

#endif
