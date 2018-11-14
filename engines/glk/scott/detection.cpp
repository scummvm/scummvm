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

#include "glk/scott/detection.h"
#include "common/file.h"
#include "common/md5.h"

namespace Glk {
namespace Scott {

struct ScottGame {
	const char *_md5;
	const char *_gameId;
	int32 _filesize;
	const char *_desc;
};

const ScottGame SCOTT_GAMES[] = {
	// PC game versions
	{ "7c6f495d757a54e73d259efc718d8024", "adventureland",     15896, "Adventureland" },
	{ "ea535fa7684508410151b4561de1f323", "pirateadventure",   16325, "Pirate Adventure" },
	{ "379c77a9a483886366b3b5c425e56410", "missionimpossible", 15275, "Mission Impossible" },
	{ "a530a6857d1092eaa177eee575c94c71", "voodoocastle",      15852, "Voodoo Castle" },
	{ "5ebb4ade985670bb2eac54f8fa202214", "thecount",          17476, "The Count" },
	{ "c57bb6df04dc77a2b232bc5bcab6e417", "strangeodyssey",    17489, "Strange Odyssey" },
	{ "ce2931ac3d5cbc270a5cb7be9e614f6e", "mysteryfunhouse",   17165, "Mystery Fun House" },
	{ "4e6127fad6b5d75eccd3f3b101f8c9c8", "pyramidofdoom",     17673, "Pyramid Of Doom" },
	{ "2c08327ab06d5490bd9e367ddaeca627", "ghosttown",         17831, "Ghost Town" },
	{ "8feb77f11d32e9567ce2fc7d435eaf44", "savageisland1",     19533, "Savage Island, Part 1" },
	{ "20c40a349f7a214ac515fb1d63c30a87", "savageisland2",     18367, "Savage Island, Part 2" },
	{ "e2a8f956ab215012d1495550c4c11ee8", "goldenvoyage",      18513, "The Golden Voyage" },
	{ "f986d7e1ee074f65b6c1d00461c9b3c3", "adventure13",       19232, "Adventure 13" },
	{ "6d98f422cc986d959a3c74351785aea3", "adventure14",       19013, "Adventure 14" },
	{ "aadcc04e6b37eb9d30a58b5bc775842e", "marveladventure",   18876, "Marvel Adventure #1" },
	{ "d569a769f304dc02b3062d97458ddd01", "scottsampler",      13854, "Adventure International's Mini-Adventure Sampler" },

	// PDA game versions
	{ "ae541fc1085da2f7d561b72ed20a6bc1", "adventureland", 18003, "Adventureland" },
	{ "cbd47ab4fcfe00231ffd71d52378d410", "pirateadventure", 18482, "Pirate Adventure" },
	{ "9251ab2c64e63559d8a6e9e6246760a5", "missionimpossible", 17227, "Mission Impossible" },
	{ "be849c5747c7fc3b201984afb4403b8e", "voodoocastle", 18140, "Voodoo Castle" },
	{ "85b75b6079b5ee572b5259b29a0e5d21", "thecount", 19999, "The Count" },
	{ "c423cae841ac1927b5b2e503607b21bc", "strangeodyssey", 20115, "Strange Odyssey" },
	{ "326b98b991d401605074e64d474ce566", "mysteryfunhouse", 19700, "Mystery Fun House" },
	{ "8ef9010399f055da9adb15ce7745a11c", "pyramidofdoom", 20320, "Pyramid Of Doom" },
	{ "fcdcca8b2acf76ba2d0006cefa3630a1", "ghosttown", 20687, "Ghost Town" },
	{ "c8aaa80f07c40fa8e4b17432644919dc", "savageisland1", 22669, "Savage Island, Part 1" },
	{ "2add0f28d9b236c866890cdf8d86ee60", "savageisland2", 21169, "Savage Island, Part 2" },
	{ "675126bd0477e8ed9230ad3db5afc45f", "goldenvoyage", 21401, "The Golden Voyage" },
	{ "0ef0def798d895ed766041fa99dd28a0", "adventure13", 22346, "Adventure 13" },
	{ "0bf1bcc649422798332a38c88588fdff", "adventure14", 22087, "Adventure 14" },
	{ "a0a5423967287dae9cbeb9abe8324479", "buckaroobonzai", 21038, "Buckaroo Banzai" },
	{ nullptr, nullptr, 0, nullptr }
};

bool ScottMetaEngine::detectGames(const Common::FSList &fslist, DetectedGames &gameList) {
	Common::File gameFile;
	Common::String md5;

	// Loop through the files of the folder
	for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		if (file->isDirectory() || !(file->getName().hasSuffixIgnoreCase(".saga")
				|| file->getName().hasSuffixIgnoreCase(".dat")))
			continue;

		if (gameFile.open(*file)) {
			md5 = Common::computeStreamMD5AsString(gameFile, 5000);

			// Scan through the Scott game list for a match
			const ScottGame *p = SCOTT_GAMES;
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

} // End of namespace Scott
} // End of namespace Glk
