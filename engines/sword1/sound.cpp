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


#include "common/endian.h"

#include "common/util.h"
#include "common/events.h"
#include "common/system.h"

#include "sword1/sound.h"
#include "sword1/resman.h"
#include "sword1/logic.h"
#include "sword1/sword1.h"

#include "sound/flac.h"
#include "sound/mp3.h"
#include "sound/vorbis.h"
#include "sound/wave.h"
#include "sound/vag.h"

namespace Sword1 {

#define SOUND_SPEECH_ID 1
#define SPEECH_FLAGS (Audio::Mixer::FLAG_16BITS | Audio::Mixer::FLAG_AUTOFREE | Audio::Mixer::FLAG_LITTLE_ENDIAN)

Sound::Sound(const char *searchPath, Audio::Mixer *mixer, ResMan *pResMan) {
	g_system->getEventManager()->registerRandomSource(_rnd, "sword1sound");
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

					if (SwordEngine::isPsx()) { ;
						uint32 size = READ_LE_UINT32(sampleData);
						Audio::AudioStream *audStream = new Audio::VagStream(new Common::MemoryReadStream(sampleData + 4, size-4), _fxList[elem->id].type == FX_LOOP);
						_mixer->playInputStream(Audio::Mixer::kSFXSoundType, &elem->handle, audStream, elem->id, volume, pan, false, false, false);
					} else {
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
						_mixer->playRaw(Audio::Mixer::kSFXSoundType, &elem->handle, sampleData + 0x2C, size, 11025, flags, elem->id, volume, pan);
					}
			}
		} else
			break;
	}
}

