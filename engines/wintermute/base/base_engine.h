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

#ifndef WINTERMUTE_BASE_ENGINE_H
#define WINTERMUTE_BASE_ENGINE_H

#include "common/str.h"
#include "common/singleton.h"
#include "common/random.h"
#include "common/language.h"

#include "engines/wintermute/detection.h"

namespace Wintermute {

class BaseFileManager;
class BaseRegistry;
class BaseGame;
class BaseSoundMgr;
class BaseRenderer;
class SystemClassRegistry;
class Timer;
class BaseEngine : public Common::Singleton<Wintermute::BaseEngine> {
	void init();
	BaseFileManager *_fileManager;
	Common::String _gameId;
	Common::String _targetName;
	BaseGame *_gameRef;
	// We need random numbers
	Common::RandomSource *_rnd;
	SystemClassRegistry *_classReg;
	Common::Language _language;
	WMETargetExecutable _targetExecutable;
	uint32 _flags;
public:
	BaseEngine();
	~BaseEngine() override;
	static void createInstance(const Common::String &targetName, const Common::String &gameId, Common::Language lang, WMETargetExecutable targetExecutable, uint32 flags);

	void setGameRef(BaseGame *gameRef) { _gameRef = gameRef; }

	Common::RandomSource *getRandomSource() { return _rnd; }
	uint32 randInt(int from, int to);

	SystemClassRegistry *getClassRegistry() { return _classReg; }
	BaseGame *getGameRef() { return _gameRef; }
	BaseFileManager *getFileManager() { return _fileManager; }
	BaseSoundMgr *getSoundMgr();
	static BaseRenderer *getRenderer();
	static const Timer *getTimer();
	static const Timer *getLiveTimer();
	static void LOG(bool res, const char *fmt, ...);
	Common::String getGameTargetName() const { return _targetName; }
	Common::String getGameId() const { return _gameId; }
	Common::Language getLanguage() const { return _language; }
	uint32 getFlags() const { return _flags; }
	WMETargetExecutable getTargetExecutable() const {
		return _targetExecutable;
	}
	static bool isFoxTailCheck(WMETargetExecutable t, WMETargetExecutable v1=FOXTAIL_OLDEST_VERSION, WMETargetExecutable v2=FOXTAIL_LATEST_VERSION) {
		return t >= v1 && t <= v2;
	}
	bool isFoxTail(WMETargetExecutable v1=FOXTAIL_OLDEST_VERSION, WMETargetExecutable v2=FOXTAIL_LATEST_VERSION) const {
		return isFoxTailCheck(_targetExecutable, v1, v2);
	}
	void addFlags(uint32 flags) { _flags |= flags; }
};

} // End of namespace Wintermute

#endif
