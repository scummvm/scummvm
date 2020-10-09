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

#ifndef PETKA_BIG_DIALOGUE_H
#define PETKA_BIG_DIALOGUE_H

#include "common/array.h"
#include "common/ustr.h"

namespace Petka {

class PetkaEngine;

enum {
	kOpcodePlay = 1,
	kOpcodeMenu,
	kOpcodeEnd,
	kOpcodeUserMessage
};

enum {
	kOperationBreak = 1,
	kOperationMenu,
	kOperationGoTo,
	kOperationDisableMenuItem,
	kOperationEnableMenuItem,
	kOperationReturn,
	kOperationPlay,
	kOperationCircle,
	kOperationUserMessage
};

struct Operation {
	union {
		struct {
			byte bits;
			uint16 bitField;
		} menu;
		struct {
			uint16 opIndex;
		} goTo;
		struct {
			uint16 opIndex;
			byte bit;
		} disableMenuItem;
		struct {
			uint16 opIndex;
			byte bit;
		} enableMenuItem;
		struct {
			uint16 messageIndex;
		} play;
		struct {
			uint16 count;
			byte curr;
		} circle;
		struct {
			uint16 arg;
		} userMsg;
	};
	byte type;
};

struct Dialog {
	uint32 startOpIndex;
	// uint32 opsCount;
	// Operation *ops;
};

struct DialogHandler {
	uint32 opcode;
	uint32 startDialogIndex;
	Common::Array<Dialog> dialogs;
};

struct DialogGroup {
	uint32 objId;
	Common::Array<DialogHandler> handlers;
};

struct SpeechInfo {
	uint32 speakerId;
	char soundName[16];
	Common::U32String text;
};

class BigDialogue {
public:
	BigDialogue(PetkaEngine &vm);

	uint opcode();

	uint choicesCount();

	void next(int choice = -1);

	const DialogHandler *findHandler(uint objId, uint opcode, bool *fallback) const;
	void setHandler(uint objId, uint opcode);

	const Common::U32String *getSpeechInfo(int *talkerId, const char **soundName, int choice);
	void getMenuChoices(Common::Array<Common::U32String> &choices);

	void load(Common::ReadStream *s);
	void save(Common::WriteStream *s);

private:
	void loadSpeechesInfo();
	bool checkMenu(uint opIndex);
	bool findOperation(uint startOpIndex, uint opType, uint *resIndex);
	void circleMoveTo(byte index);

private:
	PetkaEngine &_vm;

	Operation *_currOp;
	Common::Array<Operation> _ops;
	uint _startOpIndex;

	Common::Array<SpeechInfo> _speeches;
	Common::Array<DialogGroup> _objDialogs;
};

} // End of namespace Petka

#endif
