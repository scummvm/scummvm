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

#include "base/plugins.h"

#include "common/config-manager.h"
#include "common/file.h"
#include "common/fs.h"

#include "igor/igor.h"

struct GameDetectVersion {
	uint32 borlandOverlaySize;
	int gameVersion;
	Common::Language language;
	const char *descriptionSuffix;
};

static const GameDetectVersion igorDetectVersionsTable[] = {
	{ 4086790, Igor::kIdEngDemo100, Common::EN_ANY, " 1.00s" },
	{ 4094103, Igor::kIdEngDemo110, Common::EN_ANY, " 1.10s" }
};

static const char *igorDetectFileName = "IGOR.DAT";

static const PlainGameDescriptor igorGameDescriptor = {
	"igor", "Igor: Objective Uikokahonia"
};

GameList Engine_IGOR_gameIDList() {
	GameList games;
	games.push_back(igorGameDescriptor);
	return games;
}

GameDescriptor Engine_IGOR_findGameID(const char *gameid) {
	if (scumm_stricmp(igorGameDescriptor.gameid, gameid) == 0) {
		return igorGameDescriptor;
	}
	return GameDescriptor();
}

static const GameDetectVersion *Engine_IGOR_findGameVersion(const FSList &fslist) {
	for (FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		if (file->isDirectory() || !file->getName().equalsIgnoreCase(igorDetectFileName)) {
			continue;
		}
		Common::File f;
		if (f.open(*file)) {
			const uint32 fileSize = f.size();
			if (f.readUint32BE() == MKID_BE('FBOV')) {
				for (int i = 0; i < ARRAYSIZE(igorDetectVersionsTable); ++i) {
					if (igorDetectVersionsTable[i].borlandOverlaySize == fileSize) {
						return &igorDetectVersionsTable[i];
					}
				}
			}
		}
	}
	return 0;
}

GameList Engine_IGOR_detectGames(const FSList &fslist) {
	GameList detectedGames;
	const GameDetectVersion *gdv = Engine_IGOR_findGameVersion(fslist);
	if (gdv) {
		GameDescriptor gd(igorGameDescriptor.gameid, igorGameDescriptor.description, gdv->language, Common::kPlatformPC);
		gd.description() += gdv->descriptionSuffix;
		gd.updateDesc("Demo");
		detectedGames.push_back(gd);
	}
	return detectedGames;
}

PluginError Engine_IGOR_create(OSystem *system, Engine **engine) {
	FSList fslist;
	FilesystemNode dir(ConfMan.get("path"));
	if (!dir.getChildren(fslist, FilesystemNode::kListFilesOnly)) {
		return kInvalidPathError;
	}
	const GameDetectVersion *gdv = Engine_IGOR_findGameVersion(fslist);
	if (!gdv) {
		return kNoGameDataFoundError;
	}
	assert(engine);
	*engine = new Igor::IgorEngine(system, gdv->gameVersion);
	return kNoError;
}

REGISTER_PLUGIN(IGOR, "Igor: Objective Uikokahonia", "Igor: Objective Uikokahonia (C) Pendulo Studios");
