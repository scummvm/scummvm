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

#include "glk/tads/detection.h"
#include "glk/tads/detection_tables.h"
#include "glk/blorb.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/md5.h"
#include "engines/game.h"

namespace Glk {
namespace TADS {

void TADSMetaEngine::getSupportedGames(PlainGameList &games) {
	for (const PlainGameDescriptor *pd = TADS2_GAME_LIST; pd->gameId; ++pd)
		games.push_back(*pd);
	for (const PlainGameDescriptor *pd = TADS3_GAME_LIST; pd->gameId; ++pd)
		games.push_back(*pd);
}

GameDescriptor TADSMetaEngine::findGame(const char *gameId) {
	for (const PlainGameDescriptor *pd = TADS2_GAME_LIST; pd->gameId; ++pd) {
		if (!strcmp(gameId, pd->gameId)) {
			GameDescriptor gd = *pd;
			gd._options = OPTION_TADS2;
			gd._supportLevel = kUnstableGame;
			return gd;
		}
	}

	for (const PlainGameDescriptor *pd = TADS3_GAME_LIST; pd->gameId; ++pd) {
		if (!strcmp(gameId, pd->gameId)) {
			GameDescriptor gd = *pd;
			gd._options = OPTION_TADS3;
			gd._supportLevel = kUnstableGame;
			return gd;
		}
	}

	return GameDescriptor::empty();
}

bool TADSMetaEngine::detectGames(const Common::FSList &fslist, DetectedGames &gameList) {
	const char *const EXTENSIONS[] = { ".gam", ".t3", nullptr };

	// Loop through the files of the folder
	for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		// Check for a recognised filename
		if (file->isDirectory())
			continue;

		Common::String filename = file->getName();
		bool hasExt = Blorb::hasBlorbExt(filename), isBlorb = true;
		int tadsVersion = -1;
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
		if (Blorb::isBlorb(gameFile, ID_TAD2))
			tadsVersion = 2;
		else if (Blorb::isBlorb(gameFile, ID_TAD3))
			tadsVersion = 3;
		else
			isBlorb = false;

		if (!isBlorb)
			// Figure out the TADS version
			tadsVersion = getTADSVersion(gameFile);

		gameFile.close();

		if (tadsVersion == -1)
			// Not a TADS game, or Blorb containing TADS game, so can be ignored
			continue;

		// Check for known games
		const GlkDetectionEntry *p = TADS_GAMES;
		while (p->_gameId && p->_md5 && (md5 != p->_md5 || filesize != p->_filesize))
			++p;

		DetectedGame gd;
		if (!p->_gameId) {
			const GameDescriptor &desc = tadsVersion == 2 ? TADS2_GAME_LIST[0] : TADS3_GAME_LIST[0];
			gameList.push_back(GlkDetectedGame(desc._gameId, desc._description, filename, md5, filesize));
		} else {
			PlainGameDescriptor gameDesc = findGame(p->_gameId);
			gd = DetectedGame("glk", p->_gameId, gameDesc.description, p->_language, Common::kPlatformUnknown, p->_extra);
			gd.addExtraEntry("filename", filename);
			gameList.push_back(gd);
		}
	}

	return !gameList.empty();
}

void TADSMetaEngine::detectClashes(Common::StringMap &map) {
	for (const PlainGameDescriptor *pd = TADS2_GAME_LIST; pd->gameId; ++pd) {
		if (map.contains(pd->gameId))
			error("Duplicate game Id found - %s", pd->gameId);
		map[pd->gameId] = "";
	}
	for (const PlainGameDescriptor *pd = TADS3_GAME_LIST; pd->gameId; ++pd) {
		if (map.contains(pd->gameId))
			error("Duplicate game Id found - %s", pd->gameId);
		map[pd->gameId] = "";
	}
}

int TADSMetaEngine::getTADSVersion(Common::SeekableReadStream &game) {
	// Read in the start of the file
	char buffer[16];
	game.seek(0);
	game.read(buffer, 16);

	// Check for valid game headers
	if (memcmp(buffer, "TADS2 bin\n\r\032", 12) == 0)
		return 2;
	else if (memcmp(buffer, "T3-image\r\n\032", 11) == 0)
		return 3;
	else
		return -1;
}

} // End of namespace TADS
} // End of namespace Glk
