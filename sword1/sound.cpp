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
#include "sword1.h"

#define SOUND_SPEECH_ID 1
#define SPEECH_FLAGS (SoundMixer::FLAG_16BITS | SoundMixer::FLAG_AUTOFREE | SoundMixer::FLAG_LITTLE_ENDIAN)

SwordSound::SwordSound(const char *searchPath, SoundMixer *mixer, ResMan *pResMan) {
	strcpy(_filePath, searchPath);
	_mixer = mixer;
	_resMan = pResMan;
	_cowHeader = NULL;
	_endOfQueue = 0;
	_currentCowFile = 0;
}

int SwordSound::addToQueue(int32 fxNo) {
	bool alreadyInQueue = false;
	for (uint8 cnt = 0; (cnt < _endOfQueue) && (!alreadyInQueue); cnt++)
		if (_fxQueue[cnt].id == (uint32)fxNo)
			alreadyInQueue = true;
	if (!alreadyInQueue) {
		if (_endOfQueue == MAX_FXQ_LENGTH) {
			warning("Sound queue overflow");
			return 0;
		}
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
		uint16 fxNo = _roomsFixedFx[SwordLogic::_scriptVars[SCREEN]][cnt];
		if (fxNo) {
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
				playSample(&_fxQueue[cnt]);
		} else {
			if (!_fxQueue[cnt].handle) { // sound finished
				_resMan->resClose(_fxList[_fxQueue[cnt].id].sampleId);
				if (cnt != _endOfQueue-1)
					_fxQueue[cnt] = _fxQueue[_endOfQueue - 1];
				_endOfQueue--;
			}
		}
	}
}

void SwordSound::fnStopFx(int32 fxNo) {

	_mixer->stopID(fxNo);
	for (uint8 cnt = 0; cnt < _endOfQueue; cnt++)
		if (_fxQueue[cnt].id == (uint32)fxNo) {
			if (!_fxQueue[cnt].delay) // sound was started
				_resMan->resClose(_fxList[_fxQueue[cnt].id].sampleId);
			if (cnt != _endOfQueue-1)
				_fxQueue[cnt] = _fxQueue[_endOfQueue-1];
			_endOfQueue--;
			return ;
		}
	debug(8, "fnStopFx: id not found in queue");
}

bool SwordSound::amISpeaking(void) {
	return true;
}

bool SwordSound::speechFinished(void) {
	return (_speechHandle == 0);
}

void SwordSound::newScreen(uint32 screen) {
	if (_currentCowFile != SwordEngine::_systemVars.currentCD) {
		if (_currentCowFile)
			closeCowSystem();
		initCowSystem();			
	}
}

void SwordSound::quitScreen(void) {
	// stop all running SFX
	while (_endOfQueue)
		fnStopFx(_fxQueue[0].id);
}

void SwordSound::playSample(QueueElement *elem) {
	uint8 *sampleData = (uint8*)_resMan->fetchRes(_fxList[elem->id].sampleId);
	for (uint16 cnt = 0; cnt < MAX_ROOMS_PER_FX; cnt++) {
		if (_fxList[elem->id].roomVolList[cnt].roomNo) {
			if ((_fxList[elem->id].roomVolList[cnt].roomNo == (int)SwordLogic::_scriptVars[SCREEN]) ||
				(_fxList[elem->id].roomVolList[cnt].roomNo == -1)) {

					uint8 volL = _fxList[elem->id].roomVolList[cnt].leftVol * 10;
					uint8 volR = _fxList[elem->id].roomVolList[cnt].rightVol * 10;
					int8 pan = (volR - volL) / 2;
					uint8 volume = (volR + volL) / 2;
					uint32 size = READ_LE_UINT32(sampleData + 0x28);
					uint8 flags;
					if (READ_LE_UINT16(sampleData + 0x22) == 16)
						flags = SoundMixer::FLAG_16BITS | SoundMixer::FLAG_LITTLE_ENDIAN;
					else
						flags = SoundMixer::FLAG_UNSIGNED;
					_mixer->playRaw(&elem->handle, sampleData + 0x2C, size, 11025, flags, elem->id, volume, pan);
			}
		} else
			break;
	}
}

