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

#if defined(PSP2)

#define FORBIDDEN_SYMBOL_EXCEPTION_unistd_h

#include "backends/fs/psp2/psp2-fs-factory.h"
#include "backends/fs/psp2/psp2-fs.h"
#include "backends/platform/sdl/psp2/powerman.h"

#include <unistd.h>


void PSP2FilesystemFactory::addDrive(const Common::String &name) {
	_config.drives.push_back(Common::normalizePath(name, '/'));
}

AbstractFSNode *PSP2FilesystemFactory::makeRootFileNode() const {
	return new PSP2FilesystemNode(_config);
}

AbstractFSNode *PSP2FilesystemFactory::makeCurrentDirectoryFileNode() const {
	char buf[MAXPATHLEN];
	char *ret = 0;

	PowerMan.beginCriticalSection();
	ret = getcwd(buf, MAXPATHLEN);
	PowerMan.endCriticalSection();

	return (ret ? new PSP2FilesystemNode(buf, _config, true) : NULL);
}

AbstractFSNode *PSP2FilesystemFactory::makeFileNodePath(const Common::String &path) const {
	return new PSP2FilesystemNode(path, _config, true);
}

bool PSP2FilesystemFactory::StaticDrivesConfig::getDrives(AbstractFSList &list, bool hidden) const {
	for (uint i = 0; i < drives.size(); i++) {
		list.push_back(_factory->makeFileNodePath(drives[i]));
	}
	return true;
}

bool PSP2FilesystemFactory::StaticDrivesConfig::isDrive(const Common::String &path) const {
	DrivesArray::const_iterator drive = Common::find(drives.begin(), drives.end(), path);
	return drive != drives.end();
}

#endif
