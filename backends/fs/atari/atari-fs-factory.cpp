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

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "backends/fs/atari/atari-fs-factory.h"
#include "backends/fs/atari/atari-fs.h"

#include <unistd.h>	// getcwd

#if defined(ATARI)

AbstractFSNode *AtariFilesystemFactory::makeRootFileNode() const {
	return new AtariFilesystemNode("/");
}

AbstractFSNode *AtariFilesystemFactory::makeCurrentDirectoryFileNode() const {
	char buf[MAXPATHLEN];
	if (getcwd(buf, MAXPATHLEN) != NULL) {
		if (buf[0] == '/') {
			// de-mintlib'ize the path
			if (buf[1] == 'd' && buf[2] == 'e' && buf[3] == 'v') {
				// /dev/<drive>/<path> -> /<drive>/<path>
				strcpy(buf, &buf[4]);
			}
			// /<drive>/<path> -> <DRIVE>:/<path>
			buf[0] = toupper(buf[1]);
			buf[1] = ':';
		}
		return new AtariFilesystemNode(buf);
	} else {
		return nullptr;
	}
}

AbstractFSNode *AtariFilesystemFactory::makeFileNodePath(const Common::String &path) const {
	assert(!path.empty());
	return new AtariFilesystemNode(path);
}

#endif	// ATARI
