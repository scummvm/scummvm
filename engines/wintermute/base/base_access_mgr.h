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

/*
 * This file is based on WME.
 * http://dead-code.org/redir.php?target=wme
 * Copyright (c) 2003-2013 Jan Nedoma and contributors
 */

#ifndef WINTERMUTE_BASE_ACCESS_MGR_H
#define WINTERMUTE_BASE_ACCESS_MGR_H

#include "engines/wintermute/base/base.h"
#include "engines/wintermute/dctypes.h"

#include "common/rect.h"
#include "common/text-to-speech.h"

namespace Wintermute {

class BaseObject;

class BaseAccessMgr : public BaseClass {
public:
	BaseAccessMgr(BaseGame *inGame);
	virtual ~BaseAccessMgr();

	bool _ttsAvailable;
	bool initialize();
	bool speak(const char *str, TTTSType type);
	bool stop();
	BaseObject *getNextObject();
	BaseObject *getPrevObject();

	bool initLoop();
	bool displayBeforeGUI();
	bool displayAfterGUI();
	bool setHintRect(Common::Rect32 *hintRect = nullptr, bool afterGUI = false);
	bool setActiveObject(BaseObject *activeObj = nullptr);
	BaseObject *getActiveObject();
	bool _ctrlPressed;

private:
	TTTSType _playingType;
	bool displayInternal();
	Common::TextToSpeechManager *_voice;
	BaseObject *_activeObject;
	BaseObject *_prevActiveObject;
	Common::Rect32 _hintRect;
	bool _hintAfterGUI;
};

#endif

} // End of namespace Wintermute
