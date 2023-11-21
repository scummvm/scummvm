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

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/zip-set.h"
#include "common/compression/unzip.h"

namespace Common {

struct ArchiveMemberListBackComparator {
	bool operator()(const ArchiveMemberPtr &a, const ArchiveMemberPtr &b) {
		return a->getName() > b->getName();
	}
};

bool generateZipSet(SearchSet &searchSet, const char *defaultFile, const char *packsMask, const char *packsPath) {
	Archive *dat;
	bool changed = false;

	if (!ConfMan.get(packsPath).empty()) {
		FSDirectory *iconDir = new FSDirectory(ConfMan.get(packsPath));
		ArchiveMemberList iconFiles;

		iconDir->listMatchingMembers(iconFiles, packsMask);
		sort(iconFiles.begin(), iconFiles.end(), ArchiveMemberListBackComparator());

		for (ArchiveMemberList::iterator ic = iconFiles.begin(); ic != iconFiles.end(); ++ic) {
			dat = makeZipArchive((*ic)->createReadStream());

			if (dat) {
				searchSet.add((*ic)->getName(), dat);
				changed = true;
				debug(2, "generateZipSet: Loaded pack file: %s", (*ic)->getName().c_str());
			}
		}

		delete iconDir;
	}

	dat = nullptr;

	if (ConfMan.hasKey("themepath")) {
		FSNode *fs = new FSNode(normalizePath(ConfMan.get("themepath") + "/" + defaultFile, '/'));
		if (fs->exists()) {
			dat = makeZipArchive(*fs);
		}
		delete fs;
	}

	if (!dat) {
		File *file = new File;
		if (ConfMan.hasKey(packsPath)) {
			String path(normalizePath(ConfMan.get(packsPath) + "/" + defaultFile, '/'));

			if (File::exists(path))
				file->open(path);
		}

		if (!file->isOpen())
			if (File::exists(defaultFile))
				file->open(defaultFile);

		if (file->isOpen())
			dat = makeZipArchive(defaultFile);

		if (!dat) {
			warning("generateZipSet: Could not find '%s'", defaultFile);
		}
		delete file;
	}

	if (dat) {
		searchSet.add(defaultFile, dat);
		changed = true;
		debug(2, "generateZipSet: Loaded pack file: %s", defaultFile);
	}

	return changed;
}

} // End of namespace Common
