/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "sound.h"
#include "common/util.h"
#include "resman.h"
#include "logic.h"

#define SOUND_SPEECH_ID 1
#define SPEECH_FLAGS (SoundMixer::FLAG_16BITS | SoundMixer::FLAG_AUTOFREE | SoundMixer::FLAG_LITTLE_ENDIAN)

SwordSound::SwordSound(const char *searchPath, SoundMixer *mixer, ResMan *pResMan) {
	strcpy(_filePath, searchPath);
	_mixer = mixer;
	_resMan = pResMan;
	_cowHeader = NULL;
	initCowSystem();
	_endOfQueue = 0;
}

int SwordSound::addToQueue(int32 fxNo) {
	bool alreadyInQueue = false;
	for (uint8 cnt = 0; (cnt < _endOfQueue) && (!alreadyInQueue); cnt++)
		if (_fxQueue[cnt].id == fxNo)
			alreadyInQueue = true;
	if (!alreadyInQueue) {
		if (_endOfQueue == MAX_FXQ_LENGTH)
			error("Sound queue overflow");
		_resMan->resOpen(_fxList[fxNo].sampleId);
		_fxQueue[_endOfQueue].id = fxNo;
		if (_fxList[fxNo].type == FX_SPOT)
			_fxQueue[_endOfQueue].delay = _fxList[fxNo].delay + 1;
		else
			_fxQueue[_endOfQueue].delay = 1;
		_endOfQueue++;
		return 1;
	}
	return 0;
}

void SwordSound::engine(void) {
	// first of all, add any random sfx to the queue...
	for (uint16 cnt = 0; cnt < TOTAL_FX_PER_ROOM; cnt++) {
		uint16 fxNo;
		if (fxNo = _roomsFixedFx[SwordLogic::_scriptVars[SCREEN]][cnt]) {
			if (_fxList[fxNo].type == FX_RANDOM) {
				if (_rnd.getRandomNumber(_fxList[fxNo].delay) == 0)
					addToQueue(fxNo);
			}
		} else
			break;
	}
	// now process the queue
	for (uint8 cnt = 0; cnt < _endOfQueue; cnt++) {
		if (_fxQueue[cnt].delay > 0) {
			_fxQueue[cnt].delay--;
			if (_fxQueue[cnt].delay == 0)
				playSample(_fxQueue[cnt]);
		} else {
			if (!_fxQueue[cnt].handle) { // sound finished
				_resMan->resClose(_fxQueue[cnt].id);
				if (cnt != _endOfQueue-1)
					_fxQueue[cnt] = _fxQueue[_endOfQueue - 1];
				_endOfQueue--;
			}
		}
	}
}

bool SwordSound::amISpeaking(void) {
	return true;
}

void SwordSound::clearAllFx(void) {
	warning("Stub: SwordSound::clearAllFx()");
}

void SwordSound::closeCowSysten(void) {
	warning("stub: SwordSound::closeCowSystem()");
}

void SwordSound::fnStopFx(int32 fxNo) {
	warning("stub: SwordSound::fnStopFx(%d)", fxNo);
}

bool SwordSound::speechFinished(void) {
	//warning("stub: SwordSound::speechFinished()");
	//return true;
    return (_speechHandle == 0);
}

void SwordSound::startFxForScreen(uint16 screen) { // do we need this?
	warning("stub: SwordSound::startFxForScreen(%d)", screen);
}

