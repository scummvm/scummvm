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

#if defined(POSIX) || defined(PSP2) || defined(__DS__)

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "backends/fs/posix-drives/posix-drives-fs-factory.h"
#include "backends/fs/posix-drives/posix-drives-fs.h"

#include <unistd.h>

void DrivesPOSIXFilesystemFactory::addDrive(const Common::String &name) {
	_config.drives.push_back(Common::normalizePath(name, '/'));
}

void DrivesPOSIXFilesystemFactory::configureBuffering(DrivePOSIXFilesystemNode::BufferingMode bufferingMode, uint32 bufferSize) {
	_config.bufferingMode = bufferingMode;
	_config.bufferSize = bufferSize;
}

AbstractFSNode *DrivesPOSIXFilesystemFactory::makeRootFileNode() const {
	return new DrivePOSIXFilesystemNode(_config);
}

AbstractFSNode *DrivesPOSIXFilesystemFactory::makeCurrentDirectoryFileNode() const {
#if defined(PSP2) // The Vita does not have getcwd
	return makeRootFileNode();
#else
	char buf[MAXPATHLEN];
	return getcwd(buf, MAXPATHLEN) ? new DrivePOSIXFilesystemNode(buf, _config) : makeRootFileNode();
#endif
}

AbstractFSNode *DrivesPOSIXFilesystemFactory::makeFileNodePath(const Common::String &path) const {
	assert(!path.empty());
	return new DrivePOSIXFilesystemNode(path, _config);
}

#endif
