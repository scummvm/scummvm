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
#include "base/plugins.h"

#include <mint/osbind.h>
#include <unistd.h>	// getcwd

// all engines enabled in this build, static and dynamic
#undef PLUGIN_ENABLED_STATIC
#define PLUGIN_ENABLED_STATIC(ID) PLUGIN_ENABLED(ID)
#define LINK_PLUGIN(ID) #ID,
static const char *const engineIds[] = {
#include "engines/plugins_table.h"
	nullptr
};
#undef LINK_PLUGIN

// maps a name shortened by GEMDOS to 8+3 back to the original one, e.g. "parallac.plg" -> "parallaction.plg"
static void addLongName(AtariFilesystemNode::FileHashMap &fileHashMap, const Common::String &name) {
	const size_t dot = name.findLastOf('.');
	const Common::String shortName = name.substr(0, MIN<size_t>(dot, 8)) + name.substr(dot, 4);

	if (shortName != name)
		fileHashMap[shortName] = name;
}

AtariFilesystemFactory::AtariFilesystemFactory() {
	static const char *const otherNames[] = {
		"detection.plg",
		"fonts-cjk.dat",
		"gui-icons.dat",
		"residualvm.zip",
		"scummclassic.zip",
		"scummmodern.zip",
		"scummremastered.zip"
	};
	for (const char *name : otherNames)
		addLongName(_fileHashMap, name);

	for (const char *const *engineId = engineIds; *engineId; ++engineId) {
		Common::String name(*engineId);
		name.toLowercase();

		addLongName(_fileHashMap, name + ".plg");
		addLongName(_fileHashMap, name + ".dat");
		addLongName(_fileHashMap, name + ".zip");
	}

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
