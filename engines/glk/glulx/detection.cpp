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

#include "glk/glulx/detection.h"
#include "glk/glulx/detection_tables.h"
#include "glk/blorb.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/md5.h"
#include "engines/game.h"

namespace Glk {
namespace Glulx {

void GlulxMetaEngine::getSupportedGames(PlainGameList &games) {
	for (const PlainGameDescriptor *pd = GLULXE_GAME_LIST; pd->gameId; ++pd) {
		games.push_back(*pd);
	}
}

GameDescriptor GlulxMetaEngine::findGame(const char *gameId) {
	for (const PlainGameDescriptor *pd = GLULXE_GAME_LIST; pd->gameId; ++pd) {
		if (!strcmp(gameId, pd->gameId)) {
			GameDescriptor gd = *pd;
			gd._supportLevel = kTestingGame;
			return gd;
		}
	}

	return GameDescriptor::empty();
}

bool GlulxMetaEngine::detectGames(const Common::FSList &fslist, DetectedGames &gameList) {
	const char *const EXTENSIONS[] = { ".ulx", nullptr };

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

		// Open up the file and calculate the md5
		Common::File gameFile;
		if (!gameFile.open(*file))
			continue;
		Common::String md5 = Common::computeStreamMD5AsString(gameFile, 5000);
		size_t filesize = gameFile.size();
		gameFile.seek(0);
		isBlorb = Blorb::isBlorb(gameFile, ID_GLUL);
		gameFile.close();

		if (!isBlorb && Blorb::hasBlorbExt(filename))
			continue;

		// Check for known games
		const GlkDetectionEntry *p = GLULXE_GAMES;
		while (p->_gameId && (md5 != p->_md5 || filesize != p->_filesize))
			++p;

		if (!p->_gameId) {
			const PlainGameDescriptor &desc = GLULXE_GAME_LIST[0];
			gameList.push_back(GlkDetectedGame(desc.gameId, desc.description, filename, md5, filesize));
		} else {
			PlainGameDescriptor gameDesc = findGame(p->_gameId);
			DetectedGame gd = DetectedGame("glk", p->_gameId, gameDesc.description, p->_language, Common::kPlatformUnknown, p->_extra);

			gd.addExtraEntry("filename", filename);
			gameList.push_back(gd);
		}
	}

	return !gameList.empty();
}

void GlulxMetaEngine::detectClashes(Common::StringMap &map) {
	for (const PlainGameDescriptor *pd = GLULXE_GAME_LIST; pd->gameId; ++pd) {
		if (map.contains(pd->gameId))
			error("Duplicate game Id found - %s", pd->gameId);
		map[pd->gameId] = "";
	}
}

} // End of namespace Glulx
} // End of namespace Glk
