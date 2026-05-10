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

#include <mint/osbind.h>
#include <unistd.h>	// getcwd

AtariFilesystemFactory::AtariFilesystemFactory() {
	_fileHashMap["cryomni3.dat"] = "cryomni3d.dat";
	_fileHashMap["neverhoo.dat"] = "neverhood.dat";
	_fileHashMap["supernov.dat"] = "supernova.dat";
	_fileHashMap["teenagen.dat"] = "teenagent.dat";

	uint32 drvMap = Drvmap();
	for (int i = 0; i < 26; i++) {
		if (drvMap & 1) {
			char driveRoot[] = "A:";
			driveRoot[0] += i;

			addDrive(driveRoot);
		}

		drvMap >>= 1;
	}
}

AbstractFSNode *AtariFilesystemFactory::makeRootFileNode() const {
	return new AtariFilesystemNode(_config, _fileHashMap);
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
		return new AtariFilesystemNode(buf, _config, _fileHashMap);
	} else {
		return nullptr;
	}
}

AbstractFSNode *AtariFilesystemFactory::makeFileNodePath(const Common::String &path) const {
	assert(!path.empty());
	return new AtariFilesystemNode(path, _config, _fileHashMap);
}
