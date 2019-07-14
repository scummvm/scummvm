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

#include <common/debug.h>
#include "bigfile.h"
#include "actor.h"
#include "cursor.h"
#include "actorresource.h"
#include "talk.h"
#include "sound.h"
#include "dragons.h"
#include "dragonini.h"
#include "dragonimg.h"
#include "dragonobd.h"
#include "scene.h"
#include "font.h"
#include "scriptopcodes.h"

namespace Dragons {

Talk::Talk(DragonsEngine *vm, BigfileArchive *bigfileArchive): _vm(vm), _bigfileArchive(bigfileArchive) {

}

void Talk::init() {

}

void Talk::loadText(uint32 textIndex, uint16 *textBuffer, uint16 bufferLength) {
	char filename[13] = "drag0000.txt";
	uint32 fileNo = (textIndex >> 0xc) & 0xffff;
	uint32 fileOffset = textIndex & 0xfff;

	sprintf(filename, "drag%04d.txt", fileNo);
	uint32 size;
	byte *data = _bigfileArchive->load(filename, size);
	debug("DIALOG: %s, %s, %d", filename, data, fileOffset);
	printWideText(data + 10 + fileOffset);

	copyTextToBuffer(textBuffer, data + 10 + fileOffset, bufferLength);
	delete data;
}

void Talk::printWideText(byte *text) {
	char buf[2000];
	int i = 0;
	for (; READ_LE_INT16(text) != 0 && i < 1999; i++) {
		buf[i] = *text;
		text += 2;
	}
	buf[i] = 0;
	debug("TEXT: %s", buf);
}

void
Talk::FUN_8003239c(uint16 *dialog, int16 x, int16 y, int32 param_4, uint16 param_5, Actor *actor, uint16 startSequenceId,
				   uint16 endSequenceId, uint32 textId) {
	//TODO 0x800323a4

	//TODO dragon_text_related(textId);
	_vm->data_800633fc = 1;
	uint32 uVar4 = 0; //TODO FUN_8001d1ac(0,textId,0);

	actor->updateSequence(startSequenceId);
	_vm->_sound->playSpeech(textId);
	conversation_related_maybe(dialog, (int)x, (int)y,param_4 & 0xffff, (uint)param_5, textId, uVar4 & 0xffff);

	actor->updateSequence(endSequenceId);
}

void
Talk::conversation_related_maybe(uint16 *dialogText, uint16 x, uint16 y, uint16 param_4, int16 param_5, uint32 textId,
								 int16 param_7) {
	//TODO display dialog text here while we wait for audio stream to complete.
	_vm->_fontManager->addText(x, y, dialogText, wideStrLen(dialogText), 0);
	while (_vm->isFlagSet(ENGINE_FLAG_8000)) {
		_vm->waitForFrames(1);
	}
	_vm->_fontManager->clearText();
}


uint32 Talk::displayDialogAroundINI(uint32 iniId, uint16 *dialogText, uint32 textIndex)

{
	DragonINI *ini = iniId == 0 ? _vm->_dragonINIResource->getFlickerRecord() : _vm->getINI(iniId - 1);

	if ((ini->field_1a_flags_maybe & 1) == 0) {
		IMG *local_v1_184 = _vm->_dragonIMG->getIMG(ini->field_2);
		int x, y;
		if (local_v1_184->field_e == 0) {
			y = (uint)(ushort)local_v1_184->y;
			x = local_v1_184->field_a;
		}
		else {
			x = local_v1_184->field_a;
			y = (uint)(ushort)local_v1_184->y << 3;
		}
		displayDialogAroundPoint
				(dialogText,
						((x - _vm->_scene->_camera.x) * 0x10000) >> 0x13,
				 ((y - _vm->_scene->_camera.y) * 0x10000) >> 0x13,
				 READ_LE_UINT16(_vm->_dragonOBD->getFromOpt(ini->id) + 6)
						,1,textIndex);
	}
	else {
		displayDialogAroundActor
				(ini->actor,
				 READ_LE_UINT16(_vm->_dragonOBD->getFromOpt(ini->id) + 6),
				 dialogText,textIndex);
	}
	return 1;
}

void
Talk::displayDialogAroundPoint(uint16 *dialogText, uint16 x, uint16 y, uint16 param_4, int16 param_5, uint32 textId) {
// TODO
//
//		puVar1 = &DAT_80011a60;
//		puVar2 = local_58;
//		do {
//			puVar8 = puVar2;
//			puVar7 = puVar1;
//			uVar4 = puVar7[1];
//			uVar5 = puVar7[2];
//			uVar6 = puVar7[3];
//			*puVar8 = *puVar7;
//			puVar8[1] = uVar4;
//			puVar8[2] = uVar5;
//			puVar8[3] = uVar6;
//			puVar1 = puVar7 + 4;
//			puVar2 = puVar8 + 4;
//		} while (puVar7 + 4 != (undefined4 *)&DAT_80011a80);
//		uVar4 = puVar7[5];
//		puVar8[4] = _DAT_80011a80;
//		puVar8[5] = uVar4;
		_vm->data_800633fc = 1;

		// sVar3 = FUN_8001d1ac(0,textId,0);
		_vm->_sound->playSpeech(textId);

//		if (dialogText == (uint16_t *)0x0) {
//			dialogText = (uint16_t *)local_58;
//		}
		conversation_related_maybe(dialogText,x,y,param_4,param_5,textId,0); // sVar3); TODO I think this is audio status
}

void Talk::displayDialogAroundActor(Actor *actor, uint16 param_2, uint16 *dialogText, uint32 textIndex) {
	displayDialogAroundPoint
			(dialogText,(ushort)((int)(((uint)actor->x_pos - _vm->_scene->_camera.x) * 0x10000) >> 0x13),
			 (short)((int)((((uint)actor->y_pos - (uint)actor->frame->yOffset) - (uint)_vm->_scene->_camera.y) * 0x10000) >> 0x13) - 3,
			 param_2,1,textIndex);
}

void Talk::copyTextToBuffer(uint16 *destBuffer, byte *src, uint32 destBufferLength) {
	for(int i = 0; i < destBufferLength - 1; i++) {
		destBuffer[i] = READ_LE_UINT16(src);
		src += 2;
		if(destBuffer[i] == 0) {
			return;
		}
	}

	destBuffer[destBufferLength - 1] = 0;
}

uint32 Talk::wideStrLen(uint16 *text) {
	int i = 0;
	while(text[i] != 0) {
		i++;
	}
	return i;
}

void Talk::addTalkDialogEntry(TalkDialogEntry *talkDialogEntry) {
	_dialogEntries.push_back(talkDialogEntry);
}

void callMaybeResetData() {
	//TODO do we need this?
}

bool Talk::talkToActor(ScriptOpCall &scriptOpCall) {
	uint16 numEntries;

	uint16_t sequenceId;
	TalkDialogEntry *selectedDialogText;
	uint iniId;
	ScriptOpCall local_1d20;
	short local_990 [5];
	byte auStack2438 [390];
	short local_800 [1000];

	bool isFlag8Set = _vm->isFlagSet(ENGINE_FLAG_8);
	bool isFlag100Set = _vm->isFlagSet(ENGINE_FLAG_100);

	_vm->clearFlags(ENGINE_FLAG_8);
	Actor *flickerActor = _vm->_dragonINIResource->getFlickerRecord()->actor;

	//TODO clear entries;
	_vm->_scriptOpcodes->loadTalkDialogEntries(scriptOpCall);
	numEntries = _dialogEntries.size();
	if (numEntries == 0) {
		return 0;
	}
	_vm->setFlags(ENGINE_FLAG_100);
	do {
		callMaybeResetData();
		int numActiveDialogEntries = 0;
		for (Common::List<TalkDialogEntry*>::iterator it = _dialogEntries.begin(); it != _dialogEntries.end(); it++) {
			if (!(*it)->flags & 1) {
				numActiveDialogEntries++;
			}
		}
		if (numActiveDialogEntries == 0) {
			//TODO logic from LAB_80029bc0 reset cursor
			exitTalkMenu(isFlag8Set, isFlag100Set);
			return 1;
		}

		selectedDialogText = displayTalkDialogMenu();
		if (selectedDialogText == NULL) {
			callMaybeResetData();
			exitTalkMenu(isFlag8Set, isFlag100Set);
			return 1;
		}
		_vm->clearFlags(ENGINE_FLAG_8);
		strcpy((char *)local_990,selectedDialogText->dialogText);
//		UTF16ToUTF16Z(auStack2438,selectedDialogText->dialogText + 10);
//		load_string_from_dragon_txt(selectedDialogText->textIndex1,(char *)local_800);
//		if (local_990[0] != 0) {
//			flickerActor->setFlag(ACTOR_FLAG_2000);
//			sequenceId = flickerActor->_sequenceID;
//			playSoundFromTxtIndex(selectedDialogText->textIndex);
//			if (flickerActor->_sequenceID2 != -1) {
//				flickerActor->updateSequence(flickerActor->_sequenceID2 + 0x10);
//			}
//			displayDialogAroundINI(0,(uint8_t *)local_990,selectedDialogText->textIndex);
//			flickerActor->updateSequence(sequenceId);
//			flickerActor->clearFlag(ACTOR_FLAG_2000);
//		}
		if ((selectedDialogText->flags & 2) == 0) {
			selectedDialogText->flags = selectedDialogText->flags | 1;
		}
		callMaybeResetData();
		if (local_800[0] != 0) {
			if (selectedDialogText->field_26c == -1) {
//				displayDialogAroundINI
//						((uint)dragon_ini_index_under_active_cursor,(uint8_t *)local_800,
//						 selectedDialogText->textIndex1);
			}
			else {
				iniId = _vm->_cursor->_iniUnderCursor; //dragon_ini_index_under_active_cursor;
				if (selectedDialogText->iniId != 0) {
					iniId = selectedDialogText->iniId;
				}
				Actor *iniActor = _vm->_dragonINIResource->getRecord(iniId - 1)->actor;
				sequenceId = iniActor->_sequenceID;
//				playSoundFromTxtIndex(selectedDialogText->textIndex1);
//				iniActor->updateSequence(selectedDialogText->field_26c);
//				displayDialogAroundINI(iniId,(uint8_t *)local_800,selectedDialogText->textIndex1);
//				iniActor->updateSequence(sequenceId);
			}
		}
		local_1d20._code = selectedDialogText->scriptCodeStartPtr;
		local_1d20._codeEnd = selectedDialogText->scriptCodeEndPtr;
		_vm->_scriptOpcodes->runScript(local_1d20);
		if (_vm->_scriptOpcodes->_data_80071f5c != 0) break;
		local_1d20._code = selectedDialogText->scriptCodeStartPtr;
		local_1d20._codeEnd = selectedDialogText->scriptCodeEndPtr;
		talkToActor(local_1d20);

	} while (_vm->_scriptOpcodes->_data_80071f5c == 0);
	_vm->_scriptOpcodes->_data_80071f5c--;
//	LAB_80029bc0:
//	actors[0].x_pos = cursor_x_var;
//	actors[0].y_pos = cursor_y_var;
	exitTalkMenu(isFlag8Set, isFlag100Set);
	return 1;
}

TalkDialogEntry *Talk::displayTalkDialogMenu() {
	return *_dialogEntries.begin();
}

void Talk::exitTalkMenu(bool isFlag8Set, bool isFlag100Set) {
	_vm->clearFlags(ENGINE_FLAG_8);
	_vm->clearFlags(ENGINE_FLAG_100);

	if (isFlag8Set) {
		_vm->setFlags(ENGINE_FLAG_8);
	}
	if (isFlag100Set) {
		_vm->setFlags(ENGINE_FLAG_100);
	}

}


} // End of namespace Dragons