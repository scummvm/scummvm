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

#include "glk/adrift/detection.h"
#include "glk/adrift/detection_tables.h"
#include "glk/adrift/scprotos.h"
#include "glk/blorb.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/md5.h"
#include "engines/game.h"

namespace Glk {
namespace Adrift {

enum {
	VERSION_HEADER_SIZE = 14
};

/* Various version TAF file signatures. */
static const byte V500_SIGNATURE[VERSION_HEADER_SIZE] = {
	0x3c, 0x42, 0x3f, 0xc9, 0x6a, 0x87, 0xc2, 0xcf, 0x92, 0x45, 0x3e, 0x61, 0x30, 0x30
};
static const byte V500_SIGNATURE_2[VERSION_HEADER_SIZE] = {
	0x3c, 0x42, 0x3f, 0xc9, 0x6a, 0x87, 0xc2, 0xcf, 0x92, 0x45, 0x3e, 0x61, 0x51, 0x36
};

static const byte V400_SIGNATURE[VERSION_HEADER_SIZE] = {
	0x3c, 0x42, 0x3f, 0xc9, 0x6a, 0x87, 0xc2, 0xcf, 0x93, 0x45, 0x3e, 0x61, 0x39, 0xfa
};

static const byte V390_SIGNATURE[VERSION_HEADER_SIZE] = {
	0x3c, 0x42, 0x3f, 0xc9, 0x6a, 0x87, 0xc2, 0xcf, 0x94, 0x45, 0x37, 0x61, 0x39, 0xfa
};

static const byte V380_SIGNATURE[VERSION_HEADER_SIZE] = {
	0x3c, 0x42, 0x3f, 0xc9, 0x6a, 0x87, 0xc2, 0xcf, 0x94, 0x45, 0x36, 0x61, 0x39, 0xfa
};


void AdriftMetaEngine::getSupportedGames(PlainGameList &games) {
	for (const PlainGameDescriptor *pd = ADRIFT_GAME_LIST; pd->gameId; ++pd) {
		games.push_back(*pd);
	}

	for (const PlainGameDescriptor *pd = ADRIFT5_GAME_LIST; pd->gameId; ++pd) {
		games.push_back(*pd);
	}
}

GameDescriptor AdriftMetaEngine::findGame(const char *gameId) {
	for (const PlainGameDescriptor *pd = ADRIFT_GAME_LIST; pd->gameId; ++pd) {
		if (!strcmp(gameId, pd->gameId))
			return *pd;
	}

	for (const PlainGameDescriptor *pd = ADRIFT5_GAME_LIST; pd->gameId; ++pd) {
		if (!strcmp(gameId, pd->gameId)) {
			GameDescriptor gd = *pd;
			gd._supportLevel = kUnstableGame;
			return gd;
		}
	}

	return PlainGameDescriptor::empty();
}

bool AdriftMetaEngine::detectGames(const Common::FSList &fslist, DetectedGames &gameList) {
	int version;
	byte header[VERSION_HEADER_SIZE];

	// Loop through the files of the folder
	for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		// Check for a recognised filename
		if (file->isDirectory())
			continue;
		Common::String filename = file->getName();
		bool hasExt = Blorb::hasBlorbExt(filename) || filename.hasSuffixIgnoreCase(".taf");
		if (!hasExt)
			continue;

		// Open up the file and calculate the md5
		Common::File gameFile;
		if (!gameFile.open(*file))
			continue;

		Common::String md5 = Common::computeStreamMD5AsString(gameFile, 5000);
		size_t filesize = gameFile.size();

		gameFile.seek(0);
		gameFile.read(header, VERSION_HEADER_SIZE);

		gameFile.seek(0);
		bool isBlorb = Blorb::isBlorb(gameFile, ID_ADRI);
		gameFile.close();

		if (!isBlorb && Blorb::hasBlorbExt(filename))
			continue;

		// Check for known games
		const GlkDetectionEntry *p = ADRIFT_GAMES;
		while (p->_gameId && (md5 != p->_md5 || filesize != p->_filesize))
			++p;

		if (p->_gameId) {
			PlainGameDescriptor gameDesc = findGame(p->_gameId);
			gameList.push_back(GlkDetectedGame(p->_gameId, gameDesc.description, p->_extra, filename, p->_language));
		} else {
			version = isBlorb ? 0 : detectGameVersion(header);

			if (isBlorb || version != TAF_VERSION_NONE) {
				const PlainGameDescriptor &desc = ADRIFT_GAME_LIST[0];
				gameList.push_back(GlkDetectedGame(desc.gameId, desc.description, filename, md5, filesize));
			}
		}
	}

	return !gameList.empty();
}

void AdriftMetaEngine::detectClashes(Common::StringMap &map) {
	for (const PlainGameDescriptor *pd = ADRIFT_GAME_LIST; pd->gameId; ++pd) {
		if (map.contains(pd->gameId))
			error("Duplicate game Id found - %s", pd->gameId);
		map[pd->gameId] = "";
	}

	for (const PlainGameDescriptor *pd = ADRIFT5_GAME_LIST; pd->gameId; ++pd) {
		if (map.contains(pd->gameId))
			error("Duplicate game Id found - %s", pd->gameId);
		map[pd->gameId] = "";
	}
}

int AdriftMetaEngine::detectGameVersion(const byte *header) {
	if (memcmp(header, V500_SIGNATURE, VERSION_HEADER_SIZE) == 0 ||
		memcmp(header, V500_SIGNATURE_2, VERSION_HEADER_SIZE) == 0) {
		return TAF_VERSION_500;

	} else if (memcmp(header, V400_SIGNATURE, VERSION_HEADER_SIZE) == 0) {
		return TAF_VERSION_400;

	} else if (memcmp(header, V390_SIGNATURE, VERSION_HEADER_SIZE) == 0) {
		return TAF_VERSION_390;

	} else if (memcmp(header, V380_SIGNATURE, VERSION_HEADER_SIZE) == 0) {
		return TAF_VERSION_380;

	} else {
		return TAF_VERSION_NONE;
	}
}

} // End of namespace Adrift
} // End of namespace Glk
