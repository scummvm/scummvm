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

#ifndef POSIX_DRIVES_FILESYSTEM_FACTORY_H
#define POSIX_DRIVES_FILESYSTEM_FACTORY_H

#include "backends/fs/fs-factory.h"
#include "backends/fs/posix-drives/posix-drives-fs.h"

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
	DrivesPOSIXFilesystemFactory() : _config(this) { }

	/**
	 * Add a drive to the top-level directory
	 */
	void addDrive(const Common::String &name);

	/**
	 * Configure file stream buffering
	 *
	 * @param bufferingMode select the buffering implementation to use
	 * @param bufferSize the size of the IO buffer in bytes. A buffer size of 0 means the default size should be used
	 */
	void configureBuffering(DrivePOSIXFilesystemNode::BufferingMode bufferingMode, uint32 bufferSize);

protected:
	// FilesystemFactory API
	AbstractFSNode *makeRootFileNode() const override;
	AbstractFSNode *makeCurrentDirectoryFileNode() const override;
	AbstractFSNode *makeFileNodePath(const Common::String &path) const override;

private:
	typedef Common::Array<Common::String> DrivesArray;
	struct StaticDrivesConfig : public DrivePOSIXFilesystemNode::Config {
		StaticDrivesConfig(const DrivesPOSIXFilesystemFactory *factory) : _factory(factory) { }

		bool getDrives(AbstractFSList &list, bool hidden) const override;
		bool isDrive(const Common::String &path) const override;

		DrivesArray drives;

	private:
		const DrivesPOSIXFilesystemFactory *_factory;
	};

	StaticDrivesConfig _config;
};

#endif
