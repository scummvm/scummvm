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

#ifndef POSIX_DRIVES_FILESYSTEM_FACTORY_H
#define POSIX_DRIVES_FILESYSTEM_FACTORY_H

#include "backends/fs/fs-factory.h"

/**
 * A FilesystemFactory implementation for filesystems with a special
 * top-level directory with hard-coded entries but that otherwise
 * implement the POSIX APIs.
 *
 * For used with paths like these:
 * - 'sdcard:/games/scummvm.ini'
 * - 'hdd1:/usr/bin'
 */
class DrivesPOSIXFilesystemFactory : public FilesystemFactory {
public:
	/**
	 * Add a drive to the top-level directory
	 */
	void addDrive(const Common::String &name);

protected:
	// FilesystemFactory API
	AbstractFSNode *makeRootFileNode() const override;
	AbstractFSNode *makeCurrentDirectoryFileNode() const override;
	AbstractFSNode *makeFileNodePath(const Common::String &path) const override;

private:
	Common::Array<Common::String> _drives;
};

#endif
