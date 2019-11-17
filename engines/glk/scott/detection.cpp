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
#include "glk/blorb.h"
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
	const char *const EXTENSIONS[] = { ".saga", ".dat", nullptr };

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
		Common::String md5 = Common::computeStreamMD5AsString(gameFile, 5000);
		size_t filesize = (size_t)gameFile.size();
		gameFile.seek(0);
		isBlorb = Blorb::isBlorb(gameFile, ID_SAAI);
		gameFile.close();

		if (!isBlorb && Blorb::hasBlorbExt(filename))
			continue;

		// Scan through the Scott game list for a match
		const GlkDetectionEntry *p = SCOTT_GAMES;
		while (p->_md5 && p->_filesize != filesize && md5 != p->_md5)
			++p;

		if (!p->_gameId) {
			if (!isBlorb && filename.hasSuffixIgnoreCase(".dat"))
				continue;

			const PlainGameDescriptor &desc = SCOTT_GAME_LIST[0];
			gameList.push_back(GlkDetectedGame(desc.gameId, desc.description, filename, md5, filesize));
		} else {
			// Found a match
			PlainGameDescriptor gameDesc = findGame(p->_gameId);
			gameList.push_back(GlkDetectedGame(p->_gameId, gameDesc.description, filename));
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
