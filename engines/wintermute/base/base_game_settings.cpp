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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/base/base_game_settings.h"
#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/base_string_table.h"
#include "engines/wintermute/base/base_parser.h"
#include "engines/wintermute/wintypes.h"

namespace Wintermute {

BaseGameSettings::BaseGameSettings(BaseGame *gameRef) {
	_resWidth = 800;
	_resHeight = 600;
	_requireAcceleration = false;
	_requireSound = false;
	_TLMode = 0;
	_allowWindowed = true;
	_gameFile = nullptr;
	_allowAdvanced = false;
	_allowAccessTab = true;
	_allowAboutTab = true;
	_allowDesktopRes = false;

	_compressedSavegames = true;
	_richSavedGames = false;
	_savedGameExt = "dsv";

	_stringTable = new BaseStringTable(gameRef);
}

BaseGameSettings::~BaseGameSettings() {
	delete[] _gameFile;
	_gameFile = nullptr;
	delete _stringTable;
	_stringTable = nullptr;
}

TOKEN_DEF_START
TOKEN_DEF(GAME)
TOKEN_DEF(STRING_TABLE)
TOKEN_DEF(RESOLUTION)
TOKEN_DEF(SETTINGS)
TOKEN_DEF(REQUIRE_3D_ACCELERATION)
TOKEN_DEF(REQUIRE_SOUND)
TOKEN_DEF(HWTL_MODE)
TOKEN_DEF(ALLOW_WINDOWED_MODE)
TOKEN_DEF(ALLOW_ACCESSIBILITY_TAB)
TOKEN_DEF(ALLOW_ABOUT_TAB)
TOKEN_DEF(ALLOW_ADVANCED)
TOKEN_DEF(ALLOW_DESKTOP_RES)
TOKEN_DEF(REGISTRY_PATH)
TOKEN_DEF(RICH_SAVED_GAMES)
TOKEN_DEF(SAVED_GAME_EXT)
TOKEN_DEF(GUID)
TOKEN_DEF_END

//////////////////////////////////////////////////////////////////////////
bool BaseGameSettings::loadSettings(const char *filename) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(SETTINGS)
	TOKEN_TABLE(GAME)
	TOKEN_TABLE(STRING_TABLE)
	TOKEN_TABLE(RESOLUTION)
	TOKEN_TABLE(REQUIRE_3D_ACCELERATION)
	TOKEN_TABLE(REQUIRE_SOUND)
	TOKEN_TABLE(HWTL_MODE)
	TOKEN_TABLE(ALLOW_WINDOWED_MODE)
	TOKEN_TABLE(ALLOW_ACCESSIBILITY_TAB)
	TOKEN_TABLE(ALLOW_ABOUT_TAB)
	TOKEN_TABLE(ALLOW_ADVANCED)
	TOKEN_TABLE(ALLOW_DESKTOP_RES)
	TOKEN_TABLE(REGISTRY_PATH)
	TOKEN_TABLE(RICH_SAVED_GAMES)
	TOKEN_TABLE(SAVED_GAME_EXT)
	TOKEN_TABLE(GUID)
	TOKEN_TABLE_END


	char *origBuffer = (char *)BaseFileManager::getEngineInstance()->readWholeFile(filename);
	if (origBuffer == nullptr) {
		BaseEngine::LOG(0, "BaseGame::LoadSettings failed for file '%s'", filename);
		return STATUS_FAILED;
	}

	bool ret = STATUS_OK;

	char *buffer = origBuffer;
	char *params;
	int cmd;
	BaseParser parser;

	if (parser.getCommand(&buffer, commands, &params) != TOKEN_SETTINGS) {
		BaseEngine::LOG(0, "'SETTINGS' keyword expected in game settings file.");
		return STATUS_FAILED;
	}
	buffer = params;
	while ((cmd = parser.getCommand(&buffer, commands, &params)) > 0) {
		switch (cmd) {
			case TOKEN_GAME:
				delete[] _gameFile;
				_gameFile = new char[strlen(params) + 1];
				if (_gameFile) {
					strcpy(_gameFile, params);
				}
				break;

			case TOKEN_STRING_TABLE:
				if (DID_FAIL(_stringTable->loadFile(params))) {
					cmd = PARSERR_GENERIC;
				}
				break;

			case TOKEN_RESOLUTION:
				parser.scanStr(params, "%d,%d", &_resWidth, &_resHeight);
				break;

			case TOKEN_REQUIRE_3D_ACCELERATION:
				parser.scanStr(params, "%b", &_requireAcceleration);
				break;

			case TOKEN_REQUIRE_SOUND:
				parser.scanStr(params, "%b", &_requireSound);
				break;

			case TOKEN_HWTL_MODE:
				parser.scanStr(params, "%d", &_TLMode);
				break;

			case TOKEN_ALLOW_WINDOWED_MODE:
				parser.scanStr(params, "%b", &_allowWindowed);
				break;

			case TOKEN_ALLOW_DESKTOP_RES:
				parser.scanStr(params, "%b", &_allowDesktopRes);
				break;

			case TOKEN_ALLOW_ADVANCED:
				parser.scanStr(params, "%b", &_allowAdvanced);
				break;

			case TOKEN_ALLOW_ACCESSIBILITY_TAB:
				parser.scanStr(params, "%b", &_allowAccessTab);
				break;

			case TOKEN_ALLOW_ABOUT_TAB:
				parser.scanStr(params, "%b", &_allowAboutTab);
				break;

			case TOKEN_REGISTRY_PATH:
				//BaseEngine::instance().getRegistry()->setBasePath(params);
				break;

			case TOKEN_RICH_SAVED_GAMES:
				parser.scanStr(params, "%b", &_richSavedGames);
				break;

			case TOKEN_SAVED_GAME_EXT:
				_savedGameExt = params;
				break;

			case TOKEN_GUID:
				break;

			default:
				break;
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		BaseEngine::LOG(0, "Syntax error in game settings '%s'", filename);
		ret = STATUS_FAILED;
	}
	if (cmd == PARSERR_GENERIC) {
		BaseEngine::LOG(0, "Error loading game settings '%s'", filename);
		ret = STATUS_FAILED;
	}

	_allowWindowed = true; // TODO: These two settings should probably be cleaned out altogether.
	_compressedSavegames = true;

	delete[] origBuffer;

	return ret;
}

bool BaseGameSettings::loadStringTable(const char *filename, bool clearOld) {
	return _stringTable->loadFile(filename, clearOld);
}

//////////////////////////////////////////////////////////////////////////
void BaseGameSettings::expandStringByStringTable(char **str) const {
	_stringTable->expand(str);
}

//////////////////////////////////////////////////////////////////////////
void BaseGameSettings::expandStringByStringTable(Common::String &str) const {
	_stringTable->expand(str);
}

char *BaseGameSettings::getKeyFromStringTable(const char *str) const {
	return _stringTable->getKey(str);
}

bool BaseGameSettings::persist(BasePersistenceManager *persistMgr) {
	return _stringTable->persist(persistMgr);
}

} // End of namespace Wintermute