bool Sound::startSpeech(uint16 roomNo, uint16 localNo) {
	if (_cowHeader == NULL) {
		warning("Sound::startSpeech: COW file isn't open");
		return false;
	}

	uint32 locIndex = 0xFFFFFFFF;
	uint32 sampleSize = 0;
	uint32 index = 0;

	if (_cowMode == CowPSX) {
		Common::File file;
		uint16 i;

		if (!file.open("speech.lis")) {
			warning ("Could not open speech.lis");
			return false;
		}

		for (i = 0; !file.eos() && !file.err(); i++)
			if (file.readUint16LE() == roomNo) {
				locIndex = i;
				break;
			}
		file.close();

		if (locIndex == 0xFFFFFFFF) {
			warning ("Could not find room %d in speech.lis", roomNo);
			return false;
		}

		if (!file.open("speech.inf")) {
			warning ("Could not open speech.inf");
			return false;
		}
		
		uint16 numRooms = file.readUint16LE(); // Number of rooms 

		file.seek(locIndex * 4 + 2); // 4 bytes per room, skip first 2 bytes

		uint16 numLines = file.readUint16LE();
		uint16 roomOffset = file.readUint16LE();

		file.seek(2 + numRooms * 4 + roomOffset * 2); // The offset is in terms of uint16's, so multiply by 2. Skip the 0x112 byte header too.

		locIndex = 0xFFFFFFFF;

		for (i = 0; i < numLines; i++)
			if (file.readUint16LE() == localNo) {
				locIndex = i;
				break;
			}

		if (locIndex == 0xFFFFFFFF) {
			warning ("Could not find local number %d in room %d in speech.inf", roomNo, localNo);
			return false;
		}

		file.close();

		index = _cowHeader[(roomOffset + locIndex) * 2];
		sampleSize = _cowHeader[(roomOffset + locIndex) * 2 + 1];
	} else {
		locIndex = _cowHeader[roomNo] >> 2;
		sampleSize = _cowHeader[locIndex + (localNo * 2)];
		index = _cowHeader[locIndex + (localNo * 2) - 1];
	}

	debug(6, "startSpeech(%d, %d): locIndex %d, sampleSize %d, index %d", roomNo, localNo, locIndex, sampleSize, index);

	if (sampleSize) {
		uint8 speechVol = (_speechVolR + _speechVolL) / 2;
		int8 speechPan = (_speechVolR - _speechVolL) / 2;
		if ((_cowMode == CowWave) || (_cowMode == CowDemo)) {
			uint32 size;
			int16 *data = uncompressSpeech(index + _cowHeaderSize, sampleSize, &size);
			if (data)
				_mixer->playRaw(Audio::Mixer::kSpeechSoundType, &_speechHandle, data, size, 11025, SPEECH_FLAGS, SOUND_SPEECH_ID, speechVol, speechPan);
		} else if (_cowMode == CowPSX && sampleSize != 0xffffffff) {
			_cowFile.seek(index * 2048);
			_mixer->playInputStream(Audio::Mixer::kSpeechSoundType, &_speechHandle, new Audio::VagStream(_cowFile.readStream(sampleSize)), SOUND_SPEECH_ID, speechVol, speechPan);
			// with compressed audio, we can't calculate the wave volume.
			// so default to talking.
			for (int cnt = 0; cnt < 480; cnt++)
				_waveVolume[cnt] = true;
			_waveVolPos = 0;
		}
#ifdef USE_FLAC
		else if (_cowMode == CowFlac) {
			_cowFile.seek(index);
			Common::MemoryReadStream *tmp = _cowFile.readStream(sampleSize);
			assert(tmp);
			_mixer->playInputStream(Audio::Mixer::kSpeechSoundType, &_speechHandle, Audio::makeFlacStream(tmp, true), SOUND_SPEECH_ID, speechVol, speechPan);
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
			Common::MemoryReadStream *tmp = _cowFile.readStream(sampleSize);
			assert(tmp);
			_mixer->playInputStream(Audio::Mixer::kSpeechSoundType, &_speechHandle, Audio::makeVorbisStream(tmp, true), SOUND_SPEECH_ID, speechVol, speechPan);
			// with compressed audio, we can't calculate the wave volume.
			// so default to talking.
			for (int cnt = 0; cnt < 480; cnt++)
				_waveVolume[cnt] = true;
			_waveVolPos = 0;
		}
#endif
#ifdef USE_MAD
		else if (_cowMode == CowMp3) {
			_cowFile.seek(index);
			Common::MemoryReadStream *tmp = _cowFile.readStream(sampleSize);
			assert(tmp);
			_mixer->playInputStream(Audio::Mixer::kSpeechSoundType, &_speechHandle, Audio::makeMP3Stream(tmp, true), SOUND_SPEECH_ID, speechVol, speechPan);
			// with compressed audio, we can't calculate the wave volume.
			// so default to talking.
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
		int16 *srcData;
		uint32 srcPos;
		int16 length;
		cSize /= 2;
		headerPos += 4; // skip 'data' tag
		if (_cowMode != CowDemo) {
			resSize = READ_LE_UINT32(fBuf + headerPos) >> 1;
			headerPos += 4;
		} else {
			// the demo speech files have the uncompressed size
			// embedded in the compressed stream *sigh*
			//
			// But not always, apparently. See bug #2182450. Is
			// there any way to figure out the size other than
			// decoding the sound in that case?

			if (fBuf[headerPos + 1] == 0) {
				if (READ_LE_UINT16(fBuf + headerPos) == 1) {
					resSize = READ_LE_UINT16(fBuf + headerPos + 2);
					resSize |= READ_LE_UINT16(fBuf + headerPos + 6) << 16;
				} else
					resSize = READ_LE_UINT32(fBuf + headerPos + 2);
				resSize >>= 1;
			} else {
				resSize = 0;
				srcData = (int16*)fBuf;
				srcPos = headerPos >> 1;
				while (srcPos < cSize) {
					length = (int16)READ_LE_UINT16(srcData + srcPos);
					srcPos++;
					if (length < 0) {
						resSize -= length;
						srcPos++;
					} else {
						resSize += length;
						srcPos += length;
					}
				}
			}
		}
		assert(!(headerPos & 1));
		srcData = (int16*)fBuf;
		srcPos = headerPos >> 1;
		uint32 dstPos = 0;
		int16 *dstData = (int16*)malloc(resSize * 2);
		int32 samplesLeft = resSize;
		while (srcPos < cSize && samplesLeft > 0) {
			length = (int16)READ_LE_UINT16(srcData + srcPos);
			srcPos++;
			if (length < 0) {
				length = -length;
				if (length > samplesLeft)
					length = samplesLeft;
				for (uint16 cnt = 0; cnt < (uint16)length; cnt++)
					dstData[dstPos++] = srcData[srcPos];
				srcPos++;
			} else {
				if (length > samplesLeft)
					length = samplesLeft;
				memcpy(dstData + dstPos, srcData + srcPos, length * 2);
				dstPos += length;
				srcPos += length;
			}
			samplesLeft -= length;
		}
		if (samplesLeft > 0) {
			memset(dstData + dstPos, 0, samplesLeft * 2);
		}
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
#ifdef USE_FLAC
	if (!_cowFile.isOpen()) {
		sprintf(cowName, "SPEECH%d.CLF", SwordEngine::_systemVars.currentCD);
		_cowFile.open(cowName);
		if (_cowFile.isOpen()) {
			debug(1, "Using Flac compressed Speech Cluster");
			_cowMode = CowFlac;
		}
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
#ifdef USE_MAD
	if (!_cowFile.isOpen()) {
		sprintf(cowName, "SPEECH%d.CL3", SwordEngine::_systemVars.currentCD);
		_cowFile.open(cowName);
		if (_cowFile.isOpen()) {
			debug(1, "Using MP3 compressed Speech Cluster");
			_cowMode = CowMp3;
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

	if (SwordEngine::isPsx()) {
		// There's only one file on the PSX, so set it to the current disc.
		_currentCowFile = SwordEngine::_systemVars.currentCD;
		if (!_cowFile.isOpen()) {
			if (!_cowFile.open("speech.dat"))
				error ("Could not open speech.dat");
			_cowMode = CowPSX;
		}
	}

	if (!_cowFile.isOpen())
		_cowFile.open("speech.clu");

	if (!_cowFile.isOpen()) {
		_cowFile.open("cows.mad");
		if (_cowFile.isOpen())
			_cowMode = CowDemo;
	}

	if (_cowFile.isOpen()) {
		if (SwordEngine::isPsx()) {
			// Get data from the external table file
			Common::File tableFile;
			if (!tableFile.open("speech.tab"))
				error ("Could not open speech.tab");
			_cowHeaderSize = tableFile.size();
			_cowHeader = (uint32 *)malloc(_cowHeaderSize);
			if (_cowHeaderSize & 3)
				error("Unexpected cow header size %d", _cowHeaderSize);
			for (uint32 cnt = 0; cnt < _cowHeaderSize / 4; cnt++)
				_cowHeader[cnt] = tableFile.readUint32LE();
		} else {
			_cowHeaderSize = _cowFile.readUint32LE();
			_cowHeader = (uint32*)malloc(_cowHeaderSize);
			if (_cowHeaderSize & 3)
				error("Unexpected cow header size %d", _cowHeaderSize);
			for (uint32 cnt = 0; cnt < (_cowHeaderSize / 4) - 1; cnt++)
				_cowHeader[cnt] = _cowFile.readUint32LE();
			_currentCowFile = SwordEngine::_systemVars.currentCD;
		}
	} else
		warning("Sound::initCowSystem: Can't open SPEECH%d.CLU", SwordEngine::_systemVars.currentCD);
}

void Sound::closeCowSystem(void) {
	_cowFile.close();
	free(_cowHeader);
	_cowHeader = NULL;
	_currentCowFile = 0;
}

} // End of namespace Sword1
