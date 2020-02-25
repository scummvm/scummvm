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

#ifndef DRAGONS_TALK_H
#define DRAGONS_TALK_H

#include <common/str.h>
#include "scriptopcodes.h"

namespace Dragons {

class BigfileArchive;
class Actor;
class DragonsEngine;

struct TalkDialogEntry {
	char dialogText[600];
	uint32 textIndex;
	uint32 textIndex1;
	byte *scriptCodeStartPtr;
	byte *scriptCodeEndPtr;
	uint16 flags;
	uint8 xPosMaybe;
	uint8 yPosMaybe;
	int16 field_26c;
	uint16 iniId;
};


class Talk {
private:
	DragonsEngine *_vm;
	BigfileArchive *_bigfileArchive;
	Common::List<TalkDialogEntry*> _dialogEntries;
	uint32 defaultResponseTbl[45];

public:
	Talk(DragonsEngine *vm, BigfileArchive *bigfileArchive);
	void init();
	void loadText(uint32 textIndex, uint16 *textBuffer, uint16 bufferLength);
	void printWideText(byte *text);

	void talkFromIni(uint32 iniId, uint32 textIndex);
	void flickerRandomDefaultResponse();

	uint32 displayDialogAroundINI(uint32 iniId, uint16 *dialogText, uint32 textIndex);
	void displayDialogAroundPoint(uint16 *dialogText, uint16 x, uint16 y, uint16 param_4, int16 param_5, uint32 textId);
	void displayDialogAroundActor(Actor *actor, uint16 param_2, uint16 *dialogText, uint32 textIndex);

	void FUN_8003239c(uint16 *dialog, int16 x, int16 y, int32 param_4, uint16 param_5, Actor *actor, uint16 startSequenceId, uint16 endSequenceId, uint32 textId);

	void conversation_related_maybe(uint16 *dialogText, uint16 x, uint16 y, uint16 param_4, int16 param_5, uint32 textId, int16 param_7);

	void addTalkDialogEntry(TalkDialogEntry *talkDialogEntry);

	bool talkToActor(ScriptOpCall &scriptOpCall);

	uint somethingTextAndSpeechAndAnimRelated(Actor *actor,int16 sequenceId1,int16 sequenceId2,uint32 textIndex, uint16 param_5);
private:
	void copyTextToBuffer(uint16 *destBuffer, byte *src, uint32 destBufferLength);
	uint32 wideStrLen(uint16 *text);
	TalkDialogEntry *displayTalkDialogMenu();
	void exitTalkMenu(bool isFlag8Set, bool isFlag100Set);
	uint32 getDefaultResponseTextIndex();
	void initDefaultResponseTable();
};

} // End of namespace Dragons

#endif //DRAGONS_TALK_H
