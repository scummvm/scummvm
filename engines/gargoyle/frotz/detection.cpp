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

#include "gargoyle/frotz/detection.h"
#include "common/file.h"
#include "common/md5.h"

namespace Gargoyle {
namespace Frotz {

struct FrotzGame {
	const char *_md5;
	const char *_gameId;
	int32 _filesize;
	const char *_desc;
};

const FrotzGame FROTZ_GAMES[] = {
	{ nullptr, nullptr, 0, nullptr }
};

bool FrotzMetaEngine::detectGames(const Common::FSList &fslist, DetectedGames &gameList) {
	Common::File gameFile;
	Common::String md5;
	const char *const EXTENSIONS[9] = { ".z1", ".z2", ".z3", ".z4", ".z5", ".z6", ".z7", ".z8", ".zblorb" };

	// Loop through the files of the folder
	for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		// Check for a recognised filename
		if (file->isDirectory())
			continue;
		Common::String filename = file->getName();
		bool hasExt = false;
		for (int idx = 0; idx < 9 && !hasExt; ++idx)
			hasExt = filename.hasSuffixIgnoreCase(EXTENSIONS[idx]);
		if (!hasExt)
			continue;

		// Check for known game
		if (gameFile.open(*file)) {
			md5 = Common::computeStreamMD5AsString(gameFile, 5000);

			// Scan through the game list for a match
			const FrotzGame *p = FROTZ_GAMES;
			while (p->_md5 && p->_filesize != gameFile.size() && md5 != p->_md5)
				++p;

			if (p->_filesize) {
				// Found a match
				DetectedGame gd(p->_gameId, p->_desc, Common::EN_ANY, Common::kPlatformUnknown);
				gd.addExtraEntry("filename", file->getName());

				gameList.push_back(gd);
			}

			gameFile.close();
		}
	}

	return !gameList.empty();
}

} // End of namespace Frotz
} // End of namespace Gargoyle
