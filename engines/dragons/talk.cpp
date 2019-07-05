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
#include "actorresource.h"
#include "talk.h"
#include "sound.h"
#include "dragons.h"
#include "dragonini.h"
#include "dragonimg.h"
#include "dragonobd.h"
#include "scene.h"
#include "font.h"

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
	debug("DIALOG: %s, %d", filename, fileOffset);
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
		conversation_related_maybe(dialogText + 5,x,y,param_4,param_5,textId,0); // sVar3); TODO I think this is audio status
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


} // End of namespace Dragons