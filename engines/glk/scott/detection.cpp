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

/*
 * Based on ScottFree interpreter version 1.14 developed by Swansea
 * University Computer Society without disassembly of any other game
 * drivers, only of game databases as permitted by EEC law (for purposes
 * of compatibility).
 *
 * Licensed under GPLv2
 *
 * https://github.com/angstsmurf/spatterlight/tree/master/terps/scott
 */

#include "common/file.h"
#include "common/md5.h"
#include "engines/game.h"
#include "glk/blorb.h"
#include "glk/scott/detection.h"
#include "glk/scott/detection_tables.h"

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
	const char *const EXTENSIONS[] = {".z80", ".saga", ".dat", ".D64", ".T64", "fiad", nullptr};

	// Loop through the files of the folder
	for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		// Check for a recognised filename
		if (file->isDirectory())
			continue;

		Common::String filename = file->getName();
		bool hasExt = Blorb::hasBlorbExt(filename), isBlorb = false;
		for (const char *const *ext = &EXTENSIONS[0]; *ext && !hasExt; ++ext)
			hasExt = filename.hasSuffixIgnoreCase(*ext);
		if (!hasExt)
			continue;

		Common::File gameFile;
		if (!gameFile.open(*file))
			continue;
		Common::String md5;
		if (filename.hasSuffixIgnoreCase(".D64"))
			md5 = Common::computeStreamMD5AsString(gameFile);
		else
			md5 = Common::computeStreamMD5AsString(gameFile, 5000);

		size_t filesize = (size_t)gameFile.size();
		gameFile.seek(0);
		isBlorb = Blorb::isBlorb(gameFile, ID_SAAI);
		gameFile.close();

		if (!isBlorb && Blorb::hasBlorbExt(filename))
			continue;

		// Scan through the Scott game list for a match
		const GlkDetectionEntry *p = SCOTT_GAMES;
		while (p->_md5 && (p->_filesize != filesize || md5 != p->_md5))
			++p;

		if (!p->_gameId) {

			// ignore possible variants for common extensions to prevent flooding in mass-add
			if (!isBlorb && (filename.hasSuffixIgnoreCase(".z80") || filename.hasSuffixIgnoreCase(".dat") ||
				filename.hasSuffixIgnoreCase(".d64") || filename.hasSuffixIgnoreCase(".t64")))
				continue;

			const PlainGameDescriptor &desc = SCOTT_GAME_LIST[0];
			gameList.push_back(GlkDetectedGame(desc.gameId, desc.description, filename, md5, filesize));
		} else {
			// Found a match
			PlainGameDescriptor gameDesc = findGame(p->_gameId);
			gameList.push_back(GlkDetectedGame(p->_gameId, gameDesc.description, filename, p->_language, p->_platform));
		}
	}

	return !gameList.empty();
}

void ScottMetaEngine::detectClashes(Common::StringMap &map) {
	for (const PlainGameDescriptor *pd = SCOTT_GAME_LIST; pd->gameId; ++pd) {
		if (map.contains(pd->gameId))
			error("ScottMetaEngine::detectClashes: Duplicate game Id found - %s", pd->gameId);
		map[pd->gameId] = "";
	}
}

} // End of namespace Scott
} // End of namespace Glk
