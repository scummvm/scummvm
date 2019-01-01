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
#include "glk/scott/detection_tables.h"
#include "common/file.h"
#include "common/md5.h"
#include "engines/game.h"

namespace Glk {
namespace Scott {

void ScottMetaEngine::getSupportedGames(PlainGameList &games) {
	for (const PlainGameDescriptor *pd = SCOTT_GAME_LIST; pd->gameId; ++pd)
		games.push_back(*pd);
}

GameDescriptor ScottMetaEngine::findGame(const char *gameId) {
	for (const PlainGameDescriptor *pd = SCOTT_GAME_LIST; pd->gameId; ++pd) {
		if (!strcmp(gameId, pd->gameId))
			return *pd;
	}

	return GameDescriptor::empty();
}

bool ScottMetaEngine::detectGames(const Common::FSList &fslist, DetectedGames &gameList) {
	const char *const EXTENSIONS[] = { ".saga", ".dat", ".blb", ".blorb", nullptr };
	Common::File gameFile;
	Common::String md5;

	// Loop through the files of the folder
	for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		Common::String name = file->getName();
		if (file->isDirectory())
			continue;

		Common::String filename = file->getName();
		bool hasExt = false;
		for (const char *const *ext = &EXTENSIONS[0]; *ext && !hasExt; ++ext)
			hasExt = filename.hasSuffixIgnoreCase(*ext);
		if (!hasExt)
			continue;

		if (gameFile.open(*file)) {
			md5 = Common::computeStreamMD5AsString(gameFile, 5000);

			// Scan through the Scott game list for a match
			const ScottGame *p = SCOTT_GAMES;
			while (p->_md5 && p->_filesize != gameFile.size() && md5 != p->_md5)
				++p;

			if (p->_filesize) {
				// Found a match
				PlainGameDescriptor gameDesc = findGame(p->_gameId);
				DetectedGame gd(p->_gameId, gameDesc.description, Common::EN_ANY, Common::kPlatformUnknown);
				gd.addExtraEntry("filename", file->getName());

				gameList.push_back(gd);
			}

			gameFile.close();
		}
	}

	return !gameList.empty();
}

void ScottMetaEngine::detectClashes(Common::StringMap &map) {
	for (const PlainGameDescriptor *pd = SCOTT_GAME_LIST; pd->gameId; ++pd) {
		if (map.contains(pd->gameId))
			error("Duplicate game Id found - %s", pd->gameId);
		map[pd->gameId] = "";
	}
}

} // End of namespace Scott
} // End of namespace Glk
