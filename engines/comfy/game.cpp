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

#include "comfy/comfy.h"

#include "common/config-manager.h"

namespace Comfy {

static uint16 getLanguageId(const Common::String &directory) {
	if (directory.equalsIgnoreCase("HEBREW"))
		return 1;

	if (directory.equalsIgnoreCase("ENGLISH"))
		return 2;

	if (directory.equalsIgnoreCase("GERMAN"))
		return 3;

	if (directory.equalsIgnoreCase("FRENCH"))
		return 4;

	if (directory.equalsIgnoreCase("SPANISH"))
		return 5;

	if (directory.equalsIgnoreCase("DUTCH"))
		return 6;

	if (directory.equalsIgnoreCase("PORTUGES"))
		return 7;

	if (directory.equalsIgnoreCase("ENGUK"))
		return 8;

	if (directory.equalsIgnoreCase("ITALIAN"))
		return 9;

	if (directory.equalsIgnoreCase("DANISH"))
		return 10;

	if (directory.equalsIgnoreCase("NORWEGIA"))
		return 11;

	if (directory.equalsIgnoreCase("SWEDISH"))
		return 12;

	if (directory.equalsIgnoreCase("JAPANESE"))
		return 14;

	if (directory.equalsIgnoreCase("TURKISH"))
		return 15;

	return 0;
}

void ComfyEngine::gameConfigInit() {
	if (!strcmp(_gameDescription->gameId, "comfyland")) {
		_gameDirectory = Common::Path("COMFY1");
		_introDirectory = Common::Path("INTRO");
	} else if (!strcmp(_gameDescription->gameId, "boo")) {
		_gameDirectory = Common::Path("BOO");
		_introDirectory = Common::Path("FINTRO");
	} else if (!strcmp(_gameDescription->gameId, "first")) {
		_gameDirectory = Common::Path("FIRST");
		_introDirectory = Common::Path("FINTRO");
	} else if (!strcmp(_gameDescription->gameId, "match")) {
		_gameDirectory = Common::Path("MATCH");
		_introDirectory = Common::Path("MINTRO");
	} else if (!strcmp(_gameDescription->gameId, "colors")) {
		_gameDirectory = Common::Path("COLORS");
		_introDirectory = Common::Path("FINTRO");
	} else if (!strcmp(_gameDescription->gameId, "concert")) {
		_gameDirectory = Common::Path("CONCERT");
		_introDirectory = Common::Path("INTRO");
	} else if (!strcmp(_gameDescription->gameId, "friends")) {
		_gameDirectory = Common::Path("FRIENDS");
		_introDirectory = Common::Path("FINTRO");
	} else if (!strcmp(_gameDescription->gameId, "panther")) {
		_gameDirectory = Common::Path("PANTHER");
		_introDirectory = Common::Path("MINTRO");
	}

	findLanguageDirectories();
	ConfMan.registerDefault("comfy_language", 2);
}

void ComfyEngine::findLanguageDirectories() {
	for (uint i = 0; i < ARRAYSIZE(_languageDirectories); i++)
		_languageDirectories[i] = Common::Path();

	Common::FSNode root(ConfMan.getPath("path"));
	Common::FSList gameDirectories;
	if (!root.getChildren(gameDirectories, Common::FSNode::kListDirectoriesOnly))
		return;

	Common::FSNode gameDirectory;
	for (Common::FSList::iterator it = gameDirectories.begin(); it != gameDirectories.end(); it++) {
		if (it->getName().equalsIgnoreCase(_gameDirectory.toString())) {
			gameDirectory = *it;
			break;
		}
	}

	if (!gameDirectory.exists())
		return;

	Common::FSList directories;
	if (!gameDirectory.getChildren(directories, Common::FSNode::kListDirectoriesOnly))
		return;

	for (Common::FSList::iterator it = directories.begin(); it != directories.end(); it++) {
		uint16 language = getLanguageId(it->getName());
		if (language)
			_languageDirectories[language] = Common::Path(it->getName());
	}
}

bool ComfyEngine::iniReadGameConfig() {
	_language = ConfMan.getInt("comfy_language");
	_multiLanguage = true;
	return !_gameDirectory.empty();
}

bool ComfyEngine::iniGetGameDataPath(uint16 sceneId) {
	if (sceneId == 0) {
		if (_introDirectory.empty())
			return false;

		pathSetGameDataDir(_introDirectory);
		return true;
	}

	if (sceneId == 0x63) {
		pathSetGameDataDir(_gameDirectory.join("SETLANG"));
		return true;
	}

	Common::Path languageDirectory = getLanguageDirectory(sceneId);
	if (languageDirectory.empty())
		return false;

	pathSetGameDataDir(_gameDirectory.join(languageDirectory));
	return true;
}

void ComfyEngine::iniWriteLanguage(uint16 language) {
	_language = language;
	ConfMan.setInt("comfy_language", language);
	ConfMan.flushToDisk();
}

Common::Path ComfyEngine::getLanguageDirectory(uint16 language) {
	if (language >= ARRAYSIZE(_languageDirectories))
		return Common::Path();

	return _languageDirectories[language];
}

void ComfyEngine::pathSetGameDataDir(const Common::Path &path) {
	_gameDataPath = path;
}

Common::Path ComfyEngine::pathBuild(const Common::Path &filename, bool useGamePath) {
	if (useGamePath && !_gameDataPath.empty())
		return _gameDataPath.join(filename);

	return filename;
}

Common::SeekableReadStream *ComfyEngine::pathFOpen(const Common::Path &filename, bool useGamePath) {
	Common::File *file = new Common::File();
	if (!file->open(pathBuild(filename, useGamePath))) {
		delete file;
		return nullptr;
	}

	return file;
}

} // End of namespace Comfy