void SwordSound::playSample(QueueElement elem) {

	uint8 *sampleData = (uint8*)_resMan->fetchRes(_fxList[elem.id].sampleId);
	for (uint16 cnt = 0; cnt < MAX_ROOMS_PER_FX; cnt++) {
		if (_fxList[elem.id].roomVolList[cnt].roomNo) {
			if ((_fxList[elem.id].roomVolList[cnt].roomNo == SwordLogic::_scriptVars[SCREEN]) ||
				(_fxList[elem.id].roomVolList[cnt].roomNo == -1)) {

					uint8 volL = _fxList[elem.id].roomVolList[cnt].leftVol * 10;
					uint8 volR = _fxList[elem.id].roomVolList[cnt].rightVol * 10;
					int8 pan = (volR - volL) / 2;
					uint8 volume = (volR + volL) / 2;
					uint32 size = READ_LE_UINT32(sampleData + 0x28);
					uint8 flags;
					if (READ_LE_UINT16(sampleData + 0x22) == 16)
						flags = SoundMixer::FLAG_16BITS | SoundMixer::FLAG_LITTLE_ENDIAN;
					else
						flags = SoundMixer::FLAG_UNSIGNED;
					_mixer->playRaw(&elem.handle, sampleData + 0x2C, size, 11025, flags, elem.id, volume, pan);
			}
		} else
			break;
	}
}

uint32 SwordSound::uncompressedSize(uint8 *data) {
	return READ_LE_UINT32(data + 0x28);
}

bool SwordSound::startSpeech(uint16 roomNo, uint16 localNo) {
	if (_cowHeader == NULL) {
		warning("SwordSound::startSpeech: COW file isn't open!");
		return false;
	}

	uint32 locIndex = _cowHeader[roomNo] >> 2;
	uint32 sampleSize = _cowHeader[locIndex + (localNo * 2)];
	uint32 index = _cowHeader[locIndex + (localNo * 2) - 1];
	debug(4, "startSpeech(%d, %d): locIndex %d, sampleSize %d, index %d", roomNo, localNo, locIndex, sampleSize, index);
	if (sampleSize) {
		_cowFile.seek(index + _cowHeaderSize);
		uint8 *buf = (uint8*)malloc(sampleSize);
		_cowFile.read(buf, sampleSize);
		uint8 *smpBuf = (uint8*)malloc(uncompressedSize(buf));
        uint32 size = expandSpeech(buf, smpBuf, sampleSize);
		free(buf);
		if (!size) {
			free(smpBuf);
			return false;
		}
		_mixer->playRaw(&_speechHandle, smpBuf, size, 11025, SPEECH_FLAGS, SOUND_SPEECH_ID);
		return true;
	} else
		return false;
}

uint32 SwordSound::expandSpeech(void *src, void *dest, uint32 srcSize) {
	int16 *compData = (int16*)src;
	if (READ_BE_UINT32(compData + 0x12) != 'data') {
		warning("SwordSound::expandSpeech: 'data' tag not found in wave header");
		return 0;
	}
	srcSize >>= 1;
	int16 *expData = (int16*)dest;
	compData += 0x16;
	srcSize -= 0x16;

	uint32 srcPos = 0;
	while (srcPos < srcSize) {
		if (compData[srcPos] < 0) {
			uint16 len = (uint16)(-compData[srcPos]);
			for (uint32 cnt = 0; cnt < len; cnt++)
				*expData++ = compData[srcPos + 1];
			srcPos += 2;
		} else {
			uint32 len = (uint32)compData[srcPos];
			memcpy(expData, compData + srcPos + 1, len * 2);
			expData += len;
			srcPos += len + 1;
		}
	}
	return (uint8*)expData - (uint8*)dest;
}

void SwordSound::stopSpeech(void) {
	_mixer->stopID(SOUND_SPEECH_ID);
}

void SwordSound::initCowSystem(void) {
	_cowFile.open("SPEECH.CLU");
	if (_cowFile.isOpen()) {
		_cowHeaderSize = _cowFile.readUint32LE();
		_cowHeader = (uint32*)malloc(_cowHeaderSize);
		if (_cowHeaderSize & 3)
			error("Unexpected cow header size %d", _cowHeaderSize);
		for (uint32 cnt = 0; cnt < (_cowHeaderSize / 4) - 1; cnt++)
			_cowHeader[cnt] = _cowFile.readUint32LE();
	} else
		warning("SwordSound::initCowSystem: Can't open SPEECH.CLU");	
}
