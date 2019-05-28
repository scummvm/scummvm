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
#include "talk.h"
#include "dragons.h"

namespace Dragons {

Talk::Talk(DragonsEngine *vm, BigfileArchive *bigfileArchive): _vm(vm), _bigfileArchive(bigfileArchive) {

}

void Talk::init() {

}

char *Talk::loadText(uint32 textIndex) {
	char filename[13] = "drag0000.txt";
	uint32 fileNo = (textIndex >> 0xc) & 0xffff;
	uint32 fileOffset = textIndex & 0xfff;

	sprintf(filename, "drag%04d.txt", fileNo);
	uint32 size;
	byte *data = _bigfileArchive->load(filename, size);
	debug("DIALOG: %s, %d", filename, fileOffset);
	printWideText(data + 10 + fileOffset);
	delete data;
	return NULL; //TODO fix me
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
Talk::FUN_8003239c(char *dialog, int16 x, int16 y, int32 param_4, uint16 param_5, Actor *actor, uint16 startSequenceId,
				   uint16 endSequenceId, uint32 textId) {
	//TODO 0x800323a4

	//TODO dragon_text_related(textId);
	_vm->data_800633fc = 1;
	uint32 uVar4 = 0; //TODO FUN_8001d1ac(0,textId,0);

	actor->updateSequence(startSequenceId);
	//TODO conversation_related_maybe(&dialog[10],(int)x,(int)y,param_4 & 0xffff,(uint)param_5,textId,uVar4 & 0xffff);
	actor->updateSequence(endSequenceId);
}

} // End of namespace Dragons