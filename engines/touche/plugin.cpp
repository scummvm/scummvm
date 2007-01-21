/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"
#include "common/config-manager.h"
#include "common/fs.h"
#include "common/md5.h"

#include "base/plugins.h"

#include "touche/touche.h"

struct GameVersion {
	const char *description;
	uint32 filesize;
	const char *md5digest;
	Common::Language language;
	Common::Platform platform;
};

static const GameVersion toucheGameVersionsTable[] = {
	{ // retail version
		"Touche: The Adventures of the Fifth Musketeer",
		26350211,
		"2af0177f8887e3430f345e6b4d8b1414",
		Common::EN_ANY,
		Common::kPlatformPC
	},
	{ // retail version - tracker item #1601818
		"Touche: The Adventures of the Fifth Musketeer",
		26350190,
		"95967f0b51d2e813e99ca00325098340",
		Common::EN_ANY,
		Common::kPlatformWindows
	},
	{ // retail version
		"Touche: Les Aventures du Cinquieme Mousquetaire",
		26558232,
		"1caa20bb4d4fc2ce8eb867b6610082b3",
		Common::FR_FRA,
		Common::kPlatformPC
	},
	{ // retail version - tracker item #1598643
		"Touche - Die Abenteuer des funften Musketiers",
		26625537,
		"be2ae6454b3325e410946f2322547cd4",
		Common::DE_DEU,
		Common::kPlatformPC
	},
	{ // fan-made translation (http://www.iagtg.net/) - tracker item #1602360
		"Touche: The Adventures of the Fifth Musketeer",
		26367792,
		"1f442331d4b327c3488a9f6ffe9bdd25",
		Common::IT_ITA,
		Common::kPlatformPC
	},
	{ // demo version
		"Touche: The Adventures of the Fifth Musketeer (Demo)",
		8720683,
		"ddaed436445b2e77294ed19e8ae4aa2c",
		Common::EN_ANY,
		Common::kPlatformPC
	}
};

static const PlainGameDescriptor toucheGameDescriptor = {
	"touche", "Touche: The Adventures of the Fifth Musketeer"
};

static const char *toucheDetectFileName = "TOUCHE.DAT";

static Common::String Engine_TOUCHE_md5digest(const FilesystemNode *file) {
	static const int md5DataSize = 4096;
	uint8 md5digest[16];
	if (Common::md5_file(*file, md5digest, md5DataSize)) {
		char md5sum[32 + 1];
		for (int i = 0; i < 16; ++i) {
			sprintf(md5sum + i * 2, "%02x", (int)md5digest[i]);
		}
		return md5sum;
	}
	return "";
}

static uint32 Engine_TOUCHE_filesize(const FilesystemNode *file) {
	Common::File f;
	if (f.open(file->path().c_str())) {
		return f.size();
	}
	return 0;
}

GameList Engine_TOUCHE_gameIDList() {
	GameList games;
	games.push_back(toucheGameDescriptor);
	return games;
}

GameDescriptor Engine_TOUCHE_findGameID(const char *gameid) {
	if (scumm_stricmp(toucheGameDescriptor.gameid, gameid) == 0) {
		return toucheGameDescriptor;
	}
	return GameDescriptor();
}

GameList Engine_TOUCHE_detectGames(const FSList &fslist) {
	bool foundFile = false;
	FSList::const_iterator file;
	for (file = fslist.begin(); file != fslist.end(); ++file) {
		if (file->isDirectory()) {
			continue;
		}
		for (int i = 0; i < ARRAYSIZE(toucheGameVersionsTable); ++i) {
			if (file->name().equalsIgnoreCase(toucheDetectFileName)) {
				foundFile = true;
				break;
			}
		}
		if (foundFile) {
			break;
		}
	}
	GameList detectedGames;
	if (foundFile) {
		// Currently, the detection code is based on a MD5 checksum. If all known versions
		// have a different file size for TOUCHE.DAT, we may consider using this to do the
		// detection.
		Common::String md5digest = Engine_TOUCHE_md5digest(file);
		if (!md5digest.empty()) {
			for (int i = 0; i < ARRAYSIZE(toucheGameVersionsTable); ++i) {
				const GameVersion *gv = &toucheGameVersionsTable[i];
				if (md5digest.equalsIgnoreCase(gv->md5digest)) {
					GameDescriptor dg(toucheGameDescriptor.gameid, gv->description, gv->language, gv->platform);
					detectedGames.push_back(dg);
					break;
				}
			}
			if (detectedGames.empty()) {
				const uint32 filesize = Engine_TOUCHE_filesize(file);
				printf("Datafile size (%d) and MD5 (%s) are unknown !\n", filesize, md5digest.c_str());
				printf("Please report the details (language, platform, etc.) of this game to the ScummVM team.\n");
				detectedGames.push_back(toucheGameDescriptor);
			}
		}
	}
	return detectedGames;
}

PluginError Engine_TOUCHE_create(OSystem *system, Engine **engine) {
	FSList fslist;
	FilesystemNode dir(ConfMan.get("path"));
	if (!dir.listDir(fslist, FilesystemNode::kListFilesOnly)) {
		return kInvalidPathError;
	}
	GameList game = Engine_TOUCHE_detectGames(fslist);
	if (game.size() != 1) {
		return kNoGameDataFoundError;
	}
	assert(engine);
	*engine = new Touche::ToucheEngine(system, game[0].language());
	return kNoError;
}

REGISTER_PLUGIN(TOUCHE, "Touche Engine", "Touche: The Adventures of the 5th Musketeer (C) Clipper Software");