bool SwordSound::startSpeech(uint16 roomNo, uint16 localNo) {
	if (_cowHeader == NULL) {
		warning("SwordSound::startSpeech: COW file isn't open!");
		return false;
	}

	uint32 locIndex = _cowHeader[roomNo] >> 2;
	uint32 sampleSize = _cowHeader[locIndex + (localNo * 2)];
	uint32 index = _cowHeader[locIndex + (localNo * 2) - 1];
	debug(6, "startSpeech(%d, %d): locIndex %d, sampleSize %d, index %d", roomNo, localNo, locIndex, sampleSize, index);
	if (sampleSize) {
		uint32 size;
		int16 *data = uncompressSpeech(index + _cowHeaderSize, sampleSize, &size);
		if (data)
			_mixer->playRaw(&_speechHandle, data, size, 11025, SPEECH_FLAGS, SOUND_SPEECH_ID);
		return true;
	} else
		return false;
}

int16 *SwordSound::uncompressSpeech(uint32 index, uint32 cSize, uint32 *size) {
	uint8 *fBuf = (uint8*)malloc(cSize);
	_cowFile.seek(index);
	_cowFile.read(fBuf, cSize);
	uint32 headerPos = 0;
	while ((READ_BE_UINT32(fBuf + headerPos) != 'data') && (headerPos < 100))
		headerPos++;
	if (headerPos < 100) {
		uint32 resSize = READ_LE_UINT32(fBuf + headerPos + 4) >> 1;
		int16 *srcData = (int16*)(fBuf + headerPos + 8);
		int16 *dstData = (int16*)malloc(resSize * 2);
		uint32 srcPos = 0;
		int16 *dstPos = dstData;
		cSize = (cSize - (headerPos + 8)) / 2;
		while (srcPos < cSize) {
			int16 length = (int16)READ_LE_UINT16(srcData + srcPos);
			srcPos++;
			if (length < 0) {
				length = -length;
				for (uint16 cnt = 0; cnt < (uint16)length; cnt++)
					*dstPos++ = srcData[srcPos];
				srcPos++;
			} else {
				memcpy(dstPos, srcData + srcPos, length * 2);
				dstPos += length;
				srcPos += length;
			}
		}
		free(fBuf);
		*size = resSize * 2;
		return dstData;
	} else {
		free(fBuf);
		warning("SwordSound::uncompressSpeech(): DATA tag not found in wave header");
		*size = 0;
		return NULL;
	}
}

void SwordSound::stopSpeech(void) {
	_mixer->stopID(SOUND_SPEECH_ID);
}

void SwordSound::initCowSystem(void) {
	char cowName[25];
	sprintf(cowName, "SPEECH%d.CLU", SwordEngine::_systemVars.currentCD);
	_cowFile.open(cowName);
	if (!_cowFile.isOpen()) {
		sprintf(cowName, "speech/SPEECH%d.CLU", SwordEngine::_systemVars.currentCD);
		_cowFile.open(cowName);
	}
	if (_cowFile.isOpen()) {
		_cowHeaderSize = _cowFile.readUint32LE();
		_cowHeader = (uint32*)malloc(_cowHeaderSize);
		if (_cowHeaderSize & 3)
			error("Unexpected cow header size %d", _cowHeaderSize);
		for (uint32 cnt = 0; cnt < (_cowHeaderSize / 4) - 1; cnt++)
			_cowHeader[cnt] = _cowFile.readUint32LE();
		_currentCowFile = SwordEngine::_systemVars.currentCD;
	} else
		warning("SwordSound::initCowSystem: Can't open SPEECH%d.CLU", SwordEngine::_systemVars.currentCD);	
}

void SwordSound::closeCowSystem(void) {
	if (_cowFile.isOpen())
		_cowFile.close();
	if (_cowHeader)
		free(_cowHeader);
	_cowHeader = NULL;
	_currentCowFile = NULL;
}

