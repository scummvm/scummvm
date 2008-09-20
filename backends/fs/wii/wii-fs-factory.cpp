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

#if defined(__WII__)

#include <unistd.h>

#include "backends/fs/wii/wii-fs-factory.h"
#include "backends/fs/wii/wii-fs.cpp"

DECLARE_SINGLETON(WiiFilesystemFactory);

AbstractFilesystemNode *WiiFilesystemFactory::makeRootFileNode() const {
	return new WiiFilesystemNode();
}

AbstractFilesystemNode *WiiFilesystemFactory::makeCurrentDirectoryFileNode() const {
	char buf[MAXPATHLEN];

	if (getcwd(buf, MAXPATHLEN))
		return new WiiFilesystemNode(buf, true);
	else
		return new WiiFilesystemNode();
}

AbstractFilesystemNode *WiiFilesystemFactory::makeFileNodePath(const Common::String &path) const {
	return new WiiFilesystemNode(path, true);
}
#endif

