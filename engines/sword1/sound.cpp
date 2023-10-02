/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#include "common/endian.h"

#include "common/util.h"
#include "common/memstream.h"
#include "common/textconsole.h"

#include "sword1/sound.h"
#include "sword1/resman.h"
#include "sword1/logic.h"
#include "sword1/sword1.h"

#include "audio/audiostream.h"
#include "audio/decoders/flac.h"
#include "audio/decoders/mp3.h"
#include "audio/decoders/raw.h"
#include "audio/decoders/vorbis.h"
#include "audio/decoders/xa.h"

namespace Sword1 {

#define SOUND_SPEECH_ID 1
#define SPEECH_FLAGS (Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN)

Sound::Sound(Audio::Mixer *mixer, SwordEngine *vm, ResMan *pResMan)
	: _rnd("sword1sound") {
	_vm = vm;
	_mixer = mixer;
	_resMan = pResMan;
	_bigEndianSpeech = false;
	_cowHeader = NULL;
	_endOfQueue = 0;
	_currentCowFile = 0;
	_speechVolL = _speechVolR = _sfxVolL = _sfxVolR = 192;
}

Sound::~Sound() {
	// clean up fx queue
	_mixer->stopAll();
	for (uint8 cnt = 0; cnt < _endOfQueue; cnt++)
		if (_fxQueue[cnt].delay == 0)
			_resMan->resClose(getSampleId(_fxQueue[cnt].id));
	_endOfQueue = 0;
	closeCowSystem();
}

uint32 Sound::getSampleId(int32 fxNo) {
	byte cluster = _fxList[fxNo].sampleId.cluster;
	byte id;
	if (SwordEngine::_systemVars.isDemo && SwordEngine::_systemVars.platform == Common::kPlatformWindows && !SwordEngine::_systemVars.isSpanishDemo) {
		id = _fxList[fxNo].sampleId.idWinDemo;
	} else {
		id = _fxList[fxNo].sampleId.idStd;
	}
	return (cluster << 24) | id;
}

void Sound::checkSpeechFileEndianness() {
	// Some mac versions (not all of them) use big endian wav, although
	// the wav header doesn't indicate it.
	// Use heuristic to determine endianness of speech.
	// The heuristic consist in computing the sum of the absolute difference for
	// every two consecutive samples. This is done both with a big endian and a
	// little endian assumption. The one with the smallest sum should be the
	// correct one (the sound wave is supposed to be relatively smooth).
	// It needs at least 1000 samples to get stable result (the code below is
	// using the first 2000 samples of the wav sound).

	// Init speech file if not already done.
	if (!_currentCowFile) {
		// Open one of the speech files. It uses SwordEngine::_systemVars.currentCD
		// to decide which file to open, therefore if it is currently set to zero
		// we have to set it to either 1 or 2 (I decided to set it to 1 as this is
		// more likely to be the first file that will be needed).
		bool no_current_cd = false;
		if (SwordEngine::_systemVars.currentCD == 0) {
			SwordEngine::_systemVars.currentCD = 1;
			no_current_cd = true;
		}
		initCowSystem();
		if (no_current_cd) {
			// In case it fails with CD1 retry with CD2
			if (!_currentCowFile) {
				SwordEngine::_systemVars.currentCD = 2;
				initCowSystem();
			}
			// Reset currentCD flag
			SwordEngine::_systemVars.currentCD = 0;
		}
	}

	// Testing for endianness makes sense only if using the uncompressed files.
	if (_cowHeader == NULL || (_cowMode != CowWave && _cowMode != CowDemo))
		return;

	// I picked the sample to use randomly (I just made sure it is long enough so that there is
	// a fair chance of the heuristic to have a stable result and work for every language).
	int roomNo = _currentCowFile == 1 ? 1 : 129;
	int localNo = _currentCowFile == 1 ? 2 : 933;
	// Get the speech data and apply the heuristic
	uint32 locIndex = _cowHeader[roomNo] >> 2;
	uint32 sampleSize = _cowHeader[locIndex + (localNo * 2)];
	uint32 index = _cowHeader[locIndex + (localNo * 2) - 1];
	if (sampleSize) {
		uint32 size;
		bool leOk = false, beOk = false;
		// Compute average of difference between two consecutive samples for both BE and LE
		_bigEndianSpeech = false;
		int16 *data = uncompressSpeech(index + _cowHeaderSize, sampleSize, &size, &leOk);
		uint32 maxSamples = size > 2000 ? 2000 : size;
		double le_diff = endiannessHeuristicValue(data, size, maxSamples);
		free(data);
		_bigEndianSpeech = true;
		data = uncompressSpeech(index + _cowHeaderSize, sampleSize, &size, &beOk);
		double be_diff = endiannessHeuristicValue(data, size, maxSamples);
		free(data);
		// Set the big endian flag
		if (leOk && !beOk)
			_bigEndianSpeech = false;
		else if (beOk && !leOk)
			_bigEndianSpeech = true;
		else
			_bigEndianSpeech = (be_diff < le_diff);
		if (_bigEndianSpeech)
			debug(6, "Mac version: using big endian speech file");
		else
			debug(6, "Mac version: using little endian speech file");
		debug(8, "Speech decompression memory check: big endian = %s, little endian = %s", beOk ? "good" : "bad", leOk ? "good" : "bad");
		debug(8, "Speech endianness heuristic: average = %f for BE and %f for LE (%d samples)", be_diff, le_diff, maxSamples);
	}
}

double Sound::endiannessHeuristicValue(int16* data, uint32 dataSize, uint32 &maxSamples) {
	if (!data)
		return 50000.; // the heuristic value for the wrong endianess is about 21000 (1/3rd of the 16 bits range)

	double diff_sum = 0.;
	uint32 cpt = 0;
	int16 prev_value = (int16)FROM_LE_16(*((uint16 *)(data)));
	for (uint32 i = 1; i < dataSize && cpt < maxSamples; ++i) {
		int16 value = (int16)FROM_LE_16(*((uint16 *)(data + i)));
		if (value != prev_value) {
			diff_sum += fabs((double)(value - prev_value));
			++cpt;
			prev_value = value;
		}
	}
	if (cpt == 0)
		return 50000.;
	maxSamples = cpt;
	return diff_sum / cpt;
}

int Sound::addToQueue(uint32 fxNo) {
	bool alreadyInQueue = false;
	for (uint8 cnt = 0; cnt < _endOfQueue; cnt++) {
		if (_fxQueue[cnt].id == fxNo) {
			alreadyInQueue = true;
			break;
		}
	}

	if (alreadyInQueue) {
		debug(5, "Sound::addToQueue(): Sound %d is already in the queue, ignoring...", fxNo);
		return 0;
	} else {
		if (_endOfQueue == MAX_FXQ_LENGTH) {
			warning("Sound::addToQueue(): Sound queue overflow");
			return 0;
		}

		uint32 sampleId = getSampleId(fxNo);
		if ((sampleId & 0xFF) != 0xFF) {
			_resMan->resOpen(sampleId);
			_fxQueue[_endOfQueue].id = fxNo;

			if (_fxList[fxNo].type == FX_SPOT) {
				_fxQueue[_endOfQueue].delay = _fxList[fxNo].delay + 1;
			} else {
				_fxQueue[_endOfQueue].delay = 1;
			}

			_endOfQueue++;
			return 1; // Success!
		}

		return 0;
	}
}

void Sound::removeFromQueue(uint32 fxNo) {
	uint32 i, j;
	bool alreadyInQueue = false;
	uint8 cnt = 0;
	for (cnt = 0; cnt < _endOfQueue; j++) {
		if (_fxQueue[cnt].id == fxNo) {
			alreadyInQueue = true;
			break;
		}
	}

	if (alreadyInQueue) {
		_resMan->resClose(getSampleId(_fxQueue[cnt].id));

		for (uint8 j = 0; j < _endOfQueue; j++) { // scan fxq from start to last element stored
			if (_fxQueue[j].id == fxNo) { // if we've found the fxNo in question
				for (i = j; i < (_endOfQueue - 1); i++) { // move all the others down one to fill this space
					_fxQueue[i].id = _fxQueue[i + 1].id;
					_fxQueue[i].delay = _fxQueue[i + 1].delay;
				}

				debug(5, "Sound::addToQueue(): Sound fxNo %d removed from _fxQueue[%d] (_endOfQueue = %d)\n", fxNo, j, _endOfQueue - 1);
				_endOfQueue--; // decrement the end of queue element-number

				break;
			}
		}
	}
}

void Sound::engine() {
	// First of all, add any random sfx to the queue...
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

	// Now process the queue...
	int32 fxNo = 0;
	for (uint8 cnt = 0; cnt < _endOfQueue; cnt++) {
		fxNo = _fxQueue[cnt].id;
		if (_fxQueue[cnt].delay > 0) {
			_fxQueue[cnt].delay--;
			if (_fxQueue[cnt].delay == 0)
				PlaySample(fxNo);
		} else if (CheckSampleStatus(fxNo) == 1) {
			// Delay countdown was already zero, so the sample has
			// already been played, so check if it's finished...
			removeFromQueue(fxNo);
		}
	}
}

void Sound::fnStopFx(int32 fxNo) {
	_mixer->stopID(fxNo);
	for (uint8 cnt = 0; cnt < _endOfQueue; cnt++)
		if (_fxQueue[cnt].id == (uint32)fxNo) {
			if (!_fxQueue[cnt].delay) // sound was started
				_resMan->resClose(getSampleId(_fxQueue[cnt].id));
			if (cnt != _endOfQueue - 1)
				_fxQueue[cnt] = _fxQueue[_endOfQueue - 1];
			_endOfQueue--;
			return;
		}
	debug(8, "fnStopFx: id not found in queue");
}

bool Sound::amISpeaking(byte *buf) {
	int16 *offset;
	int16 count;
	int32 readPos;

	if (!speechSampleBusy)
		return (0);

	if (true /* AIL_sample_status(hSampleSpeech) != SMP_DONE*/) {

		speechCount += 1;

		readPos = speechCount * 919 * 2;

		// Ensure that we don't read beyond the buffer
		if (readPos + 150 * sizeof(int16) > speechSize)
			return false;

		offset = (int16 *)&buf[readPos];
		count = 0;
		for (int i = 0; i < 150; i++) {
			if ((offset[i] < NEG_MOUTH_THRESHOLD) || (offset[i] > POS_MOUTH_THRESHOLD)) {
				count += 1;
				if (count == 50) {
					return true;
				}
			}
		}
	}

	return false;

	//_waveVolPos++;
	//return _waveVolume[_waveVolPos - 1];
}

bool Sound::speechFinished() {
	return !_mixer->isSoundHandleActive(_speechHandle);
}

void Sound::newScreen(uint32 screen) {
	if (_currentCowFile != SwordEngine::_systemVars.currentCD) {
		if (_cowFile.isOpen())
			closeCowSystem();
		initCowSystem();
	}

	// Start the room's looping sounds.
	for (uint16 cnt = 0; cnt < TOTAL_FX_PER_ROOM; cnt++) {
		uint16 fxNo = _roomsFixedFx[screen][cnt];
		if (fxNo) {
			if (_fxList[fxNo].type == FX_LOOP)
				addToQueue(fxNo);
		} else
			break;
	}
}

void Sound::playSample(QueueElement *elem) {
	uint8 *sampleData = (uint8 *)_resMan->fetchRes(getSampleId(elem->id));
	for (uint16 cnt = 0; cnt < MAX_ROOMS_PER_FX; cnt++) {
		if (_fxList[elem->id].roomVolList[cnt].roomNo) {
			if ((_fxList[elem->id].roomVolList[cnt].roomNo == (int)Logic::_scriptVars[SCREEN]) ||
			        (_fxList[elem->id].roomVolList[cnt].roomNo == -1)) {

				uint8 volL = (_fxList[elem->id].roomVolList[cnt].leftVol * 10 * _sfxVolL) / 255;
				uint8 volR = (_fxList[elem->id].roomVolList[cnt].rightVol * 10 * _sfxVolR) / 255;
				int8 pan = (volR - volL) / 2;
				uint8 volume = (volR + volL) / 2;

				if (SwordEngine::isPsx()) {
					uint32 size = READ_LE_UINT32(sampleData);
					Audio::AudioStream *audStream = Audio::makeLoopingAudioStream(Audio::makeXAStream(new Common::MemoryReadStream(sampleData + 4, size - 4), 11025), (_fxList[elem->id].type == FX_LOOP) ? 0 : 1);
					_mixer->playStream(Audio::Mixer::kSFXSoundType, &elem->handle, audStream, elem->id, volume, pan);
				} else {
					uint32 size = READ_LE_UINT32(sampleData + 0x28);
					uint8 flags;
					if (READ_LE_UINT16(sampleData + 0x22) == 16)
						flags = Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN;
					else
						flags = Audio::FLAG_UNSIGNED;
					if (READ_LE_UINT16(sampleData + 0x16) == 2)
						flags |= Audio::FLAG_STEREO;
					Audio::AudioStream *stream = Audio::makeLoopingAudioStream(
					                                 Audio::makeRawStream(sampleData + 0x2C, size, 11025, flags, DisposeAfterUse::NO),
					                                 (_fxList[elem->id].type == FX_LOOP) ? 0 : 1);
					_mixer->playStream(Audio::Mixer::kSFXSoundType, &elem->handle, stream, elem->id, volume, pan);
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
			warning("Could not open speech.lis");
			return false;
		}

		for (i = 0; !file.eos() && !file.err(); i++)
			if (file.readUint16LE() == roomNo) {
				locIndex = i;
				break;
			}
		file.close();

		if (locIndex == 0xFFFFFFFF) {
			warning("Could not find room %d in speech.lis", roomNo);
			return false;
		}

		if (!file.open("speech.inf")) {
			warning("Could not open speech.inf");
			return false;
		}

		uint16 numRooms = file.readUint16LE(); // Read number of rooms referenced in this file

		file.seek(locIndex * 4 + 2); // 4 bytes per room, skip first 2 bytes

		uint16 numLines = file.readUint16LE();
		uint16 roomOffset = file.readUint16LE();

		file.seek(2 + numRooms * 4 + roomOffset * 2); // The offset is in terms of uint16's, so multiply by 2. Skip the room indexes too.

		locIndex = 0xFFFFFFFF;

		for (i = 0; i < numLines; i++)
			if (file.readUint16LE() == localNo) {
				locIndex = i;
				break;
			}

		if (locIndex == 0xFFFFFFFF) {
			warning("Could not find local number %d in room %d in speech.inf", roomNo, localNo);
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

	Audio::AudioStream *stream = 0;

	if (sampleSize) {
		uint8 speechVol = (_speechVolR + _speechVolL) / 2;
		int8 speechPan = (_speechVolR - _speechVolL) / 2;
		if ((_cowMode == CowWave) || (_cowMode == CowDemo)) {
			uint32 size;
			int16 *data = uncompressSpeech(index + _cowHeaderSize, sampleSize, &size);
			if (data) {
				stream = Audio::makeRawStream((byte *)data, size, 11025, SPEECH_FLAGS);
				_mixer->playStream(Audio::Mixer::kSpeechSoundType, &_speechHandle, stream, SOUND_SPEECH_ID, speechVol, speechPan);
			}
		} else if (_cowMode == CowPSX && sampleSize != 0xffffffff) {
			_cowFile.seek(index * 2048);
			Common::SeekableReadStream *tmp = _cowFile.readStream(sampleSize);
			assert(tmp);
			stream = Audio::makeXAStream(tmp, 11025);
			_mixer->playStream(Audio::Mixer::kSpeechSoundType, &_speechHandle, stream, SOUND_SPEECH_ID, speechVol, speechPan);
			// with compressed audio, we can't calculate the wave volume.
			// so default to talking.
			for (int cnt = 0; cnt < 480; cnt++)
				_waveVolume[cnt] = true;
			_waveVolPos = 0;
		}
#ifdef USE_FLAC
		else if (_cowMode == CowFLAC) {
			_cowFile.seek(index);
			Common::SeekableReadStream *tmp = _cowFile.readStream(sampleSize);
			assert(tmp);
			stream = Audio::makeFLACStream(tmp, DisposeAfterUse::YES);
			_mixer->playStream(Audio::Mixer::kSpeechSoundType, &_speechHandle, stream, SOUND_SPEECH_ID, speechVol, speechPan);
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
			Common::SeekableReadStream *tmp = _cowFile.readStream(sampleSize);
			assert(tmp);
			stream = Audio::makeVorbisStream(tmp, DisposeAfterUse::YES);
			_mixer->playStream(Audio::Mixer::kSpeechSoundType, &_speechHandle, stream, SOUND_SPEECH_ID, speechVol, speechPan);
			// with compressed audio, we can't calculate the wave volume.
			// so default to talking.
			for (int cnt = 0; cnt < 480; cnt++)
				_waveVolume[cnt] = true;
			_waveVolPos = 0;
		}
#endif
#ifdef USE_MAD
		else if (_cowMode == CowMP3) {
			_cowFile.seek(index);
			Common::SeekableReadStream *tmp = _cowFile.readStream(sampleSize);
			assert(tmp);
			stream = Audio::makeMP3Stream(tmp, DisposeAfterUse::YES);
			_mixer->playStream(Audio::Mixer::kSpeechSoundType, &_speechHandle, stream, SOUND_SPEECH_ID, speechVol, speechPan);
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

int16 *Sound::uncompressSpeech(uint32 index, uint32 cSize, uint32 *size, bool* ok) {
	uint8 *fBuf = (uint8 *)malloc(cSize);
	_cowFile.seek(index);
	_cowFile.read(fBuf, cSize);
	uint32 headerPos = 0;

	while ((READ_BE_UINT32(fBuf + headerPos) != 'data') && (headerPos < 100))
		headerPos++;

	if (headerPos < 100) {
		if (ok != 0)
			*ok = true;
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
			// But not always, apparently. See bug #4002. Is
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
				srcData = (int16 *)fBuf;
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
		srcData = (int16 *)fBuf;
		srcPos = headerPos >> 1;
		uint32 dstPos = 0;
		int16 *dstData = (int16 *)malloc(resSize * 2);
		int32 samplesLeft = resSize;
		while (srcPos < cSize && samplesLeft > 0) {
			length = (int16)(_bigEndianSpeech ? READ_BE_UINT16(srcData + srcPos) : READ_LE_UINT16(srcData + srcPos));
			srcPos++;
			if (length < 0) {
				length = -length;
				if (length > samplesLeft) {
					length = samplesLeft;
					if (ok != 0)
						*ok = false;
				}
				int16 value;
				if (_bigEndianSpeech) {
					value = (int16)SWAP_BYTES_16(*((uint16 *)(srcData + srcPos)));
				} else {
					value = srcData[srcPos];
				}
				for (uint16 cnt = 0; cnt < (uint16)length; cnt++)
					dstData[dstPos++] = value;
				srcPos++;
			} else {
				if (length > samplesLeft) {
					length = samplesLeft;
					if (ok != 0)
						*ok = false;
				}
				if (_bigEndianSpeech) {
					for (uint16 cnt = 0; cnt < (uint16)length; cnt++)
						dstData[dstPos++] = (int16)SWAP_BYTES_16(*((uint16 *)(srcData + (srcPos++))));
				} else {
					memcpy(dstData + dstPos, srcData + srcPos, length * 2);
					dstPos += length;
					srcPos += length;
				}
			}
			samplesLeft -= length;
		}
		if (samplesLeft > 0) {
			memset(dstData + dstPos, 0, samplesLeft * 2);
			if (ok != 0)
				*ok = false;
		}
		if (_cowMode == CowDemo) // demo has wave output size embedded in the compressed data
			*(uint32 *)dstData = 0;
		free(fBuf);
		*size = resSize * 2;
		calcWaveVolume(dstData, resSize);
		return dstData;
	} else {
		if (ok != 0)
			*ok = false;
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
			warning("Wave vol tab too small");
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

void Sound::stopSpeech() {
	_mixer->stopID(SOUND_SPEECH_ID);
}

void Sound::initCowSystem() {
	if (SwordEngine::_systemVars.currentCD == 0)
		return;

	char cowName[25];
	/* look for speech1/2.clu in the data dir
	   and speech/speech.clu (running from cd or using cd layout)
	*/
#ifdef USE_FLAC
	if (!_cowFile.isOpen()) {
		Common::sprintf_s(cowName, "SPEECH%d.CLF", SwordEngine::_systemVars.currentCD);
		_cowFile.open(cowName);
		if (_cowFile.isOpen()) {
			debug(1, "Using FLAC compressed Speech Cluster");
			_cowMode = CowFLAC;
		}
	}
#endif
#ifdef USE_VORBIS
	if (!_cowFile.isOpen()) {
		Common::sprintf_s(cowName, "SPEECH%d.CLV", SwordEngine::_systemVars.currentCD);
		_cowFile.open(cowName);
		if (_cowFile.isOpen()) {
			debug(1, "Using Vorbis compressed Speech Cluster");
			_cowMode = CowVorbis;
		}
	}
#endif
#ifdef USE_MAD
	if (!_cowFile.isOpen()) {
		Common::sprintf_s(cowName, "SPEECH%d.CL3", SwordEngine::_systemVars.currentCD);
		_cowFile.open(cowName);
		if (_cowFile.isOpen()) {
			debug(1, "Using MP3 compressed Speech Cluster");
			_cowMode = CowMP3;
		}
	}
#endif
	if (!_cowFile.isOpen()) {
		Common::sprintf_s(cowName, "SPEECH%d.CLU", SwordEngine::_systemVars.currentCD);
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
				error("Could not open speech.dat");
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
				error("Could not open speech.tab");
			_cowHeaderSize = tableFile.size();
			_cowHeader = (uint32 *)malloc(_cowHeaderSize);
			if (_cowHeaderSize & 3)
				error("Unexpected cow header size %d", _cowHeaderSize);
			for (uint32 cnt = 0; cnt < _cowHeaderSize / 4; cnt++)
				_cowHeader[cnt] = tableFile.readUint32LE();
		} else {
			_cowHeaderSize = _cowFile.readUint32LE();
			_cowHeader = (uint32 *)malloc(_cowHeaderSize);
			if (_cowHeaderSize & 3)
				error("Unexpected cow header size %d", _cowHeaderSize);
			for (uint32 cnt = 0; cnt < (_cowHeaderSize / 4) - 1; cnt++)
				_cowHeader[cnt] = _cowFile.readUint32LE();
			_currentCowFile = SwordEngine::_systemVars.currentCD;
		}
	} else
		warning("Sound::initCowSystem: Can't open SPEECH%d.CLU", SwordEngine::_systemVars.currentCD);
}

void Sound::closeCowSystem() {
	_cowFile.close();
	free(_cowHeader);
	_cowHeader = NULL;
	_currentCowFile = 0;
}

// New stuff

//  This function checks to see whether a specific sound effect has completed
//  playing yet.  It returns 1 if finished, 0 if still playing
//
int32 Sound::CheckSampleStatus(int32 id) {
	uint32 i = 0;
	Common::StackLock lock(_soundMutex);
	while (i < MAX_FX) {
		if (fxSampleID[i] == id) {
			//if ((AIL_sample_status(hSampleFX[i]) == SMP_DONE) && (fxSampleBusy[i]) && (!fxPaused[i])) {
				fxSampleBusy[i] = 0;
				return (1);
			//} else {
			//	return (0);
			//}
		}
		i += 1;
	}

	return (0);
}

int32 Sound::CheckSpeechStatus() {
	if (!speechSampleBusy || speechSamplePaused)
		return (1);

	//if (AIL_sample_status(hSampleSpeech) == SMP_DONE) {
		speechSampleBusy = 0;
		RestoreMusicVolume();
		return (1);
	//}
	//return (0);
}

int32 Sound::PlaySpeech(void *wavData, int32 size) {
	speechSize = size;
	speechCount = 0;

	//  Check to see if the handle is free, else free it
	if (speechSampleBusy)
		StopSpeech();

	//  Reset the sample parameters
	//AIL_init_sample(hSampleSpeech);
	speechSampleBusy = 1;

	//  And point the sample to our new wav data
	if (true/* AIL_set_sample_file(hSampleSpeech, wavData, -1) == 0*/) {
		speechSampleBusy = 0;
		return (0);
	} else {
		//  We have found a free handle, and the wav file header
		//  has been successfully parsed.
		//
		//  Modify the volume according to the master volume
		//AIL_set_sample_volume(hSampleSpeech, 4 * (volSpeech[0] + volSpeech[1]));

		//  Now set the pan position for the sample
		//AIL_set_sample_pan(hSampleSpeech, 64 + (4 * (volSpeech[1] - volSpeech[0])));

		//  Start the sample
		//AIL_start_sample(hSampleSpeech);

		ReduceMusicVolume();

		//  and exit the function.
		return (1);
	}
}

int32 Sound::StopSpeech() {
	if (true/* AIL_sample_status(hSampleSpeech) != SMP_DONE*/) {
		//AIL_end_sample(hSampleSpeech);
		speechSampleBusy = 0;
		RestoreMusicVolume();
		return (1);
	}
	return (0);
}

static void soundCallback(void *refCon) {
	Sound *snd = (Sound *)refCon;
	Common::StackLock lock(snd->_soundMutex);

	if (snd->volumeFadingFlag) {
		snd->volumeCount += 1;
		if (snd->volumeCount > 128 / snd->volumeFadingRate) {
			snd->volumeFadingFlag = 0;
		} else {
			if (snd->volumeFadingFlag == 1) {
				//Fade the volume up
				snd->fadeVolume[0] = snd->masterVolume[0] * snd->volumeCount * snd->volumeFadingRate / 128;
				snd->fadeVolume[1] = snd->masterVolume[1] * snd->volumeCount * snd->volumeFadingRate / 128;
			} else {
				//Fade the volume down
				snd->fadeVolume[0] = snd->masterVolume[0] - (snd->masterVolume[0] * snd->volumeCount * snd->volumeFadingRate / 128);
				snd->fadeVolume[1] = snd->masterVolume[1] - (snd->masterVolume[1] * snd->volumeFadingRate * snd->volumeCount / 128);
			}

			//AIL_set_digital_master_volume(hDigiDriver, (snd->fadeVolume[0] + snd->fadeVolume[1]) / 2);
		}
	}

	if (snd->musicFadingFlag) {
		snd->musicCount += 1;
		if (snd->musicCount > 128 / snd->musicFadingRate) {
			snd->musicFadingFlag = 0;
		} else {
			if (snd->musicFadingFlag == 1) {
				//Fade the volume up
				snd->musicFadeVolume[0] = 8 * snd->volMusic[0] * snd->musicCount * snd->musicFadingRate / 128;
				snd->musicFadeVolume[1] = 8 * snd->volMusic[1] * snd->musicCount * snd->musicFadingRate / 128;
			} else {
				//Fade the volume down
				snd->musicFadeVolume[0] = 8 * snd->volMusic[0] - (8 * snd->volMusic[0] * snd->musicCount * snd->musicFadingRate / 128);
				snd->musicFadeVolume[1] = 8 * snd->volMusic[1] - (8 * snd->volMusic[1] * snd->musicFadingRate * snd->musicCount / 128);
			}

			for (int i = 0; i < MAX_MUSIC; i++) {
				if (snd->streamSamplePlaying[i])
					;
					//AIL_set_sample_volume(snd->hStreamSample[i], (snd->musicFadeVolume[0] + snd->musicFadeVolume[1]) / 2);
			}

		}
	}

	if (snd->fxFadingFlag) {
		snd->fxCount += 1;
		if (snd->fxCount > 128 / snd->fxFadingRate) {
			snd->fxFadingFlag = 0;
		} else {
			if (snd->fxFadingFlag == 1) {
				//Fade the volume up
				snd->fxFadeVolume[0] = 8 * snd->volFX[0] * snd->fxCount * snd->fxFadingRate / 128;
				snd->fxFadeVolume[1] = 8 * snd->volFX[1] * snd->fxCount * snd->fxFadingRate / 128;
			} else {
				//Fade the volume down
				snd->fxFadeVolume[0] = 8 * snd->volFX[0] - (8 * snd->volFX[0] * snd->fxCount * snd->fxFadingRate / 128);
				snd->fxFadeVolume[1] = 8 * snd->volFX[1] - (8 * snd->volFX[1] * snd->fxFadingRate * snd->fxCount / 128);
			}
			for (int i = 0; i < MAX_FX; i++) {
				if (snd->fxSampleBusy[i])
					;
					//AIL_set_sample_volume(hSampleFX[i], (fxFadeVolume[0] + fxFadeVolume[1]) / 2);
			}
		}
	}
}

void Sound::installFadeTimer() {
	_vm->getTimerManager()->installTimerProc(&soundCallback, 1000000 / TIMER_RATE, this, "AILFadeTimer");
}

void Sound::uninstallFadeTimer() {
	_vm->getTimerManager()->removeTimerProc(&soundCallback);
}

void Sound::PlaySample(int32 fxNo) {
	uint8 *samplePtr;
	uint32 vol[2] = { 0, 0 };
	int32 screen = Logic::_scriptVars[SCREEN];

	samplePtr = (uint8 *)_resMan->fetchRes(getSampleId(fxNo));

	for (int i = 0; i < MAX_ROOMS_PER_FX; i++) {
		if (_fxList[fxNo].roomVolList[i].roomNo != 0) {
			if ((_fxList[fxNo].roomVolList[i].roomNo == screen) ||
				(_fxList[fxNo].roomVolList[i].roomNo == -1)) { // -1 indicates 'all rooms'
				vol[0] = (_fxList[fxNo].roomVolList[i].leftVol);
				vol[1] = (_fxList[fxNo].roomVolList[i].rightVol);

				debug(5, "Sound::PlaySample(): fxNo=%d, vol[0]=%d, vol[1]=%d)",fxNo,vol[0],vol[1]);
				PlayFX(fxNo, _fxList[fxNo].type, samplePtr, vol);
				break;
			}
		} else {
			break;
		}
	}
}

int32 Sound::StreamSample(char filename[], int32 looped) {
	return 0;
}

void Sound::UpdateSampleStreaming() {
	Common::StackLock lock(_soundMutex);
	for (int i = 0; i < MAX_MUSIC; i++) {
		if ((streamSamplePlaying[i]) && (!musicPaused[i])) {
			if (streamSampleFading[i]) {
				if (crossFadeIncrement) {
					crossFadeIncrement = false;

					if (streamSampleFading[i] < 0) {
						//AIL_set_sample_volume(hStreamSample[i], ((0 - streamSampleFading[i]) * 3 * (volMusic[0] + volMusic[1])) / 16);
						streamSampleFading[i] += 1;
						if (streamSampleFading[i] == 0) {
							//AIL_stop_sample(hStreamSample[i]);
							//AIL_release_sample_handle(hStreamSample[i]);
							//close(streamFile[i]);
							streamSamplePlaying[i] = 0;
							//MEM_free_lock(streamBuffer[i][0], bufferSize);
							//MEM_free_lock(streamBuffer[i][1], bufferSize);
						}
					} else {
						//AIL_set_sample_volume(hStreamSample[i], (streamSampleFading[i] * 3 * (volMusic[0] + volMusic[1])) / 16);
						streamSampleFading[i] += 1;
						if (streamSampleFading[i] == 17) {
							streamSampleFading[i] = 0;
						}
					}
				}
			}
			//ServeSample(hStreamSample[i], streamBuffer[i], bufferSize, streamFile[i], i);
			//if (AIL_sample_status(hStreamSample[i]) != SMP_PLAYING) {
			//	AIL_release_sample_handle(hStreamSample[i]);
			//	streamSamplePlaying[i] = FALSE;
			//	close(streamFile[i]);
			//	MEM_free_lock(streamBuffer[i][0], bufferSize);
			//	MEM_free_lock(streamBuffer[i][1], bufferSize);
			//}
		}
	}
}

int32 Sound::PlayFX(int32 fxID, int32 type, void *wavData, uint32 vol[2]) {
	int32 i = 0;
	int32 v0, v1;
	Common::StackLock lock(_soundMutex);
	//  Search through the fx sample handles for a free slot
	while (i < MAX_FX) {
		//  Check to see if the handle is free
		if (fxSampleBusy[i] == 0) {
			//  Reset the sample parameters
			//AIL_init_sample(hSampleFX[i]);
			fxSampleBusy[i] = 1;
			fxSampleID[i] = fxID;

			//if (type == FX_LOOP)
			//	AIL_set_sample_loop_count(hSampleFX[i], 0);

			//  And point the sample to our new wav data
			if (true /* AIL_set_sample_file(hSampleFX[i], wavData, -1) == 0*/) {
				return (0);
			} else {
				//  We have found a free handle, and the wav file header
				//  has been successfully parsed.
				//
				//  Modify the volume according to the master volume
				v0 = volFX[0] * vol[0];
				v1 = volFX[1] * vol[1];

				//AIL_set_sample_volume(hSampleFX[i], (v0 + v1) / 8);

				//  Now set the pan position for the sample
				//AIL_set_sample_pan(hSampleFX[i], 64 + ((v1 - v0) / 4));

				//  Start the sample
				//AIL_start_sample(hSampleFX[i]);

				//  and exit the function.
				return (1);
			}
		}
		i += 1;

	} // while

	return (0);
}

int32 Sound::StopFX(int32 fxID) {
	Common::StackLock lock(_soundMutex);
	for (uint32 i = 0; i < MAX_FX; i++) {
		if (fxSampleID[i] == fxID) {
			//if (AIL_sample_status(hSampleFX[i]) != SMP_DONE) {
			//	AIL_end_sample(hSampleFX[i]);
				fxSampleBusy[i] = 0;
				return (1);
			//}
		}
	}
	return (0);
}

void Sound::clearAllFx() {
	for (int j = _endOfQueue - 1; j >= 0; j--) { // remove them from the end
		if (CheckSampleStatus(_fxQueue[j].id) == 0) // delay countdown was already zero, so the sample has already been played, so check if it's finished
			StopFX(_fxQueue[j].id);
		removeFromQueue(_fxQueue[j].id);
	}

	_endOfQueue = 0; // just to be sure, like
}

void Sound::FadeVolumeDown(int32 rate) {
	Common::StackLock lock(_soundMutex);
	volumeFadingFlag = -1;
	volumeFadingRate = 2 * rate;
	volumeCount = 0;
}

void Sound::FadeVolumeUp(int32 rate) {
	Common::StackLock lock(_soundMutex);
	volumeFadingFlag = 1;
	volumeFadingRate = 2 * rate;
	volumeCount = 0;
}

void Sound::FadeMusicDown(int32 rate) {
	Common::StackLock lock(_soundMutex);
	streamSampleFading[1 - streamSamplePlaying[0]] = -12;
}

void Sound::FadeMusicUp(int32 rate) {
	Common::StackLock lock(_soundMutex);
	musicFadingFlag = 1;
	musicFadingRate = 2 * rate;
	musicCount = 0;
}

void Sound::FadeFxDown(int32 rate) {
	Common::StackLock lock(_soundMutex);
	fxFadingFlag = -1;
	fxFadingRate = 2 * rate;
	fxCount = 0;
}

void Sound::FadeFxUp(int32 rate) {
	Common::StackLock lock(_soundMutex);
	fxFadingFlag = 1;
	fxFadingRate = 2 * rate;
	fxCount = 0;
}

int32 Sound::GetSpeechSize(void *compData) {
	typedef struct wavHeader {
		char riff[4];
		int fileLength;
		char wavID[4];
		char format[4];
		int formatLen;
		short int formatTag;
		short int channels;
		short int samplesPerSec;
		short int avgBytesPerSec;
		short int blockAlign;
		short int formatSpecific;
		short int unused[8];
	} wavHeader;

	wavHeader *head;

	head = (wavHeader *)compData;

	//Pdebug("Getting speech size : %d", head->fileLength + 8);
	//Pdebug("riff %c%c%c%c", head->riff[0], head->riff[1], head->riff[2], head->riff[3]);
	//Pdebug("file length %d\n", head->fileLength);

	return (head->fileLength + 8);
}

void Sound::ReduceMusicVolume() {
	Common::StackLock lock(_soundMutex);
	musicFadeVolume[0] = volMusic[0] * MUSIC_UNDERSCORE / 100;
	musicFadeVolume[1] = volMusic[0] * MUSIC_UNDERSCORE / 100;

	//AIL_set_sample_volume(hStreamSample[0], (musicFadeVolume[0] + musicFadeVolume[1]) * 3);
}

void Sound::RestoreMusicVolume() {
	Common::StackLock lock(_soundMutex);
	//AIL_set_sample_volume(hStreamSample[0], (volMusic[0] + volMusic[1]) * 3);
}

void Sound::SetCrossFadeIncrement() {
	crossFadeIncrement = true;
}

void Sound::PauseSpeech() {
	if ((speechSampleBusy) && (!speechSamplePaused)) {
		speechSamplePaused = true;
		//AIL_stop_sample(hSampleSpeech);
	}
}

void Sound::UnpauseSpeech() {
	if ((speechSampleBusy) && (speechSamplePaused)) {
		speechSamplePaused = false;
		//AIL_resume_sample(hSampleSpeech);
	}
}

void Sound::PauseMusic() {
	Common::StackLock lock(_soundMutex);
	for (int32 i = 0; i < MAX_MUSIC; i++) {
		if (streamSamplePlaying[i]) {
			musicPaused[i] = true;
			//AIL_stop_sample(hStreamSample[i]);
		}
	}
}

void Sound::UnpauseMusic() {
	Common::StackLock lock(_soundMutex);
	for (int32 i = 0; i < MAX_MUSIC; i++) {
		if (musicPaused[i]) {
			//AIL_resume_sample(hStreamSample[i]);
			musicPaused[i] = false;
		}
	}
}

void Sound::PauseFx() {
	Common::StackLock lock(_soundMutex);
	for (uint32 i = 0; i < MAX_FX; i++) {
		if (fxSampleBusy[i]) {
			//AIL_stop_sample(hSampleFX[i]);
			fxPaused[i] = true;
		}
	}
}

void Sound::UnpauseFx() {
	Common::StackLock lock(_soundMutex);
	for (uint32 i = 0; i < MAX_FX; i++) {
		if (fxPaused[i]) {
			//AIL_resume_sample(hSampleFX[i]);
			fxPaused[i] = false;
		}
	}
}

} // End of namespace Sword1
