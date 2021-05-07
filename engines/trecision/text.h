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
private:
	TrecisionEngine *_vm;

	uint32 _someoneSpeakTime;
	uint16 SuperStringLen;
	bool substringagain;
	uint32 TalkTime;
	uint16 SpeakSomeOneAnimation;
	char SubString[MAXSUBSTRING][MAXLENSUBSTRING];
	uint16 SubStringUsed;
	const char *SuperString;
	uint16 SubStringStart;
	uint16 CurS;
	uint16 CurSubString;
	char sn[13];
	uint16 SpeakSomeOnePerson;

	void PositionString(uint16 x, uint16 y, const char *string, uint16 *posx, uint16 *posy, bool characterFl);
	void FormattingSuperString();
	void FormattingOneString();
	void CharacterTalk(const char *s);
	void CharacterContinueTalk();
	void CharacterMute();
	void SomeoneContinueTalk();
	void someoneMute();

public:
	TextManager(TrecisionEngine *vm);
	~TextManager();

	void doString();
	void ShowObjName(uint16 obj, bool showhide);
	void SomeoneTalk(uint16 s, uint16 Person, uint16 NewAnim);
	void CharacterSay(uint16 i);
	void CharacterTalkInAction(uint16 ss);
}; // end of class

} // end of namespace
#endif

