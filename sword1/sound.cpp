/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2006 The ScummVM project
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 *
 */

#include "common/stdafx.h"
#include "sword1/sound.h"
#include "common/util.h"
#include "sword1/resman.h"
#include "sword1/logic.h"
#include "sword1/sword1.h"

#include "sound/mp3.h"
#include "sound/vorbis.h"
#include "sound/wave.h"

namespace Sword1 {

#define SOUND_SPEECH_ID 1
#define SPEECH_FLAGS (Audio::Mixer::FLAG_16BITS | Audio::Mixer::FLAG_AUTOFREE | Audio::Mixer::FLAG_LITTLE_ENDIAN)

Sound::Sound(const char *searchPath, Audio::Mixer *mixer, ResMan *pResMan) {
	strcpy(_filePath, searchPath);
	_mixer = mixer;
	_resMan = pResMan;
	_cowHeader = NULL;
	_endOfQueue = 0;
	_currentCowFile = 0;
	_speechVolL = _speechVolR = _sfxVolL = _sfxVolR = 192;
}

Sound::~Sound(void) {
	// clean up fx queue
	_mixer->stopAll();
	for (uint8 cnt = 0; cnt < _endOfQueue; cnt++)
		if (_fxQueue[cnt].delay == 0)
			_resMan->resClose(_fxList[_fxQueue[cnt].id].sampleId);
	_endOfQueue = 0;
	closeCowSystem();
}

int Sound::addToQueue(int32 fxNo) {
	bool alreadyInQueue = false;
	for (uint8 cnt = 0; (cnt < _endOfQueue) && (!alreadyInQueue); cnt++)
		if (_fxQueue[cnt].id == (uint32)fxNo)
			alreadyInQueue = true;
	if (!alreadyInQueue) {
		if (_endOfQueue == MAX_FXQ_LENGTH) {
			warning("Sound queue overflow");
			return 0;
		}
		if ((fxNo == 168) && (SwordEngine::_systemVars.isDemo)) {
			// this sound doesn't exist in demo
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

void Sound::engine(void) {
	// first of all, add any random sfx to the queue...
	for (uint16 cnt = 0; cnt < TOTAL_FX_PER_ROOM; cnt++) {
		uint16 fxNo = _roomsFixedFx[Logic::_scriptVars[SCREEN]][cnt];
		if (fxNo) {
			if (_fxList[fxNo].type == FX_RANDOM) {
				if (_rnd.getRandomNumber(_fxList[fxNo].delay) == 0)
					addToQueue(fxNo);
			}
		} else
			break;
	}
	// now process the queue
	for (uint8 cnt2 = 0; cnt2 < _endOfQueue; cnt2++) {
		if (_fxQueue[cnt2].delay > 0) {
			_fxQueue[cnt2].delay--;
			if (_fxQueue[cnt2].delay == 0)
				playSample(&_fxQueue[cnt2]);
		} else {
			if (!_mixer->isSoundHandleActive(_fxQueue[cnt2].handle)) { // sound finished
				_resMan->resClose(_fxList[_fxQueue[cnt2].id].sampleId);
				if (cnt2 != _endOfQueue-1)
					_fxQueue[cnt2] = _fxQueue[_endOfQueue - 1];
				_endOfQueue--;
			}
		}
	}
}

void Sound::fnStopFx(int32 fxNo) {
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

bool Sound::amISpeaking(void) {
	_waveVolPos++;
	return _waveVolume[_waveVolPos - 1];
}

bool Sound::speechFinished(void) {
	return !_mixer->isSoundHandleActive(_speechHandle);
}

void Sound::newScreen(uint32 screen) {
	if (_currentCowFile != SwordEngine::_systemVars.currentCD) {
		if (_currentCowFile)
			closeCowSystem();
		initCowSystem();
	}
}

void Sound::quitScreen(void) {
	// stop all running SFX
	while (_endOfQueue)
		fnStopFx(_fxQueue[0].id);
}

void Sound::playSample(QueueElement *elem) {
	uint8 *sampleData = (uint8*)_resMan->fetchRes(_fxList[elem->id].sampleId);
	for (uint16 cnt = 0; cnt < MAX_ROOMS_PER_FX; cnt++) {
		if (_fxList[elem->id].roomVolList[cnt].roomNo) {
			if ((_fxList[elem->id].roomVolList[cnt].roomNo == (int)Logic::_scriptVars[SCREEN]) ||
				(_fxList[elem->id].roomVolList[cnt].roomNo == -1)) {

					uint8 volL = (_fxList[elem->id].roomVolList[cnt].leftVol * 10 * _sfxVolL) / 255;
					uint8 volR = (_fxList[elem->id].roomVolList[cnt].rightVol * 10 * _sfxVolR) / 255;
					int8 pan = (volR - volL) / 2;
					uint8 volume = (volR + volL) / 2;
					uint32 size = READ_LE_UINT32(sampleData + 0x28);
					uint8 flags;
					if (READ_LE_UINT16(sampleData + 0x22) == 16)
						flags = Audio::Mixer::FLAG_16BITS | Audio::Mixer::FLAG_LITTLE_ENDIAN;
					else
						flags = Audio::Mixer::FLAG_UNSIGNED;
					if (READ_LE_UINT16(sampleData + 0x16) == 2)
						flags |= Audio::Mixer::FLAG_STEREO;
					if (_fxList[elem->id].type == FX_LOOP)
						flags |= Audio::Mixer::FLAG_LOOP;
					_mixer->playRaw(&elem->handle, sampleData + 0x2C, size, 11025, flags, elem->id, volume, pan);
			}
		} else
			break;
	}
}

bool Sound::startSpeech(uint16 roomNo, uint16 localNo) {
	if (_cowHeader == NULL) {
		warning("Sound::startSpeech: COW file isn't open!");
		return false;
	}

	uint32 locIndex = _cowHeader[roomNo] >> 2;
	uint32 sampleSize = _cowHeader[locIndex + (localNo * 2)];
	uint32 index = _cowHeader[locIndex + (localNo * 2) - 1];
	debug(6, "startSpeech(%d, %d): locIndex %d, sampleSize %d, index %d", roomNo, localNo, locIndex, sampleSize, index);
	if (sampleSize) {
		uint8 speechVol = (_speechVolR + _speechVolL) / 2;
		int8 speechPan = (_speechVolR - _speechVolL) / 2;
		if ((_cowMode == CowWave) || (_cowMode == CowDemo)) {
			uint32 size;
			int16 *data = uncompressSpeech(index + _cowHeaderSize, sampleSize, &size);
			if (data)
				_mixer->playRaw(&_speechHandle, data, size, 11025, SPEECH_FLAGS, SOUND_SPEECH_ID, speechVol, speechPan);
		}
#ifdef USE_MAD
		else if (_cowMode == CowMp3) {
			_cowFile.seek(index);
			_mixer->playInputStream(Audio::Mixer::kSFXSoundType, &_speechHandle, makeMP3Stream(&_cowFile, sampleSize), SOUND_SPEECH_ID, speechVol, speechPan);
			// with compressed audio, we can't calculate the wave volume.
			// so default to talking.
			for (int cnt = 0; cnt < 480; cnt++)
				_waveVolume[cnt] = true;
			_waveVolPos = 0;
		}
#endif
#ifdef USE_VORBIS
		else if (_cowMode == CowVorbis) {
			_cowFile.seek(index);
			_mixer->playInputStream(Audio::Mixer::kSFXSoundType, &_speechHandle, makeVorbisStream(&_cowFile, sampleSize), SOUND_SPEECH_ID, speechVol, speechPan);
			for (int cnt = 0; cnt < 480; cnt++)
				_waveVolume[cnt] = true;
			_waveVolPos = 0;
		}
#endif
		return true;
	} else
		return false;
}

int16 *Sound::uncompressSpeech(uint32 index, uint32 cSize, uint32 *size) {
	uint8 *fBuf = (uint8*)malloc(cSize);
	_cowFile.seek(index);
	_cowFile.read(fBuf, cSize);
	uint32 headerPos = 0;

	// TODO: use loadWAVFromStream to load the WAVE data!
	/*
	int rate, size;
	bye flags;
	Common::MemoryReadStream stream(fBuf, cSize);
	isValidWAV = loadWAVFromStream(stream, size, rate, flags);
	*/

	while ((READ_BE_UINT32(fBuf + headerPos) != 'data') && (headerPos < 100))
		headerPos++;
	if (headerPos < 100) {
		int32 resSize;
		headerPos += 4; // skip 'data' tag
		if (_cowMode != CowDemo) {
			resSize = READ_LE_UINT32(fBuf + headerPos) >> 1;
			headerPos += 4;
		} else {
			// the demo speech files have the uncompressed size embedded
			// in the compressed stream *sigh*
			if (READ_LE_UINT16(fBuf + headerPos) == 1) {
				resSize = READ_LE_UINT16(fBuf + headerPos + 2);
				resSize |= READ_LE_UINT16(fBuf + headerPos + 6) << 16;
			} else
				resSize = READ_LE_UINT32(fBuf + headerPos + 2);
			resSize >>= 1;
		}
		assert(!(headerPos & 1));
		int16 *srcData = (int16*)fBuf;
		uint32 srcPos = headerPos >> 1;
		cSize /= 2;
		uint32 dstPos = 0;
		/* alloc 200 additional bytes, as the demo sometimes has ASCII junk
		   at the end of the wave data */
		int16 *dstData = (int16*)malloc(resSize * 2 + 200);
		while (srcPos < cSize) {
			int16 length = (int16)READ_LE_UINT16(srcData + srcPos);
			srcPos++;
			if (length < 0) {
				length = -length;
				for (uint16 cnt = 0; cnt < (uint16)length; cnt++)
					dstData[dstPos++] = srcData[srcPos];
				srcPos++;
			} else {
				memcpy(dstData + dstPos, srcData + srcPos, length * 2);
				dstPos += length;
				srcPos += length;
			}
		}
		assert(dstPos < (uint32)resSize + 100);
		if (_cowMode == CowDemo) // demo has wave output size embedded in the compressed data
			*(uint32*)dstData = 0;
		free(fBuf);
		*size = resSize * 2;
		calcWaveVolume(dstData, resSize);
		return dstData;
	} else {
		free(fBuf);
		warning("Sound::uncompressSpeech(): DATA tag not found in wave header");
		*size = 0;
		return NULL;
	}
}

void Sound::calcWaveVolume(int16 *data, uint32 length) {
	int16 *blkPos = data + 918;
	uint32 cnt;
	for (cnt = 0; cnt < WAVE_VOL_TAB_LENGTH; cnt++)
		_waveVolume[cnt] = false;
	_waveVolPos = 0;
	for (uint32 blkCnt = 1; blkCnt < length / 918; blkCnt++) {
		if (blkCnt >= WAVE_VOL_TAB_LENGTH) {
			warning("Wave vol tab too small.");
			return;
		}
		int32 average = 0;
		for (cnt = 0; cnt < 918; cnt++)
			average += blkPos[cnt];
		average /= 918;
		uint32 diff = 0;
		for (cnt = 0; cnt < 918; cnt++) {
			int16 smpDiff = *blkPos - average;
			diff += (uint32)ABS(smpDiff);
			blkPos++;
		}
		if (diff > WAVE_VOL_THRESHOLD)
			_waveVolume[blkCnt - 1] = true;
	}
}

void Sound::stopSpeech(void) {
	_mixer->stopID(SOUND_SPEECH_ID);
}

void Sound::initCowSystem(void) {
	char cowName[25];
	/* look for speech1/2.clu in the data dir
	   and speech/speech.clu (running from cd or using cd layout)
	*/
#ifdef USE_MAD
	sprintf(cowName, "SPEECH%d.CL3", SwordEngine::_systemVars.currentCD);
	_cowFile.open(cowName);
	if (_cowFile.isOpen()) {
		debug(1, "Using MP3 compressed Speech Cluster");
		_cowMode = CowMp3;
	}
#endif
#ifdef USE_VORBIS
	if (!_cowFile.isOpen()) {
		sprintf(cowName, "SPEECH%d.CLV", SwordEngine::_systemVars.currentCD);
		_cowFile.open(cowName);
		if (_cowFile.isOpen()) {
			debug(1, "Using Vorbis compressed Speech Cluster");
			_cowMode = CowVorbis;
		}
	}
#endif
	if (!_cowFile.isOpen()) {
		sprintf(cowName, "SPEECH%d.CLU", SwordEngine::_systemVars.currentCD);
		_cowFile.open(cowName);
		if (!_cowFile.isOpen()) {
			_cowFile.open("speech.clu");
		}
		debug(1, "Using uncompressed Speech Cluster");
		_cowMode = CowWave;
	}
	if (!_cowFile.isOpen())
		_cowFile.open("speech.clu");
	if (!_cowFile.isOpen()) {
		_cowFile.open("cows.mad");
		if (_cowFile.isOpen())
			_cowMode = CowDemo;
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
		warning("Sound::initCowSystem: Can't open SPEECH%d.CLU", SwordEngine::_systemVars.currentCD);
}

void Sound::closeCowSystem(void) {
	if (_cowFile.isOpen())
		_cowFile.close();
	if (_cowHeader)
		free(_cowHeader);
	_cowHeader = NULL;
	_currentCowFile = 0;
}

} // End of namespace Sword1
