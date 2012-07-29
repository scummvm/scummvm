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
 */

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */
 
 #include "common/str.h"
 #include "common/singleton.h"

namespace WinterMute {

class BaseFileManager;
class BaseRegistry;
class BaseGame;
class BaseEngine : public Common::Singleton<WinterMute::BaseEngine> {
	void init();
	BaseFileManager *_fileManager;
	BaseRegistry *_registry;
	Common::String _gameId;
	BaseGame *_gameRef;
public:
	BaseEngine();
	~BaseEngine();
	static void createInstance(const Common::String &gameid);
	void setGameRef(BaseGame *gameRef) { _gameRef = gameRef; }

	BaseGame *getGameRef() { return _gameRef; }
	BaseFileManager *getFileManager() { return _fileManager; }
	BaseRegistry *getRegistry() { return _registry; }
	static void LOG(bool res, const char *fmt, ...);
	const char *getGameId() { return _gameId.c_str(); }
};

} // end of namespace WinterMute
