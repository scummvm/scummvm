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
	bool operator()(const Common::ArchiveMemberPtr &a, const Common::ArchiveMemberPtr &b) {
		return a->getName() > b->getName();
	}
};

bool generateZipSet(Common::SearchSet &searchSet, const char *defaultFile, const char *packsMask, const char *packsPath) {
	Common::Archive *dat;
	bool changed = false;

	if (!ConfMan.get(packsPath).empty()) {
		Common::FSDirectory *iconDir = new Common::FSDirectory(ConfMan.get(packsPath));
		Common::ArchiveMemberList iconFiles;

		iconDir->listMatchingMembers(iconFiles, packsMask);
		Common::sort(iconFiles.begin(), iconFiles.end(), ArchiveMemberListBackComparator());

		for (Common::ArchiveMemberList::iterator ic = iconFiles.begin(); ic != iconFiles.end(); ++ic) {
			dat = Common::makeZipArchive((*ic)->createReadStream());

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
		Common::FSNode *fs = new Common::FSNode(normalizePath(ConfMan.get("themepath") + "/" + defaultFile, '/'));
		if (fs->exists()) {
			dat = Common::makeZipArchive(*fs);
		}
		delete fs;
	}

	if (!dat) {
		Common::File *file = new Common::File;
		if (ConfMan.hasKey(packsPath))
			file->open(normalizePath(ConfMan.get(packsPath) + "/" + defaultFile, '/'));

		if (!file->isOpen())
			file->open(defaultFile);

		if (file->isOpen())
			dat = Common::makeZipArchive(defaultFile);

		if (!dat) {
			warning("generateZipSet: Could not find '%s'", defaultFile);
			delete file;
		}
	}

	if (dat) {
		searchSet.add(defaultFile, dat);
		changed = true;
		debug(2, "generateZipSet: Loaded pack file: %s", defaultFile);
	}

	return changed;
}

} // End of namespace Common
