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

#include "glk/magnetic/detection.h"
#include "glk/magnetic/detection_tables.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/md5.h"
#include "engines/game.h"

namespace Glk {
namespace Magnetic {

void MagneticMetaEngine::getSupportedGames(PlainGameList &games) {
	for (const PlainGameDescriptor *pd = MAGNETIC_GAME_LIST; pd->gameId; ++pd) {
		games.push_back(*pd);
	}
}

GameDescriptor MagneticMetaEngine::findGame(const char *gameId) {
	for (const PlainGameDescriptor *pd = MAGNETIC_GAME_LIST; pd->gameId; ++pd) {
		if (!strcmp(gameId, pd->gameId))
			return *pd;
	}

	return PlainGameDescriptor::empty();
}

bool MagneticMetaEngine::detectGames(const Common::FSList &fslist, DetectedGames &gameList) {
	const char *const EXTENSIONS[] = { ".mag", ".rsc", nullptr };

	// Loop through the files of the folder
	for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		// Check for a recognised filename
		if (file->isDirectory())
			continue;
		Common::String filename = file->getName();
		bool hasExt = false;
		for (const char *const *ext = &EXTENSIONS[0]; *ext && !hasExt; ++ext)
			hasExt = filename.hasSuffixIgnoreCase(*ext);
		if (!hasExt)
			continue;

		// Open up the file and calculate the md5
		Common::File gameFile;
		if (!gameFile.open(*file))
			continue;
		if (gameFile.readUint32BE() != MKTAG('M', 'a', 'S', 'c')) {
			gameFile.close();
			continue;
		}

		gameFile.seek(0);
		Common::String md5 = Common::computeStreamMD5AsString(gameFile, 5000);
		size_t filesize = gameFile.size();
		gameFile.close();

		// Check for known games
		const GlkDetectionEntry *p = MAGNETIC_GAMES;
		while (p->_gameId && (md5 != p->_md5 || filesize != p->_filesize))
			++p;

		if (!p->_gameId) {
			const PlainGameDescriptor &desc = MAGNETIC_GAME_LIST[0];
			gameList.push_back(GlkDetectedGame(desc.gameId, desc.description, filename, md5, filesize));
		} else {
			PlainGameDescriptor gameDesc = findGame(p->_gameId);
			gameList.push_back(GlkDetectedGame(p->_gameId, gameDesc.description, p->_extra, filename, p->_language));
		}
	}

	return !gameList.empty();
}

void MagneticMetaEngine::detectClashes(Common::StringMap &map) {
	for (const PlainGameDescriptor *pd = MAGNETIC_GAME_LIST; pd->gameId; ++pd) {
		if (map.contains(pd->gameId))
			error("Duplicate game Id found - %s", pd->gameId);
		map[pd->gameId] = "";
	}
}

const gms_game_table_t *gms_gameid_lookup_game(uint32 undo_size, uint32 undo_pc) {
	const gms_game_table_t *game;

	for (game = GMS_GAME_TABLE; game->name; game++) {
		if (game->undo_size == undo_size && game->undo_pc == undo_pc)
			break;
	}

	return game->name ? game : nullptr;
}

} // End of namespace Magnetic
} // End of namespace Glk
