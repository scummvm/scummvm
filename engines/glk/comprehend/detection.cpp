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

#include "glk/comprehend/detection.h"
#include "glk/comprehend/detection_tables.h"
#include "glk/blorb.h"
#include "common/file.h"
#include "common/md5.h"
#include "engines/game.h"

namespace Glk {
namespace Comprehend {

void ComprehendMetaEngine::getSupportedGames(PlainGameList &games) {
	for (const PlainGameDescriptor *pd = COMPREHEND_GAME_LIST; pd->gameId; ++pd)
		games.push_back(*pd);
}

GameDescriptor ComprehendMetaEngine::findGame(const char *gameId) {
	for (const PlainGameDescriptor *pd = COMPREHEND_GAME_LIST; pd->gameId; ++pd) {
		if (!strcmp(gameId, pd->gameId)) {
			GameDescriptor gd = *pd;
			Common::String s(pd->gameId);
			gd._supportLevel = (s == "transylvaniav2" || s == "talisman") ?
				kUnstableGame : kTestingGame;
			return gd;
		}
	}

	return GameDescriptor::empty();
}

bool ComprehendMetaEngine::detectGames(const Common::FSList &fslist, DetectedGames &gameList) {
	// Loop through the files of the folder
	for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		// Check for a recognised filename
		if (file->isDirectory())
			continue;

		// Check if file occurs in the list
		Common::String filename = file->getName();
		bool isPossible = false;
		const ComprehendDetectionEntry *p = COMPREHEND_GAMES;
		for (; p->_gameId && !isPossible; ++p)
			isPossible = filename.equalsIgnoreCase(p->_filename);

		if (!isPossible)
			continue;

		// Get the file's MD5
		Common::File gameFile;
		if (!gameFile.open(*file))
			continue;
		Common::String md5 = Common::computeStreamMD5AsString(gameFile, 5000);
		gameFile.close();

		// Iterate through the known games
		p = COMPREHEND_GAMES;
		for (; p->_gameId; ++p) {
			if (filename.equalsIgnoreCase(p->_filename)) {
				// Check for an md5 match
				if (p->_md5 == md5) {
					// Found a match
					PlainGameDescriptor gameDesc = findGame(p->_gameId);
					GlkDetectedGame gd(p->_gameId, gameDesc.description, filename);
					gameList.push_back(gd);
				}
			}
		}
	}

	return !gameList.empty();
}

void ComprehendMetaEngine::detectClashes(Common::StringMap &map) {
	for (const PlainGameDescriptor *pd = COMPREHEND_GAME_LIST; pd->gameId; ++pd) {
		if (map.contains(pd->gameId))
			error("Duplicate game Id found - %s", pd->gameId);
		map[pd->gameId] = "";
	}
}

} // End of namespace Comprehend
} // End of namespace Glk
