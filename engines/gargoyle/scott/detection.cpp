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
	{ "cbd47ab4fcfe00231ffd71d52378d410", 18482, "Pirate Adventure" },
	{ "9251ab2c64e63559d8a6e9e6246760a5", 17227, "Mission Impossible" },
	{ "be849c5747c7fc3b201984afb4403b8e", 18140, "Voodoo Castle" },
	{ "85b75b6079b5ee572b5259b29a0e5d21", 19999, "The Count" },
	{ "c423cae841ac1927b5b2e503607b21bc", 20115, "Strange Odyssey" },
	{ "326b98b991d401605074e64d474ce566", 19700, "Mystery Fun House" },
	{ "8ef9010399f055da9adb15ce7745a11c", 20320, "Pyramid Of Doom" },
	{ "fcdcca8b2acf76ba2d0006cefa3630a1", 20687, "Ghost Town" },
	{ "c8aaa80f07c40fa8e4b17432644919dc", 22669, "Save Island, Part 1" },
	{ "2add0f28d9b236c866890cdf8d86ee60", 21169, "Savage Island, Part 2" },
	{ "675126bd0477e8ed9230ad3db5afc45f", 21401, "The Golden Voyage" },
	{ "0ef0def798d895ed766041fa99dd28a0", 22346, "Adventure 13" },
	{ "0bf1bcc649422798332a38c88588fdff", 22087, "Adventure 14" },
	{ "a0a5423967287dae9cbeb9abe8324479", 21038, "Buckaroo Banzai" },
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
