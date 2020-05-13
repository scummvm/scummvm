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

namespace Wintermute {

enum WMETargetExecutable {
	OLDEST_VERSION,
	WME_1_0_12, // DEAD:CODE 2003
	WME_1_0_19, // DEAD:CODE 2003
	WME_1_0_20, // DEAD:CODE 2003
	WME_1_0_22, // DEAD:CODE 2003
	WME_1_0_24, // DEAD:CODE 2003
	WME_1_0_25, // DEAD:CODE 2003
	WME_1_0_28, // DEAD:CODE 2003
	WME_1_0_30, // DEAD:CODE 2003
	WME_1_0_31, // DEAD:CODE 2003
	WME_1_1_33, // DEAD:CODE 2003
	WME_1_1_35, // DEAD:CODE 2003
	WME_1_1_37, // DEAD:CODE 2003
	WME_1_1_39, // DEAD:CODE 2004
	WME_1_2_43, // DEAD:CODE 2004
	WME_1_2_44, // DEAD:CODE 2004
	WME_1_3_0,  // DEAD:CODE 2004
	WME_1_3_2,  // DEAD:CODE 2004
	WME_1_3_3,  // DEAD:CODE 2004
	WME_1_4_0,  // DEAD:CODE 2005
	WME_1_4_1,  // DEAD:CODE 2005
	WME_1_5_0,  // DEAD:CODE 2005
	WME_1_5_2,  // DEAD:CODE 2005
	WME_1_6_0,  // DEAD:CODE 2006
	WME_1_6_1,  // DEAD:CODE 2006
	WME_1_7_0,  // DEAD:CODE 2007
	WME_1_7_1,  // DEAD:CODE 2007
	WME_1_7_2,  // DEAD:CODE 2007
	WME_1_7_3,  // DEAD:CODE 2007
	WME_1_7_94, // DEAD:CODE 2007
	WME_1_8_0,  // DEAD:CODE 2007
	WME_1_8_1,  // DEAD:CODE 2007
	WME_1_8_2,  // DEAD:CODE 2008
	WME_1_8_3,  // DEAD:CODE 2008
	WME_1_8_4,  // DEAD:CODE 2008
	WME_1_8_5,  // DEAD:CODE 2008
	WME_1_8_6,  // DEAD:CODE 2008
	WME_1_8_7,  // DEAD:CODE 2008, released as "1.8.7 beta"
	WME_1_8_8,  // DEAD:CODE 2008, released as "1.8.8 beta"
	WME_1_8_9,  // DEAD:CODE 2008, released as "1.8.9 beta"
	WME_1_8_10, // DEAD:CODE 2009
	WME_1_8_11, // DEAD:CODE 2009
	WME_1_9_0,  // DEAD:CODE 2009, released as "1.9.0 beta"

	// fork of WME_1_9_0
	WME_KINJAL_1_0,
	WME_KINJAL_1_1,
	WME_KINJAL_1_2,
	WME_KINJAL_1_3,
	WME_KINJAL_1_4,

	// fork of WME_KINJAL_1_4
	WME_HEROCRAFT,

	WME_1_9_1,  // DEAD:CODE 2010

	// fork of WME_1_9_1
	WME_KINJAL_1_5,
	WME_KINJAL_1_6,
	WME_KINJAL_1_7,
	WME_KINJAL_1_7a,
	WME_KINJAL_1_7b,
	WME_KINJAL_1_8,
	WME_KINJAL_1_9,
	WME_KINJAL_2_0,

	WME_1_9_2,  // DEAD:CODE 2010
	WME_1_9_3,  // DEAD:CODE 2012, released as "1.10.1 beta"
	WME_LITE,
	LATEST_VERSION,

	// fork of WME_LITE
	FOXTAIL_OLDEST_VERSION,
	FOXTAIL_1_2_227,
	FOXTAIL_1_2_230,
	FOXTAIL_1_2_304,
	FOXTAIL_1_2_362,
	FOXTAIL_1_2_527,
	FOXTAIL_LATEST_VERSION
};

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
};

} // End of namespace Wintermute

#endif
