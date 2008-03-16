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
 * $URL$
 * $Id$
 *
 */

#include "kyra/text_v2.h"
#include "kyra/kyra_v2.h"
#include "kyra/sound.h"
#include "kyra/resource.h"

#include "common/endian.h"

namespace Kyra {

uint8 *KyraEngine_v2::tim_loadFile(const char *filename, byte *buffer, int32 bufferSize) {
	ScriptFileParser file(filename, _res);
	if (!file) {
		error("Couldn't open script file '%s'", filename);
		return NULL;
	}

	int32 formBlockSize = file.getFORMBlockSize();
	if (formBlockSize == -1) {
		error("No FORM chunk found in file: '%s'", filename);
		return NULL;
	}

	if (formBlockSize < 20) {
		return NULL;
	}

	formBlockSize += sizeof(TIMHeader) +  10 * (sizeof(Cmds) + 120);

	TIMHeader *timHeader;
	if (buffer == NULL || bufferSize < formBlockSize) {
		buffer = new byte[formBlockSize];
		timHeader = (TIMHeader *)buffer;
		timHeader->deleteBufferFlag = 0xBABE;
	} else {
		timHeader = (TIMHeader *)buffer;
		timHeader->deleteBufferFlag = 0x0;
	}

	int32 chunkSize = file.getIFFBlockSize(AVTL_CHUNK);
	timHeader->unkFlag = -1;
	timHeader->unkFlag2 = 0;
	timHeader->cmdsOffset = sizeof(TIMHeader);
	timHeader->unkOffset2 = timHeader->cmdsOffset + 10 * sizeof(Cmds);
	timHeader->AVTLOffset = timHeader->unkOffset2 + 120;
	timHeader->TEXTOffset = timHeader->AVTLOffset + chunkSize;

	_TIMBuffers.AVTLChunk = buffer + timHeader->AVTLOffset;
	_TIMBuffers.TEXTChunk = buffer + timHeader->TEXTOffset;

	if (!file.loadIFFBlock(AVTL_CHUNK, _TIMBuffers.AVTLChunk, chunkSize)) {
		error("Couldn't load AVTL chunk from file: '%s'", filename);
		return NULL;
	}

	_TIMBuffers.currentCommandSet = (Cmds *)(buffer + timHeader->cmdsOffset);

	for (int i = 0; i < 10; i++) {
		_TIMBuffers.currentCommandSet[i].dataPtr = 0;
		_TIMBuffers.currentCommandSet[i].unk_2 = 0;
		_TIMBuffers.currentCommandSet[i].AVTLSubChunk = &_TIMBuffers.AVTLChunk[READ_LE_UINT16(&_TIMBuffers.AVTLChunk[i << 1]) << 1];
		_TIMBuffers.currentCommandSet[i].timer1 = 0;
		_TIMBuffers.currentCommandSet[i].timer2 = 0;
	}

	chunkSize = file.getIFFBlockSize(TEXT_CHUNK);
	if (chunkSize > 0) {
		if (!file.loadIFFBlock(TEXT_CHUNK, _TIMBuffers.TEXTChunk, chunkSize)) {
			error("Couldn't load TEXT chunk from file: '%s'", filename);
			return NULL;
		}
	}

	return buffer;
}

void KyraEngine_v2::tim_releaseBuffer(byte *buffer) {
	TIMHeader *timHeader = (TIMHeader *)buffer;
	if (timHeader->deleteBufferFlag == 0xBABE)
		delete[] buffer;
}

void KyraEngine_v2::tim_processSequence(uint8 *timBuffer, int loop) {
	if (!timBuffer)
		return;

	TIMHeader *hdr = (TIMHeader*) timBuffer;
	_TIMBuffers.offsUnkFlag = (uint8*) &hdr->unkFlag;
	_TIMBuffers.offsUnkFlag2 = (uint8*) &hdr->unkFlag2;
	_TIMBuffers.currentCommandSet = (Cmds*) (timBuffer + hdr->cmdsOffset);
	_TIMBuffers.unkCmds = timBuffer + hdr->unkOffset2;
	_TIMBuffers.AVTLChunk = timBuffer + hdr->AVTLOffset;
	_TIMBuffers.TEXTChunk = timBuffer + hdr->TEXTOffset;

	if (!_TIMBuffers.currentCommandSet->dataPtr) {
		_TIMBuffers.currentCommandSet->dataPtr = _TIMBuffers.currentCommandSet->AVTLSubChunk;
		_TIMBuffers.currentCommandSet->timer1 = _system->getMillis();
		_TIMBuffers.currentCommandSet->timer2 = _system->getMillis();
	}

	do {
		_TIMBuffers.currentEntry = 0;

		while (_TIMBuffers.currentEntry < 10) {
			Cmds *s = &_TIMBuffers.currentCommandSet[_TIMBuffers.currentEntry];
			if ((int16)READ_LE_UINT16(_TIMBuffers.offsUnkFlag) !=  -1)
				(this->*_timOpcodes[28])(_TIMBuffers.offsUnkFlag2);

			bool running = true;

			while (s->dataPtr && s->timer2 <= _system->getMillis() && running) {
				uint8 cmd = s->dataPtr[4];
				hdr->unkFlag2 = cmd;
				uint8 *para = &s->dataPtr[6];

				switch((this->*_timOpcodes[cmd])(para)) {
					case -3:
						WRITE_LE_UINT16(_TIMBuffers.offsUnkFlag, _TIMBuffers.currentEntry);
						_TIMBuffers.unk_12 = -1;
						break;

					case -2:
						running = false;
						break;

					case -1:
						loop = 0;
						running = false;
						_TIMBuffers.currentEntry = 11;
						break;

					case 22:
						s->backupPtr = 0;
						break;

					default:
						break;
				}

				if (s) {
					if (s->dataPtr) {
						s->dataPtr += (READ_LE_UINT16(s->dataPtr) * 2);
						s->timer1 = s->timer2;
						s->timer2 += (READ_LE_UINT16(s->dataPtr + 2) * _tickLength);
					}
				}
			}

			_TIMBuffers.currentEntry++;
		}
	} while (loop);

}

void KyraEngine_v2::tim_playFullSequence(const char *filename) {
	uint8 *ptr = tim_loadFile(filename, 0, 0);
	if (!ptr)
		return;

	_objectChatFinished = 0;

	while (ptr && !_objectChatFinished) {
		if (ptr)
			tim_processSequence(ptr, 0);
		if (_timChatText)
			updateWithText();
		else
			update();
	}

	if (ptr)
		tim_releaseBuffer(ptr);
}

int KyraEngine_v2::tim_o_dummy_r0(uint8 *ptr) {
	return 0;
}

int KyraEngine_v2::tim_o_dummy_r1(uint8 *ptr) {
	return 1;
}

int KyraEngine_v2::tim_o_clearCmds2(uint8 *ptr) {
	for (int i = 1; i < 10; i++)
		memset(&_TIMBuffers.unkCmds[i], 0, 12);
	_TIMBuffers.currentCommandSet[0].dataPtr = _TIMBuffers.currentCommandSet[0].AVTLSubChunk;
	_TIMBuffers.currentCommandSet[0].timer1 = _system->getMillis();
	return 1;
}

int KyraEngine_v2::tim_o_abort(uint8 *ptr) {
	_TIMBuffers.currentCommandSet[_TIMBuffers.currentEntry].dataPtr = 0;
	if(!_TIMBuffers.currentEntry)
		_objectChatFinished = true;
	return -2;
}


int KyraEngine_v2::tim_o_selectcurrentCommandSet(uint8 *ptr) {
	_TIMBuffers.currentCommandSet[READ_LE_UINT16(ptr)].dataPtr = _TIMBuffers.currentCommandSet[READ_LE_UINT16(ptr)].AVTLSubChunk ?
	_TIMBuffers.currentCommandSet[READ_LE_UINT16(ptr)].AVTLSubChunk : &_TIMBuffers.AVTLChunk[_TIMBuffers.AVTLChunk[READ_LE_UINT16(ptr) << 1] << 1];
	return 1;
}

int KyraEngine_v2::tim_o_deleteBuffer(uint8 *ptr) {
	_TIMBuffers.currentCommandSet[READ_LE_UINT16(ptr)].dataPtr = 0;
	return 1;
}

int KyraEngine_v2::tim_o_refreshTimers(uint8 *ptr) {
	for (int i = 1; i < 10; i++) {
		if (_TIMBuffers.currentCommandSet[i].dataPtr)
			_TIMBuffers.currentCommandSet[i].timer2 = _system->getMillis();
	}

	return 1;
}

int KyraEngine_v2::tim_o_execSubOpcode(uint8 *ptr) {
	return (this->*_timOpcodes[30 + READ_LE_UINT16(ptr)])(ptr + 2);
}

int KyraEngine_v2::tim_o_initActiveSub(uint8 *ptr) {
	_TIMBuffers.currentCommandSet[READ_LE_UINT16(ptr)].dataPtr = _TIMBuffers.currentCommandSet[READ_LE_UINT16(ptr)].AVTLSubChunk;
	_TIMBuffers.currentCommandSet[READ_LE_UINT16(ptr)].timer1 = _TIMBuffers.currentCommandSet[READ_LE_UINT16(ptr)].timer2 = _system->getMillis();
	return 1;
}

int KyraEngine_v2::tim_o_resetActiveSub(uint8 *ptr) {
	_TIMBuffers.currentCommandSet[READ_LE_UINT16(ptr)].dataPtr = 0;
	_TIMBuffers.currentCommandSet[READ_LE_UINT16(ptr)].timer2 = 0;
	_TIMBuffers.currentCommandSet[READ_LE_UINT16(ptr)].timer1 = 0;
	return 1;
}

int KyraEngine_v2::tim_o_printTalkText(uint8 *ptr) {
	_chatText = _timChatText = (const char*) _TIMBuffers.TEXTChunk + READ_LE_UINT16(_TIMBuffers.TEXTChunk + (READ_LE_UINT16(ptr) << 1));
	_chatObject = _timChatObject = READ_LE_UINT16(ptr + 2);

	if (_flags.lang == Common::JA_JPN) {
		for (int i = 0; i < _ingameTimJpStrSize; i += 2) {
			if (!scumm_stricmp(_timChatText, _ingameTimJpStr[i]))
				_chatText = _ingameTimJpStr[i + 1];
		}
	}
	objectChatInit(_chatText, _timChatObject);
	return 0;
}

int KyraEngine_v2::tim_o_updateSceneAnim(uint8 *ptr) {
	updateSceneAnim(READ_LE_UINT16(ptr + 2), READ_LE_UINT16(ptr));
	return 0;
}

int KyraEngine_v2::tim_o_resetChat(uint8 *ptr) {
	_text->restoreScreen();
	_chatText = 0;
	_chatObject = -1;
	_timChatText = 0;
	_timChatObject = -1;
	return 0;
}

int KyraEngine_v2::tim_o_playSoundEffect(uint8 *ptr) {
	snd_playSoundEffect(READ_LE_UINT16(ptr));
	return 0;
}

void KyraEngine_v2::tim_setupOpcodes() {
	static const TimOpc Opcodes[] = {
		&KyraEngine_v2::tim_o_clearCmds2,
		&KyraEngine_v2::tim_o_abort,
		&KyraEngine_v2::tim_o_dummy_r0,
		&KyraEngine_v2::tim_o_dummy_r0,
		&KyraEngine_v2::tim_o_selectcurrentCommandSet,
		&KyraEngine_v2::tim_o_deleteBuffer,
		&KyraEngine_v2::tim_o_dummy_r0,
		&KyraEngine_v2::tim_o_dummy_r0,
		&KyraEngine_v2::tim_o_dummy_r0,
		&KyraEngine_v2::tim_o_dummy_r0,
		&KyraEngine_v2::tim_o_dummy_r0,
		&KyraEngine_v2::tim_o_dummy_r0,
		&KyraEngine_v2::tim_o_dummy_r0,
		&KyraEngine_v2::tim_o_dummy_r0,
		&KyraEngine_v2::tim_o_dummy_r0,
		&KyraEngine_v2::tim_o_dummy_r0,
		&KyraEngine_v2::tim_o_dummy_r0,
		&KyraEngine_v2::tim_o_dummy_r0,
		&KyraEngine_v2::tim_o_dummy_r0,
		&KyraEngine_v2::tim_o_dummy_r0,
		&KyraEngine_v2::tim_o_dummy_r0,
		&KyraEngine_v2::tim_o_dummy_r0,
		&KyraEngine_v2::tim_o_dummy_r0,
		&KyraEngine_v2::tim_o_refreshTimers,
		&KyraEngine_v2::tim_o_dummy_r1,
		&KyraEngine_v2::tim_o_execSubOpcode,
		&KyraEngine_v2::tim_o_initActiveSub,
		&KyraEngine_v2::tim_o_resetActiveSub,
		&KyraEngine_v2::tim_o_dummy_r1,
		&KyraEngine_v2::tim_o_dummy_r1,
		&KyraEngine_v2::tim_o_printTalkText,
		&KyraEngine_v2::tim_o_updateSceneAnim,
		&KyraEngine_v2::tim_o_resetChat,
		&KyraEngine_v2::tim_o_playSoundEffect,
	};

	_timOpcodes = (const TimOpc*) Opcodes;
}

} // end of namespace Kyra

