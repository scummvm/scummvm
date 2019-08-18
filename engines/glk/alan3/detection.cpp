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

#include "glk/alan3/detection.h"
#include "glk/alan3/detection_tables.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/md5.h"
#include "engines/game.h"

namespace Glk {
namespace Alan3 {

void Alan3MetaEngine::getSupportedGames(PlainGameList &games) {
	for (const PlainGameDescriptor *pd = ALAN3_GAME_LIST; pd->gameId; ++pd) {
		games.push_back(*pd);
	}
}

GameDescriptor Alan3MetaEngine::findGame(const char *gameId) {
	for (const PlainGameDescriptor *pd = ALAN3_GAME_LIST; pd->gameId; ++pd) {
		if (!strcmp(gameId, pd->gameId))
			return *pd;
	}

	return GameDescriptor::empty();
}

bool Alan3MetaEngine::detectGames(const Common::FSList &fslist, DetectedGames &gameList) {
	// Loop through the files of the folder
	for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		// Check for a recognised filename
		if (file->isDirectory())
			continue;
		Common::String filename = file->getName();
		bool hasExt = filename.hasSuffix(".a3c");
		if (!hasExt)
			continue;

		// Open up the file and calculate the md5
		Common::File gameFile;
		if (!gameFile.open(*file) || gameFile.readUint32BE() != MKTAG('A', 'L', 'A', 'N'))
			continue;

		gameFile.seek(0);
		Common::String md5 = Common::computeStreamMD5AsString(gameFile, 5000);
		size_t filesize = gameFile.size();
		gameFile.close();

		// Check for known games
		const GlkDetectionEntry *p = ALAN3_GAMES;
		while (p->_gameId && (md5 != p->_md5 || filesize != p->_filesize))
			++p;

		if (!p->_gameId) {
			const PlainGameDescriptor &desc = ALAN3_GAME_LIST[0];
			gameList.push_back(GlkDetectedGame(desc.gameId, desc.description, filename, md5, filesize));
		} else {
			PlainGameDescriptor gameDesc = findGame(p->_gameId);
			gameList.push_back(GlkDetectedGame(p->_gameId, gameDesc.description, filename));
		}
	}

	return !gameList.empty();
}

void Alan3MetaEngine::detectClashes(Common::StringMap &map) {
	for (const PlainGameDescriptor *pd = ALAN3_GAME_LIST; pd->gameId; ++pd) {
		if (map.contains(pd->gameId))
			error("Duplicate game Id found - %s", pd->gameId);
		map[pd->gameId] = "";
	}
}

} // End of namespace Alan3
} // End of namespace Glk
