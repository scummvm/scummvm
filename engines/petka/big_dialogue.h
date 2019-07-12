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

struct DlgOperation {
	uint16 objId;
	byte arg;
	byte code;
};

struct DialogHandler {
	uint32 startOpIndex;
	uint32 opsCount;
	//DlgOperation *operations;
};

struct Dialog {
	uint16 opcode;
	uint16 objId;
	uint32 startHandlerIndex;
	Common::Array<DialogHandler> handlers;
};

struct ObjectDialogs {
	uint32 objId;
	Common::Array<Dialog> dialogs;
};

struct SpeechInfo {
	uint32 speakerId;
	char soundName[16];
	Common::U32String text;
};

class BigDialogue {
public:
	BigDialogue();

	const Dialog *findDialog(uint objId, uint opcode, bool *res) const;
	void setDialog(uint objId, uint opcode, int index);

	const SpeechInfo *getSpeechInfo();

private:
	void loadSpeechesInfo();

private:
	int *_ip;
	int *_code;
	uint _codeSize;
	uint _startCodeIndex;

	Common::Array<SpeechInfo> _speeches;
	Common::Array<ObjectDialogs> _objDialogs;
};

} // End of namespace Petka

#endif
