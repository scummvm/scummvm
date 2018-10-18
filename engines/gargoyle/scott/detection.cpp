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

#include "gargoyle/scott/detection.h"
#include "common/file.h"
#include "common/md5.h"

namespace Gargoyle {
namespace Scott {

struct ScottGame {
	const char *_md5;
	int32 _filesize;
	const char *_desc;
};

const ScottGame SCOTT_GAMES[] = {
	{ "ae541fc1085da2f7d561b72ed20a6bc1", 18003, "Adventureland" },
	{ nullptr, 0, nullptr }
};

void ScottMetaEngine::detectGames(const Common::FSList &fslist, DetectedGames &gameList) {
	Common::File gameFile;
	Common::String md5;

	// Loop through the files of the folder
	for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		if (file->isDirectory() || !file->getName().hasSuffix(".saga"))
			continue;

		if (gameFile.open(*file)) {
			md5 = Common::computeStreamMD5AsString(gameFile, 5000);

			// Scan through the Scott game list for a match
			const ScottGame *p = SCOTT_GAMES;
			while (p->_md5 && p->_filesize != gameFile.size() && md5 != p->_md5)
				++p;

			if (p->_filesize) {
				// Found a match
				DetectedGame gd("scott", p->_desc, Common::EN_ANY, Common::kPlatformUnknown, "Scott");
				gd.addExtraEntry("filename", file->getName());

				gameList.push_back(gd);
			}

			gameFile.close();
		}
	}
}

} // End of namespace Scott
} // End of namespace Gargoyle
