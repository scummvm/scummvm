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

#include "ags/tests/game_scanner.h"
#include "ags/detection.h"
#include "ags/shared/ac/gamesetupstruct.h"
#include "ags/shared/core/assetmanager.h"
#include "ags/shared/util/multifilelib.h"
#include "ags/shared/util/string.h"
#include "ags/engine/main/game_file.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/md5.h"
#include "common/unzip.h"

namespace AGS3 {

extern bool define_gamedata_location(const AGS::Shared::String &exe_path);
extern bool engine_try_init_gamedata(AGS::Shared::String gamepak_path);
extern GameSetupStruct game;

void GameScanner::scan() {
	Common::FSNode folder("."); //ConfMan.get("path"));
	scanFolder(folder);

	for (EntryMap::iterator it = _games.begin(); it != _games.end(); ++it) {
		debug("{ \"%s\", \"%s\" },", it->_key.c_str(), it->_value._gameName.c_str());
	}
	debug("");

	for (EntryMap::iterator it = _games.begin(); it != _games.end(); ++it) {
		debug("ENGLISH_ENTRY(\"%s\", \"%s\", \"%s\", %u),",
			it->_key.c_str(), it->_value._filename.c_str(),
			it->_value._md5.c_str(), it->_value._filesize);
	}
	debug("");
}

void GameScanner::scanFolder(const Common::FSNode &folder) {
	Common::FSList fslist;
	folder.getChildren(fslist, Common::FSNode::kListAll);

	for (uint idx = 0; idx < fslist.size(); ++idx) {
		Common::FSNode node = fslist[idx];
		Common::String filename = node.getName();

		if (node.isDirectory()) {
			scanFolder(node);
		} else if (filename.hasSuffixIgnoreCase(".exe") ||
				filename.hasSuffixIgnoreCase(".ags") ||
				filename.equalsIgnoreCase("ac2game.dat")) {
			Common::String path = node.getPath();
			scanFile(path);
		}
	}
}

void GameScanner::scanFile(const Common::String &filename) {
	Common::File f;
	Common::FSNode fsNode(filename);
	if (!f.open(fsNode))
		return;

	int32 size = f.size();
	Common::String md5 = Common::computeStreamMD5AsString(f, 5000);

	// Check if it's an already known game
	const ::AGS::AGSGameDescription *gameP = ::AGS::GAME_DESCRIPTIONS;
	for (; gameP->desc.gameId; ++gameP) {
		if (size == gameP->desc.filesDescriptions[0].fileSize &&
				md5 == gameP->desc.filesDescriptions[0].md5)
			// Known game, so skip
			return;
	}

	// Check the game file
	AGS::Shared::AssetLibInfo lib;
	AGS::Shared::StreamScummVMFile *stream = new AGS::Shared::StreamScummVMFile(&f);
	if (AGS::Shared::AssetManager::ReadDataFileTOC(stream, lib) != AGS::Shared::kAssetNoError)
		return;
	f.close();

	AGS::Shared::AssetManager::DestroyInstance();
	AGS::Shared::AssetManager::CreateInstance();
	AGS::Shared::AssetManager::SetDataFile(filename);

	if (!engine_try_init_gamedata(filename))
		return;

	AGS::Shared::HError err = preload_game_data();
	if (!err)
		return;

	// Add an entry for the found game
	Entry e;
	e._filename = fsNode.getName();
	e._filesize = size;
	e._gameName = game.gamename;
	e._md5 = md5;

	Common::String id = convertGameNameToId(e._gameName);
	_games[id] = e;
} 

Common::String GameScanner::convertGameNameToId(const Common::String &name) {
	Common::String result;

	for (uint idx = 0; idx < name.size(); ++idx) {
		char c = name[idx];
		if (Common::isAlnum(c))
			result += tolower(c);
	}

	return result;
}

} // namespace AGS3
