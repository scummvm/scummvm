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

#ifndef TRECISION_TEXT_H
#define TRECISION_TEXT_H

#include "trecision/defines.h"
#include "trecision/trecision.h"

namespace Trecision {

class TextManager {
	TrecisionEngine *_vm;

	uint32 _someoneSpeakTime;
	uint16 _superStringLen;
	bool _subStringAgain;
	uint32 _talkTime;
	uint16 _talkingPersonAnimId;
	char _subString[MAXSUBSTRING][MAXLENSUBSTRING];
	uint16 _subStringUsed;
	const char *_superString;
	uint16 _subStringStart;
	uint16 _curSentenceId;
	uint16 _curSubString;
	Common::String _lastFilename;
	uint16 _talkingPersonId;
	SDText _curString;
	SDText _oldString;
	
	Common::List<StackText> _textStack;

	void positionString(uint16 x, uint16 y, const char *string, uint16 *posx, uint16 *posy, bool characterFl);
	void formattingSuperString();
	void formattingOneString();
	void characterTalk(const char *s);
	void characterContinueTalk();
	void characterMute();
	void someoneContinueTalk();
	void someoneMute();

public:
	TextManager(TrecisionEngine *vm);
	~TextManager();

	void doString();
	void showObjName(uint16 obj, bool show);
	void someoneSay(uint16 s, uint16 Person, uint16 NewAnim);
	void characterSay(uint16 i);
	void characterSayInAction(uint16 ss);

	void addText(uint16 x, uint16 y, const char *text, uint16 tcol, uint16 scol);
	void clearLastText();
	void drawText(StackText text);
	void clearText();
	void drawTexts();	
	void redrawString();
	void clearTextStack();
	Common::Rect getOldTextRect() const;
	void clearOldText();
	
	void drawCurString();
}; // end of class

} // end of namespace
#endif

