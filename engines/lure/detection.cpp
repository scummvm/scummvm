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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
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

#include "base/plugins.h"

#include "common/advancedDetector.h"
#include "common/endian.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/md5.h"

#include "lure/luredefs.h"
#include "lure/lure.h"

namespace Lure {

enum {
	// We only compute MD5 of the first kilobyte of our data files.
	kMD5FileSizeLimit = 1024
};

struct GameSettings {
	const char *gameid;
	const char *description;
	byte id;
	uint32 features;
	Common::Language language;
	const char *md5sum;
	const char *checkFile;
};

//
static const GameSettings lure_games[] = {
	{ "lure", "Lure of the Temptress", GI_LURE, GF_FLOPPY, Common::EN_ANY,
										"b2a8aa6d7865813a17a3c636e063572e", "disk1.vga" },
/*
	{ "lure", "Lure of the Temptress", GI_LURE, GF_FLOPPY, Common::DE_DEU,
										"7aa19e444dab1ac7194d9f7a40ffe54a", "disk1.vga" },
	{ "lure", "Lure of the Temptress", GI_LURE, GF_FLOPPY, Common::FR_FRA,
										"1c94475c1bb7e0e88c1757d3b5377e94", "disk1.vga" },
*/
	{ 0, 0, 0, 0, Common::UNK_LANG, 0, 0 }
};

// Keep list of different supported games

static const PlainGameDescriptor lure_list[] = {
	{ "lure", "Lure of the Temptress" },
	{ 0, 0 }
};

} // End of namespace Lure

using namespace Lure;

GameList Engine_LURE_gameIDList() {
	return GameList(lure_list);
}

GameDescriptor Engine_LURE_findGameID(const char *gameid) {
	return Common::AdvancedDetector::findGameID(gameid, lure_list);
}

GameList Engine_LURE_detectGames(const FSList &fslist) {
	GameList detectedGames;
	const GameSettings *g;
	FSList::const_iterator file;

	// Iterate over all files in the given directory
	bool isFound = false;
	for (file = fslist.begin(); file != fslist.end(); file++) {
		if (file->isDirectory())
			continue;

		for (g = lure_games; g->gameid; g++) {
			if (scumm_stricmp(file->getName().c_str(), g->checkFile) == 0)
				isFound = true;
		}
		if (isFound)
			break;
	}

	if (file == fslist.end())
		return detectedGames;

	char md5str[32 + 1];

	if (Common::md5_file_string(*file, md5str, kMD5FileSizeLimit)) {
		for (g = lure_games; g->gameid; g++) {
			if (strcmp(g->md5sum, (char *)md5str) == 0) {
				GameDescriptor dg(g->gameid, g->description, g->language);
				dg.updateDesc((g->features & GF_FLOPPY) ? "Floppy" : 0);
				detectedGames.push_back(dg);
			}
		}
		if (detectedGames.empty()) {
			printf("Your game version appears to be unknown. Please, report the following\n");
			printf("data to the ScummVM team along with name of the game you tried to add\n");
			printf("and its version/language/etc.:\n");
			
			printf("  LURE MD5 '%s'\n\n", md5str);

			const PlainGameDescriptor *g1 = lure_list;
			while (g1->gameid) {
				detectedGames.push_back(*g1);
				g1++;
			}
		}
	}
	return detectedGames;
}

PluginError Engine_LURE_create(OSystem *syst, Engine **engine) {
	assert(engine);
	*engine = new LureEngine(syst);
	return kNoError;
}

REGISTER_PLUGIN(LURE, "Lure of the Temptress Engine", "Lure of the Temptress (C) Revolution");

namespace Lure {

void LureEngine::detectGame() {
	// Make sure all the needed files are present

	if (!Common::File::exists(SUPPORT_FILENAME))
		error("Missing %s - this is a custom file containing resources from the\n"
			"Lure of the Temptress executable. See the documentation for creating it.",
			SUPPORT_FILENAME);

	for (uint8 fileNum = 1; fileNum <= 4; ++fileNum) {
		char sFilename[10];
		sprintf(sFilename, "disk%d.vga", fileNum);

		if (!Common::File::exists(sFilename))
			error("Missing disk%d.vga", fileNum);
	}

	// Check the version of the lure.dat file
	Common::File f;
	if (!f.open(SUPPORT_FILENAME)) {
		error("Error opening %s for validation", SUPPORT_FILENAME);
	} else {
		f.seek(0xbf * 8);
		VersionStructure version;
		f.read(&version, sizeof(VersionStructure));
		f.close();

		if (READ_LE_UINT16(&version.id) != 0xffff)
			error("Error validating %s - file is invalid or out of date", SUPPORT_FILENAME);
		else if ((version.vMajor != LURE_DAT_MAJOR) || (version.vMinor != LURE_DAT_MINOR))
			error("Incorrect version of %s file - expected %d.%d but got %d.%d",
				SUPPORT_FILENAME, LURE_DAT_MAJOR, LURE_DAT_MINOR, 
				version.vMajor, version.vMinor);
	}

	// Do an md5 check 

	char md5str[32 + 1];
	const GameSettings *g;
	bool found = false;

	*md5str = 0;

	for (g = lure_games; g->gameid; g++) {
		if (!Common::File::exists(g->checkFile))
			continue;

		if (!Common::md5_file_string(g->checkFile, md5str, kMD5FileSizeLimit))
			continue;

		if (strcmp(g->md5sum, (char *)md5str) == 0) {
			_features = g->features;
			_game = g->id;
			_language = g->language;

			if (g->description)
				g_system->setWindowCaption(g->description);

			found = true;
			break;
		}
	}

	if (!found) {
		debug("Unknown MD5 (%s)! Please report the details (language, platform, etc.) of this game to the ScummVM team", md5str);
		_features = GF_LNGUNK || GF_FLOPPY;
		_game = GI_LURE;
	}
}

} // End of namespace Lure
