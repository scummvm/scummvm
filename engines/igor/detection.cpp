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

static const PlainGameDescriptor igorGameDescriptor = {
	"igor", "Igor: Objective Uikokahonia"
};

static const char *igorDetectFileName = "IGOR.DAT";
static uint32 igorDetectFileSize = 4086790;

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

GameList Engine_IGOR_detectGames(const FSList &fslist) {
	GameList detectedGames;
	for (FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		if (file->isDirectory()) {
			continue;
		}
		if (file->getName().equalsIgnoreCase(igorDetectFileName)) {
			Common::File f;
			if (!f.open(*file)) {
				continue;
			}
			const uint32 sig = f.readUint32BE();
			if (sig == MKID_BE('FBOV') && f.size() == igorDetectFileSize) {
				GameDescriptor gd(igorGameDescriptor.gameid, igorGameDescriptor.description, Common::EN_ANY, Common::kPlatformPC);
				gd.updateDesc("Demo");
				detectedGames.push_back(gd);
				break;
			}
		}
	}
	return detectedGames;
}

PluginError Engine_IGOR_create(OSystem *system, Engine **engine) {
	FSList fslist;
	FilesystemNode dir(ConfMan.get("path"));
	if (!dir.getChildren(fslist, FilesystemNode::kListFilesOnly)) {
		return kInvalidPathError;
	}
	GameList gameList = Engine_IGOR_detectGames(fslist);
	if (gameList.size() != 1) {
		return kNoGameDataFoundError;
	}
	assert(engine);
	*engine = new Igor::IgorEngine(system);
	return kNoError;
}

REGISTER_PLUGIN(IGOR, "Igor: Objective Uikokahonia", "Igor: Objective Uikokahonia (C) Pendulo Studios");
