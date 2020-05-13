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

#ifndef WINTERMUTE_BASE_GAME_SETTINGS_H
#define WINTERMUTE_BASE_GAME_SETTINGS_H

#include "common/str.h"

namespace Wintermute {
class BaseStringTable;
class BaseGame;
class BasePersistenceManager;
class BaseGameSettings {
public:
	const char *getGameFile() const { return (_gameFile ? _gameFile : "default.game"); }
	int getResWidth() const { return _resWidth; }
	int getResHeight() const { return _resHeight; }

	BaseGameSettings(BaseGame *gameRef);
	~BaseGameSettings();
	bool loadSettings(const char *filename);
	bool loadStringTable(const char *filename, bool clearOld);
	void expandStringByStringTable(char **str) const;
	void expandStringByStringTable(Common::String &str) const;
	char *getKeyFromStringTable(const char *str) const;

	bool persist(BasePersistenceManager *persistMgr);
private:
	char *_gameFile;
	int _resWidth;
	int _resHeight;
	BaseStringTable *_stringTable;
// Not ever used:
	int _TLMode;
	bool _compressedSavegames;
	Common::String _savedGameExt;
	bool _requireAcceleration;
	bool _allowWindowed;
	bool _allowAdvanced;
	bool _allowAccessTab;
	bool _allowAboutTab;
	bool _requireSound;
	bool _allowDesktopRes;
// TODO: This can probably be removed completely:
	bool _richSavedGames;
};

} // End of namespace Wintermute

#endif
