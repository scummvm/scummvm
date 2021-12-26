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

#ifndef TRECISION_TEXT_H
#define TRECISION_TEXT_H

#define MAXLENSUBSTRING 128
#define MAXSUBSTRING 16

#include "common/scummsys.h"
#include "trecision/struct.h"

namespace Trecision {

class TrecisionEngine;

struct StackText {
	uint16 _x;
	uint16 _y;
	uint16 _textColor;
	Common::String _text;
	bool _clear;
};

class TextManager {
	TrecisionEngine *_vm;

	uint32 _someoneSpeakTime;
	bool _subStringAgain;
	uint32 _talkTime;
	char _subString[MAXSUBSTRING][MAXLENSUBSTRING];
	uint16 _subStringUsed;
	Common::String _superString;
	uint16 _subStringStart;
	uint16 _curSentenceId;
	uint16 _curSubString;
	Common::String _lastFilename;
	uint16 _talkingPersonId;
	SDText _curString;
	SDText _oldString;

	Common::List<StackText> _textStack;

	Common::Point positionString(uint16 x, uint16 y, const char *string, bool characterFl);
	void formattingSuperString();
	void formattingOneString();
	void characterTalk(Common::String s);
	void characterContinueTalk();
	void characterMute();
	void someoneContinueTalk();
	void someoneMute();

public:
	TextManager(TrecisionEngine *vm);
	~TextManager();

	void doString();
	void showObjName(uint16 obj, bool show);
	void someoneSay(uint16 sentence, uint16 person);
	void characterSay(uint16 i);
	void characterSayInAction(uint16 ss);

	void addText(Common::Point pos, const char *text, uint16 textCol);
	void clearLastText();
	void drawText(StackText *text);
	void clearText();
	void drawTexts();
	void redrawString();
	void clearTextStack();
	Common::Rect getOldTextRect() const;
	void clearOldText();

	void drawCurString();
};

} // End of namespace Trecision
#endif

