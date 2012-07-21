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

#ifndef WINTERMUTE_ADSENTENCE_H
#define WINTERMUTE_ADSENTENCE_H


#include "engines/wintermute/base/base.h"
#include "engines/wintermute/persistent.h"
#include "engines/wintermute/math/rect32.h"
#include "engines/wintermute/dctypes.h"    // Added by ClassView
#include "common/rect.h"

namespace WinterMute {
class CAdTalkDef;
class CBFont;
class CBSprite;
class CBSound;
class CAdSentence : public CBBase {
public:
	bool _freezable;
	bool _fixedPos;
	CBSprite *_currentSprite;
	char *_currentSkelAnim;
	bool update(TDirection dir = DI_DOWN);
	bool setupTalkFile(const char *soundFilename);
	DECLARE_PERSISTENT(CAdSentence, CBBase)
	bool finish();
	void setSound(CBSound *Sound);
	bool _soundStarted;
	CBSound *_sound;
	TTextAlign _align;
	bool display();
	int _width;
	Point32 _pos;
	CBFont *_font;
	char *getNextStance();
	char *getCurrentStance();
	void setStances(const char *stances);
	void setText(const char *text);
	int _currentStance;
	uint32 _startTime;
	char *_stances;
	char *_text;
	uint32 _duration;
	CAdSentence(CBGame *inGame);
	virtual ~CAdSentence();
	CAdTalkDef *_talkDef;

	bool CanSkip();

private:
	char *_tempStance;
	char *getStance(int stance);

};

} // end of namespace WinterMute

#endif
