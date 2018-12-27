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

#include "glk/alan2/detection.h"
#include "glk/alan2/detection_tables.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/md5.h"
#include "engines/game.h"

namespace Glk {
namespace Alan2 {

void Alan2MetaEngine::getSupportedGames(PlainGameList &games) {
	for (const Alan2Descriptor *pd = ALAN2_GAME_LIST; pd->gameId; ++pd) {
		games.push_back(*pd);
	}
}

Alan2Descriptor Alan2MetaEngine::findGame(const char *gameId) {
	for (const Alan2Descriptor *pd = ALAN2_GAME_LIST; pd->gameId; ++pd) {
		if (!strcmp(gameId, pd->gameId))
			return *pd;
	}

	return Alan2Descriptor();
}

bool Alan2MetaEngine::detectGames(const Common::FSList &fslist, DetectedGames &gameList) {
	const char *const EXTENSIONS[3] = { ".acd", ".dat" };

	// Loop through the files of the folder
	for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		// Check for a recognised filename
		if (file->isDirectory())
			continue;
		Common::String filename = file->getName();
		bool hasExt = false;
		for (int idx = 0; idx < 3 && !hasExt; ++idx)
			hasExt = filename.hasSuffixIgnoreCase(EXTENSIONS[idx]);
		if (!hasExt)
			continue;

		// Open up the file and calculate the md5
		Common::File gameFile;
		if (!gameFile.open(*file))
			continue;
		Common::String md5 = Common::computeStreamMD5AsString(gameFile, 5000);
		size_t filesize = gameFile.size();
		gameFile.close();

		// Check for known games
		const Alan2GameDescription *p = ALAN2_GAMES;
		while (p->_gameId && (md5 != p->_md5 || filesize != p->_filesize))
			++p;

		DetectedGame gd;
		if (!p->_gameId) {
			if (filename.hasSuffixIgnoreCase(".dat"))
				continue;

			if (gDebugLevel > 0) {
				// Print an entry suitable for putting into the detection_tables.h, using the
				// name of the parent folder the game is in as the presumed game Id
				Common::String folderName = file->getParent().getName();
				if (folderName.hasSuffix("\\"))
					folderName.deleteLastChar();
				Common::String fname = filename;
				const char *dot = strchr(fname.c_str(), '.');
				if (dot)
					fname = Common::String(fname.c_str(), dot);

				debug("ENTRY0(\"%s\", \"%s\", %u),", fname.c_str(), md5.c_str(), (uint)filesize);
			}
			const PlainGameDescriptor &desc = ALAN2_GAME_LIST[0];
			gd = DetectedGame(desc.gameId, desc.description, Common::UNK_LANG, Common::kPlatformUnknown);
		} else {
			PlainGameDescriptor gameDesc = findGame(p->_gameId);
			gd = DetectedGame(p->_gameId, gameDesc.description, p->_language, Common::kPlatformUnknown, p->_extra);
			gd.setGUIOptions(GUIO4(GUIO_NOSPEECH, GUIO_NOSFX, GUIO_NOMUSIC, GUIO_NOSUBTITLES));
		}

		gd.addExtraEntry("filename", filename);
		gameList.push_back(gd);
	}

	return !gameList.empty();
}

void Alan2MetaEngine::detectClashes(Common::StringMap &map) {
	for (const Alan2Descriptor *pd = ALAN2_GAME_LIST; pd->gameId; ++pd) {
		if (map.contains(pd->gameId))
			error("Duplicate game Id found - %s", pd->gameId);
		map[pd->gameId] = "";
	}
}

} // End of namespace Alan2
} // End of namespace Glk
